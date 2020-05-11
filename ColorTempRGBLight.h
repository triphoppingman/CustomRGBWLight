#include "esphome.h"

// Mode of the light
enum mode {RGB, White};

// Constants
const float maxMired = 500;
const float minMired = 153;

/**
 * Map a RGBW Light into a tunable color / RGB Light 
 * 
 * https://gist.github.com/madjam002/31cc88640efa370630fed6914fa4eb7f
 */
class ColorTempRGBLight : public Component, public LightOutput {
public:

  ColorTempRGBLight(
    FloatOutput  *red, 
    FloatOutput  *green,  
    FloatOutput  *blue 
  ) {
    red_ = red;
    green_ = green;
    blue_ = blue;
	
    // Create initial state
    colorTemp_ = -1.0f;
    brightness_ = -1.0f;
    mode_ = White;
    
    // default scaling
    cr = 1;
    br = 0;
    cg = 0.6;
    bg = 0;
    cb = -1.5;
    bb = 0.5;
  }
 
  /**
   * Constructor with scaling provided.
   */
  ColorTempRGBLight(
    FloatOutput  *red, 
    FloatOutput  *green,  
    FloatOutput  *blue, 
    
    float _br,
    float _cr,
    float _bg,
    float _cg,
    float _bb,
    float _cb
  ) {
    red_ = red;
    green_ = green;
    blue_ = blue;

    // Create initial state
    colorTemp_ = -1.0f;
    brightness_ = -1.0f;
    mode_ = White;
    
    // Configured scaling
    cr = _cr;
    br = _br;
    cg = _cg;
    bg = _bg;
    cb = _cb;
    bb = _bb;
  }

  /** 
   * Get the traits for this light 
   */
  LightTraits get_traits() override {
    auto traits = LightTraits();
    
    traits.set_supports_brightness(true);
    traits.set_supports_rgb(true);
    traits.set_supports_rgb_white_value(false);
    traits.set_supports_color_temperature(true);
    traits.set_min_mireds(minMired); // home assistant minimum 153
    traits.set_max_mireds(maxMired); // home assistant maximum 500

    return traits;
  }

  /**
   * Write the state to this light
   */
  void write_state(LightState *state) override {
    // Get the current statue
    float colorTemp, brightness;
    state->current_values_as_brightness(&brightness);
    colorTemp = state->current_values.get_color_temperature();
    float red, green, blue;
    state->current_values_as_rgb(&red, &green, &blue);

    // Switch modes if rgb values have been sent or if color temp value has been sent
    if (colorTemp != colorTemp_) {
      mode_ = White;
    }
    else if (red != oldRed || green != oldGreen || blue != oldBlue) {
      mode_ = RGB;
    }
  
    // If the color temp changed or the color temp is unchanged but the brightness is changed then process that.
    if(mode_ == White) {
		// Normalize the colorTemp
		float xaxis = (colorTemp - minMired) / (maxMired - minMired);  // Varies from 0 to 1 as it moves from MIN to MAX

		// Place the rgb values on three lines
		float red = std::min(std::max((cr * xaxis + br) * brightness,0.0f), 1.0f);
		float green = std::min(std::max((cg * xaxis + bg) * brightness,0.0f), 1.0f);
		float blue = std::min(std::max((cb * xaxis + bb) * brightness,0.0f), 1.0f);

		this->red_->set_level(red);
		this->green_->set_level(green);
		this->blue_->set_level(blue);

    colorTemp_ = colorTemp;
	} else {
    float red, green, blue;
    state->current_values_as_rgb(&red, &green, &blue);

    if (
      red != state->current_values.get_red() ||
      green != state->current_values.get_green() ||
      blue != state->current_values.get_blue() ||
      brightness != state->current_values.get_brightness())
    {
      this->red_->set_level(red);
      this->green_->set_level(green);
      this->blue_->set_level(blue);
    }
	}
	brightness_ = brightness; 
  this->oldRed = red;
  this->oldGreen = green;
  this->oldBlue = blue;
}


protected:
  FloatOutput  *red_;
  FloatOutput  *green_;
  FloatOutput  *blue_;
  
  // Keep track of previous changes
  float oldRed;
  float oldGreen;
  float oldBlue;
  float colorTemp_;
  float brightness_;

  mode mode_;
  
  // red intercept and scale
  float br;
  float cr;
  
  // green intercept and scale
  float bg;
  float cg;
  
  // blue intercept and scale
  float bb;
  float cb;
};
