#include <cstdint>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include "args.hpp"
#include "config.hpp"

char* get_arg(int argc, char** argv, int idx) {
	if (idx + 1 >= argc) {
		return nullptr;
	}

	return argv[idx + 1];
}

int to_int(char *arg) {
	int value;

	if (arg == nullptr) {
		std::cerr << "error: missing argument" << std::endl;
		exit(EXIT_FAILURE);
	}

	try {
		value = std::stoi(arg);
	} catch (const std::invalid_argument& e) {
		std::cerr << "error: invalid integer argument: "  << e.what() << " '" << arg << "'" << std::endl;
		exit(EXIT_FAILURE);
	} catch (const std::out_of_range& e) {
		std::cerr << "error: integer argument out of range: " << e.what() << " '" << arg << "'" << std::endl;
		exit(EXIT_FAILURE);
	}

	return value;
}

Config::Protocol parse_protocol(char* protocol) {
	if (protocol == nullptr)
		return Config::Protocol::UND;

	std::string prot_str(protocol);

	if (prot_str == "tcp") {
		return Config::Protocol::TCP;
	}
	else if (prot_str == "udp") {
		return Config::Protocol::UDP;
	}
	else {
		return Config::Protocol::UND;
	}
}

int in_range(int value, int max) {
	if (value < 0 || value > max) {
		return -1;
	}

	return value;
}

void help(char* pname) {
	std::cout	<< "Client for a chat server using IPK25 protocol\n\n"
				<< "Execution:\n"
				<<  pname << " [opts]\n\n"
				<< "Options:\n"
				<< "{-t} Transport protocol to be used for connection.\n"
				<< "{-s} Server IPv4 address | hostname.\n"
				<< "[-p] Server port, default = 4567.\n"
				<< "[-d] UDP confirmation timeout in milliseconds, default = 250 ms.\n"
				<< "[-r] Maximum number of UDP retransmissions, default = 3.\n"
				<< "[-h] Prints this help message and terminates the program.\n\n"
				<< "Mandatory parameters are inside curly brackets {}.\n"
				<< "Optional parameters are in square brackets []."
				<< std::endl;

	exit(EXIT_SUCCESS);
}

int args_parse(int argc, char** argv, Config& config) {
	using namespace std;

	if (argc < 2) {
		cerr << "error: invalid number of parameters" << endl;
		return 1;
	}

	char *pname = argv[0];
	int value;

	for (int i = 1; i < argc; ++i) {
		char *arg = get_arg(argc, argv, i);
		char *param = argv[i];

		if (param[0] == '-') {
			switch (param[1]) {
				case 't':
					config.protocol = parse_protocol(arg);
					break;

				case 's':
					config.ip_hostname = arg;
					break;

				case 'p':
					value = to_int(arg);

					if (in_range(value, UINT16_MAX) == -1) {
						cerr << "error: invalid argument range " << arg << endl;
						return 1;
					}

					config.server_port = value;
					break;

				case 'd':
					value = to_int(arg);

					if (in_range(value, UINT16_MAX) == -1) {
						cerr << "error: invalid argument range " << arg << endl;
						return 1;
					}

					config.udp_timeout = value;
					break;

				case 'r':
					value = to_int(arg);

					if (in_range(value, UINT8_MAX) == -1) {
						cerr << "error: invalid argument range " << arg << endl;
						return 1;
					}

					config.udp_retransmission = value;
					break;

				case 'h':
					help(pname);
					break;

				default:
					cerr << "error: invalid parameter " << param << endl;
					break;
			}
			++i;
		}
		else {
			cerr << "error: invalid parameter " << param << endl;
			return 1;
		}
	}

	if (config.protocol == Config::Protocol::UND) {
		cerr << "error: invalid protocol" << endl;
		return 1;
	}

	if (config.ip_hostname == nullptr) {
		cerr << "error: invalid server IPv4 address/hostname" << endl;
		return 1;
	}

	return 0;
}