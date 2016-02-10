#include "texture.h"
#include "helper.h"

#include <assert.h>
#include <iostream>
#include <algorithm>


using namespace std;

namespace CMU462 {

inline void uint8_to_float( float dst[4], unsigned char* src ) {
  uint8_t* src_uint8 = (uint8_t *)src;
  dst[0] = src_uint8[0] / 255.f;
  dst[1] = src_uint8[1] / 255.f;
  dst[2] = src_uint8[2] / 255.f;
  dst[3] = src_uint8[3] / 255.f;
}

inline void float_to_uint8( unsigned char* dst, float src[4] ) {
  uint8_t* dst_uint8 = (uint8_t *)dst;
  dst_uint8[0] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[0])));
  dst_uint8[1] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[1])));
  dst_uint8[2] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[2])));
  dst_uint8[3] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[3])));
}

void Sampler2DImp::generate_mips(Texture& tex, int startLevel) {

  // NOTE(sky):
  // The starter code allocates the mip levels and generates a level
  // map simply fills each level with a color that differs from its
  // neighbours'. The reference solution uses trilinear filtering
  // and it will only work when you have mipmaps.

  // Task 7: Implement this

  // check start level
  if ( startLevel >= tex.mipmap.size() ) {
    std::cerr << "Invalid start level";
  }

  // allocate sublevels
  int baseWidth  = tex.mipmap[startLevel].width;
  int baseHeight = tex.mipmap[startLevel].height;
  int numSubLevels = (int)(log2f( (float)max(baseWidth, baseHeight)));

  numSubLevels = min(numSubLevels, kMaxMipLevels - startLevel - 1);
  tex.mipmap.resize(startLevel + numSubLevels + 1);

  int width  = baseWidth;
  int height = baseHeight;
  for (int i = 1; i <= numSubLevels; i++) {

    MipLevel& level = tex.mipmap[startLevel + i];

    // handle odd size texture by rounding down
    width  = max( 1, width  / 2); assert(width  > 0);
    height = max( 1, height / 2); assert(height > 0);

    level.width = width;
    level.height = height;
    level.texels = vector<unsigned char>(4 * width * height);
		
		//standard 2x2 box filter
		MipLevel& lastLevel = tex.mipmap[startLevel + i - 1];
		size_t lastWidth = tex.mipmap[startLevel + i - 1].width;
		for(int x = 0; x < width; ++x)
		{
			for(int y = 0; y < height; ++y)
			{
				int lastIdx00 = 8 * (x + y * lastWidth);
				unsigned short r = (lastLevel.texels[lastIdx00] +
															lastLevel.texels[lastIdx00 + 4] +
															lastLevel.texels[lastIdx00 + 4 * lastWidth] +
															lastLevel.texels[lastIdx00 + 4 + 4 * lastWidth]) / 4;
				unsigned short g = (lastLevel.texels[lastIdx00 + 1] +
														lastLevel.texels[lastIdx00 + 5] +
														lastLevel.texels[lastIdx00 + 4 * lastWidth + 1] +
														lastLevel.texels[lastIdx00 + 5 + 4 * lastWidth]) / 4;
				unsigned short b = (lastLevel.texels[lastIdx00 + 2] +
														lastLevel.texels[lastIdx00 + 6] +
														lastLevel.texels[lastIdx00 + 4 * lastWidth + 2] +
														lastLevel.texels[lastIdx00 + 6 + 4 * lastWidth]) / 4;
				unsigned short a = (lastLevel.texels[lastIdx00 + 3] +
														lastLevel.texels[lastIdx00 + 7] +
														lastLevel.texels[lastIdx00 + 4 * lastWidth + 3] +
														lastLevel.texels[lastIdx00 + 7 + 4 * lastWidth]) / 4;

				int idx00 = 4 * (x + y * height);
				level.texels[idx00] = (unsigned char)r;
				level.texels[idx00 + 1] = (unsigned char)g;
				level.texels[idx00 + 2] = (unsigned char)b;
				level.texels[idx00 + 3] = (unsigned char)a;
			}
		}

  }

  // fill all 0 sub levels with interchanging colors
//  Color colors[3] = { Color(1.0f,0.0f,0.0f,1.0f), Color(0.0f,1.0f,0.0f,1.0f), Color(0.0f,0.0f,1.0f,1.0f) };
//  for(size_t i = 1; i < tex.mipmap.size(); ++i) {
//
//    Color c = colors[i % 3];
//    MipLevel& mip = tex.mipmap[i];
//
//    for(size_t i = 0; i < 4 * mip.width * mip.height; i += 4) {
//      float_to_uint8( &mip.texels[i], &c.r );
//    }
//  }

}

Color Sampler2DImp::sample_nearest(Texture& tex,
                                   float u, float v,
                                   int level) {

  // Task ?: Implement nearest neighbour interpolation

  // return magenta for invalid level
  return Color(1.0f,0.0f,1.0f,1.0f);

}

Color Sampler2DImp::sample_bilinear(Texture& tex,
                                    float u, float v,
                                    int level) {

  // Task ?: Implement bilinear filtering

  // repeat texture for now

  // return magenta for invalid level
  if(level < 0 || level >=  tex.mipmap.size())
    return Color(1.0f,0.0f,1.0f,1.0f);

  MipLevel& currLevel = tex.mipmap[level];
	while(u < 0.0f) u++;
	while(v < 0.0) v++;
	while(u > 1.0f) u--;
	while(v > 1.0f) v--;
  u *= (float)currLevel.width;
  v *= (float)currLevel.height;
  int x_left = (floor_to_int(u - 0.5f)) % currLevel.width;
  int x_right = (x_left + 1) % currLevel.width;
  int y_up = (floor_to_int(v - 0.5f)) % currLevel.height;
  int y_down = (y_up + 1) % currLevel.height;

  int idx_00 = 4 * (x_left + y_up * currLevel.width);
  int idx_10 = 4 * (x_right + y_up * currLevel.width);
  int idx_01 = 4 * (x_left + y_down * currLevel.width);
  int idx_11 = 4 * (x_right + y_down * currLevel.width);

  Color c00{ currLevel.texels[idx_00],
      currLevel.texels[idx_00 + 1],
      currLevel.texels[idx_00 + 2],
      currLevel.texels[idx_00 + 3]};

  Color c10{ currLevel.texels[idx_10],
      currLevel.texels[idx_10 + 1],
      currLevel.texels[idx_10 + 2],
      currLevel.texels[idx_10 + 3]};

  Color c01{ currLevel.texels[idx_01],
      currLevel.texels[idx_01 + 1],
      currLevel.texels[idx_01 + 2],
      currLevel.texels[idx_01 + 3]};

  Color c11{ currLevel.texels[idx_11],
      currLevel.texels[idx_11 + 1],
      currLevel.texels[idx_11 + 2],
      currLevel.texels[idx_11 + 3]};

  float t = x_left < x_right ? u - ( (float)x_left + 0.5f ) :
                                0.5f - (float)x_right + u;
  Color c0 = c00 * (1.0f - t) + c10 * t;
  Color c1 = c01 * (1.0f - t) + c11 * t;

  t = y_up < y_down ? v - ( (float)y_up + 0.5f ) :
                        0.5f - (float)y_down + v;
  return c0 * (1.0f - t) + c1 * t;
}

Color Sampler2DImp::sample_trilinear(Texture& tex,
                                     float u, float v,
                                     float u_scale, float v_scale) {

  // Task 8: Implement trilinear filtering
	float L = std::max(u_scale, v_scale);
	float d = log2f(L);
	int low_level = floor_to_int(d);
	int high_level = low_level + 1;
	
	//fallback to bilinear
	if(low_level < 0)
			return sample_bilinear(tex, u, v);
	
	// return magenta for invalid level
	if(high_level >=  tex.mipmap.size())
		return Color(1.0f,0.0f,1.0f,1.0f);
	
	Color c0 = sample_bilinear(tex, u, v, low_level);
	Color c1 = sample_bilinear(tex, u, v, high_level);
	
	float t = frac(d);
	return c0 * (1 - t) + c1 * t;

}

} // namespace CMU462
