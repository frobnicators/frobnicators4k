#ifndef MUSIC_H
#define MUSIC_H

#include "winclude.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_music();

void update_time();

float envelope(int instrument, int voice, INT32 time_offset);

#ifdef __cplusplus
}
#endif

#endif
