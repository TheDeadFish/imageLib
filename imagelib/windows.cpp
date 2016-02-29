// DeadFish Image library
// Windows specific functions

#include "imageLib.h"
#include <malloc.h>
namespace ImageLib{

#ifndef RECT_XYWH
 #define RECT_XY(rc) (rc).left, (rc).top
 #define RECT_WH(rc) (rc).right-(rc).left, (rc).bottom-(rc).top
 #define RECT_XYWH(rc) (rc).left, (rc).top, RECT_WH(rc)
#endif

void Image::bitBlt(HDC hdc, int x, int y) const {
	bitBlt(hdc, x, y, width, height); }
void Image::bitBlt(HDC hdc, int x, int y, int w, int h) const {
	BmInfo256 bi; GetBitmapInfo(bi, true); StretchDIBits(hdc, x, y, w, h,
		0, 0, width, height, cColors, &bi, DIB_RGB_COLORS, SRCCOPY); }
		
void Image::bitBlt(HDC hdc, int x, int y, const RECT& src) const {
	BmInfo256 bi; GetBitmapInfo(bi, true); StretchDIBits(hdc, x, y, RECT_WH(src),
		RECT_XYWH(src), cColors, &bi, DIB_RGB_COLORS, SRCCOPY); }
void Image::strBlt(HDC hdc, const RECT& dst) const {
	BmInfo256 bi; GetBitmapInfo(bi, true); StretchDIBits(hdc, RECT_XYWH(dst),
		0, 0, width, height, cColors, &bi, DIB_RGB_COLORS, SRCCOPY); }
void Image::strBlt(HDC hdc, const RECT& dst, const RECT& src) const {
	BmInfo256 bi; GetBitmapInfo(bi, true); StretchDIBits(hdc, RECT_XYWH(dst),
		RECT_XYWH(src), cColors, &bi, DIB_RGB_COLORS, SRCCOPY); }

void ImageObj::BitBlt(HDC hdcDst, int x, int y) const {
	::BitBlt(hdcDst, x, y, width, height, hdc, 0, 0, SRCCOPY); }
void ImageObj::BitBlt(HDC hdcDst, int x, int y, const RECT& dst) const {
	::BitBlt(hdcDst, x, y, RECT_WH(dst), hdc, RECT_XY(dst), SRCCOPY); }
void ImageObj::StrBlt(HDC hdcDst, const RECT& dst) const { ::StretchBlt(hdcDst,
	RECT_XYWH(dst), hdc, 0, 0, width, height, SRCCOPY); }
void ImageObj::StrBlt(HDC hdcDst, const RECT& dst, const RECT& src) const {
	::StretchBlt(hdcDst, RECT_XYWH(dst), hdc, RECT_XYWH(src), SRCCOPY); }

int ImageObj::FromClip(HWND hwnd)
{
	HGLOBAL hglb; 
	LPBITMAPV5HEADER bi;
	int errCode = ERR_SYSTEM;
	
	// Try to open the clipboard
	this->Delete();
	for(int trys = 0;; trys++)
	{
		if(OpenClipboard(hwnd))
			break;
		if(trys >= CLIP_TRYS)
			return ERR_SYSTEM;
		Sleep(CLIP_WAIT);
	}
	
	// Try and get bitmap from the clipBoard
	if( ((hglb = GetClipboardData (CF_DIB)) != NULL)
	and ((bi = (LPBITMAPV5HEADER)GlobalLock (hglb)) != NULL))
	{
		errCode = FromBmInfo(bi, NULL);
	}
	else
	{
		if(GetLastError() == ERROR_FILE_NOT_FOUND)
			errCode = CLIP_NODATA;
	}
	
	// Cleanup
	GlobalUnlock(hglb); 
    CloseClipboard(); 
	return errCode;
}

int Image::ToClip(HWND hwnd)
{
	/*HGLOBAL hglb;
	PBITMAPINFO bi;
	
	// Try to open the clipboard
	for(int trys = 0;; trys++)
	{
		if(OpenClipboard(hwnd))
			break;
		if(trys >= CLIP_TRYS)
			return ERR_SYSTEM;
		Sleep(CLIP_WAIT);
	}
	EmptyClipboard();
	
	// Allocate memory
	hglb = GlobalAlloc(GMEM_MOVEABLE, 
		sizeof(BITMAPINFOHEADER)
		+ palSize*4 + pitch*height);
	bi = (PBITMAPINFO)GlobalLock(hglb);
	if(hglb == NULL)
	{
		CloseClipboard(); 
		return ERR_ALLOC;
	}
	
	// output to clipboard
	GetBitmapInfo(bi, false);
	memcpy(&bi->bmiColors[palSize],
		cColors, pitch*height);
	if(!SetClipboardData(CF_DIB, hglb))
	{
		GlobalUnlock(hglb);
		GlobalFree(hglb);
		CloseClipboard();
		return ERR_SYSTEM;
	}
	CloseClipboard();
	return ERR_NONE;*/
}
}
