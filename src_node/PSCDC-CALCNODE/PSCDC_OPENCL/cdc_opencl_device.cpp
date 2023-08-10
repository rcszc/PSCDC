// cdc_opencl_device.
#include <iostream>

#include "cdc_opencl.h"

using namespace std;

uint32_t OPENCL_TYPE_DEVICE::GetPlatformNumber() {
	cl_platform_id* platforms = nullptr;
	cl_uint numplatforms = NULL;
	cl_int errorcode = NULL;

	clGetPlatformIDs(NULL, nullptr, &numplatforms);
	platforms = new cl_platform_id[numplatforms];
	errorcode = clGetPlatformIDs(numplatforms, platforms, nullptr);

	if (errorcode != CL_SUCCESS || !numplatforms) {

		cout << "OPENCL OCLPD: Failed platforms. code:" << errorcode << endl;
	}
	platforms_num = numplatforms;
	platformsptr = platforms;

	return platforms_num;
}

uint32_t OPENCL_TYPE_DEVICE::GetDevicemNumber(uint32_t platform) {
	cl_uint numdevices = NULL;
	cl_int errorcode = NULL;

	errorcode = clGetDeviceIDs(platformsptr[platform], CL_DEVICE_TYPE_GPU, NULL, nullptr, &numdevices);
	if (errorcode != CL_SUCCESS || !numdevices) {

		cout << "OPENCL OCLPD: There is no GPU. code:" << errorcode << endl;
	}
	return numdevices;
}

void OPENCL_TYPE_DEVICE::get_device_id(uint32_t platform, uint32_t device, cl_device_id* deviceid) {
	cl_platform_id platforms = nullptr;
	cl_int errorcode = NULL;
	uint32_t numplateforms = NULL;
	uint32_t numdevices = NULL;

	clGetPlatformIDs(platform, &platforms, &numplateforms);

	if (platform <= numplateforms) {
		errorcode = clGetDeviceIDs(platforms, CL_DEVICE_TYPE_GPU, device, deviceid, &numdevices);
		if (errorcode != CL_SUCCESS || !numdevices) {

			cout << "OPENCL OCLPD: Failed get deviceID. code:" << errorcode << endl;
		}
	}
	else {
		deviceid = nullptr;
		cout << "OPENCL OCLPD: no device." << endl;
	}
}