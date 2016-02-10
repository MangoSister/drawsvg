#ifndef CMU462_SOFTWARE_RENDERER_H
#define CMU462_SOFTWARE_RENDERER_H

#include <stdio.h>
#include <vector>

#include "CMU462.h"
#include "texture.h"
#include "svg_renderer.h"
#include "mlaa.hpp"

namespace CMU462 { // CMU462

class SoftwareRenderer : public SVGRenderer {
 public:

  SoftwareRenderer( ) : sample_rate (1) { }

  // Free used resources
  virtual ~SoftwareRenderer( ) { }

  // Draw an svg input to render target
  virtual void draw_svg( SVG& svg ) = 0;

  // Set sample rate
  virtual void set_sample_rate( size_t sample_rate ) = 0;
  
  // Set render target
  virtual void set_render_target( unsigned char* render_target,
                                  size_t width, size_t height ) = 0;

  // Clear render target
  inline void clear_target() {
    memset(render_target, 255, 4 * target_w * target_h);
  }

  // Set texture sampler
  inline void set_tex_sampler( Sampler2D* sampler ) {
    this->sampler = sampler;
  }

  // Set svg to screen transformation
  inline void set_canvas_to_screen( Matrix3x3 canvas_to_screen ) {
    this->canvas_to_screen = canvas_to_screen;
  }

 protected:

  // Sample rate (square root of samples per pixel)
  size_t sample_rate;

  // Render target memory location
  unsigned char* render_target; 

  // Target buffer dimension (in pixels)
  size_t target_w; size_t target_h;

  // Texture sampler being used
  Sampler2D* sampler;

  // SVG coordinates to screen space coordinates
  Matrix3x3 canvas_to_screen;

}; // class SoftwareRenderer


class SoftwareRendererImp : public SoftwareRenderer {
 public:


  SoftwareRendererImp();

  virtual ~SoftwareRendererImp();
  // draw an svg input to render target
  void draw_svg( SVG& svg );

  // set sample rate
  void set_sample_rate( size_t sample_rate );
  
  // set render target
  void set_render_target( unsigned char* target_buffer,
                          size_t width, size_t height );
	
	inline bool get_is_MLAA_on() const { return is_MLAA_on; }
	inline void set_is_MLAA_on(bool on) { is_MLAA_on = on; }

 private:

  // Primitive Drawing //

  // Draws an SVG element
  void draw_element( SVGElement* element );

  // Draws a point
  void draw_point( Point& p );

  // Draw a line
  void draw_line( Line& line );

  // Draw a polyline
  void draw_polyline( Polyline& polyline );

  // Draw a rectangle
  void draw_rect ( Rect& rect );

  // Draw a polygon
  void draw_polygon( Polygon& polygon );

  // Draw a ellipse
  void draw_ellipse( Ellipse& ellipse );

  // Draws a bitmap image
  void draw_image( Image& image );

  // Draw a group
  void draw_group( Group& group );

  // Rasterization //

  // rasterize a point
  void rasterize_point( float x, float y, Color color );

  // rasterize a line
  void rasterize_line( float x0, float y0,
                       float x1, float y1,
                       Color color);

  // rasterize a triangle
  void rasterize_triangle( float x0, float y0,
                           float x1, float y1,
                           float x2, float y2,
                           Color color );

  // rasterize an image
  void rasterize_image( float x0, float y0,
                        float x1, float y1,
                        Texture& tex );

  // resolve samples to render target
  void resolve( void );
	
	//draw pixel utility function
	inline void draw_pixel(int ix, int iy, const Color& color)
	{
		if ( ix < 0 || ix >= target_w || iy < 0 || iy >= target_h ) return;
		
		int idx = 4 * (ix + iy * target_w);
		
		Color target_color(render_target[idx],
												render_target[idx + 1],
												render_target[idx + 2],
												render_target[idx + 3]);
		
		float out_a = (1.0f - color.a) * target_color.a + color.a;
		float inv_out_a = 1.0f / out_a;
		target_color.r = ((1.0f - color.a) * target_color.r * target_color.a + color.a * color.r) * inv_out_a;
		target_color.g = ((1.0f - color.a) * target_color.g * target_color.a + color.a * color.g) * inv_out_a;
		target_color.b = ((1.0f - color.a) * target_color.b * target_color.a + color.a * color.b) * inv_out_a;
		target_color.a = out_a;

		render_target[idx] = (uint8_t) (target_color.r * 255.0f);
		render_target[idx + 1] = (uint8_t) (target_color.g * 255.0f);
		render_target[idx + 2] = (uint8_t) (target_color.b * 255.0f);
		render_target[idx + 3] = (uint8_t) (target_color.a * 255.0f);
	}
	
	//MLAA (CPU version... sad)
	bool is_MLAA_on;
	mlaa_wrapper* mlaa;

}; // class SoftwareRendererImp


class SoftwareRendererRef : public SoftwareRenderer {
 public:

  SoftwareRendererRef( ) : SoftwareRenderer( ) { }

  // draw an svg input to render target
  void draw_svg( SVG& svg );

  // set sample rate
  void set_sample_rate( size_t sample_rate );
  
  // set render target
  void set_render_target( unsigned char* target_buffer,
                          size_t width, size_t height );

 private:

  // Primitive Drawing //

  // Draws an SVG element
  void draw_element( SVGElement* element );

  // Draws a point
  void draw_point( Point& p );

  // Draw a line
  void draw_line( Line& line );

  // Draw a polyline
  void draw_polyline( Polyline& polyline );

  // Draw a rectangle
  void draw_rect ( Rect& rect );

  // Draw a polygon
  void draw_polygon( Polygon& polygon );

  // Draw a ellipse
  void draw_ellipse( Ellipse& ellipse );

  // Draws a bitmap image
  void draw_image( Image& image );

  // Draw a group
  void draw_group( Group& group );

  // Rasterization //

  // rasterize a point
  void rasterize_point( float x, float y, Color color );

  // rasterize a line
  void rasterize_line( float x0, float y0,
                       float x1, float y1,
                       Color color);

  // rasterize a triangle
  void rasterize_triangle( float x0, float y0,
                           float x1, float y1,
                           float x2, float y2,
                           Color color );

  // rasterize an image
  void rasterize_image( float x0, float y0,
                        float x1, float y1,
                        Texture& tex );

  // resolve samples to render target
  void resolve( void );

  // Helpers //
  // HINT: you may want to have something similar //
  std::vector<unsigned char> sample_buffer; int w; int h;
  void fill_sample( int sx, int sy, const Color& c );
  void fill_pixel( int x, int y, const Color& c );

}; // class SoftwareRendererRef

} // namespace CMU462

#endif // CMU462_SOFTWARE_RENDERER_H
