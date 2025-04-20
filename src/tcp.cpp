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

/* TCP constructor */
TCP::TCP(Config& config) : Protocol(config) {}

/* Shutdown socket & connection properly */
TCP::~TCP() {
	shutdown(socket_fd, SHUT_RDWR);
}

/* TCP connect - perform a TCP handshake with server */
int TCP::connect() {	
	if (::connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0 && errno != EINPROGRESS) {
		local_error("TCP connect()");
		return NETWORK_ERROR;
	}

	return SUCCESS;
}

/* TCP send - sends any defined type of message */
int TCP::send(std::string msg) {
	int b_tx = ::send(socket_fd, msg.c_str(), msg.length(), 0);

	if (b_tx < 0) {
		local_error("TCP send()");
		return NETWORK_ERROR;
	}

	return SUCCESS;
}

/* TCP receive a message into a buffer */
int TCP::receive() {
	b_rx = recv(socket_fd, buffer + (segmentation ? offset_segment : 0), sizeof(buffer), 0);

	if (b_rx <= 0) {
		local_error("TCP receive()");
		return NETWORK_ERROR;
	}

	return SUCCESS;
}

/* Extract TCP message content */
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

	if (msg_content.length() > MAX_MSG_LEN) {
		local_error("Message is too long");
		return MESSAGE_ERROR;
	}

	return SUCCESS;
}

/* Perform a check if the message contains CRLF end marker */
bool crlf(std::string msg) {
	return msg.find(CRLF) != std::string::npos; 
}

/* TCP message parse and process function */
int TCP::process(Response& response) {
	/* Temporary parse variables */
	std::string msg(buffer, segmentation ? offset_segment + b_rx : b_rx); // Message buffer of string type, size depends on segmentation 
	std::istringstream msgs(msg);                                         // Message string stream
	std::string msg_type, component;                                      // Message type & tokens
	std::string dname, status, msg_content, error;                        // Message content
	
	/* Segmantation/Fragmentation protection */
	if (crlf(msg)) {
		segmentation = false;
		response.incomplete = false;
		offset_segment = 0;

	}
	else {
		segmentation = true;
		response.incomplete = true;
		offset_segment += b_rx;
		
		return SUCCESS;
	}

	/* Get message type */
	msgs >> msg_type;

	/* ERR FROM {DisplayName} IS {MessageContent}\r\n */
	if (str_up(msg_type) == "ERR") {
		/* FROM */
		msgs >> component;

		if (str_up(component) != "FROM") {
			return MESSAGE_ERROR;
		}

		/* DisplayName */
		msgs >> dname;

		if (dname.empty() || valid_printable(dname) == false || dname.length() > MAX_DN_LEN) {
			return MESSAGE_ERROR;
		}

		/* IS */
		msgs >> component;

		if (str_up(component) != "IS") {
			return MESSAGE_ERROR;
		}

		/* MessageContent */
		if (get_msg_content(msgs, msg_content)) {
			return MESSAGE_ERROR;
		}

		/* Process parsed ERR message */
		response.content = "ERROR FROM " + dname + ": " + msg_content;

		response.type = MsgType::ERR;
	}
	/* REPLY {"OK"|"NOK"} IS {MessageContent}\r\n */
	else if (str_up(msg_type) == "REPLY") {
		/* REPLY result OK | NOK */
		msgs >> status;

		/* IS */
		msgs >> component;

		if (str_up(component) != "IS") {
			return MESSAGE_ERROR;
		}

		/* MessageContent */
		if (get_msg_content(msgs, msg_content)) {
			return MESSAGE_ERROR;
		}

		/* Evaluate REPLY result */
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

		/* Process parsed REPLY message */
		response.type = MsgType::REPLY;
	}
	/* MSG FROM {DisplayName} IS {MessageContent}\r\n */
	else if (str_up(msg_type) == "MSG") {
		/* FROM */
		msgs >> component;

		if (str_up(component) != "FROM") {
			return MESSAGE_ERROR;
		}

		/* DisplayName */
		msgs >> dname;

		if (dname.empty() || valid_printable(dname) == false || dname.length() > MAX_DN_LEN) {
			return MESSAGE_ERROR;
		}

		/* IS */
		msgs >> component;

		if (str_up(component) != "IS") {
			return MESSAGE_ERROR;
		}

		/* MessageContent */
		if (get_msg_content(msgs, msg_content)) {
			return MESSAGE_ERROR;
		}

		/* Process parsed MSG message */
		response.content = dname + ": " + msg_content;
		
		response.type = MsgType::MSG;
	}
	/* BYE FROM {DisplayName}\r\n */
	else if (str_up(msg_type) == "BYE") {
		/* FROM */
		msgs >> component;

		if (str_up(component) != "FROM") {
			return MESSAGE_ERROR;
		}

		/* DisplayName */
		msgs >> dname;

		if (dname.empty() || valid_printable(dname) == false || dname.length() > MAX_DN_LEN) {
			return MESSAGE_ERROR;
		}

		/* Process parsed BYE message */
		response.type = MsgType::BYE;
	}
	/* Invalid message type */
	else {
		local_error("Invalid TCP server messsage");

		return MESSAGE_ERROR;
	}

	return SUCCESS;
}

/* TCP error message send function */
int TCP::error(std::string error) {
	if (send(error)) {
		return NETWORK_ERROR;
	}

	return SUCCESS;
}

/* TCP direct disconnect function, send BYE message  */
int TCP::disconnect(std::string id) {
	if (send(msg_factory->create_bye_msg(id))) {
		return NETWORK_ERROR;
	}

	return SUCCESS;
}