#include "PluginData.h"

#include "dexedPreset.h"
#include <cassert>

static uint8_t sysexChecksum(const uint8_t *sysex, int size)
{
    int sum = 0;
    for(int i = 0; i < size; sum -= sysex[i++]);
    return sum & 0x7F;
}

static void exportSysexPgm(uint8_t *dest, uint8_t *src)
{
    uint8_t header[] = { 0xF0, 0x43, 0x00, 0x00, 0x01, 0x1B };
    memcpy(dest, header, 6);
    memcpy(dest+6, src, 155); // copy 1 unpacked voices
    // make checksum for dump
    uint8_t footer[] = { sysexChecksum(src, 155), 0xF7 };

    memcpy(dest+161, footer, 2);
}

Cartridge::Cartridge()
{
    assert(sizeof(m_voiceData) == Dexed_01_syx_len);
    memcpy(m_voiceData, Dexed_01_syx, sizeof(m_voiceData));
}

void
Cartridge::setHeader()
{
    uint8_t voiceHeader[] = SYSEX_HEADER;
    memcpy(m_voiceData, voiceHeader, 6);
    m_voiceData[4102] = sysexChecksum(m_voiceData+6, 4096);
    m_voiceData[4103] = 0xF7;
}

char 
Cartridge::normparm(char value, char max, int id)
{
    if (value <= max && value >= 0)
        return value;
    // if this is beyond the max, we expect a 0-255 range, normalize this
    // to the expected return value; and this value as a random data.
    value = abs(value);
    char v = ((float)value)/255 * max;
    return v;
}

void
Cartridge::unpackProgram(uint8_t *pgm, int idx) // assumes cartridge is loaded.
{
    uint8_t const *bulk = ((uint8_t *)m_voiceData + 6 + (idx * 128));
    for (int op = 0; op < 6; op++) 
    {
        // eg rate and level, brk pt, depth, scaling
        for(int i=0; i<11; i++) 
        {
            uint8_t currparm = bulk[op * 17 + i] & 0x7F; // mask BIT7 (don't care per sysex spec) 
            pgm[op * 21 + i] = normparm(currparm, 99, i);
        }
    
        memcpy(pgm + op * 21, bulk + op * 17, 11);
        char leftrightcurves = bulk[op * 17 + 11]&0xF; // bits 4-7 don't care per sysex spec
        pgm[op * 21 + 11] = leftrightcurves & 3;
        pgm[op * 21 + 12] = (leftrightcurves >> 2) & 3;
        char detune_rs = bulk[op * 17 + 12]&0x7F;
        pgm[op * 21 + 13] = detune_rs & 7;
        char kvs_ams = bulk[op * 17 + 13]&0x1F; // bits 5-7 don't care per sysex spec
        pgm[op * 21 + 14] = kvs_ams & 3;
        pgm[op * 21 + 15] = (kvs_ams >> 2) & 7;
        pgm[op * 21 + 16] = bulk[op * 17 + 14]&0x7F;  // output level
        char fcoarse_mode = bulk[op * 17 + 15]&0x3F; //bits 6-7 don't care per sysex spec
        pgm[op * 21 + 17] = fcoarse_mode & 1;
        pgm[op * 21 + 18] = (fcoarse_mode >> 1)&0x1F;
        pgm[op * 21 + 19] = bulk[op * 17 + 16]&0x7F;  // fine freq
        pgm[op * 21 + 20] = (detune_rs >> 3) &0x7F;
    }

    for (int i=0; i<8; i++)  
    {
        uint8_t currparm = bulk[102 + i] & 0x7F; // mask BIT7 (don't care per sysex spec)
        pgm[126+i] = normparm(currparm, 99, 126+i);
    }
    pgm[134] = normparm(bulk[110]&0x1F, 31, 134); // bits 5-7 are don't care per sysex spec

    char oks_fb = bulk[111]&0xF;//bits 4-7 are don't care per spec
    pgm[135] = oks_fb & 7;
    pgm[136] = oks_fb >> 3;
    pgm[137] = bulk[112] & 0x7F; // lfs
    pgm[138] = bulk[113] & 0x7F; // lfd
    pgm[139] = bulk[114] & 0x7F; // lpmd
    pgm[140] = bulk[115] & 0x7F; // lamd
    char lpms_lfw_lks = bulk[116] & 0x7F;
    pgm[141] = lpms_lfw_lks & 1;
    pgm[142] = (lpms_lfw_lks >> 1) & 7;
    pgm[143] = lpms_lfw_lks >> 4;
    pgm[144] = bulk[117] & 0x7F;
    for (int name_idx = 0; name_idx < 10; name_idx++) 
    {
        pgm[145 + name_idx] = bulk[118 + name_idx] & 0x7F;
    } //name_idx
}

int 
Cartridge::saveVoice(char const *fn)
{
    return -1;
#if 0
    setHeader();
    
    if(! f.existsAsFile() ) {
        // file doesn't exists, create it
        return f.replaceWithData(voiceData, SYSEX_SIZE);
    }
    
    std::unique_ptr<juce::FileInputStream> fis = f.createInputStream();
    if ( fis == NULL )
        return -1;
    
    uint8 buffer[65535];
    int sz = fis->read(buffer, 65535);
    
    // if the file is smaller than 4104, it probably needs to be overridden.
    if ( sz <= 4104 ) {
        return f.replaceWithData(voiceData, SYSEX_SIZE);
    }

    // To avoid to erase the performance data, we skip the sysex stream until
    // we see the header 0xF0, 0x43, 0x00, 0x09, 0x20, 0x00
    
    int pos = 0;
    bool found = 0;
    while(pos < sz) {
        // corrupted sysex, erase everything :
        if ( buffer[pos] != 0xF0 )
            return f.replaceWithData(voiceData, SYSEX_SIZE);
        
        uint8_t header[] = SYSEX_HEADER;
        if ( memcmp(buffer+pos, header, 6) ) {
            found = true;
            memcpy(buffer+pos, voiceData, SYSEX_SIZE);
            break;
        } else {
            for(;pos<sz;pos++) {
                if ( buffer[pos] == 0xF7 )
                    break;
            }
        }
    }
    
    if ( ! found )
        return -1;
    
    return f.replaceWithData(buffer, sz);
#endif
}

/*static*/ std::string 
Cartridge::normalizePgmName(const char *sysexName)
{
    char buffer[11];
    
    memcpy(buffer, sysexName, 10);
    
    for (int j = 0; j < 10; j++) 
    {
        char c = (unsigned char) buffer[j];
        c &= 0x7F; // strip don't care most-significant bit from name
        switch (c) 
        {
        case 92:
            c = 'Y';
            break; /* yen */
        case 126:
            c = '>';
            break; /* >> */
        case 127:
            c = '<';
            break; /* << */
        default:
            if (c < 32 || c > 127)
                c = 32;
            break;
        }
        buffer[j] = c;
    }
    buffer[10] = 0;
    
    return std::string(buffer);
}

int 
Cartridge::load(const uint8_t *stream, int size) 
{
    const uint8_t *pos = stream;
    
    if(size < 4096) 
    {
        memcpy(m_voiceData+6, pos, size);
        TRACE("too small sysex rc=2");
        return 2;
    }
    
    if(pos[0] != 0xF0) 
    {
        // it is not, just copy the first 4096 bytes
        memcpy(m_voiceData + 6, pos, 4096);
        TRACE("stream is not a sysex rc=2");
        return 2;
    }
    
    // limit the size of the sysex scan
    if (size > 65535)
        size = 65535;
    
    // we loop until we find something that looks like a DX7 cartridge (based on size)
    while(size >= 4104) 
    {
        // it was a sysex first, now random data; return random
        if(pos[0] != 0xF0) 
        {
            memcpy(m_voiceData + 6, stream, 4096);
            TRACE("stream was a sysex, but not anymore rc=2");
            return 2;
        }
        
        // check if this is the size of a DX7 sysex cartridge
        for(int i=0;i<size;i++) 
        {
            if(pos[i] == 0xF7) 
            {
                if (i == SYSEX_SIZE - 1) 
                {
                    memcpy(m_voiceData, pos, SYSEX_SIZE);
                    if(sysexChecksum(m_voiceData + 6, 4096) == pos[4102]) 
                    {
                        TRACE("valid sysex found!");
                        return 0;
                    } 
                    else 
                    {
                        TRACE("sysex found, but checksum doesn't match rc=1");
                        return 1;
                    }
                }
                size -= i;
                pos += i;
                TRACE("end of sysex with wrong DX size... still scanning stream: size=%d", i);
                break;
            }
        }
        TRACE("sysex stream parsed without any end message, skipping...");
        break;
    }
    
    // it is a sysex, but doesn't seems to be related to any DX series ...
    memcpy(m_voiceData + 6, stream, 4096);
    TRACE("nothing in the sysex stream was DX related rc=2");
    return 2;
}
