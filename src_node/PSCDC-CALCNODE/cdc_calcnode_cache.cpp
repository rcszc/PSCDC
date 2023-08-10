// cdc_calcnode_cache.
#include <fstream>
#include <filesystem>
#include <bitset>

#include "cdc_calcnode.hpp"

using namespace std;

namespace NodeCache {

    // Encode float array To group string.
    string EncodeFAtoBS(const vector<float>& FloatArray) {

        string return_string = {};
        for (const auto& CpySrc : FloatArray) {

            uint32_t Ui32Temp = NULL;
            memcpy_s(&Ui32Temp, 4, &CpySrc, 4);

            return_string += to_string(Ui32Temp) + " ";
        }

        cout << TIEM_CURRENT << NODE_HEAD_CACHE
            << "Process Encode[vector->string] data.bytes: " << FloatArray.size() * 4 << endl;

        return return_string;
    }

    // Decode group string To float array.
    vector<float> DecodeBStoFA(const string& StringArray) {

        vector<float> return_array = {};
        istringstream DECISS(StringArray);

        string tempstr = {};
        while (DECISS >> tempstr) {
            uint32_t tempui32 = stoul(tempstr);
            float    tempfp32 = 0.0f;

            memcpy_s(&tempfp32, 4, &tempui32, 4);
            return_array.push_back(tempfp32);
        }

        cout << TIEM_CURRENT << NODE_HEAD_CACHE
            << "Process Decode[string->vector] data.bytes: " << StringArray.size() << endl;

        return return_array;
    }

    bool SystemTempFile(const char* Path, const string& FileStr, const string CMD) {
        bool return_state = true;

        // string => write => file.
        if (CMD == "write") {
            fstream OCLFILE(Path);

            if (filesystem::exists(Path)) {
                OCLFILE << FileStr;
            }
            else {
                cout << TIEM_CURRENT << NODE_HEAD_CACHE << "Failed openfile. Cmd: " << CMD << endl;
                return_state = false;
            }
            OCLFILE.close();
        }

        // clear => file.
        if (CMD == "clear") {

            if (filesystem::exists(Path)) {
                fstream OCLFILE(Path, ios::out | ios::trunc);
                OCLFILE.close();
            }
            else {
                cout << TIEM_CURRENT << NODE_HEAD_CACHE << "Failed openfile. Cmd: " << CMD << endl;
                return_state = false;
            }
        }
        return return_state;
    }

    mocl_Matrix3D DataProcessMatrix(std::string& StringArray, size_t x, size_t y, size_t z, bool& state) {
        COUNT_TIME s_timer;
        s_timer.Timing_Start();

        mocl_Matrix3D process_result = MOCL_TOOL_NEWMATRIX2D(x, y, z);
        vector<float> fp32datatmp = DecodeBStoFA(StringArray);

        size_t pcs_count = NULL;
        
        // Matrix1D => Matrix3D.
        for (size_t i = 0; i < x; ++i) {
            for (size_t j = 0; j < y; ++j) {
                for (size_t c = 0; c < z; ++c) {

                    process_result.data[i][j][c] = fp32datatmp[pcs_count];
                    if (pcs_count > fp32datatmp.size() - 1) {
                        
                        state = false;
                        cout << TIEM_CURRENT << NODE_HEAD_CACHE << "Failed data process matrix." << endl;
                        goto exitloop;
                    }
                    ++pcs_count;
                }
            }
        }
        cout << TIEM_CURRENT << NODE_HEAD_CACHE 
            << "Process [string->matrix] data.bytes: " << fp32datatmp.size() * 4 << endl;
        state = true;
    exitloop:

        cout << TIEM_CURRENT << NODE_HEAD_CACHE << "Process time: " << s_timer.Timing_Ended() << " ms" << endl;

        fp32datatmp.clear();
        fp32datatmp.shrink_to_fit();

        StringArray.clear();
        StringArray.shrink_to_fit();

        return process_result;
    }

    vector<float> DataProcessFP32array(mocl_Matrix3D InMatrix, bool& state) {
        COUNT_TIME s_timer;
        s_timer.Timing_Start();

        vector<float> process_result = {};
        process_result.resize(InMatrix.mat_xlength * InMatrix.mat_ylength * InMatrix.mat_zlength);

        size_t pcs_count = NULL;
        
        if (InMatrix.data != nullptr) {
            for (size_t i = 0; i < InMatrix.mat_xlength; ++i) {
                for (size_t j = 0; j < InMatrix.mat_ylength; ++j) {
                    for (size_t c = 0; c < InMatrix.mat_zlength; ++c) {

                        process_result[pcs_count] = InMatrix.data[i][j][c];
                        ++pcs_count;
                    }
                }
            }
            cout << TIEM_CURRENT << NODE_HEAD_CACHE
                << "Process [string->matrix] data.bytes: " << process_result.size() * 4 << endl;
        }
        else
            cout << TIEM_CURRENT << NODE_HEAD_CACHE << "Failed data process array." << endl;

        cout << TIEM_CURRENT << NODE_HEAD_CACHE << "Process time: " << s_timer.Timing_Ended() << " ms" << endl;

        MOCL_TOOL_FREEMATRIX3D(InMatrix); // free.
        InMatrix.data = nullptr;

        return process_result;
    }

    // global matrix data_cache.
    namespace DataMatrix {

        mocl_Matrix3D TempCalcMatrix_input = {};
        mocl_Matrix3D TempCalcMatrix_output = {};
    }
}