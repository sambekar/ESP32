#ifndef SNOWFALL_H
#define SNOWFALL_H

#include "display_config.h"

// The static PNG image frame you display
extern const uint8_t frame[80][80][3];

void initSnowfall();
void updateSnowfall();

#endif
