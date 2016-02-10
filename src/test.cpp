//
//  test.cpp
//  Project1
//
//  Created by MangoSister on 16/1/12.
//
//

#include <stdio.h>
#include "math_helper.h"

using namespace std;
using namespace CMU462;

int main( int argc, char** argv )
{
	float x0clip, y0clip, x1clip, y1clip;
	clip_screen_line(250.0f, 150.0f, 10.0f, 220.0f, 200.0f, 200.0f, x0clip, y0clip, x1clip, y1clip);
}