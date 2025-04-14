#pragma once

#include <iostream>

/* Enum containing various exit/return codes */
enum Error : int {
	SUCCESS = 0,         // Succesful function/method exit 
	PARSE_ERROR = 10,    // Argument parsing error
	CLIENT_ERROR = 20,   // Client application error 
	PROTOCOL_ERROR = 30, // IPK25-CHAT protocol error during communication
	MESSAGE_ERROR = 31,  // Malformed message
	NETWORK_ERROR = 40,  // Network error caused by the failure of transport protocol/socket functions 
	ADDRESS_ERROR = 41,  // Invalid server IPv4 address
	TIMEOUT = 50,        // Timeout of a reply/confirm packet
	GENERAL_ERROR = 60,  // General error - glibc functions failure
	SERVER_EXIT = 80     // BYE, ERR packet received from a server, used primarily for UDP 
};

/* Local client error */
inline void local_error(const std::string& msg) {
	std::cout << "ERROR: " << msg << std::endl;
}

/* Log function, used for debugging purposes */
inline void log(const std::string& msg) {
	std::cerr << "\033[36m[LOG]\033[0m " << msg << std::endl;
}