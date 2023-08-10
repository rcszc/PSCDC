// cdc_maths_tool.
#include <iostream>

#include "cdc_maths_tool.h"

using namespace std;

void COUNT_TIME::Timing_Start() {
	start_time = chrono::steady_clock::now();
}

// Time End.
double COUNT_TIME::Timing_Ended() {
	// start - end.
	int64_t result_microseconds = 
		chrono::duration_cast<chrono::microseconds>(
			chrono::steady_clock::now() - start_time
		).count();

	return double(result_microseconds / 1000.0);
}

bool test_matrixrgb_nullptr(mocl_MatrixImgRGB in) {
	if ((in.dataR != nullptr) && (in.dataG != nullptr) && (in.dataB != nullptr))
		return false;
	else
		return true;
}

// ************************ Matrix Free ************************

size_t MOCL_Matrix1Free(mocl_Matrix1D matrix) {
	size_t free_size = NULL;

	if (matrix.data != nullptr) {
		
		free_size = matrix.mat_xlength;
		delete[] matrix.data;
	}
	else
		cout << "MOPENCL Tool: Free Mat1.data nullptr." << endl;

	matrix.data = nullptr;
	matrix.mat_xlength = NULL;

	return free_size;
}

size_t MOCL_Matrix2Free(mocl_Matrix2D matrix) {
	size_t free_size = NULL;

	if (matrix.data != nullptr) {
		
		free_size = matrix.mat_xlength * matrix.mat_ylength;

		for (size_t i = 0; i < matrix.mat_xlength; i++)
			delete[] matrix.data[i];
		delete[] matrix.data;
	}
	else
		cout << "MOPENCL Tool: Free Mat2.data nullptr." << endl;

	matrix.data = nullptr;
	matrix.mat_xlength = NULL;
	matrix.mat_ylength = NULL;

	return free_size;
}

size_t MOCL_Matrix3Free(mocl_Matrix3D matrix) {
	size_t free_size = NULL;

	if (matrix.data != nullptr) {
		
		free_size = matrix.mat_xlength * matrix.mat_ylength * matrix.mat_zlength;

		for (size_t i = 0; i < matrix.mat_xlength; i++)
			for (size_t j = 0; j < matrix.mat_ylength; j++)
				delete[] matrix.data[i][j];

		for (size_t i = 0; i < matrix.mat_xlength; i++)
			delete[] matrix.data[i];
		delete[] matrix.data;
	}
	else
		cout << "MOPENCL Tool: Free Mat3.data nullptr." << endl;

	matrix.data = nullptr;
	matrix.mat_xlength = NULL;
	matrix.mat_ylength = NULL;
	matrix.mat_zlength = NULL;

	return free_size;
}

size_t MOCL_MatrixImgFree(mocl_MatrixImgRGB matrix) {
	size_t free_size = NULL;

	if (!test_matrixrgb_nullptr(matrix)) {
		
		free_size = matrix.mat_width * matrix.mat_height * 3;
		for (size_t i = 0; i < matrix.mat_width; i++) {

			delete[] matrix.dataR[i];
			delete[] matrix.dataG[i];
			delete[] matrix.dataB[i];
		}
		delete[] matrix.dataR;
		delete[] matrix.dataG;
		delete[] matrix.dataB;

		matrix.dataR = nullptr;
		matrix.dataG = nullptr;
		matrix.dataB = nullptr;
	}
	else {
		if (matrix.dataR != nullptr) cout << "MOPENCL Tool: Free MatImg.data.r nullptr." << endl;
		if (matrix.dataG != nullptr) cout << "MOPENCL Tool: Free MatImg.data.g nullptr." << endl;
		if (matrix.dataB != nullptr) cout << "MOPENCL Tool: Free MatImg.data.b nullptr." << endl;
	}

	matrix.mat_width = NULL;
	matrix.mat_height = NULL;

	return free_size;
}

// ************************  Matrix Convert ************************

mocl_Matrix2D MOCL_Matrix3toMatrix2(mocl_Matrix3D matrix) {
	mocl_Matrix2D return_matrix = {};
	size_t _count = NULL;

	if (matrix.data != nullptr) {
		return_matrix = MOCL_TOOL_NEWMATRIX1D(matrix.mat_xlength, matrix.mat_ylength * matrix.mat_zlength);

		for (size_t i = 0; i < matrix.mat_xlength; i++) {
			for (size_t j = 0; j < matrix.mat_ylength; j++) {
				for (size_t n = 0; n < matrix.mat_zlength; n++) {

					return_matrix.data[i][_count] = matrix.data[i][j][n];
					_count++;
				}
			}
			_count = NULL;
		}
	}
	else
		cout << "MOPENCL Tool: Mat3 to Mat2 in.nullptr." << endl;
	MOCL_TOOL_FREEMATRIX3D(matrix);

	return return_matrix;
}

mocl_Matrix1D MOCL_Matrix2toMatrix1(mocl_Matrix2D matrix) {
	mocl_Matrix1D return_matrix = {};
	size_t _count = NULL;

	if (matrix.data != nullptr) {

		return_matrix.mat_xlength = size_t(matrix.mat_xlength * matrix.mat_ylength);
		return_matrix.data = new float[return_matrix.mat_xlength];

		for (size_t i = 0; i < matrix.mat_xlength; i++) {
			for (size_t j = 0; j < matrix.mat_ylength; j++) {

				return_matrix.data[_count] = matrix.data[i][j];
				_count++;
			}
		}
	}
	else
		cout << "MOPENCL Tool: Mat2 to Mat1 in.nullptr." << endl;
	MOCL_TOOL_FREEMATRIX2D(matrix);

	return return_matrix;
}

mocl_Matrix2D MOCL_Matrix1toMatrix2(mocl_Matrix1D matrix, size_t xlen, size_t ylen) {
	mocl_Matrix2D return_matrix = {};
	size_t _count = NULL;

	if (matrix.data != nullptr) {
		return_matrix = MOCL_TOOL_NEWMATRIX1D(xlen, ylen);

		for (size_t i = 0; i < xlen; i++) {
			for (size_t j = 0; j < ylen; j++) {

				if (_count < matrix.mat_xlength)
					return_matrix.data[i][j] = matrix.data[_count];
				else {
					cout << "MOPENCL Tool: Mat1 to Mat2 length > limit." << endl;
					exit(-1);
				}
				_count++;
			}
		}
	}
	else
		cout << "MOPENCL Tool: Mat1 to Mat2 in.nullptr." << endl;

	delete[] matrix.data;

	matrix.data = nullptr;
	matrix.mat_xlength = NULL;

	return return_matrix;
}

mocl_Matrix3D MOCL_MatrixImgtoMatrix3(mocl_MatrixImgRGB matrix) {
	mocl_Matrix3D return_matrix = {};

	if (!test_matrixrgb_nullptr(matrix)) {
		return_matrix = MOCL_TOOL_NEWMATRIX2D(3, matrix.mat_width, matrix.mat_height);

		for (size_t i = 0; i < matrix.mat_width; i++) {
			for (size_t j = 0; j < matrix.mat_height; j++) {

				return_matrix.data[0][i][j] = matrix.dataR[i][j];
				return_matrix.data[1][i][j] = matrix.dataG[i][j];
				return_matrix.data[2][i][j] = matrix.dataB[i][j];
			}
		}
	}
	else
		cout << "MOPENCL Tool: MatImg to Mat3 in.nullptr." << endl;
	MOCL_TOOL_FREEMATRIXIMG(matrix);

	return return_matrix;
}

mocl_MatrixImgRGB MOCL_Matrix3toMatrixImg(mocl_Matrix3D matrix) {
	mocl_MatrixImgRGB return_matrix = {};

	if (matrix.data != nullptr) {
		return_matrix = MOCL_TOOL_NEWMATRIXIMG(matrix.mat_ylength, matrix.mat_zlength);

		for (size_t i = 0; i < matrix.mat_ylength; i++) {
			for (size_t j = 0; j < matrix.mat_zlength; j++) {

				return_matrix.dataR[i][j] = matrix.data[0][i][j];
				return_matrix.dataG[i][j] = matrix.data[1][i][j];
				return_matrix.dataB[i][j] = matrix.data[2][i][j];
			}
		}
	}
	else
		cout << "MOPENCL Tool: Mat3 to MatImg in.nullptr." << endl;
	MOCL_TOOL_FREEMATRIX3D(matrix);

	return return_matrix;
}

// ************************  Matrix Create ************************

mocl_Matrix2D MOCL_CreateMatrix1(size_t Matrix_Num, size_t xlength) {
	mocl_Matrix2D return_matrix = {};

	return_matrix.mat_xlength = Matrix_Num;
	return_matrix.mat_ylength = xlength;

	return_matrix.data = new float* [return_matrix.mat_xlength];
	for (size_t i = 0; i < return_matrix.mat_xlength; i++)
		return_matrix.data[i] = new float[return_matrix.mat_ylength];

	return return_matrix;
}

mocl_Matrix3D MOCL_CreateMatrix2(size_t Matrix_Num, size_t xlength, size_t ylength) {
	mocl_Matrix3D return_matrix = {};

	return_matrix.mat_xlength = Matrix_Num;
	return_matrix.mat_ylength = xlength;
	return_matrix.mat_zlength = ylength;

	return_matrix.data = new float** [return_matrix.mat_xlength];
	for (size_t i = 0; i < return_matrix.mat_xlength; i++)
		return_matrix.data[i] = new float* [return_matrix.mat_ylength];

	for (size_t i = 0; i < return_matrix.mat_xlength; i++)
		for (size_t j = 0; j < return_matrix.mat_ylength; j++)
			return_matrix.data[i][j] = new float[return_matrix.mat_zlength];

	return return_matrix;
}

mocl_MatrixImgRGB MOCL_CreateMatrixImg(size_t width, size_t height) {
	mocl_MatrixImgRGB return_matrix = {};

	return_matrix.mat_width = width;
	return_matrix.mat_height = height;

	return_matrix.dataR = new float* [return_matrix.mat_width];
	return_matrix.dataG = new float* [return_matrix.mat_width];
	return_matrix.dataB = new float* [return_matrix.mat_width];

	for (size_t i = 0; i < return_matrix.mat_width; i++) {
		return_matrix.dataR[i] = new float[return_matrix.mat_height];
		return_matrix.dataG[i] = new float[return_matrix.mat_height];
		return_matrix.dataB[i] = new float[return_matrix.mat_height];
	}

	for (size_t i = 0; i < return_matrix.mat_width; i++) {
		for (size_t j = 0; j < return_matrix.mat_height; j++) {

			return_matrix.dataR[i][j] = 0.0f;
			return_matrix.dataG[i][j] = 0.0f;
			return_matrix.dataB[i][j] = 0.0f;
		}
	}

	return return_matrix;
}

// ************************ Image Matrix processing ************************
// MartixImg x rotate.
mocl_MatrixImgRGB MOCL_MirrorRotateX(mocl_MatrixImgRGB matrix) {
	mocl_MatrixImgRGB result_mat = MOCL_TOOL_NEWMATRIXIMG(matrix.mat_width, matrix.mat_height);

	if (!test_matrixrgb_nullptr(matrix)) {
		size_t widthCount = matrix.mat_width - 1;

		for (size_t i = 0; i < matrix.mat_width; i++) {
			for (size_t j = 0; j < matrix.mat_height; j++) {

				result_mat.dataR[i][j] = matrix.dataR[widthCount][j];
				result_mat.dataG[i][j] = matrix.dataG[widthCount][j];
				result_mat.dataB[i][j] = matrix.dataB[widthCount][j];
			}
			widthCount--;
		}
	}
	else
		cout << "MOPENCL Tool: MirrorRotate.x in.nullptr." << endl;
	MOCL_TOOL_FREEMATRIXIMG(matrix);

	return result_mat;
}

// MartixImg y rotate.
mocl_MatrixImgRGB MOCL_MirrorRotateY(mocl_MatrixImgRGB matrix) {
	mocl_MatrixImgRGB result_mat = MOCL_TOOL_NEWMATRIXIMG(matrix.mat_width, matrix.mat_height);

	if (!test_matrixrgb_nullptr(matrix)) {
		size_t heightCount = matrix.mat_height - 1;

		for (size_t i = 0; i < matrix.mat_width; i++) {
			for (size_t j = 0; j < matrix.mat_height; j++) {

				result_mat.dataR[i][j] = matrix.dataR[heightCount][j];
				result_mat.dataG[i][j] = matrix.dataG[heightCount][j];
				result_mat.dataB[i][j] = matrix.dataB[heightCount][j];
				heightCount--;
			}
			heightCount = matrix.mat_height - 1;
		}
	}
	else
		cout << "MOPENCL Tool: MirrorRotate.y in.nullptr." << endl;
	MOCL_TOOL_FREEMATRIXIMG(matrix);

	return result_mat;
}

// MartixImg cpoy.
mocl_MatrixImgRGB MOCL_CopyMatrixImg(mocl_MatrixImgRGB matrix) {
	mocl_MatrixImgRGB return_matrix = {};

	if (!test_matrixrgb_nullptr(matrix)) {
		return_matrix = MOCL_TOOL_NEWMATRIXIMG(matrix.mat_width, matrix.mat_height);

		for (size_t i = 0; i < matrix.mat_width; i++) {
			for (size_t j = 0; j < matrix.mat_height; j++) {

				return_matrix.dataR[i][j] = matrix.dataR[i][j];
				return_matrix.dataG[i][j] = matrix.dataG[i][j];
				return_matrix.dataB[i][j] = matrix.dataB[i][j];
			}
		}
	}
	else
		cout << "MOPENCL Tool: CopyMatImg in.nullptr" << endl;

	return return_matrix;
}

// Matrix1D => Matrix3D.
void mocl_sys_Matrix1ToMatrix3(mocl_Matrix3D mat, size_t matcount, mocl_Matrix1D matin, size_t x, size_t y) {
	size_t _count = NULL;

	if ((mat.data != nullptr) && (matin.data != nullptr)) {

		for (size_t i = 0; i < x; i++) {
			for (size_t j = 0; j < y; j++) {

				mat.data[matcount][i][j] = matin.data[_count];
				_count++;
			}
		}
	}
	else {
		cout << "MOPENCL Tool: system func 'mocl_sys_Matrix1ToMatrix3'." << endl;
		exit(-1);
	}
}