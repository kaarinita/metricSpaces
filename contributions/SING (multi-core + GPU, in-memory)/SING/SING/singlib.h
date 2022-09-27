#ifndef __B_H_

#define __B_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cuda.h>
#include <cuda_runtime.h>

typedef unsigned char sax_type;

extern "C" {
    void initialdevice();
    void GPUsyn();
    void GPUfree(void *devicememorypointer);

    float* initialGPU(float *qts, float *gqts, sax_type *saxarray, sax_type *gsaxarray, float *dictionary, float *gdictionary,long unsigned datasize, float *sax_breakpoints );
    float* initialgqts(float *gqts);
    sax_type* initialgsaxarray(sax_type *gsaxarray,unsigned long datasize);
    sax_type* initialsaxarray(sax_type *saxarray,unsigned long datasize);
    float* initialgdictionary(float *gdictionary);
    bool* initialgposbitmap(bool *gposbitmap,unsigned long datasize);
    bool* initialposbitmap(bool *posbitmap,unsigned long datasize);
    float* initialgposbitmapfloat(float *gposbitmap,unsigned long datasize);
    float* initialposbitmapfloat(float *posbitmap,unsigned long datasize);

    void gpumemcpy(sax_type *gsaxarray,sax_type *saxarray,unsigned long datasize);
    void gpudictionarymemcpy(float *gdictionary,float *sax_breakpoints);
    void copyqts(float * qts,float * gqts);
    void LBDfloatstreamGPU(sax_type *saxarray, float *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,float * gposbitmap,int segmentnumber,float segmentsize);
    void LBDstreamGPU(sax_type *saxarray, bool *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool * gposbitmap,int segmentnumber,float segmentsize);

/*
    void SIMSlowerGPU(sax_type *saxarray, bool *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool *gposbitmap,float *gdictionary);
    void SIMSlowerGPUfloat(sax_type *saxarray, float *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,float *gposbitmap,float *gdictionary);

    void initialGPU_grid2(float *qts, float *gqts, sax_type **saxarray, sax_type **gsaxarray, bool **posbitmap, bool **gposbitmap, float *dictionary, float *gdictionary, unsigned long datasize,unsigned long *gridsize,float *sax_breakpoints );
    void SIMSlowerGPUgridstream(sax_type **saxarray, bool **posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool ** gposbitmap,float *gdictionary, long int *nodemap, unsigned long *gridnumber);
    void SIMSlowerGPUgrid(sax_type **saxarray, bool **posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool ** gposbitmap,float *gdictionary, long int *nodemap, unsigned long *gridnumber);
    void SIMSstreamlowerGPU2(sax_type *saxarray, bool *posbitmap,float * qts,float * gqts,float BSF,unsigned long int startnumber,unsigned long int stopnomber,bool * gposbitmap,float *gdictionary);
    void gpusaxgridmemcpy(sax_type *gsaxarray,sax_type *saxarray,unsigned long datasize);
    void SIMSlowerGPUsmall(sax_type *saxarray, bool *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool * gposbitmap,float *gdictionary);
    void SIMSlowerGPUfloatdevide(sax_type *saxarray, float *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,float * gposbitmap,float *gdictionary,unsigned long int offsetnum,long int origindatasize);
    void SIMSlowerGPUfloatstreamdevide(sax_type *saxarray, float *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,float * gposbitmap,float *gdictionary,int startn, int stopn,float sigmentsize);
    */
}


#endif
