#include "LinearAlgebra.h"
#include "VisionMemory.h"
#include <stdio.h>
#include <gsl/gsl_linalg.h>

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






/*


int ComputeFundamentalMatrix(void)
{
    //The following program solves the linear system A x = b. The system to be solved is, and the solution is found using LU decomposition of the matrix A.
       double a_data[] = { 0.18, 0.60, 0.57, 0.96,
                           0.41, 0.24, 0.99, 0.58,
                           0.14, 0.30, 0.97, 0.66,
                           0.51, 0.13, 0.19, 0.85 };

       double b_data[] = { 1.0, 2.0, 3.0, 4.0 };

       gsl_matrix_view m
         = gsl_matrix_view_array (a_data, 4, 4);

       gsl_vector_view b
         = gsl_vector_view_array (b_data, 4);

       gsl_vector *x = gsl_vector_alloc (4);

       int s;

       gsl_permutation * p = gsl_permutation_alloc (4);

       gsl_linalg_LU_decomp (&m.matrix, p, &s);

       gsl_linalg_LU_solve (&m.matrix, p, &b.vector, x);

       printf ("x = \n");
       gsl_vector_fprintf (stdout, x, "%g");

       gsl_permutation_free (p);
       gsl_vector_free (x);
       return 0;
}
*/


int ComputeHomographyFromPointCorrespondanceOpenCV(struct FeatureList * source,struct TransformationMatrix * E)
{
   if ( source->current_features == 0 ) { return 0; }
   int i=0;

   CvMat* srcPoints = cvCreateMat(2,source->current_features,CV_32FC1);
    for ( i=0; i<source->current_features; i++ )
     {   cvmSet(srcPoints,0,i,source->list[i].last_x);
         cvmSet(srcPoints,1,i,source->list[i].last_y); }

   CvMat* dstPoints = cvCreateMat(2,source->current_features,CV_32FC1);
    for ( i=0; i<source->current_features; i++ )
     {   cvmSet(srcPoints,0,i,source->list[i].x);
         cvmSet(srcPoints,1,i,source->list[i].y); }

   CvMat* H =  cvCreateMat(3,3,CV_32FC1);
   int res = cvFindHomography(srcPoints,dstPoints,H,CV_RANSAC,5,0);

   i=0;
   int x,y;
    for(y=0; y<3; y++)
     {
       for(x=0; x<3; x++)
       {
         //fprintf(stderr, "%f ",cvmGet(H,x,y));
         E->item[i++]=cvmGet(H,x,y);
       }
       //fprintf(stderr, "\n");
     }

   cvReleaseMat(&srcPoints);
   cvReleaseMat(&dstPoints);
   cvReleaseMat(&H);

   return res;
}

int ComputeFundamentalMatrixFromPointCorrespondance(struct FeatureList * list,struct TransformationMatrix * E)
{

    // TODO TODO TODO TODO
    //The following program solves the linear system A x = b. The system to be solved is, and the solution is found using LU decomposition of the matrix A.
       double a_data[] = { 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 ,
                           1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 ,
                           1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 ,
                           1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 ,
                           1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 ,
                           1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 ,
                           1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 ,
                           1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 ,
                           1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9
                         };

       double b_data[] = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  };
       unsigned int array_offset=0;
       unsigned int array_row=0;
       unsigned int feature_num=0;

   int i=0;
     while ( i < 9 )
      {

       array_offset =  9 * array_row;
       a_data[array_offset + 0] = list->list[feature_num].x * list->list[feature_num].last_x;
       a_data[array_offset + 1] = list->list[feature_num].x * list->list[feature_num].last_x;
       a_data[array_offset + 2] = list->list[feature_num].x;
       a_data[array_offset + 3] = list->list[feature_num].y * list->list[feature_num].last_x;
       a_data[array_offset + 4] = list->list[feature_num].y * list->list[feature_num].last_y;
       a_data[array_offset + 5] = list->list[feature_num].y;
       a_data[array_offset + 6] = list->list[feature_num].last_x;
       a_data[array_offset + 7] = list->list[feature_num].last_y;
       a_data[array_offset + 8] = 1;

       ++i;
       ++array_row;
       ++feature_num;
      }


       gsl_matrix_view m
         = gsl_matrix_view_array (a_data, 9, 9);

       gsl_vector_view b
         = gsl_vector_view_array (b_data, 9);

       gsl_vector *x = gsl_vector_alloc (9);

       int s;

       gsl_permutation * p = gsl_permutation_alloc (9);

       if ( gsl_linalg_LU_decomp (&m.matrix, p, &s) )
       {
         gsl_linalg_LU_solve (&m.matrix, p, &b.vector, x);



         E->item[0] = gsl_vector_get ( x , 0); // e11
         E->item[1] = gsl_vector_get ( x , 1); // e12
         E->item[2] = gsl_vector_get ( x , 2); // e13
         E->item[3] = gsl_vector_get ( x , 3); // e21
         E->item[4] = gsl_vector_get ( x , 4); // e22
         E->item[5] = gsl_vector_get ( x , 5); // e23
         E->item[6] = gsl_vector_get ( x , 6); // e31
         E->item[7] = gsl_vector_get ( x , 7); // e32
         E->item[8] = gsl_vector_get ( x , 8); // e33

         printf ("x = \n");
         gsl_vector_fprintf (stdout, x, "%g");
       }

       gsl_permutation_free (p);
       gsl_vector_free (x);
       return 0;
}



