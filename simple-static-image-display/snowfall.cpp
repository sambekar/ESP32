#include "snowfall.h"
#include "display_config.h"
#include <Arduino.h>

// ---------------- CONFIG ----------------
#define NUM_FLAKES 30
static const int snowInterval = 20;

// ---------------- INTERNAL STATE ----------------
struct Snowflake {
    int x, y;
    int drift;     // -1, 0, +1
    int speed;     // fall speed
    bool active;
};

static Snowflake flakes[NUM_FLAKES];
static int revealHeight[80];
static unsigned long lastSnowTime = 0;

// ---------------- HELPERS ----------------

static void drawSnowPixel(int x, int y) {
    virtualDisp->drawPixelRGB888(x, y, 255, 255, 255);
}

static void spawnFlake() {
    for (int i = 0; i < NUM_FLAKES; i++) {
        if (!flakes[i].active) {
            flakes[i].active = true;
            flakes[i].x = random(0, 80);
            flakes[i].y = 0;
            flakes[i].drift = random(-1, 2);
            flakes[i].speed = random(1, 3);
            return;
        }
    }
}

static void updateReveal() {
    for (int i = 0; i < 4; i++) {   // reveal rate
        int x = random(0, 80);

        if (revealHeight[x] < 80) {
            int y = revealHeight[x];
            uint8_t r = frame[y][x][0];
            uint8_t g = frame[y][x][1];
            uint8_t b = frame[y][x][2];
            virtualDisp->drawPixelRGB888(x, y, r, g, b);
            revealHeight[x]++;
        }
    }
}

static void updateFlakes() {
    for (int i = 0; i < NUM_FLAKES; i++) {
        if (!flakes[i].active) continue;

        // Erase old flake
        if (flakes[i].y < revealHeight[flakes[i].x]) {
            uint8_t r = frame[flakes[i].y][flakes[i].x][0];
            uint8_t g = frame[flakes[i].y][flakes[i].x][1];
            uint8_t b = frame[flakes[i].y][flakes[i].x][2];
            virtualDisp->drawPixelRGB888(flakes[i].x, flakes[i].y, r, g, b);
        } else {
            virtualDisp->drawPixelRGB888(flakes[i].x, flakes[i].y, 0, 0, 0);
        }

        // Move down
        flakes[i].y += flakes[i].speed;

        // Drift
        flakes[i].x += flakes[i].drift;
        if (flakes[i].x < 0) flakes[i].x = 0;
        if (flakes[i].x > 79) flakes[i].x = 79;

        // Deactivate at bottom
        if (flakes[i].y >= 80) {
            flakes[i].active = false;
            continue;
        }

        drawSnowPixel(flakes[i].x, flakes[i].y);
    }
}

// ---------------- PUBLIC API ----------------

void initSnowfall() {
    for (int x = 0; x < 80; x++)
        revealHeight[x] = 0;

    for (int i = 0; i < NUM_FLAKES; i++)
        flakes[i].active = false;

    lastSnowTime = millis();
}

void updateSnowfall() {
    unsigned long now = millis();

    if (now - lastSnowTime >= snowInterval) {
        lastSnowTime = now;

        updateReveal();

        if (random(0, 100) < 40)
            spawnFlake();

        updateFlakes();
    }
}
