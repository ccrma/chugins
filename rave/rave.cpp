//-----------------------------------------------------------------------------
// Entaro ChucK Developer!
// This is a Chugin boilerplate, generated by chuginate!
//-----------------------------------------------------------------------------

/*
TODOS
- clean up models and stuff
- set buffer size method (0 will be threadless fast mode)
*/
#pragma once

// nn_tilde includes
#include "backend.h"
#include "nn_tilde/src/frontend/maxmsp/shared/circular_buffer.h"

// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"
#include "chuck_ugen.h"

// general includes
#include <stdio.h>
#include <limits.h>


// declaration of chugin constructor
CK_DLL_CTOR(rave_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(rave_dtor);

// load model
CK_DLL_MFUN(rave_load);
CK_DLL_MFUN(rave_getModel);

// set method
CK_DLL_MFUN(rave_setMethod);
CK_DLL_MFUN(rave_getMethod);

// get channels methods
CK_DLL_MFUN(rave_getChannels);
CK_DLL_MFUN(rave_getOutChannels);
CK_DLL_MFUN(rave_getInChannels);
CK_DLL_MFUN(rave_setEnable);
CK_DLL_MFUN(rave_getEnable);


// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICKF(rave_tickf);

// this is a special offset reserved for Chugin internal data
t_CKINT rave_data_offset = 0;

// maximum number of channels that rave can output
const int max_channels = 64;

unsigned power_ceil(unsigned x) {
    if (x <= 1)
        return 1;
    int power = 2;
    x--;
    while (x >>= 1)
        power <<= 1;
    return power;
}

// class definition for internal Chugin data
// (note: this isn't strictly necessary, but serves as example
// of one recommended approach)
class Rave
{
public:
    Backend m_model;
    std::string m_method;
    std::string m_model_path;

    // AUDIO PERFORM
    bool m_use_thread;
    std::unique_ptr<std::thread> m_compute_thread;

    // BUFFER RELATED MEMBERS
    int m_buffer_size;
    std::unique_ptr<circular_buffer<SAMPLE, float>[]> m_in_buffer;
    std::unique_ptr<circular_buffer<float, SAMPLE>[]> m_out_buffer;
    std::vector<std::unique_ptr<float[]>> m_in_model, m_out_model;

    // m_in_dim & m_out_dim correspond to number of in and out channels.
    int m_in_dim, m_in_ratio, m_out_dim, m_out_ratio, m_higher_ratio;

    bool m_enable;

    // constructor
    Rave( t_CKFLOAT fs, Chuck_UGen* self )
    {
        m_model = Backend();
        m_method = "forward";
        m_buffer_size = 2048;
        m_use_thread = true;
        m_compute_thread = nullptr;
        m_enable = true;
        m_self = self;
    }

    // destructor
    ~Rave() {
        // return channels sizes to initialized amount
        m_self->m_num_ins = max_channels;
        m_self->m_num_outs = max_channels;
        m_self->m_multi_chan_size = max_channels;

        if (m_compute_thread) m_compute_thread->join();
    }

    // for Chugins extending UGen
    void tick(SAMPLE* in, SAMPLE* out, t_CKUINT nframes)
    {
        auto dsp_vec_size = nframes;

        memset(out, 0, sizeof(SAMPLE) * max_channels * nframes);

        // Output zeros of model isn't running
        if (!m_model.is_loaded() || !m_enable) {
            return;
        }

        // TODO have variable buffer sizes to deal w/ latency
        // Check if DSP vector size is 
        if (dsp_vec_size > m_buffer_size) {
            std::cerr << "vector size (" << dsp_vec_size << ") ";
            std::cerr << "larger than buffer size (" << m_buffer_size << "). ";
            std::cerr << "disabling model.";
            std::cerr << std::endl;
            m_enable = false;
            // fill_with_zero(output);
            return;
        }

        perform(in, out, nframes);
    }

    void model_perform() {
        std::vector<SAMPLE*> in_model, out_model;

        for (int c(0); c < m_in_dim; c++)
            in_model.push_back(m_in_model[c].get());
        for (int c(0); c < m_out_dim; c++)
            out_model.push_back(m_out_model[c].get());

        m_model.perform(in_model, out_model, m_buffer_size, m_method, 1);
    }

    void perform(SAMPLE* in, SAMPLE* out, t_CKUINT nframes) {
        // auto vec_size = nframes;

        // COPY INPUT TO CIRCULAR BUFFER
        for (int c(0); c < m_in_dim; c++) {
            // std::cout << "copy input " << c << std::endl;

            m_in_buffer[c].put_interleave(in, max_channels, nframes);
            in++;
        }

        if (m_in_buffer[0].full()) { // BUFFER IS FULL
            // IF USE THREAD, CHECK THAT COMPUTATION IS OVER
            if (m_compute_thread && m_use_thread) {
                m_compute_thread->join();
            }

            // TRANSFER MEMORY BETWEEN INPUT CIRCULAR BUFFER AND MODEL BUFFER
            for (int c(0); c < m_in_dim; c++) {
                // std::cout << "m_in_buffer[" << c << "]" << std::endl;
                m_in_buffer[c].get(m_in_model[c].get(), m_buffer_size);
            }

            if (!m_use_thread) // PROCESS DATA RIGHT NOW
                model_perform();

            // TRANSFER MEMORY BETWEEN OUTPUT CIRCULAR BUFFER AND MODEL BUFFER
            for (int c(0); c < m_out_dim; c++)
                m_out_buffer[c].put(m_out_model[c].get(), m_buffer_size);

            if (m_use_thread) // PROCESS DATA LATER
                m_compute_thread = std::make_unique<std::thread>(&Rave::model_perform, this);
        }

        // std::cout << "copy to output" << std::endl;

        // COPY CIRCULAR BUFFER TO OUTPUT
        for (int c(0); c < m_out_dim; c++) {
            m_out_buffer[c].get_interleave(out+c, max_channels, nframes);
        }
    }

    // set and load model
    std::string load(const std::string& path) {
        // TRY TO LOAD MODEL
        if (m_model.load(std::string(path))) {
            std::cerr << "error during loading" << std::endl;
            m_model_path = "";
            return m_model_path;
        }

        std::cout << "loading succeeded" << std::endl;
        m_model_path = path;

        m_higher_ratio = m_model.get_higher_ratio();

        // this will set the method if it hasn't been already
        setMethod(m_method);
        return m_model_path;
    }

    t_CKBOOL setMethod(const std::string& method) {
        // GET MODEL'S METHOD PARAMETERS
        m_method = method;

        if (!m_model.is_loaded()) {
            return TRUE;
        }

        auto params = m_model.get_method_params(method);

        if (!params.size()) {
            std::cerr << "method " << m_method << " not found !" << std::endl;
            return FALSE;
        }

        m_in_dim = params[0];
        m_in_ratio = params[1];
        m_out_dim = params[2];
        m_out_ratio = params[3];

        // set buffer size depending on context
        if (!m_buffer_size) {
            // NO THREAD MODE
            m_use_thread = false;
            m_buffer_size = m_higher_ratio;
        }
        else if (m_buffer_size < m_higher_ratio) {
            m_buffer_size = m_higher_ratio;
            std::cerr << "buffer size too small, switching to " << m_buffer_size << std::endl;
        }
        else {
            m_buffer_size = power_ceil(m_buffer_size);
        }

        // Calling forward in a thread causes memory leak in windows.
        // See https://github.com/pytorch/pytorch/issues/24237
#ifdef _WIN32
        m_use_thread = false;
#endif

        // Clip the UGen's inputs to the actual num of dimensions
        // as an optimization   
        m_self->m_num_ins = m_in_dim;
        m_self->m_num_outs = m_out_dim;
        m_self->m_multi_chan_size = m_in_dim > m_out_dim ? m_in_dim: m_out_dim;

        // Create buffers
        m_in_buffer = std::make_unique<circular_buffer<SAMPLE, SAMPLE>[]>(m_in_dim);
        for (int i(0); i < m_in_dim; i++) {
            m_in_buffer[i].initialize(m_buffer_size);
            m_in_model.push_back(std::make_unique<SAMPLE[]>(m_buffer_size));
        }

        m_out_buffer = std::make_unique<circular_buffer<SAMPLE, SAMPLE>[]>(m_out_dim);
        for (int i(0); i < m_out_dim; i++) {
            m_out_buffer[i].initialize(m_buffer_size);
            m_out_model.push_back(std::make_unique<SAMPLE[]>(m_buffer_size));
        }


        return TRUE;
    }

    t_CKINT getOutChannels() { return m_out_dim; }
    t_CKINT getInChannels() { return m_in_dim; }

    
private:
    // instance data
    Chuck_UGen* m_self;
};

// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( rave )
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "Rave");
    
    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "Rave", "UGen_Multi");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, rave_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, rave_dtor);
    
    // for UGen's only: add tick function
    // QUERY->add_ugen_func(QUERY, rave_tick, NULL, 1, 1);
    QUERY->add_ugen_funcf(QUERY, rave_tickf, NULL, max_channels, max_channels);
    
    // NOTE: if this is to be a UGen with more than 1 channel, 
    // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
    // and declare a tickf function using CK_DLL_TICKF

    // register load method
    QUERY->add_mfun(QUERY, rave_load, "string", "model");
    QUERY->add_arg(QUERY, "string", "path");
    QUERY->doc_func(QUERY, "Load a model from a filepath.");

    QUERY->add_mfun(QUERY, rave_getModel, "string", "model");
    QUERY->doc_func(QUERY, "Get model filepath.");

    // register setMethod method
    QUERY->add_mfun(QUERY, rave_setMethod, "string", "method");
    QUERY->add_arg(QUERY, "string", "method");
    QUERY->doc_func(QUERY, "Set the model's method (default is 'forward'). Returning an empty string indicates the method wasn't found.");

    QUERY->add_mfun(QUERY, rave_getMethod, "string", "method");
    QUERY->doc_func(QUERY, "Get the current method.");

    // register channels method
    QUERY->add_mfun(QUERY, rave_getChannels, "int", "outChannels");
    QUERY->add_mfun(QUERY, rave_getInChannels, "int", "inChannels");

    // register enable methods
    QUERY->add_mfun(QUERY, rave_setEnable, "int", "enable");
    QUERY->add_arg(QUERY, "int", "enable");
    QUERY->doc_func(QUERY, "Enable sound rendering. 1 is enable, 0 is disable.");

    QUERY->add_mfun(QUERY, rave_getEnable, "int", "enable");
    QUERY->doc_func(QUERY, "Get sound rendering status. 1 is enable, 0 is disable.");

    // this reserves a variable in the ChucK internal class to store 
    // referene to the c++ class we defined above
    rave_data_offset = QUERY->add_mvar(QUERY, "int", "@r_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(rave_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, rave_data_offset) = 0;

    // cast SELF as a chuck_ugen so that we can get channel info
    Chuck_UGen* self = (Chuck_UGen*)SELF;
    
    // instantiate our internal c++ class representation
    Rave * r_obj = new Rave(API->vm->get_srate(API, SHRED), self);

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, rave_data_offset) = (t_CKINT) r_obj;
}


// implementation for the destructor
CK_DLL_DTOR(rave_dtor)
{
    // get our c++ class pointer
    Rave * r_obj = (Rave *) OBJ_MEMBER_INT(SELF, rave_data_offset);
    // check it
    if( r_obj )
    {
        // clean up
        delete r_obj;
        OBJ_MEMBER_INT(SELF, rave_data_offset) = 0;
        r_obj = NULL;
    }
}


// implementation for tickf function
CK_DLL_TICKF(rave_tickf)
{
    // get our c++ class pointer
    Rave * r_obj = (Rave *) OBJ_MEMBER_INT(SELF, rave_data_offset);
 
    // invoke our tick function; store in the magical out variable
    if(r_obj) r_obj->tick(in, out, nframes);

    // yes
    return TRUE;
}

CK_DLL_MFUN(rave_load)
{
    // get our c++ class pointer
    Rave* r_obj = (Rave*)OBJ_MEMBER_INT(SELF, rave_data_offset);
    // set the return value
    // RETURN->v_float = r_obj->setParam(GET_NEXT_FLOAT(ARGS));
    std::string model = r_obj->load(GET_NEXT_STRING(ARGS)->str());
    RETURN->v_string = (Chuck_String*)API->object->create_string(API, SHRED, model);
}

CK_DLL_MFUN(rave_getModel)
{
    Rave* r_obj = (Rave*)OBJ_MEMBER_INT(SELF, rave_data_offset);
    std::string model = r_obj->m_model_path;
    RETURN->v_string = (Chuck_String*)API->object->create_string(API, SHRED, model);
}

CK_DLL_MFUN(rave_setMethod)
{
    // get our c++ class pointer
    Rave* r_obj = (Rave*)OBJ_MEMBER_INT(SELF, rave_data_offset);
    // set the return value

    std::string method = "";

    // RETURN->v_float = r_obj->setParam(GET_NEXT_FLOAT(ARGS));
    if (r_obj->setMethod(GET_NEXT_STRING(ARGS)->str())) {
        method = r_obj->m_method;
    }
    RETURN->v_string = (Chuck_String*)API->object->create_string(API, SHRED, method);
}

CK_DLL_MFUN(rave_getMethod)
{
    // get our c++ class pointer
    Rave* r_obj = (Rave*)OBJ_MEMBER_INT(SELF, rave_data_offset);

    std::string method = r_obj->m_method;
    RETURN->v_string = (Chuck_String*)API->object->create_string(API, SHRED, method);
}

CK_DLL_MFUN(rave_getChannels)
{
    // get our c++ class pointer
    Rave* r_obj = (Rave*)OBJ_MEMBER_INT(SELF, rave_data_offset);
    // set the return value
    RETURN->v_int = r_obj->getOutChannels();
}

CK_DLL_MFUN(rave_getInChannels)
{
    // get our c++ class pointer
    Rave* r_obj = (Rave*)OBJ_MEMBER_INT(SELF, rave_data_offset);
    // set the return value
    RETURN->v_int = r_obj->getInChannels();
}

CK_DLL_MFUN(rave_setEnable)
{
    // get our c++ class pointer
    Rave* r_obj = (Rave*)OBJ_MEMBER_INT(SELF, rave_data_offset);

    int enable = GET_NEXT_INT(ARGS);
    r_obj->m_enable = enable;

    RETURN->v_int = enable;
}

CK_DLL_MFUN(rave_getEnable)
{
    // get our c++ class pointer
    Rave* r_obj = (Rave*)OBJ_MEMBER_INT(SELF, rave_data_offset);

    RETURN->v_int = r_obj->m_enable;
}
