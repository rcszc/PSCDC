// cdc_calcnode_network.
#include <chrono>
#include <functional>
#include <boost/asio.hpp>

#include "cdc_calcnode.hpp"
#include "PSCDC_OPENCL/cdc_opencl.h"

using boost::asio::ip::tcp;
using namespace std;

boost::asio::io_context http_io_context;
boost::asio::ip::tcp::acceptor http_acceptor(http_io_context);

namespace NODE_GLOBAL {
	uint32_t global_node_state = NODE_STATE_IDLE;
	string   global_node_name = {};
}

namespace system_request {
	struct request_msgstr {
		
		string sysmain_unique;
		string head_command;

		string middle_data_len;
		string middle_data;

		string data_infoparam[3];
	};

	/*
	* DATA-STRCUT: [address][command][data.length][data.string][data.x][data.y][data.z][http.version][END]
	* Matrix 3D. Length = x * y * z * 4 (float)
	*/
	request_msgstr RequestMessage(boost::asio::streambuf& request) {

		string request_string = boost::asio::buffer_cast<const char*>(request.data());
		string request_method = {}, http_version = {};
		request_msgstr reqdata = {};

		istringstream ISS(request_string);
		{
			ISS >> reqdata.sysmain_unique;
			ISS >> reqdata.head_command >> reqdata.middle_data_len;

			size_t dataarray_len = stoull(reqdata.middle_data_len);

			if (dataarray_len == NULL) {
				ISS >> reqdata.middle_data;
				reqdata.middle_data = "";
			}
			else if (request_string.size() > dataarray_len) {
				ISS.get();
				for (size_t i = 0; i < dataarray_len; ++i) {

					char temp = NULL;
					ISS.get(temp);
					reqdata.middle_data += temp;
				}
			}
			else
				cout << NODE_HEAD_NETWORK << "Param error (length)." << endl;

			for (size_t i = 0; i < 3; ++i)
				ISS >> reqdata.data_infoparam[i];

			ISS >> http_version;
		}

		cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "Request: version: " << http_version
			<< ", command: " << reqdata.head_command << endl;
		return reqdata;
	}
}

namespace system_command {

	string CommandReturnName(size_t& length) { 

		length = NODE_GLOBAL::global_node_name.size();
		return NODE_GLOBAL::global_node_name;
	}

	string CommandReturnState(size_t& length) { 

		length = to_string(NODE_GLOBAL::global_node_state).size();
		return to_string(NODE_GLOBAL::global_node_state);
	}

	string CommandRunCalc(size_t& length) {

		if (NODE_GLOBAL::global_node_state == NODE_STATE_DTOK) {

			cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "[OCLwork]: Calculate..." << endl;
			NODE_GLOBAL::global_node_state = NODE_STATE_CALC;
			// network => work thread => opencl
			{
				WorkThread::Work_OpenCLexec();
			}
			length = NULL;
		}
		else
			cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "[OCLwork]: Failed state." << endl;
		return "";
	}

	// ******************************** data command ********************************

	// [数据指令] program => clear file => file.
	void CommandProgram(const string& path, const string& program) {

		cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "[DataCMD]: OpenCL program(kernel)." << endl;

		NodeCache::SystemTempFile(path.c_str(), program, "clear");
		// write temp file.
		NodeCache::SystemTempFile(path.c_str(), program, "write");
	}

	// [数据指令] config => clear file => file.
	void CommandProgramConfig(const string& path, const string& config) {

		cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "[DataCMD]: OpenCL program(config)." << endl;

		NodeCache::SystemTempFile(path.c_str(), config, "clear");
		// write temp file.
		NodeCache::SystemTempFile(path.c_str(), config, "write");
	}

	// [数据指令] data.receive => process.
	void CommandDataReceive(system_request::request_msgstr data) {

		// 节点为 空闲状态(NODE_STATE_IDLE) 才能接收数据.
		if (NODE_GLOBAL::global_node_state == NODE_STATE_IDLE) {

			cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "[DataCMD]: OpenCL matrix data process..." << endl;
			bool process_state = true;

			NodeCache::DataMatrix::TempCalcMatrix_input =
				NodeCache::DataProcessMatrix(
					data.middle_data,
					stoull(data.data_infoparam[0]),
					stoull(data.data_infoparam[1]),
					stoull(data.data_infoparam[2]),
					process_state
				);

			if (process_state)
				NODE_GLOBAL::global_node_state = NODE_STATE_DTOK;
		}
	}

	// [数据指令] data.return => return.
	void CommandDataReturn(string& data, size_t& data_length) {

		cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "[DataCMD]: OpenCL matrix data process." << endl;
		bool process_state = true;

		vector<float> temparray =
			NodeCache::DataProcessFP32array(NodeCache::DataMatrix::TempCalcMatrix_output, process_state);
		data = NodeCache::EncodeFAtoBS(temparray);
		data_length = data.size();

		// free float array.
		temparray.clear();
		temparray.shrink_to_fit();

		NODE_GLOBAL::global_node_state = NODE_STATE_IDLE;
	}

	// [数据指令] ocl_info => return.
	void CommandDeviceReturn(string& data, size_t& data_length) {

		cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "[DataCMD]: OpenCL device info." << endl;
		string returndata = {};

		if (MOCL_GETINFO_SYSTEM(returndata)) {

			data = returndata;
			data_length = data.size();
		}
	}
}

namespace NodeNetwork {
	using CommandHandler = function<string(size_t&)>;

	void start_node_server(const char* system_config) {
		// load command.
		unordered_map<string, CommandHandler> CmdMap;
		{
			CmdMap[NODE_REQCMD_NAME]  = system_command::CommandReturnName;
			CmdMap[NODE_REQCMD_STATE] = system_command::CommandReturnState;
			CmdMap[NODE_REQCMD_CALC]  = system_command::CommandRunCalc;
		}

		// load config.
		Config::loadconfig nodesys_config(system_config);

		if (nodesys_config.module_state == CFG_STATE_FAILED_FILE) {

			cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "Error system config: " << system_config << endl;
			exit(-1);
		}

		int64_t sleep_time = (int64_t)nodesys_config.find_type_double("calc_node_maintick");
		
		string kernel_path = nodesys_config.find_type_string("calc_node_cltmp");
		string config_path = nodesys_config.find_type_string("calc_node_fgmcfg");

		NODE_GLOBAL::global_node_name = nodesys_config.find_type_string("calc_node_name");

		// boost asio network.
		http_acceptor = boost::asio::ip::tcp::acceptor(
			http_io_context,
			boost::asio::ip::tcp::endpoint(
				boost::asio::ip::tcp::v4(),
				(uint16_t)nodesys_config.find_type_double("calc_node_port")
			)
		);

		// [Enable] 非阻塞监听.
		http_acceptor.non_blocking(true);
		cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "Node system ready[√]." << endl;

		chrono::steady_clock::time_point start_time = chrono::steady_clock::now();
		while (true) {

			// http 请求监听.
			if (
				chrono::duration_cast<chrono::milliseconds>(
					chrono::steady_clock::now() - start_time
				).count() > sleep_time
				) {
				// create socket link.
				unique_ptr<tcp::socket> socket_link = make_unique<tcp::socket>(http_io_context);

				boost::system::error_code continue_fg;
				http_acceptor.accept(*socket_link, continue_fg);

				// create socket => connect? => process string.
				if (!continue_fg) {

					string return_data_string = "-";
					size_t return_data_length = NULL;

					boost::asio::streambuf request_str;
					boost::asio::read_until(*socket_link, request_str, HTTP_END);

					// get request message.
					system_request::request_msgstr result = system_request::RequestMessage(request_str);
					// res command.
					{
						auto it = CmdMap.find(result.head_command);
						if (it != CmdMap.end()) {

							// execute command.
							return_data_string = it->second(return_data_length);
						}

						// [接收程序]. main => http => node => cache.
						if (result.head_command == NODE_REQCMD_INPGM) {
							system_command::CommandProgram(
								kernel_path,
								result.middle_data
							);
						}

						// [接收配置]. main => http => node => cache.
						if (result.head_command == NODE_REQCMD_INCFG) {
							system_command::CommandProgram(
								config_path,
								result.middle_data
							);
						}

						// [接收数据]. main => http => node => cache.
						if (result.head_command == NODE_REQCMD_INDAT)
							system_command::CommandDataReceive(result);

						// [返回数据]. node => cache => http => main.
						if (result.head_command == NODE_REQCMD_RETD) {
							system_command::CommandDataReturn(
								return_data_string,
								return_data_length
							);
						}

						// [返回信息]. node => info => http => main.
						if (result.head_command == NODE_REQCMD_INFO) {
							system_command::CommandDeviceReturn(
								return_data_string,
								return_data_length
							);
						}

						// close node server.
						if (result.head_command == NODE_REQCMD_CLOSE)
							break;
					}
					/*
					* DATA-STRUCT: [port][command][data.length][data.string][END]
					* Matrix 3D. Length = x * y * z * 4 (float).
					*/
					string response = 
						result.sysmain_unique +         " " +
						result.head_command +           " " +
						to_string(return_data_length) + " " + 
						return_data_string +            HTTP_END;

					cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "NodeReturn data.bytes: " << return_data_length << endl;
					// return result string data.
					boost::asio::write(*socket_link, boost::asio::buffer(response));
					socket_link->close();
				}
			}
		}
		cout << TIEM_CURRENT << NODE_HEAD_NETWORK << "System close server." << endl;
	}
}