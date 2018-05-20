/*
	Code written by James William Fletcher. 
	Copyright 2010 - 2011. 
	purrr@coy.cat
	http://voxdsp.com/

	This is a fixed point 32 bit 2D graphics class.
*/

#ifndef GFX32_H
#define GFX32_H

#include <windows.h>
#include <stdio.h>
#include <string.h>

/*
	*****************************************************************************************************
												GFX32 FUNCTIONS
	*****************************************************************************************************
*/

//Containers
struct RGBA32;
struct Rect;
struct SURFACE32;

//Image Recognition
bool GFX32_FindImagePosition(SURFACE32* src, SURFACE32* find, SURFACE32* findtollerancemap, unsigned int* x, unsigned int* y);

//Buffer Conversion
void GFX32_ConvertHWNDtoGFX32(HWND hWnd, SURFACE32* dest);
void GFX32_ConvertBMPtoGFX32(const char* bitmappath, SURFACE32* dest);

//Clear Buffer
void GFX32_ClearBuffer(SURFACE32* dest, RGBA32 clr);
void GFX32_ClearBuffer(SURFACE32* dest);

//Buffer Access
void GFX32_BlendPixel(SURFACE32* dest, UINT32 x, UINT32 y, RGBA32 clr);
void GFX32_SetPixel(SURFACE32* dest, UINT32 x, UINT32 y, RGBA32 clr);
RGBA32 GFX32_GetPixel(SURFACE32* src, UINT32 x, UINT32 y);

//Buffer Allocation / Deallocation
void GFX32_CreateSurface(UINT32 w, UINT32 h, SURFACE32* dest);
void GFX32_FreeSurface(SURFACE32* src);
void GFX32_ReplaceSurface(SURFACE32* src, SURFACE32* dest);

//Bit-Block Transfer
void GFX32_Blit(SURFACE32* src, SURFACE32* dest);
void GFX32_KeyBlit(SURFACE32* src, SURFACE32* dest, UINT32 ix, UINT32 iy, RGBA32 key);
void GFX32_LineBlit(SURFACE32* src, SURFACE32* dest, UINT32 ix, UINT32 iy);
void GFX32_LineBlitEx(SURFACE32* src, Rect area, SURFACE32* dest, UINT32 ix, UINT32 iy);
void GFX32_AlphaBlit(SURFACE32* src, SURFACE32* dest, UINT32 ix, UINT32 iy);
void GFX32_AlphaBlitEx(SURFACE32* src, Rect area, SURFACE32* dest, UINT32 ix, UINT32 iy);
void GFX32_AlphaBlendBlit(SURFACE32* src, Rect area, SURFACE32* dest, UINT32 ix, UINT32 iy, UINT32 alpha);
void GFX32_AlphaSaturateBlit(SURFACE32* src, Rect area, SURFACE32* dest, UINT32 ix, UINT32 iy, UINT32 alpha);
void GFX32_TransparentBlit(SURFACE32* src, SURFACE32* dest, UINT32 ix, UINT32 iy, UINT32 alpha);

//Image Transformations
void GFX32_SetHSL(SURFACE32* surf, const float h, const float s, const float l);
void GFX32_Invert(SURFACE32* surf);

//Shape Functions
void GFX32_DrawCircle(SURFACE32* dest, UINT32 x0, UINT32 y0, UINT32 radius, RGBA32 clr);
void GFX32_DrawLine(SURFACE32* dest, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2, RGBA32 clr);
void GFX32_DrawHLine(SURFACE32* dest, UINT32 x1, UINT32 x2, UINT32 y, RGBA32 clr);
void GFX32_DrawVLine(SURFACE32* dest, UINT32 y1, UINT32 y2, UINT32 x, RGBA32 clr);

//Alpha Shape Functions
void GFX32_FillRectAlpha(SURFACE32* dest, Rect area, RGBA32 clr);
void GFX32_DrawCircleAlpha(SURFACE32* dest, UINT32 x0, UINT32 y0, UINT32 radius, RGBA32 clr);
void GFX32_DrawLineAlpha(SURFACE32* dest, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2, RGBA32 clr);
void GFX32_DrawHLineAlpha(SURFACE32* dest, UINT32 x1, UINT32 x2, UINT32 y, RGBA32 clr);
void GFX32_DrawVLineAlpha(SURFACE32* dest, UINT32 y1, UINT32 y2, UINT32 x, RGBA32 clr);

//Misc Functions
void GFX32_ReplaceColour(SURFACE32* dest, Rect area, RGBA32 find_clr, RGBA32 new_clr);

/*
	*****************************************************************************************************
												RGBA32 Container
	*****************************************************************************************************
*/

//RGBA Colour
struct RGBA32
{
	//Variables
	UINT8 r,g,b,a;

	//Constructor
	RGBA32() : r(0), g(0), b(0), a(0) {}
	RGBA32(const UINT8 ir, const UINT8 ig, const UINT8 ib, const UINT8 ia);
	RGBA32(const UINT8 ir, const UINT8 ig, const UINT8 ib);

	//Set Colour Values
	void set(const UINT8 ir, const UINT8 ig, const UINT8 ib, const UINT8 ia);

	//Initalize to NULL
	void initNULL();

	//Equality Check
	bool operator==(const RGBA32 &rhs);

	//Assignment
	void operator=(const RGBA32 &rhs);
};

inline RGBA32::RGBA32(UINT8 ir, UINT8 ig, UINT8 ib)
{
	r = ir, g = ig, b = ib, a = 255;
}

inline RGBA32::RGBA32(UINT8 ir, UINT8 ig, UINT8 ib, UINT8 ia)
{
	r = ir, g = ig, b = ib, a = ia;
}

inline void RGBA32::set(const UINT8 ir, const UINT8 ig, const UINT8 ib, const UINT8 ia)
{
	r = ir, g = ig, b = ib, a = ia;
}

inline void RGBA32::initNULL()
{
	r = 0, g = 0, b = 0, a = 0;
}

inline bool RGBA32::operator==(const RGBA32 &rhs)
{
	if(rhs.r == r && rhs.g == g && rhs.b == b && rhs.a == a){return 1;}
	return 0;
}

inline void RGBA32::operator=(const RGBA32 &rhs)
{
	r = rhs.r, g = rhs.g, b = rhs.b, a = rhs.a;
}

/*
	*****************************************************************************************************
											RECTANGLE Container
	*****************************************************************************************************
*/

//Rectangle Shape
struct Rect
{
	//Variables
	UINT32 x,y,w,h;

	//Constructor
	Rect(const UINT32 ix, const UINT32 iy, const UINT32 iw, const UINT32 ih);

	//Initalize to NULL
	void initNULL();
};

inline Rect::Rect(const UINT32 ix, const UINT32 iy, const UINT32 iw, const UINT32 ih)
{
	x = ix, y = iy, w = iw, h = ih;
}

inline void Rect::initNULL()
{
	x = 0, y = 0, w = 0, h = 0;
}

/*
	*****************************************************************************************************
												SURFACE32 Container
	*****************************************************************************************************
*/

//Surface32
struct SURFACE32
{
	SURFACE32() : data(NULL), w(0), h(0) {}
	UINT8* data;
	UINT32 w, h, bytes;
};

/*
	*****************************************************************************************************
												GFX32 FUNCTIONS (INLINE CODE)
	*****************************************************************************************************
*/

inline void GFX32_Blit(SURFACE32* src, SURFACE32* dest)
{
	memcpy(dest->data, src->data, src->bytes);
}

inline void GFX32_ClearBuffer(SURFACE32* dest, RGBA32 clr)
{
	for(UINT32 x = 0; x < dest->w; ++x)
		for(UINT32 y = 0; y < dest->h; ++y)
			GFX32_SetPixel(dest, x, y, clr);
}

inline void GFX32_ClearBuffer(SURFACE32* dest)
{
	memset(dest->data, 0, dest->bytes);
}

inline void GFX32_BlendPixel(SURFACE32* dest, UINT32 x, UINT32 y, RGBA32 clr)
{
	const RGBA32 dc = GFX32_GetPixel(dest, x, y);
	GFX32_SetPixel(dest, x, y, RGBA32( dc.r + UINT8(int(clr.a*(clr.r - dc.r))>>8),
									   dc.g + UINT8(int(clr.a*(clr.g - dc.g))>>8),
									   dc.b + UINT8(int(clr.a*(clr.b - dc.b))>>8),
									   255) );
}

inline void GFX32_SetPixel(SURFACE32* dest, UINT32 x, UINT32 y, RGBA32 clr)
{
	const int sc = ((y*dest->w)+x)*4;
	dest->data[sc+2] = clr.r;
	dest->data[sc+1] = clr.g;
	dest->data[sc] = clr.b;
	dest->data[sc+3] = clr.a;
}

inline RGBA32 GFX32_GetPixel(SURFACE32* src, UINT32 x, UINT32 y)
{
	const int sc = ((y*src->w)+x)*4;
	return RGBA32(src->data[sc+2], src->data[sc+1], src->data[sc], src->data[sc+3]);
}

inline RGBA32 GFX32_GetPixelInvert(SURFACE32* src, UINT32 x, UINT32 y)
{
	const int sc = src->bytes-(((y*src->w)+(src->w-x))*4);
	return RGBA32(src->data[sc+2], src->data[sc+1], src->data[sc], src->data[sc+3]);
}

inline void GFX32_ReplaceSurface(SURFACE32* src, SURFACE32* dest)
{
	GFX32_FreeSurface(dest);
	dest->data = src->data;
	dest->bytes = src->bytes;
	dest->w = src->w;
	dest->h = src->h;
}


#endif

