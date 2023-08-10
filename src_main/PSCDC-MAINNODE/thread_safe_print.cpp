// thread_safe_print.
#include "thread_safe_print.hpp"

using namespace std;

mutex print_mutex = {};

void safe_logprint(string logstr) {
    {
        lock_guard<mutex> lock(print_mutex);
        cout << logstr << endl;
    }
}