// cdc_opencl. [CDC-OPENCL]
// RCSZ. cpp17, x64.

#ifndef _CDC_OPENCL_H
#define _CDC_OPENCL_H
#define CL_TARGET_OPENCL_VERSION 300
#include <string>
#include <CL/cl.h>

#include "cdc_maths_tool.h"

#define _CVT_SIZET size_t
#define _FLOAT_NUMSIZE(n) n * (size_t)4
#define _FLOAT_SIZENUM(n) n / (size_t)4

#define _ERROR_CONTEXT      0xC0EE00 // 上下文
#define _ERROR_COMMANDQUEUE 0xC0EE01 // 指令队列
#define _ERROR_PROGRAM      0xC0EE02 // CL程序
#define _ERROR_KERNEL       0xC0EE03 // CL内核

#define WORKGROUP_1DFILL 2

#define MCLASS_STATE_CMEMOBJ 0x1E0001 // Create memory objects error
#define MCLASS_STATE_CKERNEL 0x1E0002 // Create kernel error

struct mocl_CLpacket {

	cl_device_id     Device;       // OCL设备
	cl_context       Context;      // OCL上下文
	cl_command_queue CommandQueue; // OCL命令队列
	cl_program       Program;      // OCL程序
	cl_kernel        Kernel;       // OCL核函数

	cl_mem* MemObjects;            // OCL内存对象
	size_t  MemObjects_num;
};

bool __opencl_clear_hd(
	cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel,
	cl_mem* mem_objects, size_t size_objects
);
void __opencl_errorexit(int32_t exitcode, mocl_CLpacket packet);

bool MOCL_GETINFO_SYSTEM(std::string& get_system_str);
bool MOCL_FUNC_CLEARPK(mocl_CLpacket packet);

// Device.
class OPENCL_TYPE_DEVICE {
protected:
	cl_platform_id* platformsptr = nullptr;
	uint32_t platforms_num = NULL; // 平台数量.

	// get platform number => platforms_num
	void get_device_id(uint32_t platform, uint32_t device, cl_device_id* id);

public:
	OPENCL_TYPE_DEVICE() {};
	~OPENCL_TYPE_DEVICE() {
		
		if (platformsptr != nullptr)
			delete[] platformsptr;
	};

	uint32_t GetPlatformNumber();
	uint32_t GetDevicemNumber(uint32_t platform);
};

// cdc_opencl_core [OpenCL API]
// version 1.5.5
class CDC_CORE_OPENCL :public OPENCL_TYPE_DEVICE {
protected:
	size_t Array_size = NULL; // length * sizeof(float), 1d = x.length, 2d = x.length * y.length

	size_t out_matrix_xlength = NULL; // out_matrix x [1D]
	size_t out_matrix_ylength = NULL; // out_matrix y [2D]

	uint32_t node_platform = 1;
	uint32_t node_device   = 1;

	char*            OCL_ReadKernelSourceFile(const char* filename, size_t length);
	cl_context       OCL_CreateContext(cl_device_id* device);
	cl_command_queue OCL_CreateCommandQueue(cl_context context, cl_device_id device);
	cl_program       OCL_CreateProgram(cl_context context, cl_device_id device, const char* filename);

	// mem_objects_num = MEMOBJ_in + MEMOBJ_out, ( clCreateBuufer + clEnqueueWriteBuffer )
	bool OCL_CreateMemoryObjects(cl_context context, size_t MEMOBJ_in, size_t MEMOBJ_out, cl_mem* memObjects);
	bool OCL_WriteMemoryObjects(cl_command_queue command, size_t MEMOBJ_in, cl_mem* memObjects, mocl_Matrix2D in_data);

	bool OCL_SetKernelFunctionParam(cl_kernel kernel, size_t memObj_num, cl_mem* memObj);
};

// load opencl [2D] [kernel function file]
// version 1.1.5
class OPENCL_CALC_MATRIX :public CDC_CORE_OPENCL {
protected:
	static size_t WORK_GROUP_SIZE[2];
	COUNT_TIME calc_totaltime; // 计时.

	size_t memory_object_in  = NULL;
	size_t memory_object_out = NULL;
	mocl_CLpacket opencl_packet = {};

public:
	OPENCL_CALC_MATRIX() {};
	~OPENCL_CALC_MATRIX() {

		MOCL_FUNC_CLEARPK(opencl_packet);
	};

	// writebuffer => compute => readbuffer 用时[ms] 精度[us]
	double calc_total_time = NULL;

    uint32_t statecode = NULL;

	// Context => Command_queue => Program => Kernel
	void CLAC_init_opencl(const char* CLfile_path, const char* CLfunction);

	// set gpu work.group.size Mat1D => x, Mat2D => [x,y]
	void CALC_set_workgroup(size_t x, size_t y);

	// => Create_Memory_objects 
	// 2D[3D]: data_length = y.len * z.len
	void CALC_set_computeIO(size_t in_matrix_num, size_t out_matrix_num, size_t data_length);
	void CALC_type_device(uint32_t NP, uint32_t ND);

	// write => GPU mem_data
	bool CALC_write_matrix(mocl_Matrix3D& inMatrix);
	// GPU mem_data => main
	mocl_Matrix3D CALC_read_result();
};

#endif