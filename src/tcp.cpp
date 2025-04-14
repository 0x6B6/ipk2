#include "tcp.hpp"
#include "protocol.hpp"
#include "message.hpp"
#include "error.hpp"

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>

TCP::TCP(Config& config) : Protocol(config) {}

TCP::~TCP() {
	shutdown(socket_fd, SHUT_RDWR);
}

int TCP::connect() {	
	if (::connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0 && errno != EINPROGRESS) {
		local_error("TCP connect()");
		return NETWORK_ERROR;
	}

	return SUCCESS;
}

int TCP::send(std::string msg) {
	int b_tx = ::send(socket_fd, msg.c_str(), msg.length(), 0);

	if (b_tx < 0) {
		local_error("TCP send()");
		return NETWORK_ERROR;
	}

	return SUCCESS;
}

int TCP::receive() {
	int b_rx = recv(socket_fd, buffer, 2048, 0);

	if (b_rx <= 0) {
		local_error("TCP receive()");
		return NETWORK_ERROR;
	}

	return SUCCESS;
}

int get_msg_content(std::istringstream& msgs, std::string& msg_content) {
	std::string content;

	std::getline(msgs >> std::ws, msg_content);

	if (msg_content.back() == '\r') {
		msg_content.pop_back();
	}

	if (msg_content.empty() || valid_printable_msg(msg_content) == false) {
		local_error("Message contains invalid characters");
		return MESSAGE_ERROR;
	}

	return SUCCESS;
}

int TCP::process(Response& response) {
	std::string msg(buffer);
	std::istringstream msgs(msg);					// message string stream
	std::string msg_type, component;				// Message type & tokens
	std::string dname, status, msg_content, error;	// Message content
	// segmentation logic here
	msgs >> msg_type;

	/* ERR FROM {DisplayName} IS {MessageContent}\r\n */
	if (str_up(msg_type) == "ERR") {
		msgs >> component;

		if (str_up(component) != "FROM") {
			return MESSAGE_ERROR;
		}

		msgs >> dname;

		if (dname.empty() || valid_printable(dname) == false) {
			return MESSAGE_ERROR;
		}

		msgs >> component;

		if (str_up(component) != "IS") {
			return MESSAGE_ERROR;
		}

		if (get_msg_content(msgs, msg_content)) {
			return MESSAGE_ERROR;
		}

		response.content = "ERROR FROM " + dname + ": " + msg_content;
		response.type = MsgType::ERR;
	}
	/* REPLY {"OK"|"NOK"} IS {MessageContent}\r\n */
	else if (str_up(msg_type) == "REPLY") {
		msgs >> status; // OK | NOK

		msgs >> component; // IS

		if (str_up(component) != "IS") {
			return MESSAGE_ERROR;
		}

		if (get_msg_content(msgs, msg_content)) {
			return MESSAGE_ERROR;
		}

		if (str_up(status) == "OK") {
			response.status = ResponseStatus::OK;
			response.content = "Action Success: " + msg_content;
		}
		else if (str_up(status) == "NOK") {
			response.status = ResponseStatus::NOK;
			response.content = "Action Failure: " + msg_content;
		}
		else {
			return MESSAGE_ERROR;
		}

		response.type = MsgType::REPLY;
	}
	/* MSG FROM {DisplayName} IS {MessageContent}\r\n */
	else if (str_up(msg_type) == "MSG") {
		msgs >> component;

		if (str_up(component) != "FROM") {
			return MESSAGE_ERROR;
		}

		msgs >> dname;

		if (dname.empty() || valid_printable(dname) == false) {
			return MESSAGE_ERROR;
		}

		msgs >> component;

		if (str_up(component) != "IS") {
			return MESSAGE_ERROR;
		}

		if (get_msg_content(msgs, msg_content)) {
			return MESSAGE_ERROR;
		}

		response.content = dname + ": " + msg_content;
		response.type = MsgType::MSG;
	}
	/* BYE FROM {DisplayName}\r\n */
	else if (str_up(msg_type) == "BYE") {

		msgs >> component;

		if (str_up(component) != "FROM") {
			return MESSAGE_ERROR;
		}

		msgs >> dname;

		if (dname.empty() || valid_printable(dname) == false) {
			return MESSAGE_ERROR;
		}

		response.type = MsgType::BYE;
	}
	else {
		local_error("Invalid TCP server messsage");
		return MESSAGE_ERROR;
	}

	return SUCCESS;
}

int TCP::error(std::string error) {
	if (send(error)) {
		return NETWORK_ERROR;
	}

	return SUCCESS;
}

int TCP::disconnect(std::string id) {
	if (send(msg_factory->create_bye_msg(id))) {
		return NETWORK_ERROR;
	}

	return SUCCESS;
}