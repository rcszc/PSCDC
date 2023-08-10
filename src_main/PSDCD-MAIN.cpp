// PSDCD-MAIN.
// PomeloStar Main Node.

#include <iostream>

#include <sstream>
#include <iomanip>
#include <string>

#include "PSCDC-MAINNODE/cdc_mainnode_panel.h"
#include "PSCDC-MAINNODE/cdc_mainnode_net.h"

using namespace std;

int main() {
	
	// PSCDC-MAINSYSTEM/TestData.fpdat
	Config::loadconfig cfg("PSCDC-MAINSYSTEM/opencl_calc_config.cfg");

	CalcProgram::cl_matrix_config[0] = (size_t)cfg.find_type_double("opencl_matrix_output_num");
	CalcProgram::cl_matrix_config[1] = (size_t)cfg.find_type_double("opencl_matrix_xlen");
	CalcProgram::cl_matrix_config[2] = (size_t)cfg.find_type_double("opencl_matrix_ylen");

	CalcProgram::init_calc_program(
		"PSCDC-MAINSYSTEM/opencl_calc_kernel.cl",
		"PSCDC-MAINSYSTEM/opencl_calc_config.cfg"
	);

	opengl_panel_init("MainNode", 1000, 600, []() {

		GLFW_IMGUI_PANEL::draw_imgui_panel();
		CalcNodes::calcnode_process_event(CalcNodes::calcnodes_state);
	});
}