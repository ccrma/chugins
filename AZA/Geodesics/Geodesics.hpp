//***********************************************************************************************
//Geodesics: A modular collection for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from the Fundamental plugins by Andrew Belt 
//  and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//***********************************************************************************************


#pragma once

#include "rack.hpp"
#include "GeoWidgets.hpp"


using namespace rack;


extern Plugin *pluginInstance;


// All modules that are part of plugin go here
extern Model *modelBlackHoles;
extern Model *modelPulsars;
extern Model *modelBranes;
extern Model *modelIons;
extern Model *modelEntropia;
extern Model *modelEnergy;
extern Model *modelDarkEnergy;
extern Model *modelTorus;
extern Model *modelFate;
// extern Model *modelTwinParadox;
extern Model *modelBlankLogo;
extern Model *modelBlankInfo;



// General constants
//static const bool retrigGatesOnReset = true; no need yet, since no geodesic sequencers emit gates
static constexpr float clockIgnoreOnResetDuration = 0.001f;// disable clock on powerup and reset for 1 ms (so that the first step plays)

static const float blurRadiusRatio = 0.06f;


// Variations on existing knobs, lights, etc


// Ports

struct GeoPort : DynamicSVGPort {
	GeoPort() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteLight/Jack-WL.svg")));
		addFrameAlt(asset::plugin(pluginInstance, "res/DarkMatter/Jack-DM.svg"));
		shadow->blurRadius = 1.0f;
	}
};


struct BlankPort : SvgPort {
	BlankPort() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/PJ301M.svg")));
		shadow->opacity = 0.0;
		sw->setVisible(false);
	}
};



// Buttons and switches

struct GeoPushButton : DynamicSVGSwitch {
	GeoPushButton() {
		momentary = true;
		addFrameAll(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteLight/PushButton1_0.svg")));
		addFrameAll(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteLight/PushButton1_1.svg")));
		addFrameAlt0(asset::plugin(pluginInstance, "res/DarkMatter/PushButton1_0.svg"));
		addFrameAlt1(asset::plugin(pluginInstance, "res/DarkMatter/PushButton1_1.svg"));
	}
};



// Knobs

struct GeoKnob : DynamicSVGKnob {
	GeoKnob() {
		minAngle = -0.73 * float(M_PI);
		maxAngle = 0.73 * float(M_PI);
		//shadow->box.pos = Vec(0.0, box.size.y * 0.15); may need this if knob is small (taken from IMSmallKnob)
		addFrameAll(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteLight/Knob-WL.svg")));
		addFrameAlt(asset::plugin(pluginInstance, "res/DarkMatter/Knob-DM.svg"));
		
		addFrameBgAll(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteLight/Knob-WL_bg.svg")));
		addFrameBgAlt(asset::plugin(pluginInstance, "res/DarkMatter/Knob-DM_bg.svg"));
		
		addFrameFgAll(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteLight/Knob-WL_fg.svg")));
		addFrameFgAlt(asset::plugin(pluginInstance, "res/DarkMatter/Knob-DM_fg.svg"));
		
		shadow->blurRadius = box.size.y * blurRadiusRatio;
	}
};

struct GeoKnobTopRight : GeoKnob {
	GeoKnobTopRight() {setOrientation(float(M_PI) / 4.0f);}
};
struct GeoKnobRight : GeoKnob {
	GeoKnobRight() {setOrientation(float(M_PI) / 2.0f);}
};
struct GeoKnobBotRight : GeoKnob {
	GeoKnobBotRight() {setOrientation(3.0f * float(M_PI) / 4.0f);}
};
struct GeoKnobBottom : GeoKnob {
	GeoKnobBottom() {setOrientation(float(M_PI));}
};
struct GeoKnobBotLeft : GeoKnob {
	GeoKnobBotLeft() {setOrientation(5.0f * float(M_PI) / 4.0f);}
};
struct GeoKnobLeft : GeoKnob {
	GeoKnobLeft() {setOrientation(float(M_PI) / -2.0f);}
};
struct GeoKnobTopLeft : GeoKnob {
	GeoKnobTopLeft() {setOrientation(float(M_PI) / -4.0f);}
};


struct BlankCKnob : DynamicSVGKnob {
	BlankCKnob() {
		minAngle = -0.73 * float(M_PI);
		maxAngle = 0.73 * float(M_PI);
		addFrameAll(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteLight/C-WL.svg")));
		addFrameAlt(asset::plugin(pluginInstance, "res/DarkMatter/C-DM.svg"));
		shadow->opacity = 0.0;
	}
};



// Lights

struct GeoGrayModuleLight : ModuleLightWidget {
	GeoGrayModuleLight() {
		bgColor = nvgRGBA(0x33, 0x33, 0x33, 0xff);
		borderColor = nvgRGBA(0, 0, 0, 53);
	}
	
	void drawLight(const DrawArgs &args) override { // from app/LightWidget.cpp (only nvgStrokeWidth of border was changed)
		float radius = box.size.x / 2.0;

		nvgBeginPath(args.vg);
		nvgCircle(args.vg, radius, radius, radius);

		// Background
		if (bgColor.a > 0.0) {
			nvgFillColor(args.vg, bgColor);
			nvgFill(args.vg);
		}

		// Foreground
		if (color.a > 0.0) {
			nvgFillColor(args.vg, color);
			nvgFill(args.vg);
		}

		// Border
		if (borderColor.a > 0.0) {
			nvgStrokeWidth(args.vg, 1.0);//0.5);
			nvgStrokeColor(args.vg, borderColor);
			nvgStroke(args.vg);
		}
	}
};

struct GeoWhiteLight : GeoGrayModuleLight {
	GeoWhiteLight() {
		addBaseColor(SCHEME_WHITE);
	}
};
struct GeoBlueLight : GeoGrayModuleLight {
	GeoBlueLight() {
		addBaseColor(SCHEME_BLUE);
	}
};
struct GeoRedLight : GeoGrayModuleLight {
	GeoRedLight() {
		addBaseColor(SCHEME_RED);
	}
};
struct GeoYellowLight : GeoGrayModuleLight {
	GeoYellowLight() {
		addBaseColor(SCHEME_YELLOW);
	}
};

struct GeoWhiteBlueLight : GeoGrayModuleLight {
	GeoWhiteBlueLight() {
		addBaseColor(SCHEME_WHITE);
		addBaseColor(SCHEME_BLUE);
	}
};
struct GeoBlueYellowLight : GeoGrayModuleLight {
	GeoBlueYellowLight() {
		addBaseColor(SCHEME_BLUE);
		addBaseColor(SCHEME_YELLOW);
	}
};

struct GeoBlueYellowWhiteLight : GeoGrayModuleLight {
	GeoBlueYellowWhiteLight() {
		addBaseColor(SCHEME_BLUE);
		addBaseColor(SCHEME_YELLOW);
		addBaseColor(SCHEME_WHITE);
	}
};

struct GeoBlueYellowRedWhiteLight : GeoGrayModuleLight {
	GeoBlueYellowRedWhiteLight() {
		addBaseColor(SCHEME_BLUE);
		addBaseColor(SCHEME_YELLOW);
		addBaseColor(SCHEME_RED);
		addBaseColor(SCHEME_WHITE);
	}
};


// Other

struct VecPx : Vec {
	// temporary method to avoid having to convert all px coordinates to mm; no use when making a new module (since mm is the standard)
	static constexpr float scl = 5.08f / 15.0f;
	VecPx(float _x, float _y) {
		x = mm2px(_x * scl);
		y = mm2px(_y * scl);
	}
};


struct RefreshCounter {
	static const unsigned int displayRefreshStepSkips = 256;
	static const unsigned int userInputsStepSkipMask = 0xF;// sub interval of displayRefreshStepSkips, since inputs should be more responsive than lights
	// above value should make it such that inputs are sampled > 1kHz so as to not miss 1ms triggers
	
	unsigned int refreshCounter = (random::u32() % displayRefreshStepSkips);// stagger start values to avoid processing peaks when many Geo and Impromptu modules in the patch
	
	bool processInputs() {
		return ((refreshCounter & userInputsStepSkipMask) == 0);
	}
	bool processLights() {// this must be called even if module has no lights, since counter is decremented here
		refreshCounter++;
		bool process = refreshCounter >= displayRefreshStepSkips;
		if (process) {
			refreshCounter = 0;
		}
		return process;
	}
};


struct Trigger {
	bool state = true;

	void reset() {
		state = true;
	}
	
	bool isHigh() {
		return state;
	}
	
	bool process(float in) {
		if (state) {
			// HIGH to LOW
			if (in <= 0.1f) {
				state = false;
			}
		}
		else {
			// LOW to HIGH
			if (in >= 0.9f) {
				state = true;
				return true;
			}
		}
		return false;
	}	
};	


struct TriggerRiseFall {
	bool state = false;

	void reset() {
		state = false;
	}

	int process(float in) {
		if (state) {
			// HIGH to LOW
			if (in <= 0.1f) {
				state = false;
				return -1;
			}
		}
		else {
			// LOW to HIGH
			if (in >= 0.9f) {
				state = true;
				return 1;
			}
		}
		return 0;
	}	
};	


struct HoldDetect {
	long modeHoldDetect = 0l;// 0 when not detecting, downward counter when detecting
	
	void reset() {
		modeHoldDetect = 0l;
	}
	
	void start(long startValue) {
		modeHoldDetect = startValue;
	}

	bool process(float paramValue) {
		bool ret = false;
		if (modeHoldDetect > 0l) {
			if (paramValue < 0.5f)
				modeHoldDetect = 0l;
			else {// button held long enough
				if (modeHoldDetect == 1l) {
					ret = true;
				}
				modeHoldDetect--;
			}
		}
		return ret;
	}
};

int getWeighted1to8random();

