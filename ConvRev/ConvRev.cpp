//-----------------------------------------------------------------------------
// ConvRev: Convolution Reverb Chugin
// Made for M220A Final Project in Summer 2021
//
// author: Andrew Zhu Aday (https://ccrma.stanford.edu/~azaday/)
// date: Summer 2021
//-----------------------------------------------------------------------------

// include chuck dynamic linking header
#include "chugin.h"

// vendor includes
#include "FFTConvolver.h"

// local includes
// #define CONV_REV_PROFILE // TODO: remove when building

#ifdef CONV_REV_PROFILE
#include "Timer.h"
#endif

// general includes
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#define CONV_REV_BLOCKSIZE 128  // default FFT blocksize

CK_DLL_CTOR(convrev_ctor);
CK_DLL_DTOR(convrev_dtor);

// Set FFT blocksize
CK_DLL_MFUN(convrev_setBlockSize);
CK_DLL_MFUN(convrev_getBlockSize);

// Set order of IR buffer
CK_DLL_MFUN(convrev_setOrder);
CK_DLL_MFUN(convrev_getOrder);

// populate individual IR sample values
CK_DLL_MFUN(convrev_setCoeff);
CK_DLL_MFUN(convrev_getCoeff);

// initialize convolution engine
CK_DLL_MFUN(convrev_init);  

// load entire buffer at once  (see fluidsynth for how to take in array arg)
// CK_DLL_MFUN(convrev_setIRBuffer);
// Problems: no way to return an array, no way to get an array from sndbuf ugen

// tick
CK_DLL_TICK(convrev_tick);

// this is a special offset reserved for chugin internal data
t_CKINT convrev_data_offset = 0;

class ConvRev
{
private:  // internal data
    t_CKFLOAT _SR;       // sample rate
    t_CKINT _blocksize;  // FFT blocksize
    t_CKINT _order;      // filter order

    // internal buffers
    std::vector<fftconvolver::Sample> _ir_buffer;

    // input double buffer
    std::vector<fftconvolver::Sample> _input_buffer;
    std::vector<fftconvolver::Sample> _staging_in_buffer;
    bool _which_input_buffer;

    // output double buffer
    fftconvolver::Sample* _output_buffer;
    fftconvolver::Sample* _staging_out_buffer;
    bool _which_output_buffer;

    fftconvolver::FFTConvolver _convolver;  // convolution engine

    size_t _idx;  // to track head of circular input buffer

    // threading
    std::thread _conv_thr;

    // scale factor to normalize output
    float _scale_factor;

public:
    ConvRev( t_CKFLOAT fs ) 
    : _SR(fs), _blocksize(CONV_REV_BLOCKSIZE), _order(0), _convolver(),
    _idx(0), _scale_factor(1.0f), _which_input_buffer(false), _which_output_buffer(false),
    _output_buffer(nullptr), _staging_out_buffer(nullptr)
    {}

    ~ConvRev() { 
        if (_conv_thr.joinable()) _conv_thr.join(); 
        if (_output_buffer) delete[] _output_buffer;
        if (_staging_out_buffer) delete[] _staging_out_buffer;
    }

    // double buffer getters
    std::vector<fftconvolver::Sample>& getInputBuffer() { 
        return _which_input_buffer ? _input_buffer : _staging_in_buffer;
    }
    std::vector<fftconvolver::Sample>& getStagingInputBuffer() { 
        return _which_input_buffer ? _staging_in_buffer : _input_buffer;
    }

    fftconvolver::Sample* getOutputBuffer() {
        return _which_output_buffer ? _output_buffer : _staging_out_buffer;
    }

    fftconvolver::Sample* getStagingOutputBuffer() {
        return _which_output_buffer ? _staging_out_buffer : _output_buffer;
    }

    // buffer swappers
    void swapBuffers() {
        _which_input_buffer = !_which_input_buffer;
        _which_output_buffer = !_which_output_buffer;
    }

    // for Chugins extending UGen
    SAMPLE tick( SAMPLE in )
    {
#ifdef CONV_REV_PROFILE
        Timer timer("tick", _blocksize);
#endif
        getInputBuffer()[_idx] = in;
        SAMPLE output  = _scale_factor * (getOutputBuffer()[_idx]);

        // increment circular buffer head
        _idx++;

        if (_idx == _blocksize) {
#ifdef CONV_REV_PROFILE
        Timer timer("----tick at blocksize");
#endif
            _idx = 0; // reset circular buffer head

            // wait for convolution engine to finish processing next block
            if (_conv_thr.joinable()) _conv_thr.join();

            // swap input and output buffers
            swapBuffers();

            // start processing the new input block
            // on VR lab laptop drops average Tick time from 1.5ms --> 1.3ms
            // shockingly low... but the more complex the ugen graph, the more
            // we should see a benefit from puting the convolution engine on a
            // separate thread
            _conv_thr = std::thread([&]() {
                _process(getStagingInputBuffer(), getStagingOutputBuffer());
            });
        }

        return output;
    }

    void _process(std::vector<fftconvolver::Sample>& input_buffer, fftconvolver::Sample* output_buffer) {
        // TODO process does not work with std::vector for output buffer. why??
#ifdef CONV_REV_PROFILE
        Timer timer("--------convolver.process()");
#endif
		_convolver.process(input_buffer.data(), output_buffer, _blocksize);
    }

    // set parameter example
    t_CKFLOAT setBlockSize( t_CKFLOAT p )
    {
        _blocksize = p;
        return p;
    }

    // get parameter example
    t_CKFLOAT getBlockSize() { return _blocksize; }

    void setOrder( t_CKINT m )
    {
        _order = m;
        _ir_buffer.resize(_order, 0);
    }

    t_CKINT getOrder() {
        return _order;
    }

    void setCoeff(t_CKINT idx, t_CKFLOAT val) {
        _ir_buffer[idx] = val;
    }

    t_CKFLOAT getCoeff(t_CKINT idx) { return _ir_buffer[idx]; }

    t_CKVOID init() {
        // resize buffers and zero buffers
        _input_buffer.resize(_blocksize, 0);
        _staging_in_buffer.resize(_blocksize, 0);

        // free old buffer
        if (_output_buffer) delete[] _output_buffer;
        if (_staging_out_buffer) delete[] _staging_out_buffer;
        // allocate new buffer
        _output_buffer = new fftconvolver::Sample[_blocksize];
        _staging_out_buffer = new fftconvolver::Sample[_blocksize];
        // zero out new buffer
        memset( _output_buffer, 0, _blocksize * sizeof(fftconvolver::Sample));
        memset(_staging_out_buffer, 0, _blocksize * sizeof(fftconvolver::Sample));

        // initialize convolution engine
        _convolver.init(_blocksize, _ir_buffer.data(), _order);

        // set normalization scale factor
        _scale_factor = _SR / _order;
        if (_scale_factor > 1) { _scale_factor = 1; }
    }
};


CK_DLL_QUERY( ConvRev )
{
    QUERY->setname( QUERY, "ConvRev" );
    
    // begin the class definition
    QUERY->begin_class( QUERY, "ConvRev", "UGen" );
    QUERY->doc_class(QUERY, "Convolution Reverb Chugin");
    QUERY->add_ex(QUERY, "effects/ConvRev.ck");

    QUERY->add_ctor( QUERY, convrev_ctor );
    QUERY->add_dtor( QUERY, convrev_dtor );

    // for UGens only: add tick function
    QUERY->add_ugen_func( QUERY, convrev_tick, NULL, 1, 1 );
    // NOTE: if this is to be a UGen with more than 1 channel,
    // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
    // and declare a tickf function using CK_DLL_TICKF

    QUERY->add_mfun(QUERY, convrev_setBlockSize, "float", "blocksize");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, 
        "Set the blocksize of the FFT convolution engine. "
        "Larger blocksize means more efficient processing, but more latency. "
        "Latency is equal to blocksize / sample rate."
        "Defaults to 128 samples."
    );

    QUERY->add_mfun(QUERY, convrev_getBlockSize, "float", "blocksize");
    QUERY->doc_func(QUERY, "Get the blocksize of the FFT convolution engine.");

    QUERY->add_mfun(QUERY, convrev_setOrder, "int", "order");
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->doc_func(QUERY, 
        "Set the order of the convolution filter. "
        "This should be set to the length of the impulse response buffer in samples"
    );

    QUERY->add_mfun(QUERY, convrev_getOrder, "int", "order");
    QUERY->doc_func(QUERY, "Get the order of the convolution filter.");

    QUERY->add_mfun(QUERY, convrev_setCoeff, "float", "coeff");
    QUERY->add_arg(QUERY, "int", "index");
    QUERY->add_arg(QUERY, "float", "coefficient");
    QUERY->doc_func(QUERY, 
        "Set the coefficient of the convolution filter at position <index>. "
    );

    QUERY->add_mfun(QUERY, convrev_getCoeff, "float", "coeff");
    QUERY->add_arg(QUERY, "int", "index");
    QUERY->doc_func(QUERY, 
        "Get the coefficient of the convolution filter at position <index>. "
    );

    QUERY->add_mfun(QUERY, convrev_init, "void", "init");
    QUERY->doc_func(QUERY, 
        "Initialize the convolution engine. Performs memory allocations, pre-computes the IR FFT etc."
        "This should be called after setting the order and coefficients of the filter, and before using the UGen."
    );

    // this reserves a variable in the ChucK internal class to store
    // reference to the c++ class we defined above
    convrev_data_offset = QUERY->add_mvar(QUERY, "int", "@cr_data", false);

    // end the class definition
    QUERY->end_class(QUERY);

    return TRUE;
}

CK_DLL_CTOR(convrev_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, convrev_data_offset) = 0;

    // instantiate our internal c++ class representation
    ConvRev * cr_obj = new ConvRev(API->vm->srate(VM));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, convrev_data_offset) = (t_CKINT) cr_obj;
}

CK_DLL_DTOR(convrev_dtor)
{
    // get our c++ class pointer
    ConvRev * cr_obj = (ConvRev *) OBJ_MEMBER_INT(SELF, convrev_data_offset);
    // free it
    CK_SAFE_DELETE(cr_obj);
    // zero out memory
    OBJ_MEMBER_INT(SELF, convrev_data_offset) = 0;
}

CK_DLL_TICK(convrev_tick)
{
    // get our c++ class pointer
    ConvRev * cr_obj = (ConvRev *) OBJ_MEMBER_INT(SELF, convrev_data_offset);

    // invoke our tick function; store in the magical out variable
    if(cr_obj) *out = cr_obj->tick(in);

    return TRUE;
}

CK_DLL_MFUN(convrev_setBlockSize)
{
    ConvRev * cr_obj = (ConvRev *) OBJ_MEMBER_INT(SELF, convrev_data_offset);
    RETURN->v_float = cr_obj->setBlockSize(GET_NEXT_FLOAT(ARGS));
}


CK_DLL_MFUN(convrev_getBlockSize)
{
    ConvRev * cr_obj = (ConvRev *) OBJ_MEMBER_INT(SELF, convrev_data_offset);
    RETURN->v_float = cr_obj->getBlockSize();
}

CK_DLL_MFUN(convrev_setOrder)
{
    ConvRev * cr_obj = (ConvRev *) OBJ_MEMBER_INT(SELF, convrev_data_offset);
    t_CKINT order = GET_NEXT_INT(ARGS);

    if (order < 0) {
        API->vm->throw_exception(
            "InvalidArgument",
            (std::string("Trying to set convolution filter order to a negative value!\n") 
            + "order = " + std::to_string(order) + ".").c_str(),
            SHRED 
        );
    } else {
        cr_obj->setOrder(order);
    }

    RETURN->v_int = order;
}

CK_DLL_MFUN(convrev_getOrder)
{
    ConvRev * cr_obj = (ConvRev *) OBJ_MEMBER_INT(SELF, convrev_data_offset);
    RETURN->v_int = cr_obj->getOrder();
}

CK_DLL_MFUN(convrev_setCoeff)
{
    ConvRev * cr_obj = (ConvRev *) OBJ_MEMBER_INT(SELF, convrev_data_offset);
    t_CKINT idx = GET_NEXT_INT(ARGS);
    t_CKFLOAT val = GET_NEXT_FLOAT(ARGS);
    auto order = cr_obj->getOrder();

    if (idx >= order || idx < 0) {
        API->vm->throw_exception(
            "IndexOutOfBounds",
            (std::string("Illegal index out of bounds in setting convolver filter coefficient!\n") 
            + "idx = " + std::to_string(idx) + " on size " + std::to_string(order) + ".").c_str(),
            SHRED 
        );
    } else {
        cr_obj->setCoeff(idx, val);
    }

    RETURN->v_int = val;
}

CK_DLL_MFUN(convrev_getCoeff)
{
    ConvRev * cr_obj = (ConvRev *) OBJ_MEMBER_INT(SELF, convrev_data_offset);
    RETURN->v_int = cr_obj->getCoeff(GET_CK_INT(ARGS));
}

CK_DLL_MFUN(convrev_init)
{
  ConvRev * cr_obj = (ConvRev *) OBJ_MEMBER_INT(SELF, convrev_data_offset);
  cr_obj->init();
}
