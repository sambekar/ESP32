#ifndef REVEAL_SHADER_H
#define REVEAL_SHADER_H

#include <stdint.h>
#include "display_config.h"
extern const uint8_t frame[80][80][3];

// shader types
enum RevealMode {
  REVEAL_LINEAR_TOP_DOWN = 0,
  REVEAL_RADIAL_CENTER,
  REVEAL_NOISE_DISSOLVE,
  REVEAL_SLICE_LEFT_TO_RIGHT
};

struct RevealParams {
  RevealMode mode;
  uint16_t speed;        // 1..255 - higher = faster
  uint8_t density;       // 1..255 used by noise dissolve (how many pixels per tick)
  float  noiseScale;     // noise frequency for dissolve
  int    centerX, centerY; // for radial
  bool   looping;        // whether to reset after fully revealed
};

// Public API
void reveal_init(const RevealParams& params);
void reveal_setParams(const RevealParams& params);
void reveal_start();                // start the reveal
void reveal_stop();                 // stop/pause the reveal
void reveal_reset();                // reset mask to initial (fully hidden)
void reveal_update();               // call from loop() frequently; draws pixels
bool reveal_isFinished();           // true when reveal completed

// helper to directly reveal entire image (instant)
void reveal_forceComplete();

#endif
