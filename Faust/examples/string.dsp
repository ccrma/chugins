import("music.lib");
import("filter.lib");
import("effect.lib");

frequency = hslider("freq",440,51,2000,0.01);
gain = hslider("gain",1,0,1,0.01);
feedback = hslider("feedback",0.995,0.9,1,0.01);
trig = button("gate");
beta = hslider("pick_position", 0.13, 0.02, 0.5, 0.01);

myString(freq,feedback) = +~(fdelay4(1024,delLength) <: (_+_')/2 : *(feedback))
with{
	delLength = SR/freq - 1;
};

noiseburst(g,P) = noise : *(g : trigger(P))*gain : pickposfilter
with {
  diffgtz(x) = (x-x') > 0;
  decay(n,x) = x - (x>0)/n;
  release(n) = + ~ decay(n);
  trigger(n) = diffgtz : release(n) : > (0.0);
  ppdel = beta*P; // pick position delay
  pickposfilter = ffcombfilter(4096,ppdel,-1); // defined in filter.lib
};

process = noiseburst(trig,(SR/frequency)) : lowpass(2,frequency) : myString(frequency,feedback);










