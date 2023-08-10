// cdc_opencl.
#include <iostream>
#include <stdio.h>

#include "cdc_opencl.h"

using namespace std;

#define _END '\0'
#define OCL_PROGRAM_LOGLEN 10240

/*
* @brief 清理opencl资源
* __clear_opencl_hd => MOCL_ClearCLpacket
*/
bool __opencl_clear_hd(
	cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel,
	cl_mem* mem_objects, size_t size_objects
) {
	bool MEMnullptr = true;
	if (mem_objects) {
		for (int32_t i = 0; i < size_objects; ++i)
			clReleaseMemObject(mem_objects[i]);
	}
	else
		MEMnullptr = false;

	if (commandQueue) clReleaseCommandQueue(commandQueue);
	if (kernel)       clReleaseKernel(kernel);
	if (program)      clReleaseProgram(program);
	if (context)      clReleaseContext(context);

	return MEMnullptr;
}

bool MOCL_FUNC_CLEARPK(mocl_CLpacket packet) {
	bool returnstate = __opencl_clear_hd(packet.Context, packet.CommandQueue, packet.Program, packet.Kernel,
		packet.MemObjects, packet.MemObjects_num);
	return returnstate;
}

void __opencl_errorexit(int32_t exitcode, mocl_CLpacket packet) {
	// free => error exit
	MOCL_FUNC_CLEARPK(packet);
	exit(exitcode);
}

/*
* @brief 创建opencl上下文
*/
cl_context CDC_CORE_OPENCL::OCL_CreateContext(cl_device_id* device) {
	int32_t errorcode = NULL;
	cl_context context = NULL;

	get_device_id(node_platform, node_device, device);
	
	context = clCreateContext(NULL, 1, device, NULL, NULL, &errorcode);
	if (errorcode != CL_SUCCESS) {

		cout << "OPENCL Core: Create context. code:" << errorcode << endl;
		return nullptr;
	}
	return context;
}

/*
* @brief 创建opencl命令队列
*/
cl_command_queue CDC_CORE_OPENCL::OCL_CreateCommandQueue(cl_context context, cl_device_id device) {
	cl_command_queue commandQueue = nullptr;
	// OpenCL 2.0 的用法
	// CommandQueue = clCreateCommandQueue(context, device, 0, NULL);

	commandQueue = clCreateCommandQueueWithProperties(context, device, NULL, NULL);
	if (commandQueue == NULL) {

		cout << "OPENCL Core: Failed create command_queue for device 0." << endl;
		return nullptr;
	}
	return commandQueue;
}

/*
* @brief 读取opencl核函数文件
* Kernel function File: [.cl]
*/
char* CDC_CORE_OPENCL::OCL_ReadKernelSourceFile(const char* filename, size_t length) {
	FILE* readfile = nullptr;
	char*  sourceString = nullptr;
	size_t sourceLenth = NULL;
	size_t readnum = NULL;

	// test.file
	fopen_s(&readfile, filename, "rb");
	if (!readfile) {

		cout << "OPENCL Core: Can't open: " << filename << endl;
		return nullptr;
	}

	// file.end
	fseek(readfile, NULL, SEEK_END);
	sourceLenth = ftell(readfile);

	// file.begin
	fseek(readfile, NULL, SEEK_SET);
	sourceString = new char[sourceLenth + 1];
	sourceString[0] = _END;
	if (sourceString)
		readnum = fread(sourceString, sourceLenth, (_CVT_SIZET)1, readfile);
	if (!readnum) {

		cout << "OPENCL Core: Cant't read source: " << filename << endl;
		return nullptr;
	}

	fclose(readfile);
	if (length)
		length = sourceLenth;
	sourceString[sourceLenth] = _END;

	return sourceString;
}

/*
* @brief 创建opencl程序
* Read kernel function
*/
cl_program CDC_CORE_OPENCL::OCL_CreateProgram(cl_context context, cl_device_id device, const char* filename) {
	cl_program program = nullptr;
	size_t     program_length = NULL;
	int32_t errorcode = NULL;

	char* const source = OCL_ReadKernelSourceFile(filename, program_length);
	program = clCreateProgramWithSource(context, 1, (const char**)&source, NULL, NULL);

	if (!program) {

		cout << "OPENCL Core: Failed to creae CL program from source." << endl;
		return nullptr;
	}

	errorcode = clBuildProgram(program, NULL, NULL, NULL, NULL, NULL);
	if (errorcode != CL_SUCCESS) {

		char* buildLog = new char[OCL_PROGRAM_LOGLEN];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, OCL_PROGRAM_LOGLEN, buildLog, NULL);

		cout << "CLError in kernel: " << buildLog << endl;

		clReleaseProgram(program);
		// free => ret null.
		delete[] buildLog; return nullptr;
	}
	delete[] source;
	return program;
}

/*
* @brief 创建opencl内存对象
* computing.
* clCreateBuffer + clEnqueueWriteBuffer
* [clCreateBuffer]
*/
bool CDC_CORE_OPENCL::OCL_CreateMemoryObjects(cl_context context, size_t MEMOBJ_in, size_t MEMOBJ_out, cl_mem* memObjects) {
	size_t i = NULL;
	bool returnstate = true;

	if (MEMOBJ_in && MEMOBJ_out) {
		// in matrix data memory_object.
		for (i; i < MEMOBJ_in; i++) {
			memObjects[i] = clCreateBuffer(context, CL_MEM_READ_ONLY, Array_size, nullptr, NULL);

			if (!memObjects[i])
				returnstate = false;
		}
		// output memory data.
		for (size_t j = MEMOBJ_in; j < MEMOBJ_in + MEMOBJ_out; j++) {
			memObjects[j] = clCreateBuffer(context, CL_MEM_READ_WRITE, Array_size, nullptr, NULL);

			if (!memObjects[j])
				returnstate = false;
		}
	}
	else
		cout << "OPENCL Core: memory object_io > 0." << endl;

	return returnstate;
}

// [clEnqueueWriteBuffer]
bool CDC_CORE_OPENCL::OCL_WriteMemoryObjects(cl_command_queue command, size_t MEMOBJ_in, cl_mem* memObjects, mocl_Matrix2D in_data) {
	bool returnstate = true;

	if (MEMOBJ_in) {
		if (in_data.data != nullptr) {
			for (size_t i = 0; i < MEMOBJ_in; i++)
				clEnqueueWriteBuffer(command, memObjects[i], CL_TRUE, NULL, Array_size, &in_data.data[i][0], NULL, NULL, NULL);
		}
	}
	else {
		cout << "OPENCL Core: memory object_in > 0." << endl;
		returnstate = false;
	}
	MOCL_TOOL_FREEMATRIX2D(in_data);

	return returnstate;
}

/*
* @brief 设置Kernel参数
* [0,num - 1]
* output_parameters: num - 1 (最右参数开始)
*/
bool CDC_CORE_OPENCL::OCL_SetKernelFunctionParam(cl_kernel kernel, size_t memObj_num, cl_mem* memObj) {
	bool returnstate = true;
	int32_t errorcode = NULL;

	for (size_t i = 0; i < memObj_num; i++) {
		errorcode = clSetKernelArg(kernel, (uint32_t)i, sizeof(cl_mem), &memObj[i]);
		if (errorcode != CL_SUCCESS) {

			cout << "OPENCL Core: Setting kernel arguments. Num:" << int32_t(i) << endl;
			returnstate = false;
		}
	}
	return returnstate;
}