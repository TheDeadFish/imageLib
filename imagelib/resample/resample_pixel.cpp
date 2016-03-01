// Basic nerest neigbour pixel resampler
// DeadFish Shitware 2014
#include "resample.h"
namespace ImageLib {

void Resample_Copy(Image& dst, const Image& src)
{
	uint h = dst.height;
	uint w = dst.width;
	
	if(dst.palSize != 0 )
	{
		byte* dstLine = dst.bColors;
		byte* srcLine = src.bColors;
		int dstExtra = dst.sizeExtra8(w);
		int srcExtra = src.sizeExtra8(w);		
		
		while(h--)
		{
			memcpy8_ref(dstLine, srcLine, w);
			dstLine += dstExtra;
			srcLine += srcExtra;
		}
	} else 
	{
		Color* dstLine = dst.cColors;
		Color* srcLine = src.cColors;	
		int dstExtra = dst.sizeExtra32(w);
		int srcExtra = src.sizeExtra32(w);
		
		while(h--)
		{
			memcpy32_ref(dstLine, srcLine, w);
			PTRADD(dstLine, dstExtra);
			PTRADD(srcLine, srcExtra);
		}
	}
}

void Resample_Pixel(Image& dst, const Image& src)
{
	uint w1 = src.width; uint h1 = src.height;
	uint w2 = dst.width; uint h2 = dst.height;
	uint x_ratio = (uint)((w1<<16)/w2) +1;
	uint y_ratio = (uint)((h1<<16)/h2) +1;
	
	if(dst.palSize != 0 )
	{
		for (uint i=0;i<h2;i++) 
		{
			byte* dstLine = dst.get8(0, i);
			const byte* srcLine = src.get8(0,
				((i*y_ratio)>>16));
			uint xpos = 0;
			for (uint j=0;j<w2;j++) {
				dstLine[j] = srcLine[xpos>>16];
				xpos += x_ratio; }
		}
	} else 
	{
		for (uint i=0;i<h2;i++) 
		{
			Color* dstLine = dst.get32(0, i);
			const Color* srcLine = src.get32(0,
				((i*y_ratio)>>16));
			uint xpos = 0;
			for (uint j=0;j<w2;j++) {
				dstLine[j] = srcLine[xpos>>16];
				xpos += x_ratio; }
		}
	}
}
}
