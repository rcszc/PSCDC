// cdc_mainnode_net.
#include <fstream>
#include <boost/asio.hpp>

#include "cdc_mainnode_net.h"

using namespace std;

string send_string_format(
    string head_unique,
    string command      = "null",
    string data_length  = "0",
    string data_source  = "-",
    string data_index_x = "0", string data_index_y = "0", string data_index_z = "0",
    string http_version = "HTTP/1.1"
) {
    return head_unique + " " + command + " " +
        data_length + " " + data_source + " " + 
        data_index_x + " " + data_index_y + " " + data_index_z + " " + 
        http_version;
}

namespace NetworkReceive {
    struct request_msgstr {

        // return node unique.
        // IP_Address + port
        string head_unique;

        string node_ret_command;

        size_t node_ret_datalen;
        string node_ret_data;
    };

    request_msgstr RequestReturn(const string& retdata) {

        request_msgstr reqdata = {};
        string datalength = {};

        istringstream ISS(retdata);

        ISS >> reqdata.head_unique >> reqdata.node_ret_command;
        ISS >> datalength;
        // length: string => size_t.
        reqdata.node_ret_datalen = stoull(datalength);

        if (reqdata.node_ret_datalen == NULL) {
            ISS >> reqdata.node_ret_data;
            reqdata.node_ret_data = "";
        }
        else if (retdata.size() > reqdata.node_ret_datalen) {
            ISS.get();
            for (size_t i = 0; i < reqdata.node_ret_datalen; ++i) {

                char temp = NULL;
                ISS.get(temp);
                reqdata.node_ret_data += temp;
            }
        }
        else
            LOGOUT(NETWORK_INFOHEAD + string("Param error (length)."));

        return reqdata;
    }
}

namespace NetworkRequest {

    string SendRequest(const string& server_ip, const uint16_t& server_port, const string& request, bool& conn_state) {
        // boost network.
        boost::asio::io_service        psas_io_service;
        boost::asio::ip::tcp::socket   psas_socket(psas_io_service);
        boost::asio::ip::tcp::endpoint psas_endpoint(boost::asio::ip::address::from_string(server_ip), server_port);
        // connect => server.
        try {
            // type = ipv4.
            psas_endpoint = boost::asio::ip::tcp::endpoint(
                boost::asio::ip::make_address_v4(server_ip), server_port
            );

            // socket connect.
            psas_socket.connect(psas_endpoint);

            string sendreq = request + HTTP_END;
            // send request => server.
            try {
                boost::asio::write(psas_socket, boost::asio::buffer(sendreq));
            }
            catch (...) {
                // const boost::system::system_error& errcode
                // error write.

                string temp = "Failed send string";
                LOGOUT(NETWORK_INFOHEAD + temp);
            }

            // receive server result.
            boost::asio::streambuf response;
            boost::asio::read_until(psas_socket, response, HTTP_END);

            // return string.
            istream res_stream(&response);
            string  result_string = {};

            stringstream buffer;
            buffer << res_stream.rdbuf();
            result_string = buffer.str();

            conn_state = true;
            return result_string;
        }
        catch (...) {

            conn_state = false;
            string temp = "Failed connect: ";
            LOGOUT(NETWORK_INFOHEAD + temp + server_ip + ":" + to_string(server_port));
            return "";
        }
        psas_socket.close();
    }
}

// 节点返回 信息 & 数据 处理队列.
namespace ProcessQueue {
    // result queue & thread mutex.
    queue<NetworkReceive::request_msgstr> pcs_global_queue = {};
    mutex                                 pcs_global_mutex = {};

    void PushQueue(const NetworkReceive::request_msgstr& request) {
        {
            lock_guard<mutex> lock(pcs_global_mutex);
            pcs_global_queue.push(request);
        }
    }

    namespace process {
        // 发送指令.
        // threads. task => push => threadpool.
        class send_process_task {
        protected:
            bool sendstate = false;

        public:
            send_process_task(const string& send, const string& server_ip, const uint16_t& server_port) {

                istringstream ISS(send);
                string cmdtemp = {};
                ISS >> cmdtemp >> cmdtemp; cmdtemp.insert(NULL, "MainNode.send_request: ");

                LOGOUT(NETWORK_INFOHEAD + cmdtemp + " " + server_ip + ":" + to_string(server_port));

                NetworkReceive::request_msgstr result_dat = NetworkReceive::RequestReturn(
                    NetworkRequest::SendRequest(server_ip, server_port, send, sendstate)
                );

                // send => wait => return => process => push_queue.
                if (sendstate)
                    PushQueue(result_dat);
            };
            ~send_process_task() {};
        };
    }

    void ProcessQueue(vector<CDC_CALC_NODE>& nodedat) {
        static size_t FEnumber = NULL;
        {
            // 搜索节点数据 匹配 队列首元素(请求接收队列). 
            lock_guard<mutex> lock(pcs_global_mutex);
            while (!pcs_global_queue.empty()) {
                
                NetworkReceive::request_msgstr pcsdata = pcs_global_queue.front();
                for (auto& data : nodedat) {

                    if (pcsdata.head_unique == 
                        (data.node_server_address + string(":") + to_string(data.node_server_port))
                    ) {
                        // command => data.
                        // [节点返回状态处理].
                        if (pcsdata.node_ret_command == NODE_REQCMD_NAME)  data.node_name  = pcsdata.node_ret_data;
                        if (pcsdata.node_ret_command == NODE_REQCMD_STATE) data.node_state = (uint32_t)stoi(pcsdata.node_ret_data);
                        if (pcsdata.node_ret_command == NODE_REQCMD_INFO) {

                            data.node_device_info = pcsdata.node_ret_data;
                            data.node_server_connect = true;
                        }

                        // [节点返回数据处理].
                        if (pcsdata.node_ret_command == NODE_REQCMD_RETD) {

                            string savefilename_temp = 
                                "PSCDC-MAINSYSTEM/ResultData/cdc_result" + to_string(FEnumber) + ".fpdat";
                            ++FEnumber; // name unique count.

                            bool process_state = true;

                            // stream data => write matrix file.
                            MainDataFile::WriteFloatDataFile(
                                savefilename_temp.c_str(),
                                CalcDataProcess::DataProcessMatrix(
                                    pcsdata.node_ret_data,
                                    CalcProgram::cl_matrix_config[0],
                                    CalcProgram::cl_matrix_config[1],
                                    CalcProgram::cl_matrix_config[2],
                                    process_state
                                )
                            );
                        }
                    }
                }
                pcs_global_queue.pop();
            }
        }
    }
}

namespace CalcProgram {

    string  cl_kernel_program = {};
    string  cl_kernel_program_config = {};
    size_t  cl_matrix_config[3] = {};
    
    void init_calc_program(const char* pgm, const char* cfg) {

        cl_kernel_program = load_program_file(pgm);
        cl_kernel_program_config = load_program_file(cfg);
    }

    string load_program_file(const char* FilePath) {

        ifstream loadcldata(FilePath);
        if (!loadcldata.is_open()) {

            LOGOUT(NETWORK_INFOHEAD + string("Failed cl_kernel file: ") + FilePath);
            return "";
        }

        stringstream str_buffer;
        str_buffer << loadcldata.rdbuf();

        return str_buffer.str();
    }
}

#include "PSCDC_THREADPOOL/cdc_main_threadpool.hpp"

namespace CalcNodes {
    // concurrent thread pool.
    ThreadPool::xcore_threadpool SystemThreadPool(32);

    namespace loadfile {
        // 分布式部署 => 文件处理线程池 =移交=> 发送线程池.
        class process_loadfile {
        public:
            process_loadfile(string name, string send, CDC_CALC_NODE node) {

                // 3D matrix => string.
                bool         state_temp  = false;
                DataMatrix3D matrix_temp = MainDataFile::ReadFloatDataFile(name.c_str());

                // matrix_temp.mat_xlength == 0 无效数据.
                if (matrix_temp.mat_xlength != NULL) {

                    // Strcut: ["data1"][" "]["data2"][" "]...
                    string data_temp = CalcDataProcess::EncodeFAtoBS(
                        CalcDataProcess::DataProcessFP32array(
                            matrix_temp, state_temp
                        )
                    );
                    if (state_temp) {

                        SystemThreadPool.Tp_PushTask<ProcessQueue::process::send_process_task>
                            (send_string_format(
                                send,                        // 头标识
                                NODE_REQCMD_INDAT,           // 指令
                                to_string(data_temp.size()), // 数据长度
                                data_temp,                   // 计算数据
                                to_string(matrix_temp.mat_xlength), // mat x
                                to_string(matrix_temp.mat_ylength), // mat y
                                to_string(matrix_temp.mat_zlength)  // mat z
                            ),
                                node.node_server_address, node.node_server_port);

                        // 从任务数据队列中删除.
                        CALC_DATA_INDEX::TaskDataFile.pop();
                    }
                    else
                        LOGOUT(NETWORK_INFOHEAD + string("Failed send data_matrix."));
                }
                else
                    LOGOUT(NETWORK_INFOHEAD + string("Failed data error."));
            };
            ~process_loadfile() {};
        };
    }

	vector<CDC_CALC_NODE> calcnodes_state = {};
    bool calcnode_global_operations[5] = {};

	void calcnode_push(vector<CDC_CALC_NODE>& data) {

		CDC_CALC_NODE NODE_ADDTEMP = {};
		calcnodes_state.push_back(NODE_ADDTEMP);
	}

    // calcnode_process_state => calcnode_state_monitor.
    // sample = 1000ms.
    void calcnode_state_monitor(vector<CDC_CALC_NODE>& data) {

        // timer sample.
        static chrono::steady_clock::time_point timetemp = chrono::steady_clock::now();

        // [5000ms].
        if (calcnode_global_operations[0] &&
            (chrono::duration_cast<chrono::milliseconds>(
                chrono::steady_clock::now() - timetemp
            ).count() > 5000)
        ) {
            // flag => find state.
            for (auto& pcsdat : data)
                pcsdat.oper_state = true;

            timetemp = chrono::steady_clock::now();
        }
    }
    
    // [执行分布式部署].
    void calcnode_arrange_res(vector<CDC_CALC_NODE>& data, ThreadPool::xcore_threadpool& threadpool) {
        // loadfile thread pool.
        static ThreadPool::xcore_threadpool LoadFileThreadPool(16);

        if (calcnode_global_operations[2]) {
            for (const auto& dat : data) {
                // 计算节点 已连接 & 空闲.
                if (dat.node_server_connect && (dat.node_state == NODE_STATE_IDLE)) {

                    string nodehead =
                        dat.node_server_address + string(":") + to_string(dat.node_server_port);

                    /*
                    * 发送计算程序配置.
                    */
                    threadpool.Tp_PushTask<ProcessQueue::process::send_process_task>
                        (send_string_format(
                            nodehead,                                                // 头标识
                            NODE_REQCMD_INCFG,                                       // 指令
                            to_string(CalcProgram::cl_kernel_program_config.size()), // 数据长度
                            CalcProgram::cl_kernel_program_config                    // 配置程序
                        ),
                            dat.node_server_address, dat.node_server_port);
                    /*
                    * 发送 OCL 计算程序.
                    */
                    threadpool.Tp_PushTask<ProcessQueue::process::send_process_task>
                        (send_string_format(
                            nodehead,                                          // 头标识
                            NODE_REQCMD_INPGM,                                 // 指令
                            to_string(CalcProgram::cl_kernel_program.size()),  // 数据长度
                            CalcProgram::cl_kernel_program                     // 计算程序
                        ),
                            dat.node_server_address, dat.node_server_port);
                    /*
                    * 发送计算任务数据.
                    * 文件处理线程池 => 发送线程池.
                    * IndexQueue => File => MatrixData => StreamData => Send
                    */
                    LoadFileThreadPool.Tp_PushTask<loadfile::process_loadfile>(CALC_DATA_INDEX::TaskDataFile.front(), nodehead, dat);
                }
                else LOGOUT(NETWORK_INFOHEAD + string("Failed send queue = 0."));
            }
        }
    }

    // [执行分布式计算].
    void calcnode_arrange_calc(vector<CDC_CALC_NODE>& data, ThreadPool::xcore_threadpool& threadpool) {

        if (calcnode_global_operations[3]) {
            for (const auto& dat : data) {
                // 计算节点 已连接 & 就绪.
                if (dat.node_server_connect && (dat.node_state == NODE_STATE_DTOK)) {

                    string nodehead =
                        dat.node_server_address + string(":") + to_string(dat.node_server_port);
                    /*
                    * 发送计算指令.
                    */
                    threadpool.Tp_PushTask<ProcessQueue::process::send_process_task>
                        (send_string_format(nodehead, NODE_REQCMD_CALC), dat.node_server_address, dat.node_server_port);
                }
                else
                    LOGOUT(NETWORK_INFOHEAD + string("[CALC]: Failed node: ") + dat.node_name);
            }
        }
    }

    // [执行分布式写回数据].
    void calcnode_arrange_ret(vector<CDC_CALC_NODE>& data, ThreadPool::xcore_threadpool& threadpool) {

        if (calcnode_global_operations[4]) {
            for (const auto& dat : data) {
                // 计算节点 已连接 & 等待.
                if (dat.node_server_connect && (dat.node_state == NODE_STATE_WAIT)) {

                    string nodehead =
                        dat.node_server_address + string(":") + to_string(dat.node_server_port);
                    /*
                    * 发送返回数据指令.
                    */
                    threadpool.Tp_PushTask<ProcessQueue::process::send_process_task>
                        (send_string_format(nodehead, NODE_REQCMD_RETD), dat.node_server_address, dat.node_server_port);
                }
                else
                    LOGOUT(NETWORK_INFOHEAD + string("[RET]: Failed node: ") + dat.node_name);
            }
        }
    }

    void calcnode_process_event(vector<CDC_CALC_NODE>& data) {

        calcnode_state_monitor(data);

        calcnode_arrange_res(data, SystemThreadPool);
        calcnode_arrange_calc(data, SystemThreadPool);
        calcnode_arrange_ret(data, SystemThreadPool);

        // [其他非数据指令处理].
        for (auto& pcsdat : data) {

            // [连接计算节点].
            if (pcsdat.oper_connect) {

                string nodehead = 
                    pcsdat.node_server_address + string(":") + to_string(pcsdat.node_server_port);
                // node name, node state, node device info.
                {
                    SystemThreadPool.Tp_PushTask<ProcessQueue::process::send_process_task>
                        (send_string_format(nodehead, NODE_REQCMD_NAME), pcsdat.node_server_address, pcsdat.node_server_port);

                    SystemThreadPool.Tp_PushTask<ProcessQueue::process::send_process_task>
                        (send_string_format(nodehead, NODE_REQCMD_STATE), pcsdat.node_server_address, pcsdat.node_server_port);

                    SystemThreadPool.Tp_PushTask<ProcessQueue::process::send_process_task>
                        (send_string_format(nodehead, NODE_REQCMD_INFO), pcsdat.node_server_address, pcsdat.node_server_port);
                }
            }

            // [查询节点状态].
            if (pcsdat.oper_state || calcnode_global_operations[1]) {

                string nodehead =
                    pcsdat.node_server_address + string(":") + to_string(pcsdat.node_server_port);

                SystemThreadPool.Tp_PushTask<ProcessQueue::process::send_process_task>
                    (send_string_format(nodehead, NODE_REQCMD_STATE), pcsdat.node_server_address, pcsdat.node_server_port);
                pcsdat.oper_state = false;
            }

            // [关闭计算节点].
            if (pcsdat.oper_close) {

                string nodehead =
                    pcsdat.node_server_address + string(":") + to_string(pcsdat.node_server_port);

                SystemThreadPool.Tp_PushTask<ProcessQueue::process::send_process_task>
                    (send_string_format(nodehead, NODE_REQCMD_CLOSE), pcsdat.node_server_address, pcsdat.node_server_port);

                pcsdat.node_device_info    = "null";
                pcsdat.node_name           = "null";
                pcsdat.node_server_connect = false;
                pcsdat.node_state          = 0xEEEE;
            }
        }
        // [监听 & 处理 返回结果].
        ProcessQueue::ProcessQueue(calcnodes_state);
    }
}