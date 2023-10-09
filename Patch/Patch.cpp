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
// TODO: type check arguments of provided functions (prob requires more api updates)
// TODO: use operator overloading to do SinOsc s => Patch p => r.freq ?

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
  Patch()
  {
    m_dest = nullptr;
  }

  // destructor
  ~Patch()
  {
    disconnect();
  }

    // for Chugins extending UGen
    SAMPLE tick( SAMPLE in )
    {
      // Patch hasn't been connected yet
      if (m_method == "") return in;

      // Connect failed to find a valid member function
      if (m_vt_offset < 0) return in;

      // construct arg from input sample
      Chuck_DL_Arg* arg = new Chuck_DL_Arg();
      arg->kind = kindof_FLOAT;
      arg->value.v_float = in;

      // call the destination object's member function with "in" as the input argument
      Chuck_DL_Return ret = m_api->vm->invoke_mfun_immediate_mode(m_dest, m_vt_offset, m_vm, m_shred, arg, 1);

      return in;
    }

    // set parameter example
    void connect( Chuck_Object* dest, std::string method, Chuck_VM* vm, Chuck_VM_Shred* shred, CK_DL_API api)
    {
      // cleanup if patch was previously connected
      disconnect();

      m_dest = dest;
      m_vm = vm;
      m_api = api;
      m_shred = shred;

      m_method = method;
      m_vt_offset = findMethodOffset(method);

      // add reference to dest object for VM memory management
      m_api->object->add_ref(m_dest);
    }

    void disconnect() {
      // release object from reference
      if (m_dest)
        m_api->object->release(m_dest);

      m_dest = nullptr;
      m_vm = nullptr;
      m_api = nullptr;
      m_shred = nullptr;
    };

    std::string getMethod() { return m_method; }


    void setMethod(std::string method) {
      m_vt_offset = findMethodOffset(method);
      m_method = method;
    }
    
private:
  // instance data

  // TODO: can this just be an object?
  Chuck_Object* m_dest;
  Chuck_VM_Shred* m_shred;
  Chuck_VM* m_vm;
  CK_DL_API m_api;
  t_CKUINT m_vt_offset;
  std::string m_method;

  // find class method vtable offset
  t_CKINT findMethodOffset(std::string method) {
    // first hard code ugen, then figure out general version
    Chuck_DL_Api::Type obj_type = m_dest->type_ref;

    // just gain for now
    t_CKINT offset = m_api->type->get_vtable_offset(m_vm, obj_type, method.c_str());

    // TODO check for error, and make non-crashing exception
    if (offset < 0) {
      // TODO: type->name() doesn't work, figure out why
      std::string message = "Member function " + method + "() not found in object " + obj_type->base_name;
      m_api->vm->throw_exception("MemberFunctionNotFound", message.c_str(), m_shred);
    }

    return offset;
  }
};


// query function: chuck calls this when loading the Chugin
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
    QUERY->add_arg(QUERY, "Object", "dest" );
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
    Patch * p_obj = new Patch();
    
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
    RETURN->v_string = (Chuck_String*)API->object->create_string(VM, method.c_str(), FALSE);
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
    Chuck_Object * dest = (Chuck_Object *)GET_NEXT_OBJECT(ARGS);
    Chuck_String* method = (Chuck_String*)GET_NEXT_STRING(ARGS);

    p_obj->connect(dest, method->str(), VM, SHRED, API);
 }

CK_DLL_MFUN(patch_disconnect)
{
    // get our c++ class pointer
    Patch* p_obj = (Patch*)OBJ_MEMBER_INT(SELF, patch_data_offset);

    p_obj->disconnect();
}
