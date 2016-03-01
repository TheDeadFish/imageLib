// Image resizer
// DeadFish Shitware 2014
#include "resample_pixel.cpp"

namespace ImageLib {

Rect Resize_FixAspect(
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
	Rect rc = Rect::RectXYWH(
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
	Rect rc = Rect::RectXYWH(
		x, y, src.width, src.height);
	return ResizeResample(dst, src,
		bkgnd, w, h, rc, RESAMPLE_PIXEL);	
}

// Resample image to fit wh	
int Resample(ImageObj& dst, const Image& src,
	int w, int h, int mode )
{
	Rect rc(0, 0, w, h);
	return ResizeResample(dst, src,
		0,	w, h, rc, mode);	
}

// Resample image maintaing aspect ratio
int Resample(ImageObj& dst, const Image& src,
	DWORD bkgnd, int w, int h, int mode )
{
	Rect rc = Resize_FixAspect(w, h,
		src.width, src.height);
	return ResizeResample(dst, src,
		bkgnd, w, h, rc, mode);
}

int ResizeResample(ImageObj& dst, const Image& src,
	DWORD bkgnd, int w, int h, const Rect& rc0, int mode)
{
	printf("%d, %d, %d, %d, %d, %d\n", w, h, rc0);


	// create destination
	ImageObj tmp;
	ERR_CHK(tmp.Create(src, w, h));
	Image dstImg(tmp, rc0);
	tmp.fillNotRect(rc0, bkgnd);

	// create source Image context
	Rect rc(rc0.negExtX(), rc0.negExtY(),
		src.width, src.height);
	printf("%d, %d, %d, %d\n", rc);
		
	Image srcImg(src, rc);
	
	// peform the resample
	if(( rc0.Width() == src.width )
	&&( rc0.Height() == src.height ))
		Resample_Copy(dstImg, srcImg);
	else {
		switch(mode)
		{
		case RESAMPLE_PIXEL:
			printf("%d, %d\n", dstImg.width, 
				srcImg.width);
			
		
			Resample_Pixel(dstImg, srcImg);
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
	}

	// success return
	dst.Swap(tmp);
	return true;
}
}
