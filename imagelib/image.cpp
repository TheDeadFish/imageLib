#include <algorithm>
#include "imageLib.h"

namespace ImageLib{

int Image::alphaType(void) const
{
	return (colType & HAS_PALETTE) ?
		palette->alphaType(palSize) :
		cColors->alphaType(width, height, pitch);
}

int Image::calcNBits(int colType) const
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
{	memcpyX(this, &that, 1); Rect rc; that.clipRect(rc, rc0);
	bColors = (byte*)that.getPtr(rc.left, rc.top);
	width = rc.Width(); height = rc.Height();  return *this; }
	
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

int Image::Save(LPCSTR fName, int format, void* opts) {
	FILE* fp = fopen(fName, "wb"); if(!fp) return NOT_FOUND;
	SCOPE_EXIT(fclose(fp)); return Save((FileOut*) fp, format, opts); }
int Image::Save(xarray<byte>& data, int format, void* opts) {
	FileOut fo; int result = Save(&fo, format, opts);
	if(result == 0) { data = fo.getData(); } else { 
	free(fo.data); data = {0,0}; } return result; }	
int Image::Save(FileOut* fo, int format, void* opts) {
	return SaveBitmap(fo, opts); }

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


int ImageObj::Create(uint w, int h, int colType_) {
	return Create(w, h, colType_, 0, 0); }
int ImageObj::CreateDib(uint w, int h, bool alpha) {
	return Create(w, h, alpha ? TYPE_ARGB8 : TYPE_RGB8, 0, true); }	
	
int ImageObj::Create(uint w, int h, 
	int colType_, int nBits_, BOOL dibMode)
{
	this->Delete(); h = abs(h);
	if((w > 65535)||(h > 65535)) return ERR_PARAM;
	width = w; height = h; colType = colType_; 
	nBits = nBits_; nBits = calcNBits();
	pitch = calcPitch(width, colType);
	
	// allocate pixels
	if(dibMode) { BITMAPV5HEADER bmInfo;
		GetBitmapInfo(CAST(BmInfo256, bmInfo), true);
		if((!(hdc = CreateCompatibleDC(NULL)))
		||(! SelectObject(hdc, CreateDIBSection(hdc, (BITMAPINFO*)
			&bmInfo, DIB_RGB_COLORS, (VOID**)&cColors, NULL, 0))))
		{	ALLOC_ERR: this->Delete(); return ERR_ALLOC; }
	} else { cColors = (Color*)malloc(pitch * height);
		if( cColors == NULL ) goto ALLOC_ERR; }
	
	// allocate palette
	if(colType & HAS_PALETTE) {
		palSize = 1<<nBits;	
		palette = (Color*)malloc(1024);
		if(palette == NULL) goto ALLOC_ERR; }
	return ERR_NONE;
}

int ImageObj::Create(uint w, int h,
	Color* p, int psz, BOOL dibMode) {
	IFRET(Create(w, h, HAS_PALETTE, 0, dibMode));
	setPalette(p, psz); return 0; }
int ImageObj::Create(const Image& that, Color* p, int psz) {
	return Create(that.width, that.height, p, psz); }

int ImageObj::Load(void* data, int size)
{
	// check header
	if(size >= 4) {
	if(*(word*)data == 0x4D42)
		return LoadBmp(data, size);
	if(*(uint*)data == 0x38464947)
		return LoadGif(data, size);
	if(*(uint*)data == 0x474E5089)
		return loadPng(data, size);
	} return BAD_IMAGE;	
}

int ImageObj::Load(LPCSTR fName)
{
	// load file
	auto file = loadFile(fName); if(file == NULL) return 
		isNeg(file.size) ? NOT_FOUND : ERR_ALLOC;
	SCOPE_EXIT(free(file.data));
	return Load(file.data, file.size);
}

// Soul transferance
int ImageObj::Create(const Image& that, bool dibMode)
{	return Create(that, that.width, that.height, dibMode); }
int ImageObj::Create(const Image& that, int w, int h, bool dibMode)
{	
	IFRET(Create(w, h, that.colType, that.nBits, 0));
	if(colType & HAS_PALETTE) setPalette(that.palette, palSize);
	return 0;
}

int ImageObj::Copy(const ImageObj& that, bool dibMode)
{	
	IFRET(Create(that, dibMode)); memcpy(cColors,
		that.cColors, pitch * height); return 0;
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
	ImageLib_assert((palSize <= 256)
		&&(colType & HAS_PALETTE));
	DWORD mask = palette->alphaType(
		palSize) ? 0xFF000000 : 0;
		
	// copy alpha array
	for(int i : Range(0,(int)palSize))
		this->palette[i] = palette[i] | mask;
	this->palSize = palSize;
	nBits = max(nBits, snapToBits(palSize));
	
	// update DIB color table
	if(hdc) { SetDIBColorTable(hdc,
		0, palSize, (RGBQUAD*)palette); }
}

BOOL Image::clipRect(RECT& dst, const RECT& src) const {
	RECT rc = {0,0,width,height}; 
	return IntersectRect(&dst, &src, &rc); }
	
const byte* Image::getPtr(int x, int y) const {
	const byte* line = getLn(y);
	if(hasPalette()) return line+x;
	ei(colType & HAS_16BIT) return line+x*8;
	else return line+x*4; }

ASM_FUNC("Image_pixSize", "movzbl 22(%eax), %eax;"
	"test $8, %al; jne 1f; and $8, %eax; add $4, "
	"%eax; ret; 1: mov $1, %eax; ret;");

}
