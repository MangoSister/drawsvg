//
//  math_helper.h
//  Project1
//
//  Created by MangoSister on 16/1/12.
//
//

#ifndef helper_h
#define helper_h


namespace CMU462
{
	const float INV_1_255 = 0.00392156862;
	
	int floor_to_int(float x);
	
	float frac(float x);
	float inv_frac(float x);
	
	float cross_product(float x0, float y0, float x1, float y1);
	
	bool is_counter_clockwise(float x0, float y0, float x1, float y1, float x2, float y2);
	
	void flip_tri_counter_clockwise(float& x0, float& y0, float& x1, float& y1, float& x2, float& y2);

	
	bool clip_screen_line(float x0, float y0, float x1, float y1,
						  float width, float height,
						  float& x0clip, float& y0clip, float& x1clip, float& y1clip);

	
	//MLAA
	unsigned char get_gray_scale(unsigned char r, unsigned char g, unsigned char b);
	
//	unsigned char* load_tex(const char *filePath);
//	unsigned char* load_tex_rg(const char *filePath);
//	unsigned char* load_tex(const char *filePath, unsigned int& w, unsigned int& h);
//	unsigned char* load_tex_rg(const char *filePath, unsigned int& w, unsigned int& h);
//	void save_tex(const char* filePath, unsigned char* tex, unsigned int width, unsigned int height);
//	
	unsigned char uchar_abs(unsigned char a, unsigned char b);
	
}

#endif /* helper_h */
