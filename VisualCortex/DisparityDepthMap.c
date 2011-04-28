/*
The Visual Cortex of guarddog
URLs: http://ammarkov.ath.cx
Written by Ammar Qammaz a.k.a. AmmarkoV 2010

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "DisparityDepthMap.h"
#include "VisCortexFilters.h"
#include "IntegralImageConversion.h"
#include "MovementRegistration.h"
#include "Precalculations.h"
#include <stdio.h>
#include <string.h>

inline int ComparePatchesUsingHistogram(int hist_reg_left,int hist_reg_right,unsigned int *source_x1,unsigned int *source_y1,unsigned int *target_x1,unsigned int *target_y1)
{
  if (  settings[DEPTHMAP_IMPROVE_USING_HISTOGRAM] ==0 ) { return 0; }

    // NEW HISTOGRAM TESTING :) ++SPEED ++ACCURACY
    struct Histogram hist1;
    struct Histogram hist2;

    CompressedHistogramPatch(l_video_register[HISTOGRAM_COMPRESSED_LEFT].pixels,&hist1,*source_x1,*source_y1);
    CompressedHistogramPatch(l_video_register[HISTOGRAM_COMPRESSED_RIGHT].pixels,&hist2,*target_x1,*target_y1);

    if (
        (precalc_sub[hist1.median_r][hist2.median_r]>settings[PATCH_HIST_THRESHOLD_R])||
        (precalc_sub[hist1.median_g][hist2.median_g]>settings[PATCH_HIST_THRESHOLD_G])||
        (precalc_sub[hist1.median_b][hist2.median_b]>settings[PATCH_HIST_THRESHOLD_B])
       )
          {
            ++metrics[HISTOGRAM_DENIES];
            return 1;
          } /*If histograms greatly different fail the test immediately*/

    // NEW HISTOGRAM TESTING :) ++SPEED ++ACCURACY
    return 0;
}



unsigned int inline ComparePatches(struct ImageRegion source_block,
                                   struct ImageRegion target_block,
                                   unsigned char *left_view,
                                   unsigned char *right_view,
                                   unsigned char *left_sobel,
                                   unsigned char *right_sobel,
                                   unsigned int movement_difference,
                                   unsigned int patch_x,
                                   unsigned int patch_y,
                                   unsigned int image_x,
                                   unsigned int image_y,
                                   unsigned int best_result_yet
                                  )
{

    unsigned int score_threshold = settings[DEPTHMAP_COMPARISON_THRESHOLD];

    	//Kovoume ta source ektos eikonas
	if (source_block.x1+patch_x>=image_x) { return score_threshold+1; }
    if (source_block.y1+patch_y>=image_y) { return score_threshold+1; }
	if (target_block.x1+patch_x>=image_x) { return score_threshold+1; }
    if (target_block.y1+patch_y>=image_y) { return score_threshold+1; }


    if (settings[DEPTHMAP_COMPARISON_DO_NOT_PERFORM_FULL_COUNT])
       { /*In case we dont care for a full count of the total Comparison the threshold is the best result yet :)
           (The only reason we would like a full count is for improving this filter via statistics)*/
           if ( best_result_yet > 0 ) score_threshold = best_result_yet;
       }


    if (ComparePatchesUsingHistogram(HISTOGRAM_COMPRESSED_LEFT,HISTOGRAM_COMPRESSED_RIGHT,&source_block.x1,&source_block.y1,&target_block.x1,&target_block.y1))
          {
            ++metrics[HISTOGRAM_DENIES];
            return score_threshold+1;
          } //If histograms greatly different fail the test immediately ++SPEED ++ACCURACY



    unsigned int total_score=0,pixel_score=0;
	unsigned int sobel_diffr=0;
	register unsigned int y=0;
	char sobel_mismatch=0;

	register BYTE *sobel_px1,*sobel_px2;
	register BYTE *image_px1,*image_px2;
	register BYTE *stopx1;

    unsigned int source_start_memory_point=(unsigned int) precalc_memplace_3byte[source_block.x1][y+source_block.y1];
	unsigned int target_start_memory_point=(unsigned int) precalc_memplace_3byte[target_block.x1][y+target_block.y1];
	unsigned int incrementation_3byte_step = 3 * ( image_x-patch_x );

	  image_px1= (BYTE *) left_view+source_start_memory_point;
	  image_px2= (BYTE *) right_view+target_start_memory_point;
  	  sobel_px1= (BYTE *) left_sobel+source_start_memory_point;
      sobel_px2= (BYTE *) right_sobel+target_start_memory_point;
      //Pointers are ready ..!

	while (y<patch_y)
	{
      //Prepare the pointers of the left/right patch image and the left/right patch sobel
      image_px1+=incrementation_3byte_step;
      image_px2+=incrementation_3byte_step;
      sobel_px1+=incrementation_3byte_step;
      sobel_px2+=incrementation_3byte_step;

      stopx1=image_px1+patch_x;

	  sobel_mismatch=0;
      while (image_px1<stopx1)
	  {
		pixel_score=0; // This pixel initially has no score..

		// BIGER SCORE -> MORE PATCH DIFFERENCE  !
		sobel_diffr=precalc_sub[*sobel_px1][*sobel_px2]; //This holds the sobel difference value
        if ( sobel_diffr > 30 ) { sobel_mismatch=1; }
        // BIGER SCORE -> MORE PATCH DIFFERENCE  !
        sobel_px1+=3; sobel_px2+=3;
        pixel_score=sobel_diffr;



        // BIGER SCORE -> MORE PATCH DIFFERENCE  !
		pixel_score+= ( precalc_sub[*image_px1] [*image_px2]  ); image_px1++; image_px2++;
		pixel_score+= ( precalc_sub[*image_px1] [*image_px2]  ); image_px1++; image_px2++;
		pixel_score+= ( precalc_sub[*image_px1] [*image_px2]  ); image_px1++; image_px2++;
		// BIGER SCORE -> MORE PATCH DIFFERENCE  !



		if ( sobel_mismatch == 1 )
            { pixel_score = pixel_score << 2; } // *8 Multiply score ( because the sobel edges are mismatched

		total_score+=pixel_score;
        if ( score_threshold<total_score )
          {
             ++metrics[COMPAREPATCH_ALGORITHM_DENIES];
             return total_score; // SPEED CUT :) ++PERFOMANCE  --DEBUGGING :P
          }

	  }


	  ++y;
	}

/* SHOULD BE ADDED ON SPEED CUT TO
	  if ( settings[DEPTHMAP_IMPROVE_USING_MOVEMENT] )
                        {
                           //TODO ADD HERE MOVEMENT SCORE
                           if ( movement_difference > 150 ) { movement_difference<<3; total_score+=movement_difference; }
                       }*/


  if ( total_score < 100 )
    {
        fprintf(stderr,"PATCH %u,%u -> %u,%u yields %u (patch size %ux%u) \n",source_block.x1,source_block.y1,target_block.x1,target_block.y1,total_score,patch_x,patch_y);
        fprintf(stderr,"Source Start memory point %u Target Start memory point %u \n",precalc_memplace_3byte[source_block.x1][y+source_block.y1],precalc_memplace_3byte[target_block.x1][y+target_block.y1]);
    }


 return total_score;
}

void inline FillDepthMemWithData(unsigned short * depth_data_raw,struct DepthData * depth_data_full,struct DepthData *depth_data,unsigned int image_x,unsigned int image_y)
{

	 unsigned int far_away=0;
     if (depth_data->x1_patch-depth_data->x2_patch>0) { far_away=depth_data->x1_patch-depth_data->x2_patch; } else
     if (depth_data->x2_patch-depth_data->x1_patch>0) { far_away=depth_data->x2_patch-depth_data->x1_patch; }

	 unsigned int x=0;
	 unsigned int y=0;
	 unsigned int xlim=0 , ylim=0 , full_lim=image_x*image_y;

	y=depth_data->y1_patch;
	ylim=y+depth_data->patch_size_y;
	while (y<ylim)
	{
		x = ( (y) * image_x ) + ( depth_data->x1_patch );
	    xlim=x+depth_data->patch_size_x;

		while ( ( x<xlim ) && ( x < full_lim) )
		{
		    depth_data_raw[x]=far_away;
			depth_data_full[x].depth=depth_data->depth;
            depth_data_full[x].score=depth_data->score;
            depth_data_full[x].edge_count=depth_data->edge_count;
            depth_data_full[x].movement_count=depth_data->movement_count;
            depth_data_full[x].movement_difference=depth_data->movement_difference;
            depth_data_full[x].x1_patch=depth_data->x1_patch;
            depth_data_full[x].y1_patch=depth_data->y1_patch;
            depth_data_full[x].x2_patch=depth_data->x2_patch;
            depth_data_full[x].y2_patch=depth_data->y2_patch;
		    depth_data_full[x].patch_size_x=depth_data->patch_size_x;
            depth_data_full[x].patch_size_y=depth_data->patch_size_y;
			++x;
		}
	  ++y;
	}

}


void PassGuessNextDepthMem(unsigned int prox,unsigned int patch_x,unsigned int patch_y,unsigned short * depth_data_raw,struct DepthData * depth_data_full,struct DepthData *depth_data,unsigned int image_x,unsigned int image_y)
{

		//if (depth_data.x1_patch>image_x) { depth_data.x1_patch=image_x; }
	    //if (depth_data.x2_patch>image_x) { depth_data.x2_patch=image_x; }
		//if (depth_data.x2_patch<=depth_data.x1_patch) {depth_data.x1_patch=depth_data.x2_patch-1;}
        depth_data->score=prox;
        FillDepthMemWithData(depth_data_raw,depth_data_full,depth_data,image_x,image_y);
}



void DepthMapFull  ( unsigned int left_view_reg,
                     unsigned int right_view_reg,
                     unsigned int left_depth_reg,
                     unsigned int right_depth_reg,
                     unsigned int image_x,
                     unsigned int image_y,
                     unsigned char clear_depth_arrays /*Cleaning the depth arrays takes a little longer :) */
                    )
{
    unsigned char *left_view=video_register[left_view_reg].pixels;
    unsigned char *right_view=video_register[right_view_reg].pixels;
    unsigned short *left_depth=l_video_register[left_depth_reg].pixels;
    unsigned short *right_depth=l_video_register[right_depth_reg].pixels;

	if ( (left_view==0)||(right_view==0)||(left_depth==0)||(right_depth==0)) { fprintf(stderr,"DepthMap Video `registers` not ready..!"); return; }


	unsigned int edges_required_to_process=( (unsigned int) ( metrics[VERTICAL_BUFFER] * metrics[HORIZONTAL_BUFFER] * settings[PATCH_COMPARISON_EDGES_PERCENT_REQUIRED] )  / 100 );

    if ( clear_depth_arrays == 1 )
    {
     ClearLargeVideoRegister(left_depth_reg);
     ClearLargeVideoRegister(right_depth_reg);
     memset(depth_data_array,0,sizeof(struct DepthData) * image_x*image_y );// CLEAR DEPTH MAP FROM ARTIFACTS
    }

   if (  settings[DEPTHMAP_IMPROVE_USING_HISTOGRAM] == 1 )
   {
       GenerateCompressHistogramOfImage(video_register[LEFT_EYE].pixels,l_video_register[HISTOGRAM_COMPRESSED_LEFT].pixels,metrics[HORIZONTAL_BUFFER],metrics[VERTICAL_BUFFER]);
       GenerateCompressHistogramOfImage(video_register[RIGHT_EYE].pixels,l_video_register[HISTOGRAM_COMPRESSED_RIGHT].pixels,metrics[HORIZONTAL_BUFFER],metrics[VERTICAL_BUFFER]);
   }

    PrepareCleanSobeledGaussian(video_register[LEFT_EYE].pixels,video_register[EDGES_LEFT].pixels,settings[DEPTHMAP_EDGE_STRICTNESS]);
    PrepareCleanSobeledGaussian(video_register[RIGHT_EYE].pixels,video_register[EDGES_RIGHT].pixels,settings[DEPTHMAP_EDGE_STRICTNESS]);

    if ( clear_depth_arrays == 1 )
     {

        // CompressPresenceRegister(EDGES_LEFT,GENERAL_XLARGE_1,5);
        // I dont like the following , I should really change the sobel function to give a one byte response..!
        CopyRegister(EDGES_LEFT,GENERAL_3);
        ConvertRegisterFrom3ByteTo1Byte(GENERAL_3,metrics[RESOLUTION_X],metrics[RESOLUTION_Y]);
        PixelsOverThresholdSetAsOne(GENERAL_3,1);
        CompressRegister(GENERAL_3,GENERAL_XLARGE_1);
        CompressRegister(MOVEMENT_LEFT,MOVEMENT_GROUPED_LEFT);
        CompressRegister(MOVEMENT_RIGHT,MOVEMENT_GROUPED_RIGHT);
     }

    metrics[HISTOGRAM_DENIES]=0;
    metrics[COMPAREPATCH_ALGORITHM_DENIES]=0;
	struct DepthData best_match={0};
	best_match.patch_size_x=(unsigned short) metrics[HORIZONTAL_BUFFER];
	best_match.patch_size_y=(unsigned short) metrics[VERTICAL_BUFFER];

    unsigned int xlim=image_x-metrics[HORIZONTAL_BUFFER];
    unsigned int ylim=image_y-metrics[VERTICAL_BUFFER]; // Added 5/3/2010 :) SPEED++ Quality ++
    unsigned int x=settings[DEPTHMAP_STARTLEFT_X],y=1;

	unsigned int x_vima=metrics[HORIZONTAL_BUFFER] , y_vima=metrics[VERTICAL_BUFFER];

	if ( settings[DEPTHMAP_DETAIL] <= 0 ) { settings[DEPTHMAP_DETAIL]=1; } // :D , swstos programmatismos!
    x_vima= (unsigned int) (metrics[HORIZONTAL_BUFFER] / settings[DEPTHMAP_DETAIL]);
    y_vima= (unsigned int) (metrics[VERTICAL_BUFFER] / settings[DEPTHMAP_DETAIL]);

	unsigned int xblock=0 , yblock=0 ;
    unsigned int  prox=0,movement_left=0 , movement_right=0 ,movement_difference =0;
    unsigned int  max_prox_score = settings[DEPTHMAP_COMPARISON_THRESHOLD]+settings[DEPTHMAP_COMPARISON_THRESHOLD_ADDED];



    while (y<ylim)
	   {
	     x=settings[DEPTHMAP_STARTLEFT_X];
         while (x<xlim)
		 {
		    // ARXI EKSWTERIKIS EPILOGIS BLOCK
            best_match.score=settings[DEPTHMAP_COMPARISON_THRESHOLD]+1;
            best_match.x1_patch=0; best_match.y1_patch=0;
            best_match.x2_patch=0; best_match.y2_patch=0;
            best_match.edge_count=0;
            best_match.movement_difference =0; best_match.movement_count=0;

             //THA PREPEI TO SOURCE KOMMATI NA EXEI EDGES GIATI ALLIWS DEN EINAI K POLY AKRIVES TO ANTISTOIXO POU THA VRETHEI

             //unsigned int  counted_edges=CountEdges(edges_required_to_process,x,y,metrics[HORIZONTAL_BUFFER],metrics[VERTICAL_BUFFER],video_register[EDGES_LEFT].pixels);
             unsigned int  counted_edges=GetCompressedRegisterPatchSum1Byte(GENERAL_XLARGE_1,x,y,metrics[HORIZONTAL_BUFFER],metrics[VERTICAL_BUFFER]);


             //DEBUG COMMAND TO FIND WTF IS WRONG if (counted_edges>edges_required_to_process) { counted_edges=edges_required_to_process+1; }
		     //THA PREPEI TO SOURCE KOMMATI NA EXEI EDGES GIATI ALLIWS DEN EINAI K POLY AKRIVES TO ANTISTOIXO POU THA VRETHEI

				if (counted_edges>edges_required_to_process)
				      {
				         best_match.edge_count=counted_edges;  //APOTHIKEVOUME TA EDGES!

			             // ARXIZEI H SYGKRISI!
                          unsigned int x_start_right=0;
                          unsigned int x_end_right=x;

                          if ( (x > settings[DEPTHMAP_CLOSEST_DEPTH] ) && (settings[DEPTHMAP_COMPARISON_DO_NOT_PROCESS_FURTHER_THAN_CLOSEST_DEPTH]) )
                            {
                              /* If the x point is very right , we should only process the patches , near at least
                              settings[DEPTHMAP_CLOSEST_DEPTH] pixels to the patch on the right*/
                              x_start_right = x-settings[DEPTHMAP_CLOSEST_DEPTH];
                            }

						 // ESWTERIKO LOOP I DEKSIA EIKONA
						 xblock=x_start_right;  //THELOUME NA KSEKINISOUME APO TIN ARXI TIS DEKSIAS EIKONAS
                         yblock=y-settings[DEPTHMAP_VERT_OFFSET_UP]; //THELOUME NA KSEKINISOUME STIN IDIA SCAN LINE ME ENA PITHANO OFFSET LOGW KAKOU CALIBRATION


						 struct ImageRegion source_rgn,target_rgn;
                         source_rgn.x1=x; source_rgn.y1=y; // To source ( tou aristerou matiou einai x,y )
                         source_rgn.width=metrics[HORIZONTAL_BUFFER]; source_rgn.height=metrics[VERTICAL_BUFFER]; // These are standard
                         target_rgn.width=metrics[HORIZONTAL_BUFFER]; target_rgn.height=metrics[VERTICAL_BUFFER]; // These are standard

                         movement_left=GetMovementAtBlock(MOVEMENT_GROUPED_LEFT,&source_rgn);

                         while ( yblock <= y+settings[DEPTHMAP_VERT_OFFSET_DOWN] )
						 { //KATHETI METATWPISI TOU BUFFER DEKSIAS EIKONAS
						     while ( xblock <= x_end_right )
							 {
							   //ORIZONTIA METATWPISI TOU BUFFER DEKSIAS EIKONAS

                                target_rgn.x1=xblock; target_rgn.y1=yblock;
                                target_rgn.x2=xblock+metrics[HORIZONTAL_BUFFER]; target_rgn.y2=yblock+metrics[VERTICAL_BUFFER];

                                movement_right=GetMovementAtBlock(MOVEMENT_GROUPED_RIGHT,&target_rgn);
                                movement_difference=abs(movement_right-movement_left);

								prox = ComparePatches( source_rgn , target_rgn,
                                                       left_view  , right_view,
                                                       video_register[EDGES_LEFT].pixels , video_register[EDGES_RIGHT].pixels ,
                                                       movement_difference ,
                                                       metrics[HORIZONTAL_BUFFER] , metrics[VERTICAL_BUFFER],
                                                       image_x , image_y , best_match.score);

								// TEST/DOKIMASTIKO DEPTH GIA TEST CULLING
								// H IDEA EINAI OTI ANTIKEIMENA POU EINAI PAAAAAAARA POLY KONTA EINIA MALLON ERRORS
								 unsigned int depth_act;
								 if ( x > xblock ) { depth_act=x-xblock; } else
								                   { depth_act=xblock-x; }
								                   depth_act=0;
								// TEST/DOKIMASTIKO ALLA VGAZEI KALO APOTELESMA

                                if (
                                         (prox<best_match.score) // kanoume qualify san kalytero apotelesma
                                      && (prox<max_prox_score) // to threshold mas
									  && (depth_act<settings[DEPTHMAP_CLOSEST_DEPTH])  // TEST -> Praktika dedomena deixnoun oti synithws apotelesmata panw apo 100 einai thoryvos!
								    )
								{
                                    /* THIS IS THE BEST PATCH SO FAR! */
								    best_match.score=prox;
                                    best_match.x1_patch=x;   // SYNTETAGMENES ARXIS PATCH X STIN EIKONA #1
                                    best_match.y1_patch=y;   // SYNTETAGMENES ARXIS PATCH Y STIN EIKONA #1
                                    best_match.x2_patch=xblock; // SYNTETAGMENES ARXIS PATCH X STIN EIKONA #2
                                    best_match.y2_patch=yblock;   // SYNTETAGMENES ARXIS PATCH Y STIN EIKONA #2
                                    best_match.movement_count=movement_right;
                                    best_match.movement_difference = movement_difference ;

									FillDepthMemWithData(left_depth,depth_data_array,&best_match,image_x,image_y);


							  if (settings[DEPTHMAP_GUESSES]==1)
							    { // CODE TO GUESS NEXT BLOCK!
									 source_rgn.x1=best_match.x1_patch+metrics[HORIZONTAL_BUFFER]; source_rgn.x2=source_rgn.x1+metrics[HORIZONTAL_BUFFER];
									 target_rgn.x1=best_match.x2_patch+metrics[HORIZONTAL_BUFFER]; target_rgn.x2=target_rgn.x1+metrics[HORIZONTAL_BUFFER];
									 if ( (source_rgn.x1>image_x) || (source_rgn.x2>image_x) || (target_rgn.x1>image_x) || (target_rgn.x2>image_x) )
									 {    } // OUT OF MEMORY SPACE NOT GUESSING
								     else
								     {
									  prox = ComparePatches( source_rgn,target_rgn,
                                                             left_view, right_view,
                                                             video_register[EDGES_LEFT].pixels , video_register[EDGES_RIGHT].pixels,
                                                             0,
                                                             metrics[HORIZONTAL_BUFFER] , metrics[VERTICAL_BUFFER],
                                                             image_x , image_y , best_match.score);
									  // HEURISTIC GIA NA PETAME KAKES PROVLEPSEIS

									  if ( (best_match.edge_count>edges_required_to_process) /*&& (prox<settings[PATCH_COMPARISON_SCORE_MIN])*/ && (best_match.score>=prox) )
                                      // HEURISTIC GIA NA PETAME KAKES PROVLEPSEIS
									   {
									     PassGuessNextDepthMem(prox,metrics[HORIZONTAL_BUFFER],metrics[VERTICAL_BUFFER],left_depth,depth_data_array,&best_match,image_x,image_y);
									   }
									 }
								  }	 // CODE TO GUESS NEXT BLOCK!

								}

                               // ORIZONTIA METATWPISI TOU BUFFER DEKSIAS EIKONAS
                               ++xblock;
							 }
                          // KATHETI METATWPISI TOU BUFFER DEKSIAS EIKONAS
                           ++yblock;
						 }

					  }

            // TELOS EKSWTERIKIS EPILOGIS BLOCK
		   x+=x_vima;
		 }
		 y+=y_vima;
	   }

  if ( settings[DEPTHMAP_IMPROVE_FILLING_HOLES]!=0 )  EnhanceDepthMapFillHoles(video_register[LEFT_EYE].pixels,left_depth,image_x,image_y);
  if ( settings[DEPTHMAP_IMPROVE_USING_EDGES]!=0 )  EnhanceDepthMapWithEdges(video_register[LEFT_EYE].pixels,left_depth,video_register[EDGES_LEFT].pixels,image_x,image_y);
  //fprintf(stderr,"Histogram denied %u bad patches / ComparePatches Algorithm denied %u patches \n",metrics[HISTOGRAM_DENIES],metrics[COMPAREPATCH_ALGORITHM_DENIES]);

  return;
}


unsigned char InstantMovementDisparity(unsigned short *left_depth,unsigned short *right_depth)
{

 return 1;
}



unsigned char DisparityMapNormalize(unsigned short *left_depth,unsigned short *right_depth)
{
	// ENA GRIGORO CHECK POU AN ENA SIMEIO EXEI POLY DISPARITY SE SXESI ME TA GEITONIKA
	// TO OMOGENOPOIEI
/*
  unsigned int memplace,y,xa=0,ya=0,xb=0,yb=0,memsize=0; //a simainei aristeri cam , b deksia
  unsigned int x_start_left=15,avg_x;
  xa=x_start_left;
  memsize=max_res_x*max_res_y;
  while (xa<max_res_x)
	   {
		 memplace=xa;
		 ya=0;
		 avg_x=0;
         while (memplace<memsize)
		 {
            memplace+=max_res_x;
			++ya;
		 }
		++xa;
	   }
  */
  return 1;
}





inline unsigned short FindNeighborDepth(
                                               unsigned short * full_depth_map,
                                               unsigned char * rgb_image,
                                               unsigned int search_area_stop,
                                               unsigned char r,unsigned char g,unsigned char b,
                                               unsigned int dpth_ptr,unsigned int img_ptr,
                                               unsigned int x,unsigned int y,unsigned int depth
                                       )
{
  // Heuristic , finds neighborhood depth..

  unsigned int SHIFT_3_BYTE = metrics[RESOLUTION_X_3_BYTE];
  unsigned int SHIFT_1_BYTE = metrics[RESOLUTION_X];

  unsigned int stop_search_up=0,stop_search_down=0;
  unsigned int dpth_ptr_up=dpth_ptr , dpth_ptr_down=dpth_ptr , y_up=y , y_down=y , img_ptr_up=img_ptr , img_ptr_down=img_ptr;
  unsigned int search_area=0;
  unsigned int THRESHOLD=20;
  while (search_area<search_area_stop)
  {
    if ( stop_search_up==0 )
    {
     if (y_up<=1) stop_search_up=1; else
     {
      dpth_ptr_up-=SHIFT_1_BYTE;
      img_ptr_up-=SHIFT_3_BYTE;
      --y_up;

      if ( (precalc_sub[r][rgb_image[img_ptr_up]]>THRESHOLD) || (precalc_sub[g][rgb_image[img_ptr_up+1]]>THRESHOLD) || (precalc_sub[b][rgb_image[img_ptr_up+2]]>THRESHOLD) ) stop_search_up=1; else
      {
        if (full_depth_map[dpth_ptr_up]!=0) return full_depth_map[dpth_ptr_up];
      }
     }
    }

    if ( stop_search_down==0 )
    {
     if (y_down>=metrics[RESOLUTION_Y]) stop_search_down=1; else
     {
      dpth_ptr_down+=SHIFT_1_BYTE;
      img_ptr_down+=SHIFT_3_BYTE;
      ++y_down;
      if ( (precalc_sub[r][rgb_image[img_ptr_down]]>THRESHOLD) || (precalc_sub[g][rgb_image[img_ptr_down+1]]>THRESHOLD) || (precalc_sub[b][rgb_image[img_ptr_down+2]]>THRESHOLD) ) stop_search_down=1; else
      {
        if (full_depth_map[dpth_ptr_down]!=0) return full_depth_map[dpth_ptr_down];
      }
     }
    }

    if ((stop_search_down!=0)&&(stop_search_up!=0)) return 0;

    ++search_area;
  }
  return 0;
}


void EnhanceDepthMapWithEdges(unsigned char * rgb_image,unsigned short *full_depth_map,unsigned char * edges_map,unsigned int image_x,unsigned int image_y)
{
  // Heuristic , fills depth map holes with neighborhood edge depth..

  fprintf(stderr,"Todo : FindNeighborDepth is not complete , fix it ! \n");
  memcpy(l_video_register[GENERAL_LARGE_1].pixels,full_depth_map,metrics[RESOLUTION_MEMORY_LIMIT_3BYTE]);
  unsigned int SHIFT_1_BYTE = metrics[RESOLUTION_X];

  unsigned int SHIFT_1_BYTE2 = SHIFT_1_BYTE*2;
  unsigned int SHIFT_1_BYTE3 = SHIFT_1_BYTE*3;
  unsigned int SHIFT_1_BYTE4 = SHIFT_1_BYTE*4;
  unsigned int SHIFT_1_BYTE5 = SHIFT_1_BYTE*5;
  //unsigned int SHIFT_1_BYTE6 = SHIFT_1_BYTE*6;

  unsigned int ptr=0,ptr3bit=0;//,dpth_lim=image_x*image_y;
  unsigned int xp=0,yp=0;
  unsigned int left_scan=0,right_scan=0,up_scan=0,down_scan=0;
  char  request_matchup=0;

 for (yp=0; yp<image_y; yp++)
  {
    for (xp=0; xp<image_x; xp++)
    {
     if (edges_map[ptr3bit]==0)
     {
       left_scan=0; right_scan=0; up_scan=0; down_scan=0;

      // KANOUME ENA SCAN PANW KATW ARISTERA K DEKSIA GIA NA VROUME MIA KONTINI EDGE
      // VRISKONTAS MIA KONTINI EDGE ELPIZOUME OTI THA EXEI SWSTO DEPTH VALUE

       if (xp <image_x-10 )
      {
       if  ( full_depth_map[ptr+1]>0 )  { ++right_scan; }
       if  ( full_depth_map[ptr+2]>0 )  { ++right_scan; }
       if  ( full_depth_map[ptr+3]>0 )  { ++right_scan; }
       if  ( full_depth_map[ptr+4]>0 )  { ++right_scan; }
       if  ( full_depth_map[ptr+5]>0 )  { ++right_scan; }
      }

      if (xp >10 )
      {
       if  ( full_depth_map[ptr-1]>0 )  { ++left_scan; }
       if  ( full_depth_map[ptr-2]>0 )  { ++left_scan; }
       if  ( full_depth_map[ptr-3]>0 )  { ++left_scan; }
       if  ( full_depth_map[ptr-4]>0 )  { ++left_scan; }
       if  ( full_depth_map[ptr-5]>0 )  { ++left_scan; }
      }

     if (yp <image_y-10 )
      {
       if  ( full_depth_map[ptr+SHIFT_1_BYTE]>0 )  { ++down_scan; }
       if  ( full_depth_map[ptr+SHIFT_1_BYTE2]>0 )  { ++down_scan; }
       if  ( full_depth_map[ptr+SHIFT_1_BYTE3]>0 )  { ++down_scan; }
       if  ( full_depth_map[ptr+SHIFT_1_BYTE4]>0 )  { ++down_scan; }
       if  ( full_depth_map[ptr+SHIFT_1_BYTE5]>0 )  { ++down_scan; }
      }

      if (yp >10 )
      {
       if  ( full_depth_map[ptr-SHIFT_1_BYTE]>0 )  { ++up_scan; }
       if  ( full_depth_map[ptr-SHIFT_1_BYTE2]>0 )  { ++up_scan; }
       if  ( full_depth_map[ptr-SHIFT_1_BYTE3]>0 )  { ++up_scan; }
       if  ( full_depth_map[ptr-SHIFT_1_BYTE4]>0 )  { ++up_scan; }
       if  ( full_depth_map[ptr-SHIFT_1_BYTE5]>0 )  { ++up_scan; }
      }
         // EXOUME PROSMETRISEI POSES EDGES YPARXOUN DEKSIA ARISTER PANW K KATW
         // TWRA AN EIMASTE ANAMESA SE EDGES , PAROTI THA VRISKOMASTE SE KENO SPOT
         // WSTOSO EXOUME KALI PITHANOTITA NA PAROUME KAPOIA GEITONIKI TIMI KAI NA OMOGENOPOIITHEI TO MAP

         request_matchup=0;
         // AN VRISKOMASTE ANAMESA SE EDGES TOTE SE PERIPTWSI POU EXOUME KAPOIO DEPTH VALUE TO DIATIROUME
         // ALLIWS AN DEN EXOUME PSAXNOUME GYRW GYRW NA VROUME ENA
         if ( (up_scan>=1)&&(down_scan>=5) )  { if (full_depth_map[ptr]==0) { request_matchup=1; } } else
         if ( (up_scan>=5)&&(down_scan>=1) )  { if (full_depth_map[ptr]==0) { request_matchup=1; } } else
         if ( (left_scan>=1)&&(right_scan>=5) )  { if (full_depth_map[ptr]==0) { request_matchup=1; } } else
         if ( (left_scan>=5)&&(right_scan>=1) )  { if (full_depth_map[ptr]==0) { request_matchup=1; } } else
         {
           // AMA DEN EIMASTE ANAMESA SE KAPOIA EDGES APLA SVINETAI TO SIMEIO
           // YPARXEI MEGALI PITHANOTITA NA MIN EINAI SWSTO!
           //if ( PixelIsBright(rgb_image,ptr3bit) == 0 )
           if ( PixelBelongsWithSurroundingPixels(rgb_image,xp,yp,ptr3bit) == 0 )
           { //UNRELATED PIXEL , REMOVE IT !
             l_video_register[GENERAL_LARGE_1].pixels[ptr]=0;
           }
         }

         if (request_matchup!=0 )
          {

          }
       } else
       {  // GRAPHICAL DEBUGING
         // l_video_register[GENERAL_LARGE_1].pixels[ptr]= edges_map[ptr3bit];
       }

       ++ptr;
      ptr3bit+=3;

    }
  }

  memcpy(full_depth_map,l_video_register[GENERAL_LARGE_1].pixels,metrics[RESOLUTION_MEMORY_LIMIT_3BYTE]);
  return;
}


unsigned int EnhanceDepthMapFillHoles(unsigned char * rgb_image,unsigned short *full_depth_map,unsigned int image_x,unsigned int image_y)
{
  // Heuristic , fills depth map holes with neighborhood depth..

  fprintf(stderr,"Todo : EnhanceDepthMapFillHoles is not complete , fix it ! \n");
  memcpy(l_video_register[GENERAL_LARGE_1].pixels,full_depth_map,metrics[RESOLUTION_MEMORY_LIMIT_3BYTE]);
 // unsigned int SHIFT_1_BYTE = metrics[RESOLUTION_X];

  unsigned int ptr=0,ptr3bit=0;
  unsigned int xp=0,yp=0,fill_counter=0;
  char r=0 , g=0 , b=0 ;
 for (yp=0; yp<image_y; yp++)
  {
    for (xp=0; xp<image_x; xp++)
    {

      if (full_depth_map[ptr]==0)
      {
          r=rgb_image[ptr3bit]; g=rgb_image[ptr3bit+1]; b=rgb_image[ptr3bit+2];
          l_video_register[GENERAL_LARGE_1].pixels[ptr] = FindNeighborDepth(full_depth_map,rgb_image,25,r,g,b,ptr,ptr3bit,xp,yp,0);
          ++fill_counter;
      }

      ++ptr;
      ptr3bit+=3;
    }
  }
  memcpy(full_depth_map,l_video_register[GENERAL_LARGE_1].pixels,metrics[RESOLUTION_MEMORY_LIMIT_3BYTE]);
  fprintf(stderr,"%u pixels filled \n",fill_counter);
  return fill_counter;
}


int DepthMapToVideo(unsigned int depth_reg,unsigned int vid_reg)
{
  // Convert from the Large ( unsigned short ) to 3 bytes per pixel storage ( r,g,b ) needed for outputting video
  unsigned short *full_depth_map=l_video_register[depth_reg].pixels;
  unsigned char *vid_depth_map=video_register[vid_reg].pixels;
  unsigned int image_x=video_register[vid_reg].size_x;
  unsigned int image_y=video_register[vid_reg].size_y;


  register BYTE *px;
  register BYTE *r;
  register BYTE *g;
  register BYTE *b;
  unsigned char val;
  px = (BYTE *)  vid_depth_map;

  unsigned int ptr,ptr_3byte=0,dpth_lim=image_x*image_y;
  for ( ptr = 0; ptr < dpth_lim; ptr++)
   {
       r = px++;
       g = px++;
       b = px++;
       if ( full_depth_map[ptr] > 255 ) { val = 255; } else
       //val = full_depth_map[ptr] / 65535;
       val = ( unsigned char ) full_depth_map[ptr];
       *r= val;
       *g= val;
       *b= val;
       ptr_3byte+=3;
   }

   return 1;
}
