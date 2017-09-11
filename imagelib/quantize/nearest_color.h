#ifndef _NEAREST_COLOR_H_
#define _NEAREST_COLOR_H_
#include "imageLib.h"

namespace ImageLib{

struct nearest_color
{
	void* nodes;
	
	// 
	nearest_color() : nodes(0) {}
	~nearest_color() { ::free(nodes); }
	nearest_color(Color* palette, int palSize);
	void create(Color* palette, int palSize);
	void free(void) { free_ref(nodes); }
	DEF_RETPAIR(pair_t, int, index, Color, color);
	pair_t nearest(Color color);
	
	// line conversion
	void convLine32ToIdx(byte* src, byte* dst, int width);
	void convLine32To32(byte* src, byte* dst, int width);
};

void nearest_conv(Image& src, Image& dst, 
	LineConvFunc2 fn = &copyLine32, size_t arg = 0);
void nearest_conv(Image& src, Image& dst, Color* pal, int psz,
	LineConvFunc2 fn = &copyLine32, size_t arg = 0);

int nearest_conv(ImageObj& img, Color* pal, int psz,
	LineConvFunc2 fn = &copyLine32, size_t arg = 0);
int nearest_conv32(ImageObj& img, Color* pal, int psz,
	LineConvFunc2 fn = &copyLine32, size_t arg = 0);
	

	
	
	
}
#endif
