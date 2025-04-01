#include "tcp.hpp"
#include "message.hpp"
#include "protocol.hpp"

#include <asm-generic/socket.h>
#include <cerrno>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <sys/socket.h>

TCP::TCP(Config& config) : Protocol(config) {
}

TCP::~TCP() {
	std::cout << "TCP BYE" << std::endl;
	disconnect();
	shutdown(socket_fd, SHUT_RDWR); // ???
}

int TCP::connect() {
	std::cout << "Connecting TCP\n";
	
	if (::connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0/* && errno != EINPROGRESS*/) {
		std::cerr << "ERROR: TCP connect()" << std::endl;
		exit(1);
		return 1;
	}

	return 0;
}

int TCP::send(std::string msg) {
	std::cout << "Sending TCP\n";
	int b_tx = ::send(socket_fd, msg.c_str(), msg.length(), 0);

	if (b_tx < 0) {
		std::cerr << "ERROR: TCP send()" << std::endl;
		return 1;
	}

	return 0;
}

int TCP::receive(std::string msg) {
	std::cout << "Receiving TCP\n";

	uint8_t buffer[2048];

	int b_rx = recv(socket_fd, buffer, sizeof(buffer), 0);

	if (b_rx <= 0) {
		std::cerr << "ERROR: TCP recv()" << std::endl;
		return 1;
	}

	return 0;
}

int TCP::process(std::string msg) {
	std::istringstream msgs(msg); // message string stream
	std::string msg_type, component, content;

	msgs >> msg_type;

	std::cout << msg << std::endl;

	/* ERR FROM {DisplayName} IS {MessageContent}\r\n */
	if (msg_type == "ERR") {
		msgs >> component;

		if (component != "FROM") {
			return 1;
		}

		msgs >> component;

		if (component.empty() || valid_printable(component) == false) {
			return 1;
		}

		content = "ERROR FROM " + component + ":";

		msgs >> component;

		if (component != "IS") {
			return 1;
		}

		msgs >> component;

		if (component.empty()) {
			return 1;
		}
	}
	/* REPLY {"OK"|"NOK"} IS {MessageContent}\r\n */
	else if (msg_type == "REPLY") {
		std::cout << "REPLY message";
	}
	/* MSG FROM {DisplayName} IS {MessageContent}\r\n */
	else if (msg_type == "MSG") {
		std::cout << "MSG message";
	}
	/* BYE FROM {DisplayName}\r\n */
	else if (msg_type == "BYE") {
		std::cout << "BYE message";
	}
	else {
		std::cerr << "error: invalid messsage" << std::endl;
		return 1;
	}

	return 0;
}

int TCP::error(std::string error) {
	if (send(msg_factory->create_err_msg(client_r->get_name(), error))) {
		return 1;
	}

	return 0;
}

int TCP::disconnect() {
	/* Disconnect only when when communication has started */
	if (client_r->get_state() != Client::State::START) {
		if (send(msg_factory->create_bye_msg(client_r->get_name()))) {
			return 1;
		}
	}

	return 0;
}