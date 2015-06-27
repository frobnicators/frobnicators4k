#include "music.h"
#include "mmsystem.h"
#include "mmreg.h"

void init_music() {

}

const DWORD * music_time() {
	static DWORD temp = 0;
	return &temp;
}

float envelope(int instrument, int voice, INT32 time_offset) {
	return 0;
}
