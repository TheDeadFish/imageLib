// 

byte* SHITCALL rle_decodeLine8(
	byte* rlePos, byte* rleEnd,
	byte* out, byte* outEnd)
{
	while(rlePos+2 <= rleEnd) { int ch = RDI(rlePos);
	if(ch > 0) { if((out+ch) > outEnd) 
		return NULL; byte ch2 = RDI(rlePos);
		while(--ch >= 0) WRI(out, ch2);
	} else { if((ch = RDI(rlePos)) >= 3) {
		if((out+ch) > outEnd) return NULL;
		int tmp = ch; while(--ch >= 0)  WRI(out,
			RDI(rlePos)); if(tmp&1) rlePos++; }
		else { return (ch != 2) ? rlePos : 0; }
	}} return NULL;
}

byte* SHITCALL rle_decodeLine4(
	byte* rlePos, byte* rleEnd,
	byte* out, byte* outEnd)
{
	while(rlePos+2 <= rleEnd) { int ch = RDI(rlePos);
	if(ch > 0) { 
		if((out+ch) > outEnd) return NULL;
		byte ch2 = RDI(rlePos); while(--ch >= 0) {
			WRI(out, ch2>>4); if(--ch < 0) 
			break; WRI(out, ch2 & 15); }
	} else { if((ch = RDI(rlePos)) >= 3) {
		if((out+ch) > outEnd) return NULL;
		int tmp = ch; while(--ch >= 0) { 
		byte ch2 = RDI(rlePos); WRI(out, ch2>>4);
		if(--ch < 0) break; WRI(out, ch2 & 15); }
		if(tmp&3) rlePos++; }
		else { return (ch != 2) ? rlePos : 0; }
	}} return NULL;
}

struct RleRun4 { int len = 0; 
	bool ex(byte* data) { if((len >= 2)
		&&(data[0] != data[-2])) return true;
		len += 1; return false; }
	bool ex2(byte* data) { bool r = ex(data);
		if(r) len = 1; return r; }	
};

int SHITCALL rle_getRun4(byte* data, byte* end)
{
	RleRun4 run; int maxLen = min(end-data, 255);
	while((run.len < maxLen)&&(!run.ex(
		data+run.len))); return run.len;
}

int SHITCALL rle_absLen4(byte* data, byte* end)
{
	RleRun4 run; 
	
	int maxPos = min(end-data, 255);
	
	int rleIdx = 0; int rleCost = 0; 
	for(int curPos = 1; curPos 
		<= maxPos; curPos += 1)
	{
		int relPos = curPos-rleIdx;
		int absCost = ALIGN4(relPos);
		
		// 
		if(run.ex2(&data[curPos-1])) rleCost += 4;
		//	printf("%d, %d, %d\n", curPos, absCost/2, rleCost/2);
		
		
		if(!(relPos & 3)&&(absCost > rleCost)) break;
		if(absCost < rleCost) { rleCost = 4;
		run.len = 0; curPos = rleIdx += absCost; }
	}
	
	return min(rleIdx, maxPos);
}

int SHITCALL rle_encodeLineLen4(
	byte* data, int width)
{
	byte * end = data+width;
	int length = 2;
	while(data < end)
	{
		int runLen = rle_getRun4(data, end);
		
		// absolute encoding
		if(runLen < 4)  {
		int absLen = rle_absLen4(data, end);
		if(absLen) { 
			data += absLen;  length += 
			ALIGN4(absLen)/2+2; continue; }
		}
		
		// rle encoding
		length += 2;
		data += runLen;	
	}
	
	//printf("%d\n", length);
	
	return length;
}

byte* SHITCALL rle_encodeLine4(byte* out, 
	byte* data, int width)
{
	byte * end = data+width;
	while(data < end)
	{
		int runLen = rle_getRun4(data, end);

		// absolute encoding
		if(runLen < 4)  {
		int absLen = rle_absLen4(data, end);
		if(absLen) { WRI(PW(out), absLen<<8); 
			byte* out0 = out; while(--absLen >= 0) { 
			*out = *data << 4; out++; data++; VARFIX(absLen);
			if(--absLen < 0) break; out[-1] |= *data; data++;}
			out += (out0-out)&1; continue;
		}}
		
		// rle encoding
		
		byte ch = data[0] << 4;
		if(runLen > 1) ch |= data[1];
		out[0] = runLen; out[1] = ch;
		data += runLen; out += 2;
	}
	
	// end of line
	WRI(PW(out), 0);
	return out;	
}

int SHITCALL rle_absLen8(byte* data, byte* end)
{
	int maxPos = end-data; 
	if(maxPos > 254) maxPos = 254;
	
	int rleIdx = 0; int rleCost = 0; 
	int curPos = 2;

	for(; curPos <= maxPos; curPos += 1)
	{
		int relPos = curPos-rleIdx;
		int absCost = ALIGN(relPos, 1);
		
		if(data[curPos-2] != data[curPos-1]) {
			rleCost += 2; //if((relPos & 1)
			//&&(absCost == rleCost)) { rleCost = 2;
			//	rleIdx = curPos-1; 	continue; }
		}
		
		if(!(relPos & 1)&&(relPos > rleCost))
			break;
		if(absCost < rleCost) { 
			curPos = rleIdx += absCost;
			rleCost = 2; curPos += 1; }
	}
	
	return rleIdx;
}


int SHITCALL rle_getRun8(byte* data, byte* end)
{
	int maxLen = end-data;
	if(maxLen > 255) maxLen = 255;
	int runLen = 1; byte val = *data;
	while((runLen < maxLen)
	&&(data[runLen] == val))
		runLen++;
	return runLen;
}

int SHITCALL rle_encodeLineLen8(
	byte* data, int width)
{
	byte * end = data+width;
	int length = 2;
	while(data < end)
	{
		int runLen = rle_getRun8(data, end);
		
		// absolute encoding
		if(runLen == 1)  {
		int absLen = rle_absLen8(data, end);
		if(absLen) { data += absLen; length += 
			ALIGN(absLen,1)+2; continue; }
		}
		
		// rle encoding
		length += 2;
		data += runLen;	
	}
	
	return length;	
}

byte* SHITCALL rle_encodeLine8(byte* out, 
	byte* data, int width)
{
	byte * end = data+width;
	while(data < end)
	{
		int runLen = rle_getRun8(data, end);
		
		// absolute encoding
		if(runLen == 1)  {
		int absLen = rle_absLen8(data, end);
		if(absLen) { WRI(PW(out), absLen<<8); 
			int absLen1 = absLen;
			while(--absLen >= 0) WRI(out, RDI(data));
			if(absLen1 & 1) WRI(out, 0); continue;
		}}
		
		// rle encoding
		WRI(out, runLen);
		WRI(out, *data);
		data += runLen;
	}
	
	// end of line
	WRI(PW(out), 0);
	return out;	
}

// 




// helper functions
static inline bool rle_rleMode(BITMAPFILEHEADER* fh) {
	return RI(&fh->bfReserved1); }
static inline auto rle_lineFunc(BITMAPFILEHEADER* fh) {
	return (((LPBITMAPINFOHEADER)(fh+1))->biCompression
		== BI_RLE8) ?  rle_encodeLine8 : rle_encodeLine4; }
static inline int rle_maxLine(BITMAPFILEHEADER* fh) {
	return release(RI(&fh->bfReserved1)); }
		
void rle_initBmpHeader(BITMAPFILEHEADER* fh,
	byte* data, byte* dataEnd, int pitch)
{
	LPBITMAPINFOHEADER bi = Void(fh+1);
	if(is_one_of(bi->biBitCount, 4, 8)) {
		auto fn = (bi->biBitCount == 8) ?
		rle_encodeLineLen8 : rle_encodeLineLen4;
	int rleSize = 0; int lineMax = 0;
	while(dataEnd > data) { int len = 
		fn(dataEnd -= pitch, bi->biWidth);
		max_ref(lineMax, len); rleSize += len; }
	if(fh->bfSize > rleSize) {
		RI(&fh->bfReserved1) = lineMax;
		fh->bfSize = bi->biSizeImage = rleSize;
		bi->biCompression = (bi->biBitCount == 8)
			? BI_RLE8 : BI_RLE4; }
	}
}
