#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"


__global__ void calculate_ed(const float * const Subject,const float * const Query, const long int M, const int N,float * gposbitmap,float bsf) 
{
	const int thid = blockDim.x*blockIdx.x + threadIdx.x;
	float dist=0;
	long int poss;
	float currentbsfdist=bsf;
	for (int i = thid; i < M; i += gridDim.x*blockDim.x)
	{	
		dist=0;
		poss=i*N;
		for(int j =0;j<N;j++)
		{
			dist += (Subject[poss+j]-Query[j])*(Subject[poss+j]-Query[j]);
			if(dist>currentbsfdist)
			{
				break;
			}
		}
		gposbitmap[i]=dist;
	}
}

__global__ void collectbsf(float *d_data, int N)
{
	int tid = threadIdx.x;
	int index = blockIdx.x*blockDim.x + threadIdx.x;
	float *data = d_data + blockIdx.x*blockDim.x;
	if (index >= N) return;
	for (int strize = 1; strize < blockDim.x; strize *= 2)
	{
		int idx = tid*strize * 2;
		if (idx < blockDim.x)
		{
			if(data[idx]> data[idx+strize])
			data[idx]= data[idx+strize];
		}
		__syncthreads();
	}
}



int main(int argc, char **argv)
{
// hello from cpu

    cudaSetDevice(0); 
	float *gposbitmap = NULL;
	float *posbitmap = NULL;
	float *qts = NULL,*gqts = NULL;
	float *dataset=NULL,*gdataset=NULL;

//helloFromGPU <<<1, 10>>>();
	struct timeval total_time_start;
	struct timeval current_time;
	double total_time;
	double tS;
	double tE;

	FILE * dfile;
	dfile = fopen (argv[1],"rb");
	FILE * qfile;
	qfile = fopen (argv[2],"rb");

	cudaEvent_t start, stop;
	float time;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	unsigned long datasize = atoi(argv[3]);
	int queriessize = atoi(argv[4]);
	int timeseriessize = atoi(argv[5]);
	unsigned long chunksize=atoi(argv[6]);
	int i;

	//i//nt breakpointsize=sizeof(sax_breakpoints);

	cudaMallocHost(&qts, sizeof(float)*timeseriessize);
	cudaMallocHost(&dataset, sizeof(float)*datasize*timeseriessize); 
	cudaMallocHost(&posbitmap, sizeof(float)*datasize); 
	cudaMalloc(&gposbitmap, sizeof(float)*datasize); 
	cudaMalloc(&gdataset, sizeof(float)*chunksize*timeseriessize); 
	cudaMalloc(&gqts, sizeof(float)*timeseriessize); 
	fread(dataset, sizeof(float),timeseriessize*datasize,dfile);
	gettimeofday(&total_time_start, NULL);   
	cudaEventRecord(start, 0);
	printf("finished to read data\n");
	for(i=0; i<queriessize;i++)
	{
		float BSF=2000000.0f;
		fread(qts, sizeof(float),timeseriessize,qfile);
		cudaMemcpy(gqts, qts, sizeof(float)*timeseriessize,cudaMemcpyHostToDevice);

		

		unsigned long int currentchunksize;
		for(unsigned long j=0;j<datasize;j+=chunksize)
		{
			if(j+chunksize<=datasize)
			{
				currentchunksize=chunksize;
			}
			else
			{
				currentchunksize=j-datasize;
			}
			int streamnumber=10;
			cudaStream_t streams[streamnumber];
			for(int k=0;k<streamnumber;k++)
			{
				cudaStreamCreate(&streams[k]);
			}
			for( int k=0;k<streamnumber;k++)
			{
				cudaMemcpyAsync(&gdataset[k*timeseriessize*currentchunksize/streamnumber], &dataset[j*timeseriessize+k*timeseriessize*currentchunksize/streamnumber], sizeof(float)*timeseriessize*currentchunksize/streamnumber,cudaMemcpyHostToDevice,streams[i]);
				calculate_ed<<<500,200,10,streams[i]>>> (&gdataset[k*timeseriessize*currentchunksize/streamnumber], gqts, currentchunksize/streamnumber, timeseriessize,&gposbitmap[j+k*currentchunksize/streamnumber],BSF); 
				cudaMemcpyAsync(&posbitmap[j+k*currentchunksize/streamnumber], &gposbitmap[j+k*currentchunksize/streamnumber], sizeof(float)*currentchunksize/streamnumber,cudaMemcpyDeviceToHost,streams[i]);
			}
			cudaDeviceSynchronize();
		}
		for(unsigned long j=0;j<datasize;j++)
		{
			if(posbitmap[j]<BSF)
			BSF=posbitmap[j];
		}
		printf("the distance is %f \n",BSF);
	}
	cudaError_t aaaa= cudaDeviceSynchronize();
	cudaDeviceSynchronize();
	gettimeofday(&current_time, NULL);
    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
            
	tS = total_time_start.tv_sec*1000000 + (total_time_start.tv_usec); 
	tE = current_time.tv_sec*1000000  + (current_time.tv_usec); 
	total_time = (tE - tS); 
	//printf("The distance is %ld\n",sizeof(float));
	printf("the time is \t%f\n",total_time);
	cudaDeviceReset();

	return 0;
}
