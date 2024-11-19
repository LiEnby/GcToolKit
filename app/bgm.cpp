#include <soloud.h>
#include <soloud_wavstream.h>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
extern "C"
{
	static SoLoud::Soloud soloud;
	static SoLoud::WavStream bgm;
	
	void init_sound() {
		soloud.init();
		soloud.setGlobalVolume(1.0);
		
		// load bgm
		bgm.load("app0:/res/bgm.ogg");
		bgm.setLooping(1);
		soloud.play(bgm);
		
	}
	
	void term_sound() {
		soloud.deinit();
	}
	
}