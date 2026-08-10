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

// general includes
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// local includes
// #define CONV_REV_PROFILE // TODO: remove when building

#ifdef CONV_REV_PROFILE
#include "Timer.h"
#endif

#ifdef __EMSCRIPTEN__
#define CONV_REV_BLOCKSIZE 512 // default FFT blocksize
#else
#define CONV_REV_BLOCKSIZE 128 // default FFT blocksize
#endif

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

// tick
CK_DLL_TICK(convrev_tick);

// this is a special offset reserved for chugin internal data
t_CKINT convrev_data_offset = 0;

class ConvRev
{
private:                // internal data
    t_CKFLOAT _SR;      // sample rate
    t_CKINT _blocksize; // FFT blocksize
    t_CKINT _order;     // filter order

    // internal buffers
    std::vector<fftconvolver::Sample> _ir_buffer;

    // input/output buffers (single buffer logic)
    std::vector<fftconvolver::Sample> _input_buffer;
    std::vector<fftconvolver::Sample> _output_buffer;

    fftconvolver::FFTConvolver _convolver; // convolution engine

    size_t _idx; // to track head of circular input buffer

public:
    ConvRev(t_CKFLOAT fs)
        : _SR(fs), _blocksize(CONV_REV_BLOCKSIZE), _order(0), _convolver(),
          _idx(0)
    {
        _input_buffer.resize(_blocksize, 0);
        _output_buffer.resize(_blocksize, 0);
    }

    ~ConvRev()
    {
    }

    // for Chugins extending UGen
    SAMPLE tick(SAMPLE in)
    {
        _input_buffer[_idx] = in;
        
        // Output computed value from previous block
        // (This latency is inherent to block-based FFT convolution)
        SAMPLE output = _output_buffer[_idx];

        // increment circular buffer head
        _idx++;

        if (_idx == _blocksize)
        {
            _idx = 0; // reset circular buffer head

            // Process the block synchronously
            // This is "real-time safe" in the sense that it doesn't allocate or lock,
            // but it does spike CPU usage every _blocksize samples.
            // This is standard behavior for simple partition convolvers.
            _convolver.process(_input_buffer.data(), _output_buffer.data(), _blocksize);
        }

        return output;
    }

    // set parameter example
    t_CKFLOAT setBlockSize(t_CKFLOAT p)
    {
        // Ensure blocksize is a power of 2 and somewhat reasonable
        int size = (int)p;
        if (size < 32) size = 32;
        int p2 = 1;
        while (p2 < size) p2 <<= 1;
        
        _blocksize = p2;
        
        // Resize buffers
        _input_buffer.resize(_blocksize, 0);
        _output_buffer.resize(_blocksize, 0);
        
        // Reset state
        std::fill(_input_buffer.begin(), _input_buffer.end(), 0.0f);
        std::fill(_output_buffer.begin(), _output_buffer.end(), 0.0f);
        _idx = 0;
        
        return _blocksize;
    }

    // get parameter example
    t_CKFLOAT getBlockSize() { return _blocksize; }

    void setOrder(t_CKINT m)
    {
        _order = m;
        _ir_buffer.resize(_order, 0);
    }

    t_CKINT getOrder()
    {
        return _order;
    }

    void setCoeff(t_CKINT idx, t_CKFLOAT val)
    {
        if (idx >= 0 && idx < _ir_buffer.size()) {
            _ir_buffer[idx] = val;
        }
    }

    t_CKFLOAT getCoeff(t_CKINT idx) { 
        if (idx >= 0 && idx < _ir_buffer.size()) {
            return _ir_buffer[idx]; 
        }
        return 0;
    }

    t_CKVOID init()
    {
        // Safety: Check for NaN/Inf in IR
        bool hasPoly = false;
        for (float f : _ir_buffer) {
            if (std::isnan(f) || std::isinf(f)) {
                 hasPoly = true;
                 break;
            }
        }
        
        if (hasPoly) {
            // zero out if bad data 
            std::fill(_ir_buffer.begin(), _ir_buffer.end(), 0.0f);
            std::cout << "[ConvRev]: Invalid IR data (NaN/Inf) detected. Zeroing buffer." << std::endl;
        }

        // Safety: Peak Normalize to 0.5 (Conservative default)
        // Convolution adds a lot of energy.
        float max_val = 0.0f;
        for (float f : _ir_buffer) {
            if (std::abs(f) > max_val) max_val = std::abs(f);
        }

        if (max_val > 0.000001f) {
            // Scale to 0.5 peak
            float scale = 0.5f / max_val; 
            for (size_t i=0; i<_ir_buffer.size(); ++i) _ir_buffer[i] *= scale;
        }

        // zero out buffers
        std::fill(_input_buffer.begin(), _input_buffer.end(), 0.0f);
        std::fill(_output_buffer.begin(), _output_buffer.end(), 0.0f);
        _idx = 0;

        // initialize convolution engine
        _convolver.init(_blocksize, _ir_buffer.data(), _order);
    }
};

CK_DLL_QUERY(ConvRev)
{
    QUERY->setname(QUERY, "ConvRev");

    // begin the class definition
    QUERY->begin_class(QUERY, "ConvRev", "UGen");
    QUERY->doc_class(QUERY, "Convolution Reverb Chugin");
    QUERY->add_ex(QUERY, "effects/ConvRev.ck");

    QUERY->add_ctor(QUERY, convrev_ctor);
    QUERY->add_dtor(QUERY, convrev_dtor);

    // for UGens only: add tick function
    QUERY->add_ugen_func(QUERY, convrev_tick, NULL, 1, 1);
    // NOTE: if this is to be a UGen with more than 1 channel,
    // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
    // and declare a tickf function using CK_DLL_TICKF

    QUERY->add_mfun(QUERY, convrev_setBlockSize, "float", "blocksize");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY,
                    "Set the blocksize of the FFT convolution engine. "
                    "Larger blocksize means more efficient processing, but more latency. "
                    "Latency is equal to blocksize / sample rate."
                    "Defaults to 128 samples.");

    QUERY->add_mfun(QUERY, convrev_getBlockSize, "float", "blocksize");
    QUERY->doc_func(QUERY, "Get the blocksize of the FFT convolution engine.");

    QUERY->add_mfun(QUERY, convrev_setOrder, "int", "order");
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->doc_func(QUERY,
                    "Set the order of the convolution filter. "
                    "This should be set to the length of the impulse response buffer in samples");

    QUERY->add_mfun(QUERY, convrev_getOrder, "int", "order");
    QUERY->doc_func(QUERY, "Get the order of the convolution filter.");

    QUERY->add_mfun(QUERY, convrev_setCoeff, "float", "coeff");
    QUERY->add_arg(QUERY, "int", "index");
    QUERY->add_arg(QUERY, "float", "coefficient");
    QUERY->doc_func(QUERY,
                    "Set the coefficient of the convolution filter at position <index>. ");

    QUERY->add_mfun(QUERY, convrev_getCoeff, "float", "coeff");
    QUERY->add_arg(QUERY, "int", "index");
    QUERY->doc_func(QUERY,
                    "Get the coefficient of the convolution filter at position <index>. ");

    QUERY->add_mfun(QUERY, convrev_init, "void", "init");
    QUERY->doc_func(QUERY,
                    "Initialize the convolution engine. Performs memory allocations, pre-computes the IR FFT etc."
                    "This should be called after setting the order and coefficients of the filter, and before using the UGen.");

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
    ConvRev *cr_obj = new ConvRev(API->vm->srate(VM));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, convrev_data_offset) = (t_CKINT)cr_obj;
}

CK_DLL_DTOR(convrev_dtor)
{
    // get our c++ class pointer
    ConvRev *cr_obj = (ConvRev *)OBJ_MEMBER_INT(SELF, convrev_data_offset);
    // free it
    CK_SAFE_DELETE(cr_obj);
    // zero out memory
    OBJ_MEMBER_INT(SELF, convrev_data_offset) = 0;
}

CK_DLL_TICK(convrev_tick)
{
    // get our c++ class pointer
    ConvRev *cr_obj = (ConvRev *)OBJ_MEMBER_INT(SELF, convrev_data_offset);

    // invoke our tick function; store in the magical out variable
    if (cr_obj)
        *out = cr_obj->tick(in);

    return TRUE;
}

CK_DLL_MFUN(convrev_setBlockSize)
{
    ConvRev *cr_obj = (ConvRev *)OBJ_MEMBER_INT(SELF, convrev_data_offset);
    RETURN->v_float = cr_obj->setBlockSize(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(convrev_getBlockSize)
{
    ConvRev *cr_obj = (ConvRev *)OBJ_MEMBER_INT(SELF, convrev_data_offset);
    RETURN->v_float = cr_obj->getBlockSize();
}

CK_DLL_MFUN(convrev_setOrder)
{
    ConvRev *cr_obj = (ConvRev *)OBJ_MEMBER_INT(SELF, convrev_data_offset);
    t_CKINT order = GET_NEXT_INT(ARGS);

    if (order < 0)
    {
        API->vm->throw_exception(
            "InvalidArgument",
            (std::string("Trying to set convolution filter order to a negative value!\n") + "order = " + std::to_string(order) + ".").c_str(),
            SHRED);
    }
    else
    {
        cr_obj->setOrder(order);
    }

    RETURN->v_int = order;
}

CK_DLL_MFUN(convrev_getOrder)
{
    ConvRev *cr_obj = (ConvRev *)OBJ_MEMBER_INT(SELF, convrev_data_offset);
    RETURN->v_int = cr_obj->getOrder();
}

CK_DLL_MFUN(convrev_setCoeff)
{
    ConvRev *cr_obj = (ConvRev *)OBJ_MEMBER_INT(SELF, convrev_data_offset);
    t_CKINT idx = GET_NEXT_INT(ARGS);
    t_CKFLOAT val = GET_NEXT_FLOAT(ARGS);
    auto order = cr_obj->getOrder();

    if (idx >= order || idx < 0)
    {
        API->vm->throw_exception(
            "IndexOutOfBounds",
            (std::string("Illegal index out of bounds in setting convolver filter coefficient!\n") + "idx = " + std::to_string(idx) + " on size " + std::to_string(order) + ".").c_str(),
            SHRED);
    }
    else
    {
        cr_obj->setCoeff(idx, val);
    }

    RETURN->v_int = val;
}

CK_DLL_MFUN(convrev_getCoeff)
{
    ConvRev *cr_obj = (ConvRev *)OBJ_MEMBER_INT(SELF, convrev_data_offset);
    RETURN->v_int = cr_obj->getCoeff(GET_CK_INT(ARGS));
}

CK_DLL_MFUN(convrev_init)
{
    ConvRev *cr_obj = (ConvRev *)OBJ_MEMBER_INT(SELF, convrev_data_offset);
    cr_obj->init();
}
