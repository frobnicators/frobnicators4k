#ifndef MUSIC_H
#define MUSIC_H


#include "winclude.h"
#include "4klang.h"


void init_music();

/* Returns the current sample */
const DWORD * music_time();

float envelope(int instrument, int voice, INT32 time_offset);

#endif