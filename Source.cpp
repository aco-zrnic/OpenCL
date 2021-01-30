#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include <string>


struct Pixel
{
	unsigned char r, g, b;
};


static char* readKernelSource(const char* filename)
{
	char* kernelSource = nullptr;
	long length;
	FILE* f = fopen(filename, "r");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		kernelSource = (char*)calloc(length, sizeof(char));
		if (kernelSource)
			fread(kernelSource, 1, length, f);
		fclose(f);
	}
	return kernelSource;
}

static void readImage(const char* filename, Pixel* &array, int& width, int& height)
{
	FILE* fp = fopen(filename, "rb"); /* b - binary mode */
	if (!fscanf(fp, "P6\n%d %d\n255\n", &width, &height)) {
		throw "error";
	}
	size_t bytes = width * height * sizeof(Pixel); // Size, in bytes, of each vector
	Pixel* image = (Pixel*)malloc(bytes);
	fread(image, sizeof(Pixel), (size_t)width * (size_t)height, fp);
	fclose(fp);
	array = image;
}

static void writeImage(const char* filename,const Pixel* array, const int width, const int height)
{
	FILE* fp = fopen(filename, "wb"); /* b - binary mode */
	fprintf(fp, "P6\n%d %d\n255\n", width, height);
	fwrite(array, sizeof(Pixel), (size_t)width * (size_t)height, fp);
	fclose(fp);
}

int main()
{
	int width = 256;
	int height = 256;
	Pixel* buffer = nullptr;
	readImage("img0rgb.ppm", buffer, width, height);
	int imageSize = width * height *sizeof(Pixel);
	for (int i = 0; i < 30; i++)
	{
		cl_mem d_a;

		cl_platform_id cpPlatform;        // OpenCL platform
		cl_device_id device_id;           // device ID
		cl_context context;               // context
		cl_command_queue queue;           // command queue
		cl_program program;               // program
		cl_kernel kernel;                 // kernel

		size_t globalSize[2], localSize[2];
		cl_int err;

		// Number of work items in each local work group
		localSize[0] = localSize[1] = 8;

		// Number of total work items - localSize must be devisor
		globalSize[0] = (size_t)ceil(width / (float)localSize[0]) * localSize[0];
		globalSize[1] = (size_t)ceil(height / (float)localSize[1]) * localSize[1];

		// Bind to platform
		err = clGetPlatformIDs(1, &cpPlatform, NULL);

		// Get ID for the device
		err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

		// Create a context  
		context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

		// Create a command queue 
		queue = clCreateCommandQueue(context, device_id, 0, &err);

		char* kernelSource = readKernelSource("GameOfLife.cl");

		// Create the compute program from the source buffer
		program = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, NULL, &err);

		// Build the program executable 
		err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

		if (err)
		{
			// Determine the size of the log
			size_t log_size;
			clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

			// Allocate memory for the log
			char* log = (char*)malloc(log_size);

			// Get the log
			clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

			// Print the log
			printf("%s\n", log);

			free(log);
		}

		// Create the compute kernel in the program we wish to run
		kernel = clCreateKernel(program, "gameOfLife", &err);

		size_t kernelWorkGroupSize = 0;
		clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &kernelWorkGroupSize, nullptr);

		// Create the input and output arrays in device memory for our calculation
		d_a = clCreateBuffer(context, CL_MEM_READ_WRITE, imageSize, NULL, NULL);

		// Write our data set into the input array in device memory
		err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0, imageSize, buffer, 0, NULL, NULL);

		// Set the arguments to our compute kernel
		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
		err |= clSetKernelArg(kernel, 1, sizeof(int), &width);

		// Execute the kernel over the entire range of the data set  
		err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, localSize, 0, NULL, NULL);

		// Wait for the command queue to get serviced before reading back results
		clFinish(queue);

		// Read the results from the device
		clEnqueueReadBuffer(queue, d_a, CL_TRUE, 0, imageSize, buffer, 0, NULL, NULL);

		clFinish(queue);

		// release OpenCL resources
		clReleaseMemObject(d_a);
		clReleaseProgram(program);
		clReleaseKernel(kernel);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);

		//release host memory
		free(kernelSource);

		const std::string outFile = std::string("image") + std::to_string(i + 1) + std::string(".ppm");
		writeImage(outFile.c_str(), buffer, width, height);
	}
	return 0;
}