#include "udp.hpp"
#include "message.hpp"
#include "protocol.hpp"

#include <cstdint>
#include <iostream>
#include <sys/socket.h>

UDP::UDP(Config& config)
	: Protocol(config)
	, message_id{0}
	, retransmission{config.udp_retransmission}
	, timeout{config.udp_timeout} {
}

UDP::~UDP() {
	std::cout << "UDP BYE" << std::endl;
	disconnect();
}

int UDP::connect() {return 0;} // Empty

int send(std::string msg); // Real send
int bind_msg_id(uint16_t msg_id); // MessageID

int UDP::send(std::string msg) {

	do {
		int b_tx = sendto(socket_fd, msg.c_str(), msg.length(), 0, (struct sockaddr *) &server_address, sizeof(server_address));

		if (b_tx < 0) {
			std::cerr << "ERROR: UDP send()" << std::endl;
			return 1;
		}

		// if response then exitt

	} while(retransmission--);

	return 0;
}

int UDP::receive(std::string msg) {
	//recvfrom()

	uint8_t buffer[2048];

	struct sockaddr_in src {};
	socklen_t addr_len;

	int b_rx = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &src, &addr_len);

	if (b_rx <= 0) {
		std::cerr << "ERROR: UDP recvfrom()" << std::endl;
		return 1;
	}

	return 0;
}

int UDP::process(std::string msg) {
	// switch case
	return 0;
}

int UDP::error(std::string error) {
	return 0;
}

int UDP::disconnect() {
	// bye 
	return 0;
}

int UDP::confirm(uint16_t message_id) {
	std::string confirm_msg = std::string(1, MsgType::CONFIRM) + std::string(2, ntohs(message_id));

	return 0;
}