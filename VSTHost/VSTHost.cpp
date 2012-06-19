
/*----------------------------------------------------------------------------
 ChucK VSTHost Unit Generator
 
 Copyright (c) 2012 Spencer Salazar.  All rights reserved.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 U.S.A.
 -----------------------------------------------------------------------------*/

/* Features liberal copying of code from Audacity, copied under terms of GPL */
/*----------------------------------------------------------------------------
 
 Note: Audacity is distributed under the terms of the GNU GPL.
 This includes everything in the source code distribution
 except code in the lib-src directory.  Code in the lib-src
 directory may be released under a different license
 (which is GPL-compatible).  For a summary, see the
 README.txt file, and for specific details, see the license
 information inside each subdirectory of lib-src.
 
 Documentation is distributed under the Creative Commons license.
 Please see here for that:
 http://creativecommons.org/licenses/by/3.0/legalcode
 
 -----------------------------------------------------------------------------*/


#include "chuck_dl.h"
#include "chuck_def.h"

#include <CoreFoundation/CoreFoundation.h>

#include <stdio.h>
#include <limits.h>

#include "aeffectx.h"


CK_DLL_CTOR(vsthost_ctor);
CK_DLL_DTOR(vsthost_dtor);

CK_DLL_TICKV(vsthost_tick);

CK_DLL_MFUN(vsthost_load);

t_CKINT vsthost_data_offset = 0;


#define chuckVSTID CCONST('C', 'h', 'c', 'K');

typedef AEffect *(*vstPluginMain)(audioMasterCallback audioMaster);


static long int audioMaster(AEffect * effect,
                            long int opcode,
                            long int index,
                            long int value,
                            void * ptr,
                            float opt);

class VSTHost
{
public:
    
    VSTHost(float fs)
    {
        m_effect = NULL;
        m_resource = -1;
        m_module = NULL;
        m_bundle = NULL;
        
        m_vst_buffer_size = 256;
        m_vst_buffer_in = new float*[2];
        m_vst_buffer_in[0] = new float[m_vst_buffer_size];
        m_vst_buffer_in[1] = new float[m_vst_buffer_size];
        m_vst_buffer_out = new float*[2];
        m_vst_buffer_out[0] = new float[m_vst_buffer_size];
        m_vst_buffer_out[1] = new float[m_vst_buffer_size];
    }
    
    void tick(SAMPLE **in, SAMPLE **out, int nFrames)
    {
        assert(nFrames <= m_vst_buffer_size);
        
        // de-interleave
        for(int i = 0; i < nFrames; i++)
        {
            m_vst_buffer_in[0][i] = in[i][0];
            m_vst_buffer_in[1][i] = in[i][1];
        }
        
        if(m_effect)
            m_effect->processReplacing(m_effect, m_vst_buffer_in, 
                                       m_vst_buffer_out, nFrames);
        
        // interleave
        for(int i = 0; i < nFrames; i++)
        {
            out[i][0] = m_vst_buffer_out[0][i];
            out[i][1] = m_vst_buffer_out[1][i];
        }
    }
    
    int load(std::string name)
    {
        int rval = -1;
        vstPluginMain pluginMain = NULL;
        CFStringRef path = NULL;
        CFURLRef urlRef = NULL;
        CFURLRef exeRef = NULL;
        Boolean success;
        
        // Start clean
        m_bundle = NULL;
        
        // Don't really know what this should be initialize to
        m_resource = -1;
        
        // Convert the path to a CFSTring
        path = CFStringCreateWithCString(NULL, name.c_str(), kCFStringEncodingUTF8); 
        
        // Convert the path to a URL
        urlRef = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                               path,
                                               kCFURLPOSIXPathStyle,
                                               true);
        
        if (urlRef == NULL) goto error;        
        // Create the bundle using the URL
        m_bundle = CFBundleCreate(kCFAllocatorDefault, urlRef);
                
        // Bail if the bundle wasn't created
        if (m_bundle == NULL) goto error;
        
        // Retrieve a reference to the executable
        exeRef = CFBundleCopyExecutableURL(m_bundle);
        if (exeRef == NULL) goto error;
        
        // Convert back to path
        UInt8 exePath[PATH_MAX];
        if(!CFURLGetFileSystemRepresentation(exeRef, true, exePath, sizeof(exePath)))
            goto error;
        
        // Attempt to open it
        m_module = dlopen((char *) exePath, RTLD_NOW | RTLD_LOCAL);
        if (m_module == NULL) goto error;
        
        // Try to locate the new plugin entry point
        pluginMain = (vstPluginMain) dlsym(m_module, "VSTPluginMain");
        
        // If not found, try finding the old entry point
        if (pluginMain == NULL) {
            pluginMain = (vstPluginMain) dlsym(m_module, "main_macho");
        }
        
        // Must not be a VST plugin
        if (pluginMain == NULL) goto error;
        
        // Open the resource map ... some plugins (like GRM Tools) need this.
        m_resource = CFBundleOpenBundleResourceMap(m_bundle);
        
        // Initialize the plugin
        m_effect = pluginMain(audioMaster);
        
        // Was it successful?
        if(!m_effect) goto error;
        
        m_effect->user = this;
        
        //
        callDispatcher(effOpen, 0, 0, NULL, 0.0);
        
        // Ensure that it looks like a plugin and can deal with ProcessReplacing
        // calls.  Also exclude synths for now.
        if(m_effect->magic == kEffectMagic &&
           !(m_effect->flags & effFlagsIsSynth) &&
           m_effect->flags & effFlagsCanReplacing)
        {
            
            //                mVendor = GetString(effGetVendorString);
            //                mName = GetString(effGetEffectName);
            //                mInputs = mAEffect->numInputs;
            //                mOutputs = mAEffect->numOutputs;
            
            // We could even go so far as to run a small test here.
                
            rval = 0;
            goto cleanup;
        }
        else
        {
            goto error;
        }
        
    error:
        if(m_effect) { callDispatcher(effClose, 0, 0, NULL, 0.0); m_effect = NULL; }
        if(m_resource != -1) { CFBundleCloseBundleResourceMap(m_bundle, m_resource); m_resource = -1; }
        if(m_module) { dlclose(m_module); m_module = NULL; }
        if(m_bundle) { CFRelease(m_bundle); m_bundle = NULL; }
        if(pluginMain) { pluginMain = NULL; }
        
    cleanup:
        if(exeRef) { CFRelease(exeRef); exeRef = NULL; }
        if(urlRef) { CFRelease(urlRef); urlRef = NULL; }
        if(path) { CFRelease(path); path = NULL; }
        
        return rval;
    }
    
    long callDispatcher(long opcode, long index, long value, 
                        void *ptr, float opt)
    {
        return m_effect->dispatcher(m_effect, opcode, index, value, ptr, opt);
    }
    
    void callProcess(float **inputs, float **outputs, long sampleframes)
    {
        m_effect->process(m_effect, inputs, outputs, sampleframes);
    }
    
    void callProcessReplacing(float **inputs, float **outputs, 
                              long sampleframes)
    {
        m_effect->processReplacing(m_effect, inputs, outputs, sampleframes);
    }
    
    void callSetParameter(long index, float parameter)
    {
        m_effect->setParameter(m_effect, index, parameter);
    }
    
    float callGetParameter(long index)
    {
        return m_effect->getParameter(m_effect, index);
    }
    
private:
    
    AEffect * m_effect;
    
    CFBundleRef m_bundle;
    CFBundleRefNum m_resource;
    void *m_module;
    
    int m_vst_buffer_size;
    float ** m_vst_buffer_in;
    float ** m_vst_buffer_out;
};


static long int audioMaster(AEffect * effect,
                     long int opcode,
                     long int index,
                     long int value,
                     void * ptr,
                     float opt)
{
    VSTHost *vst = (effect ? (VSTHost *) effect->user : NULL);
    
    // Handles operations during initialization...before VSTEffect has had a
    // chance to set its instance pointer.
    switch (opcode)
    {
        case audioMasterVersion:
            return 2400;
            
        case audioMasterCurrentId:
            return chuckVSTID;
            
            // Let the effect know if a pin (channel in our case) is connected
        case audioMasterPinConnected:
//            if (vst) {
//                return (index < vst->GetChannels() ? 0 : 1);
//            }
            break;
            
            // Some (older) effects depend on an effIdle call when requested.  An
            // example is the Antress Modern plugins which uses the call to update
            // the editors display when the program (preset) changes.
        case audioMasterNeedIdle:
//            if (vst) {
//                return vst->callDispatcher(effIdle, 0, 0, NULL, 0.0);
//            }
            break;
            
            // Give the effect a chance to update the editor display
        case audioMasterUpdateDisplay:
//            if (vst) {
//                return vst->callDispatcher(effEditIdle, 0, 0, NULL, 0.0);
//            }
            break;
            
            // Return the current time info.
        case audioMasterGetTime:
//            if (vst) {
//                return (long int) vst->GetTimeInfo();
//            }
            break;
            
            // Ignore these
        case audioMasterBeginEdit:
        case audioMasterEndEdit:
        case audioMasterAutomate:
        case audioMasterGetCurrentProcessLevel:
        case audioMasterIdle:
        case audioMasterWantMidi:
            return 0;
            
        case audioMasterCanDo:
            return 0;
    }
}


CK_DLL_QUERY(VSTHost)
{
    QUERY->setname(QUERY, "VSTHost");
    
    QUERY->begin_class(QUERY, "VSTHost", "UGen");
    
    QUERY->add_ctor(QUERY, vsthost_ctor);
    QUERY->add_dtor(QUERY, vsthost_dtor);
    
    QUERY->add_ugen_funcv(QUERY, vsthost_tick, NULL, 2, 2);
    
    QUERY->add_mfun(QUERY, vsthost_load, "int", "load");
    QUERY->add_arg(QUERY, "string", "arg");
    
    vsthost_data_offset = QUERY->add_mvar(QUERY, "int", "@vsthost_data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(vsthost_ctor)
{
    OBJ_MEMBER_INT(SELF, vsthost_data_offset) = 0;
    
    VSTHost * bcdata = new VSTHost(API->vm->get_srate());
    
    OBJ_MEMBER_INT(SELF, vsthost_data_offset) = (t_CKINT) bcdata;
}

CK_DLL_DTOR(vsthost_dtor)
{
    VSTHost * bcdata = (VSTHost *) OBJ_MEMBER_INT(SELF, vsthost_data_offset);
    if(bcdata)
    {
        delete bcdata;
        OBJ_MEMBER_INT(SELF, vsthost_data_offset) = 0;
        bcdata = NULL;
    }
}

CK_DLL_TICKV(vsthost_tick)
{
    VSTHost * c = (VSTHost *) OBJ_MEMBER_INT(SELF, vsthost_data_offset);
    
    if(c) c->tick(in, out, nframes);

    return TRUE;
}

CK_DLL_MFUN(vsthost_load)
{
    VSTHost * bcdata = (VSTHost *) OBJ_MEMBER_INT(SELF, vsthost_data_offset);
    // TODO: sanity check
    RETURN->v_int = bcdata->load(GET_NEXT_STRING(ARGS)->str);
}

