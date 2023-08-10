// cdc_mainnode_net_data.
#include <sstream>

#include "cdc_mainnode_net.h"

using namespace std;

namespace CALC_DATA_INDEX {
    queue<string> TaskDataFile = {};
}

namespace CalcDataProcess {

    // Encode float array To group string.
    string EncodeFAtoBS(const vector<float>& FloatArray) {

        string return_string = {};
        for (const auto& CpySrc : FloatArray) {

            uint32_t Ui32Temp = NULL;
            memcpy_s(&Ui32Temp, 4, &CpySrc, 4);

            return_string += to_string(Ui32Temp) + " ";
        }
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
        return return_array;
    }

    DataMatrix3D DataProcessMatrix(string& StringArray, size_t x, size_t y, size_t z, bool& state) {
        
        DataMatrix3D process_result = DataMatrix3DCreate(x, y, z);
        vector<float> fp32datatmp = DecodeBStoFA(StringArray);

        size_t pcs_count = NULL;

        // Matrix1D => Matrix3D.
        for (size_t i = 0; i < x; ++i) {
            for (size_t j = 0; j < y; ++j) {
                for (size_t c = 0; c < z; ++c) {

                    process_result.data[i][j][c] = fp32datatmp[pcs_count];
                    if (pcs_count > fp32datatmp.size() - 1) {

                        state = false;
                        LOGOUT(NETWORK_INFOHEAD + string("Failed data process matrix."));
                        goto exitloop;
                    }
                    ++pcs_count;
                }
            }
        }
        LOGOUT(NETWORK_INFOHEAD + string("Process [string->matrix] data.bytes: ") + to_string(fp32datatmp.size() * 4));
        state = true;
    exitloop:

        fp32datatmp.clear();
        fp32datatmp.shrink_to_fit();

        StringArray.clear();
        StringArray.shrink_to_fit();

        return process_result;
    }

    vector<float> DataProcessFP32array(DataMatrix3D InMatrix, bool& state) {

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
            LOGOUT(NETWORK_INFOHEAD + string("Process [mat->str] data.bytes: ") + to_string(process_result.size() * 4));
            state = true;
        }
        else {
            LOGOUT(NETWORK_INFOHEAD + string("Failed data process array."));
            state = false;
        }

        DataMatrix3DFree(InMatrix); // free.
        return process_result;
    }
}