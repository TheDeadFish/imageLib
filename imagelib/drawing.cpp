// Image handling library
// Simple drawing routines
// DeadFish Shitware 2014
#include "stdshit.h"
#include "imageLib.h"

namespace ImageLib {

void Image::fillImage(DWORD color)
{
	int yCount = height; int xLength = width;
	if(palSize != 0) { byte* dstLine = bColors;
		while(yCount--) { memset(dstLine, color, xLength);
			PTRADD(dstLine, pitch); }
	}else {	Color* dstLine = cColors;
		while(yCount--) { std::fill(dstLine, dstLine+xLength,
			Color(color));	PTRADD(dstLine, pitch); }}
}

static RECT flipRect(const RECT& rc0)
{	RECT rc = rc0; if(rc.left > rc.right) swapReg(rc.left, rc.right);
	if(rc.top > rc.bottom) swapReg(rc.top, rc.bottom); return rc; }

void Image::fillRect(
	const RECT& rc0, DWORD color)
{
	RECT rc; clipRect(rc, rc0); 
	byte* dstLine = getPtr(rc.left, rc.top);
	int count = RECT_H(rc); int width = RECT_W(rc);
	
	while(--count >= 0) { if(hasPalette()) {
			memfillX(dstLine, width, color);
		} else { memfillX((DWORD*)dstLine, 
			width, color); }
		PTRADD(dstLine, pitch); }
}

void Image::fillNotRect(const RECT& rc, DWORD color)
{
	for(int i = 0; i < 4; i++) {
		Rect rc2 = NotRect::Get(i, rc, width, height);
		fillRect(rc2, color); }
}


void Image::drawLineH(int y, int x1, int x2, DWORD color)
{
	if(palSize != 0) {
		byte* curPos = get8(x1, y); byte* endPos = get8(x2, y);
		while(curPos < endPos) { *curPos = color; curPos++; }
	} else {
		Color* curPos = get32(x1, y); Color* endPos = get32(x2, y);
		while(curPos < endPos) { *curPos = color; curPos++; }
	}
}

void Image::drawLineV(int x, int y1, int y2, DWORD color)
{
	if(palSize != 0) {
		byte* curPos = get8(x, y1); byte* endPos = get8(x, y2);
		while(curPos < endPos) { *curPos = color; PTRADD(curPos, pitch); }
	} else {
		Color* curPos = get32(x, y1); Color* endPos = get32(x, y2);
		while(curPos < endPos) { *curPos = color; PTRADD(curPos, pitch); }
	}
}

void Image::drawRect(const RECT& dst, DWORD color) 
{
	RECT rc = flipRect(dst);
	drawLineH(rc.top, rc.left, rc.right, color);
	drawLineH(rc.bottom-1, rc.left, rc.right, color);
	drawLineV(rc.left, rc.top, rc.bottom, color);
	drawLineV(rc.right-1, rc.top, rc.bottom, color);
}

void Image::drawRect(const RECT& dst, DWORD line, DWORD fill)
{
	fillRect(dst, fill); drawRect(dst, line);
}
}

