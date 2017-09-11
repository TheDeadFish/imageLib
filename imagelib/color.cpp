// Color manipulation functions
#include "stdshit.h"
#include "color.h"

#define COLORX_DEFFUNC(ret, func, ...) ColorX_TMPX(ret) func  __VA_ARGS__  \
	template ret ColorX_<BYTE,DWORD> func; template ret ColorX_<WORD,DWORD64> func; 
	
COLORX_DEFFUNC(void, ::toolBlend(mem_t alpha, ColorX_ color), 
{
	for(int i = 0; i < 4; i++) data[i] = 
		alphaBlend(alpha, color[i], data[i]);
})
	
COLORX_DEFFUNC(void, ::bkgdBlend(ColorX_ bkgdBlend),
{
	mem_t alpha = GetA();
	for(int i = 0; i < 3; i++) data[i] = 
		alphaBlend(alpha, data[i], bkgdBlend[i]);
	SetA(-1);
})

COLORX_DEFFUNC(auto, ::sideMinMax(SideMinMax* __restrict__ result,
	int length, int pitch)  __restrict__ -> SideMinMax*,
{
	result->maxCorner = *this; result->minCorner = *this;
	ColorX_* colors = this;
	while(--length)
	{
		*(byte**)&colors += pitch;
		for(int i = 0; i < 4; i++) {
			max_ref(result->maxCorner[i], colors->data[i]);
			min_ref(result->minCorner[i], colors->data[i]);
		}
	}
	return result;
	
})

COLORX_DEFFUNC(auto, ::sideLength(SideLength* __restrict__ result,
	int length, int pitch) __restrict__ -> SideLength*,
{
	result = (SideLength*)sideMinMax((SideMinMax*)result, length, pitch);
	result->sideLen.SetA(result->maxCorner.GetA() - result->minCorner.GetA());
	result->sideLen.SetR(result->maxCorner.GetR() - result->minCorner.GetR());
	result->sideLen.SetG(result->maxCorner.GetG() - result->minCorner.GetG());
	result->sideLen.SetB(result->maxCorner.GetB() - result->minCorner.GetB());
	return result;
})

COLORX_DEFFUNC(auto, ::longestSide(int length, int pitch) -> side_t,
{
	SideLength sideLen; sideLength(
		&sideLen, length, pitch);
	
	side_t ret = {0,0};
	int alpha = sideLen.maxCorner.GetA();
	length = sideLen.sideLen.getRef(3)*255;
	if(ret.length < length) { ret = {3, length}; }
	length = sideLen.sideLen.getRef(1)*alpha;
	if(ret.length < length) { ret = {1, length}; }
	length = sideLen.sideLen.getRef(2)*alpha;
	if(ret.length < length) { ret = {2, length}; }
	length = sideLen.sideLen.getRef(0)*alpha;
	if(ret.length < length) { ret = {0, length}; }
	return ret;
})

COLORX_DEFFUNC(bool, ::isUnique(int count, ColorX_ color),
{
	for(int j = 0; j < count; j++)
		if(this[j] == color) return false;
	return true;
})

COLORX_DEFFUNC(int, ::makeUnique(int length),
{
	int count = 0;
	for(int i = 0; i < length; i++) {
		ColorX_ curColor = this[i];
		for(int j = 0; j < count; j++)
			if(this[j] == curColor) goto FOUND_COLOR;
		this[count++] = curColor;
	FOUND_COLOR:; } return count;
})

COLORX_DEFFUNC(int, ::diff3(const ColorX_& that),
{
	return (r - that.r)*(r - that.r) + (g - that.g)*
		(g - that.g) + (b - that.b)*(b - that.b);
})

COLORX_DEFFUNC(int, ::alphaType(int length),
{
	if(length == 0) return -1;
	mem_t first = this->GetA();
	for(int i = 0; i < length; i++) {
		if(this[i].GetA() != first) return 0; }
	if(first == mem_t(-1)) return -1;
	if(first == 0) return 1; return 0;
})

COLORX_DEFFUNC(int, ::alphaType(int w, int h, int p),
{
	auto* linePos = this; int type = -1;
	while(--h >= 0) { int t2 = linePos->alphaType(w);
		if(type != t2) { if(!t2) return t2;
			if(linePos != this) break; type = t2;
		} PTRADD(linePos, p); } return type;
})

// Color sort forward
static int color_sort0(const void* a, const void* b) { return RB(a,0) - RB(b,0); }
static int color_sort1(const void* a, const void* b) { return RB(a,1) - RB(b,1); }
static int color_sort2(const void* a, const void* b) { return RB(a,2) - RB(b,2); }
static int color_sort3(const void* a, const void* b) { return RB(a,3) - RB(b,3); }
static int (*const color_sortFunc[4]) (const void*, const void*) = {
	color_sort0, color_sort1, color_sort2, color_sort3};
void Color::sort(int index, int length, int pitch) {
	qsort(this, length, pitch, color_sortFunc[index]); }
	
// Color sort backwards
static int color_sortr0(const void* a, const void* b) { return RB(b,0) - RB(a,0); }
static int color_sortr1(const void* a, const void* b) { return RB(b,1) - RB(a,1); }
static int color_sortr2(const void* a, const void* b) { return RB(b,2) - RB(a,2); }
static int color_sortr3(const void* a, const void* b) { return RB(b,3) - RB(a,3); }
static int (*const color_sortrFunc[4]) (const void*, const void*) = {
	color_sortr0, color_sortr1, color_sortr2, color_sortr3};
void Color::sortr(int index, int length, int pitch) {
	qsort(this, length, pitch, color_sortrFunc[index]); }
	
	

/*


float Color::GammaExpand_sRGB(int nonlin8)
{    
	float nonlinear = nonlin8 / 255.0;
    return   ( nonlinear <= 0.04045f )
           ? ( nonlinear / 12.92f )
           : ( powf( (nonlinear+0.055f)/1.055f, 2.4f ) );
}

int Color::GammaCompress_sRGB(float linear)
{    
    float nonlinear = ( linear <= 0.0031308f )
           ? ( 12.92f * linear )
           : ( 1.055f * powf( linear, 1.0f/2.4f ) - 0.055f );
	return lrint(nonlinear * 255);
}

WORD Color::toLin[256]; BYTE Color::toSrgb[65536];
__attribute__((constructor))
void Color_initTable(void) { for(int i = 0; i < 256; i++)
		Color::toLin[i] = lrintf(Color::GammaExpand_sRGB(i) * 65535);
	for(int i = 0; i < 65536; i++) {
		Color::toSrgb[i] = Color::GammaCompress_sRGB(i / 65535.0F); }
}
*/
