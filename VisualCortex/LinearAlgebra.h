#ifndef LINEARALGEBRA_H_INCLUDED
#define LINEARALGEBRA_H_INCLUDED


#include "VisionMemory.h"

int GetCameraCoords_From_Angle(float horizontal_angle,float vertical_angle,unsigned int * uint_pixel_x,unsigned int * uint_pixel_y);

int ComputeFundamentalMatrixFromPointCorrespondance(struct FeatureList * list,struct FundamentalMatrix * E);

#endif // LINEARALGEBRA_H_INCLUDED
