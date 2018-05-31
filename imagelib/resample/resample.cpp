// Image resizer
// DeadFish Shitware 2014
#include "resample.h"

namespace ImageLib {

struct Resampler
{
	Image dst; Image src; RECT dstRc; 
	byte *dstPos, *dstEnd; uint lineLen;
	uint x_ratio, y_ratio, xpos, ypos;
	
	void init(Image& dst, const 
		RECT& dstRc, const Image& src);
		
		
		
	NEVER_INLINE static REGCALL(2) 
		uint calcRatio(uint sz1, uint sz2);
	
		
		
	
	void pixel();
	REGCALL(3) void pixel8(byte* dLn, byte* sLn);
	REGCALL(3) void pixel32(byte* dLn, byte* sLn);
};

uint Resampler::calcRatio(uint sz1, uint sz2)
{
	return ((sz1<<16) + (sz2-1)) / sz2;
}

void Resampler::init(Image& dst_, 
	const RECT& dstRc_, const Image& src_)
{
	dst.initRef(dst_);
	src.initRef(src_);

	// calculate ratio
	x_ratio = calcRatio(src.width, RECT_W(dstRc_));
	y_ratio = calcRatio(src.height, RECT_H(dstRc_));
	
	// calculate xpos
	dst.clipRect(dstRc, dstRc_);
	xpos = (dstRc.left-dstRc_.left)*x_ratio;
	ypos = (dstRc.top-dstRc_.top)*y_ratio;
	
	// 
	lineLen = RECT_W(dstRc)*dst.pixSize();
	dstEnd = dst.getLn(dstRc.bottom);
	dstPos = dst.getPtr(dstRc.left, dstRc.top);
}

void Resampler::pixel8(byte* dLn, byte* sLn)
{
	uint xpos = this->xpos;
	byte* dLnEnd = dLn + lineLen; 
	while(dLn < dLnEnd) { *dLn = sLn[xpos>>16]; 
		dLn++; xpos += x_ratio;  }
}

void Resampler::pixel32(byte* dLn, byte* sLn)
{
	uint xpos = this->xpos;
	byte* dLnEnd = dLn + lineLen; 
	while(dLn < dLnEnd) { RI(dLn) = PI(sLn)
		[xpos>>16]; dLn += 4; xpos += x_ratio; }
}

void Resampler::pixel()
{
	for(byte* dLn = dstPos; 
	dLn < dstEnd; dLn += dst.pitch) {
		byte* sLn = src.getLn(ypos>>16);
		ypos += y_ratio;		
		if(dst.hasPalette()) { pixel8(dLn, sLn);
		} else { pixel32(dLn, sLn); }
	}
	
	nothing();
}

__stdcall void Resample_Pixel(Image& dst, 
	const RECT& dstRc, const Image& src)
{
	Resampler rs; rs.init(dst, dstRc, src);
	rs.pixel();
}

RECT Resize_FixAspect(
	int newWidth, int newHeight,
	int oldWidth, int oldHeight)
{
	long double ratio = (long double)(oldHeight) / oldWidth;
	int scaleSize = lrintl(newWidth * ratio);
	if( scaleSize <= newHeight )
		return Rect::RectXYWH(0, (newHeight - scaleSize)>>1,
			newWidth, scaleSize);
	scaleSize = lrintl(newHeight / ratio);
	return Rect::RectXYWH((newWidth - scaleSize)>>1, 0, 
		scaleSize, newHeight);
}

// Resize canvas and centre the image
int Resize(ImageObj& dst, const Image& src,
	DWORD bkgnd, int w, int h)
{
	RECT rc = Rect::RectXYWH(
		((w - src.width)>>1),
		((h - src.height)>>1),
		src.width, src.height);
	return ResizeResample(dst, src,
		bkgnd, w, h, rc, RESAMPLE_PIXEL);
}

// Resize canvas and locate image at xy
int Resize(ImageObj& dst, const Image& src,
	DWORD bkgnd, int w, int h, int x, int y)
{
	RECT rc = Rect::RectXYWH(
		x, y, src.width, src.height);
	return ResizeResample(dst, src,
		bkgnd, w, h, rc, RESAMPLE_PIXEL);	
}

// Resample image to fit wh	
int Resample(ImageObj& dst, const Image& src,
	int w, int h, int mode )
{
	RECT rc = {0, 0, w, h};
	return ResizeResample(dst, src,
		0,	w, h, rc, mode);	
}

// Resample image maintaing aspect ratio
int Resample(ImageObj& dst, const Image& src,
	DWORD bkgnd, int w, int h, int mode )
{
	RECT rc = Resize_FixAspect(w, h,
		src.width, src.height);
	return ResizeResample(dst, src,
		bkgnd, w, h, rc, mode);
}

int ResizeResample(ImageObj& dst, const Image& src,
	DWORD bkgnd, int w, int h, const RECT& dstRc, int mode)
{
	// create destination
	ImageObj tmp;
	IFRET(tmp.Create(src, w, h));
	tmp.fillNotRect(dstRc, bkgnd);
	
	// peform the resample
	switch(mode)
	{
	case RESAMPLE_PIXEL:
		Resample_Pixel(tmp, dstRc, src);
		break;
	case RESAMPLE_BILINEAR:
		//if(!Resample_Bilinear(dst, src))
		//	return false;
		break;
	case RESAMPLE_BICUBIC:
		//if(!Resample_Bicubic(dst, src))
		//	return false;
		break;
	}

	// success return
	dst.Swap(tmp);
	return true;
}
}
