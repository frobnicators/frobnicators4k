#include "music.h"
#include "mmsystem.h"
#include "mmreg.h"

#include "libv2.h"
#include "v2mplayer.h"

static V2MPlayer player;
extern "C" const sU8 tune[];
extern "C" float time;

extern "C" void init_music() {
	player.Init();
	player.Open(tune);
	dsInit(player.RenderProxy, &player, GetForegroundWindow());
	player.Play();
}

extern "C" void update_time() {
	const long t = dsGetCurSmp();
	time = (float)t / (44100.f*4);
}

extern "C" float envelope(int instrument, int voice, INT32 time_offset) {
	return 0;
}
