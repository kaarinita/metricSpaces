#include <float.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <cuda.h>
#include "cuda_runtime.h"
#include "singlib.h"



__global__ void calculate_lbd(const sax_type * const saxarray,const float * const paa, const long int M, const int N,bool * positionarray,const float BSF,float segmentsize) 
{
	const int thid = blockDim.x*blockIdx.x + threadIdx.x;
	float distance = 0;
	float lbsf=BSF/segmentsize;

	int i=0;
        		float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        		float breakpoint_upper = 0; // <-- - || -

	
	for (int j = thid; j < M; j += gridDim.x*blockDim.x)
	{
		distance=0;
		for (i=0; i<N; i++) {
                	if(distance<lbsf)
		{
        	
        		sax_type v = saxarray[j*N+i];

        		sax_type region_lower = v ;//shift operation 
        		sax_type region_upper = (~((int)MAXFLOAT) | region_lower);



        	
        		if (region_lower == 0)
			{
	            		breakpoint_lower = -2000000;
				float breaku=((float)region_lower-127.0f)/128.0f;
            			breakpoint_upper = breaku*(1.1362582192*breaku*breaku+0.99800);//sax_breakpoints[region_upper+1];//search in a list(why?)
				if(breakpoint_upper < paa[i])
				{
            				distance += (breakpoint_upper - paa[i])*(breakpoint_upper - paa[i]);
        			}
			}
        		else if (region_upper == 256 - 1) 
			{
            			breakpoint_upper = +2000000;
				float breakx=((float)region_lower-128.0f)/128.0f;
           			breakpoint_lower = breakx*(breakx*breakx*1.1362582192+0.99800);//sax_breakpoints[region_lower];
				if (breakpoint_lower > paa[i]) 
				{
            				distance += (breakpoint_lower - paa[i])*(breakpoint_lower - paa[i]);
        			}
        		}
        		else 
			{
				float breakx=((float)region_lower-128.0f)/128.0f;
           			breakpoint_lower = breakx*(breakx*breakx*1.1362582192+0.99800);//sax_breakpoints[region_lower];
				if (breakpoint_lower > paa[i]) 
				{
            				distance += (breakpoint_lower - paa[i])*(breakpoint_lower - paa[i]);
        			}
				else
				{
					float breaku=((float)region_lower-127.0f)/128.0f;
            				breakpoint_upper = breaku*(1.1362582192*breaku*breaku+0.99800);//sax_breakpoints[region_upper+1];//search in a list(why?)
					if(breakpoint_upper < paa[i])
					{
            					distance += (breakpoint_upper - paa[i])*(breakpoint_upper - paa[i]);
        				}
        			} 

        		}
		}
						        		
    	}

		if(distance<lbsf)
		{positionarray[j]=true;}
		else
		{positionarray[j]=false;}
	}
}



__global__ void calculate_lbdfloat(const sax_type * const saxarray,const float * const paa, const long int M, const int N,float * positionarray,const float BSF, float segmentsize) 
{
	const int thid = blockDim.x*blockIdx.x + threadIdx.x;
	float distance = 0;
	

	int i=0;
        		float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        		float breakpoint_upper = 0; // <-- - || -


	for (int j = thid; j < M; j += gridDim.x*blockDim.x)
	{
		distance=0;
		for (i=0; i<N; i++) 
		{
			if(segmentsize*distance<BSF)
			{
				sax_type v = saxarray[j*N+i];
				sax_type region_lower = v ;//shift operation 
				sax_type region_upper = (~((int)MAXFLOAT) | region_lower);
				if (region_lower == 0)
				{
					breakpoint_lower = -2000000;
					float breaku=((float)region_lower-127.0f)/128.0f;
					breakpoint_upper = breaku*(1.1362582192*breaku*breaku+0.99800);//sax_breakpoints[region_upper+1];//search in a list(why?)
					if(breakpoint_upper < paa[i])
					{
						distance += (breakpoint_upper - paa[i])*(breakpoint_upper - paa[i]);
					}
				}
				else if (region_upper == 256 - 1)
				{
					breakpoint_upper = +2000000;
					float breakx=((float)region_lower-128.0f)/128.0f;
					breakpoint_lower = breakx*(breakx*breakx*1.1362582192+0.99800);//sax_breakpoints[region_lower];
					if (breakpoint_lower > paa[i]) 
					{
						distance += (breakpoint_lower - paa[i])*(breakpoint_lower - paa[i]);
					}
				}
				else
				{
					float breakx=((float)region_lower-128.0f)/128.0f;
					breakpoint_lower = breakx*(breakx*breakx*1.1362582192+0.99800);//sax_breakpoints[region_lower];
					if (breakpoint_lower > paa[i]) 
					{
						distance += (breakpoint_lower - paa[i])*(breakpoint_lower - paa[i]);
					}
					else
					{
						float breaku=((float)region_lower-127.0f)/128.0f;
						breakpoint_upper = breaku*(1.1362582192*breaku*breaku+0.99800);//sax_breakpoints[region_upper+1];//search in a list(why?)
						if(breakpoint_upper < paa[i])
						{
							distance += (breakpoint_upper - paa[i])*(breakpoint_upper - paa[i]);
						}
					}
				}
			}
		}
		positionarray[j]=segmentsize*distance;
	}
}









extern "C" float* initialGPU(float *qts, float *gqts, sax_type *saxarray, sax_type *gsaxarray, float *dictionary, float *gdictionary,unsigned long datasize,float *sax_breakpoints )
{

	cudaSetDevice(0); 
	cudaMalloc(&gqts, sizeof(float)*16); 
	return gqts;
}
extern "C" float* initialgqts(float *gqts)
{
		cudaMalloc(&gqts, sizeof(float)*16); 
		return gqts;
}
extern "C" void GPUsyn()
{
cudaDeviceSynchronize();
}




extern "C" float* initialgdictionary(float *gdictionary)
{
		cudaMalloc(&gdictionary, sizeof(float)*257);

		return gdictionary;
}
extern "C" bool* initialgposbitmap(bool *gposbitmap,unsigned long datasize)
{
	cudaMalloc(&gposbitmap, sizeof(bool)*datasize); 
		return gposbitmap;
}
extern "C" bool* initialposbitmap(bool *posbitmap,unsigned long datasize)
{
	cudaMallocHost(&posbitmap, sizeof(bool)*datasize); 
		return posbitmap;
}
extern "C" float* initialgposbitmapfloat(float *gposbitmap,unsigned long datasize)
{
	cudaMalloc(&gposbitmap, sizeof(float)*datasize); 
		return gposbitmap;
}
extern "C" float* initialposbitmapfloat(float *posbitmap,unsigned long datasize)
{
cudaMallocHost(&posbitmap, sizeof(float)*datasize); 
		return posbitmap;
}

extern "C" sax_type* initialgsaxarray(sax_type *gsaxarray,unsigned long datasize)
{
	cudaMalloc(&gsaxarray, sizeof(sax_type)*datasize*16); 

		return gsaxarray;
}
extern "C" sax_type* initialsaxarray(sax_type *saxarray,unsigned long datasize)
{
	cudaMallocHost(&saxarray, sizeof(sax_type)*datasize*16); 

		return saxarray;
}
extern "C" void initialdevice()
{
	cudaSetDevice(0);
}
extern "C" void gpumemcpy(sax_type *gsaxarray,sax_type *saxarray,unsigned long datasize)
{	

	cudaMemcpy(gsaxarray, saxarray,sizeof(sax_type)*datasize*16,cudaMemcpyHostToDevice);
}
extern "C" void gpusaxgridmemcpy(sax_type *gsaxarray,sax_type *saxarray,unsigned long datasize)
{	
	cudaMemcpy(gsaxarray, saxarray,sizeof(sax_type)*datasize*16,cudaMemcpyHostToDevice);
}

extern "C" void gpudictionarymemcpy(float *gdictionary,float *sax_breakpoints)
{	
int offset = ((256 - 1) * (256 - 2)) / 2;
	cudaMemcpy(gdictionary, &sax_breakpoints[offset-1], sizeof(float)*257,cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();
}





extern "C" void GPUfree(void *devicememorypointer)
{
	cudaFree(devicememorypointer);
}






extern "C" void LBDfloatstreamGPU(sax_type *saxarray, float *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,float * gposbitmap,int segmentnumber,float segmentsize)
{
	int streamnumber=10;
	cudaMemcpy(gqts, qts,sizeof(float)*segmentnumber,cudaMemcpyHostToDevice);
	cudaStream_t streams[streamnumber];
	for(int i=0;i<streamnumber;i++)
	{
		cudaStreamCreate(&streams[i]);
	}

	for(int i=0;i<streamnumber;i++)
	{
		calculate_lbdfloat<<<200,500,10,streams[i]>>> (saxarray+i*datasize*segmentnumber/streamnumber,gqts, datasize/streamnumber, segmentnumber, gposbitmap+i*datasize/streamnumber,BSF,segmentsize); 
        cudaMemcpyAsync(posbitmap+i*datasize/streamnumber, gposbitmap+i*datasize/streamnumber, sizeof(float)*datasize/streamnumber,cudaMemcpyDeviceToHost,streams[i]);
	}
	cudaDeviceSynchronize();
}




extern "C" void LBDstreamGPU(sax_type *saxarray, bool *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool * gposbitmap,int segmentnumber,float segmentsize)
{
	int streamnumber=20;
	cudaMemcpy(gqts, qts,sizeof(float)*segmentnumber,cudaMemcpyHostToDevice);
	cudaStream_t streams[streamnumber];
	for(int i=0;i<streamnumber;i++)
	{
		cudaStreamCreate(&streams[i]);
	}
	for(int i=0;i<streamnumber;i++)
	{
		calculate_lbd<<<200,500,10,streams[i]>>> (saxarray+i*datasize*segmentnumber/streamnumber,gqts, datasize/streamnumber, segmentnumber,gposbitmap+i*datasize/streamnumber,BSF, segmentsize); 
		cudaMemcpyAsync(posbitmap+i*datasize/streamnumber, gposbitmap+i*datasize/streamnumber, sizeof(bool)*datasize/streamnumber,cudaMemcpyDeviceToHost,streams[i]);
	}
	cudaDeviceSynchronize();
}





















/*
extern "C" void SIMSlowerGPU(sax_type *saxarray, bool *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool * gposbitmap,float *gdictionary)
{

	cudaMemcpy(gqts, qts,sizeof(float)*16,cudaMemcpyHostToDevice);
	calculate_lbd<<<200,200,10>>> (saxarray,gqts, datasize, 16, gposbitmap,BSF,16); 

	cudaMemcpy(posbitmap, gposbitmap, sizeof(bool)*datasize,cudaMemcpyDeviceToHost);
//cudaDeviceSynchronize();


}
extern "C" void SIMSlowerGPUfloat(sax_type *saxarray, float *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,float * gposbitmap,float *gdictionary)
{

	cudaMemcpy(gqts, qts,sizeof(float)*16,cudaMemcpyHostToDevice);
	calculate_lbdfloat<<<200,500,10>>> (saxarray,gqts, datasize, 16,gdictionary, gposbitmap,BSF,16); 
	cudaMemcpy(posbitmap, gposbitmap, sizeof(float)*datasize,cudaMemcpyDeviceToHost);
	cudaDeviceSynchronize();
}

extern "C" void initialGPU_grid2(float *qts, float *gqts, sax_type **saxarray, sax_type **gsaxarray, bool **posbitmap, bool **gposbitmap, float *dictionary, float *gdictionary, unsigned long datasize,unsigned long *gridsize,float *sax_breakpoints )
{
	cudaSetDevice(0); 

	for(int i =0;i<65536;i++)
	{
		cudaMallocHost(&saxarray[i], sizeof(sax_type)*gridsize[i]*16); 
		cudaMalloc(&gposbitmap[i], sizeof(float)*gridsize[i]); 
		cudaMalloc(&gsaxarray[i], sizeof(float)*gridsize[i]*16); 
		cudaMallocHost(&posbitmap[i], sizeof(float)*gridsize[i]); 
		cudaMemcpy(gsaxarray[i], saxarray[i],sizeof(float)*gridsize[i]*16,cudaMemcpyHostToDevice);
	}
	cudaMallocHost(&dictionary, 257*sizeof(float)); 
	cudaMallocHost(&qts, sizeof(float)*256); 
	int offset = ((256 - 1) * (256 - 2)) / 2;
	//memcpy(dictionary,&sax_breakpoints[offset-1],sizeof(float)*257);

	cudaMalloc(&gdictionary, sizeof(float)*257);
	cudaMalloc(&gqts, sizeof(float)*256); 

	cudaMemcpy(gdictionary, &sax_breakpoints[offset-1], sizeof(float)*257,cudaMemcpyHostToDevice);
}
extern "C" void SIMSlowerGPUgridstream(sax_type **saxarray, bool **posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool ** gposbitmap,float *gdictionary,long int *nodemap, unsigned long *gridnumber)
{
	int streamnumber=(int)datasize;
	cudaStream_t streams[streamnumber];
	cudaMemcpy(gqts, qts,sizeof(float)*16,cudaMemcpyHostToDevice);
	for(int i=0;i<streamnumber;i++)
	{
		cudaStreamCreate(&streams[i]);
	}

	for(long int i=0;i<streamnumber;i++)
	{

		calculate_lbd2<<<500,100,10,streams[i]>>> (saxarray[nodemap[i]],gqts, gridnumber[nodemap[i]], 16,gdictionary, gposbitmap[nodemap[i]],BSF); 
        cudaMemcpyAsync(posbitmap[nodemap[i]], gposbitmap[nodemap[i]], sizeof(bool)*gridnumber[nodemap[i]],cudaMemcpyDeviceToHost,streams[i]);
	}
	for(long int i=0;i<datasize;i++)
	{
		//cudaMemcpy(posbitmap[nodemap[i]], gposbitmap[nodemap[i]], sizeof(bool)*gridnumber[nodemap[i]],cudaMemcpyDeviceToHost,streams[i]);
	}
	cudaDeviceSynchronize();
}
extern "C" void SIMSstreamlowerGPU2(sax_type *saxarray, bool *posbitmap,float * qts,float * gqts,float BSF,unsigned long int startnumber,unsigned long int stopnomber,bool * gposbitmap,float *gdictionary)
{

	int streamnumber=10;
	cudaMemcpy(gqts, qts,sizeof(float)*16,cudaMemcpyHostToDevice);
	unsigned long int datasize=100000000;

	int aaa=startnumber/100000000;
	int bbb=stopnomber/100000000+1;

	if(bbb>=10)
	bbb=10;


	cudaStream_t streams[streamnumber];
	for(int i=0;i<streamnumber;i++)
	{
		cudaStreamCreate(&streams[i]);
	}
	for(int i=0;i<streamnumber;i++)
	{
		calculate_lbd3<<<200,500,10,streams[i]>>> (saxarray+i*datasize*16/streamnumber,gqts, datasize/streamnumber, 16,gposbitmap+i*datasize/streamnumber,BSF,16.0); 
       	cudaMemcpyAsync(posbitmap+i*datasize/streamnumber, gposbitmap+i*datasize/streamnumber, sizeof(bool)*datasize/streamnumber,cudaMemcpyDeviceToHost,streams[i]);
	}

	cudaDeviceSynchronize();

}
extern "C" void SIMSlowerGPUgrid(sax_type **saxarray, bool **posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool ** gposbitmap,float *gdictionary,long int *nodemap, unsigned long *gridnumber)
{
	cudaMemcpy(gqts, qts,sizeof(float)*16,cudaMemcpyHostToDevice);
	for(long int i=0;i<datasize;i++)
	{

		calculate_lbd2<<<500,200,10>>> (saxarray[nodemap[i]],gqts, gridnumber[nodemap[i]], 16,gdictionary, gposbitmap[nodemap[i]],BSF); 
	}
	for(long int i=0;i<datasize;i++)
	{
		cudaMemcpy(posbitmap[nodemap[i]], gposbitmap[nodemap[i]], sizeof(bool)*gridnumber[nodemap[i]],cudaMemcpyDeviceToHost);
	}
	cudaDeviceSynchronize();
}

__global__ void calculate_lbd4(const sax_type * const saxarray,const float * const paa, const long int M, const int N,bool * positionarray,const float BSF,const long int offset) 
{
	const int thid = blockDim.x*blockIdx.x + threadIdx.x;
	float distance = 0;
	float lbsf=BSF/16.0;

	int i=0;
        		float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        		float breakpoint_upper = 0; // <-- - || -

	
	for (int j = thid; j < M; j += gridDim.x*blockDim.x)
	{
		distance=0;
		for (i=0; i<N; i++) {
                	if(distance<lbsf)
		{
        	
        		sax_type v = saxarray[j*N+i];

        		sax_type region_lower = v ;//shift operation 
        		sax_type region_upper = (~((int)MAXFLOAT) | region_lower);



        	
        		if (region_lower == 0)
			{
	            		breakpoint_lower = -2000000;
				float breaku=((float)region_lower-127.0f)/128.0f;
            			breakpoint_upper = breaku*(1.1362582192*breaku*breaku+0.99800);//sax_breakpoints[region_upper+1];//search in a list(why?)
				if(breakpoint_upper < paa[i])
				{
            				distance += (breakpoint_upper - paa[i])*(breakpoint_upper - paa[i]);
        			}
			}
        		else if (region_upper == 256 - 1) 
			{
            			breakpoint_upper = +2000000;
				float breakx=((float)region_lower-128.0f)/128.0f;
           			breakpoint_lower = breakx*(breakx*breakx*1.1362582192+0.99800);//sax_breakpoints[region_lower];
				if (breakpoint_lower > paa[i]) 
				{
            				distance += (breakpoint_lower - paa[i])*(breakpoint_lower - paa[i]);
        			}
        		}
        		else 
			{
				float breakx=((float)region_lower-128.0f)/128.0f;
           			breakpoint_lower = breakx*(breakx*breakx*1.1362582192+0.99800);//sax_breakpoints[region_lower];
				if (breakpoint_lower > paa[i]) 
				{
            				distance += (breakpoint_lower - paa[i])*(breakpoint_lower - paa[i]);
        			}
				else
				{
					float breaku=((float)region_lower-127.0f)/128.0f;
            				breakpoint_upper = breaku*(1.1362582192*breaku*breaku+0.99800);//sax_breakpoints[region_upper+1];//search in a list(why?)
					if(breakpoint_upper < paa[i])
					{
            					distance += (breakpoint_upper - paa[i])*(breakpoint_upper - paa[i]);
        				}
        			} 

        		}
		}
						        		
    	}

		if(distance<lbsf)
		{positionarray[j]=true;}
		else
		{positionarray[j]=false;}
	}
}


__global__ void calculate_lbd2float(const sax_type * const saxarray,const float * const paa, const long int M, const int N,float * const sax_breakpoints,float * positionarray,const float BSF) 
{
	const int thid = blockDim.x*blockIdx.x + threadIdx.x;
	float distance = 0;
	

	int i=0;
        		float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        		float breakpoint_upper = 0; // <-- - || -


	for (int j = thid; j < M; j += gridDim.x*blockDim.x)
	{
		distance=0;
		for (i=0; i<N; i++) 
		{
        	if(16*distance<BSF)
		{
        	
        		sax_type v = saxarray[j*N+i];

        		sax_type region_lower = v ;//shift operation 
        		sax_type region_upper = (~((int)MAXFLOAT) | region_lower);



        	
        		if (region_lower == 0)
			{
	            		breakpoint_lower = -2000000;
        		}
        		else
        		{
				float breakx=((float)region_lower-128.0f)/128.0f;
           			breakpoint_lower = breakx*(breakx*breakx*1.1362582192+0.99800);//sax_breakpoints[region_lower];
        		}




        		if (region_upper == 256 - 1) 
			{
            			breakpoint_upper = +2000000;
        		}
        		else 
			{
			float breaku=((float)region_lower-127.0f)/128.0f;
            			breakpoint_upper = breaku*(1.1362582192*breaku*breaku+0.99800);//sax_breakpoints[region_upper+1];//search in a list(why?)
        		} 

	        			

        		if (breakpoint_lower > paa[i]) 
			{

            			distance += (breakpoint_lower - paa[i])*(breakpoint_lower - paa[i]);
        		}
        		else if(breakpoint_upper < paa[i])
			{
            			distance += (breakpoint_upper - paa[i])*(breakpoint_upper - paa[i]);
        		}
		}
			        				        		
    		}


		positionarray[j]=16*distance;
	}
}

__global__ void calculate_ed2(const float * const Subject, const float * const Query, const long int M, const int N,float * gposbitmap) 
{
	const int thid = blockDim.x*blockIdx.x + threadIdx.x;
	float dist=0;
	long int poss;
	//printf("hello jfeowjfw %d\n",thid);
	for (int i = thid; i < M; i += gridDim.x*blockDim.x)
	{	
		dist=0;
		poss=i*N;
		for(int j =0;j<N;j++)
		{
        		dist += (Subject[poss+j]-Query[j])*(Subject[poss+j]-Query[j]);
				
		}
		gposbitmap[i]=dist;
		//if(dist<BSF)
		//resultmap[i]=dist;
	}		
}
__global__ void calculate_lbdold(const sax_type * const saxarray,const float * const paa, const long int M, const int N,float * const sax_breakpoints,bool * positionarray,const float BSF) 
{
	const int thid = blockDim.x*blockIdx.x + threadIdx.x;
	float distance = 0;
	

	int i=0;
        		float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        		float breakpoint_upper = 0; // <-- - || -


	for (int j = thid; j < M; j += gridDim.x*blockDim.x)
	{
		distance=0;
		for (i=0; i<N; i++) {
        
        	
        		sax_type v = saxarray[j*N+i];

        		sax_type region_lower = v ;//shift operation 
        		sax_type region_upper = (~((int)MAXFLOAT) | region_lower);



        	
        		if (region_lower == 0)
			{
	            		breakpoint_lower = -2000000;
        		}
        		else
        		{
           			breakpoint_lower = sax_breakpoints[region_lower];//(float)(region_lower-128)*(region_lower-128)/16484.0f;//sax_breakpoints[region_lower];
        		}

        		if (region_upper == 256 - 1) 
			{
            			breakpoint_upper = +2000000;
        		}
        		else
        		{
            			breakpoint_upper = sax_breakpoints[region_lower+1];//(float)(region_upper+1-128)*(region_upper+1-128)/16484.0f;//sax_breakpoints[region_upper+1];//search in a list(why?)
        		}

	        			

        		if (breakpoint_lower > paa[i]) 
			{

            			distance += (breakpoint_lower - paa[i])*(breakpoint_lower - paa[i]);
        		}
        		else if(breakpoint_upper < paa[i])
			{
            			distance += (breakpoint_upper - paa[i])*(breakpoint_upper - paa[i]);
        		}

    		}

		if(16*distance<BSF)
		{positionarray[j]=true;}
		else
		{positionarray[j]=false;}
	}
}
__global__ void calculate_lbd2(const sax_type * const saxarray,const float * const paa, const long int M, const int N,float * const sax_breakpoints,bool * positionarray,const float BSF) 
{
	const int thid = blockDim.x*blockIdx.x + threadIdx.x;
	float distance = 0;
	

	int i=0;
        		float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        		float breakpoint_upper = 0; // <-- - || -


	for (int j = thid; j < M; j += gridDim.x*blockDim.x)
	{
		distance=0;
		for (i=0; i<N; i++) {
                	if(16*distance<BSF)
		{
        	
        		sax_type v = saxarray[j*N+i];

        		sax_type region_lower = v ;//shift operation 
        		sax_type region_upper = (~((int)MAXFLOAT) | region_lower);



        	
        		if (region_lower == 0)
			{
	            		breakpoint_lower = -2000000;
        		}
        		else
        		{
				float breakx=((float)region_lower-128.0f)/128.0f;
           			breakpoint_lower = breakx*(breakx*breakx*1.1362582192+0.99800);//sax_breakpoints[region_lower];
        		}

        		if (region_upper == 256 - 1) 
			{
            			breakpoint_upper = +2000000;
        		}
        		else 
			{
			float breaku=((float)region_lower-127.0f)/128.0f;
            			breakpoint_upper = breaku*(1.1362582192*breaku*breaku+0.99800);//sax_breakpoints[region_upper+1];//search in a list(why?)
        		} 

	        			

        		if (breakpoint_lower > paa[i]) 
			{

            			distance += (breakpoint_lower - paa[i])*(breakpoint_lower - paa[i]);
        		}
        		else if(breakpoint_upper < paa[i])
			{
            			distance += (breakpoint_upper - paa[i])*(breakpoint_upper - paa[i]);
        		}
}
						        		
    		}

		if(16*distance<BSF)
		{positionarray[j]=true;}
		else
		{positionarray[j]=false;}
	}
}
extern "C" void SIMSlowerGPUsmall(sax_type *saxarray, bool *posbitmap,float * qts,float * gqts,float BSF,long unsigned datasize,bool * gposbitmap,float *gdictionary)
{

	calculate_lbd<<<20,20,10>>> (saxarray,gqts, datasize, 16,gdictionary, gposbitmap,BSF); 

	cudaMemcpy(posbitmap, gposbitmap, sizeof(bool)*datasize,cudaMemcpyDeviceToHost);

}
extern "C" void copyqts(float * qts,float * gqts)
{
	cudaMemcpy(gqts, qts,sizeof(float)*16,cudaMemcpyHostToDevice);
}
*/