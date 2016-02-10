#include "viewport.h"

#include "CMU462.h"

namespace CMU462 {

void ViewportImp::set_viewbox( float x, float y, float span ) {

  // Task 4 (part 2): 
  // Set svg to normalized device coordinate transformation. Your input
  // arguments are defined as SVG canvans coordinates.
  this->x = x;
  this->y = y;
  this->span = span;
	
	//svg_2_nrm
	Matrix3x3 tBack2Origin = Matrix3x3::identity();
	tBack2Origin(0,2) = -x;
	tBack2Origin(1,2) = -y;
	Matrix3x3 tNormalize = Matrix3x3::identity();
	float inv_span = 0.5f / span;
	tNormalize(0,0) = inv_span;
	tNormalize(1,1) = inv_span;
	Matrix3x3 tOffset = Matrix3x3::identity();
	tOffset(0,2) = 0.5f;
	tOffset(1,2) = 0.5f;
	
	svg_2_norm = tOffset * tNormalize * tBack2Origin;
}

void ViewportImp::update_viewbox( float dx, float dy, float scale ) { 
  
  this->x -= dx;
  this->y -= dy;
  this->span *= scale;
  set_viewbox( x, y, span );
}

} // namespace CMU462