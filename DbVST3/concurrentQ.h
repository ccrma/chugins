#ifndef concurrentQ_h
#define concurrentQ_h

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

template <typename T>
class ConcurrentQ
{
public:
    ConcurrentQ()
    {
        m_bailing.store(false);
    }
    ConcurrentQ(const ConcurrentQ&) = delete; // disable copying
    ConcurrentQ& operator=(const ConcurrentQ&) = delete; // disable assignment

    T Pop() 
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        while(m_queue.empty() && IsActive()) 
            m_cond.wait(lk); // <----------- block
        
        if(!IsActive()) return [](){}; // empty 
        auto val = m_queue.front();
        m_queue.pop();
        lk.unlock(); // before notify_one
        m_cond.notify_one();
        return val;
    }

    void Pop(T& item) 
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        while(m_queue.empty() && IsActive())
            m_cond.wait(lk);

        if(!IsActive()) return [](){}; // empty 
        item = m_queue.front();
        m_queue.pop();
        lk.unlock();
        m_cond.notify_one();
    }

    void Push(const T& item) 
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        while(m_queue.size() >= BUFFER_SIZE) 
            m_cond.wait(lk);
        m_queue.push(item);
        lk.unlock();
        m_cond.notify_one();
    }

    int Size()
    {
        // std::unique_lock<std::mutex> lk(m_mutex);
        return m_queue.size();
    }

    void Bail()
    {
        m_bailing.store(true);
    }

    bool IsActive()
    {
        return m_bailing.load() == false;
    }

 private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::atomic<bool> m_bailing;
    const static unsigned int BUFFER_SIZE = 10;
};

#endif