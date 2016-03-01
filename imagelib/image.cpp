#include <algorithm>
#include "imageLib.h"

namespace ImageLib{

int Image::calcNBits(u8 colType) const
{
	if(colType & HAS_PALETTE) return min(8,
		max(nBits, snapToBits(palSize)));
	int sz = (colType & HAS_16BIT) ? 16 : 8;
	int l = (colType & HAS_COLOR) ? sz*3 : sz;
	return ((colType & HAS_ALPHA)
		||(nBits == (l+sz))) ? (l+sz) : l;
}

uint Image::calcBpp(uint colType, uint nBpc)
{
	int nchnl = (1 + (colType & HAS_RGBA));
	if(colType & HAS_16BIT) nchnl <<= 1;
	return nBpc * nchnl;
}

Image& Image::initRef(const Image& that) {
	memcpyX(this, &that, 1); return *this; }
Image& Image::initRef(const Image& that, const Rect& rc0)
{	Rect rc = that.clipRect(rc0);
	bColors = (byte*)that.getPtr(rc.left, rc.top);
	width = rc.Width(); height = rc.Height();
	palette = that.palette;	pitch = that.pitch;
	CAST(u32, palSize) = CAST(u32, that.palSize); }
	
int Image::initSize(uint w, uint h, uint colType, uint nBits)
{
	if((w > 65535)||(h > 65535)) return -1;
	this->width = w; this->height = h;
	this->colType = colType; this->nBits = nBits;
	int pitch = calcPitch(width, colType);
	this->pitch = pitch; return pitch * h;
}

Image::alloc_t Image::alloc(int width,
	int height, uint colType)
{
	uint pitch = calcPitch(width, colType);
	return {malloc(pitch * height), pitch};
}

Image::alloc_t Image::alloc(uint colType)
{
	return alloc(width, height, colType);

} 

int Image::Save(LPCTSTR file, int format)
{
	// load file into memory
	FILE* fp = _tfopen(file, _T("wb"));
	if(fp == NULL)
		return NOT_FOUND;
		
	int result = SaveBitmap(fp);
	fclose(fp);
	return result;
}

bool Image::IsCompatible(const Image& that) const
{
	if( palSize != that.palSize ) return false;
	for(int i = 0; i < palSize; i++)
	  if( palette[i] != that.palette[i] )
		return false; return true;
}

void ImageObj::Delete(void) 
{
	if(hdc != NULL) {
		DeleteObject(SelectObject(hdc, GetStockObject(21)));
		DeleteObject(hdc); this->resetObj(); } else if(cColors) {
		free(cColors); free(palette); this->resetObj(); }
}
	
void ImageObj::resetObj(void) {
	memset(this, 0, sizeof(*this)); }

int ImageObj::Create(uint w, int h, u8 colType_, u8 nBits_)
{
	this->Delete(); h = abs(h);
	if((w > 65535)||(h > 65535)) return ERR_PARAM;
	width = w; height = h; colType = colType_; 
	nBits = nBits_; nBits = calcNBits();
	if(colType & HAS_PALETTE) palSize = 1<<nBits;
	return this->alloc_();
}

int ImageObj::Load(LPCTSTR fName)
{
	// load file
	auto file = loadFile(fName); if(file == NULL) return 
		isNeg(file.size) ? NOT_FOUND : ERR_ALLOC;
	SCOPE_EXIT(free(file.data));
	
	// check header
	if(file.size >= 4) {
	if(*(word*)file.data == 0x4D42)
		return LoadBmp(file.data, file.size);
	if(*(uint*)file.data == 0x38464947)
		return LoadGif(file.data, file.size);
	if(*(uint*)file.data == 0x474E5089)
		return loadPng(file.data, file.size);
	} return BAD_IMAGE;
}


// Soul transferance
int ImageObj::Create(const Image& that)
{	return Create(that, that.width, that.height); }
int ImageObj::Create(const Image& that, int w, int h)
{	
	this->Delete(); width = w; height = h;	
	CAST(u32, palSize) = CAST(u32, that.palSize);
	int result = alloc_(); if(!result)
		memcpy(palette, that.palette, palSize*4);
	return result;
}

int ImageObj::Copy(const ImageObj& that)
{	
	int result = Create(that);	if(!result) memcpy(cColors,
		that.cColors, pitch * height); return result;
}

void ImageObj::Swap(ImageObj& that)
{
	for(int i = 0; i < sizeof(ImageObj)/4; i++)
		std::swap(((DWORD*)this)[i], ((DWORD*)&that)[i]);


	//struct SwapType {
	//	DWORD data[sizeof(ImageObj)/4];	};
	//std::swap(*(SwapType*)this,
	//	*(SwapType*)&that);
}

void ImageObj::setPalette(Color* palette, uint palSize)
{
	// determin alpha type
	if(palSize == 0) goto NO_PALETTE; 
	ImageLib_assert(palSize <= 256);
	{ DWORD mask = 0;
	BYTE first = palette->GetA();
	for(Color color : Range(palette, palSize))
	  if(color.GetA() != first) goto GOT_APLHA;
	if((first == 0)||(first == 255)) mask = 0xFF000000;
	
	// copy alpha array
GOT_APLHA:
	for(int i : Range(0,(int)palSize))
		this->palette[i] = palette[i] | mask; }		
NO_PALETTE: this->palSize = palSize;
	nBits = max(nBits, snapToBits(palSize));
}


int ImageObj::alloc_()
{
	pitch = calcPitch(width, colType);
	if(colType & HAS_HBMP) {
		assert(!hasPalette()); BITMAPV5HEADER bmInfo;
		GetBitmapInfo(CAST(BmInfo256, bmInfo), true);
		hdc = CreateCompatibleDC(NULL);
		if(! SelectObject(hdc, CreateDIBSection(hdc, (BITMAPINFO*)
			&bmInfo, DIB_RGB_COLORS, (VOID**)&cColors, NULL, 0)))
		{	this->Delete(); return ERR_ALLOC; }
	} else {
		cColors = (Color*)malloc(pitch * height);
		if( cColors == NULL ) { ALLOC_ERR:
			this->Delete(); return ERR_ALLOC; }
		if(colType & HAS_PALETTE) {
			palette = (Color*)malloc(1024);
			if(palette == NULL) goto ALLOC_ERR; }
	} 	return ERR_NONE;	
}





}