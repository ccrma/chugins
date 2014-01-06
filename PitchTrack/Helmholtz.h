#include <Instrument.h>
#include <Helmholtz_dsp.h>
#include <Ougens.h>

class HELMHOLTZ : public Instrument {
  
 public:
  HELMHOLTZ();
  virtual ~HELMHOLTZ();
  virtual int init(double p[], int);
  virtual int run();
  virtual int configure();

 private:
  void doupdate();
  Oequalizer *_filt;
  float* _in;
  float* _hhbuf_in;
  float* _hhbuf_out;
  int _inchan, _branch;
  float _fidelity, _amp;
  double _freq;
  float _fidelitythreshold, _maxfreq;
  Helmholtz *_helmholtz;
};
