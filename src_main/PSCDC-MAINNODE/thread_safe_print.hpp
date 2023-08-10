// thread_safe_print.

#ifndef _THREAD_SAFE_PRINT_HPP
#define _THREAD_SAFE_PRINT_HPP
#include <iostream>
#include <string>
#include <mutex>

void safe_logprint(std::string logstr);
#define LOGOUT safe_logprint

#endif