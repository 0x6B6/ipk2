#include <iostream>

#include "args.hpp"
#include "config.hpp"
#include "client.hpp"

int main(int argc, char **argv) {
	Config config;

	/* Parse program parameters */
	if (args_parse(argc, argv, config)) {
		std::cerr << "error: argument parsing failed" << std::endl;
		return 1;
	}

	/* Select and setup given protocol */
	auto protocol = Protocol::protocol_setup(config);

	if (protocol == nullptr) {
		std::cerr << "error: protocol setup failed" << std::endl;
		return 1;
	}

	/* Launch and run client */
	Client client(std::move(protocol));

	if (client.client_run()) {
			std::cerr << "error: client runtime" << std::endl;
			return 1;
	}

	return 0;
}