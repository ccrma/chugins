/**
 *
 * Copyright (c) 2014-2016 Pascal Gauthier.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#ifndef PLUGINDATA_H_INCLUDED
#define PLUGINDATA_H_INCLUDED

#include "Dexed.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <cassert>
#include <vector>

#define SYSEX_HEADER { 0xF0, 0x43, 0x00, 0x09, 0x20, 0x00 }
#define SYSEX_SIZE 4104 /* size of a cartridge (see ../presets) */

class Cartridge 
{
    uint8_t m_voiceData[SYSEX_SIZE];
    uint8_t m_perfData[SYSEX_SIZE];
    
    void setHeader();
    
public:
    Cartridge();
    
    Cartridge(const Cartridge &cpy)
    {
        memcpy(m_voiceData, cpy.m_voiceData, SYSEX_SIZE);
        memcpy(m_perfData, cpy.m_perfData, SYSEX_SIZE);
    }

    Cartridge operator=(const Cartridge other) 
    {
        memcpy(m_voiceData, other.m_voiceData, SYSEX_SIZE);
        memcpy(m_perfData, other.m_perfData, SYSEX_SIZE);
        return *this;
    }

    static std::string 
    normalizePgmName(const char *sysexName);
    
    int load(char const *filepath) 
    {
        std::ifstream istr;
        istr.open(filepath);
        if(!istr.good())
            return -1;
        int rc = load(istr);
        istr.close();
        return rc;
    }
    
    /**
     * Loads sysex stream
     * Returns 0 if it was parsed successfully
     * Returns -1 if it cannot open the stream
     */
    int load(std::ifstream &fis) 
    {
        char buffer[65535];
        fis.seekg(0, fis.end);
        int sz = fis.tellg();
        fis.seekg(0, fis.beg);
        fis.read(buffer, sz);
        if(!fis)
            return -1;
        return load(buffer, sz);
    }
    
    /**
     * Loads sysex buffer
     * Returns 0 if it was parsed successfully
     * Returns 1 if sysex checksum didn't match
     * Returns 2 if no sysex data found, probably random data
     */
    int load(const char *stream, int size);
    
    int saveVoice(char const *fn);
    void saveVoice(uint8_t *sysex) 
    {
        setHeader();
        memcpy(sysex, m_voiceData, SYSEX_SIZE);
    }
    char *getRawVoice() 
    {
        return (char *) m_voiceData + 6;
    }
    char *getVoiceSysex() 
    {
        setHeader();
        return (char *) m_voiceData;
    }
    
    void getProgramNames(std::vector<std::string> &dest) 
    {
        dest.clear();
        for(int i = 0; i < 32; i++)
            dest.push_back(normalizePgmName(getRawVoice() + ((i * 128) + 118)));
    }
    
    char normparm(char value, char max, int id);
    void unpackProgram(uint8_t *pgm, int idx);

    void packProgram(uint8_t *src, int idx, std::string name, char *opSwitch)
    {
        assert(!"unimplemented");
    }
};

#endif  // PLUGINDATA_H_INCLUDED
