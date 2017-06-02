#ifndef _COLOR_QUANTIZE_H_
#define _COLOR_QUANTIZE_H_
#include "imageLib.h"
namespace ImageLib{

#define SPLITMODE_MEDIAN	0x00
#define SPLITMODE_MODE		0x01

int medianCut_core(Color* image, int numColors,
	Color* palette, int reqColors, int split_mode);
int medianCut(Image& img, Color* palette, int reqColors, int split_mode,
	LineConvFunc2 fn = &copyLine32, size_t arg = 0);
	
	
byte* REGCALL(3) btPosterizeLine(byte* src,
	byte* dst, size_t arg, int width);
byte* REGCALL(3) unPosterizeLine(byte* src,
	byte* dst, size_t arg, int width);
byte* REGCALL(3) posterizeLine(byte* src,
	byte* dst, size_t arg, int width);
	
}
#endif
