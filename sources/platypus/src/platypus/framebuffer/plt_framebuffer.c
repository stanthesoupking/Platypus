#include "plt_framebuffer.h"
#include "platypus/base/platform.h"

void plt_framebuffer_clear(Plt_Framebuffer framebuffer, Plt_Color8 color) {
	#ifdef PLT_PLATFORM_MACOS
		memset_pattern4(framebuffer.pixels, &color, sizeof(Plt_Color8) * framebuffer.width * framebuffer.height);
	#else 
		int total_pixels = framebuffer.width * framebuffer.height;
		for(int i = 0; i < total_pixels; ++i) {
			framebuffer.pixels[i] = color;
		}
	#endif
}