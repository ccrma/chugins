declare name "BrassFaust";
declare description "Simple brass instrument physical model with physical parameters.";
declare license "MIT";
declare copyright "(c)Romain Michon, CCRMA (Stanford University), GRAME";

import("stdfaust.lib");
	fb     = hslider("fb", 0, -0.999, 0.999, 0.001);
process = fb , pm.brass_ui <: _,_;
