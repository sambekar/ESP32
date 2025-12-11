#include "reveal_shader.h"
#include <Arduino.h>
#include <stdlib.h>
#include <math.h>

// Dimensions taken from your project
static const int W = 80;
static const int H = 80;

// mask: 0 = hidden, 255 = visible
static uint8_t mask[H][W];

// params & runtime
static RevealParams cfg;
static bool running = false;
static bool finished = false;
static unsigned long lastTick = 0;

// simple hash/noise helper (cheap value noise)
static inline uint32_t hash32(uint32_t x) {
  x = (x ^ 61) ^ (x >> 16);
  x *= 9;
  x = x ^ (x >> 4);
  x *= 0x27d4eb2d;
  x = x ^ (x >> 15);
  return x;
}
static float cheap_noise(int xi, int yi) {
  uint32_t v = hash32((uint32_t)xi * 374761393u + (uint32_t)yi * 668265263u);
  return (v & 0xFFFFFF) / float(0xFFFFFF); // 0..1
}

// internal single-pixel reveal function (applies frame color)
static inline void reveal_pixel_set(int x, int y) {
  // bounds sanity
  if (x < 0 || x >= W || y < 0 || y >= H) return;
  uint8_t r = frame[y][x][0];
  uint8_t g = frame[y][x][1];
  uint8_t b = frame[y][x][2];
  virtualDisp->drawPixelRGB888(x, y, r, g, b);
}

// erase pixel to black (if needed)
static inline void reveal_pixel_clear(int x, int y) {
  if (x < 0 || x >= W || y < 0 || y >= H) return;
  virtualDisp->drawPixelRGB888(x, y, 0, 0, 0);
}

// paint from mask (called when mask transitions to visible)
static inline void commitPixelIfNeeded(int x, int y, uint8_t oldVal) {
  if (oldVal < 255 && mask[y][x] == 255) {
    reveal_pixel_set(x, y);
  }
}

// fill entire screen from mask (useful on reset/boot)
static void paintFromMask() {
  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      if (mask[y][x])
        reveal_pixel_set(x, y);
      else
        reveal_pixel_clear(x, y);
    }
  }
}

void reveal_init(const RevealParams& params) {
  cfg = params;
  reveal_reset();
  running = false;
  finished = false;
  lastTick = millis();
}

// set new params on the fly
void reveal_setParams(const RevealParams& params) {
  cfg = params;
}

// start/pause/reset helpers
void reveal_start() { running = true; finished = false; lastTick = millis(); }
void reveal_stop() { running = false; }
void reveal_reset() {
  // default hidden state: mask = 0
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x)
      mask[y][x] = 0;
  paintFromMask();
  finished = false;
}

// instant reveal
void reveal_forceComplete() {
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      uint8_t old = mask[y][x];
      mask[y][x] = 255;
      commitPixelIfNeeded(x, y, old);
    }
  finished = true;
}

bool reveal_isFinished() { return finished; }

// core per-frame update dispatcher
void reveal_update() {
  if (!running || finished) return;

  unsigned long now = millis();
  unsigned long dt = now - lastTick;
  if (dt < 10) return; // small throttle so we don't run crazy fast
  lastTick = now;

  // scale speed to steps per tick
  int step = max(1, (int)cfg.speed / 4);

  if (cfg.mode == REVEAL_LINEAR_TOP_DOWN) {
    // Each tick, advance each column by 'step' rows
    bool anyChanged = false;
    for (int x = 0; x < W; ++x) {
      // find topmost hidden pixel in column
      int y;
      for (y = 0; y < H && mask[y][x] == 255; ++y);
      // reveal up to step pixels
      for (int s = 0; s < step && y < H; ++s, ++y) {
        uint8_t old = mask[y][x];
        mask[y][x] = 255;
        commitPixelIfNeeded(x, y, old);
        anyChanged = true;
      }
    }
    if (!anyChanged) {
      finished = true;
      if (cfg.looping) { reveal_reset(); running = true; finished = false; }
    }
  }
  else if (cfg.mode == REVEAL_SLICE_LEFT_TO_RIGHT) {
    // find leftmost column containing hidden pixels
    bool anyChanged = false;
    int col;
    for (col = 0; col < W; ++col) {
      bool full = true;
      for (int y = 0; y < H; ++y) { if (mask[y][col] == 0) { full = false; break; } }
      if (!full) break;
    }
    if (col >= W) {
      finished = true;
      if (cfg.looping) { reveal_reset(); running = true; finished = false; }
      return;
    }
    // reveal 'step' rows in that column
    int revealed = 0;
    for (int y = 0; y < H && revealed < step; ++y) {
      if (mask[y][col] == 0) {
        uint8_t old = mask[y][col];
        mask[y][col] = 255;
        commitPixelIfNeeded(col, y, old);
        revealed++;
      }
    }
  }
  else if (cfg.mode == REVEAL_RADIAL_CENTER) {
    // Grow radius from center
    static float radius = 0.0f;
    radius += (cfg.speed / 50.0f); // configure growth
    bool any = false;
    float maxr = sqrtf(W*W + H*H);
    for (int y = 0; y < H; ++y) {
      for (int x = 0; x < W; ++x) {
        if (mask[y][x] == 255) continue;
        float dx = x - cfg.centerX;
        float dy = y - cfg.centerY;
        float d = sqrtf(dx*dx + dy*dy);
        if (d <= radius) {
          uint8_t old = mask[y][x];
          mask[y][x] = 255;
          commitPixelIfNeeded(x, y, old);
          any = true;
        }
      }
    }
    if (radius > maxr) {
      finished = true;
      if (cfg.looping) { radius = 0.0f; reveal_reset(); running = true; finished = false; }
    }
  }
  else { // REVEAL_NOISE_DISSOLVE (default)
    // Randomly pick pixels; reveal if noise < threshold
    int attempts = cfg.density; // how many random samples per update
    bool any = false;
    for (int i = 0; i < attempts; ++i) {
      int x = random(0, W);
      int y = random(0, H);
      if (mask[y][x] == 255) continue;
      // sample cheap noise at scaled coordinates
      float nx = x * cfg.noiseScale;
      float ny = y * cfg.noiseScale;
      int xi = (int)nx;
      int yi = (int)ny;
      float nval = cheap_noise(xi, yi);
      // threshold influenced by speed: higher speed -> easier reveal
      float threshold = (cfg.speed / 255.0f); // 0..1
      if (nval < threshold) {
        uint8_t old = mask[y][x];
        mask[y][x] = 255;
        commitPixelIfNeeded(x, y, old);
        any = true;
      }
    }
    // check completion
    bool all = true;
    for (int yy = 0; yy < H && all; ++yy)
      for (int xx = 0; xx < W; ++xx)
        if (mask[yy][xx] == 0) { all = false; break; }
    if (all) {
      finished = true;
      if (cfg.looping) { reveal_reset(); running = true; finished = false; }
    }
  }
}
