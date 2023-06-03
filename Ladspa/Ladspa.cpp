//-----------------------------------------------------------------------------
// Entaro ChucK Developer!
// This is a Chugin boilerplate, generated by chuginate!
//-----------------------------------------------------------------------------

// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"
#include "ladspa.h"
//#include "utils.h"

// general includes
#include <math.h>
//#include <stdio.h>
//#include <limits.h>
//#include <dlfcn.h>
//#include <assert.h>
//#include <cstring>
//#include <string>


#define DEFAULT_BUFSIZE 1

#define DEBUG

// declaration of chugin constructor
CK_DLL_CTOR(ladspa_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(ladspa_dtor);

CK_DLL_MFUN(ladspa_load);
CK_DLL_MFUN(ladspa_list);
CK_DLL_MFUN(ladspa_info);
CK_DLL_MFUN(ladspa_label);
CK_DLL_MFUN(ladspa_set);
CK_DLL_MFUN(ladspa_get);
CK_DLL_MFUN(ladspa_verbose);

// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICKF(ladspa_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT ladspa_data_offset = 0;

// class definition for internal Chugin data
// (note: this isn't strictly necessary, but serves as example
// of one recommended approach)
class Ladspa
{
public:
  // create port type to differentiate control ports
  enum port_t { INPUT, OUTPUT };
  
  // control data is written into an array of ControlData structs
  struct ControlData
  {
	unsigned short ladspaIndex;
	LADSPA_Data value;
	port_t porttype;
  };

  // constructor
  Ladspa( t_CKFLOAT fs) :
    pluginLoaded(false),
    pluginActivated(false),
    verbose(true),
    srate(fs),
    bufsize(DEFAULT_BUFSIZE)
  {
  }

  // destructor
  ~Ladspa()
  {
    if (pluginActivated) psDescriptor->cleanup(pPlugin);
    if (pluginLoaded)
      {
	dlclose(pvPluginHandle);
	if (verbose) printf("LADSPA: closed plugin\n");
      }
	for (int i=0; i<inports; i++)
	  delete inbuf[i];
	for (int i=0; i<outports; i++)
	  delete outbuf[i];
	delete [] inbuf;
	delete [] outbuf;
	delete [] kbuf;
  }
  
  // for Chugins extending UGen
  void tick( SAMPLE *in, SAMPLE *out, int nframes )
  {
	if (pluginActivated)
	  {
		for (int i=0; i<inports; i++) inbuf[i][0] = (LADSPA_Data)in[i%2];
		psDescriptor->run(pPlugin, bufsize);
		for (int i=0; i<2; i++) out[i%2] = (SAMPLE)outbuf[i%outports][0];
	  }
	else
	  {
		out[0] = in[0];
		out[1] = in[1];
	  }
  }

  float set( float val, int param)
  {
#ifdef DEBUG
    printf ("DEBUG: method 'set' : param number %d, value %f\n",param, val);
#endif
	if (pluginActivated)
	  {
		if (param < kports)
		  if (kbuf[param].porttype != INPUT)
			{
			  printf ( "LADSPA error: selected param is output only.\n");
			  return 0;
			}
		  else
			{
			  if (verbose) printf ("LADSPA: setting parameter \"%s\" to %g\n",
								   psDescriptor->PortNames[kbuf[param].ladspaIndex], val);
			  kbuf[param].value = (LADSPA_Data)val;
			  psDescriptor->run(pPlugin, 0);
			}
		else
		  if (kports>1)
			printf ("LADSPA error: param must be between 0 and %d.\n", kports-1);
		  else
			printf ("LADSPA error: param must be 0.\n");
	  }
	return val;
  }
  
  float get (int param)
  {
	if (pluginActivated)
	  {
		if (param < kports)
		  return kbuf[param].value;
		if (kports>0)
		  printf ("LADSPA error: param must be between 0 and %d.\n", kports-1);
		else
		  printf ("LADSPA error: param must be 0.\n");
	  }
	return 0;
  }

  int LADSPAverbose (int val)
  {
#ifdef DEBUG
    printf ("DEBUG: method 'verbose' : value %d\n", val);
#endif
	if (val) verbose = true;
	else verbose = false;
	return val;
  }
  
  int LadspaActivate ( const char * pcPluginLabel )
  {
	assert(pcPluginLabel != NULL);
	if (pluginLoaded)
	  {
#ifdef DEBUG
	    printf("DEBUG: method 'LadspaActivate' received string \"%s\"\n", pcPluginLabel);
#endif
	    if (pluginActivated)
	      {
		psDescriptor->cleanup(pPlugin);
		if (verbose) printf("LADSPA: deactivating current plugin...\n");
		pluginActivated = false;
	      }
	    for (int i=0;; i++)
		  {
			psDescriptor = pfDescriptorFunction(i);
			if (psDescriptor == NULL)
			  {
				printf("LADSPA error: unable to find label \"%s\" in plugin.\n", pcPluginLabel);
				return 0;
			  }
			if (strcmp(psDescriptor->Label, pcPluginLabel) == 0)
			  {
				if (verbose) printf("LADSPA: activating plugin \"%s\"\n", pcPluginLabel);
				pPlugin = psDescriptor->instantiate(psDescriptor, srate);
				connectPorts();
				pluginActivated = true;
				return 1;
			  }
		  }
	  }
	else
	  printf ("LADSPA error: no plugin loaded yet!\n");
	return 0;
  }

  int LADSPA_load ( const char *  pcPluginFilename )
  {
#ifdef DEBUG
	    printf("DEBUG: method 'LADSPA_load' received string \"%s\"\n", pcPluginFilename);
#endif
    if (pluginActivated)
	  {
		psDescriptor->cleanup(pPlugin);
		if (verbose) printf("LADSPA: deactivating current plugin...\n");
	  }
    if (pluginLoaded)
	  {
		dlclose(pvPluginHandle);
		if (verbose) printf("LADSPA: unloading current plugin...\n");
	  }
	pluginActivated = false;
	pluginLoaded = false;
	assert(pcPluginFilename != NULL);
#ifdef DEBUG
	printf("DEBUG: received string \"%s\"\n", pcPluginFilename);
#endif
    if (verbose) printf("LADSPA: loading plugin %s\n", pcPluginFilename);
    pvPluginHandle = dlopen(pcPluginFilename, RTLD_NOW);
    dlerror();
    
    pfDescriptorFunction = (LADSPA_Descriptor_Function)dlsym(pvPluginHandle, "ladspa_descriptor");
    if (!pfDescriptorFunction)
      {
		const char * pcError = dlerror();
		if (pcError) 
		  printf("LADSPA error: unable to find ladspa_descriptor() function in plugin file "
				 "\"%s\"\n"
				 "Are you sure this is a LADSPA plugin file?\n", 
				 pcPluginFilename);
		return 0;
      }
	pluginLoaded = true;
	return 1;
  }

  // TODO: error checking  
  int LADSPA_list ()
  {
#ifdef DEBUG
    printf("DEBUG: method 'Ladspa_list'\n");
#endif
    if (!pluginLoaded)
      {
	printf ("LADSPA error: no plugin loaded yet\n");
	return 0;
      }
    printf ("Plugins available under this LADSPA file:\n");
    for (int i = 0;; i++)
      {
	const LADSPA_Descriptor * testDescriptor = pfDescriptorFunction(i);
	if (!testDescriptor)
	  break;
	putchar('\n');
	printf("Plugin Label: \"%s\"\n", testDescriptor->Label);
	printf("Plugin Name: \"%s\"\n", testDescriptor->Name);
      }
    printf("--------------------------------------------------\n");
    return 1;
  }

int LADSPA_info ()
  {
#ifdef DEBUG
	    printf("DEBUG: method 'Ladspa_info'\n");
#endif
	if (pluginActivated)
	  {
		bool bFound;
		int knum;
		unsigned long lIndex;
		LADSPA_PortRangeHintDescriptor iHintDescriptor;
		LADSPA_Data fBound;
		printf("--------------------------------------------------\n");	
		printf(
			   "Plugin \"%s\" has the following control inputs:\n",
			   psDescriptor->Name);
		
		knum = 0;
		bFound = false;
		for (lIndex = 0; lIndex < psDescriptor->PortCount; lIndex++)
		  {
			if (LADSPA_IS_PORT_CONTROL(psDescriptor->PortDescriptors[lIndex]))
			  {
				if (LADSPA_IS_PORT_INPUT(psDescriptor->PortDescriptors[lIndex]))
				  {
					printf("\tControl %d: %s", knum++, psDescriptor->PortNames[lIndex]);
					bFound = true;
					iHintDescriptor = psDescriptor->PortRangeHints[lIndex].HintDescriptor;
					if (LADSPA_IS_HINT_BOUNDED_BELOW(iHintDescriptor)
						|| LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor))
					  {
						printf( " (");
						if (LADSPA_IS_HINT_BOUNDED_BELOW(iHintDescriptor))
						  {
							fBound = psDescriptor->PortRangeHints[lIndex].LowerBound;
							if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
							  {
								if (fBound == 0) printf( "0");
								else printf( "%g * sample rate", fBound);
							  }
							else printf( "%g", fBound);
						  }
						else
						  printf( "...");
						printf( " to ");
						if (LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor))
						  {
							fBound = psDescriptor->PortRangeHints[lIndex].UpperBound;
							if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
							  {
								if (fBound == 0)
								  printf( "0");
								else
								  printf( "%g * sample rate", fBound);
							  }
							else
							  printf( "%g", fBound);
						  }
						else
						  printf( "...");
						printf( "), default: %g\n", get_default(lIndex));
					  }
					else printf("\n");
				  }
				else
				  {
					printf("\tOutput %d: %s\n",knum++, psDescriptor->PortNames[lIndex]);
					bFound = true;
				  }
			  }
		  }
		
		if (!bFound)
		  printf( "\tnone\n");
		printf("--------------------------------------------------\n");
	  }
	else
	  if (!pluginLoaded)
		printf ("LADSPA error: no plugin loaded yet!\n");
	  else
		printf ("LADSPA error: plugin not yet activated\n");
	return 0;
  }

private:
  
  void connectPorts()
  {
	//const LADSPA_Descriptor * thisDescriptor = psDescriptor;
    //printf("Connecting LADSPA audio ports...\n\n");

    // Count ports
    inports = 0; // audio in
    outports = 0; // audio out
    kports = 0; // control data (in and out)

    for (int i=0; i<psDescriptor->PortCount; i++)
      {
	iPortDescriptor = psDescriptor->PortDescriptors[i];
	if (LADSPA_IS_PORT_AUDIO(iPortDescriptor))
	  {
	    if (LADSPA_IS_PORT_INPUT(iPortDescriptor)) inports++;
	    else if (LADSPA_IS_PORT_OUTPUT(iPortDescriptor)) outports++;
	  }
	else if (LADSPA_IS_PORT_CONTROL(iPortDescriptor))
	  {
	    kports++;
	  }
      }
    
    inbuf = new LADSPA_Data*[inports];
    outbuf = new LADSPA_Data*[outports];
	kbuf = new ControlData[kports];
    for (int i=0; i<inports; i++)
      {
		inbuf[i] = new LADSPA_Data[bufsize];
      }
    for (int i=0; i<outports; i++)
      {
		outbuf[i] = new LADSPA_Data[bufsize];
      }
    for (int i=0; i<kports; i++)
      {
	kbuf[i].value = 0.0;
	kbuf[i].ladspaIndex = 0;
	kbuf[i].porttype = INPUT;
      }
	
    //printf("Audio inports: %d, outports: %d, Control ports: %d\n",inports, outports, kports);
    
    int inbufIndex = 0;
    int outbufIndex = 0;
    int kbufIndex = 0;

    // connect ports
    for (int i=0; i<psDescriptor->PortCount; i++)
      {
	iPortDescriptor = psDescriptor->PortDescriptors[i];
	if (LADSPA_IS_PORT_AUDIO(iPortDescriptor))
	  {
	    if (LADSPA_IS_PORT_INPUT(iPortDescriptor))
	      psDescriptor->connect_port(pPlugin, i, inbuf[inbufIndex++]);
	    else if (LADSPA_IS_PORT_OUTPUT(iPortDescriptor))
	      psDescriptor->connect_port(pPlugin, i, outbuf[outbufIndex++]);
	  }
	else if (LADSPA_IS_PORT_CONTROL(iPortDescriptor))
	  {
	    psDescriptor->connect_port(pPlugin, i, &kbuf[kbufIndex].value);
	    kbuf[kbufIndex].ladspaIndex = i;
	    if (LADSPA_IS_PORT_INPUT (iPortDescriptor))
	      {
		kbuf[kbufIndex].porttype = INPUT;
		LADSPA_Data portDefault = get_default(i);
		kbuf[kbufIndex].value = portDefault;
	      }
	    else kbuf[kbufIndex].porttype = OUTPUT;
	    kbufIndex++;
	  }
      }
	
	if (psDescriptor->activate != NULL)
	  {
		psDescriptor->activate(pPlugin);
		pluginActivated = true;
	  }
  }

  LADSPA_Data get_default ( int index )
  {
	iHintDescriptor = psDescriptor->PortRangeHints[index].HintDescriptor;
	switch (iHintDescriptor & LADSPA_HINT_DEFAULT_MASK) {
	case LADSPA_HINT_DEFAULT_NONE:
	  break;
	case LADSPA_HINT_DEFAULT_MINIMUM:
	  fDefault = psDescriptor->PortRangeHints[index].LowerBound;
	  break;
	case LADSPA_HINT_DEFAULT_LOW:
	  if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
		fDefault 
		  = exp(log(psDescriptor->PortRangeHints[index].LowerBound) 
				* 0.75
				+ log(psDescriptor->PortRangeHints[index].UpperBound) 
				* 0.25);
	  }
	  else {
		fDefault 
		  = (psDescriptor->PortRangeHints[index].LowerBound
			 * 0.75
			 + psDescriptor->PortRangeHints[index].UpperBound
			 * 0.25);
	  }
	  break;
	case LADSPA_HINT_DEFAULT_MIDDLE:
	  if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
		fDefault 
		  = sqrt(psDescriptor->PortRangeHints[index].LowerBound
				 * psDescriptor->PortRangeHints[index].UpperBound);
	  }
	  else {
		fDefault 
		  = 0.5 * (psDescriptor->PortRangeHints[index].LowerBound
				   + psDescriptor->PortRangeHints[index].UpperBound);
	  }
	  break;
	case LADSPA_HINT_DEFAULT_HIGH:
	  if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
		fDefault 
		  = exp(log(psDescriptor->PortRangeHints[index].LowerBound) 
				* 0.25
				+ log(psDescriptor->PortRangeHints[index].UpperBound) 
				* 0.75);
	  }
	  else {
		fDefault 
		  = (psDescriptor->PortRangeHints[index].LowerBound
			 * 0.25
			 + psDescriptor->PortRangeHints[index].UpperBound
			 * 0.75);
	  }
	  break;
	case LADSPA_HINT_DEFAULT_MAXIMUM:
	  fDefault = psDescriptor->PortRangeHints[index].UpperBound;
	  break;
	case LADSPA_HINT_DEFAULT_0:
	  fDefault = 0;
	  break;
	case LADSPA_HINT_DEFAULT_1:
	  fDefault = 1;
	  break;
	case LADSPA_HINT_DEFAULT_100:
	  fDefault = 100;
	  break;
	case LADSPA_HINT_DEFAULT_440:
	  fDefault = 440;
	  break;
	default:
	  printf("LADSPA warning: UNKNOWN DEFAULT CODE\n");
	  // (Not necessarily an error - may be a newer version.)
	  break;
	}
	return fDefault;
  }
  
  // instance data
  LADSPA_Descriptor_Function pfDescriptorFunction;
  const LADSPA_Descriptor * psDescriptor;
  LADSPA_PortRangeHintDescriptor iHintDescriptor;
  LADSPA_PortDescriptor iPortDescriptor;
  LADSPA_Data fBound;
  LADSPA_Data fDefault;
  LADSPA_Handle pPlugin;
  LADSPA_Data ** inbuf, ** outbuf; // audio in and out buffers (multichannel)
  ControlData * kbuf; // control data buffers
  void * pvPluginHandle;
  bool pluginLoaded, pluginActivated;
  bool verbose;
  int bufsize;
  unsigned short numchans;
  unsigned short kports, inports, outports;
  float srate;
};


// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( Ladspa )
{
  // hmm, don't change this...
  QUERY->setname(QUERY, "LADSPA");
  
  // begin the class definition
  // can change the second argument to extend a different ChucK class
  QUERY->begin_class(QUERY, "LADSPA", "UGen");
  
  // register the constructor (probably no need to change)
  QUERY->add_ctor(QUERY, ladspa_ctor);
  // register the destructor (probably no need to change)
  QUERY->add_dtor(QUERY, ladspa_dtor);
  
  // for UGen's only: add tick function
  QUERY->add_ugen_funcf(QUERY, ladspa_tick, NULL, 2, 2);
  
  // NOTE: if this is to be a UGen with more than 1 channel, 
  // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
  // and declare a tickf function using CK_DLL_TICKF
  
  // example of adding setter method
  QUERY->add_mfun(QUERY, ladspa_load, "int", "load");
  // example of adding argument to the above method
  QUERY->add_arg(QUERY, "string", "filename");
  
  // example of adding setter method
  QUERY->add_mfun(QUERY, ladspa_label, "int", "activate");
  // example of adding argument to the above method
  QUERY->add_arg(QUERY, "string", "label");

  // example of adding setter method
  QUERY->add_mfun(QUERY, ladspa_set, "float", "set");
  // example of adding argument to the above method
  QUERY->add_arg(QUERY, "int", "param");
  QUERY->add_arg(QUERY, "float", "val");

  // example of adding setter method
  QUERY->add_mfun(QUERY, ladspa_get, "float", "get");
  QUERY->add_arg(QUERY, "int", "param");
  
  // example of adding setter method
  QUERY->add_mfun(QUERY, ladspa_info, "int", "info");

  // example of adding setter method
  QUERY->add_mfun(QUERY, ladspa_list, "int", "list");

  // example of adding setter method
  QUERY->add_mfun(QUERY, ladspa_verbose, "int", "verbose");
  QUERY->add_arg(QUERY, "int", "val");
  
  // this reserves a variable in the ChucK internal class to store 
  // referene to the c++ class we defined above
  ladspa_data_offset = QUERY->add_mvar(QUERY, "int", "@l_data", false);
  
  // end the class definition
  // IMPORTANT: this MUST be called!
  QUERY->end_class(QUERY);
  
  // wasn't that a breeze?
  return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(ladspa_ctor)
{
  // get the offset where we'll store our internal c++ class pointer
  OBJ_MEMBER_INT(SELF, ladspa_data_offset) = 0;
  
  // instantiate our internal c++ class representation
  Ladspa * bcdata = new Ladspa(API->vm->get_srate(API, SHRED));
  
  // store the pointer in the ChucK object member
  OBJ_MEMBER_INT(SELF, ladspa_data_offset) = (t_CKINT) bcdata;
}


// implementation for the destructor
CK_DLL_DTOR(ladspa_dtor)
{
  // get our c++ class pointer
  Ladspa * bcdata = (Ladspa *) OBJ_MEMBER_INT(SELF, ladspa_data_offset);
  // check it
  if( bcdata )
    {
      // clean up
      delete bcdata;
      OBJ_MEMBER_INT(SELF, ladspa_data_offset) = 0;
      bcdata = NULL;
    }
}

// implementation for tick function
CK_DLL_TICKF(ladspa_tick)
{
  // get our c++ class pointer
  Ladspa * c = (Ladspa *) OBJ_MEMBER_INT(SELF, ladspa_data_offset);
  
  // invoke our tick function; store in the magical out variable
  //if(c) *out = c->tick(in);
  if(c) c->tick(in, out, nframes);

  // yes
  return TRUE;
}

// example implementation for setter
CK_DLL_MFUN(ladspa_load)
{
  // get our c++ class pointer
  Ladspa * bcdata = (Ladspa *) OBJ_MEMBER_INT(SELF, ladspa_data_offset);
  // set the return value
  std::string chuckstr = GET_CK_STRING_SAFE(ARGS);
  char * name = new char [chuckstr.length()];
  strcpy (name, chuckstr.data());
  RETURN->v_int = bcdata->LADSPA_load(name);
}

// example implementation for setter
CK_DLL_MFUN(ladspa_label)
{
  // get our c++ class pointer
  Ladspa * bcdata = (Ladspa *) OBJ_MEMBER_INT(SELF, ladspa_data_offset);
  // set the return value
  std::string name = GET_CK_STRING_SAFE(ARGS);
  RETURN->v_int = bcdata->LadspaActivate(name.c_str());
}

// example implementation for setter
CK_DLL_MFUN(ladspa_info)
{
  // get our c++ class pointer
  Ladspa * bcdata = (Ladspa *) OBJ_MEMBER_INT(SELF, ladspa_data_offset);
  // set the return value
  RETURN->v_int = bcdata->LADSPA_info();
}

// example implementation for setter
CK_DLL_MFUN(ladspa_list)
{
  // get our c++ class pointer
  Ladspa * bcdata = (Ladspa *) OBJ_MEMBER_INT(SELF, ladspa_data_offset);
  // set the return value
  RETURN->v_int = bcdata->LADSPA_list();
}

// example implementation for setter
CK_DLL_MFUN(ladspa_set)
{
  // get our c++ class pointer
  Ladspa * bcdata = (Ladspa *) OBJ_MEMBER_INT(SELF, ladspa_data_offset);
  // set the return value
  t_CKINT param = GET_NEXT_INT(ARGS);
  t_CKFLOAT val = GET_NEXT_FLOAT(ARGS);

  RETURN->v_float = bcdata->set(val, param);
}

// example implementation for setter
CK_DLL_MFUN(ladspa_get)
{
  // get our c++ class pointer
  Ladspa * bcdata = (Ladspa *) OBJ_MEMBER_INT(SELF, ladspa_data_offset);
  // set the return value
  t_CKINT param = GET_NEXT_INT(ARGS);

  RETURN->v_float = bcdata->get(param);
}

// example implementation for setter
CK_DLL_MFUN(ladspa_verbose)
{
  // get our c++ class pointer
  Ladspa * bcdata = (Ladspa *) OBJ_MEMBER_INT(SELF, ladspa_data_offset);
  // set the return value
  RETURN->v_int = bcdata->LADSPAverbose(GET_NEXT_INT(ARGS));
}


// windows
#if defined(__PLATFORM_WIN32__)
extern "C"
{
#ifndef __CHUNREAL_ENGINE__
#include <windows.h>
#else
	// 1.5.0.0 (ge) | #chunreal
	// unreal engine on windows disallows including windows.h
#include "Windows/MinWindows.h"
#endif // #ifndef __CHUNREAL_ENGINE__

	void* dlopen(const char* path, int mode)
	{
#ifndef __CHUNREAL_ENGINE__
		return (void*)LoadLibrary(path);
#else
		// 1.5.0.0 (ge) | #chunreal; explicitly call ASCII version
		// the build envirnment seems to force UNICODE
		return (void*)LoadLibraryA(path);
#endif
	}

	int dlclose(void* handle)
	{
		FreeLibrary((HMODULE)handle);
		return 1;
	}

	void* dlsym(void* handle, const char* symbol)
	{
		return (void*)GetProcAddress((HMODULE)handle, symbol);
	}

	const char* dlerror(void)
	{
		int error = GetLastError();
		if (error == 0) return NULL;
		// 1.4.2.0 (ge) | changed to snprintf
		snprintf(dlerror_buffer, DLERROR_BUFFER_LENGTH, "%i", error);
		return dlerror_buffer;
	}
}
#endif
