#include "LinearAlgebra.h"
#include "VisionMemory.h"


int GetCameraCoords_From_Angle(float horizontal_angle,float vertical_angle,unsigned int * uint_pixel_x,unsigned int * uint_pixel_y)
{
 /*
    horizontal_angle == 0 AND vertical_angle == 0 ---> The center of the Image

    horizontal_angle ( left is less , right is more )
    vertical angle ( down is less , up is more )

     When X is 0 horizontal -> angle -field_of_view
     When X is RES_X/2 -> horizontal angle 0
     When X is RES_X -> horizontal angle +field_of_view
  */
  float degree_step_x = 0 , degree_step_y = 0;
  if (camera_horizontal_field_of_view !=0) { degree_step_x = metrics[RESOLUTION_X] / camera_horizontal_field_of_view; } else
                                           { fprintf(stderr,"Camera horizontal field of view NOT set .. , this will probably keep popping up , until you call VisCortx_SetCamerasGeometry\n"); return 0; }
  if ( degree_step_x == 0 ) { fprintf(stderr,"Camera resolution probably not set , cannot continue to get depth \n"); return 0; }


  if (camera_vertical_field_of_view !=0) { degree_step_y = metrics[RESOLUTION_Y] / camera_vertical_field_of_view; } else
                                         { fprintf(stderr,"Camera vertical field of view NOT set .. , this will probably keep popping up , until you call VisCortx_SetCamerasGeometry\n"); return 0; }
  if ( degree_step_y == 0 ) { fprintf(stderr,"Camera resolution probably not set , cannot continue to get depth \n"); return 0; }

  *uint_pixel_x=metrics[RESOLUTION_X] / 2;
  *uint_pixel_y=metrics[RESOLUTION_Y] / 2;

  float abs_horizontal_angle = horizontal_angle; if ( abs_horizontal_angle < 0 ) { abs_horizontal_angle = abs_horizontal_angle  * (-1); }
  float abs_vertical_angle = vertical_angle;     if ( abs_vertical_angle < 0 ) { abs_vertical_angle = abs_vertical_angle  * (-1); }

  float pixel_x = abs_horizontal_angle / degree_step_x  , pixel_y =abs_vertical_angle / degree_step_y;
  if ( horizontal_angle < 0 ) { pixel_x  = (metrics[RESOLUTION_X] / 2) - abs_horizontal_angle; }
  if ( vertical_angle < 0   ) { pixel_y  = (metrics[RESOLUTION_Y] / 2) - abs_vertical_angle;   }

  if ( ( pixel_x >= 0.0 ) && (pixel_x < metrics[RESOLUTION_X] ) ) { *uint_pixel_x = (unsigned int) pixel_x; }
  if ( ( pixel_y >= 0.0 ) && (pixel_y < metrics[RESOLUTION_Y] ) ) { *uint_pixel_y = (unsigned int) pixel_y; }

  return 1;
}