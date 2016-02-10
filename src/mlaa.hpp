#ifndef mlaa_hpp
#define mlaa_hpp

#include <cmath>
#include <color.h>
#include "helper.h"

using CMU462::Color;

class mlaa_wrapper
{
private:
	
	unsigned char* mlaa_precomputed_tex = nullptr;
	
	unsigned char* _render_target = nullptr;
	unsigned int _target_w;
	unsigned int _target_h;
	
	bool* edge_detect_tex = nullptr; //2 channel
	const unsigned char edge_threshold = 16;
	
	void gen_precomputed_tex();
	
	int search_edge_end_dist_left(int ix, int iy);
	int search_edge_end_dist_right(int ix, int iy);
	int search_edge_end_dist_up(int ix, int iy);
	int search_edge_end_dist_down(int ix, int iy);
	const int MAX_SEARCH_STEP = 8;
	//void lookup_area(int d1, int d2, int e1, int e2, unsigned char& a1, unsigned char& a2);
	
	inline void lookup_area(int d1, int d2, int e1, int e2, unsigned char& a1, unsigned char& a2)
	{
		const unsigned char* area_tex =  mlaa_precomputed_tex;
		int idx_x = e1 * (MAX_SEARCH_STEP + 1) + d1;
		int idx_y = e2 * (MAX_SEARCH_STEP + 1) + d2;
		a1 = area_tex[ 2 * ( idx_x + idx_y * ((MAX_SEARCH_STEP + 1) * 5) )];
		a2 = area_tex[ 2 * ( idx_x + idx_y * ((MAX_SEARCH_STEP + 1) * 5) ) + 1];
	}
	
	Color* blend_weight_tex = nullptr;
	Color compute_blend_weight(int ix, int iy);
	
	inline void draw_pixel(int ix, int iy, const Color& color)
	{
		if ( ix < 0 || ix >= _target_w || iy < 0 || iy >= _target_h ) return;
		
		int idx = 4 * (ix + iy * _target_w);
		
		Color target_color(_render_target[idx],
												_render_target[idx + 1],
												_render_target[idx + 2],
												_render_target[idx + 3]);
		
		float out_a = (1.0f - color.a) * target_color.a + color.a;
		float inv_out_a = 1.0f / out_a;
		target_color.r = ((1.0f - color.a) * target_color.r * target_color.a + color.a * color.r) * inv_out_a;
		target_color.g = ((1.0f - color.a) * target_color.g * target_color.a + color.a * color.g) * inv_out_a;
		target_color.b = ((1.0f - color.a) * target_color.b * target_color.a + color.a * color.b) * inv_out_a;
		target_color.a = out_a;
		
		_render_target[idx] = (uint8_t) (target_color.r * 255.0f);
		_render_target[idx + 1] = (uint8_t) (target_color.g * 255.0f);
		_render_target[idx + 2] = (uint8_t) (target_color.b * 255.0f);
		_render_target[idx + 3] = (uint8_t) (target_color.a * 255.0f);
	}
	
public:
	mlaa_wrapper(unsigned char* img, unsigned int w, unsigned int h);
	mlaa_wrapper(const mlaa_wrapper&) = delete;
	mlaa_wrapper(mlaa_wrapper&&) = delete;
	mlaa_wrapper& operator = (const mlaa_wrapper& ) = delete;
	mlaa_wrapper& operator = (mlaa_wrapper && ) = delete;
	~mlaa_wrapper();
	
	void process_mlaa();
	void output_result(const char* edgeFileName, const char* weightFileName, const char* resultFileName);
	
};

#endif /* mlaa_hpp */
