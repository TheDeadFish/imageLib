// DeadFish image library
// Gif File functions

#include <malloc.h>
#include "imageLib.h"
namespace ImageLib{

struct GifLzwDecodeState
{
	// line output state
	uint8_t *rowp, *rowend, *imgend;
	int extra, pitch; int mColorMask;

	// decoder state
	enum { MAX_LZW_BITS = 12, MAX_BITS = 4097 };
    uint8_t *stackp; int datasize; int codesize; int codemask;
	int avail; int oldcode; uint8_t firstchar; int bits; int32_t datum; 
	uint16_t* prefix; uint8_t* suffix; uint8_t* stack;
	
	GifLzwDecodeState() { ZINIT; }
	~GifLzwDecodeState() { free(prefix); }
	int init(Image::alloc_t imgData, 
		byte minSize, int w, int y, int nColorTable)
	{
		// line output state
		mColorMask = nColorTable-1;
		rowp = imgData.data; pitch = imgData.pitch;
		rowend = rowp + pitch; extra = pitch-w;
		imgend = rowp + pitch*y;
	
		// decoder state
		datasize = minSize;
		const int clear_code = (1 << datasize);
		if((datasize > MAX_LZW_BITS)
		||(clear_code >= MAX_BITS))
			return ImageObj::BAD_IMAGE;
		avail = clear_code + 2;	oldcode = -1;
		codesize = datasize + 1;
		codemask = (1 << codesize) - 1;
		firstchar = 0; bits = 0; datum = 0;
		
		if(!(prefix = malloc(4*4098)))
			return ImageObj::ERR_ALLOC;
		suffix = (uint8_t*)&prefix[4098];
		stackp = stack = (uint8_t*)&suffix[4098];
		for (int i = 0; i < clear_code; i++)
			suffix[i] = i;
		return 0;
    }

	bool decodeBlock(uint8_t* ch, uint8_t* ech)
	{
		if(rowp == imgend) return false;
		#define OUTPUT_ROW() { rowp += extra; \
			rowend += pitch; if(rowp == imgend) return true; }

		const int clear_code = 1 << datasize;
		for (; ch < ech; ch++) { datum += ((int32_t) *ch) << bits; bits += 8;
		while (bits >= codesize) { int code = datum & codemask;
			datum >>= codesize; bits -= codesize;
	    if (code == clear_code) { codesize = datasize + 1;
        codemask = (1 << codesize) - 1; avail = clear_code + 2;
		oldcode = -1; continue; }
		if (code == (clear_code + 1)) { return (rowp == imgend); }
		if (oldcode == -1) { if (code >= MAX_BITS) return false;
        *rowp++ = suffix[code] & mColorMask; if (rowp == rowend) OUTPUT_ROW();
        firstchar = oldcode = code; continue; }
		int incode = code; if (code >= avail) { *stackp++ = firstchar;
        code = oldcode; if (stackp >= stack + MAX_BITS) return false; }
		while (code >= clear_code) { if ((code >= MAX_BITS) || (code == prefix[code]))
			return false; *stackp++ = suffix[code]; code = prefix[code];
		if (stackp == stack + MAX_BITS) return false;}
		*stackp++ = firstchar = suffix[code]; if (avail < 4096) {
        prefix[avail] = oldcode; suffix[avail] = firstchar; avail++;
        if (((avail & codemask) == 0) && (avail < 4096)) { codesize++;
          codemask += avail; }} oldcode = incode;
		do { *rowp++ = *--stackp & mColorMask; if (rowp == rowend)
          OUTPUT_ROW(); } while (stackp > stack); }
		} return true;
	}
};

template<class T,class U> T pstAdd(T&ptr,U delta) {
	T tmpPtr=ptr; ptr+=delta; return tmpPtr; }
#define IFRET(...) if(auto result = __VA_ARGS__) return result;
	
struct VoidLen : Void { int len; VoidLen() = default;
	VoidLen(Void buff, int l) : Void(buff), len(l) {} };
struct VoidRng : Void { Void end; VoidRng() = default;
	VoidRng(Void buff, Void e) : Void(buff), end(e) {}
	VoidRng(Void buff, int l) : Void(buff), end(buff+l) {}
	int len() { return offset(end); } 
	VoidLen skip(int len) { char* oldPos = pstAdd(data, len);
		return VoidLen(oldPos, offset(end)); }
	VoidLen skipl(int len) { char* oldPos = pstAdd(data, len);
		return VoidLen(oldPos,len); }
	VoidRng skipr(int len) { char* oldPos = pstAdd(data, len);
		return VoidRng(oldPos, data); }
		
	bool read(void* buff, int len) { VoidLen data = skip(len); if(data.len < len)
		return false;  memcpy(buff, data.data, len); return true; }
};

struct GceInfo
{
	u16 delayTime; u8 transIndex;
	union { u8 flags; struct {
		u8 transFlag : 1; u8 uinpFlag : 1;
		u8 dispMthd : 3; u8 reserved : 3; 
	}; };
	uint transIdx() { return transFlag 
		? transIndex : -1; }
};

struct GifFrmInfo
{
	GceInfo gceInfo; u16 x, y, w, h; 
	u16 nColorTable; byte* colorTable;
	Image::alloc_t imgData;
	int parse(VoidRng& curPos,
		GceInfo& gceInfo, int nGloblTable);
};

struct GifInfo
{
	u16 width, height; u8 bkcol, aspect;
	u16 nColorTable; byte* colorTable;
	int frameCount; GifFrmInfo* frmList;
	
	GifInfo() { ZINIT; }
	~GifInfo() { free(colorTable);
		for(auto frm : Range(frmList, frameCount)) {
		free(frm.colorTable); free(frm.imgData.data); }
	} int load(Void data, int size);
};

static
int __stdcall gifReadColors(VoidRng& curPos,
	byte*& colorTable, u16& nColors_, char flags)
{
	if(flags >= 0) { nColors_ = 0; return 0; }
	int nColors = 2 << (flags & 7);	nColors_ = nColors;
	if(!(colorTable = malloc(3*nColors)))
		return ImageObj::ERR_ALLOC;
	if(!curPos.read(colorTable, 3*nColors))
		return ImageObj::BAD_IMAGE; return 0;
}

static
VoidRng __thiscall gifReadData(VoidRng& curPos_) 
{
	Void curPos = curPos_; int avail = curPos_.len();
	int blen = curPos[0]; if(avail <= blen) return VoidRng(0, 0);
	SCOPE_EXIT(curPos_.data = curPos); 
	Void retPos = curPos += 1; curPos += blen;
	if(!blen) asm("xor %0,%0" : "=r"(retPos));
	return VoidRng(retPos, curPos);
}

int GifFrmInfo::parse(VoidRng& curPos,
	GceInfo& gceInfo, int nGloblTable)
{
	// parse Image Descriptor
	if(this == NULL) return ImageObj::ERR_ALLOC;
	colorTable = 0; imgData.data = 0;
	memcpy(&this->gceInfo, &gceInfo, sizeof(gceInfo));
	memset(&gceInfo, 0, sizeof(gceInfo));
	VoidLen imgInfo = curPos.skip(11);
	if(imgInfo.len < 11) return ImageObj::BAD_IMAGE;
	CAST(INT64, x) = imgInfo.Ref<INT64>(1);
	IFRET(gifReadColors(curPos, colorTable,
		nColorTable, imgInfo[9]));
		
	// parse Image Data
	imgData = Image::alloc(w, h, Image::HAS_PALETTE);
	if(imgData.data == NULL) return ImageObj::ERR_ALLOC;
	int nColors = nColorTable ? nColorTable : nGloblTable;
	if(nColors == 0) return ImageObj::BAD_IMAGE;
	GifLzwDecodeState lzwState; IFRET(lzwState.init(
		imgData, imgInfo[10], w, h, nColors));
	VoidRng block; while(block = gifReadData(curPos)) {
		if(!lzwState.decodeBlock((byte*)block.data, (byte*)block.end))
			return ImageObj::BAD_IMAGE;
	} return (block.end == 0) ? ImageObj::BAD_IMAGE : 0;
}

int GifInfo::load(Void data, int size)
{
	// parse header
	VoidRng curPos(data, size);
	VoidLen header = curPos.skip(13);
	if(header.len < 13) return ImageObj::BAD_IMAGE;
	CAST(u32, width) = header.Dword(6);
	CAST(u16,bkcol) = header.Dword(11);
	IFRET(gifReadColors(curPos, colorTable,
		nColorTable, header[10]));

	// parse blocks
	GceInfo gceInfo = {0};
NEXT_BLOCK: VoidLen block = curPos.skip(0);
	if(block.len <= 0) return ImageObj::BAD_IMAGE; 
	switch(block[0]) {
	case 0x3B: return 0;
	case 0x2C:
		IFRET(NextAlloc(frmList, frameCount).
			parse(curPos, gceInfo, nColorTable));		
		goto NEXT_BLOCK;
	case 0x21:
		if(block.len < 3) return ImageObj::BAD_IMAGE;
		switch(block[1]) {
		case 0xF9: if(block.len < 8) return ImageObj::BAD_IMAGE;
			gceInfo.delayTime = block.Word(4);
			gceInfo.transIndex = block.Byte(6);
			gceInfo.flags = block.Byte(3);
			curPos.skip(8); goto NEXT_BLOCK;;
		default: {
			curPos.skip(2); VoidRng block;
			while(block = gifReadData(curPos)) {}
			if(block.end == 0) return ImageObj::BAD_IMAGE;
			goto NEXT_BLOCK; }
		}
	default:
		return ImageObj::BAD_IMAGE;
	}
}

#include <malloc.h>

int ImageObj::LoadGif(void* data, int size)
{
	// load gif image
	GifInfo gifInfo;
	IFRET(gifInfo.load(data, size));
	if(gifInfo.frameCount != 1) return USP_IMAGE;
	auto& frmInfo = gifInfo.frmList[0];
	if(frmInfo.x || frmInfo.y || (frmInfo.w != gifInfo.width)
	||( frmInfo.h != gifInfo.height)) return USP_IMAGE;
	colType = HAS_PALETTE | HAS_COLOR;

	// copy palette
	if(!(palette = malloc(1024))) return ERR_ALLOC;
	byte* gifPal = gifInfo.colorTable;
	int palSize = gifInfo.nColorTable;
	if(palSize == 0) { gifPal = frmInfo.colorTable;
		palSize = frmInfo.nColorTable; }
	for(int i = 0; i < palSize; i++) { palette[i].SetValue(
		255, gifPal[0], gifPal[1], gifPal[2]); gifPal += 3; }
	uint transIdx = frmInfo.gceInfo.transIdx();
	if(transIdx < palSize) { colType |= HAS_ALPHA;	
		palette[transIdx].SetA(0); }
	this->palSize = palSize;
	
	// initialize remaining field
	bColors = release(frmInfo.imgData.data);
	pitch = frmInfo.imgData.pitch;
	width = frmInfo.w; height = frmInfo.h;
	nBits = 8; return 0;
}
}
