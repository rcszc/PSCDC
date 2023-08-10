// cdc_data_file.

#ifndef _CDC_DATA_FILE_H
#define _CDC_DATA_FILE_H

struct DataMatrix3D {

	float*** data;

	size_t mat_xlength;
	size_t mat_ylength;
	size_t mat_zlength;
};

// Create Matrix 3D. (N * 2D).
DataMatrix3D DataMatrix3DCreate(size_t matrix_mum, size_t xlength, size_t ylength);
// Free Matrix 3D.
size_t       DataMatrix3DFree(DataMatrix3D matrix);

namespace MainDataFile {

	DataMatrix3D ReadFloatDataFile(const char* File);
	bool WriteFloatDataFile(const char* File, DataMatrix3D Data);
	/*
	* ≤‚ ‘¥Ú”°æÿ’Û.
	* y.length < max, z.length < max.
	*/
	void TestPrintMatrix(DataMatrix3D mat, size_t max = 32);
}

#endif