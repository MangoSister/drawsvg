//
//  mlaa.cpp
//  Project1
//
//  Created by MangoSister on 16/1/22.
//
//

#include "mlaa.hpp"
#include "helper.h"

using namespace CMU462;

mlaa_wrapper::mlaa_wrapper(unsigned char* img, unsigned int w, unsigned int h)
:_render_target(img), _target_w(w), _target_h(h)
{
	if(edge_detect_tex == nullptr)
	{
		edge_detect_tex = new bool[_target_w * _target_h * 2] { false };
	}
	
	if(blend_weight_tex == nullptr)
	{
		blend_weight_tex = new Color[_target_w * _target_h] { Color(0.0f, 0.0f, 0.0f, 0.0f) };
	}
	gen_precomputed_tex();
	
}

mlaa_wrapper::~mlaa_wrapper()
{
	if(edge_detect_tex != nullptr)
	{
		delete[] edge_detect_tex;
		edge_detect_tex = nullptr;
	}
	
	if(blend_weight_tex != nullptr)
	{
		delete[] blend_weight_tex;
		blend_weight_tex = nullptr;
	}
	
	if(mlaa_precomputed_tex != nullptr)
	{
		delete [] mlaa_precomputed_tex;
		mlaa_precomputed_tex = nullptr;
	}
	
}

void mlaa_wrapper::gen_precomputed_tex()
{
	float A[64][64];
	
	for(int i = 0; i < 64; ++i)
	{
		float left = 0.5f;
		for(int j = 0; j < i; ++j)
		{
			float x = (float)i * 0.5;
			float right = 0.5f * (x - j - 1) / x;
			float a = (left * right >= 0 || std::abs(left) != std::abs(right)) ?
								std::abs(0.5 * (left + right)) :
								0.5f * std::abs(0.5 * left);
			left = right;
			A[i][j] = a;
		}
	}

	float area2d[32][32];

	for(int left = 0; left < 32; ++left)
	{
		for(int right = 0; right < 32; ++right)
		{
			int x = left + right + 1;
			area2d[left][right] = A[x][left];
		}
	}
	
	
	unsigned char* area4d_scaled = new unsigned char
	[4 * (MAX_SEARCH_STEP + 1) * 5 * (MAX_SEARCH_STEP + 1) * 5];
	int strip = (MAX_SEARCH_STEP + 1) * 5;
	for(int e2 = 0; e2 < 5; ++e2)
	{
		for(int e1 = 0; e1 < 5; ++e1)
		{
			for(int left = 0; left < (MAX_SEARCH_STEP + 1); ++left)
			{
				for(int right = 0; right < (MAX_SEARCH_STEP + 1); ++right)
				{
					int px = left, py = right;
					float a = area2d[px][py];
					px = px + e1 * (MAX_SEARCH_STEP + 1);
					py = py + e2 * (MAX_SEARCH_STEP + 1);
					
					if(e1 == 2 || e2 == 2)
					{
						area4d_scaled[4 * (px * strip + py)    ] = 0;
						area4d_scaled[4 * (px * strip + py) + 1] = 0;
						area4d_scaled[4 * (px * strip + py) + 2] = 0;
						area4d_scaled[4 * (px * strip + py) + 3] = 255;
					}
					else if(left > right)
					{
						if(e2 == 0)
						{
							area4d_scaled[4 * (px * strip + py)    ] = 0;
							area4d_scaled[4 * (px * strip + py) + 1] = 0;
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e2 == 1)
						{
							area4d_scaled[4 * (px * strip + py)    ] = 0;
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e2 == 3)
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = 0;
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
					}
					else if (left < right)
					{
						if(e1 == 0)
						{
							area4d_scaled[4 * (px * strip + py)    ] = 0;
							area4d_scaled[4 * (px * strip + py) + 1] = 0;
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e1 == 1)
						{
							area4d_scaled[4 * (px * strip + py)    ] = 0;
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e1 == 3)
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = 0;
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
					}
					else
					{
						if(e1 + e2 == 0)
						{
							area4d_scaled[4 * (px * strip + py)    ] = 0;
							area4d_scaled[4 * (px * strip + py) + 1] = 0;
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e1 + e2 == 1)
						{
							area4d_scaled[4 * (px * strip + py)    ] = 0;
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e1 + e2 == 2)
						{
							area4d_scaled[4 * (px * strip + py)    ] = 0;
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(2 * a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e1 + e2 == 3)
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = 0;
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e1 + e2 == 4)
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e1 + e2 == 5)
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(2 * a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e1 + e2 == 6)
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(2 * a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = 0;
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else if(e1 + e2 == 7)
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(2 * a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
						else
						{
							area4d_scaled[4 * (px * strip + py)    ] = (unsigned char)(2 * a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 1] = (unsigned char)(2 * a * 255.0f);
							area4d_scaled[4 * (px * strip + py) + 2] = 0;
							area4d_scaled[4 * (px * strip + py) + 3] = 255;
						}
					}
				}
			}
		}
	}
	
	mlaa_precomputed_tex = new unsigned char[2 * (MAX_SEARCH_STEP + 1) * 5 * (MAX_SEARCH_STEP + 1) * 5];
	for(int x = 0; x < (MAX_SEARCH_STEP + 1) * 5; ++x)
		for(int y = 0; y < (MAX_SEARCH_STEP + 1) * 5; ++y)
		{
			mlaa_precomputed_tex[2 * (x + y * (MAX_SEARCH_STEP + 1) * 5)  ] =
			area4d_scaled[4 * (x + y * (MAX_SEARCH_STEP + 1) * 5)  ];
			
			mlaa_precomputed_tex[2 * (x + y * (MAX_SEARCH_STEP + 1) * 5)  + 1] =
			area4d_scaled[4 * (x + y * (MAX_SEARCH_STEP + 1) * 5) + 1];
		}
	delete [] area4d_scaled;

}

int mlaa_wrapper::search_edge_end_dist_left(int ix, int iy)
{
	int step = 0;
	for(step = 0 ; step < MAX_SEARCH_STEP; ++step)
	{
		if(ix <= 0)
			break;
		bool val = edge_detect_tex[2 * (ix + iy * _target_w) + 1];
		if(!val)
			break;
		--ix;
	}
	
	return step;
}

int mlaa_wrapper::search_edge_end_dist_right(int ix, int iy)
{
	int step = 0;
	for(step = 0 ; step < MAX_SEARCH_STEP; ++step)
	{
		if(ix >= _target_w - 1)
			break;
		bool val = edge_detect_tex[2 * (ix + iy * _target_w) + 1];
		if(!val)
			break;
		++ix;
	}
	
	return step;
}

int mlaa_wrapper::search_edge_end_dist_up(int ix, int iy)
{
	int step = 0;
	for(step = 0 ; step < MAX_SEARCH_STEP; ++step)
	{
		if(iy <= 0)
			break;
		bool val = edge_detect_tex[2 * (ix + iy * _target_w)];
		if(!val)
			break;
		--iy;
	}
	
	return step;
}

int mlaa_wrapper::search_edge_end_dist_down(int ix, int iy)
{
	int step = 0;
	for(step = 0 ; step < MAX_SEARCH_STEP; ++step)
	{
		if(iy >= _target_h - 1)
			break;
		bool val = edge_detect_tex[2 * (ix + iy * _target_w)];
		if(!val)
			break;
		++iy;
	}
	
	return step;
}

Color mlaa_wrapper::compute_blend_weight(int ix, int iy)
{
	unsigned char w[4]{ 0x00, 0x00, 0x00, 0x00 };
	bool edge_left = edge_detect_tex[2 * (ix + iy * _target_w)];
	bool edge_top = edge_detect_tex[2 * (ix + iy * _target_w) + 1];
	if(edge_left)
	{
		int e1, e2;
		int dist_up = search_edge_end_dist_up(ix, iy) - 1;
		int y_up_end = iy - dist_up;
		if(y_up_end < 0)
			e1 = 0;
		else
		{
			e1 = (int)edge_detect_tex[2 * (ix + y_up_end * _target_w) + 1] * 3 +
			(int)edge_detect_tex[2 * (ix - 1 + y_up_end * _target_w) + 1];
		}
		
		int dist_down = search_edge_end_dist_down(ix, iy) - 1;
		int y_down_end = iy + dist_down + 1;
		if(y_down_end > _target_h - 1)
			e2 = 0;
		else
		{
			e2 = (int)edge_detect_tex[2 * (ix + y_down_end * _target_w) + 1] * 3 +
			(int)edge_detect_tex[2 * (ix - 1 + y_down_end * _target_w) + 1];
		}
		
		lookup_area(dist_up, dist_down, e1, e2, w[2], w[3]);
	}
	
	if(edge_top)
	{
		int e1, e2;
		int dist_left = search_edge_end_dist_left(ix, iy) - 1;
		int x_left_end = ix - dist_left;
		if(x_left_end < 0)
			e1 = 0;
		else
		{
			e1 = (int)edge_detect_tex[2 * (x_left_end + iy * _target_w)] * 3 +
			(int)edge_detect_tex[2 * (x_left_end + (iy-1) * _target_w)];
		}
		
		int dist_right = search_edge_end_dist_right(ix, iy) - 1;
		int x_right_end = ix + dist_right + 1;
		if(x_right_end > _target_w - 1)
			e2 = 0;
		else
		{
			e2 = (int)edge_detect_tex[2 * (x_right_end + iy * _target_w)] * 3 +
			(int)edge_detect_tex[2 * (x_right_end + (iy-1) * _target_w)];
		}
		
		lookup_area(dist_left, dist_right, e1, e2, w[0], w[1]);
	}
	
	Color weight;
	weight.r = (float)w[0] * INV_1_255;
	weight.g = (float)w[1] * INV_1_255;
	weight.b = (float)w[2] * INV_1_255;
	weight.a = (float)w[3] * INV_1_255;
	//weight.a = 1.0f;
	return weight;
}

void mlaa_wrapper::process_mlaa()
{
	//edge detection
	for(int x = 0; x < _target_w; ++x)
	{
		for(int y = 0; y < _target_h; ++y)
		{
			int idx = x + y * _target_w;
			edge_detect_tex[2 * idx] = false;
			edge_detect_tex[2 * idx + 1] = false;
			
			unsigned char gray = get_gray_scale(_render_target[4 * idx    ],
												_render_target[4 * idx + 1],
												_render_target[4 * idx + 2]);
			
			if(x != 0)
			{
				unsigned char gray_left = get_gray_scale(_render_target[4 * ( idx - 1 )    ],
														 _render_target[4 * ( idx - 1 ) + 1],
														 _render_target[4 * ( idx - 1 ) + 2]);
				if(uchar_abs(gray, gray_left) > edge_threshold)
					edge_detect_tex[2 * idx] = true; //edge left
				
			}
			
			if(y != 0)
			{
				unsigned char gray_top = get_gray_scale(_render_target[4 * ( idx - _target_w )    ],
													 _render_target[4 * ( idx - _target_w ) + 1],
													 _render_target[4 * ( idx - _target_w ) + 2]);
				if(uchar_abs(gray, gray_top) > edge_threshold)
					edge_detect_tex[2 * idx + 1] = true; //edge top
			}
		}
	}
	
	//compute blend weight
	for(int x = 0; x < _target_w; ++x)
		for(int y = 0; y < _target_h; ++y)
		{
			blend_weight_tex[x + y * _target_w] = Color(0.0f, 0.0f, 0.0f, 0.0f);
			blend_weight_tex[x + y * _target_w] = compute_blend_weight(x, y);
		}
	

	
	//blend pixel and its neighbors
	for(int x = 0; x < _target_w; ++x)
	{
		for(int y = 0; y < _target_h; ++y)
		{
			int idx = x + y * _target_w;
			Color blend(blend_weight_tex[idx].r,
						y >= _target_h - 1 ? 0.0f : blend_weight_tex[idx + _target_w].g,
						blend_weight_tex[x + y * _target_w].b,
						x >= _target_w - 1 ? 0.0f : blend_weight_tex[idx + 1].a);
			
			float sum = blend.r + blend.g + blend.b + blend.a;
			if(sum > 0.0f)
			{
				Color old;
				old.r = _render_target[4 * idx];
				old.g = _render_target[4 * idx + 1];
				old.b = _render_target[4 * idx + 2];
				old.a = _render_target[4 * idx + 3];
				old *= INV_1_255;
				Color left_blend, right_blend, up_blend, down_blend;
				
				if(blend.r != 0.0f)
				{
					Color up;
					int up_idx = 4 * (idx - _target_w);
					up.r = _render_target[up_idx];
					up.g = _render_target[up_idx + 1];
					up.b = _render_target[up_idx + 2];
					up.a = _render_target[up_idx + 3];
					up *= INV_1_255;
					up_blend = old * (1.0f - blend.r) + up * blend.r;
				}
				else up_blend = old;
				
				if(blend.g != 0.0f)
				{
					Color down;
					int down_idx = 4 * (idx + _target_w);
					down.r = _render_target[down_idx];
					down.g = _render_target[down_idx + 1];
					down.b = _render_target[down_idx + 2];
					down.a = _render_target[down_idx + 3];
					down *= INV_1_255;
					down_blend = old * (1.0f - blend.g) + down * blend.g;
				}
				else down_blend = old;
				
				if(blend.b != 0.0f)
				{
					Color left;
					int left_idx = 4 * (idx - 1);
					left.r = _render_target[left_idx];
					left.g = _render_target[left_idx + 1];
					left.b = _render_target[left_idx + 2];
					left.a = _render_target[left_idx + 3];
					left *= INV_1_255;
					left_blend = old * (1.0f - blend.b) + left * blend.b;
				}
				else left_blend = old;
				
				if(blend.a != 0.0f)
				{
					Color right;
					int right_idx = 4 * (idx + 1);
					right.r = _render_target[right_idx];
					right.g = _render_target[right_idx + 1];
					right.b = _render_target[right_idx + 2];
					right.a = _render_target[right_idx + 3];
					right *= INV_1_255;
					right_blend = old * (1.0f - blend.a) + right * blend.a;
				}
				else right_blend = old;

				Color col_new = up_blend * blend.r + down_blend * blend.g + left_blend * blend.b + right_blend * blend.a;
				col_new *= (1.0f / sum);
				//get_gray_scale(col_new.r * 255.0f, col_new.g * 255.0f, col_new.b * 255.0f)
				//col_new.a = 1.0f;
				draw_pixel(x, y, col_new);
			}
		}
	}
	
	//output_result("edge.tif", "weight.tif", "output.tif");
}

void mlaa_wrapper::output_result(const char* edgeFileName, const char* weightFileName, const char* resultFileName)
{
	unsigned char* edge_out = new unsigned char[4 * _target_w * _target_h];
	unsigned char* weight_out = new unsigned char[4 * _target_w * _target_h];
	
	for(unsigned int x = 0; x < _target_w; ++x)
	{
		
		for(unsigned int y = 0; y < _target_h; ++y)
		{
			//ABGR
			unsigned int idx = x + y * _target_w;
			edge_out[4 * idx + 3] = 0xFF; //A
			edge_out[4 * idx + 2] = 0x00; //B
			edge_out[4 * idx + 1] = edge_detect_tex[2 * idx + 1] ? 0xFF : 0x00; //G
			edge_out[4 * idx + 0] = edge_detect_tex[2 * idx] ? 0xFF : 0x00; //R
			
			weight_out[4 * idx + 3] = (uint8_t) (blend_weight_tex[idx].a * 255);
			weight_out[4 * idx + 2] = (uint8_t) (blend_weight_tex[idx].b * 255);
			weight_out[4 * idx + 1] = (uint8_t) (blend_weight_tex[idx].g * 255);
			weight_out[4 * idx + 0] = (uint8_t) (blend_weight_tex[idx].r * 255);
			//idx = x + y * _target_w;
		}
	}
	
//	save_tex(edgeFileName, edge_out, _target_w, _target_h);
//	save_tex(weightFileName, weight_out, _target_w, _target_h);
//	save_tex(resultFileName, _render_target, _target_w, _target_h);
	
	delete [] edge_out;
	delete [] weight_out;
	
}