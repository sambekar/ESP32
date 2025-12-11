#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>

// ---------------- PANEL CONFIG ----------------
#define PANEL_RES_X 80
#define PANEL_RES_Y 40
#define VDISP_NUM_ROWS 2
#define VDISP_NUM_COLS 1

#define PANEL_CHAIN_TYPE CHAIN_BOTTOM_RIGHT_UP
#define PANEL_SCAN_TYPE FOUR_SCAN_40PX_HIGH

// Mapping typedef used by your display
using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;

// Extern so all files share the same virtual display
extern VirtualMatrixPanel_T<PANEL_CHAIN_TYPE, MyScanTypeMapping>* virtualDisp;

#endif
