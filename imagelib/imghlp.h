// DeadFish image library
// helpers routines & structures
namespace ImageLib {

// packed pixel types
#define DEF_PAKPIX(nm,it,ot,cp) struct nm { UNPAREN(it); void get(\
	byte* col) { ((ot*)col)cp; } enum { osz = sizeof(ot) }; };
DEF_PAKPIX(IDX8, (byte v), byte, [0]=v);
DEF_PAKPIX(GRY8, (byte v), Color, ->SetValue(-1,v,v,v));
DEF_PAKPIX(GRYA8, (byte v,a), Color, ->SetValue(a,v,v,v));
DEF_PAKPIX(RGB8, (byte r,g,b), Color, ->setRGB(this));
DEF_PAKPIX(RGBA8, (byte r,g,b,a), Color, ->setRGBA(this));
DEF_PAKPIX(GRY16B, (u16 v), Color16, ->setGRY16B(v));
DEF_PAKPIX(GRYA16B, (u16 v,a), Color16, ->setGRYA16B(a,v));
DEF_PAKPIX(RGB16B, (u16 r,g,b), Color16, ->setRGB16B(this));
DEF_PAKPIX(RGBA16B, (u16 r,g,b,a), Color16, ->setRGBA16B(this));

struct NotRect : Rect
{
	template <bool f>
	static Rect& GetX(Rect& This, int step,	const Rect& rc, int width, int height) { switch(step) {
	case 0: This.Set2<1|f,1|f,1|f,1|f>(0, 0, width, rc.top); return This;
	case 1:	This.Set2<0|f,1|f,0|f,1|f>(0, rc.bottom, width, height); return This;
	case 2:	This.Set2<1|f,1|f,0|f,1|f>(rc.right, rc.top, width, rc.bottom);	return This;
	default: This.Set2<1|f,0|f,1|f,0|f>(0, rc.top, rc.left, rc.bottom);	return This; }}
	REGCALL(3) static Rect Get(int step, const Rect& rc, int width, int height);
	template <int step> static Rect Get(const Rect& rc, int width, int height) {
		Rect This; return GetX<0>(This, step, rc, width, height); }
	Rect& Init(int step, const Rect& rc, int width, int height) {
		return GetX<1>(*this, step, rc, width, height); }
	Rect& Init2(int step, const Rect& rc, int width, int height) {
		return GetX<0>(*this, step, rc, width, height); }
};





SHITCALL int snapToBits(int val);


// bitmap helpers
struct BmInfo256 : BITMAPV5HEADER { RGBQUAD bmiColors_[256]; 
	LPBITMAPINFO operator&() { return (LPBITMAPINFO)this; }
	RGBQUAD& bmiColors(int idx) { return ((RGBQUAD*)(((void*)this)+bV5Size))[idx]; }
	int getSize() { return PTRDIFF(&bmiColors(bV5ClrUsed), this); } };

// line-packing
void REGCALL(2) convLine8To1(byte* src, byte* dst, int width);
void REGCALL(2) convLine8To2(byte* src, byte* dst, int width);
void REGCALL(2) convLine8To4(byte* src, byte* dst, int width);
void REGCALL(2) convLine32To24(byte* src, byte* dst, int width);
void REGCALL(2) convLine64To48(byte* src, byte* dst, int width);
typedef void (REGCALL(2) *linePackFunc_t)(byte* src, byte* dst, int width);
linePackFunc_t __stdcall linePackFunc(int nBits);

// bgra line-unpacking
void REGCALL(2) convLine1To8(byte* src, byte* dst, int width);
void REGCALL(2) convLine2To8(byte* src, byte* dst, int width);
void REGCALL(2) convLine4To8(byte* src, byte* dst, int width);
void REGCALL(2) convLine8To8(byte* src, byte* dst, int width);
void REGCALL(2) convLine24To32(byte* src, byte* dst, int width);
void REGCALL(2) convLine32To32(byte* src, byte* dst, int width);
void REGCALL(2) convLine48To64(byte* src, byte* dst, int width);
void REGCALL(2) convLine64To64(byte* src, byte* dst, int width);
linePackFunc_t __stdcall lineUnpackFunc(int nBits);

// rgba line-unpacking
DEF_RETPAIR(convLine_t, byte*, src, byte*, dst);
#define convLineXX_(n) convLine_t REGCALL(3) MCAT( \
	convLine1,n)(byte* src, byte* dst, int delta, int width);
convLineXX_(1To8_); convLineXX_(2To8_); convLineXX_(4To8_);
convLineXX_(8To8_); convLineXX_(8To32_); convLineXX_(16To64_);
convLineXX_(16To32_); convLineXX_(32To64_); convLineXX_(24To32_);
convLineXX_(48To64_); convLineXX_(32To32_); convLineXX_(64To64_);
typedef convLine_t (REGCALL(3) *linePackFunc2_t)(byte*, byte*, int, int);
linePackFunc2_t __stdcall lineDeintFunc(u8 u8colType, u8 bpp); 

// pixel format conversion
typedef byte* (REGCALL(3) *LineConvFunc2)(byte*, byte*, size_t, int);
byte* REGCALL(2) convLineTo32(Image& img, byte* src, byte* dst);
byte* REGCALL(3) copyLine32(byte* src, byte* dst, size_t arg, int width);
void SHITCALL convImgTo32(Image& img, byte* dst, int pitch,
	LineConvFunc2 fn = &copyLine32, size_t arg = 0);
void SHITCALL convImgTo32(Image& src, Image& dst,
	LineConvFunc2 fn = &copyLine32, size_t arg = 0);
	
uint maxIndex(Image& image);



	

}
