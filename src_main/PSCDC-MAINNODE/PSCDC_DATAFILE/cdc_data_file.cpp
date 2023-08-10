// cdc_data_file.
#include <fstream>
#include <sstream>

#include "cdc_data_file.h"
#include "../thread_safe_print.hpp"

using namespace std;
#define CDC_DATA_LOGHEAD "[DataFile]: "

DataMatrix3D DataMatrix3DCreate(size_t matrix_num, size_t xlength, size_t ylength) {
    DataMatrix3D return_matrix = {};

    return_matrix.mat_xlength = matrix_num;
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

size_t DataMatrix3DFree(DataMatrix3D matrix) {
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
        LOGOUT("matrix.free mat.data nullptr.");

    matrix.data = nullptr;
    matrix.mat_xlength = NULL;
    matrix.mat_ylength = NULL;
    matrix.mat_zlength = NULL;

    return free_size;
}

namespace MainDataFile {

    DataMatrix3D ReadFloatDataFile(const char* File) {

        string datatemp_head = {}, dataline = {}, param = {};
        size_t matrix_num = NULL, matrix_x = NULL, matrix_y = NULL;

        DataMatrix3D return_mat = {};
        fstream src_data(File);

        if (src_data) {
            LOGOUT(CDC_DATA_LOGHEAD + string("Open read datafile: ") + File);

            getline(src_data, datatemp_head);
            istringstream HeadInfo(datatemp_head);

            HeadInfo >> param;
            if (param == "$INFO") {

                HeadInfo >> param; matrix_num = stoull(param); // matrix number
                HeadInfo >> param; matrix_x   = stoull(param); // matrix x length
                HeadInfo >> param; matrix_y   = stoull(param); // matrix y length

                // create result matrix.
                return_mat = DataMatrix3DCreate(matrix_num, matrix_x, matrix_y);

                size_t datacount[2] = {}; // 0:number, 1:xlength.
                while (getline(src_data, dataline)) {

                    istringstream DATA(dataline);
                    
                    if (dataline != "matrix_end") {
                        if ((datacount[1] >= matrix_x) || (datacount[0] >= matrix_num)) {

                            LOGOUT(CDC_DATA_LOGHEAD + string("Error matrix param."));
                            break;
                        }
                        for (size_t i = 0; i < matrix_y; ++i) {

                            DATA >> param;
                            return_mat.data[datacount[0]][datacount[1]][i] = stof(param);
                        }
                        ++datacount[1];
                    }
                    else {
                        datacount[1] = NULL;
                        ++datacount[0];
                    }
                }
                LOGOUT(CDC_DATA_LOGHEAD + string("Read data matrix: ")
                    + to_string(return_mat.mat_xlength) + "x" 
                    + to_string(return_mat.mat_ylength) + "x" 
                    + to_string(return_mat.mat_zlength));
            }
            else
                LOGOUT(CDC_DATA_LOGHEAD + string("Error filehead param."));
        }
        else
            LOGOUT(CDC_DATA_LOGHEAD + string("Failed read openfile: ") + File);
        src_data.close();

        return return_mat;
	}

    bool WriteFloatDataFile(const char* File, DataMatrix3D Data) {

        fstream src_data(File, ios::out | ios::trunc);
        if (src_data) {
            LOGOUT(CDC_DATA_LOGHEAD + string("Open write datafile: ") + File);

            src_data << "$INFO" << " " << Data.mat_xlength << " " << Data.mat_ylength << " " << Data.mat_zlength << endl;
            
            for (size_t i = 0; i < Data.mat_xlength; ++i) {
                for (size_t j = 0; j < Data.mat_ylength; ++j) {
                    for (size_t c = 0; c < Data.mat_zlength; ++c) {

                        src_data << Data.data[i][j][c] << " ";
                    }
                    src_data << endl;
                }
                src_data << "matrix_end" << endl;
            }

            LOGOUT(CDC_DATA_LOGHEAD + string("Write file matrix: ")
                + to_string(Data.mat_xlength) + "x"
                + to_string(Data.mat_ylength) + "x"
                + to_string(Data.mat_zlength));
            return true;
        }
        else {
            LOGOUT(CDC_DATA_LOGHEAD + string("Failed write openfile: ") + File);
            return false;
        }
    }

    void TestPrintMatrix(DataMatrix3D mat, size_t max) {

        if ((max > mat.mat_ylength) && (max > mat.mat_zlength)) {
            for (size_t i = 0; i < mat.mat_xlength; ++i) {

                cout << "Matrix(" << i << "):" << endl;

                for (size_t j = 0; j < mat.mat_ylength; ++j) {
                    for (size_t c = 0; c < mat.mat_zlength; ++c) {

                        cout << mat.data[i][j][c] << " ";
                    }
                    cout << endl;
                }
            }
        }
        else
            cout << "TestPrint.Filter " << mat.mat_xlength << "x" << mat.mat_ylength << "x" << mat.mat_zlength << endl;
    }
}