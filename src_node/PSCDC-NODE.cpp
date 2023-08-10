// PSCDC-NODE. Main Thread.
// RCSZ. 2023.8

#include <iostream>

#include "PSCDC-CALCNODE/cdc_calcnode.hpp"
#include "PSCDC-CALCNODE/PSCDC_OPENCL/cdc_opencl.h"

using namespace std;

int main() {
	
	WorkThread::Work_ThreadStart();
	{
		NodeNetwork::start_node_server("PSCDC-SYSTEM/system_config.cfg");
	}
	WorkThread::Work_ThreadExit();
}