#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>
#include "animationFrames.h"  // we will generate this



#define PANEL_RES_X     80
#define PANEL_RES_Y     40
#define VDISP_NUM_ROWS      2 // Number of rows of individual LED panels, change to 1 for single panel
#define VDISP_NUM_COLS      1
#define PANEL_CHAIN_LEN     (VDISP_NUM_ROWS*VDISP_NUM_COLS)
#define PANEL_CHAIN_TYPE CHAIN_BOTTOM_RIGHT_UP // change to CHAIN_NONE for single panel
#define PANEL_SCAN_TYPE  FOUR_SCAN_40PX_HIGH
MatrixPanel_I2S_DMA *dma_display = nullptr;
using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;
VirtualMatrixPanel_T<PANEL_CHAIN_TYPE, MyScanTypeMapping>* virtualDisp = nullptr;
HUB75_I2S_CFG mxconfig(PANEL_RES_X*2,PANEL_RES_Y/2,PANEL_CHAIN_LEN);
void setupPins() {
  mxconfig.gpio.r1 = 25;
  mxconfig.gpio.g1 = 26;
  mxconfig.gpio.b1 = 27;

  mxconfig.gpio.r2 = 14;
  mxconfig.gpio.g2 = 12;
  mxconfig.gpio.b2 = 13;

  mxconfig.gpio.a  = 23;
  mxconfig.gpio.b  = 19;
  mxconfig.gpio.c  = 5;
  mxconfig.gpio.d  = 17;
  mxconfig.gpio.e  = 18;

  mxconfig.gpio.lat = 4;
  mxconfig.gpio.oe  = 15;
  mxconfig.gpio.clk = 16;
}


int currentFrame = 0;
unsigned long lastFrameTime = 0;


const int frameDelay = 50;        // Slow animation
const int holdLastFrame = 0;   // 10 seconds = 10000 ms
bool animationFinished = false;


void setup() {
    setupPins();  
    mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
    mxconfig.clkphase = false;
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(50); //0-255
    dma_display->clearScreen();
    virtualDisp = new VirtualMatrixPanel_T<PANEL_CHAIN_TYPE, MyScanTypeMapping>(VDISP_NUM_ROWS, VDISP_NUM_COLS, PANEL_RES_X, PANEL_RES_Y);
    virtualDisp->setDisplay(*dma_display);
}

void drawFrame(int frameIndex) {
    for (int y = 0; y < 80; y++) {
        for (int x = 0; x < 80; x++) {
            uint8_t r = animationFrames[frameIndex][y][x][0];
            uint8_t g = animationFrames[frameIndex][y][x][1];
            uint8_t b = animationFrames[frameIndex][y][x][2];
            virtualDisp->drawPixelRGB888(x, y, r, g, b);
        }
    }
}




void loop() {
    unsigned long now = millis();

    if (!animationFinished) {

        if (now - lastFrameTime >= frameDelay) {
            lastFrameTime = now;

            // Draw current frame
            drawFrame(currentFrame);

            // Advance
            currentFrame++;

            // If last frame reached
            if (currentFrame >= NUM_FRAMES) {
                currentFrame = NUM_FRAMES - 1; // stay on last
                animationFinished = true;
                lastFrameTime = now; // reset timer for holding
            }
        }

    } else {
        // We are on the last frame, just hold for 10 seconds
        if (now - lastFrameTime >= holdLastFrame) {
            // Optional:
            // Restart animation:
            currentFrame = 0;
            animationFinished = false;

            // Or: keep it frozen forever (delete above 3 lines)
        }
    }
}

