#ifndef REVEAL_SHADER_PRESETS_H
#define REVEAL_SHADER_PRESETS_H

#include "reveal_shader.h"

// convenience presets
static inline RevealParams preset_topdown_fast() {
  RevealParams p;
  p.mode = REVEAL_LINEAR_TOP_DOWN;
  p.speed = 200;
  p.density = 20;
  p.noiseScale = 0.1f;
  p.centerX = 40; p.centerY = 40;
  p.looping = false;
  return p;
}

static inline RevealParams preset_radial_slow() {
  RevealParams p;
  p.mode = REVEAL_RADIAL_CENTER;
  p.speed = 20;
  p.density = 10;
  p.noiseScale = 0.08f;
  p.centerX = 40; p.centerY = 20;
  p.looping = false;
  return p;
}

static inline RevealParams preset_dissolve() {
  RevealParams p;
  p.mode = REVEAL_NOISE_DISSOLVE;
  p.speed = 120;
  p.density = 60;
  p.noiseScale = 0.07f;
  p.centerX = 40; p.centerY = 40;
  p.looping = false;
  return p;
}

#endif
