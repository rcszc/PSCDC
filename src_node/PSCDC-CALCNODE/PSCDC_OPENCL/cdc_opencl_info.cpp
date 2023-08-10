// cdc_opencl_info.
#include <iostream>
#include <sstream>

#include "cdc_opencl.h"

using namespace std;
// version 1.1.0

#define _INFOPRINT_HEAD ""

#define CLCHAR_LENGTH_PLATFORM 30
#define CLCHAR_LENGTH_DEVICE   50

/*
* 打印系统平台设备信息.
* clGetPlatformIDs, clGetDeviceIDs
*/
bool MOCL_GETINFO_SYSTEM(string& get_system_str) {
	ostringstream pdinfo_string = {};
	bool return_state = true;

	cl_platform_id* platforms = nullptr;
	cl_uint         platforms_number = NULL;

	// clGetPlatfromIDs get.platforms_number.
	cl_int status = clGetPlatformIDs(NULL, nullptr, &platforms_number);
	if (status != CL_SUCCESS) {

		cout << "getinfo error: getting platforms failed.";
		return_state = false;
	}

	pdinfo_string << "OPENCL FIND " << platforms_number << " PLATFORM(S)\n" << endl;

	platforms = new cl_platform_id[platforms_number];
	status = clGetPlatformIDs(platforms_number, platforms, nullptr);

	cl_uint device_number = NULL;

	// 遍历所有平台设备.
	for (uint32_t i = 0; i < platforms_number; i++) {
		// platform.
		cl_char* param = new cl_char[CLCHAR_LENGTH_PLATFORM];

		pdinfo_string << "PLATFORM " << i << " INFOMATION: " << endl;
		clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, CLCHAR_LENGTH_PLATFORM, param, nullptr);
		pdinfo_string << _INFOPRINT_HEAD << "> p.name - " << param << endl;

		clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, CLCHAR_LENGTH_PLATFORM, param, nullptr);
		pdinfo_string << _INFOPRINT_HEAD << "> vendor - " << param << endl;

		clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, CLCHAR_LENGTH_PLATFORM, param, nullptr);
		pdinfo_string << _INFOPRINT_HEAD << "> version - " << param << endl;

		clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, CLCHAR_LENGTH_PLATFORM, param, nullptr);
		pdinfo_string << _INFOPRINT_HEAD << "> profile - " << param << endl;

		pdinfo_string << "\n************************************************\n" << endl;
		delete[] param;

		// get device.
		cl_device_id* devices = nullptr;
		cl_uint       devices_numebr = NULL;

		status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, NULL, nullptr, &devices_numebr);
		devices = new cl_device_id[devices_numebr];

		pdinfo_string << "PLATFORM " << i << " HAS " << devices_numebr << " DEVICE(S): " << endl;
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, devices_numebr, devices, nullptr);

		// 获取平台下设备.
		for (uint32_t j = 0; j < devices_numebr; j++) {
			// device.
			cl_char* device_param = new cl_char[CLCHAR_LENGTH_DEVICE]; 
			pdinfo_string << "DEVICE " << j << " INFOMATION: " << endl;

			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, CLCHAR_LENGTH_DEVICE, device_param, nullptr);
			pdinfo_string << "> d.name - " << device_param << endl;

			clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, CLCHAR_LENGTH_DEVICE, device_param, nullptr);
			pdinfo_string << "> vendor - " << device_param << endl;

			clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, CLCHAR_LENGTH_DEVICE, device_param, nullptr);
			pdinfo_string << "> version - " << device_param << endl;

			clGetDeviceInfo(devices[j], CL_DEVICE_PROFILE, CLCHAR_LENGTH_DEVICE, device_param, nullptr);
			pdinfo_string << "> profile - " << device_param << endl;

			// opencl system size_t.
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(size_t), &device_number, nullptr);
			pdinfo_string << _INFOPRINT_HEAD << "Const_ParamMax - " << device_number << " Entry" << endl;

			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(size_t), &device_number, nullptr);
			pdinfo_string << _INFOPRINT_HEAD << "Clock_FrequencyMax - " << device_number << " MHz" << endl;

			clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(size_t), &device_number, nullptr);
			pdinfo_string << _INFOPRINT_HEAD << "Global_MemorySize - " 
				<< device_number / double(1024.0 * 1024.0) << " MiB" << endl;

			clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(size_t), &device_number, nullptr);
			pdinfo_string << _INFOPRINT_HEAD << "Global_CahceSize  - " 
				<< device_number / 1024.0 << " KiB" << endl;

			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(size_t), &device_number, nullptr);
			pdinfo_string << _INFOPRINT_HEAD << "Const_BufferSize - " 
				<< device_number / double(1024.0 * 1024.0) << " MiB" << endl;

			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &device_number, nullptr);
			int32_t rectmat = int32_t(sqrt(device_number));
			pdinfo_string << _INFOPRINT_HEAD << "WorkGroup_NumberMax - " 
				<< device_number << " (" << rectmat << "x" << rectmat << ")" << endl;

			pdinfo_string << "------------------------------------------------" << endl;

			delete[] device_param;
		}
		delete[] devices;
	}
	delete[] platforms;
	get_system_str = pdinfo_string.str();
	
	return return_state;
}