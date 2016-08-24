Hid hi;
HidMsg msg;

Faust string => Faust distortion => Faust autoWah => dac;

string.compile("string.dsp");

distortion.eval(`
process = cubicnl_demo;
`);

autoWah.eval("
process = autowah(1);
");

distortion.v("/CUBIC_NONLINEARITY_cubicnl/Drive",1);
distortion.dump();

0 => int device;
if( me.args() ) me.arg(0) => Std.atoi => device;

// open keyboard (get device number from command line)
if( !hi.openKeyboard( device ) ) me.exit();

// infinite event loop
while( true )
{
    // wait on event
    hi => now;
    
    // get one or more messages
    while( hi.recv( msg ) )
    {
        // check for action type
        if( msg.isButtonDown() )
        {
            string.v("feedback",0.98); 
            string.v("gate",1);
            if(msg.which == 4){
              string.v("freq",Std.mtof(60));
           }
           else if(msg.which == 26){ 
               string.v("freq",Std.mtof(61));
           }
           else if(msg.which == 22){ 
               string.v("freq",Std.mtof(62));
           }
           else if(msg.which == 8){ 
               string.v("freq",Std.mtof(63));
           }
           else if(msg.which == 7){ 
               string.v("freq",Std.mtof(64));
           }
           else if(msg.which == 9){ 
               string.v("freq",Std.mtof(65));
           }
           else if(msg.which == 23){ 
               string.v("freq",Std.mtof(66));
           }
           else if(msg.which == 10){ 
               string.v("freq",Std.mtof(67));
           }
           else if(msg.which == 28){ 
               string.v("freq",Std.mtof(68));
           }
           else if(msg.which == 11){ 
               string.v("freq",Std.mtof(69));
           }
           else if(msg.which == 24){ 
               string.v("freq",Std.mtof(70));
           }
           else if(msg.which == 13){ 
               string.v("freq",Std.mtof(71));
           }
           else if(msg.which == 14){ 
               string.v("freq",Std.mtof(72));
           }
           
        }
        
        else
        {
            string.v("gate",0);
            string.v("feedback",0);
        }
        
        
    }
}