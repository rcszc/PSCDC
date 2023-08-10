# CALC Node

### 计算节点计算库采用 OpenCL 3.0 API
目前测试是 RTX4060 Labtop + i7-13700HX.

- cdc_calcnode.hpp
```cpp
// 启动节点服务器主线程事件循环
// 节点系统配置 ("calc_node_maintick"暂时没用)
# IPv4 0.0.0.0 Node Port.
calc_node_port = 11451
# Main Thread Speed(ms).
calc_node_maintick = 20
# Calc Node Name.
calc_node_name = "TEST-PSCDC"
# Temp OpenCL Program.
calc_node_cltmp  = "PSCDC-SYSTEM/system_program_tmp.cl"
calc_node_fgmcfg = "PSCDC-SYSTEM/system_program_cfg.cfg"

// "calc_node_cltmp"  接收到主机分配CL程序暂存文件
// "calc_node_fgmcfg" 接收到主机分配CL配置暂存文件

void NodeNetwork::start_node_server(const char* system_config);

// 全局节点信息, 节点状态 & 节点名称
uint32_t    NODE_GLOBAL::global_node_state;
std::string NODE_GLOBAL::global_node_name;

#define NODE_STATE_IDLE 0xA000 // 节点空闲中
#define NODE_STATE_DTOK 0xA001 // 节点接收数据完毕 (等待计算)
#define NODE_STATE_CALC 0xA002 // 节点计算中
#define NODE_STATE_WAIT 0xA003 // 节点等待中 (未提取结果)

// OpenCL 计算处理线程(ocl程序运行为避免阻塞主线程影响状态返回)
void WorkThread::Work_OpenCLexec();  // 从系统暂存文件加载计算 cl 程序
void WorkThread::Work_ThreadStart(); // 启动 OCL 处理子线程
void WorkThread::Work_ThreadExit();  // 退出 OCL 处理子线程
```
- 处理数据
```cpp
// 全局暂存数据
// 主机部署的数据会一直保留在内存
// 输入计算3D矩阵 & 输出结果3D矩阵
mocl_Matrix3D NodeCache::DataMatrix::TempCalcMatrix_input;
mocl_Matrix3D NodeCache::DataMatrix::TempCalcMatrix_output;

// 数据计算 cdc_opencl & cdc_maths_tool

// 加载配置 & 初始化 cdc_opencl
Config::loadconfig opencl_calc_config("PSCDC-SYSTEM/system_program_cfg.cfg");
OPENCL_CALC_MATRIX opencl_calc_matrix;

// 选择 OpenCL 计算平台设备, 默认 1,1
opencl_calc_matrix.CALC_type_device(
	(uint32_t)opencl_calc_config.find_type_double("device_type_p"),
	(uint32_t)opencl_calc_config.find_type_double("device_type_d")
);

// 传入 OpenCL 计算程序 .cl [kernel] & 函数名
opencl_calc_matrix.CLAC_init_opencl(
  	"PSCDC-SYSTEM/system_program_tmp.cl",
	opencl_calc_config.find_type_string("kernel_function_name").c_str()
);

// 设置 OpenCL 工作组大小, 测试 1024(32x32), 比例必须和矩阵 xy 相等			
opencl_calc_matrix.CALC_set_workgroup(
  	(size_t)opencl_calc_config.find_type_double("opencl_work_size_x"),
  	(size_t)opencl_calc_config.find_type_double("opencl_work_size_y")
);                     

// 单个源矩阵数据长度 length = mat.x * mat.y
size_t matrix_size =
  	(size_t)opencl_calc_config.find_type_double("opencl_matrix_xlen") * 
  	(size_t)opencl_calc_config.find_type_double("opencl_matrix_ylen");

// 设置 cdc_opencl 计算IO大小, 输入矩阵数量 & 输出矩阵数量 & 矩阵数据长度(单个)		
opencl_calc_matrix.CALC_set_computeIO(
  	(size_t)opencl_calc_config.find_type_double("opencl_matrix_input_num"),
  	(size_t)opencl_calc_config.find_type_double("opencl_matrix_output_num"),
	matrix_size
);

// 写入计算矩阵数据 Matrix3D
opencl_calc_matrix.CALC_write_matrix(NodeCache::DataMatrix::TempCalcMatrix_input);

// 读取计算结果矩阵数据 Matrix3D
NodeCache::DataMatrix::TempCalcMatrix_output =
  	opencl_calc_matrix.CALC_read_result();
```
- cdc_opencl 为 opencl 3.0 再次封装, 打包初始化和矩阵转换
- opencl 本身输出结果全部为 1D 数据 
```c
// OpenCL Kernel 参数设置根据输入到输出矩阵数量 依次排列
// 输入 global const float* 输出 global float*
__kernel void TestKernelBloom(global const float* MatR, global const float* MatG, global const float* MatB,
    global float* OutR, global float* OutG, global float* OutB)

// 从 1D 访问 2D 数组
int i = get_global_id(0);
int j = get_global_id(1);

int width  = get_global_size(0);
int height = get_global_size(1);
xxx[i * width + j];
```
整个计算节点处理为:

接收 string => vector float => Matrix3D => CALC => Matrix3D => vector float => string => 返回

这个只是写着玩, 不懂分布式计算, 大佬勿喷.

2023-08-11 12:47:00 RCSZ.

