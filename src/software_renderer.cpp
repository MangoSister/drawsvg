#include "software_renderer.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

#include "triangulation.h"
#include "helper.h"

using namespace std;

namespace CMU462 {


// Implements SoftwareRenderer //

	SoftwareRendererImp:: SoftwareRendererImp( )
	: SoftwareRenderer( ), mlaa(nullptr)
	{
		is_MLAA_on = false;
	}

	SoftwareRendererImp::~SoftwareRendererImp()
	{
		if(mlaa != nullptr)
		{
			delete mlaa;
			mlaa = nullptr;
		}
	}

void SoftwareRendererImp::draw_svg( SVG& svg ) {

  // set top level transformation
  transformation = canvas_to_screen;

  // draw all elements
  for ( size_t i = 0; i < svg.elements.size(); ++i ) {
    draw_element(svg.elements[i]);
  }

  // draw canvas outline
  Vector2D a = transform(Vector2D(    0    ,     0    )); a.x--; a.y++;
  Vector2D b = transform(Vector2D(svg.width,     0    )); b.x++; b.y++;
  Vector2D c = transform(Vector2D(    0    ,svg.height)); c.x--; c.y--;
  Vector2D d = transform(Vector2D(svg.width,svg.height)); d.x++; d.y--;

  rasterize_line(a.x, a.y, b.x, b.y, Color::Black);
  rasterize_line(a.x, a.y, c.x, c.y, Color::Black);
  rasterize_line(d.x, d.y, b.x, b.y, Color::Black);
  rasterize_line(d.x, d.y, c.x, c.y, Color::Black);

  // resolve and send to render target
  resolve();

}

void SoftwareRendererImp::set_sample_rate( size_t sample_rate ) {

  // Task 3:
  // You may want to modify this for supersampling support
  this->sample_rate = sample_rate;

}

void SoftwareRendererImp::set_render_target( unsigned char* render_target,
                                             size_t width, size_t height ) {

  // Task 3:
  // You may want to modify this for supersampling support
  this->render_target = render_target;
  this->target_w = width;
  this->target_h = height;

	if(mlaa != nullptr)
	{
		delete mlaa;
		mlaa = nullptr;
	}
	
	mlaa = new mlaa_wrapper(render_target, target_w, target_h);
	
}

void SoftwareRendererImp::draw_element( SVGElement* element ) {

  // Task 4 (part 1):
  // Modify this to implement the transformation stack
	Matrix3x3 matView = transformation;
	//push matrix
	transformation  = transformation * element->transform; //matModel

  switch(element->type) {
    case POINT:
      draw_point(static_cast<Point&>(*element));
      break;
    case LINE:
      draw_line(static_cast<Line&>(*element));
      break;
    case POLYLINE:
      draw_polyline(static_cast<Polyline&>(*element));
      break;
    case RECT:
      draw_rect(static_cast<Rect&>(*element));
      break;
    case POLYGON:
      draw_polygon(static_cast<Polygon&>(*element));
      break;
    case ELLIPSE:
      draw_ellipse(static_cast<Ellipse&>(*element));
      break;
    case IMAGE:
      draw_image(static_cast<Image&>(*element));
      break;
    case GROUP:
      draw_group(static_cast<Group&>(*element));
      break;
    default:
      break;
  }

	//pop matrix
	transformation = matView;
}


// Primitive Drawing //

void SoftwareRendererImp::draw_point( Point& point ) {

  Vector2D p = transform(point.position);
  rasterize_point( p.x, p.y, point.style.fillColor );

}

void SoftwareRendererImp::draw_line( Line& line ) {

  Vector2D p0 = transform(line.from);
  Vector2D p1 = transform(line.to);
  rasterize_line( p0.x, p0.y, p1.x, p1.y, line.style.strokeColor );

}

void SoftwareRendererImp::draw_polyline( Polyline& polyline ) {

  Color c = polyline.style.strokeColor;

  if( c.a != 0 ) {
    int nPoints = polyline.points.size();
    for( int i = 0; i < nPoints - 1; i++ ) {
      Vector2D p0 = transform(polyline.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polyline.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_rect( Rect& rect ) {

  Color c;

  // draw as two triangles
  float x = rect.position.x;
  float y = rect.position.y;
  float w = rect.dimension.x;
  float h = rect.dimension.y;

  Vector2D p0 = transform(Vector2D(   x   ,   y   ));
  Vector2D p1 = transform(Vector2D( x + w ,   y   ));
  Vector2D p2 = transform(Vector2D(   x   , y + h ));
  Vector2D p3 = transform(Vector2D( x + w , y + h ));

  // draw fill
  c = rect.style.fillColor;
  if (c.a != 0 ) {
    rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    rasterize_triangle( p2.x, p2.y, p1.x, p1.y, p3.x, p3.y, c );
  }

  // draw outline
  c = rect.style.strokeColor;
  if( c.a != 0 ) {
    rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    rasterize_line( p1.x, p1.y, p3.x, p3.y, c );
    rasterize_line( p3.x, p3.y, p2.x, p2.y, c );
    rasterize_line( p2.x, p2.y, p0.x, p0.y, c );
  }

}

void SoftwareRendererImp::draw_polygon( Polygon& polygon ) {

  Color c;

  // draw fill
  c = polygon.style.fillColor;
  if( c.a != 0 ) {

    // triangulate
    vector<Vector2D> triangles;
    triangulate( polygon, triangles );

    // draw as triangles
    for (size_t i = 0; i < triangles.size(); i += 3) {
      Vector2D p0 = transform(triangles[i + 0]);
      Vector2D p1 = transform(triangles[i + 1]);
      Vector2D p2 = transform(triangles[i + 2]);
      rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    }
  }

  // draw outline
  c = polygon.style.strokeColor;
  if( c.a != 0 ) {
    int nPoints = polygon.points.size();
    for( int i = 0; i < nPoints; i++ ) {
      Vector2D p0 = transform(polygon.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polygon.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_ellipse( Ellipse& ellipse ) {

  // Extra credit

}

void SoftwareRendererImp::draw_image( Image& image ) {

  Vector2D p0 = transform(image.position);
  Vector2D p1 = transform(image.position + image.dimension);

  rasterize_image( p0.x, p0.y, p1.x, p1.y, image.tex );
}

void SoftwareRendererImp::draw_group( Group& group ) {

  for ( size_t i = 0; i < group.elements.size(); ++i ) {
    draw_element(group.elements[i]);
  }

}

// Rasterization //

// The input arguments in the rasterization functions
// below are all defined in screen space coordinates

void SoftwareRendererImp::rasterize_point( float x, float y, Color color ) {

  // fill in the nearest pixel
  int sx = (int) floor(x);
  int sy = (int) floor(y);

  // check bounds
  if ( sx < 0 || sx >= target_w ) return;
  if ( sy < 0 || sy >= target_h ) return;

  // fill sample - NOT doing alpha blending!
  render_target[4 * (sx + sy * target_w)    ] = (uint8_t) (color.r * 255);
  render_target[4 * (sx + sy * target_w) + 1] = (uint8_t) (color.g * 255);
  render_target[4 * (sx + sy * target_w) + 2] = (uint8_t) (color.b * 255);
  render_target[4 * (sx + sy * target_w) + 3] = (uint8_t) (color.a * 255);

}

void SoftwareRendererImp::rasterize_line( float x0, float y0,
                                          float x1, float y1,
                                          Color color) {

  // Task 1:
  // Implement line rasterization

	//HANDLE SPECIAL CONDITION, LINE LESS THAN ONE PIXEL

	//swap based on symmetry
	bool flip = abs(y0 - y1) > abs(x0 - x1);
	float fw = (float)target_w, fh = (float)target_h;
	if(flip)
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		std::swap(fw, fh);
	}
	if(x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	//now x0 <= x1, and abs(slope) <= 1
	//truncate the outside part of the line
	float x0clip, y0clip, x1clip, y1clip;
	if(!clip_screen_line(x0, y0, x1, y1, fw, fh, x0clip, y0clip, x1clip, y1clip))
		return;

	x0 = x0clip;
	y0 = y0clip;
	x1 = x1clip;
	y1 = y1clip;

	//run Wu's antialiasing algo
	float grad = (y0 - y1) / (x0 - x1);

	//endpoint (x0,y0)
	//extend it to the nearest mid coord
	float x_end_extend = std::floor(x0) + 0.5f;
	float y_end_extend = y0 + grad * (x_end_extend - x0);
	//compute pair weight
	float w_up = inv_frac(y_end_extend - 0.5f);
	float w_down = 1.0f - w_up;
	//compute boundary weight
	float bw = inv_frac(x0);
	w_up *= bw;
	w_down *= bw;
	//compute upper index
	int x0extend_i = floor_to_int(x0);
	int y0extend_i_up = floor_to_int(y_end_extend - 0.5f); //lower index + 1
	//draw
	if(flip)
	{
		draw_pixel(y0extend_i_up, x0extend_i, color * w_up);
		draw_pixel(y0extend_i_up + 1, x0extend_i, color * w_down);
	}
	else
	{
		draw_pixel(x0extend_i, y0extend_i_up, color * w_up);
		draw_pixel(x0extend_i, y0extend_i_up + 1, color * w_down);
	}

	float y_acc = y_end_extend + grad;

	//endpoint (x1, y1)
	//extend it to the nearest mid coord
	x_end_extend = std::floor(x1) + 0.5f;
	y_end_extend = y1 + grad * (x_end_extend - x1);
	//compute pair weight
	w_up = inv_frac(y_end_extend - 0.5f);
	w_down = 1.0f - w_up;
	//compute boundary weight
	bw = frac(x0);
	w_up *= bw;
	w_down *= bw;
	//compute upper index
	int x1extend_i = floor_to_int(x1);
	int y1extend_i_up = floor_to_int(y_end_extend - 0.5f); //lower index + 1
	//draw
	if(flip)
	{
		draw_pixel(y1extend_i_up, x1extend_i, color * w_up);
		draw_pixel(y1extend_i_up + 1, x1extend_i, color * w_down);
	}
	else
	{
		draw_pixel(x1extend_i, y1extend_i_up, color * w_up);
		draw_pixel(x1extend_i, y1extend_i_up + 1, color * w_down);
	}

	for(int x = x0extend_i + 1; x <= x1extend_i - 1; ++x)
	{
		w_up = inv_frac(y_acc - 0.5f);
		w_down = 1.0f - w_up;
		int y = floor_to_int(y_acc - 0.5f);
		if(flip)
		{
			draw_pixel(y, x, color * w_up);
			draw_pixel(y + 1, x, color * w_down);
		}
		else
		{
			draw_pixel(x, y, color * w_up);
			draw_pixel(x, y + 1, color * w_down);
		}

		y_acc += grad;
	}

}

void SoftwareRendererImp::rasterize_triangle( float x0, float y0,
                                              float x1, float y1,
                                              float x2, float y2,
                                              Color color ) {

//	x0 = 1160.0f; y0 = 40.0f;
//	x1 = 1060.0f; y1 = 230.0f;
//	x2 = 1330.0f; y2 = 240.0f;
//
//	x0 = 581.236; y0 = 608.850f;
//	x1 = 1364.716; y1 = 544.662f;
//	x2 = 972.977; y2 = 593.747f;

  // Task 2:
  // Implement triangle rasterization
	flip_tri_counter_clockwise(x0 ,y0, x1, y1, x2, y2);
	//clockwise, flip
	//std::swap(x1, x2);
	//std::swap(y1, y2);

	//compute top bound & bottom bound row
	auto comp = [=](float a, float b){ return a < b; };
	auto scope = std::minmax( {y0, y1, y2} , comp);
	int top = floor_to_int(scope.first);
	int bottom = floor_to_int(scope.second); //bottom should be larger
	if(bottom < top)
		return;

	//compute left/right boundary
	int rowNum = bottom - top + 1;
	int* left_boundary = new int[rowNum];
	int* right_boundary = new int[rowNum];
	for(int i = 0; i < rowNum; i ++)
	{
		left_boundary[i] = std::numeric_limits<int>::max();
		right_boundary[i] = std::numeric_limits<int>::min();
	}


	float xs[3] {x0, x1, x2};
	float ys[3] {y0, y1, y2};
	for(int i = 0; i < 3; i++)
	{
		float xstart = xs[i], ystart = ys[i];
		float xend = xs[(i + 1) % 3], yend = ys[(i + 1) % 3];
		float deltax = xend - xstart;
		float deltay = yend - ystart;

		//horizontal edge, skip
		if(deltay == 0.0f)
			continue;

		float a = deltay, b = -deltax, c = ystart * deltax - xstart * deltay;
		if(deltay > 0.0f)
		{
			//left edge
			float y_sp_start = std::floor(ystart + 0.5f) + 0.5f;
			float y_sp_end = std::floor(yend - 0.5f) + 0.5f;

			float x_sp_init = std::ceil((-b * y_sp_start + c) / a);
			float d_init = a*x_sp_init + b*y_sp_start + c;

			for(float y = y_sp_start; y <= y_sp_end; ++y)
			{
				//find the leftmost sample on each row
				float x_sp_left = x_sp_init - std::floor((d_init + b * (y - y_sp_start)) / a);
				if(std::floor(x_sp_left) < left_boundary[floor_to_int(y) - top])
					left_boundary[floor_to_int(y) - top] = std::floor(x_sp_left);
			}
		}
		else
		{
			//right edge
			float y_sp_start = std::floor(ystart - 0.5f) + 0.5f;
			float y_sp_end = std::floor(yend + 0.5f) + 0.5f;

			float x_sp_init = std::floor((-b * y_sp_start + c) / a);
			float d_init = a*x_sp_init + b*y_sp_start + c;

			for(float y = y_sp_start; y >= y_sp_end; --y)
			{
				//find the rightmost sample on each row
				float x_sp_right = x_sp_init - std::ceil( (d_init + b * (y - y_sp_start)) / a );
				if(std::floor(x_sp_right) > right_boundary[floor_to_int(y) - top])
					right_boundary[floor_to_int(y) - top] = std::floor(x_sp_right);
			}
		}
	}

	//draw each row
	for(int yOffset = 0; yOffset <= bottom - top; ++ yOffset)
	{
		if(left_boundary[yOffset] > right_boundary[yOffset])
			continue;
		for(int x = left_boundary[yOffset]; x <= right_boundary[yOffset]; ++x)
			draw_pixel(x, yOffset + top, color);
	}

	//clean up
	delete[] left_boundary;
	delete[] right_boundary;

}

void SoftwareRendererImp::rasterize_image( float x0, float y0,
                                           float x1, float y1,
                                           Texture& tex ) {
  // Task ?:
  // Implement image rasterization
  float x_min = std::min(x0, x1);
  float x_max = std::max(x0, x1);
  float y_min = std::min(y0, y1);
  float y_max = std::max(y0, y1);

  float x_start = std::floor(x_min + 0.5f);
  float x_end = std::floor(x_max - 0.5f);
  float y_start = std::floor(y_min + 0.5f);
  float y_end = std::floor(y_max - 0.5f);

  float img_width = x_max - x_min;
  float img_height = y_max - y_min;
	
	switch(sampler->get_sample_method())
	{
		case TRILINEAR:
		{
			for(float x = x_start; x <= x_end; x += 1.0f)
			{
				for(float y = y_start; y <= y_end; y += 1.0f)
				{
					
					Color c = sampler->sample_trilinear(tex, (x - x_min) / img_width,
																										(y - y_min) / img_height,
																										(float)tex.width / img_width,
																										(float)tex.height / img_height);
					draw_pixel(floor_to_int(x), floor_to_int(y), c);
				}
			}
			break;
		}
		case BILINEAR: default:
		{
			for(float x = x_start; x <= x_end; x += 1.0f)
			{
				for(float y = y_start; y <= y_end; y += 1.0f)
				{
					Color c = sampler->sample_bilinear(tex, (x - x_min) / img_width,
																						 (y - y_min) / img_height );
					draw_pixel(floor_to_int(x), floor_to_int(y), c);
				}
			}
			break;
		}
	}
}

// resolve samples to render target
void SoftwareRendererImp::resolve( void )
{

	// Task 3:
	// Implement supersampling
	// You may also need to modify other functions marked with "Task 3".

	//MLAA
	if(!is_MLAA_on)
		return;

	mlaa->process_mlaa();
}

}// namespace CMU462
