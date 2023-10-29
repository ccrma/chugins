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
#include "chuck_dl.h"

// general includes
#include <iostream>


// declaration of chugin constructor
CK_DLL_CTOR( line_ctor );
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

// keyOn function
CK_DLL_MFUN( line_keyOn );

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
  Line( t_CKFLOAT fs )
  {
    m_fs = fs;
    m_elapsed = 0;
    m_state = -1;

    // setup defaults
    m_initial = 0;
    m_value = 0;

    std::vector<t_CKFLOAT> targets{1};
    m_targets = targets;

    std::vector<t_CKDUR> durs{1000};
    m_durs = durs;

    m_keyOff_dur = 1000;

    setRates();
    setCumulative();
  }

  // for chugins extending UGen
  SAMPLE tick( SAMPLE in ) {
    // envelope isn't on, do nothing.
    if (m_state == -1) return m_initial;

    t_CKFLOAT target = m_targets[m_state];
    t_CKFLOAT rate = m_rates[m_state];

    // update the current value and then clip it to the target's value
    // if needed
    m_value += rate;

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

  void set(t_CKDUR d, t_CKFLOAT target) {
    m_initial = 0;
    m_value = 0;
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

  void keyOn() {
    m_elapsed = 0;
    m_state = 0;
  }

  t_CKFLOAT last() {
    return m_value;
  }

private:
  // the envelope params
  t_CKFLOAT m_initial;
  t_CKFLOAT m_value;
  t_CKFLOAT m_fs;

  std::vector<t_CKFLOAT> m_targets;
  std::vector<t_CKDUR> m_durs, m_durs_cumulative;
  std::vector<t_CKFLOAT> m_rates;

  t_CKDUR m_keyOff_dur;

  // the current state of the envelope
  // -1: not on
  // 0,1,...: index of the current envelope
  int m_state;
  t_CKDUR m_elapsed;

  // TODO: I don't need this, I just need an envelope of size 1...
  bool m_singular;

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
};


// query function: chuck calls this when loading the chugin
// (NOTE developer will need to modify this function to add additional functions to this chugin)
CK_DLL_QUERY( Line )
{
  // hmm, don't change this...
  QUERY->setname( QUERY, "Line" );

  // begin the class definition
  // NOTE to create a non-UGen class, change the second argument
  // to extend a different ChucK class (e.g., "Object")
  QUERY->begin_class( QUERY, "Line", "UGen" );

  // register the constructor (probably no need to change)
  QUERY->add_ctor( QUERY, line_ctor );
  // register the destructor (probably no need to change)
  QUERY->add_dtor( QUERY, line_dtor );

  // for UGens only: add tick function
  // NOTE a non-UGen class should remove or comment out this next line
  QUERY->add_ugen_func( QUERY, line_tick, NULL, 1, 1 );
  // NOTE: if this is to be a UGen with more than 1 channel,
  // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
  // and declare a tickf function using CK_DLL_TICKF

  // singular set with all defaults
  QUERY->add_mfun( QUERY, line_setSingle, "void", "set" );
  QUERY->add_arg( QUERY, "dur", "duration" );

  // singular set with default start
  QUERY->add_mfun( QUERY, line_setSingleTarget, "void", "set" );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->add_arg( QUERY, "dur", "duration" );

  // singular set with no defaults
  QUERY->add_mfun( QUERY, line_setSingleTargetStart, "void", "set" );
  QUERY->add_arg( QUERY, "float", "initial" );
  QUERY->add_arg( QUERY, "float", "target" );
  QUERY->add_arg( QUERY, "dur", "duration" );

  // array set with default start
  QUERY->add_mfun( QUERY, line_setArray, "void", "set" );
  QUERY->add_arg( QUERY, "float[]", "targets" );
  QUERY->add_arg( QUERY, "dur[]", "durations" );

  // array set with no defaults
  QUERY->add_mfun( QUERY, line_setArrayStart, "void", "set" );
  QUERY->add_arg( QUERY, "float", "initial" );
  QUERY->add_arg( QUERY, "float[]", "targets" );
  QUERY->add_arg( QUERY, "dur[]", "durations" );

  // keyOn function
  QUERY->add_mfun( QUERY, line_keyOn, "Event", "keyOn" );

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
  Line * l_obj = new Line( API->vm->srate(VM) );

  // store the pointer in the ChucK object member
  OBJ_MEMBER_INT( SELF, line_data_offset ) = (t_CKINT)l_obj;
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
}


// single-ramp with 0 start and user-defined target
CK_DLL_MFUN( line_setSingleTarget )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  t_CKFLOAT target = GET_NEXT_FLOAT( ARGS );
  t_CKDUR duration = GET_NEXT_DUR( ARGS );

  l_obj->set(duration, target);
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
  API->object->array_float_size(durations, durations_size);
  API->object->array_float_size(targets, targets_size);

  if (durations_size != targets_size) {
    API->vm->em_log(3, "Duration and target arrays must be same size");
    return;
  }

  // copy over array contents
  std::vector<t_CKDUR> durations_vec;
  std::vector<t_CKFLOAT> targets_vec;

  for (int i = 0; i < durations_size; i++) {
    t_CKDUR dur;
    t_CKFLOAT target;

    API->object->array_float_get_idx(durations, i, dur);
    API->object->array_float_get_idx(targets, i, target);

    durations_vec.push_back(dur);
    targets_vec.push_back(target);
  }

  l_obj->set(durations_vec, targets_vec);
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
  API->object->array_float_size(durations, durations_size);
  API->object->array_float_size(targets, targets_size);

  if (durations_size != targets_size) {
    API->vm->em_log(3, "Duration and target arrays must be same size");
    return;
  }

  // copy over array contents
  std::vector<t_CKDUR> durations_vec;
  std::vector<t_CKFLOAT> targets_vec;

  for (int i = 0; i < durations_size; i++) {
    t_CKDUR dur;
    t_CKFLOAT target;

    API->object->array_float_get_idx(durations, i, dur);
    API->object->array_float_get_idx(targets, i, target);

    durations_vec.push_back(dur);
    targets_vec.push_back(target);
  }

  l_obj->set(durations_vec, targets_vec, initial);
}

// https://github.com/ccrma/chuck/blob/b47a2c1e4cc2d234d76b6b7733de839df75477ed/src/core/ugen_stk.cpp#L3698
CK_DLL_MFUN( line_keyOn )
{
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  l_obj->keyOn();
}

CK_DLL_MFUN( line_last ) {
  // get our c++ class pointer
  Line * l_obj = (Line *)OBJ_MEMBER_INT( SELF, line_data_offset );

  RETURN->v_float = l_obj->last();
}
