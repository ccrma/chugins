//***********************************************************************************************
//Relativistic Oscillator module for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from the Fundamental plugins by Andrew Belt and graphics  
//  from the Component Library. 
//Also based on the BogAudio FM-OP oscillator by Matt Demanett
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//***********************************************************************************************


#include "Geodesics.hpp"
#include "EnergyOsc.hpp"


struct Energy : Module {
	enum ParamIds {
		ENUMS(PLANCK_PARAMS, 2),// push buttons
		ENUMS(MODTYPE_PARAMS, 2),// push buttons
		ROUTING_PARAM,// push button
		ENUMS(FREQ_PARAMS, 2),// rotary knobs (middle)
		ENUMS(MOMENTUM_PARAMS, 2),// rotary knobs (top)
		CROSS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(FREQCV_INPUTS, 2), // respectively "mass" and "speed of light"
		FREQCV_INPUT, // main voct input
		MULTIPLY_INPUT,
		ENUMS(MOMENTUM_INPUTS, 2),
		NUM_INPUTS
	};
	enum OutputIds {
		ENERGY_OUTPUT,// main output
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(PLANCK_LIGHTS, 2 * 3), // room for two Blue Yellow White leds
		ENUMS(ADD_LIGHTS, 2),
		ENUMS(AMP_LIGHTS, 2),
		ENUMS(ROUTING_LIGHTS, 3),
		ENUMS(MOMENTUM_LIGHTS, 2),
		ENUMS(FREQ_ROUTING_LIGHTS, 2 * 2),// room for blue/yellow
		CROSS_LIGHT,
		NUM_LIGHTS
	};
	
	
	// Constants
	static const int N_POLY = 16;
	
	// Need to save, no reset
	int panelTheme;
	
	// Need to save, with reset
	std::vector<FMOp> oscM;// size N_POLY
	std::vector<FMOp> oscC;// size N_POLY
	int routing;// routing of knob 1. 
		// 0 is independant (i.e. blue only) (bottom light, light index 0),
		// 1 is control (i.e. blue and yellow) (top light, light index 1),
		// 2 is spread (i.e. blue and inv yellow) (middle, light index 2)
	int plancks[2];// index is left/right, value is: 0 = not quantized, 1 = semitones, 2 = 5th+octs, 3 = adds -10V offset
	int modtypes[2];// index is left/right, value is: {0 to 3} = {bypass, add, amp}
	int cross;// cross momentum active or not
	
	// No need to save, with reset
	int numChan;
	float feedbacks[2][N_POLY];
	float modSignals[2][N_POLY];
	
	// No need to save, no reset
	RefreshCounter refresh;
	Trigger routingTrigger;
	Trigger planckTriggers[2];
	Trigger modtypeTriggers[2];
	Trigger crossTrigger;
	SlewLimiter multiplySlewers[N_POLY];
	
	
	Energy() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configButton(CROSS_PARAM, "Momentum crossing");		
		configParam(MOMENTUM_PARAMS + 0, 0.0f, 1.0f, 0.0f, "Momentum M");
		configParam(MOMENTUM_PARAMS + 1, 0.0f, 1.0f, 0.0f, "Momentum C");
		configParam(FREQ_PARAMS + 0, -3.0f, 3.0f, 0.0f, "Freq M");
		configParam(FREQ_PARAMS + 1, -3.0f, 3.0f, 0.0f, "Freq C");
		configButton(ROUTING_PARAM, "Routing");
		configButton(PLANCK_PARAMS + 0, "Quantize (Planck) M");
		configButton(PLANCK_PARAMS + 1, "Quantize (Planck) C");
		configButton(MODTYPE_PARAMS + 0, "CV mod type M");
		configButton(MODTYPE_PARAMS + 1, "CV mod type C");		
		
		configInput(FREQCV_INPUTS + 0, "Mass");
		configInput(FREQCV_INPUTS + 1, "Speed of light");
		configInput(FREQCV_INPUT, "1V/oct");
		configInput(MULTIPLY_INPUT, "Multiply");
		configInput(MOMENTUM_INPUTS + 0, "Momentum M");
		configInput(MOMENTUM_INPUTS + 1, "Momentum C");
		
		configOutput(ENERGY_OUTPUT, "Energy");
		
		oscM.reserve(N_POLY);
		oscC.reserve(N_POLY);
		for (int c = 0; c < N_POLY; c++) {
			oscM.push_back(FMOp(APP->engine->getSampleRate()));
			oscC.push_back(FMOp(APP->engine->getSampleRate()));
			feedbacks[0][c] = 0.0f;
			feedbacks[1][c] = 0.0f;
		}
		onSampleRateChange();
		onReset();

		panelTheme = loadDarkAsDefault();
	}
	
	
	void onReset() override final {
		for (int c = 0; c < N_POLY; c++) {
			oscM[c].onReset();
			oscC[c].onReset();
		}
		routing = 1;// default is control (i.e. blue and yellow) (top light, light index 1),
		for (int i = 0; i < 2; i++) {
			plancks[i] = 0;
			modtypes[i] = 1;// default is add mode
		}
		cross = 0;
		resetNonJson();
	}
	void resetNonJson() {
		numChan = 1;
		for (int c = 0; c < N_POLY; c++) {
			calcModSignals(c);
			calcFeedbacks(c);
		}			
	}	

	
	void onRandomize() override {
	}
	

	void onSampleRateChange() override final {
		float sampleRate = APP->engine->getSampleRate();
		for (int c = 0; c < N_POLY; c++) {
			oscM[c].onSampleRateChange(sampleRate);
			oscC[c].onSampleRateChange(sampleRate);
			multiplySlewers[c].setParams2(sampleRate, 2.5f, 20.0f, 1.0f);
		}
	}
	
	
	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		// panelTheme
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

		// oscM and oscC
		oscM[0].dataToJson(rootJ, "oscM_");// legacy so do outside loop
		oscC[0].dataToJson(rootJ, "oscC_");// legacy so do outside loop
		for (int c = 1; c < N_POLY; c++) {
			oscM[c].dataToJson(rootJ, string::f("osc%iM_",c));
			oscC[c].dataToJson(rootJ, string::f("osc%iC_",c));
		}

		// routing
		json_object_set_new(rootJ, "routing", json_integer(routing));

		// plancks
		json_object_set_new(rootJ, "planck0", json_integer(plancks[0]));
		json_object_set_new(rootJ, "planck1", json_integer(plancks[1]));

		// modtypes
		json_object_set_new(rootJ, "modtype0", json_integer(modtypes[0]));
		json_object_set_new(rootJ, "modtype1", json_integer(modtypes[1]));
		
		// cross
		json_object_set_new(rootJ, "cross", json_integer(cross));

		return rootJ;
	}

	
	void dataFromJson(json_t *rootJ) override {
		// panelTheme
		json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
		if (panelThemeJ)
			panelTheme = json_integer_value(panelThemeJ);

		// oscM and oscC
		oscM[0].dataFromJson(rootJ, "oscM_");// legacy so do outside loop
		oscC[0].dataFromJson(rootJ, "oscC_");// legacy so do outside loop
		for (int c = 1; c < N_POLY; c++) {
			oscM[c].dataFromJson(rootJ, string::f("osc%iM_",c));
			oscC[c].dataFromJson(rootJ, string::f("osc%iC_",c));
		}

		// routing
		json_t *routingJ = json_object_get(rootJ, "routing");
		if (routingJ)
			routing = json_integer_value(routingJ);

		// plancks
		json_t *planck0J = json_object_get(rootJ, "planck0");
		if (planck0J)
			plancks[0] = json_integer_value(planck0J);
		json_t *planck1J = json_object_get(rootJ, "planck1");
		if (planck1J)
			plancks[1] = json_integer_value(planck1J);

		// modtypes
		json_t *modtype0J = json_object_get(rootJ, "modtype0");
		if (modtype0J)
			modtypes[0] = json_integer_value(modtype0J);
		json_t *modtype1J = json_object_get(rootJ, "modtype1");
		if (modtype1J)
			modtypes[1] = json_integer_value(modtype1J);

		// cross
		json_t *crossJ = json_object_get(rootJ, "cross");
		if (crossJ)
			cross = json_integer_value(crossJ);
		
		resetNonJson();
	}

	void process(const ProcessArgs &args) override {	
		// user inputs
		if (refresh.processInputs()) {
			numChan = std::max(1, inputs[FREQCV_INPUT].getChannels());
			numChan = std::min(numChan, (int)N_POLY);
			outputs[ENERGY_OUTPUT].setChannels(numChan);

			// routing
			if (routingTrigger.process(params[ROUTING_PARAM].getValue())) {
				if (++routing > 2)
					routing = 0;
			}
			
			// plancks
			for (int i = 0; i < 2; i++) {
				if (planckTriggers[i].process(params[PLANCK_PARAMS + i].getValue())) {
					if (++plancks[i] > 3)
						plancks[i] = 0;
				}
			}
			
			// modtypes
			for (int i = 0; i < 2; i++) {
				if (modtypeTriggers[i].process(params[MODTYPE_PARAMS + i].getValue())) {
					if (++modtypes[i] > 2)
						modtypes[i] = 0;
				}
			}
			
			// cross
			if (crossTrigger.process(params[CROSS_PARAM].getValue())) {
				if (++cross > 1)
					cross = 0;
			}
		}// userInputs refresh
		
		
		// main signal flow
		// ----------------		
		for (int c = 0; c < numChan; c++) {
			// pitch modulation and feedbacks
			if ((refresh.refreshCounter & 0x3) == (c & 0x3)) {
				// stagger0 updates channels 0, 4, 8,  12
				// stagger1 updates channels 1, 5, 9,  13
				// stagger2 updates channels 2, 6, 10, 14
				// stagger3 updates channels 3, 7, 11, 15
				calcModSignals(c);// voct modulation, a given channel is updated at sample_rate / 4
				calcFeedbacks(c);// feedback (momentum), a given channel is updated at sample_rate / 4
			}
			
			if (!outputs[ENERGY_OUTPUT].isConnected()) {// this is placed here such that feedbacks and mod signals of chan 0 are always calculated, since they are used in lights
				break;
			}
			
			// vocts
			const float vocts[2] = {modSignals[0][c] + inputs[FREQCV_INPUT].getVoltage(c), modSignals[1][c] + inputs[FREQCV_INPUT].getVoltage(c)};
			
			// oscillators
			float oscMout = oscM[c].step(vocts[0], feedbacks[0][c] * 0.3f);
			float oscCout = oscC[c].step(vocts[1], feedbacks[1][c] * 0.3f);
			
			// multiply 
			float slewInput = 1.0f;
			if (inputs[MULTIPLY_INPUT].isConnected()) {
				int chan = std::min(inputs[MULTIPLY_INPUT].getChannels() - 1, c);
				slewInput = (clamp(inputs[MULTIPLY_INPUT].getVoltage(chan) / 10.0f, 0.0f, 1.0f));
			}
			float multiplySlewValue = multiplySlewers[c].next(slewInput) * 0.2f;
			
			// final attenuverters
			float attv1 = oscCout * oscCout * multiplySlewValue;
			float attv2 = attv1 * oscMout * 0.2f;
			
			// output
			outputs[ENERGY_OUTPUT].setVoltage(attv2, c);
		}

		// lights
		if (refresh.processLights()) {
			// routing
			for (int i = 0; i < 3; i++)
				lights[ROUTING_LIGHTS + i].setBrightness(routing == i ? 1.0f : 0.0f);
			
			for (int i = 0; i < 2; i++) {
				// plancks (was white/blue/red), now BlueYellowWhite
				lights[PLANCK_LIGHTS + i * 3 + 2].setBrightness(plancks[i] == 1 ? 1.0f : 0.0f);// white
				lights[PLANCK_LIGHTS + i * 3 + 0].setBrightness(plancks[i] == 2 ? 1.0f : 0.0f);// blue
				lights[PLANCK_LIGHTS + i * 3 + 1].setBrightness(plancks[i] == 3 ? 1.0f : 0.0f);// yellow (was red)
				
				// modtypes
				lights[ADD_LIGHTS + i].setBrightness(modtypes[i] == 1 ? 1.0f : 0.0f);
				lights[AMP_LIGHTS + i].setBrightness(modtypes[i] == 2 ? 1.0f : 0.0f);
				
				// momentum (cross)
				lights[MOMENTUM_LIGHTS + i].setBrightness(feedbacks[i][0]);// lights show first channel only when poly

				// freq
				float modSignalLight = modSignals[i][0] / 3.0f;
				lights[FREQ_ROUTING_LIGHTS + 2 * i + 0].setBrightness(modSignalLight);// blue diode
				lights[FREQ_ROUTING_LIGHTS + 2 * i + 1].setBrightness(-modSignalLight);// yellow diode
			}
			
			// cross
			lights[CROSS_LIGHT].setBrightness(cross == 1 ? 1.0f : 0.0f);

		}// lightRefreshCounter
		
	}// step()
	
	inline float calcFreqKnob(int osci) {
		if (plancks[osci] == 0)// off (smooth)
			return params[FREQ_PARAMS + osci].getValue();
		if (plancks[osci] == 1)// semitones
			return std::round(params[FREQ_PARAMS + osci].getValue() * 12.0f) / 12.0f;
		if (plancks[osci] == 3)// -10V offset
			return params[FREQ_PARAMS + osci].getValue() - 10.0f;
		// 5ths and octs (plancks[osci] == 2)
		int retcv = (int)std::round((params[FREQ_PARAMS + osci].getValue() + 3.0f) * 2.0f);
		if ((retcv & 0x1) != 0)
			return (float)(retcv)/2.0f - 3.0f + 0.08333333333f;
		return (float)(retcv)/2.0f - 3.0f;
	}
	
	inline void calcModSignals(int chan) {
		for (int osci = 0; osci < 2; osci++) {
			float freqValue = calcFreqKnob(osci);
			if (modtypes[osci] == 0 || !inputs[FREQCV_INPUTS + osci].isConnected()) {// bypass
				modSignals[osci][chan] = freqValue;
			}
			else {
				int chanIn = std::min(inputs[FREQCV_INPUTS + osci].getChannels() - 1, chan);
				if (modtypes[osci] == 1) {// add
					modSignals[osci][chan] = freqValue + inputs[FREQCV_INPUTS + osci].getVoltage(chanIn);
				}
				else {// amp
					modSignals[osci][chan] = freqValue * (clamp(inputs[FREQCV_INPUTS + osci].getVoltage(chanIn), 0.0f, 10.0f) / 10.0f);
				}
			}
		}
		if (routing == 1) {
			modSignals[1][chan] += modSignals[0][chan];
		}
		else if (routing == 2) {
			modSignals[1][chan] -= modSignals[0][chan];
		}
	}
	
	inline void calcFeedbacks(int chan) {
		float moIn[2]; 	
		for (int osci = 0; osci < 2; osci++) {
			moIn[osci] = 0.0f;
			if (inputs[MOMENTUM_INPUTS + osci].isConnected()) {
				int chanIn = std::min(inputs[MOMENTUM_INPUTS + osci].getChannels() - 1, chan);
				moIn[osci] = inputs[MOMENTUM_INPUTS + osci].getVoltage(chanIn);
			}
			feedbacks[osci][chan] = params[MOMENTUM_PARAMS + osci].getValue();
		}
		
		if (cross == 0) {
			feedbacks[0][chan] += moIn[0] * 0.1f;
			feedbacks[1][chan] += moIn[1] * 0.1f;
		}
		else {// cross momentum
			if (moIn[0] > 0)
				feedbacks[0][chan] += moIn[0] * 0.2f;
			else 
				feedbacks[1][chan] += moIn[0] * -0.2f;
			if (moIn[1] > 0)
				feedbacks[1][chan] += moIn[1] * 0.2f;
			else 
				feedbacks[0][chan] += moIn[1] * -0.2f;
		}
		feedbacks[0][chan] = clamp(feedbacks[0][chan], 0.0f, 1.0f);
		feedbacks[1][chan] = clamp(feedbacks[1][chan], 0.0f, 1.0f);
	}
};


struct EnergyWidget : ModuleWidget {
	int lastPanelTheme = -1;
	std::shared_ptr<window::Svg> light_svg;
	std::shared_ptr<window::Svg> dark_svg;

	void appendContextMenu(Menu *menu) override {
		Energy *module = static_cast<Energy*>(this->module);
		assert(module);

		createPanelThemeMenu(menu, &(module->panelTheme));
	}	
	
	EnergyWidget(Energy *module) {
		setModule(module);

		// Main panels from Inkscape
 		light_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteLight/Energy-WL.svg"));
		dark_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/DarkMatter/Energy-DM.svg"));
		int panelTheme = isDark(module ? (&((static_cast<Energy*>(module))->panelTheme)) : NULL) ? 1 : 0;// need this here since step() not called for module browser
		setPanel(panelTheme == 0 ? light_svg : dark_svg);		

		// Screws 
		// part of svg panel, no code required
		
		float colRulerCenter = box.size.x / 2.0f;
		static const float offsetX = 30.0f;

		// main output
		addOutput(createDynamicPort<GeoPort>(VecPx(colRulerCenter, 380.0f - 332.5f), false, module, Energy::ENERGY_OUTPUT, module ? &module->panelTheme : NULL));

		// multiply input
		addInput(createDynamicPort<GeoPort>(VecPx(colRulerCenter, 380.0f - 280.5f), true, module, Energy::MULTIPLY_INPUT, module ? &module->panelTheme : NULL));
		
		// momentum inputs
		addInput(createDynamicPort<GeoPort>(VecPx(colRulerCenter, 380.0f - 236.5f), true, module, Energy::MOMENTUM_INPUTS + 1, module ? &module->panelTheme : NULL));
		addInput(createDynamicPort<GeoPort>(VecPx(colRulerCenter, 380.0f - 181.5f), true, module, Energy::MOMENTUM_INPUTS + 0, module ? &module->panelTheme : NULL));
		
		// cross button and light
		addParam(createDynamicParam<GeoPushButton>(VecPx(colRulerCenter, 380.0f - 205.5f), module, Energy::CROSS_PARAM, module ? &module->panelTheme : NULL));		
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(colRulerCenter - 7.5f, 380.0f - 219.5f), module, Energy::CROSS_LIGHT));
		
		// routing lights
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(39, 380.0f - 141.5f), module, Energy::ROUTING_LIGHTS + 0));// bottom
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(51, 380.0f - 154.5f), module, Energy::ROUTING_LIGHTS + 1));// top
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(45, 380.0f - 148.5f), module, Energy::ROUTING_LIGHTS + 2));// middle
		
		// momentum knobs
		addParam(createDynamicParam<GeoKnob>(VecPx(colRulerCenter - offsetX, 380 - 209), module, Energy::MOMENTUM_PARAMS + 0, module ? &module->panelTheme : NULL));
		addParam(createDynamicParam<GeoKnob>(VecPx(colRulerCenter + offsetX, 380 - 209), module, Energy::MOMENTUM_PARAMS + 1, module ? &module->panelTheme : NULL));
		
		// momentum lights
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(colRulerCenter - offsetX, 380.0f - 186.0f), module, Energy::MOMENTUM_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(colRulerCenter + offsetX, 380.0f - 186.0f), module, Energy::MOMENTUM_LIGHTS + 1));

		// freq routing lights (below momentum lights)
		addChild(createLightCentered<SmallLight<GeoBlueYellowLight>>(VecPx(colRulerCenter - offsetX, 380.0f - 177.0f), module, Energy::FREQ_ROUTING_LIGHTS + 2 * 0));
		addChild(createLightCentered<SmallLight<GeoBlueYellowLight>>(VecPx(colRulerCenter + offsetX, 380.0f - 177.0f), module, Energy::FREQ_ROUTING_LIGHTS + 2 * 1));

		// freq knobs
		addParam(createDynamicParam<GeoKnob>(VecPx(colRulerCenter - offsetX, 380 - 126), module, Energy::FREQ_PARAMS + 0, module ? &module->panelTheme : NULL));
		addParam(createDynamicParam<GeoKnob>(VecPx(colRulerCenter + offsetX, 380 - 126), module, Energy::FREQ_PARAMS + 1, module ? &module->panelTheme : NULL));
		
		// routing button
		addParam(createDynamicParam<GeoPushButton>(VecPx(colRulerCenter, 380.0f - 113.5f), module, Energy::ROUTING_PARAM, module ? &module->panelTheme : NULL));
		
		// freq input (v/oct)
		addInput(createDynamicPort<GeoPort>(VecPx(colRulerCenter, 380 - 84), true, module, Energy::FREQCV_INPUT, module ? &module->panelTheme : NULL));

		// planck buttons
		addParam(createDynamicParam<GeoPushButton>(VecPx(colRulerCenter - offsetX - 0.5f, 380.0f - 83.5f), module, Energy::PLANCK_PARAMS + 0, module ? &module->panelTheme : NULL));
		addParam(createDynamicParam<GeoPushButton>(VecPx(colRulerCenter + offsetX + 0.5f, 380.0f - 83.5f), module, Energy::PLANCK_PARAMS + 1, module ? &module->panelTheme : NULL));
		
		// planck lights
		addChild(createLightCentered<SmallLight<GeoBlueYellowWhiteLight>>(VecPx(colRulerCenter - offsetX - 0.5f, 380.0f - 97.5f), module, Energy::PLANCK_LIGHTS + 0 * 3));
		addChild(createLightCentered<SmallLight<GeoBlueYellowWhiteLight>>(VecPx(colRulerCenter + offsetX + 0.5f, 380.0f - 97.5f), module, Energy::PLANCK_LIGHTS + 1 * 3));
		
		// mod type buttons
		addParam(createDynamicParam<GeoPushButton>(VecPx(colRulerCenter - offsetX - 0.5f, 380.0f - 57.5f), module, Energy::MODTYPE_PARAMS + 0, module ? &module->panelTheme : NULL));
		addParam(createDynamicParam<GeoPushButton>(VecPx(colRulerCenter + offsetX + 0.5f, 380.0f - 57.5f), module, Energy::MODTYPE_PARAMS + 1, module ? &module->panelTheme : NULL));
		
		// mod type lights
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(colRulerCenter - 17.5f, 380.0f - 62.5f), module, Energy::ADD_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(colRulerCenter + 17.5f, 380.0f - 62.5f), module, Energy::ADD_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(colRulerCenter - 41.5f, 380.0f - 47.5f), module, Energy::AMP_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GeoWhiteLight>>(VecPx(colRulerCenter + 41.5f, 380.0f - 47.5f), module, Energy::AMP_LIGHTS + 1));
		
		// freq inputs (mass and speed of light)
		addInput(createDynamicPort<GeoPort>(VecPx(colRulerCenter - offsetX - 0.5f, 380.0f - 32.5f), true, module, Energy::FREQCV_INPUTS + 0, module ? &module->panelTheme : NULL));
		addInput(createDynamicPort<GeoPort>(VecPx(colRulerCenter + offsetX + 0.5f, 380.0f - 32.5f), true, module, Energy::FREQCV_INPUTS + 1, module ? &module->panelTheme : NULL));
	}
	
	void step() override {
		int panelTheme = isDark(module ? (&((static_cast<Energy*>(module))->panelTheme)) : NULL) ? 1 : 0;
		if (lastPanelTheme != panelTheme) {
			lastPanelTheme = panelTheme;
			SvgPanel* panel = static_cast<SvgPanel*>(getPanel());
			panel->setBackground(panelTheme == 0 ? light_svg : dark_svg);
			panel->fb->dirty = true;
		}
		Widget::step();
	}
};

Model *modelEnergy = createModel<Energy, EnergyWidget>("Energy");