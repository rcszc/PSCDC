// cdc_opencl_calc.
#include <iostream>

#include "cdc_opencl.h"

using namespace std;
// 仅支持 Matrix2D ( in.Matrix3D, out.Matrix3D )

//init opencl [packet].
void OPENCL_CALC_MATRIX::CLAC_init_opencl(const char* CLfile_path, const char* CLfunction) {
	// init config opencl
	opencl_packet.Context = OCL_CreateContext(&opencl_packet.Device);
	if (!opencl_packet.Context)
		__opencl_errorexit(_ERROR_CONTEXT, opencl_packet);

	opencl_packet.CommandQueue = OCL_CreateCommandQueue(opencl_packet.Context, opencl_packet.Device);
	if (!opencl_packet.CommandQueue)
		__opencl_errorexit(_ERROR_COMMANDQUEUE, opencl_packet);

	opencl_packet.Program = OCL_CreateProgram(opencl_packet.Context, opencl_packet.Device, CLfile_path);
	if (!opencl_packet.Program)
		__opencl_errorexit(_ERROR_PROGRAM, opencl_packet);

	opencl_packet.Kernel = clCreateKernel(opencl_packet.Program, CLfunction, NULL);
	if (!opencl_packet.Kernel)
		__opencl_errorexit(_ERROR_KERNEL, opencl_packet);
}

size_t OPENCL_CALC_MATRIX::WORK_GROUP_SIZE[2] = { 1,1 }; // GPU工作组.

void OPENCL_CALC_MATRIX::CALC_set_workgroup(size_t x, size_t y) {
	if ((x > 1) && (y > 1)) {
		WORK_GROUP_SIZE[0] = x;
		WORK_GROUP_SIZE[1] = y;
	}
	else
		cout << "MOPENCL Calc: Set work group n > 1." << endl;
}

// set input_memobj_num output_memobj_num
// Create memory objects.
void OPENCL_CALC_MATRIX::CALC_set_computeIO(size_t in_matrix_num, size_t out_matrix_num, size_t data_length) {
	memory_object_in  = in_matrix_num;
	memory_object_out = out_matrix_num;
	Array_size = _FLOAT_NUMSIZE(data_length);

	if (opencl_packet.MemObjects == nullptr) {
		opencl_packet.MemObjects_num = in_matrix_num + out_matrix_num;
		opencl_packet.MemObjects = new cl_mem[opencl_packet.MemObjects_num];
	}
	else {
		// free buffer.
		for (int32_t i = 0; i < opencl_packet.MemObjects_num; i++)
			clReleaseMemObject(opencl_packet.MemObjects[i]);
	}
	OCL_CreateMemoryObjects(opencl_packet.Context, in_matrix_num, out_matrix_num, opencl_packet.MemObjects);
}

void OPENCL_CALC_MATRIX::CALC_type_device(uint32_t NP, uint32_t ND) {
	if ((NP < 8) && (ND < 8)) {
		node_platform = NP;
		node_device = ND;
	}
	else
		cout << "MOPENCL Calc: Failed set platform device." << endl;
}

/*
* data => buffer matrix2d
* size = set.IO
*
* input.Matrix.size = output.Matrix.size
* input.Matrix.number / input.Matrix.number
*/
bool OPENCL_CALC_MATRIX::CALC_write_matrix(mocl_Matrix3D& inMatrix) {
	bool returnstate = true;

	if (inMatrix.mat_xlength == memory_object_in) {

		out_matrix_xlength = inMatrix.mat_ylength;
		out_matrix_ylength = inMatrix.mat_zlength;

		// convert.
		mocl_Matrix2D inbuffer = MOCL_TOOL_3DTO2D(inMatrix);

		calc_totaltime.Timing_Start();
		// write data => buffer
		if (OCL_WriteMemoryObjects(opencl_packet.CommandQueue, memory_object_in, opencl_packet.MemObjects, inbuffer))
			statecode = MCLASS_STATE_CMEMOBJ;

		if (OCL_SetKernelFunctionParam(opencl_packet.Kernel, opencl_packet.MemObjects_num, opencl_packet.MemObjects))
			statecode = MCLASS_STATE_CKERNEL;

		size_t matrix_num[2] = { out_matrix_xlength, out_matrix_ylength };

		int32_t errorcode = clEnqueueNDRangeKernel(opencl_packet.CommandQueue, opencl_packet.Kernel,
			2, NULL, matrix_num, WORK_GROUP_SIZE, NULL, nullptr, nullptr);

		if (errorcode != CL_SUCCESS) {
			// Execution failed.
			cout << "MOPENCL Calc: queueing kernel for execution. code:" << errorcode << endl;

			MOCL_FUNC_CLEARPK(opencl_packet);
			returnstate = false;
		}
	}
	else {
		cout << "MOPENCL Calc: Matrix entry != set_in_matrix" << endl;
		returnstate = false;
	}
	// [2023_08_08] bug. memory error.
	inMatrix.data = nullptr;

	return returnstate;
}

/*
* 结果拷贝到主机
* output n * matrix
* output.matrix = n * 2d = 3d
*/
mocl_Matrix3D OPENCL_CALC_MATRIX::CALC_read_result() {
	mocl_Matrix1D readgpubuffer = {};
	int32_t errorcode = NULL;

	// "clEnqueueReadBuffer" out matrix 1D
	readgpubuffer.data = new float[_FLOAT_SIZENUM(Array_size)];
	readgpubuffer.mat_xlength = _FLOAT_SIZENUM(Array_size);

	mocl_Matrix3D returnMatrx = MOCL_TOOL_NEWMATRIX2D(memory_object_out, out_matrix_xlength, out_matrix_ylength);

	// read output.
	// num = [0, _MemObj_out] out = [_MemObj_in, _MemObj_out]
	size_t count_out = NULL;
	for (size_t i = memory_object_in; i < memory_object_in + memory_object_out; i++) {

		errorcode = clEnqueueReadBuffer(opencl_packet.CommandQueue, opencl_packet.MemObjects[i], 
			CL_TRUE, NULL, Array_size, &readgpubuffer.data[0], NULL, NULL, NULL);

		// 主机数据重新组合成3维矩阵.
		mocl_sys_Matrix1ToMatrix3(returnMatrx, count_out, readgpubuffer, out_matrix_xlength, out_matrix_ylength);
		count_out++;
	}
	MOCL_TOOL_FREEMATRIX1D(readgpubuffer);

	// kernel compute end.
	calc_total_time = calc_totaltime.Timing_Ended();

	if (errorcode != CL_SUCCESS) {

		cout << "MOPENCL Calc: Reading result buffer. code:" << errorcode << endl;
		MOCL_TOOL_FREEMATRIX3D(returnMatrx);
	}
	return returnMatrx;
}