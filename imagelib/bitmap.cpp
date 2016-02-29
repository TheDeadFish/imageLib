// DeadFish image library
// Windows Bitmap functions

#include <malloc.h>
#include "imageLib.h"
namespace ImageLib{

int bmCalcPitch(LPBITMAPV5HEADER bi) { return ALIGN(
	bi->bV5Width * bi->bV5BitCount, 31) >> 3; }

int Image::GetBitmapInfo(BmInfo256& bi, bool blitMode) const
{
	// initialize bitmap header
	memset(&bi, 0, sizeof(BITMAPV5HEADER));
	bi.bV5Size = sizeof(BITMAPINFOHEADER); bi.bV5Planes = 1;
	bi.bV5Compression = BI_RGB;	bi.bV5ClrUsed = palSize;
	if(blitMode == true) { bi.bV5Height = -height;
		bi.bV5Width = hasPalette() ? pitch : pitch/4;
		bi.bV5BitCount = calcNBits(colType | HAS_COLOR | HAS_ALPHA);	
	} else {
		bi.bV5Width = width; bi.bV5Height = height;
		bi.bV5BitCount = calcNBits(colType | HAS_COLOR);
		if(hasAlpha()) {
			bi.bV5Size = sizeof(BITMAPINFOHEADER);
			bi.bV5Compression = BI_BITFIELDS;
			bi.bV5RedMask = 0x00FF0000;		bi.bV5GreenMask = 0x0000FF00;
			bi.bV5BlueMask = 0x000000FF;	bi.bV5AlphaMask = 0xFF000000; 
		}
	}
	
	// initialize palette
	if(hasPalette()) {
		if(blitMode) bi.bV5BitCount = 8;
		ei(bi.bV5BitCount == 2) bi.bV5BitCount = 4;
		for(int i = 0; i < palSize; i++)
			bi.bmiColors(i) = palette[i]; }
	return bmCalcPitch((LPBITMAPV5HEADER)&bi);
}

int Image::SaveBitmap(FILE* fp)
{
	// write bitmap header
	PACK1(struct { BITMAPFILEHEADER fh; BmInfo256 bmi; } bi;)
	int bmPitch = GetBitmapInfo(bi.bmi, false);
	bi.fh.bfType = 0x4D42; bi.fh.bfReserved1 = 0; bi.fh.bfReserved2 = 0;
	bi.fh.bfOffBits = sizeof(bi.fh) + bi.bmi.getSize();
	bi.fh.bfSize = bi.fh.bfOffBits + bmPitch * height;
	if(fwrite(&bi.fh, bi.fh.bfOffBits, 1, fp) != 1)
		return ERR_SYSTEM;
		
	// write line data
	if(setvbuf(fp, NULL, _IOFBF, bmPitch+2))
		return ERR_SYSTEM;
	auto packLine = linePackFunc(bi.bmi.bV5BitCount);
	byte* src = EndPtr();
	while(src > bColors) { src -= pitch;
		packLine(src, fBuf(fp, bmPitch), width);
		if(wBuf(fp) < 0) return ERR_SYSTEM; 
	}
	return 0;
}

int ImageObj::LoadBmp(void* data, int size)
{
	LPBITMAPFILEHEADER bmHdr = Void(data);
	if((bmHdr->bfType != 0x4D42)
	||(bmHdr->bfSize != size))
		return BAD_IMAGE;
	return FromBmInfo(Void(bmHdr+1),
		Void(bmHdr)+bmHdr->bfOffBits);
}

int ImageObj::FromBmInfo(LPBITMAPV5HEADER bi, BYTE* imgData)
{
	// process header
	int hdrsz = ((bi->bV5Compression == BI_BITFIELDS)
	&&(bi->bV5Size == 40)) ? 52 : bi->bV5Size;
	Color* colTable = Void(bi) + hdrsz;
	int biClrUsed = (bi->bV5BitCount > 8) ? 0 :
		bi->bV5ClrUsed ? bi->bV5ClrUsed : 1<<bi->bV5BitCount;
	if(!imgData) imgData = (BYTE*)&colTable[biClrUsed];

	// process bitfields
	bool hasAplha = false;
	if(bi->bV5Compression == BI_BITFIELDS) {
		if((bi->bV5RedMask != 0x00FF0000)
		||(bi->bV5GreenMask != 0x0000FF00)
		||(bi->bV5BlueMask != 0x000000FF))
			return USP_IMAGE;
		if(bi->bV5Size >= 56) {
			if(bi->bV5AlphaMask == 0xFF000000) hasAplha = true;
			ei(bi->bV5AlphaMask != 0) return USP_IMAGE;
		}
	}
	
	// create image
	u8 colType_ = biClrUsed ? HAS_PALETTE : 
		hasAplha ? HAS_RGBA : HAS_COLOR;
	if(int errCode = Create(bi->bV5Width, bi->bV5Height,
		colType_, bi->bV5BitCount)) return errCode;
	if(biClrUsed) setPalette(colTable, biClrUsed);
		
	// copy pixel data
	int biPitch = bmCalcPitch(bi);
	if(bi->bV5Height > 0) {
		imgData += biPitch*(height-1);
		biPitch = -biPitch; }
	auto unpackLine = lineUnpackFunc(nBits);
	byte* dst = bColors;
	int count = height; while(count--) {
		unpackLine(imgData, dst, width);
		imgData += biPitch; dst += pitch; }		

	
		
	/* determin alpha type
	if(bi->biBitCount == 32)
	{
		int nCColors = width * height;
		
		// Guess the type of alpha
		bool hasAlpha = false;
		bool realAlpha = false;
		char weirdAlpha = 0;
		for(int i = 0; i < nCColors; i++)
		{
			BYTE alpha = cColors[i].GetA();
			if(alpha != 0)
				realAlpha = true;
			if(alpha != 255)
			{
				hasAlpha = true;
				if(weirdAlpha >= 0)
				{
					if((bi->biCompression == BI_BITFIELDS) || (alpha != 0))
					{
						weirdAlpha = 1;
						if((cColors[i].GetR() > alpha)
						or (cColors[i].GetG() > alpha)
						or (cColors[i].GetB() > alpha))
							weirdAlpha = -1;
					}
				}
			}
		}
		
		// alpha correction
		if(hasAlpha)
		{
			if(weirdAlpha > 0)
			{
				// Guess that alpha is weird
				// Undo the weird browser alpha
				for(int i = 0; i < nCColors; i++)
					if(cColors[i].GetA() > 0)
					{
						cColors[i] = Color(cColors[i].GetA(), 
							(cColors[i].GetR()*255)/cColors[i].GetA(),
							(cColors[i].GetG()*255)/cColors[i].GetA(),
						(cColors[i].GetB()*255)/cColors[i].GetA()
						);
					}
			}
			else
			{
				// Alpha is not weird
				if((!realAlpha) || (bi->biCompression == BI_BITFIELDS))
				{
					// Guessed that alpha is not real
					// Set all alpha values to opaqe
					for(int i = 0; i < nCColors; i++)
						cColors[i].SetA(255);
				}
			}
		}
	} */
	
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
	
	

	

	return ERR_NONE;
}

void ImageObj::CopyBmBuffer(byte* imgData, int biHeight)
{

}
}
