#ifndef complexDelay_h
#define complexDelay_h

#include <vector>
#include <complex>
#include <cassert>

class ComplexDelayTable
{
public:
    ComplexDelayTable() {}
    ~ComplexDelayTable() {}

    void Resize(int freqBins, int maxDelay/*samples*/) 
    {
        if(m_delayLines.size() != freqBins)
            m_delayLines.resize(freqBins);

        for(int i=0;i<m_delayLines.size();i++)
            m_delayLines[i].Resize(maxDelay);
    }

    void PutSamp(int bin, float r, float i)
    {
        assert(bin < m_delayLines.size());
        m_delayLines[bin].PutSamp(r, i);
    }

    void GetSamp(int bin, float *r, float *i, int sampsAgo)
    {
        assert(bin < m_delayLines.size());
        m_delayLines[bin].GetSamp(r, i, sampsAgo);
    }

private:
    struct complexDelay
    {
        complexDelay()
        {
            m_head = 0;
        }
        void Resize(int maxDelay)
        {
            int oldSize = m_data.size();
            if(oldSize != maxDelay)
            {
                m_data.resize(maxDelay);
                for(int i=oldSize;i<maxDelay;i++)
                {
                    m_data[i].real(0.f);
                    m_data[i].imag(0.f);
                }
            }
        }
        void PutSamp(float r, float i)
        {
            t_complex &x = m_data[m_head++];
            x.real(r);
            x.imag(i);
            if(m_head == m_data.size())
                m_head = 0;
        }
        void GetSamp(float *r, float *i, int sampsAgo)
        {
            int outSampIndex = (m_head - sampsAgo);
            if(outSampIndex < 0)
            {
                while(outSampIndex < 0)
                    outSampIndex += m_data.size();
            }
            t_complex &x = m_data[outSampIndex];
            *r = x.real();
            *i =  x.imag();
        }
        using t_complex = std::complex<float>;
        std::vector<t_complex> m_data;
        int m_head;
    };
    std::vector<complexDelay> m_delayLines;
};

#endif