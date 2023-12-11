//-------------------------------------------------------------------
// name: Line.cpp
//
// desc: Line UGen is a sequential linear ramp generator. It is
// similar to line~ in Max/MSP where you can provide either a single
// ramp or a list of ramps.
//
// example:
//
// authors: Nick Shaheed (nshaheed@ccrma.stanford.edu)
// date: Fall 2023
//
//-------------------------------------------------------------------

/*
  - copy over vals
  - TODO, add stop function which stops at current val
  - TODO, add start which continues it
  - TODO examples
  - TODO get durs/targets/etc
  - TODO actually return event and broadcast it
  - TODO doc strings
 */

// include chuck dynamic linking header
#include "chugin.h"

// general includes
#include <iostream>
#include <vector>


// declaration of chugin constructor
CK_DLL_CTOR( line_ctor );
CK_DLL_CTOR( line_ctor_single );
CK_DLL_CTOR( line_ctor_singleTarget );
CK_DLL_CTOR( line_ctor_singleTargetStart );
CK_DLL_CTOR( line_ctor_setArray );
CK_DLL_CTOR( line_ctor_setArrayStart );
// declaration of chugin desctructor
CK_DLL_DTOR( line_dtor );

// single-ramp with 0 start and 1 target
CK_DLL_MFUN( line_setSingle );
// single-ramp with 0 start and user-defined target
CK_DLL_MFUN( line_setSingleTarget );
// single-ramp with user-defined start and user-defined target
CK_DLL_MFUN( line_setSingleTargetStart );

// multi-ramp with 0 start
CK_DLL_MFUN( line_setArray );
// multi-ramp with user-defined start
CK_DLL_MFUN( line_setArrayStart );

// getters
CK_DLL_MFUN( line_getInitial );
CK_DLL_MFUN( line_getTargets );
CK_DLL_MFUN( line_getDurations );

// keyOn functions
CK_DLL_MFUN( line_keyOn );
// immediately reset to initial value before ramping
CK_DLL_MFUN( line_keyOnInitial );
// single-ramp with 0 start and 1 target
CK_DLL_MFUN( line_keyOnSingle );
// single-ramp with 0 start and user-defined target
CK_DLL_MFUN( line_keyOnSingleTarget );
// single-ramp with user-defined start and user-defined target
CK_DLL_MFUN( line_keyOnSingleTargetStart );
// multi-ramp with 0 start
CK_DLL_MFUN( line_keyOnArray );
// multi-ramp with user-defined start
CK_DLL_MFUN( line_keyOnArrayStart );


// keyOff function
CK_DLL_MFUN( line_keyOff );
// keyOff function (set dur)
CK_DLL_MFUN( line_keyOffDur );
// keyOff function (set target)
CK_DLL_MFUN( line_keyOffTarget );
// keyOff function (set dur and target)
CK_DLL_MFUN( line_keyOffDurTarget );

// overwriting last function
CK_DLL_MFUN( line_last );


// for chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICK( line_tick );

// this is a special offset reserved for chugin internal data
t_CKINT line_data_offset = 0;


// internal Line class
class Line
{
public:
  // constructor
  Line( t_CKFLOAT fs, CK_DL_API API, Chuck_VM* VM, Chuck_VM_Shred* shred )
  {
    m_fs = fs;
    m_elapsed = 1;
    m_state = -1;

    m_API = API;
    m_VM = VM;
    m_shred = shred;

    // setup defaults
    m_initial = 0;
    m_value = 0;

    std::vector<t_CKFLOAT> targets{1};
    m_targets = targets;

    std::vector<t_CKDUR> durs{1000};
    m_durs = durs;
    m_keyOffDur = 1000;

    setRates();
    setCumulative();
  }

  SAMPLE tick(SAMPLE in) {
    // envelope isn't on, do nothing.
    if (m_state == -1) return m_initial;
    // key off is enabled
    if (m_state == -2) return keyOffTick(in);

    t_CKFLOAT target = m_targets[m_state];
    t_CKFLOAT rate = m_rates[m_state];

    // Calculate how much of the current ramp has elapsed
    t_CKDUR curr = m_elapsed;
    if (m_state > 0) 
        curr = m_elapsed - m_durs_cumulative[m_state - 1];

    // Get the value that the current state starts in
    t_CKFLOAT val = m_initial;
    if (m_state > 0)
        val = m_targets[m_state - 1];
    
    // calculate the current value of the envelope
    val += rate * curr;
    m_value = val;

    // clip envelope if moving in a positive direction
    if (rate >= 0 && m_value > target) {
        m_value = target;
    }
    // clip envelope if moving in a negative direction
    if (rate < 0 && m_value < target) {
        m_value = target;
    }

    // increment time
    m_elapsed++;

    // advance envelope state...
    if (m_state == m_durs_cumulative.size() - 1) {
        // do nothing
    }
    else if (m_elapsed >= m_durs_cumulative[m_state]) {
        m_state++;
    }

    // scale the input by the current value of the envelope;
    return in * m_value;
  }

  void set(t_CKDUR d) {
    m_initial = 0;
    m_value = 0;
    std::vector<t_CKFLOAT> targets{1};
    m_targets = targets;

    std::vector<t_CKDUR> durs{d};
    m_durs = durs;

    setRates();
    setCumulative();
  }

  void setValue(t_CKFLOAT val) {
    m_initial = val;
    m_value = val;

    setRates();
    setCumulative();
  }

  void set(t_CKDUR d, t_CKFLOAT target) {
    m_initial = 0;
    // m_value = 0;
    std::vector<t_CKFLOAT> targets{target};
    m_targets = targets;

    std::vector<t_CKDUR> durs{d};
    m_durs = durs;

    setRates();
    setCumulative();
  }

  void set(t_CKDUR d, t_CKFLOAT target, t_CKFLOAT initial) {
    m_initial = initial;
    m_value = initial;
    std::vector<t_CKFLOAT> targets{target};
    m_targets = targets;

    std::vector<t_CKDUR> durs{d};
    m_durs = durs;

    setRates();
    setCumulative();
  }

  void set(std::vector<t_CKDUR> d, std::vector<t_CKFLOAT> target) {
    m_initial = 0;
    m_value = 0;
    m_targets = target;
    m_durs = d;

    setRates();
    setCumulative();
  }

  void set(std::vector<t_CKDUR> d, std::vector<t_CKFLOAT> target, t_CKFLOAT initial) {
    m_initial = initial;
    m_value = initial;
    m_targets = target;
    m_durs = d;

    setRates();
    setCumulative();
  }

  t_CKDUR keyOn() {
    m_elapsed = 1;
    m_state = 0;
    m_initial = m_value; // whatever the current value is

    // update first rate
    updateInitialRate();

    return m_durs_cumulative.back();
  }

  t_CKDUR keyOffDur(t_CKDUR d) {
    m_keyOffDur = d;
    return d;
  }

  t_CKDUR keyOff() {
    m_elapsed = 1;
    m_state = -2;

    m_keyOffRate = (m_initial - m_value) / m_keyOffDur;
    return m_keyOffDur;
  }

  t_CKDUR keyOff(t_CKDUR d) {
    m_elapsed = 1;
    m_state = -2;

    m_keyOffDur = d;
    m_keyOffRate = (m_initial - m_value) / m_keyOffDur;

    return m_keyOffDur;
  }

  t_CKDUR keyOffTarget(t_CKFLOAT target) {
    m_elapsed = 1;
    m_state = -2;

    m_initial = target;
    m_keyOffRate = (m_initial - m_value) / m_keyOffDur;

    return m_keyOffDur;
  }

  t_CKDUR keyOff(t_CKFLOAT target, t_CKDUR d) {
    m_elapsed = 1;
    m_state = -2;
    m_initial = target;

    m_keyOffDur = d;
    m_keyOffRate = (m_initial - m_value) / m_keyOffDur;

    return m_keyOffDur;
  }

  t_CKFLOAT last() {
    return m_value;
  }

  t_CKFLOAT initial() {
    return m_initial;
  }

  std::vector<t_CKFLOAT> targets() {
      return m_targets;
  }

  std::vector<t_CKDUR> durations() {
      return m_durs;
  }

  // return position in envelope.
  // 0 indicates that it's not on
  // x.%% - x indicates state/index of which envlope
  // %% indicates the percentage complete of that envelope
  t_CKFLOAT pos() {
      // TODO
      return 0.0;
  }


private:
  // API-related stuff
  CK_DL_API m_API;
  Chuck_VM* m_VM;
  Chuck_VM_Shred* m_shred;

  // the envelope params
  t_CKFLOAT m_initial;
  t_CKFLOAT m_value;
  t_CKFLOAT m_fs;

  std::vector<t_CKFLOAT> m_targets;
  std::vector<t_CKDUR> m_durs, m_durs_cumulative;
  std::vector<t_CKFLOAT> m_rates;

  t_CKDUR m_keyOffDur;
  t_CKFLOAT m_keyOffRate;

  // the current state of the envelope
  // -1: not on
  // 0,1,...: index of the current envelope
  int m_state;
  t_CKDUR m_elapsed;

  // TODO: I don't need this, I just need an envelope of size 1...
  bool m_singular;

  Chuck_Object* m_keyOn;
  Chuck_Object* m_keyOff;

  // calculate the rates of every interval
  void setRates() {
    m_rates.clear();

    if (m_durs.size() == 0) return;

    // setup first rate
    t_CKDUR rate = (m_targets[0] - m_initial) / m_durs[0];
    m_rates.push_back(rate);

    for (int i = 1; i < m_durs.size(); i++) {
      rate = (m_targets[i] - m_targets[i-1]) / m_durs[i];
      m_rates.push_back(rate);
    }
  }

  // update the first rate based off of what the current value is
  // (rather than m_initial)
  void updateInitialRate() {
    if (m_durs.size() == 0) return;

    t_CKDUR rate = (m_targets[0] - m_value) / m_durs[0];
    m_rates[0] = rate;
  }

  // calculate the cumulative durs for the envelope
  void setCumulative() {
    m_durs_cumulative.clear();

    if (m_durs.size() == 0) return;

    // setup first duration
    m_durs_cumulative.push_back(m_durs[0]);

    for (int i = 1; i < m_durs.size(); i++) {
      t_CKDUR dur = m_durs_cumulative[i-1] + m_durs[i];
      m_durs_cumulative.push_back(dur);
    }
  }

  SAMPLE keyOffTick(SAMPLE in) {
    m_value += m_keyOffRate;
    if (m_keyOffRate > 0 && m_value > m_initial) m_value = m_initial;
    if (m_keyOffRate < 0 && m_value < m_initial) m_value = m_initial;

    m_elapsed++;

    // scale the input by the current value of the envelope;
    return in * m_value;
  }

};


// query function: chuck calls this when loading the chugin
// (NOTE developer will need to modify this function to add additional functions to this chugin)
CK_DLL_QUERY( Line )
{
  // hmm, don't change this...
  QUERY->setname( QUERY, "Line" );

  // ------------------------------------------------------------------------
  // Chugin metadata
  // ------------------------------------------------------------------------
  QUERY->setinfo( QUERY, CHUGIN_INFO_AUTHORS, "Nick Shaheed" );
  // the version string of this chugin, e.g., "v1.2"
  QUERY->setinfo( QUERY, CHUGIN_INFO_CHUGIN_VERSION, "1.0" );
  // text description of this chugin; what is it? what does it do? who is it for?
  QUERY->setinfo( QUERY, CHUGIN_INFO_DESCRIPTION, "Envelope of a arbitrary ramps (ala Max/PD's line~)." );
  // (optional) URL of the homepage for this chugin
  QUERY->setinfo( QUERY, CHUGIN_INFO_URL, "https://github.com/ccrma/chugins/tree/main/Line" );
  // (optional) contact email
  QUERY->setinfo( QUERY, CHUGIN_INFO_EMAIL, "nshaheed@ccrma.stanford.edu" );

  // begin the class definition
  // NOTE to create a non-UGen class, change the second argument
  // to extend a different ChucK class (e.g., "Object")
  QUERY->begin_class( QUERY, "Line", "UGen" );

  // register the constructor (probably no need to change)
  QUERY->add_ctor( QUERY, line_ctor );

  QUERY->add_ctor( QUERY, line_ctor_single );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Set ramp, going [0,1] over duration (default 1000::samp).");

  QUERY->add_ctor( QUERY, line_ctor_singleTarget );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Set ramp, going [0,target] over duration.");

  QUERY->add_ctor( QUERY, line_ctor_singleTargetStart );
  QUERY->add_arg( QUERY, "float", "initial" );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Set ramp, going [initial,target] over duration.");

  QUERY->add_ctor( QUERY, line_ctor_setArray );
  QUERY->add_arg( QUERY, "float[]", "targets" );
  QUERY->add_arg( QUERY, "dur[]", "durations" );
  QUERY->doc_func( QUERY, "Set ramp, starting at 0 and going through all (target, duration) pairs.");

  QUERY->add_ctor( QUERY, line_ctor_setArrayStart );
  QUERY->add_arg( QUERY, "float", "initial" );
  QUERY->add_arg( QUERY, "float[]", "targets" );
  QUERY->add_arg( QUERY, "dur[]", "durations" );
  QUERY->doc_func( QUERY, "Set ramp, starting at initial and going through all (target, duration) pairs.");


  // register the destructor (probably no need to change)
  QUERY->add_dtor( QUERY, line_dtor );

  // for UGens only: add tick function
  // NOTE a non-UGen class should remove or comment out this next line
  QUERY->add_ugen_func( QUERY, line_tick, NULL, 1, 1 );
  // NOTE: if this is to be a UGen with more than 1 channel,
  // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
  // and declare a tickf function using CK_DLL_TICKF

  // singular set with all defaults
  QUERY->add_mfun( QUERY, line_setSingle, "int", "set" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Set ramp, going [0,1] over duration (default 1000::samp).");

  // singular set with default start
  QUERY->add_mfun( QUERY, line_setSingleTarget, "int", "set" );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Set ramp, going [0,target] over duration.");

  // singular set with no defaults
  QUERY->add_mfun( QUERY, line_setSingleTargetStart, "int", "set" );
  QUERY->add_arg( QUERY, "float", "initial" );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Set ramp, going [initial,target] over duration.");

  // array set with default start
  QUERY->add_mfun( QUERY, line_setArray, "int", "set" );
  QUERY->add_arg( QUERY, "float[]", "targets" );
  QUERY->add_arg( QUERY, "dur[]", "durations" );
  QUERY->doc_func( QUERY, "Set ramp, starting at 0 and going through all (target, duration) pairs.");

  // array set with no defaults
  QUERY->add_mfun( QUERY, line_setArrayStart, "int", "set" );
  QUERY->add_arg( QUERY, "float", "initial" );
  QUERY->add_arg( QUERY, "float[]", "targets" );
  QUERY->add_arg( QUERY, "dur[]", "durations" );
  QUERY->doc_func( QUERY, "Set ramp, starting at initial and going through all (target, duration) pairs.");

  // getters
  QUERY->add_mfun( QUERY, line_getInitial, "float", "initial");
  QUERY->doc_func( QUERY, "Get initial value of ramp.");

  QUERY->add_mfun( QUERY, line_getTargets, "float[]", "targets");
  QUERY->doc_func( QUERY, "Get ramp target values.");

  QUERY->add_mfun(QUERY, line_getDurations, "dur[]", "durations");
  QUERY->doc_func(QUERY, "Get ramp durations.");


  // keyOn function
  QUERY->add_mfun( QUERY, line_keyOn, "dur", "keyOn" );
  QUERY->doc_func( QUERY, "Trigger ramp");

  QUERY->add_mfun( QUERY, line_keyOnInitial, "dur", "keyOn" );
  QUERY->add_arg( QUERY, "float", "inital" );
  QUERY->doc_func( QUERY, "Immediately set output to intial, before ramping.");

  QUERY->add_mfun( QUERY, line_keyOnSingle, "dur", "keyOn" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Set and trigger ramp (see set(...)). Returns total duration of envelope.");

  // singular keyOn with default start
  QUERY->add_mfun( QUERY, line_keyOnSingleTarget, "dur", "keyOn" );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Set and trigger ramp (see set(...)). Returns total duration of envelope.");

  // singular keyOn with no defaults
  QUERY->add_mfun( QUERY, line_keyOnSingleTargetStart, "dur", "keyOn" );
  QUERY->add_arg( QUERY, "float", "initial" );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Set and trigger ramp (see set(...)). Returns total duration of envelope.");

  // array keyOn with default start
  QUERY->add_mfun( QUERY, line_keyOnArray, "dur", "keyOn" );
  QUERY->add_arg( QUERY, "float[]", "targets" );
  QUERY->add_arg( QUERY, "dur[]", "durations" );
  QUERY->doc_func( QUERY, "Set and trigger ramp (see set(...)). Returns total duration of envelope.");

  // array keyOn with no defaults
  QUERY->add_mfun( QUERY, line_keyOnArrayStart, "dur", "keyOn" );
  QUERY->add_arg( QUERY, "float", "initial" );
  QUERY->add_arg( QUERY, "float[]", "targets" );
  QUERY->add_arg( QUERY, "dur[]", "durations" );
  QUERY->doc_func( QUERY, "Set and trigger ramp (see set(...)). Returns total duration of envelope.");

  // keyOff functions
  QUERY->add_mfun( QUERY, line_keyOff, "dur", "keyOff" );
  QUERY->doc_func( QUERY, "Ramp down from current value to initial (default duration is 1000:samp). Returns total duration of envelope.");

  QUERY->add_mfun( QUERY, line_keyOffDur, "dur", "keyOff" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Ramp down from current value to initial over duration. Returns total duration of envelope.");

  QUERY->add_mfun( QUERY, line_keyOffTarget, "dur", "keyOff" );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->doc_func( QUERY, "Ramp down from current value to target. Sets initial value to target. Returns total duration of envelope.");

  QUERY->add_mfun( QUERY, line_keyOffDurTarget, "dur", "keyOff" );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->add_arg( QUERY, "dur", "duration" );
  QUERY->doc_func( QUERY, "Ramp down from current value to target over duration. Sets initial value to target. Returns total duration of envelope.");

  QUERY->add_mfun( QUERY, line_last, "float", "last" );
  QUERY->doc_func( QUERY, "get the last sample value of the unit generator.");

  // keyOff function - take in a dur and go to initial
  // - keyOff(dur d) // go to inital
  // - keyOff(dur d, float target) // go to target

  // this reserves a variable in the ChucK internal class to store
  // referene to the c++ class we defined above
  line_data_offset = QUERY->add_mvar( QUERY, "int", "@l_data", false );

  // end the class definition
  // IMPORTANT: this MUST be called!
  QUERY->end_class( QUERY );

  // wasn't that a breeze?
  return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR( line_ctor )
{
  // get the offset where we'll store our internal c++ class pointer
  OBJ_MEMBER_INT( SELF, line_data_offset ) = 0;

  // instantiate our internal c++ class representation
  Line * l_obj = new Line( API->vm->srate(VM), API, VM, SHRED );

  // store the pointer in the ChucK object member
  OBJ_MEMBER_INT( SELF, line_data_offset ) = (t_CKINT)l_obj;
}


// implementation for the constructor
CK_DLL_CTOR( line_ctor_single )
{
  line_ctor(SELF, ARGS, VM, SHRED, API);
  Chuck_DL_Return* ret = new Chuck_DL_Return;
  line_setSingle(SELF, ARGS, ret, VM, SHRED, API);
}


CK_DLL_CTOR( line_ctor_singleTarget )
{
  line_ctor(SELF, ARGS, VM, SHRED, API);
  Chuck_DL_Return* ret = new Chuck_DL_Return;
  line_setSingleTarget(SELF, ARGS, ret, VM, SHRED, API);
}


CK_DLL_CTOR( line_ctor_singleTargetStart )
{
  line_ctor(SELF, ARGS, VM, SHRED, API);
  Chuck_DL_Return* ret = new Chuck_DL_Return;
  line_setSingleTargetStart(SELF, ARGS, ret, VM, SHRED, API);
}


CK_DLL_CTOR( line_ctor_setArray )
{
  line_ctor(SELF, ARGS, VM, SHRED, API);
  Chuck_DL_Return* ret = new Chuck_DL_Return;
  line_setArray(SELF, ARGS, ret, VM, SHRED, API);
}


CK_DLL_CTOR( line_ctor_setArrayStart )
{
  line_ctor(SELF, ARGS, VM, SHRED, API);
  Chuck_DL_Return* ret = new Chuck_DL_Return;
  line_setArrayStart(SELF, ARGS, ret, VM, SHRED, API);
}


// implementation for the destructor
CK_DLL_DTOR( line_dtor )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  // clean up (this macro tests for NULL, deletes, and zeros out the variable)
  CK_SAFE_DELETE( l_obj );
  // set the data field to 0
  OBJ_MEMBER_INT( SELF, line_data_offset ) = 0;
}


// implementation for tick function
CK_DLL_TICK( line_tick )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT(SELF, line_data_offset);

  // invoke our tick function; store in the magical out variable
  if( l_obj ) *out = l_obj->tick( in );

  // yes
  return TRUE;
}


// single-ramp with 0 start and 1 target
CK_DLL_MFUN( line_setSingle )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKDUR duration = GET_NEXT_DUR( ARGS );

  l_obj->set(duration);

  RETURN->v_int = true;
}


// single-ramp with 0 start and user-defined target
CK_DLL_MFUN( line_setSingleTarget )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKFLOAT target = GET_NEXT_FLOAT( ARGS );
  t_CKDUR duration = GET_NEXT_DUR( ARGS );

  l_obj->set(duration, target);
  RETURN->v_int = true;
}

// single-ramp with user-defined start and user-defined target
CK_DLL_MFUN( line_setSingleTargetStart )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKFLOAT initial = GET_NEXT_FLOAT( ARGS );
  t_CKFLOAT target = GET_NEXT_FLOAT( ARGS );
  t_CKDUR duration = GET_NEXT_DUR( ARGS );

  l_obj->set(duration, target, initial);
  RETURN->v_int = true;
}

// multi-ramp with 0 start
CK_DLL_MFUN( line_setArray )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  // get arguments
  Chuck_ArrayFloat* targets = (Chuck_ArrayFloat *)GET_NEXT_OBJECT( ARGS );
  Chuck_ArrayFloat* durations = (Chuck_ArrayFloat *)GET_NEXT_OBJECT( ARGS );

  // bounds checking
  t_CKINT durations_size, targets_size;
  durations_size = API->object->array_float_size(durations);
  targets_size = API->object->array_float_size(targets);

  if (durations_size != targets_size) {
    API->vm->throw_exception("LineMismatchedArrays", "Duration and target arrays must be same size in a Line.set(...)", nullptr);
    RETURN->v_int = false;
    return;
  }

  // copy over array contents
  std::vector<t_CKDUR> durations_vec;
  std::vector<t_CKFLOAT> targets_vec;

  for (int i = 0; i < durations_size; i++) {
    durations_vec.push_back( API->object->array_float_get_idx( durations, i ) );
    targets_vec.push_back( API->object->array_float_get_idx( targets, i ) );
  }

  l_obj->set(durations_vec, targets_vec);
  RETURN->v_int = true;
}

// multi-ramp with user-defined start
CK_DLL_MFUN( line_setArrayStart )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  // get arguments
  t_CKFLOAT initial = GET_NEXT_FLOAT( ARGS );
  Chuck_ArrayFloat* targets = (Chuck_ArrayFloat *)GET_NEXT_OBJECT( ARGS );
  Chuck_ArrayFloat* durations = (Chuck_ArrayFloat *)GET_NEXT_OBJECT( ARGS );

  // bounds checking
  t_CKINT durations_size, targets_size;
  durations_size = API->object->array_float_size(durations);
  targets_size = API->object->array_float_size(targets);

  if (durations_size != targets_size) {
    API->vm->throw_exception("LineMismatchedArrays", "Duration and target arrays must be same size in a Line.set(...)", nullptr);
    RETURN->v_int = false;
    return;
  }

  // copy over array contents
  std::vector<t_CKDUR> durations_vec;
  std::vector<t_CKFLOAT> targets_vec;

  for (int i = 0; i < durations_size; i++) {
    durations_vec.push_back( API->object->array_float_get_idx( durations, i ) );
    targets_vec.push_back( API->object->array_float_get_idx( targets, i ) );
  }

  l_obj->set(durations_vec, targets_vec, initial);
  RETURN->v_int = true;
}

// https://github.com/ccrma/chuck/blob/b47a2c1e4cc2d234d76b6b7733de839df75477ed/src/core/ugen_stk.cpp#L3698
CK_DLL_MFUN( line_keyOn )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  RETURN->v_dur = l_obj->keyOn();
}

// immediately reset to initial value before ramping
CK_DLL_MFUN( line_keyOnInitial )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKFLOAT init = GET_NEXT_FLOAT( ARGS );

  l_obj->setValue(init);
  RETURN->v_dur = l_obj->keyOn();
}

// single-ramp with 0 start and 1 target
CK_DLL_MFUN( line_keyOnSingle )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKDUR duration = GET_NEXT_DUR( ARGS );

  l_obj->set(duration);
  RETURN->v_dur = l_obj->keyOn();
}

// single-ramp with 0 start and user-defined target
CK_DLL_MFUN( line_keyOnSingleTarget )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKFLOAT target = GET_NEXT_FLOAT( ARGS );
  t_CKDUR duration = GET_NEXT_DUR( ARGS );

  l_obj->set(duration, target);
  RETURN->v_dur = l_obj->keyOn();
}

// single-ramp with user-defined start and user-defined target
CK_DLL_MFUN( line_keyOnSingleTargetStart )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKFLOAT initial = GET_NEXT_FLOAT( ARGS );
  t_CKFLOAT target = GET_NEXT_FLOAT( ARGS );
  t_CKDUR duration = GET_NEXT_DUR( ARGS );

  l_obj->set(duration, target, initial);
  RETURN->v_dur = l_obj->keyOn();
}

// multi-ramp with 0 start
CK_DLL_MFUN( line_keyOnArray )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  // get arguments
  Chuck_ArrayFloat* targets = (Chuck_ArrayFloat *)GET_NEXT_OBJECT( ARGS );
  Chuck_ArrayFloat* durations = (Chuck_ArrayFloat *)GET_NEXT_OBJECT( ARGS );

  // bounds checking0
  t_CKINT durations_size, targets_size;
  durations_size = API->object->array_float_size(durations);
  targets_size = API->object->array_float_size(targets);

  if (durations_size != targets_size) {
    API->vm->throw_exception("LineMismatchedArrays", "Duration and target arrays must be same size in a Line.keyOn(...)", nullptr);
    return;
  }

  // copy over array contents
  std::vector<t_CKDUR> durations_vec;
  std::vector<t_CKFLOAT> targets_vec;

  for (int i = 0; i < durations_size; i++) {
      durations_vec.push_back( API->object->array_float_get_idx( durations, i ) );
      targets_vec.push_back( API->object->array_float_get_idx( targets, i ) );
  }

  l_obj->set(durations_vec, targets_vec);
  RETURN->v_dur = l_obj->keyOn();
}
// multi-ramp with user-defined start
CK_DLL_MFUN( line_keyOnArrayStart )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  // get arguments
  t_CKFLOAT initial = GET_NEXT_FLOAT( ARGS );
  Chuck_ArrayFloat* targets = (Chuck_ArrayFloat *)GET_NEXT_OBJECT( ARGS );
  Chuck_ArrayFloat* durations = (Chuck_ArrayFloat *)GET_NEXT_OBJECT( ARGS );

  // bounds checking
  t_CKINT durations_size, targets_size;
  durations_size = API->object->array_float_size(durations);
  targets_size = API->object->array_float_size(targets);

  if (durations_size != targets_size) {
    API->vm->throw_exception("LineMismatchedArrays", "Duration and target arrays must be same size in a Line.keyOn(...)", nullptr);
    return;
  }

  // copy over array contents
  std::vector<t_CKDUR> durations_vec;
  std::vector<t_CKFLOAT> targets_vec;

  for (int i = 0; i < durations_size; i++) {
      durations_vec.push_back( API->object->array_float_get_idx( durations, i ) );
      targets_vec.push_back( API->object->array_float_get_idx( targets, i ) );
  }

  l_obj->set(durations_vec, targets_vec, initial);
  RETURN->v_dur = l_obj->keyOn();
}


CK_DLL_MFUN( line_keyOff )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );
  RETURN->v_dur = l_obj->keyOff();
}

CK_DLL_MFUN( line_keyOffDur )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKDUR duration = GET_NEXT_DUR( ARGS );

  RETURN->v_dur = l_obj->keyOff(duration);
}

CK_DLL_MFUN( line_keyOffTarget )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKFLOAT target = GET_NEXT_FLOAT( ARGS );

  RETURN->v_dur = l_obj->keyOffTarget(target);
}

CK_DLL_MFUN( line_keyOffDurTarget )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKFLOAT target = GET_NEXT_FLOAT( ARGS );
  t_CKDUR duration = GET_NEXT_DUR( ARGS );

  RETURN->v_dur = l_obj->keyOff(target, duration);
}

CK_DLL_MFUN( line_last ) {
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  RETURN->v_float = l_obj->last();
}

CK_DLL_MFUN( line_getInitial ) {
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  RETURN->v_float = l_obj->initial();
}

CK_DLL_MFUN( line_getTargets ) {

  Chuck_DL_Api::Object target2 = API->object->create(SHRED, API->type->lookup(VM, "float[][]"), false);
  Chuck_ArrayInt* target_arr2 = (Chuck_ArrayInt*)target2;

  for (int i = 0; i < 3; i++) {
      Chuck_DL_Api::Object target_tmp = API->object->create(SHRED, API->type->lookup(VM, "float[]"), false);
      Chuck_ArrayFloat* target_arr_tmp = (Chuck_ArrayFloat*)target_tmp;

      for (int j = 0; j < 3; j++) {
          API->object->array_float_push_back(target_arr_tmp, j);
      }

      API->object->array_int_push_back(target_arr2, (t_CKINT)target_arr_tmp);
  }

  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  // Create a float[] array
  Chuck_DL_Api::Object target = API->object->create(SHRED, API->type->lookup(VM, "float[]"), false);
  Chuck_ArrayFloat * target_arr = (Chuck_ArrayFloat *) target;

  for (auto x : l_obj->targets()) {
      API->object->array_float_push_back(target_arr, x);
  }

  // Need to cast back to object due to lost inheirtience structure
  RETURN->v_object = (Chuck_Object*) target_arr;
}

CK_DLL_MFUN(line_getDurations) {
    Line* l_obj = (Line*)OBJ_MEMBER_INT(SELF, line_data_offset);

    // Create a dur[] array
    Chuck_DL_Api::Object dur = API->object->create(SHRED, API->type->lookup(VM, "dur[]"), false);
    Chuck_ArrayFloat* dur_arr = (Chuck_ArrayFloat*)dur;

    for (auto x : l_obj->durations()) {
        API->object->array_float_push_back(dur_arr, x);
    }

    RETURN->v_object = (Chuck_Object*) dur_arr;

}
