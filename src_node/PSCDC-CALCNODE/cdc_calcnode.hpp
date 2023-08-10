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
#define TIEM_CURRENT __get_current_time() // 时间戳.

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
#define NODE_REQCMD_NAME  "NodeName"  //   节点名称. ret.cmd
#define NODE_REQCMD_STATE "NodeState" //   节点状态. ret.cmd
#define NODE_REQCMD_INFO  "NodeDevi"  //   节点设备. ret.dat
#define NODE_REQCMD_CALC  "NodeCalc"  //   节点开始计算. ret.cmd
#define NODE_REQCMD_RETD  "NodeRet"   // $ 节点返回数据. ret.dat (Mat3D => string)
#define NODE_REQCMD_INPGM "NodePgm"   // $ 节点接收程序. ret.cmd (CL核文件)
#define NODE_REQCMD_INCFG "NodeCfg"   // $ 节点就收配置. ret.cmd (计算配置)
#define NODE_REQCMD_INDAT "NodeRec"   // $ 节点接收数据. ret.cmd (string => Mat3D)
#define NODE_REQCMD_CLOSE "NodeClose" // * 关闭节点服务器.

#define NODE_STATE_IDLE 0xA000 // 节点空闲中.
#define NODE_STATE_DTOK 0xA001 // 节点接收数据完毕. (等待计算)
#define NODE_STATE_CALC 0xA002 // 节点计算中.
#define NODE_STATE_WAIT 0xA003 // 节点等待中. (未提取结果)

#define NODE_HEAD_THREAD "[node_thread]: "
namespace WorkThread {

	void Work_OpenCLexec();  // run opencl script.
	void Work_ThreadStart(); // start work thread[async].
	void Work_ThreadExit();  // exit work thread.
}

#endif