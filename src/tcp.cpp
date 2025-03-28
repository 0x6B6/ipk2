#include "tcp.hpp"
#include "protocol.hpp"

#include <iostream>
#include <sys/socket.h>

TCP::TCP(Config& config) : Protocol(config) {
}

TCP::~TCP() {
	std::cout << "TCP BYE" << std::endl;
}

int TCP::connect() {
	std::cout << "Connecting TCP\n"; 

	return 0;
}