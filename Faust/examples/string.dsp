import("stdfaust.lib");

frequency = hslider("freq",440,51,2000,0.01);
gain = hslider("gain",1,0,1,0.01);
feedback = hslider("feedback",0.995,0,1,0.01);
trig = button("gate");
beta = hslider("pick_position", 0.13, 0.02, 0.5, 0.01);
stringsFadeTime = hslider("fadeTime",2,0,50,0.1)*0.001;


fadeDelay(n,d,tau) = _ <: de.fdelay1(n,delLength1)*(gsmooth),de.fdelay1(n,delLength2)*(1-gsmooth) :> _
with{
	swit(t) = +(t)~_ : %(2);
	g = (d != d') : swit; //'
	gsmooth = g : si.smooth(ma.tau2pole(tau));
	hold(t,x) = (_*(t)+(x'*(t != t'))~_)*t + x*(1-t);
	delLength1 = hold(1-g,d) : min(15000) : max(1);
	delLength2 = hold(g,d) : min(15000) : max(1);
};

myString(freq,feedback) = +~(de.fdelay4(1024,delLength) <: (_+_')/2 : *(feedback))
//myString(freq,feedback) = +~(fadeDelay(1024,delLength,stringsFadeTime) <: (_+_')/2 : *(feedback))
with{
	delLength = ma.SR/freq - 1;
};

noiseburst(g,P) = no.noise : *(g : trigger(P))*gain : pickposfilter
with {
  diffgtz(x) = (x-x') > 0;
  decay(n,x) = x - (x>0)/n;
  release(n) = + ~ decay(n);
  trigger(n) = diffgtz : release(n) : > (0.0);
  ppdel = beta*P; // pick position delay
  pickposfilter = fi.ffcombfilter(4096,ppdel,-1); // defined in filter.lib
};

process = noiseburst(trig,(ma.SR/frequency)) : fi.lowpass(3,frequency) : myString(frequency,feedback);










