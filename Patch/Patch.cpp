//-----------------------------------------------------------------------------
// name: Patch.cpp
// desc: Pipe UGen signals into UGen methods directly!
//       The primary use of this is easily and efficiently
//       control UGen parameters with tick-level LFOs.
// 
// example:
// SinOsc sin => Pan2 pan => dac;
//
// // Our LFO is fed into Patch's input...
// SinOsc lfo = > Patch p = > blackhole;
// 1 = > lfo.freq;
//
// // ... where it then updates the pan position at every tick
// p.connect(pan, "pan");
// 
// 1::hour = > now;
//
// authors: Nick Shaheed (nshaheed@ccrma.stanford.edu)
// date: Winter 2022
//
//-----------------------------------------------------------------------------

// TODO: add disconnect method

#include "chuck_dl.h"
#include "chuck_vm.h"

// general includes
#include <stdio.h>
#include <limits.h>
#include <iostream>

// declaration of chugin constructor
CK_DLL_CTOR(patch_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(patch_dtor);

// Patch methods
CK_DLL_MFUN(patch_connect);
CK_DLL_MFUN(patch_disconnect);
CK_DLL_MFUN(patch_getMethod);
CK_DLL_MFUN(patch_setMethod);

// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICK(patch_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT patch_data_offset = 0;

// The Patch class
class Patch
{
public:
    // constructor
    Patch( t_CKFLOAT fs)
    {
        m_func = nullptr;
    }

    // for Chugins extending UGen
    SAMPLE tick( SAMPLE in )
    {
        // if the method is set, patch through the value
        if (m_func) {
            // default: this passes whatever input is patched into Chugin
            t_CKFLOAT val = (t_CKFLOAT)in;
            Chuck_DL_Return ret;

            // these three lines took a very long time to figure out...
            Chuck_VM_Code* func = m_func->code;
            // cast the function as a dynamically-linked member func
            f_mfun f = (f_mfun)func->native_func;
            // call said function
            f((Chuck_Object*)(m_dest), &val, &ret, m_vm, m_shred, m_api); // api?
        }
        return in;
    }

    // set parameter example
    void connect( Chuck_UGen* dest, std::string method, Chuck_VM* vm, Chuck_VM_Shred* shred, CK_DL_API api)
    {
        m_dest = dest;
        m_vm = vm;
        m_api = api;
        m_shred = shred;

        findMethod(method);
    }

    void disconnect() {
        m_dest = NULL;
        m_vm = NULL;
        m_api = NULL;
        m_shred = NULL;
        m_func = NULL;
    };

    std::string getMethod() { return m_func->base_name; }


    void setMethod(std::string method) {
        findMethod(method);
    }
    
private:
    // instance data
    Chuck_UGen* m_dest;
    Chuck_VM_Shred* m_shred;
    Chuck_VM* m_vm;
    Chuck_Func* m_func;
    CK_DL_API m_api;

    // Given a UGen, try to the find the methods among its
    // associated functions.
    void findMethod(std::string method) {
        Chuck_Func* found = NULL;

        for (int i = 0; i < m_dest->vtable->funcs.size(); i++)
        {
            Chuck_Func* func = m_dest->vtable->funcs[i];

            // funcs can be overwritten or have multiple defn, look for the right one
            if (func->base_name == method &&
                func->def()->arg_list != NULL &&
                // we only want funcs with one arg
                func->def()->arg_list->next == NULL &&
                // ensure arg is float
                func->def()->arg_list->type == m_shred->vm_ref->env()->ckt_float)
            {
                found = func;
                break;
            }
        }

        if (!found) {
            std::cerr << "Patch.connect(): unable to find method " << method << std::endl;
            return;
        }

        Chuck_Func* curr = found;
        // traverse overloads to find top of stack
        while (curr->next != NULL) {
            if (curr->def()->arg_list != NULL &&
                // we only want funcs with one arg
                curr->def()->arg_list->next == NULL &&
                // ensure arg is float
                curr->def()->arg_list->type == m_shred->vm_ref->env()->ckt_float)
            {
                found = curr;
            }
            curr = curr->next;
        }

        m_func = found;

        return;
    }
};


// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( Patch )
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "Patch");
    
    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "Patch", "UGen");

    QUERY->doc_class(QUERY, "Pipe UGen signals into UGen methods directly! "
        "The primary use of this is to easily and efficiently "
        "control UGen parameters with tick-level LFOs."
    );

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, patch_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, patch_dtor);
    
    // for UGen's only: add tick function
    QUERY->add_ugen_func(QUERY, patch_tick, NULL, 1, 1);

    // connect method
    QUERY->add_mfun(QUERY, patch_connect, "void", "connect");
    QUERY->add_arg(QUERY, "UGen", "dest" );
    QUERY->add_arg(QUERY, "string", "method");
    QUERY->doc_func(QUERY, "Call the method in dest with Patch's input. "
        "Method must be a method that takes a single float as an input, "
        "i.e. setters such as SinOsc.freq and Pan2.pan.");
    
    QUERY->add_mfun(QUERY, patch_disconnect, "void", "disconnect");
    QUERY->doc_func(QUERY, "Disconnect the currently attached method from Patch. "
        "If the method is not set, nothing will happen.");

    // method getter and setter
    QUERY->add_mfun(QUERY, patch_getMethod, "string", "method");
    QUERY->doc_func(QUERY, "Get method name");

    QUERY->add_mfun(QUERY, patch_setMethod, "string", "method");
    QUERY->add_arg(QUERY, "string", "method");
    QUERY->doc_func(QUERY, "Set method name");

    
    // this reserves a variable in the ChucK internal class to store 
    // referene to the c++ class we defined above
    patch_data_offset = QUERY->add_mvar(QUERY, "int", "@p_data", false);


    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(patch_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, patch_data_offset) = 0;
    
    // instantiate our internal c++ class representation
    Patch * p_obj = new Patch(API->vm->srate(VM));
    
    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, patch_data_offset) = (t_CKINT) p_obj;
}


// implementation for the destructor
CK_DLL_DTOR(patch_dtor)
{
    // get our c++ class pointer
    Patch * p_obj = (Patch *) OBJ_MEMBER_INT(SELF, patch_data_offset);
    // check it
    if( p_obj )
    {
        // clean up
        delete p_obj;
        OBJ_MEMBER_INT(SELF, patch_data_offset) = 0;
        p_obj = NULL;
    }
}


// implementation for tick function
CK_DLL_TICK(patch_tick)
{
    // get our c++ class pointer
    Patch * p_obj = (Patch *) OBJ_MEMBER_INT(SELF, patch_data_offset);
 
    // invoke our tick function; store in the magical out variable
    if(p_obj) *out = p_obj->tick(in);

    // yes
    return TRUE;
}

// get the name of the current method being patched
CK_DLL_MFUN(patch_getMethod)
{
    Patch* p_obj = (Patch*)OBJ_MEMBER_INT(SELF, patch_data_offset);
    
    std::string method = p_obj->getMethod();
    // create chuck string, addRef=false since we are returning the created string
    // and do not keep a reference of it as part of this chugin
    RETURN->v_string = (Chuck_String*)API->object->create_string(VM, method.c_str(), false);
}

// get the name of the current method being patched
CK_DLL_MFUN(patch_setMethod)
{
    Patch* p_obj = (Patch*)OBJ_MEMBER_INT(SELF, patch_data_offset);
    Chuck_String* method = (Chuck_String*)GET_NEXT_STRING(ARGS);

    p_obj->setMethod(method->str());
    RETURN->v_string = method;
}


// connect the ugen's method so that patch's input will set it
CK_DLL_MFUN(patch_connect)
{
    // get our c++ class pointer
    Patch * p_obj = (Patch *) OBJ_MEMBER_INT(SELF, patch_data_offset);
    Chuck_UGen * dest = (Chuck_UGen *)GET_NEXT_OBJECT(ARGS);
    Chuck_String* method = (Chuck_String*)GET_NEXT_STRING(ARGS);

    p_obj->connect(dest, method->str(), VM, SHRED, API);
 }

CK_DLL_MFUN(patch_disconnect)
{
    // get our c++ class pointer
    Patch* p_obj = (Patch*)OBJ_MEMBER_INT(SELF, patch_data_offset);

    p_obj->disconnect();
}
