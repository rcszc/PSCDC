// cdc_calcnode. RCSZ.
// @pomelo_star_studio 2023-2024.
// OpenCL, Boost.Asio

#ifndef _CDC_CALCNODE_HPP
#define _CDC_CALCNODE_HPP
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

// CDC-CALCNODE => CDC-OPENCL
#include "PSCDC_OPENCL/cdc_maths_tool.h"

/*
* struct:
* |__ main thread: network request.
* |__ work thread: opencl calculate.
* |
* [main => command => work]
*/

namespace NODE_GLOBAL {

	extern uint32_t    global_node_state;
	extern std::string global_node_name;
}

std::string __get_current_time();
#define TIEM_CURRENT __get_current_time() // ʱ���.

#define NODE_HEAD_CACHE "[node_cache]: "
// data process & cache.
namespace NodeCache {
	namespace DataMatrix {

		extern mocl_Matrix3D TempCalcMatrix_input;
		extern mocl_Matrix3D TempCalcMatrix_output;
	}
	/*
	* @param  vector<float>& (input fp32 array)
	* @return string         (binary data array)
	*/
	std::string EncodeFAtoBS(const std::vector<float>& FloatArray);
	/*
	* @param  string&        (binary data array)
	* @return vector<float>& (input fp32 array)
	*/
	std::vector<float> DecodeBStoFA(const std::string& StringArray);

	// string => float array => matrix3d => free string.
	mocl_Matrix3D DataProcessMatrix(
		std::string& StringArray,
		size_t x, size_t y, size_t z, 
		bool& state
	);

	// matrix 3d => float array => free matrix 3d.
	std::vector<float> DataProcessFP32array(mocl_Matrix3D InMatrix, bool& state);

	// file path, string, command["write", "clear"].
	bool SystemTempFile(const char* Path, const std::string& FileStr, const std::string CMD);
}

// key-value config. [.cfg]
namespace Config {

#define CFG_STATE_FAILED_FILE  0xE001
#define CFG_STATE_FAILED_MATCH 0xE002 
#define CFG_STATE_FAILED_LINE  0xE003

	class loadconfig {
	private:
		std::unordered_map<std::string, std::string> config_hashmap = {};
		std::string                                  config_filepath = "null";

	public:
		loadconfig(const std::string& xcore_configfile);
		~loadconfig() {}

		uint32_t module_state = 0x0001;

		std::string find_type_string(const std::string& find_key);
		double      find_type_double(const std::string& find_key);

		// set. return 0:failure, 1:success.
		bool modify_list_value(const std::string& key, const std::string& modify_value);

		// add. key_value.
		void add_list_value(const std::string& key, const std::string& value) {
			config_hashmap[key] = value;
		}

		// cover write.
		bool write_configfile();
	};
}

#define HTTP_END "\r\n\r\n"
#define NODE_HEAD_NETWORK "[node_network]: "
// boost asio, network & command.
namespace NodeNetwork {
	/*
	* start node_server main thread.
	* @param  conat char* (config file path)
	* @return void
	*/
	void start_node_server(const char* system_config);
}
// CDC Server Command.
#define NODE_REQCMD_NAME  "NodeName"  //   �ڵ�����. ret.cmd
#define NODE_REQCMD_STATE "NodeState" //   �ڵ�״̬. ret.cmd
#define NODE_REQCMD_INFO  "NodeDevi"  //   �ڵ��豸. ret.dat
#define NODE_REQCMD_CALC  "NodeCalc"  //   �ڵ㿪ʼ����. ret.cmd
#define NODE_REQCMD_RETD  "NodeRet"   // $ �ڵ㷵������. ret.dat (Mat3D => string)
#define NODE_REQCMD_INPGM "NodePgm"   // $ �ڵ���ճ���. ret.cmd (CL���ļ�)
#define NODE_REQCMD_INCFG "NodeCfg"   // $ �ڵ��������. ret.cmd (��������)
#define NODE_REQCMD_INDAT "NodeRec"   // $ �ڵ��������. ret.cmd (string => Mat3D)
#define NODE_REQCMD_CLOSE "NodeClose" // * �رսڵ������.

#define NODE_STATE_IDLE 0xA000 // �ڵ������.
#define NODE_STATE_DTOK 0xA001 // �ڵ�����������. (�ȴ�����)
#define NODE_STATE_CALC 0xA002 // �ڵ������.
#define NODE_STATE_WAIT 0xA003 // �ڵ�ȴ���. (δ��ȡ���)

#define NODE_HEAD_THREAD "[node_thread]: "
namespace WorkThread {

	void Work_OpenCLexec();  // run opencl script.
	void Work_ThreadStart(); // start work thread[async].
	void Work_ThreadExit();  // exit work thread.
}

#endif