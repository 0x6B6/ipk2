#pragma once

#include <iostream>

enum Error : int {
	SUCCESS = 0,
	PARSE_ERROR = 10,
	CLIENT_ERROR = 20,
	PROTOCOL_ERROR = 30,
	MESSAGE_ERROR = 31,
	NETWORK_ERROR = 40,
	ADDRESS_ERROR = 41,
	TIMEOUT_ERROR = 50,
	GENERAL_ERROR = 60,
	SERVER_ERROR = 80
};

inline void local_error(const std::string& msg) {
	std::cout << "ERROR: " << msg << std::endl;
}

inline void log(const std::string& msg) {
	std::cerr << "\033[36m[LOG]\033[0m " << msg << std::endl;
}