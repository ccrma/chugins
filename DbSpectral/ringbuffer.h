#ifndef ringbuffer_h
#define ringbuffer_h

/*!
 * Simple SPSC ring buffer implementation
 * A Lock free, with no wasted slots ringbuffer implementation based 
 * on (https://github.com/jnk0le/Ring-Buffer) license: CC0 1.0 Universal
 * 
 * modifications/simplifications by Data Batali, same license.
 */

#include <stdint.h>
#include <stddef.h>
#include <limits>
#include <atomic>

/*!
* \brief Lock free, with no wasted slots ringbuffer implementation
*
* \tparam T Type of buffered elements
* \tparam buffer_size Size of the buffer. Must be a power of 2.
* \tparam fake_tso Omit generation of explicit barrier code to avoid 
*         unnecesary instructions in tso scenario (e.g. simple 
*         microcontrollers/single core)
* \tparam cacheline_size Size of the cache line, to insert appropriate padding 
*         in between indexes and buffer
* \tparam index_t Type of array indexing type. Serves also as placeholder for 
*         future implementations.
*/
template<typename T, size_t buffer_size = 16, bool fake_tso = false, 
        size_t cacheline_size = 0, typename index_t = size_t>
class Ringbuffer
{
public:
    /*!
     * \brief Default constructor, will initialize head and tail indexes
     */
    Ringbuffer() : m_head(0), m_tail(0) {}

    /*!
     * \brief Special case constructor to premature out unnecessary initialization code when object is
     * instatiated in .bss section
     * \warning If object is instantiated on stack, heap or inside noinit section then the contents have to be
     * explicitly cleared before use
     * \param dummy Ignored
     */
    Ringbuffer(int dummy) { (void)(dummy); }

    /*!
     * \brief Clear buffer from producer side
     * \warning function may return without performing any action if consumer tries to read data at the same time
     */
    void producerClear(void) 
    {
        // head modification will lead to underflow if cleared during consumer read
        // doing this properly with CAS is not possible without modifying the consumer code
        consumerClear();
    }

    /*!
     * \brief Clear buffer from consumer side
     */
    void consumerClear(void) 
    {
        m_tail.store(m_head.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    /*!
     * \brief Check if buffer is empty
     * \return True if buffer is empty
     */
    bool isEmpty(void) const 
    {
        return readAvailable() == 0;
    }

    /*!
     * \brief Check if buffer is full
     * \return True if buffer is full
     */
    bool isFull(void) const 
    {
        return writeAvailable() == 0;
    }

    /*!
     * \brief Check how many elements can be read from the buffer
     * \return Number of elements that can be read
     */
    index_t readAvailable(void) const 
    {
        return m_head.load(s_index_acquire_barrier) - 
               m_tail.load(std::memory_order_relaxed);
    }

    /*!
     * \brief Check how many elements can be written into the buffer
     * \return Number of free slots that can be be written
     */
    index_t writeAvailable(void) const 
    {
        return buffer_size - 
            (m_head.load(std::memory_order_relaxed) - 
             m_tail.load(s_index_acquire_barrier));
    }

    /*!
     * \brief Inserts data into internal buffer, without blocking
     * \param data element to be inserted into internal buffer
     * \return True if data was inserted
     */
    bool insert(T data)
    {
        index_t tmp_head = m_head.load(std::memory_order_relaxed);
        if((tmp_head - m_tail.load(s_index_acquire_barrier)) == buffer_size)
            return false;
        else
        {
            m_data_buff[tmp_head++ & s_buffer_mask] = data;
            std::atomic_signal_fence(std::memory_order_release);
            m_head.store(tmp_head, s_index_release_barrier);
        }
        return true;
    }

    /*!
     * \brief Reads one element from internal buffer without blocking
     * \param[out] data Pointer to memory location where removed element will be stored
     * \param reset value to clear buffer value to after reading.
     * \return True if data was fetched from the internal buffer
     */
    bool remove(T* data, T reset) 
    {
        index_t tmp_tail = m_tail.load(std::memory_order_relaxed);

        if(tmp_tail ==m_head.load(s_index_acquire_barrier))
            return false;
        else
        {
            *data = m_data_buff[tmp_tail & s_buffer_mask];
            m_data_buff[tmp_tail++ & s_buffer_mask] = reset;
            std::atomic_signal_fence(std::memory_order_release);
            m_tail.store(tmp_tail, s_index_release_barrier);
        }
        return true;
    }

    /*!
     * \brief Insert multiple elements into internal buffer without blocking
     *
     * This function will insert as much data as possible from given buffer.
     *
     * \param[in] buff Pointer to buffer with data to be inserted from
     * \param count Number of elements to write from the given buffer
     * \return Number of elements written into internal buffer
     */
    size_t writeBuff(const T* buff, size_t count, size_t step, bool accum=false);
    size_t writeBuff(const T* buff, size_t count) { this->writeBuff(buff, count, count); }

    /*!
     * \brief Load multiple elements from internal buffer without blocking
     *
     * This function will read up to specified amount of data. When
     * sucessful, it will forward the read pointer by step.
     *
     * \param[out] buff Pointer to buffer where data will be loaded into
     * \param count Number of elements to load into the given buffer
     * \param step Number of elements to count as read.  Must be <= count.
     * \return Number of elements that were read from internal buffer
     */
    size_t readBuff(T* buff, size_t count, size_t step);
    size_t readBuff(T* buff, size_t count) { return this->readBuff(buff, count, count); }

private:
    constexpr static index_t s_buffer_mask = buffer_size-1; //!< bitwise mask for a given buffer size
    constexpr static std::memory_order s_index_acquire_barrier = fake_tso ?
                std::memory_order_relaxed
            : std::memory_order_acquire; // do not load from, or store to buffer before confirmed by the opposite side
    constexpr static std::memory_order s_index_release_barrier = fake_tso ?
                std::memory_order_relaxed
            : std::memory_order_release; // do not update own side before all operations on data_buff committed

    alignas(cacheline_size) std::atomic<index_t> m_head; //!< head index
    alignas(cacheline_size) std::atomic<index_t> m_tail; //!< tail index

    // put buffer after variables so everything can be reached with short offsets
    alignas(cacheline_size) T m_data_buff[buffer_size]; //!< actual buffer

    // let's assert that no UB will be compiled in
    static_assert((buffer_size != 0), "buffer cannot be of zero size");
    static_assert((buffer_size & s_buffer_mask) == 0, "buffer size is not a power of 2");
    static_assert(sizeof(index_t) <= sizeof(size_t),
        "indexing type size is larger than size_t, operation is not lock free and doesn't make sense");

    static_assert(std::numeric_limits<index_t>::is_integer, "indexing type is not integral type");
    static_assert(!(std::numeric_limits<index_t>::is_signed), "indexing type shall not be signed");
    static_assert(s_buffer_mask <= ((std::numeric_limits<index_t>::max)() >> 1),
            "buffer size is too large for a given indexing type (maximum size for n-bit type is 2^(n-1))");
};

/* ---------------------------------------------------------------------------- */

template<typename T, size_t buffer_size, bool fake_tso, size_t cacheline_size, typename index_t>
size_t Ringbuffer<T, buffer_size, fake_tso, cacheline_size, index_t>
::writeBuff(const T* buff, size_t count, size_t step, bool accum)
{
    index_t available = 0;
    index_t tmp_head = m_head.load(std::memory_order_relaxed);
    index_t orig_head = tmp_head;
    size_t to_write = count;

    available = buffer_size - (tmp_head - m_tail.load(s_index_acquire_barrier));

    if(available < count) // do not write more than we can
        to_write = available;

    if(accum)
    {
        for(size_t i = 0; i < to_write; i++)
            m_data_buff[tmp_head++ & s_buffer_mask] += buff[i];
    }
    else
    {
        for(size_t i = 0; i < to_write; i++)
            m_data_buff[tmp_head++ & s_buffer_mask] = buff[i];
    }

    tmp_head = orig_head + step;

    std::atomic_signal_fence(std::memory_order_release);
    m_head.store(tmp_head, s_index_release_barrier);

    return to_write;
}

template<typename T, size_t buffer_size, bool fake_tso, size_t cacheline_size, typename index_t>
size_t Ringbuffer<T, buffer_size, fake_tso, cacheline_size, index_t>
::readBuff(T* buff, size_t count, size_t step)
{
    index_t available = 0;
    index_t tmp_tail = m_tail.load(std::memory_order_relaxed);
    index_t new_tail; 
    size_t to_read = count;

    available = m_head.load(s_index_acquire_barrier) - tmp_tail;

    if(available < count) // do not read more than we can
        to_read = available;

    if(available < step)
        new_tail = tmp_tail + to_read;
    else
        new_tail = tmp_tail + step;

    // maybe divide it into 2 separate reads
    for(size_t i = 0; i < to_read; i++)
        buff[i] = m_data_buff[tmp_tail++ & s_buffer_mask];

    std::atomic_signal_fence(std::memory_order_release);
    m_tail.store(new_tail, s_index_release_barrier);

    return to_read;
}

#endif 