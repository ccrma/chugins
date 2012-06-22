
class MCSine extends Chugen
{
    440 => float m_freq;
    0 => float m_epsilon;
    
    m_freq => freq;
    
    1 => float m_x;
    0 => float m_y;
    
    fun float tick(float in)
    {
        m_x + m_epsilon*m_y => m_x;
        -m_epsilon*m_x + m_y => m_y;
                
        return m_y;
    }
    
    fun float freq(float f)
    {
        f => m_freq;
        2*Math.sin(2*pi*(m_freq/(second/samp))/2.0) => m_epsilon;
        return m_freq;
    }
    
    fun float freq()
    {
        return m_freq;
    }
}


MCSine s => dac;
1 => s.gain;
1::day => now;

