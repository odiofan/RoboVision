#include <stdio.h>
#include <stdlib.h>
#include "../MasterpathPlanning.h"


int main()
{
    printf("MasterpathPlanning tester commencing!!\n");
    struct Map * testmap = CreateMap(1000,1000,100);

    unsigned int x=0,y=0;

    printf("State Cleaning Test!!\n");
    for ( y=0; y<1000; y++)
     {
       for ( x=0; x<1000; x++)
       {
           if ( ObstacleExists(testmap,x,y) ) { fprintf(stderr,"Bug , world should be automatically cleared when created!\n"); }
       }
     }


    printf("Obstacle Setting Test part 1/2!!\n");
    for ( y=0; y<1000; y++)
     {
       for ( x=0; x<1000; x++)
       {
           if ( !SetObstacle(testmap,x,y,2) ) { fprintf(stderr,"Bug , could not set obstacle !\n"); }
       }
     }

     printf("Obstacle Setting Test part 2/2!!\n");
    for ( y=0; y<1000; y++)
     {
       for ( x=0; x<1000; x++)
       {
           if ( !ObstacleExists(testmap,x,y) ) { fprintf(stderr,"Bug , world should be completely filled with obstacles !\n"); }
       }
     }



    DeleteMap(testmap);
    return 0;
}