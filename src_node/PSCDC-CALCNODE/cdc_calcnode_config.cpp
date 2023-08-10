// cdc_calcnode_config.
#include <fstream>
#include <iomanip>

#include "cdc_calcnode.hpp"

using namespace std;

// Time [xxxx.xx.xx.xx:xx:xx:xx ms].
string __get_current_time() {
	auto timenow = chrono::system_clock::now();
	auto timemillisecond = chrono::duration_cast<chrono::milliseconds>(timenow.time_since_epoch()) % 1000;
	auto t = chrono::system_clock::to_time_t(timenow);
	tm _time = {};
#ifdef _WIN32
	localtime_s(&_time, &t);
#else
	localtime_r(&t, &_time);
#endif
	stringstream _sstream;
	_sstream << put_time(&_time, "[%Y.%m.%d %H:%M:%S") << " " << setfill('0') << setw(3)
		<< timemillisecond.count() << " ms]";

	return _sstream.str();
}

namespace Config {
	// read config.
	loadconfig::loadconfig(const string& xcore_configfile) {
		ifstream read_config(xcore_configfile);

		if (!read_config.is_open()) {
			// open file failed.
			module_state = CFG_STATE_FAILED_FILE;
		}
		else {
			string read_config_line = {};
			size_t config_lines = NULL;
			config_filepath = xcore_configfile;

			while (getline(read_config, read_config_line)) {
				// delete head,Tail '\t'.
				read_config_line.erase(0, read_config_line.find_first_not_of(" \t"));
				read_config_line.erase(read_config_line.find_last_not_of(" \t") + 1);

				if (read_config_line.empty() || read_config_line[0] == '#')
					continue;

				// decode key_value.
				vector<string> _config_parts = {};
				istringstream Iss(read_config_line);
				string _pushpart = {};

				bool str_quotes = false; // Flag.
				while (Iss >> _pushpart) {

					// push string data.
					if (str_quotes)
						_config_parts.back() += " " + _pushpart;
					else {
						// delete str'"x"'
						if (_pushpart.front() == '"' && _pushpart.back() == '"') {
							_pushpart = _pushpart.substr(1, _pushpart.size() - 2);
						}
						_config_parts.push_back(_pushpart);
					}

					if (_pushpart.front() == '"' && !str_quotes) {
						// enter str'"'.
						str_quotes = true;
						_pushpart = _pushpart.substr(1);
					}

					if (_pushpart.back() == '"' && str_quotes) {
						// exit str'"'.
						str_quotes = false;
						_pushpart.pop_back();
					}
				}

				if (_config_parts[1] != "=") {
					module_state = CFG_STATE_FAILED_LINE;
					continue;
				}

				if (_config_parts[2].front() == '"' && _config_parts[2].back() == '"')
					_config_parts[2] = _config_parts[2].substr(1, _config_parts[2].size() - 2);

				// save_data => unordered_map.
				config_hashmap[_config_parts[0]] = _config_parts[2];
				config_lines++;
			}
		}
		read_config.close();
	}

	string loadconfig::find_type_string(const string& find_key) {
		auto it = config_hashmap.find(find_key);
		if (it == config_hashmap.end()) {

			module_state = CFG_STATE_FAILED_MATCH;
			return "";
		}
		else
			return it->second;
	}

	double loadconfig::find_type_double(const string& find_key) {
		auto it = config_hashmap.find(find_key);
		if (it == config_hashmap.end()) {

			module_state = CFG_STATE_FAILED_MATCH;
			return 0.0;
		}
		else
			return stod(it->second);
	}

	bool loadconfig::modify_list_value(const string& key, const string& modify_value) {
		auto it = config_hashmap.find(key);
		if (it == config_hashmap.end()) {

			module_state = CFG_STATE_FAILED_MATCH;
			return false;
		}
		else {
			it->second = modify_value;
			return true;
		}
	}

	bool loadconfig::write_configfile() {
		// open, clear.
		ofstream write_config(config_filepath, ios::trunc);
		bool return_stateflag = true;

		if (!write_config.is_open() || (config_filepath == "__null")) {

			module_state = CFG_STATE_FAILED_FILE;
			return_stateflag = false;
		}
		else {
			// overwrite write.
			for (auto it = config_hashmap.begin(); it != config_hashmap.end(); it++)
				write_config << it->first << " = " << '"' << it->second << '"' << endl;
		}
		write_config.close();

		return return_stateflag;
	}
}