/*
	Code written by James William Fletcher. 
	Copyright 2010 - 2011. 
	purrr@coy.cat
	http://voxdsp.com/
*/


#include <math.h>
#include "GFX32.h"
#include "MathLib.h"
#include "Colours.h"
#include <strsafe.h>

//Throw Error Message
void ThrowError(LPTSTR lpszFunction) 
{ 
    //Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    //Display the error message and exit the process
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf); 
    
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}


void GFX32_CreateSurface(UINT32 w, UINT32 h, SURFACE32* dest)
{
	const UINT32 bytes = h * (w * 4);
	dest->data = new UINT8[bytes];
	dest->w = w, dest->h = h;
	dest->bytes = bytes;
}

void GFX32_FreeSurface(SURFACE32* src)
{
	if(src != NULL)
	{
		if(src->data != NULL)
		{
			delete [] src->data;
			src->data = NULL;
		}
	}
}

bool GFX32_FindImagePosition(SURFACE32* src, SURFACE32* find, SURFACE32* findtollerancemap, unsigned int* ox, unsigned int* oy)
{
	const UINT32 sh = src->h, sw = src->w;
	const UINT32 fh = find->h, fw = find->w;

	//For Every Pixel
	for(UINT32 y = 0; y < sh; ++y)
	{
		for(UINT32 x = 0; x < sw; ++x)
		{
			//Dont go over bounds
			if(y+fh >= sh || x+fw >= sw)
				continue;

			//Check image is at that location
			bool die = false;
			for(UINT32 y2 = y; y2 < y+fh; ++y2)
			{
				for(UINT32 x2 = x; x2 < x+fw; ++x2)
				{
					//Sample pixel colours
					RGBA32 ftc = GFX32_GetPixelInvert(findtollerancemap, x2-x, y2-y);
					if(ftc.r >= 255)
						continue;

					const RGBA32 sc = GFX32_GetPixelInvert(src, x2, y2);
					const RGBA32 fc = GFX32_GetPixelInvert(find, x2-x, y2-y);
					if(ftc.r != 0)
						ftc.r /= 2;

					//If the pixel is not in the desired range terminate search
					if( sc.r >= fc.r-ftc.r && sc.r <= fc.r+ftc.r &&
						sc.g >= fc.g-ftc.g && sc.g <= fc.g+ftc.g &&
						sc.b >= fc.b-ftc.b && sc.b <= fc.b+ftc.b)
					{}else
					{
						die = true;
						break;
					}
				}
				if(die == true)
					break;
			}
			if(die == false)
			{
				*ox = x;
				*oy = y;
				return true;
			}
		}
	}

	//Couldent find image
	return false;
}

void GFX32_ConvertBMPtoGFX32(const char* bitmappath, SURFACE32* dest)
{
	//Load Bitmap and Bitmap info
	HBITMAP bmp = (HBITMAP)LoadImage(0, bitmappath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	BITMAP bmpi;
	GetObject(bmp, sizeof(BITMAP), &bmpi);
	HDC hDCbmp = CreateCompatibleDC(GetDC(NULL));
	SelectObject(hDCbmp, bmp);

	//Create GFX32 Surface
	const UINT32 bytes = bmpi.bmHeight * (bmpi.bmWidth * 4);
	dest->data = new UINT8[bytes];
	dest->w = bmpi.bmWidth, dest->h = bmpi.bmHeight;
	dest->bytes = bytes;

	//Create DIBits Info
	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = dest->w;
	bi.bmiHeader.biHeight = dest->h;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = dest->w * dest->h * 4;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	//Create DIB Section
	HDC hDCCom = CreateCompatibleDC(GetDC(NULL));
	BYTE* bitPointer;
	HBITMAP hBitmap2 = CreateDIBSection(hDCCom, &bi, DIB_RGB_COLORS, (void**) (&bitPointer), NULL, NULL);
	SelectObject(hDCCom, hBitmap2);
	BitBlt(hDCCom, 0, 0, bmpi.bmWidth, bmpi.bmHeight, hDCbmp, 0, 0, SRCCOPY); 

	memcpy(dest->data, bitPointer, bi.bmiHeader.biSizeImage);

	DeleteDC(hDCCom);
    DeleteObject(hBitmap2);
}

void GFX32_ConvertHWNDtoGFX32(HWND hWnd, SURFACE32* dest)
{
	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = dest->w;
	bi.bmiHeader.biHeight = dest->h;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = dest->w * dest->h * 4;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	HDC hDCCom = CreateCompatibleDC(GetDC(NULL));
	BYTE* bitPointer;
	HBITMAP hBitmap2 = CreateDIBSection(hDCCom, &bi, DIB_RGB_COLORS, (void**) (&bitPointer), NULL, NULL);
	SelectObject(hDCCom, hBitmap2);
	PrintWindow(hWnd, hDCCom, PW_CLIENTONLY);

	memcpy(dest->data, bitPointer, bi.bmiHeader.biSizeImage);

	DeleteDC(hDCCom);
    DeleteObject(hBitmap2);
}

void GFX32_KeyBlit(SURFACE32* src, SURFACE32* dest, UINT32 ix, UINT32 iy, RGBA32 key)
{
	const UINT32 my = iy+src->h, mx = ix+src->w;

	for(UINT32 y = iy; y < my; ++y)
	{
		for(UINT32 x = ix; x < mx; ++x)
		{
			const RGBA32 sc = GFX32_GetPixel(src, x-ix, y-iy);

			if(sc.r == key.r && sc.g == key.g && sc.b == key.b)
				continue;

			GFX32_SetPixel(dest, x, y, sc);	
		}
	}
}

void GFX32_LineBlit(SURFACE32* src, SURFACE32* dest, UINT32 ix, UINT32 iy)
{
	const UINT32 srb = src->w*4;
	const UINT32 drb = dest->w*4;
	const UINT32 line_len = srb;

	for(UINT32 y = 0, doffset = ((iy*dest->w)+ix)*4; y < src->h; ++y, doffset += drb)
		memcpy(&dest->data[doffset], &src->data[y*srb], line_len);
}

void GFX32_LineBlitEx(SURFACE32* src, Rect area, SURFACE32* dest, UINT32 ix, UINT32 iy)
{
	const UINT32 srb = src->w*4;
	const UINT32 drb = dest->w*4;
	const UINT32 xoffset = area.x*4;

	for(UINT32 y = area.y, doffset = ((iy*dest->w)+ix)*4; y < src->h; ++y, doffset += drb)
		memcpy(&dest->data[doffset], &src->data[(y*srb) + xoffset], drb);
}

void GFX32_AlphaBlit(SURFACE32* src, SURFACE32* dest, UINT32 ix, UINT32 iy)
{
	const UINT32 my = iy+src->h, mx = ix+src->w;

	for(UINT32 y = iy; y < my; ++y)
	{
		for(UINT32 x = ix; x < mx; ++x)
		{
			//Get Src Pixel
			const RGBA32 sc = GFX32_GetPixel(src, x-ix, y-iy);

			//If source pixel is opaque just copy it
			if(sc.a == 255)
			{
				GFX32_SetPixel(dest, x, y, sc);
				continue;
			}

			//If source pixel is fully transparent do nothing
			if(sc.a == 0)
				continue;

			//Otherwise blend the colours together
			GFX32_BlendPixel(dest, x, y, sc);
		}
	}
}

void GFX32_AlphaBlitEx(SURFACE32* src, Rect area, SURFACE32* dest, UINT32 ix, UINT32 iy)
{
	const UINT32 my = area.y+area.h, mx = area.x+area.w;

	for(UINT32 y = area.y; y < my; ++y)
	{
		for(UINT32 x = area.x; x < mx; ++x)
		{
			//Get Src Pixel
			const RGBA32 sc = GFX32_GetPixel(src, x, y);

			//If source pixel is opaque just copy it
			if(sc.a == 255)
			{
				GFX32_SetPixel(dest, (x-area.x)+ix, (y-area.y)+iy, sc);
				continue;
			}

			//If source pixel is fully transparent do nothing
			if(sc.a == 0)
				continue;

			//Otherwise blend the colours together
			GFX32_BlendPixel(dest, (x-area.x)+ix, (y-area.y)+iy, sc);
		}
	}
}

void GFX32_AlphaSaturateBlit(SURFACE32* src, Rect area, SURFACE32* dest, UINT32 ix, UINT32 iy, UINT32 alpha)
{
	const UINT32 my = area.y+area.h, mx = area.x+area.w;

	for(UINT32 y = area.y; y < my; ++y)
	{
		for(UINT32 x = area.x; x < mx; ++x)
		{
			//Get Src Pixel
			RGBA32 sc = GFX32_GetPixel(src, x, y);

			//Otherwise blend the colours together
			sc.r += alpha, sc.g += alpha, sc.b += alpha;
			GFX32_BlendPixel(dest, (x-area.x)+ix, (y-area.y)+iy, sc);
		}
	}
}

void GFX32_AlphaBlendBlit(SURFACE32* src, Rect area, SURFACE32* dest, UINT32 ix, UINT32 iy, UINT32 alpha)
{
	const UINT32 my = area.y+area.h, mx = area.x+area.w;

	for(UINT32 y = area.y; y < my; ++y)
	{
		for(UINT32 x = area.x; x < mx; ++x)
		{
			//Get Src Pixel
			const RGBA32 sc = GFX32_GetPixel(src, x, y);

			//Otherwise blend the colours together
			const RGBA32 dc = GFX32_GetPixel(dest, (x-area.x)+ix, (y-area.y)+iy);
			GFX32_SetPixel(dest, (x-area.x)+ix, (y-area.y)+iy, RGBA32( dc.r + UINT8(int(alpha*(sc.r - dc.r))>>8),
																	   dc.g + UINT8(int(alpha*(sc.g - dc.g))>>8),
																	   dc.b + UINT8(int(alpha*(sc.b - dc.b))>>8),
																	   dc.a + UINT8(int(alpha*(sc.a - dc.a))>>8) ) );
		}
	}
}

void GFX32_TransparentBlit(SURFACE32* src, SURFACE32* dest, UINT32 ix, UINT32 iy, UINT32 alpha)
{
	const UINT32 my = src->h, mx = src->w;

	for(UINT32 y = 0; y < my; ++y)
	{
		for(UINT32 x = 0; x < mx; ++x)
		{
			//Get Src Pixel
			const RGBA32 sc = GFX32_GetPixel(src, x, y);

			//Otherwise blend the colours together
			const RGBA32 dc = GFX32_GetPixel(dest, x+ix, y+iy);
			GFX32_SetPixel(dest, x+ix, y+iy, RGBA32( dc.r + UINT8(int(alpha*(sc.r - dc.r))>>8),
													 dc.g + UINT8(int(alpha*(sc.g - dc.g))>>8),
													 dc.b + UINT8(int(alpha*(sc.b - dc.b))>>8),
													 dc.a + UINT8(int(alpha*(sc.a - dc.a))>>8) ) );
		}
	}
}

void GFX32_FillRectAlpha(SURFACE32* dest, Rect area, RGBA32 clr)
{
	const UINT32 my = area.y + area.h, mx = area.x + area.w;

	for(UINT32 y = area.y; y < my; ++y)
	{
		for(UINT32 x = area.x; x < mx; ++x)
		{
			GFX32_BlendPixel(dest, x, y, clr);
		}
	}
}

void GFX32_ReplaceColour(SURFACE32* dest, Rect area, RGBA32 find_clr, RGBA32 new_clr)
{
	const UINT32 my = area.y + area.h, mx = area.x + area.w;

	for(UINT32 y = area.y; y < area.y+area.h; ++y)
	{
		for(UINT32 x = area.x; x < area.x+area.w; ++x)
		{
			const RGBA32 dc = GFX32_GetPixel(dest, x, y);
			if(dc.r == find_clr.r && dc.g == find_clr.g && dc.b == find_clr.b)
				GFX32_SetPixel(dest, x, y, new_clr);
		}
	}
}

void GFX32_DrawHLineAlpha(SURFACE32* dest, UINT32 x1, UINT32 x2, UINT32 y, RGBA32 clr)
{
	for(UINT32 x = x1; x < x2; ++x)
		GFX32_BlendPixel(dest, x, y, clr);
}

void GFX32_DrawVLineAlpha(SURFACE32* dest, UINT32 y1, UINT32 y2, UINT32 x, RGBA32 clr)
{
	for(UINT32 y = y1; y < y2; ++y)
		GFX32_BlendPixel(dest, x, y, clr);
}

void GFX32_DrawHLine(SURFACE32* dest, UINT32 x1, UINT32 x2, UINT32 y, RGBA32 clr)
{
	for(UINT32 x = x1; x < x2; ++x)
		GFX32_SetPixel(dest, x, y, clr);
}

void GFX32_DrawVLine(SURFACE32* dest, UINT32 y1, UINT32 y2, UINT32 x, RGBA32 clr)
{
	for(UINT32 y = y1; y < y2; ++y)
		GFX32_SetPixel(dest, x, y, clr);
}

void GFX32_DrawLineAlpha(SURFACE32* dest, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2, RGBA32 clr)
{
	int dx, dy, inx, iny, e;
	
	dx = x2 - x1;
	dy = y2 - y1;
	inx = dx > 0 ? 1 : -1;
	iny = dy > 0 ? 1 : -1;
	dx = abs(dx);
	dy = abs(dy);
	
	if(dx >= dy)
	{
		dy <<= 1;
		e = dy - dx;
		dx <<= 1;

		while (x1 != x2)
		{
			GFX32_BlendPixel(dest, x1, y1, clr);

			if(e >= 0)
			{
				y1 += iny;
				e-= dx;
			}

			e += dy;
			x1 += inx;
		}
	}
	else
	{
		dx <<= 1;
		e = dx - dy;
		dy <<= 1;

		while (y1 != y2)
		{
			GFX32_BlendPixel(dest, x1, y1, clr);

			if(e >= 0)
			{
				x1 += inx;
				e -= dy;
			}

			e += dx;
			y1 += iny;
		}
	}

	GFX32_BlendPixel(dest, x1, y1, clr);
}

void GFX32_DrawLine(SURFACE32* dest, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2, RGBA32 clr)
{
	int dx, dy, inx, iny, e;
	
	dx = x2 - x1;
	dy = y2 - y1;
	inx = dx > 0 ? 1 : -1;
	iny = dy > 0 ? 1 : -1;
	dx = abs(dx);
	dy = abs(dy);
	
	if(dx >= dy)
	{
		dy <<= 1;
		e = dy - dx;
		dx <<= 1;

		while (x1 != x2)
		{
			GFX32_SetPixel(dest, x1, y1, clr);

			if(e >= 0)
			{
				y1 += iny;
				e-= dx;
			}

			e += dy;
			x1 += inx;
		}
	}
	else
	{
		dx <<= 1;
		e = dx - dy;
		dy <<= 1;

		while (y1 != y2)
		{
			GFX32_SetPixel(dest, x1, y1, clr);

			if(e >= 0)
			{
				x1 += inx;
				e -= dy;
			}

			e += dx;
			y1 += iny;
		}
	}

	GFX32_SetPixel(dest, x1, y1, clr);
}

void GFX32_DrawCircleAlpha(SURFACE32* dest, UINT32 x0, UINT32 y0, UINT32 radius, RGBA32 clr)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
    
	GFX32_BlendPixel(dest, x0, y0 + radius, clr);
	GFX32_BlendPixel(dest, x0, y0 - radius, clr);
	GFX32_BlendPixel(dest, x0 + radius, y0, clr);
	GFX32_BlendPixel(dest, x0 - radius, y0, clr);
 
	while(x < y)
	{
		if(f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;    
      
		GFX32_BlendPixel(dest, x0 + x, y0 + y, clr);
		GFX32_BlendPixel(dest, x0 - x, y0 + y, clr);
		GFX32_BlendPixel(dest, x0 + x, y0 - y, clr);
		GFX32_BlendPixel(dest, x0 - x, y0 - y, clr);
		GFX32_BlendPixel(dest, x0 + y, y0 + x, clr);
		GFX32_BlendPixel(dest, x0 - y, y0 + x, clr);
		GFX32_BlendPixel(dest, x0 + y, y0 - x, clr);
		GFX32_BlendPixel(dest, x0 - y, y0 - x, clr);
	}
}

void GFX32_DrawCircle(SURFACE32* dest, UINT32 x0, UINT32 y0, UINT32 radius, RGBA32 clr)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
    
	GFX32_SetPixel(dest, x0, y0 + radius, clr);
	GFX32_SetPixel(dest, x0, y0 - radius, clr);
	GFX32_SetPixel(dest, x0 + radius, y0, clr);
	GFX32_SetPixel(dest, x0 - radius, y0, clr);
 
	while(x < y)
	{
		if(f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;    
      
		GFX32_SetPixel(dest, x0 + x, y0 + y, clr);
		GFX32_SetPixel(dest, x0 - x, y0 + y, clr);
		GFX32_SetPixel(dest, x0 + x, y0 - y, clr);
		GFX32_SetPixel(dest, x0 - x, y0 - y, clr);
		GFX32_SetPixel(dest, x0 + y, y0 + x, clr);
		GFX32_SetPixel(dest, x0 - y, y0 + x, clr);
		GFX32_SetPixel(dest, x0 + y, y0 - x, clr);
		GFX32_SetPixel(dest, x0 - y, y0 - x, clr);
	}
}

void GFX32_Invert(SURFACE32* surf)
{
	for(UINT32 y = 0; y < surf->h; ++y)
	{
		for(UINT32 x = 0; x < surf->w; ++x)
		{
			RGBA32 pixel = GFX32_GetPixel(surf, x, y);
			pixel.r = 255 - pixel.r;
			pixel.g = 255 - pixel.g;
			pixel.b = 255 - pixel.b;
			GFX32_SetPixel(surf, x, y, pixel);
		}
	}
}

void GFX32_SetHSL(SURFACE32* surf, const float h, const float s, const float l)
{
	for(UINT32 y = 0; y < surf->h; ++y)
	{
		for(UINT32 x = 0; x < surf->w; ++x)
		{
			RGBA32 pixel = GFX32_GetPixel(surf, x, y);
			
			clr::mColour hsl = clr::RGBtoHSL(clr::mColour(float(pixel.r) / 255.f, float(pixel.g) / 255.f, float(pixel.b) / 255.f));
			hsl.r = h;
			hsl.g = s;
			hsl.b += l;
			hsl = clr::HSLtoRGB(hsl);

			GFX32_SetPixel(surf, x, y, RGBA32(hsl.r * 255, hsl.g * 255, hsl.b * 255, pixel.a));
		}
	}
}








