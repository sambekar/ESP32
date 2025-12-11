#include "display_config.h"
#include "snowfall.h"
#include "framedefinition.h"

#define VDISP_NUM_ROWS 2
#define VDISP_NUM_COLS 1

MatrixPanel_I2S_DMA* dma_display = nullptr;
VirtualMatrixPanel_T<PANEL_CHAIN_TYPE, MyScanTypeMapping>* virtualDisp = nullptr;

HUB75_I2S_CFG mxconfig(PANEL_RES_X * 2, PANEL_RES_Y / 2, VDISP_NUM_ROWS * VDISP_NUM_COLS);

void setupPins() {
  mxconfig.gpio.r1 = 25;
  mxconfig.gpio.g1 = 26;
  mxconfig.gpio.b1 = 27;

  mxconfig.gpio.r2 = 14;
  mxconfig.gpio.g2 = 12;
  mxconfig.gpio.b2 = 13;

  mxconfig.gpio.a = 23;
  mxconfig.gpio.b = 19;
  mxconfig.gpio.c = 5;
  mxconfig.gpio.d = 17;
  mxconfig.gpio.e = 18;

  mxconfig.gpio.lat = 4;
  mxconfig.gpio.oe = 15;
  mxconfig.gpio.clk = 16;
}

void setup() {
  setupPins();

  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  mxconfig.clkphase = false;

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(50);
  dma_display->clearScreen();

  virtualDisp = new VirtualMatrixPanel_T<PANEL_CHAIN_TYPE, MyScanTypeMapping>(
      VDISP_NUM_ROWS, VDISP_NUM_COLS, PANEL_RES_X, PANEL_RES_Y);

  virtualDisp->setDisplay(*dma_display);

  initSnowfall();
}

void loop() {
  updateSnowfall();
}
