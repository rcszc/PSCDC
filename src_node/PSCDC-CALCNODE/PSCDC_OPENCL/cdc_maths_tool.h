// cdc_maths_tool. [MATHS-TOOL]
// 老项目 Maths OpenCL 改.
// RCSZ. cpp17, x64.

#ifndef _CDC_MATHS_TOOL_H
#define _CDC_MATHS_TOOL_H
#include <vector>
#include <chrono>

typedef float*   mocl_mat1ptr;
typedef float**  mocl_mat2ptr;
typedef float*** mocl_mat3ptr;

struct mocl_Matrix1D {

	mocl_mat1ptr data;

	size_t mat_xlength;
};

struct mocl_Matrix2D {

	mocl_mat2ptr data;

	size_t mat_xlength;
	size_t mat_ylength;
};

struct mocl_Matrix3D {

	mocl_mat3ptr data;

	size_t mat_xlength;
	size_t mat_ylength;
	size_t mat_zlength;
};

struct mocl_MatrixImgRGB {

	mocl_mat2ptr dataR;
	mocl_mat2ptr dataG;
	mocl_mat2ptr dataB;

	size_t mat_width;
	size_t mat_height;
};

// mocl timer ms.
// 结果单位 [毫秒], 精度 [微秒].
class COUNT_TIME {
protected:
	std::chrono::steady_clock::time_point start_time = {};

public:
	void   Timing_Start();
	double Timing_Ended();
};

// Maths function.

size_t MOCL_Matrix1Free(mocl_Matrix1D matrix);
size_t MOCL_Matrix2Free(mocl_Matrix2D matrix);
size_t MOCL_Matrix3Free(mocl_Matrix3D matrix);
size_t MOCL_MatrixImgFree(mocl_MatrixImgRGB matrix);

mocl_Matrix2D     MOCL_Matrix3toMatrix2(mocl_Matrix3D matrix);
mocl_Matrix1D     MOCL_Matrix2toMatrix1(mocl_Matrix2D matrix);
mocl_Matrix2D     MOCL_Matrix1toMatrix2(mocl_Matrix1D matrix, size_t xlen, size_t ylen);

mocl_Matrix3D     MOCL_MatrixImgtoMatrix3(mocl_MatrixImgRGB matrix);
mocl_MatrixImgRGB MOCL_Matrix3toMatrixImg(mocl_Matrix3D matrix);

mocl_Matrix2D     MOCL_CreateMatrix1(size_t Matrix_Num, size_t xlength);
mocl_Matrix3D     MOCL_CreateMatrix2(size_t Matrix_Num, size_t xlength, size_t ylength);
mocl_MatrixImgRGB MOCL_CreateMatrixImg(size_t width, size_t height);

mocl_MatrixImgRGB MOCL_MirrorRotateX(mocl_MatrixImgRGB matrix);
mocl_MatrixImgRGB MOCL_MirrorRotateY(mocl_MatrixImgRGB matrix);

mocl_MatrixImgRGB MOCL_CopyMatrixImg(mocl_MatrixImgRGB matrix);

// 矩阵内存释放.
#define MOCL_TOOL_FREEMATRIX1D  MOCL_Matrix1Free    // Free matrix1D ( delete[] data, length.set = NULL ) return free.size
#define MOCL_TOOL_FREEMATRIX2D  MOCL_Matrix2Free    // Free matrix2D ( delete[] data, length.set = NULL ) return free.size
#define MOCL_TOOL_FREEMATRIX3D  MOCL_Matrix3Free    // Free matrix3D ( delete[] data, length.set = NULL ) return free.size
#define MOCL_TOOL_FREEMATRIXIMG MOCL_MatrixImgFree  // Free matrixImg ( delete[] data R G B, length.set = NULL ) return free.size

// 矩阵数据转换.
#define MOCL_TOOL_3DTO2D  MOCL_Matrix3toMatrix2  // ( in_matrix_3d ) return matrix_2d
#define MOCL_TOOL_2DTO1D  MOCL_Matrix2toMatrix1  // ( in_matrix_2d ) return matrix_1d
#define MOCL_TOOL_1DTO2D  MOCL_Matrix1toMatrix2  // ( in_matrix_1d, matrix_2d_x_length, matrix_2d_y_length ) return matrix_2d

#define MOCL_TOOL_IMGTO3D MOCL_MatrixImgtoMatrix3  // ( in_matrix_ImgRGB ) return matrix_3d
#define MOCL_TOOL_3DTOIMG MOCL_Matrix3toMatrixImg  // ( in_matrix_3d ) return in_matrix_ImgRGB

// 矩阵内存创建.
#define MOCL_TOOL_NEWMATRIX1D  MOCL_CreateMatrix1    // ( create_matrix_number, matrix_1d_x_length ) return matrix_2d
#define MOCL_TOOL_NEWMATRIX2D  MOCL_CreateMatrix2    // ( create_matrix_number, matrix_2d_x_length, matrix_2d_y_length ) return matrix_3d
#define MOCL_TOOL_NEWMATRIXIMG MOCL_CreateMatrixImg  // ( matrix_width, matrix_height ) return matrix_ImgRGB

// 矩阵操作.
#define MOCL_TOOL_ROTATEMIRX MOCL_MirrorRotateX  // x.镜像旋转 ( in_matrix_ImgRGB ) return in_matrix_ImgRGB
#define MOCL_TOOL_ROTATEMIRY MOCL_MirrorRotateY  // y.镜像旋转 ( in_matrix_ImgRGB ) return in_matrix_ImgRGB

#define MOCL_TOOL_ROTATECOPY MOCL_CopyMatrixImg  // copy ( in_matrix_ImgRGB ) return in_matrix_ImgRGB

// system func.[core]
void mocl_sys_Matrix1ToMatrix3(mocl_Matrix3D mat, size_t matcount, mocl_Matrix1D matin, size_t x, size_t y);

#endif