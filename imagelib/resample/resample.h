// Image resizing routines
// DeadFish Shitware 2014
#ifndef _RESAMPLE_H_
#define _RESAMPLE_H_
#include "imageLib.h"

namespace ImageLib{

enum ResampleType {
	RESAMPLE_PIXEL,
	RESAMPLE_BILINEAR,
	RESAMPLE_BICUBIC
};

RECT Resize_FixAspect(
	int newWidth, int newHeight,
	int oldWidth, int oldHeight);

// Resize canvas and centre the image	
__stdcall int Resize(ImageObj& dst, const Image& src,
	DWORD bkgnd, int w, int h);
// Resize canvas and locate image at xy			
__stdcall int Resize(ImageObj& dst, const Image& src,
	DWORD bkgnd, int w, int h, int x, int y);
// Resample image to fit wh	
__stdcall int Resample(ImageObj& dst, const Image& src,
	int w, int h, int mode );
// Resample image maintaing aspect ratio	
__stdcall int Resample(ImageObj& dst, const Image& src,
	DWORD bkgnd, int w, int h, int mode );
// Resize/Resample core
__stdcall int ResizeResample(ImageObj& img, const Image& src,
	DWORD bkgnd, int w, int h, const RECT& dstRc, int mode);

// Resample Core
__stdcall void Resample_Pixel(Image& dst, const RECT& dstRc, const Image& src);
__stdcall bool Resample_Bilinear(Image& dst, const RECT& dstRc, const Image& src);
__stdcall bool Resample_Bicubic(Image& dst, const RECT& dstRc, const Image& src);	
}
#endif
