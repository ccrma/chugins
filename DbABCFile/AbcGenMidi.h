#ifndef AbcGenMidi_h
#define AbcGenMidi_h

class AbcGenMidi
{
public:
    static const int MAXCHORDNAMES = 80;
    static const int MAXPARTS = 100;
    static const int MAXCHANS = 16;
    static const int DIV = 480;
    static const int MAXTRACKS = 40;

    enum miditracktype
    {
        NOTES, 
        WORDS, 
        NOTEWORDS, 
        GCHORDS, 
        DRUMS, 
        DRONE
    };
    struct trackstruct 
    {
        miditracktype tracktype;
        int voicenum;
        int midichannel; 
    };

    AbcGenMidi();
    ~AbcGenMidi();

public:
    /* XXX: part state might better live within AbcStore? */
    std::string partspec;
    int parts, partno, partlabel;
    int part_start[26], part_count[26];

    int channel_in_use[MAXCHANS + 3]; 
    int additive;
    int nseg, segnum, segden;
    int ngain[32];
    float maxdur, fdur[32], fdursum[32];
    int beatmodel;
    int stressmodel;
    int stress_pattern_loaded;
    int barflymode;
    int bar_num;
    int bar_denom;
    trackstruct trackdescriptor[MAXTRACKS];
    int drum_map[256];

    void Init();

    void reduce(int *num, int *denom);
    void set_meter();
    void addunits();
    void set_gchords(char const *);
    void set_drums(char const *);
    /* required by queues.c */
    void midi_noteoff();
    void progress_sequence();

    int parse_drummap(char const *);

    int parse_stress_params(char *s);
    void readstressfile(char const *filepath);
    void calculate_stress_parameters(int time_num, int time_denom);
};


#endif