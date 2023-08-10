// cdc_calcnode_work.
#include <thread>
#include <mutex>

#include "cdc_calcnode.hpp"
#include "PSCDC_OPENCL/cdc_opencl.h"

using namespace std;

// USE NVIDIA GPU.
extern "C" {
	_declspec(dllexport)
		unsigned long NvOptimusEnablement = 0x00000001;
}

thread calc_work_thread = {};
mutex  calc_work_mutex  = {};
bool   calc_workflag_exit = false;
bool   calc_workflag_task = false;

void WorkThread_EventLoop() {
	// work thread event_loop.
	while (true) {
		if (calc_workflag_task) {

			Config::loadconfig opencl_calc_config("PSCDC-SYSTEM/system_program_cfg.cfg");
			OPENCL_CALC_MATRIX opencl_calc_matrix;

			opencl_calc_matrix.CALC_type_device(
				(uint32_t)opencl_calc_config.find_type_double("device_type_p"),
				(uint32_t)opencl_calc_config.find_type_double("device_type_d")
			);
			/*
			* OpenCL ��ʼ��.
			* ���� [.cl] ���ļ��˺���.
			*/
			opencl_calc_matrix.CLAC_init_opencl(
				"PSCDC-SYSTEM/system_program_tmp.cl",
				opencl_calc_config.find_type_string("kernel_function_name").c_str()
			);
			/*
			* ���� GPU �������С.
			*/
			opencl_calc_matrix.CALC_set_workgroup(
				(size_t)opencl_calc_config.find_type_double("opencl_work_size_x"),
				(size_t)opencl_calc_config.find_type_double("opencl_work_size_y")
			);                     
			/*
			* ���ö�д��������.
			* ���õ����������ݴ�С.
			*/
			size_t matrix_size =
				(size_t)opencl_calc_config.find_type_double("opencl_matrix_xlen") * 
				(size_t)opencl_calc_config.find_type_double("opencl_matrix_ylen");
			
			opencl_calc_matrix.CALC_set_computeIO(
				(size_t)opencl_calc_config.find_type_double("opencl_matrix_input_num"),
				(size_t)opencl_calc_config.find_type_double("opencl_matrix_output_num"),
				matrix_size
			);
			
			// ���� n * 2D ����[3D].
			opencl_calc_matrix.CALC_write_matrix(NodeCache::DataMatrix::TempCalcMatrix_input);

			NodeCache::DataMatrix::TempCalcMatrix_output = 
				opencl_calc_matrix.CALC_read_result(); // д�� n * 2D ����[3D]

			// set state.
			NODE_GLOBAL::global_node_state = NODE_STATE_WAIT;
			calc_workflag_task = false;
			
			// ����ʱ�� �ٶ� & ��Ϣ.
			cout << "[OPENCL.CALC]: Calculate time: " << (float)opencl_calc_matrix.calc_total_time << " ms" << endl;
			cout << "[OPENCL.CALC]: Calculate speed: " <<
				float(
					NodeCache::DataMatrix::TempCalcMatrix_output.mat_xlength *
					NodeCache::DataMatrix::TempCalcMatrix_output.mat_ylength *
					NodeCache::DataMatrix::TempCalcMatrix_output.mat_zlength * 4
					) / 
				(float)opencl_calc_matrix.calc_total_time / 1048.576f;

			cout << " Mib / s" << endl;
		}
		if (calc_workflag_exit)
			break;
	}
}

namespace WorkThread {

	void Work_OpenCLexec() {
		{
			unique_lock<mutex> lock(calc_work_mutex);
			calc_workflag_task = true;
		}
	}
	
	void Work_ThreadStart() {
		// start work thread.
		try {
			calc_work_thread = thread(WorkThread_EventLoop);
			cout << TIEM_CURRENT << NODE_HEAD_THREAD << "OpenCL work thread start." << endl;
		}
		catch (const system_error& errinfo) {
			// error process.
			cout << NODE_HEAD_THREAD << "Failed create thread." << endl;
			cout << NODE_HEAD_THREAD << "Error info: " << errinfo.what() << endl;
		}
	}

	void Work_ThreadExit() {
		// free work thread.
		{
			unique_lock<mutex> lock(calc_work_mutex);
			calc_workflag_exit = true;
		}
		try {
			calc_work_thread.join();
			cout << TIEM_CURRENT << NODE_HEAD_THREAD << "OpenCL work thread exit." << endl;
		}
		catch (const system_error& errinfo) {
			// error process.
			cout << NODE_HEAD_THREAD << "Failed free thread." << endl;
			cout << NODE_HEAD_THREAD << "Error info: " << errinfo.what() << endl;
		}
	}
}