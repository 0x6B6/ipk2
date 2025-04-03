#include "tcp.hpp"
#include "protocol.hpp"
#include "message.hpp"
#include "error.hpp"

#include <asm-generic/socket.h>
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>

TCP::TCP(Config& config) : Protocol(config) {}

TCP::~TCP() {
	log("TCP BYE");
	
	disconnect();
	shutdown(socket_fd, SHUT_RDWR); // ???
}

int TCP::connect() {
	log("Connecting TCP");
	
	if (::connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0 && errno != EINPROGRESS) {
		local_error("TCP connect()");
		exit(1);
		return 1;
	}

	return 0;
}

int TCP::send(std::string msg) {
	log("Sending TCP");

	int b_tx = ::send(socket_fd, msg.c_str(), msg.length(), 0);

	if (b_tx < 0) {
		local_error("TCP send()");
		return 1;
	}

	return 0;
}

int TCP::receive() {
	log("Receiving TCP");

	int b_rx = recv(socket_fd, buffer, 2048, 0);

	if (b_rx <= 0) {
		local_error("TCP receive()");
		return 1;
	}

	return 0;
}

int get_msg_content(std::istringstream& msgs, std::string& msg_content) {
	std::string content;

	std::getline(msgs >> std::ws, msg_content);

	if (msg_content.back() == '\r') {
		msg_content.pop_back();
	}

	if (msg_content.empty() || valid_printable_msg(msg_content) == false) {
		local_error("Message contains invalid characters");
		return 1;
	}

	return 0;
}

int TCP::process(Response& response) {
	std::string msg(buffer);
	std::istringstream msgs(msg);					// message string stream
	std::string msg_type, component;				// Message type & tokens
	std::string dname, status, msg_content, error;	// Message content

	msgs >> msg_type;

	/* ERR FROM {DisplayName} IS {MessageContent}\r\n */
	if (msg_type == "ERR") {
		msgs >> component;

		if (component != "FROM") {
			return 1;
		}

		msgs >> dname;

		if (dname.empty() || valid_printable(dname) == false) {
			return 1;
		}

		msgs >> component;

		if (component != "IS") {
			return 1;
		}

		if (get_msg_content(msgs, msg_content)) {
			return 1;
		}

		response.content = "ERROR FROM " + dname + ": " + msg_content;
		response.type = MsgType::ERR;
	}
	/* REPLY {"OK"|"NOK"} IS {MessageContent}\r\n */
	else if (msg_type == "REPLY") {
		msgs >> status; // OK | NOK

		msgs >> component; // IS

		if (component != "IS") {
			return 1;
		}

		if (get_msg_content(msgs, msg_content)) {
			return 1;
		}

		if (status == "OK") {
			response.status = ResponseStatus::OK;
			response.content = "Action Success: " + msg_content;
		}
		else if (status == "NOK") {
			response.status = ResponseStatus::NOK;
			response.content = "Action Failure: " + msg_content;
		}
		else {
			return 1;
		}

		response.type = MsgType::REPLY;
	}
	/* MSG FROM {DisplayName} IS {MessageContent}\r\n */
	else if (msg_type == "MSG") {
		msgs >> component;

		if (component != "FROM") {
			return 1;
		}

		msgs >> dname;

		if (dname.empty() || valid_printable(dname) == false) {
			return 1;
		}

		msgs >> component;

		if (component != "IS") {
			return 1;
		}

		if (get_msg_content(msgs, msg_content)) {
			return 1;
		}

		response.content = dname + ": " + msg_content;
		response.type = MsgType::MSG;
	}
	/* BYE FROM {DisplayName}\r\n */
	else if (msg_type == "BYE") {

		msgs >> component;

		if (component != "FROM") {
			return 1;
		}

		msgs >> dname;

		if (dname.empty() || valid_printable(dname) == false) {
			return 1;
		}

		response.type = MsgType::BYE;
	}
	else {
		local_error("Invalid TCP server messsage");
		return MESSAGE_ERROR;
	}

	return 0;
}

int TCP::error(std::string error) {
	/* Client side error */
	if (send(error)) {
		return 1;
	}

	return 0;
}

int TCP::disconnect() {
	/* Disconnect only when when communication has started */
	if (client_r->get_state() == Client::State::OPEN) {
		if (send(msg_factory->create_bye_msg(client_r->get_name()))) {
			return 1;
		}
	}

	return 0;
}