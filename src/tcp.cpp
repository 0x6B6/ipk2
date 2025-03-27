#include "tcp.hpp"
#include "protocol.hpp"

#include <iostream>
#include <sys/socket.h>

TCP::TCP(Config& config) : Protocol(config) {
}

int TCP::connect() {
	std::cout << "Connecting TCP\n"; 

	return 0;
}