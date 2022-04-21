// -*-c++-*-
/**
 * TuningsImpl.h
 * Copyright 2019-2020 Paul Walker
 * Released under the MIT License. See LICENSE.md
 *
 * This contains the nasty nitty gritty implementation of the api in Tunings.h. You probably
 * don't need to read it unless you have found and are fixing a bug, are curious, or want
 * to add a feature to the API. For usages of this library, the documentation in Tunings.h and
 * the usages in tests/all_tests.cpp should provide you more than enough guidance.
 */

#ifndef __INCLUDE_TUNINGS_IMPL_H
#define __INCLUDE_TUNINGS_IMPL_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <sstream>
#include <cctype>

namespace Tunings
{
// Thank you to: https://gist.github.com/josephwb/df09e3a71679461fc104
inline std::istream &getlineEndingIndependent(std::istream &is, std::string &t)
{
    t.clear();

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    for (;;)
    {
        int c = sb->sbumpc();
        switch (c)
        {
        case '\n':
            return is;
        case '\r':
            if (sb->sgetc() == '\n')
            {
                sb->sbumpc();
            }
            return is;
        case EOF:
            is.setstate(std::ios::eofbit);
            if (t.empty())
            {
                is.setstate(std::ios::badbit);
            }
            return is;
        default:
            t += (char)c;
        }
    }
}

inline double locale_atof(const char *s)
{
    double result = 0;
    std::istringstream istr(s);
    istr.imbue(std::locale("C"));
    istr >> result;
    return result;
}

inline Tone toneFromString(const std::string &line, int lineno)
{
    Tone t;
    t.stringRep = line;
    if (line.find(".") != std::string::npos)
    {
        t.type = Tone::kToneCents;
        t.cents = locale_atof(line.c_str());
    }
    else
    {
        t.type = Tone::kToneRatio;
        auto slashPos = line.find("/");
        if (slashPos == std::string::npos)
        {
            t.ratio_n = atoi(line.c_str());
            t.ratio_d = 1;
        }
        else
        {
            t.ratio_n = atoi(line.substr(0, slashPos).c_str());
            t.ratio_d = atoi(line.substr(slashPos + 1).c_str());
        }

        if (t.ratio_n == 0 || t.ratio_d == 0)
        {
            std::string s = "Invalid tone in SCL file.";
            if (lineno >= 0)
                s += "Line " + std::to_string(lineno) + ".";
            s += " Line is '" + line + "'.";
            throw TuningError(s);
        }
        // 2^(cents/1200) = n/d
        // cents = 1200 * log(n/d) / log(2)

        t.cents = 1200 * log(1.0 * t.ratio_n / t.ratio_d) / log(2.0);
    }
    t.floatValue = t.cents / 1200.0 + 1.0;
    return t;
}

inline Scale readSCLStream(std::istream &inf)
{
    std::string line;
    const int read_header = 0, read_count = 1, read_note = 2, trailing = 3;
    int state = read_header;

    Scale res;
    std::ostringstream rawOSS;
    int lineno = 0;
    while (getlineEndingIndependent(inf, line))
    {
        rawOSS << line << "\n";
        lineno++;

        if ((state == read_note && line.empty()) || line[0] == '!')
        {
            continue;
        }
        switch (state)
        {
        case read_header:
            res.description = line;
            state = read_count;
            break;
        case read_count:
            res.count = atoi(line.c_str());
            if (res.count < 1)
            {
                throw TuningError("Invalid SCL note count.");
            }
            state = read_note;
            break;
        case read_note:
            auto t = toneFromString(line, lineno);
            res.tones.push_back(t);
            if ((int)res.tones.size() == res.count)
                state = trailing;

            break;
        }
    }

    if (!(state == read_note || state == trailing))
    {
        std::ostringstream oss;
        oss << "Incomplete SCL content. Only able to read " << lineno
            << " lines of data. Found content up to ";
        switch (state)
        {
        case read_header:
            oss << "reading header.";
            break;
        case read_count:
            oss << "reading scale count.";
            break;
        default:
            oss << "unknown state.";
            break;
        }
        throw TuningError(oss.str());
    }

    if ((int)res.tones.size() != res.count)
    {
        std::string s =
            "Read fewer notes than count in file. Count = " + std::to_string(res.count) +
            " notes. Array size = " + std::to_string(res.tones.size());
        throw TuningError(s);
    }
    res.rawText = rawOSS.str();
    return res;
}

inline Scale readSCLFile(std::string fname)
{
    std::ifstream inf;
    inf.open(fname);
    if (!inf.is_open())
    {
        std::string s = "Unable to open file '" + fname + "'";
        throw TuningError(s);
    }

    auto res = readSCLStream(inf);
    res.name = fname;
    return res;
}

inline Scale parseSCLData(const std::string &d)
{
    std::istringstream iss(d);
    auto res = readSCLStream(iss);
    res.name = "Scale from patch";
    return res;
}

inline Scale evenTemperament12NoteScale()
{
    std::string data = R"SCL(! 12 Tone Equal Temperament.scl
!
12 Tone Equal Temperament | ED2-12 - Equal division of harmonic 2 into 12 parts
 12
!
 100.00000
 200.00000
 300.00000
 400.00000
 500.00000
 600.00000
 700.00000
 800.00000
 900.00000
 1000.00000
 1100.00000
 2/1
)SCL";
    return parseSCLData(data);
}

inline Scale evenDivisionOfSpanByM(int Span, int M)
{
    if (Span <= 0)
        throw Tunings::TuningError("Span should be a positive number. You entered " +
                                   std::to_string(Span));
    if (M <= 0)
        throw Tunings::TuningError(
            "You must divide the period into at least one step. You entered " + std::to_string(M));

    std::ostringstream oss;
    oss.imbue(std::locale("C"));
    oss << "! Automatically generated ED" << Span << "-" << M << " scale\n";
    oss << "Automatically generated ED" << Span << "-" << M << " scale\n";
    oss << M << "\n";
    oss << "!\n";

    double topCents = 1200.0 * log(1.0 * Span) / log(2.0);
    double dCents = topCents / M;
    for (int i = 1; i < M; ++i)
        oss << std::fixed << dCents * i << "\n";
    oss << Span << "/1\n";

    return parseSCLData(oss.str());
}

inline KeyboardMapping readKBMStream(std::istream &inf)
{
    std::string line;

    KeyboardMapping res;
    std::ostringstream rawOSS;
    res.keys.clear();

    enum parsePosition
    {
        map_size = 0,
        first_midi,
        last_midi,
        middle,
        reference,
        freq,
        degree,
        keys,
        trailing
    };
    parsePosition state = map_size;

    int lineno = 0;
    while (getlineEndingIndependent(inf, line))
    {
        rawOSS << line << "\n";
        lineno++;
        if (line[0] == '!')
        {
            continue;
        }

        if (line == "x")
            line = "-1";
        else if (state != trailing)
        {
            const char *lc = line.c_str();
            bool validLine = line.length() > 0;
            char badChar = '\0';
            while (validLine && *lc != '\0')
            {
                if (!(*lc == ' ' || std::isdigit(*lc) || *lc == '.' || *lc == (char)13 ||
                      *lc == '\n'))
                {
                    validLine = false;
                    badChar = *lc;
                }
                lc++;
            }
            if (!validLine)
            {
                throw TuningError("Invalid line " + std::to_string(lineno) + ". line='" + line +
                                  "'. Bad character is '" + badChar + "/" +
                                  std::to_string((int)badChar) + "'");
            }
        }

        int i = std::atoi(line.c_str());
        double v = locale_atof(line.c_str());

        switch (state)
        {
        case map_size:
            res.count = i;
            break;
        case first_midi:
            res.firstMidi = i;
            break;
        case last_midi:
            res.lastMidi = i;
            break;
        case middle:
            res.middleNote = i;
            break;
        case reference:
            res.tuningConstantNote = i;
            break;
        case freq:
            res.tuningFrequency = v;
            res.tuningPitch = res.tuningFrequency / 8.17579891564371;
            break;
        case degree:
            res.octaveDegrees = i;
            break;
        case keys:
            res.keys.push_back(i);
            if ((int)res.keys.size() == res.count)
                state = trailing;
            break;
        case trailing:
            break;
        }
        if (!(state == keys || state == trailing))
            state = (parsePosition)(state + 1);
        if (state == keys && res.count == 0)
            state = trailing;
    }

    if (!(state == keys || state == trailing))
    {
        std::ostringstream oss;
        oss << "Incomplete KBM stream. Only able to read " << lineno << " lines. Read up to ";
        switch (state)
        {
        case map_size:
            oss << "map size.";
            break;
        case first_midi:
            oss << "first midi note.";
            break;
        case last_midi:
            oss << "last midi note.";
            break;
        case middle:
            oss << "scale zero note.";
            break;
        case reference:
            oss << "scale reference note.";
            break;
        case freq:
            oss << "scale reference frequency.";
            break;
        case degree:
            oss << "scale degree.";
            break;
        default:
            oss << "unknown state";
            break;
        }
        throw TuningError(oss.str());
    }

    if ((int)res.keys.size() != res.count)
    {
        throw TuningError("Different number of keys than mapping file indicates. Count is " +
                          std::to_string(res.count) + " and we parsed " +
                          std::to_string(res.keys.size()) + " keys.");
    }

    res.rawText = rawOSS.str();
    return res;
}

inline KeyboardMapping readKBMFile(std::string fname)
{
    std::ifstream inf;
    inf.open(fname);
    if (!inf.is_open())
    {
        std::string s = "Unable to open file '" + fname + "'";
        throw TuningError(s);
    }

    auto res = readKBMStream(inf);
    res.name = fname;
    return res;
}

inline KeyboardMapping parseKBMData(const std::string &d)
{
    std::istringstream iss(d);
    auto res = readKBMStream(iss);
    res.name = "Mapping from patch";
    return res;
}

inline Tuning::Tuning() : Tuning(evenTemperament12NoteScale(), KeyboardMapping()) {}
inline Tuning::Tuning(const Scale &s) : Tuning(s, KeyboardMapping()) {}
inline Tuning::Tuning(const KeyboardMapping &k) : Tuning(evenTemperament12NoteScale(), k) {}

inline Tuning::Tuning(const Scale &s, const KeyboardMapping &k)
{
    scale = s;
    keyboardMapping = k;

    if (s.count <= 0)
        throw TuningError("Unable to tune to a scale with no notes. Your scale provided " +
                          std::to_string(s.count) + " notes.");

    // From the KBM Spec: When not all scale degrees need to be mapped, the size of the map can be
    // smaller than the size of the scale.
    if (k.octaveDegrees > s.count)
    {
        throw TuningError("Unable to apply mapping of size " + std::to_string(k.octaveDegrees) +
                          " to smaller scale of size " + std::to_string(s.count));
    }

    double pitches[N];

    int posPitch0 = 256 + k.tuningConstantNote;
    int posScale0 = 256 + k.middleNote;

    double pitchMod = log(k.tuningPitch) / log(2) - 1;

    int scalePositionOfTuningNote = k.tuningConstantNote - k.middleNote;
    if (k.count > 0)
        scalePositionOfTuningNote = k.keys[scalePositionOfTuningNote];

    double tuningCenterPitchOffset;
    if (scalePositionOfTuningNote == 0)
        tuningCenterPitchOffset = 0;
    else
    {
        double tshift = 0;
        double dt = s.tones[s.count - 1].floatValue - 1.0;
        while (scalePositionOfTuningNote < 0)
        {
            scalePositionOfTuningNote += s.count;
            tshift += dt;
        }
        while (scalePositionOfTuningNote > s.count)
        {
            scalePositionOfTuningNote -= s.count;
            tshift -= dt;
        }

        if (scalePositionOfTuningNote == 0)
            tuningCenterPitchOffset = -tshift;
        else
            tuningCenterPitchOffset =
                s.tones[scalePositionOfTuningNote - 1].floatValue - 1.0 - tshift;
    }

    for (int i = 0; i < N; ++i)
    {
        // TODO: ScaleCenter and PitchCenter are now two different notes.
        int distanceFromPitch0 = i - posPitch0;
        int distanceFromScale0 = i - posScale0;

        if (distanceFromPitch0 == 0)
        {
            pitches[i] = 1;
            lptable[i] = pitches[i] + pitchMod;
            ptable[i] = pow(2.0, lptable[i]);
            scalepositiontable[i] = scalePositionOfTuningNote % s.count;
#if DEBUG_SCALES
            std::cout << "PITCH: i=" << i << " n=" << i - 256 << " p=" << pitches[i]
                      << " lp=" << lptable[i] << " tp=" << ptable[i]
                      << " fr=" << ptable[i] * 8.175798915 << std::endl;
#endif
        }
        else
        {
            /*
              We used to have this which assumed 1-12
              Now we have our note number, our distance from the
              center note, and the key remapping
              int rounds = (distanceFromScale0-1) / s.count;
              int thisRound = (distanceFromScale0-1) % s.count;
            */

            int rounds;
            int thisRound;
            int disable = false;
            if ((k.count == 0))
            {
                rounds = (distanceFromScale0 - 1) / s.count;
                thisRound = (distanceFromScale0 - 1) % s.count;
            }
            else
            {
                /*
                ** Now we have this situation. We are at note i so we
                ** are m away from the center note which is distanceFromScale0
                **
                ** If we mod that by the mapping size we know which note we are on
                */
                int mappingKey = distanceFromScale0 % k.count;
                if (mappingKey < 0)
                    mappingKey += k.count;
                // Now have we gone off the end
                int rotations = 0;
                int dt = distanceFromScale0;
                if (dt > 0)
                {
                    while (dt >= k.count)
                    {
                        dt -= k.count;
                        rotations++;
                    }
                }
                else
                {
                    while (dt < 0)
                    {
                        dt += k.count;
                        rotations--;
                    }
                }

                int cm = k.keys[mappingKey];
                int push = 0;
                if (cm < 0)
                {
                    disable = true;
                }
                else
                {
                    push = mappingKey - cm;
                }

                if (k.octaveDegrees > 0 && k.octaveDegrees != k.count)
                {
                    rounds = rotations;
                    thisRound = cm - 1;
                    if (thisRound < 0)
                    {
                        thisRound = k.octaveDegrees - 1;
                        rounds--;
                    }
                }
                else
                {
                    rounds = (distanceFromScale0 - push - 1) / s.count;
                    thisRound = (distanceFromScale0 - push - 1) % s.count;
                }

#ifdef DEBUG_SCALES
                if (i > 256 + 53 && i < 265 + 85)
                    std::cout << "MAPPING n=" << i - 256 << " pushes ds0=" << distanceFromScale0
                              << " cmc=" << k.count << " tr=" << thisRound << " r=" << rounds
                              << " mk=" << mappingKey << " cm=" << cm << " push=" << push
                              << " dis=" << disable << " mk-p-1=" << mappingKey - push - 1
                              << " rotations=" << rotations << " od=" << k.octaveDegrees
                              << std::endl;
#endif
            }

            if (thisRound < 0)
            {
                thisRound += s.count;
                rounds -= 1;
            }

            if (disable)
            {
                pitches[i] = 0;
                scalepositiontable[i] = -1;
            }
            else
            {
                pitches[i] = s.tones[thisRound].floatValue +
                             rounds * (s.tones[s.count - 1].floatValue - 1.0) -
                             tuningCenterPitchOffset;
                scalepositiontable[i] = (thisRound + 1) % s.count;
            }

            lptable[i] = pitches[i] + pitchMod;
            ptable[i] = pow(2.0, pitches[i] + pitchMod);

#if DEBUG_SCALES
            if (i > 296 && i < 340)
                std::cout << "PITCH: i=" << i << " n=" << i - 256 << " ds0=" << distanceFromScale0
                          << " dp0=" << distanceFromPitch0 << " r=" << rounds << " t=" << thisRound
                          << " p=" << pitches[i] << " t=" << s.tones[thisRound].floatValue << " "
                          << s.tones[thisRound].cents << " dis=" << disable << " tp=" << ptable[i]
                          << " fr=" << ptable[i] * 8.175798915 << " tcpo="
                          << tuningCenterPitchOffset

                          //<< " l2p=" << log(otp)/log(2.0)
                          //<< " l2p-p=" << log(otp)/log(2.0) - pitches[i] - rounds - 3
                          << std::endl;
#endif
        }
    }
}

inline double Tuning::frequencyForMidiNote(int mn) const
{
    auto mni = std::min(std::max(0, mn + 256), N - 1);
    return ptable[mni] * MIDI_0_FREQ;
}

inline double Tuning::frequencyForMidiNoteScaledByMidi0(int mn) const
{
    auto mni = std::min(std::max(0, mn + 256), N - 1);
    return ptable[mni];
}

inline double Tuning::logScaledFrequencyForMidiNote(int mn) const
{
    auto mni = std::min(std::max(0, mn + 256), N - 1);
    return lptable[mni];
}

inline int Tuning::scalePositionForMidiNote(int mn) const
{
    auto mni = std::min(std::max(0, mn + 256), N - 1);
    return scalepositiontable[mni];
}

inline bool Tuning::isMidiNoteMapped(int mn) const
{
    auto mni = std::min(std::max(0, mn + 256), N - 1);
    return scalepositiontable[mni] >= 0;
}

inline Tuning Tuning::withSkippedNotesInterpolated() const
{
    Tuning res = *this;
    for (int i = 1; i < N - 1; ++i)
    {
        if (scalepositiontable[i] < 0)
        {
            int nxt = i + 1;
            int prv = i - 1;
            while (prv >= 0 && scalepositiontable[prv] < 0)
                prv--;
            while (nxt < N && scalepositiontable[nxt] < 0)
                nxt++;
            float dist = nxt - prv;
            float frac = (i - prv) / dist;
            res.lptable[i] = (1.0 - frac) * lptable[prv] + frac * lptable[nxt];
            res.ptable[i] = pow(2.0, res.lptable[i]);
        }
    }
    return res;
}

inline KeyboardMapping::KeyboardMapping()
    : count(0), firstMidi(0), lastMidi(127), middleNote(60), tuningConstantNote(60),
      tuningFrequency(MIDI_0_FREQ * 32.0), tuningPitch(32.0), octaveDegrees(0), rawText(""),
      name("")
{
    std::ostringstream oss;
    oss.imbue(std::locale("C"));
    oss << "! Default KBM file\n";
    oss << count << "\n"
        << firstMidi << "\n"
        << lastMidi << "\n"
        << middleNote << "\n"
        << tuningConstantNote << "\n"
        << tuningFrequency << "\n"
        << octaveDegrees << "\n";
    rawText = oss.str();
}

inline KeyboardMapping tuneA69To(double freq) { return tuneNoteTo(69, freq); }

inline KeyboardMapping tuneNoteTo(int midiNote, double freq)
{
    return startScaleOnAndTuneNoteTo(60, midiNote, freq);
}

inline KeyboardMapping startScaleOnAndTuneNoteTo(int scaleStart, int midiNote, double freq)
{
    std::ostringstream oss;
    oss.imbue(std::locale("C"));
    oss << "! Automatically generated mapping, tuning note " << midiNote << " to " << freq
        << " Hz\n"
        << "!\n"
        << "! Size of map\n"
        << 0 << "\n"
        << "! First and last MIDI notes to map - map the entire keyboard\n"
        << 0 << "\n"
        << 127 << "\n"
        << "! Middle note where the first entry in the scale is mapped.\n"
        << scaleStart << "\n"
        << "! Reference note where frequency is fixed\n"
        << midiNote << "\n"
        << "! Frequency for MIDI note " << midiNote << "\n"
        << freq << "\n"
        << "! Scale degree for formal octave. This is am empty mapping, so:\n"
        << 0 << "\n"
        << "! Mapping. This is an empty mapping so list no keys\n";

    return parseKBMData(oss.str());
}

} // namespace Tunings
#endif
