#ifndef MUSIC_H
#define MUSIC_H

#include "winclude.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_music();

/* Returns the current time in ms */
const DWORD * music_time();

float envelope(int instrument, int voice, INT32 time_offset);

#ifdef __cplusplus
}
#endif

#endif
