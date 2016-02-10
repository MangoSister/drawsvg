//
//  math_helper.cpp
//  Project1
//
//  Created by MangoSister on 16/1/12.
//
//

#include <stdio.h>
#include <cmath>

#include "helper.h"
//#include <tiffio.h>
#include <string.h>

namespace CMU462
{
	int floor_to_int(float x)
	{
		return (int)std::floor(x);
	}
	
	float frac(float x)
	{
		return x - std::floor(x);
	}
	
	float inv_frac(float x)
	{
		return 1.0f - frac(x);
	}
	
	float cross_product(float x0, float y0, float x1, float y1)
	{
		return x0 * y1 - x1 * y0;
	}
	
	//cross product > 0 => counter-clockwise
	bool is_counter_clockwise(float x0, float y0, float x1, float y1, float x2, float y2)
	{
		float e0_x = x1 - x0;
		float e0_y = y1 - y0;
		float e1_x = x2 - x1;
		float e1_y = y2 - y1;
		return cross_product(e0_x, e0_y, e1_x, e1_y) < 0.0f;
	}
	
	void flip_tri_counter_clockwise(float& x0, float& y0, float& x1, float& y1, float& x2, float& y2)
	{
		if(is_counter_clockwise(x0, y0, x1, y1, x2, y2))
			return;
		//clockwise, flip
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
	
	bool clip_screen_line(float x0, float y0, float x1, float y1,
						  float width, float height,
						  float& x0clip, float& y0clip, float& x1clip, float& y1clip)
	{
		float t0 = 0.0f, t1 = 1.0f;
		float xdelta = x1 - x0, ydelta = y1 - y0;
		
		//p: orthogonal axis delta
		//q: orthogonal axis (signed) distance from x0 to edge
		float p, q, r;
		
		for(int edge = 0; edge < 4; ++edge)
		{
			// Traverse through left, right, bottom, top edges.
			if (edge == 0) {  p = -xdelta; q = x0; } //left
			else if (edge == 1) {  p = xdelta; q = (width - x0); } //right
			else if (edge == 2) {  p = -ydelta; q = y0; } //top
			else /* edge == 3 */ {  p = ydelta; q =  (height-y0); } // bottom
			
			if(p == 0)
			{
				if(q < 0) //line is parallel to the edge but is outside
					return false;
				else continue; //line is parallel to the edge and inside
			}
			
			r = q / p;
			
			if(p < 0)
			{
				if(r > t1)
					return false; 
				else if(r > t0)
					t0 = r;
			}
			else if(p > 0)
			{
				if(r < t0)
					return false;
				else if(r < t1)
					t1 = r;
			}
		}
		
		x0clip = x0 + t0 * xdelta;
		y0clip = y0 + t0 * ydelta;
		x1clip = x0 + t1 * xdelta;
		y1clip = y0 + t1 * ydelta;
		
		return true;
	}
	
	
	unsigned char get_gray_scale(unsigned char r, unsigned char g, unsigned char b)
	{
		return 0.2126f * r + 0.7152f * g + 0.0722f * b;
	}
	
	unsigned char uchar_abs(unsigned char a, unsigned char b)
	{
		return a > b ? (a - b) : (b - a);
	}
	
//	unsigned char* load_tex_rg(const char *filePath)
//	{
//		TIFF* tif = TIFFOpen(filePath, "r");
//		if(tif != nullptr)
//		{
//			uint32 w, h;
//			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
//			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
//			size_t npixels = w * h;
//			uint32* rasters = (uint32*) _TIFFmalloc(npixels * sizeof(uint32));
//			unsigned char* tex = nullptr;
//			if(rasters != nullptr)
//			{
//				TIFFReadRGBAImage(tif, w, h, rasters); //A-R-G-B
//				tex = new unsigned char[npixels * 2];
//				
//				for(int i = 0; i < npixels; ++i)
//				{
//					tex[2 * (i % w + w * ( (h-1) - i/w ) )    ] = (rasters[i] & 0x000000FF); //R
//					tex[2 * (i % w + w * ( (h-1) - i/w ) ) + 1] = (rasters[i] & 0x0000FF00) >> 8; //G
//				}
//				_TIFFfree(rasters);
//			}
//			TIFFClose(tif);
//			return tex;
//		}
//		else return nullptr;
//	}

	
//	unsigned char* load_tex(const char *filePath, uint32& w, uint32& h)
//	{
//		TIFF* tif = TIFFOpen(filePath, "r");
//		if(tif != nullptr)
//		{
//			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
//			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
//			
//			size_t npixels = w * h;
//			uint32* rasters = (uint32*) _TIFFmalloc(npixels * sizeof(uint32));
//			unsigned char* tex = nullptr;
//			if(rasters != nullptr)
//			{
//				TIFFReadRGBAImage(tif, w, h, rasters); //A-R-G-B
//				tex = new unsigned char[npixels * 4];
//				
//				for(int i = 0; i < npixels; ++i)
//				{
//					tex[4 * (i % w + w * ( (h-1) - i/w ) ) + 3] = (rasters[i] & 0xFF000000) >> 24; //A
//					tex[4 * (i % w + w * ( (h-1) - i/w ) ) + 2] = (rasters[i] & 0x00FF0000) >> 16; //B
//					tex[4 * (i % w + w * ( (h-1) - i/w ) ) + 1] = (rasters[i] & 0x0000FF00) >> 8; //G
//					tex[4 * (i % w + w * ( (h-1) - i/w ) ) + 0] = (rasters[i] & 0x000000FF); //R
//				}
//				_TIFFfree(rasters);
//			}
//			TIFFClose(tif);
//			return tex;
//		}
//		else return nullptr;
//	}
	
//	unsigned char* load_tex(const char *filePath)
//	{
//		TIFF* tif = TIFFOpen(filePath, "r");
//		if(tif != nullptr)
//		{
//			uint32 w, h;
//			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
//			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
//			
//			size_t npixels = w * h;
//			uint32* rasters = (uint32*) _TIFFmalloc(npixels * sizeof(uint32));
//			unsigned char* tex = nullptr;
//			if(rasters != nullptr)
//			{
//				TIFFReadRGBAImage(tif, w, h, rasters); //A-R-G-B
//				tex = new unsigned char[npixels * 4];
//				
//				for(int i = 0; i < npixels; ++i)
//				{
//					tex[4 * (i % w + w * ( (h-1) - i/w ) ) + 3] = (rasters[i] & 0xFF000000) >> 24; //A
//					tex[4 * (i % w + w * ( (h-1) - i/w ) ) + 2] = (rasters[i] & 0x00FF0000) >> 16; //B
//					tex[4 * (i % w + w * ( (h-1) - i/w ) ) + 1] = (rasters[i] & 0x0000FF00) >> 8; //G
//					tex[4 * (i % w + w * ( (h-1) - i/w ) ) + 0] = (rasters[i] & 0x000000FF); //R
//				}
//				_TIFFfree(rasters);
//			}
//			TIFFClose(tif);
//			return tex;
//		}
//		else return nullptr;
//	}
//	
//	unsigned char* load_tex_rg(const char *filePath, uint32& w, uint32& h)
//	{
//		TIFF* tif = TIFFOpen(filePath, "r");
//		if(tif != nullptr)
//		{
//			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
//			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
//
//			size_t npixels = w * h;
//			uint32* rasters = (uint32*) _TIFFmalloc(npixels * sizeof(uint32));
//			unsigned char* tex = nullptr;
//			if(rasters != nullptr)
//			{
//				TIFFReadRGBAImage(tif, w, h, rasters); //A-R-G-B
//				tex = new unsigned char[npixels * 2];
//				
//				for(int i = 0; i < npixels; ++i)
//				{
//					tex[2 * (i % w + w * ( (h-1) - i/w ) )    ] = (rasters[i] & 0x000000FF); //R
//					tex[2 * (i % w + w * ( (h-1) - i/w ) ) + 1] = (rasters[i] & 0x0000FF00) >> 8; //G
//				}
//				_TIFFfree(rasters);
//			}
//			TIFFClose(tif);
//			return tex;
//		}
//		else return nullptr;
//	}
	
//	void save_tex(const char* filePath, unsigned char* tex, unsigned int width, unsigned int height)
//	{
//		TIFF* tif = TIFFOpen(filePath, "w");
//		if(tif == nullptr)
//			return;
//		
//		TIFFSetField (tif, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
//		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
//		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);   // set number of channels per pixel
//		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
//		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
//
//		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
//		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
//
//		tsize_t linebytes = 4 * width;
//		
//		unsigned char *buf = nullptr;        // buffer used to store the row of pixel information for writing to file
//		// allocating memory to store the pixels of current row
//		if (TIFFScanlineSize(tif) == linebytes)
//			buf =(unsigned char *)_TIFFmalloc(linebytes);
//		else
//			buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(tif));
//		
//		// set the strip size of the file to be size of one row of pixels
//		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, width * 4));
//		
//		for (uint32 row = 0; row < height; row++)
//		{
//			memcpy(buf, &tex[ (row) * linebytes], linebytes);
//			if (TIFFWriteScanline(tif, buf, row, 0) < 0)
//				break;
//		}
//		
//		if (buf != nullptr)
//			_TIFFfree(buf);
//		
//		TIFFClose(tif);
//	}
}

