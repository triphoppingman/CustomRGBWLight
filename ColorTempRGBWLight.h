#include "esphome.h"

enum mode {RGB, White};

// Constants
const float maxMired = 500;
const float minMired = 153;

/**
 * Map a RGBW Light into a tunable color / RGB Light
 *
 * Lane Roberts
 *
 * derived from:
 * https://gist.github.com/madjam002/31cc88640efa370630fed6914fa4eb7f
 * https://gist.github.com/triphoppingman/76153ddf58072b10e229e9147b2bdf72
 */
class CustomRGBWLight : public Component, public LightOutput {
public:

  CustomRGBWLight(
    FloatOutput *red,
    FloatOutput *green,
    FloatOutput *blue,
    FloatOutput *white
  ) {
    red_output = red;
    green_output = green;
    blue_output = blue;
    white_output = white;

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
    cw = -0.7;
    bw = 1;
  }

  /**
   * Constructor with scaling provided.
   */
  CustomRGBWLight(
    FloatOutput *red,
    FloatOutput *green,
    FloatOutput *blue,
    FloatOutput *white,
    float _br,
    float _cr,
    float _bg,
    float _cg,
    float _bb,
    float _cb,
    float _bw,
    float _cw
  ) {
    red_output = red;
    green_output = green;
    blue_output = blue;
    white_output = white;

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
    cw = _cw;
    bw = _bw;
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
    float colorTemp, brightness;
    state->current_values_as_brightness(&brightness);
    colorTemp = state->current_values.get_color_temperature();
    float red, green, blue, cwhite, wwhite;
    state->current_values_as_rgbww(&red, &green, &blue, &cwhite, &wwhite);

    // Switch modes if rgb values have been sent or if color temp value has been sent
    if (colorTemp != colorTemp_)
    {
      mode_ = White;
    }
    else if (red != oldRed || green != oldGreen || blue != oldBlue)
    {
      mode_ = RGB;
    }
    if (brightness != brightness_ && (cwhite > 0.0f || wwhite > 0.0f))
    {
      mode_ = White;
    }
    // ESP_LOGD("custom", "  CWhite: %.2f | WWhite: %.2f ", cwhite, wwhite);

    if(mode_ == White) {
        // Normalize the colorTemp
        float xaxis = (colorTemp - minMired) / (maxMired - minMired);  // Varies from 0 to 1 as it moves from MIN to MAX

        // Place the rgb values on three lines
        float red = std::min(std::max((cr * xaxis + br) * brightness,0.0f), 1.0f);
        float green = std::min(std::max((cg * xaxis + bg) * brightness,0.0f), 1.0f);
        float blue = std::min(std::max((cb * xaxis + bb) * brightness,0.0f), 1.0f);
        float white = std::min(std::max((cw * xaxis + bw) * brightness,0.0f), 1.0f);

        this->red_output->set_level(red);
        this->green_output->set_level(green);
        this->blue_output->set_level(blue);
        this->white_output->set_level(white);

        // Store this
      colorTemp_ = colorTemp;
    } else {
        this->red_output->set_level(red);
        this->green_output->set_level(green);
        this->blue_output->set_level(blue);
        this->white_output->set_level(0);
    }
    this->oldRed = red;
    this->oldGreen = green;
    this->oldBlue = blue;

    brightness_ = brightness;

  }


protected:
  FloatOutput *red_output;
  FloatOutput *green_output;
  FloatOutput *blue_output;
  FloatOutput *white_output;


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

  // white intercept and scale
  float bw;
  float cw;
};
