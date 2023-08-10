// cdc_mainnode_net. RCSZ.
// @pomelo_star 2023 - 2024.
// PomeloStar Studio Link Server.

#ifndef _CDC_MAINNODE_NET_H
#define _CDC_MAINNODE_NET_H
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>

#include "PSCDC_DATAFILE/cdc_data_file.h"
#include "thread_safe_print.hpp"

#define NETWORK_INFOHEAD "[MAINCORE]: "
#define ADDRESS_CHAR_LENv4 16

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

namespace CALC_DATA_INDEX {
	extern std::queue<std::string> TaskDataFile;
}

// �ڵ�״̬��Ϣ.
struct CDC_CALC_NODE {

	char    node_server_address[ADDRESS_CHAR_LENv4];
	int32_t node_server_port;
	bool    node_server_connect;

	std::string node_name;
	std::string node_device_info;
	uint32_t    node_state;

	bool oper_connect; // �������Ӽ���ڵ�.
	bool oper_state;   // ��ѯ����ڵ�״̬.
	bool oper_task;    // �������(����).
	bool oper_close;   // �رռ���ڵ�(����).

	CDC_CALC_NODE() :
		node_server_address{ '1','2','7','.','0','.','0','.','1' },
		node_server_port(1000), node_server_connect(false),
		node_name("null"),      node_device_info("null"), node_state(0xEEEE),
		oper_connect(false)
	{}
};

namespace CalcProgram {

	extern std::string cl_kernel_program;
	extern std::string cl_kernel_program_config;
	// �������� ����ת������.
	extern size_t      cl_matrix_config[3];
	/*
	* @param  const char* (program)
	* @param  const char* (config)
	* @return void
	*/
	void init_calc_program(const char* pgm, const char* cfg);
	std::string load_program_file(const char* FilePath);
}

// CALC in DataMatrix size = CALC out DataMatrix size
namespace CalcNodes {

	extern std::vector<CDC_CALC_NODE> calcnodes_state;
	// 0:ʵʱ���, 1:ˢ�����нڵ�״̬, 2:����ֲ�ʽ, 3:�ֲ�ʽ����, 4:��ȡ������
	extern bool calcnode_global_operations[5];

	void calcnode_push(std::vector<CDC_CALC_NODE>& data);

	// eventloop tick. process => send.
	void calcnode_process_event(std::vector<CDC_CALC_NODE>& data);
}
#define NODE_STATE_IDLE 0xA000 // �ڵ������.
#define NODE_STATE_DTOK 0xA001 // �ڵ�����������(�ȴ�����).
#define NODE_STATE_CALC 0xA002 // �ڵ������.
#define NODE_STATE_WAIT 0xA003 // �ڵ�ȴ���(δ��ȡ���).

// ����ת��(���߳�). string <=> matrix.
// ��������ͬ.
namespace CalcDataProcess {
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

	// matrix 3d => float array => free matrix 3d.
	std::vector<float> DataProcessFP32array(DataMatrix3D InMatrix, bool& state);

	// string => float array => matrix3d => free string.
	DataMatrix3D DataProcessMatrix(
		std::string& StringArray,
		size_t x, size_t y, size_t z, bool& state
	);
}

#define HTTP_END "\r\n\r\n"

namespace NetworkRequest {
	/*
	* @param  string   (ip address)
	* @param  uint16_t (server port)
	* @param  string   (request)
	* @param  bool     (result state)
	* @return string   (result data)
	*/
	std::string SendRequest(
		const std::string& server_ip,
		const uint16_t&    server_port,
		const std::string& request,
		bool&              conn_state
	);
}

// CDC Server Command.
#define NODE_REQCMD_NAME  "NodeName"  //   �ڵ�����. ret.cmd
#define NODE_REQCMD_STATE "NodeState" //   �ڵ�״̬. ret.cmd
#define NODE_REQCMD_INFO  "NodeDevi"  //   �ڵ��豸. ret.dat

#define NODE_REQCMD_CALC  "NodeCalc"  //   �ڵ㿪ʼ����. ret.cmd
#define NODE_REQCMD_RETD  "NodeRet"   // $ �ڵ㷵������. ret.dat

#define NODE_REQCMD_INPGM "NodePgm"   // $ �ڵ���ճ���. ret.cmd (CL���ļ�)
#define NODE_REQCMD_INCFG "NodeCfg"   // $ �ڵ��������. ret.cmd (��������)
#define NODE_REQCMD_INDAT "NodeRec"   // $ �ڵ��������. ret.cmd (3ά����)

#define NODE_REQCMD_CLOSE "NodeClose" // * �رսڵ������.

#endif