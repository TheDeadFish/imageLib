
namespace ImageLib{

byte* REGCALL(3) btPosterizeLine(byte* src,
	byte* dst, size_t arg, int width)
{
	arg = 8-arg; byte* dstPos = dst;
	while(--width >= 0) { for(int i = 0; i < 3; i++) {
		RB(dstPos, i) = RB(src, i) >> arg; }
		RB(dstPos, 3) = 0xFF; src += 4; dstPos += 4; }
	return dst;
}

byte* REGCALL(3) unPosterizeLine(byte* src,
	byte* dst, size_t arg, int width)
{
	static const u16 scales[8] = { 65280, 21760,
		9326, 4352, 2106, 1037, 515, 256 };
	uint scale = scales[arg-1]; byte* dstPos = dst;
	while(--width >= 0) { for(int i = 0; i < 3; i++) {
			RB(dstPos, i) = (RB(src, i) * scale) >> 8; }
		RB(dstPos, 3) = 0xFF; src += 4; dstPos += 4; }
	return dst;
}


byte* REGCALL(3) posterizeLine(byte* src,
	byte* dst, size_t arg, int width)
{
	dst = btPosterizeLine(src, dst, arg, width);
	return unPosterizeLine(dst, dst, arg, width);
}
}
