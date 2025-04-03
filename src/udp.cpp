#include "udp.hpp"
#include "client.hpp"
#include "error.hpp"
#include "message.hpp"
#include "protocol.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <sys/socket.h>

UDP::UDP(Config& config)
	: Protocol(config)
	, message_id{0}
	, retransmission{config.udp_retransmission}
	, udp_timeout{config.udp_timeout} {}

UDP::~UDP() {
	log("[UDP] BYE");
	disconnect();
}

/* Empty, UDP has no connection */
int UDP::connect() { 
	log("[UDP] connect()");
	return SUCCESS; 
}

/* MessageID assignment */
void bind_msg_id(std::string& msg, uint16_t msg_id) {
	uint16_t big_end_msg_id = htons(msg_id);

	if (msg.length() >= 3) {
		msg[1] = static_cast<char>(big_end_msg_id & 0xFF);
		msg[2] = static_cast<char>((big_end_msg_id >> 8) & 0xFF);
	}
}

/* Directly send once */
int UDP::direct_send(std::string msg) {
	int b_tx = sendto(socket_fd, msg.c_str(), msg.length(), 0, (struct sockaddr *) &server_address, sizeof(server_address));

	if (b_tx < 0) {
		local_error("[UDP] send()");
		return NETWORK_ERROR;
	}

	return SUCCESS;
}

/* UDP retry send */
int UDP::send(std::string msg) {
	int retransmission = UDP::retransmission, await_result;
	Response response = {};
	bind_msg_id(msg, message_id);

	do {
		if (direct_send(msg)) {
			return NETWORK_ERROR;
		}

		if ((await_result = await_response(udp_timeout, MsgType::CONFIRM, response)) == SUCCESS) {
			log("[UDP] confirmed");
			break;
		}

	} while(retransmission--);

	if (await_result != SUCCESS) {
		return NETWORK_ERROR;
	}

	++message_id;

	return SUCCESS;
}

/* UDP receive */
int UDP::receive() {
	struct sockaddr_in src {};
	socklen_t addr_len;

	b_rx = recvfrom(socket_fd, buffer, 2048, 0, (struct sockaddr *) &src, &addr_len);

	/* Verify IP address */
	if (memcmp(&server_address.sin_addr, &src.sin_addr, sizeof(struct in_addr)) != 0) {
		local_error("[UDP] received packet from wrong IPv4 address");
		return ADDRESS_ERROR;
	}

	/* Acquirement of dynamic port */
	server_address.sin_port = src.sin_port;

	if (b_rx < 0) {
		local_error("[UDP] receive()");
		return 1;
	}

	log("Bytes rx: " + std::to_string(b_rx));

	return 0;
}

/* Extract ID from a message */
uint16_t get_msg_id(char* buffer) {
	uint16_t msg_id = *reinterpret_cast<uint16_t*>(buffer);

	return ntohs(msg_id);
}

int get_msg_content(char* msg_bp, std::string& content) {
	std::string dname, msg_content;

	dname = std::string(msg_bp);
	
	msg_bp += dname.length() + 1;

	msg_content = std::string(msg_bp);

	if (dname.empty() || msg_content.empty()) {
		local_error("Message contents empty");
		return MESSAGE_ERROR;
	}

	content = dname + ": " + msg_content;

	if (valid_printable_msg(msg_content) == false) {
		local_error("Message contains invalid characters");
		return MESSAGE_ERROR;
	}

	return SUCCESS;
}

/* Process response message */
int UDP::process(Response& response) {
	uint8_t msg_type;
	uint16_t ref_message_id, server_msg_id;
	std::string msg_content;

	log("[UDP] processing server message");
	for (int i = 0; i < b_rx; ++i) std::cout << "[" << buffer[i] << (int) buffer[i] << "] "; std::cout << std::endl;

	/* Minimum response size of 3 bytes */
	if (b_rx < 3) {
		local_error("UDP message integrity");
		return PROTOCOL_ERROR;
	}

	msg_type = buffer[0];
	server_msg_id = get_msg_id(buffer + 1);

	log("Proccessed_msg_id: " + std::to_string(processed_msg_id) + " Server_msg_id: " + std::to_string(server_msg_id));

	/* Confirm message arrival */
	if (msg_type != CONFIRM) {
		if (confirm(server_msg_id)) {
			return NETWORK_ERROR;
		}
		/* This message has already been processed */
		if (server_msg_id <= processed_msg_id) {
			response.duplicate = true;
			log("Duplicate detected");
			return SUCCESS;
		}

		processed_msg_id = server_msg_id;
	}

	switch (msg_type) {
		case CONFIRM: { log("CONFIRM");
			ref_message_id = get_msg_id(buffer + 1);

			if (ref_message_id != message_id) {
				local_error("Confirm response to invalid client message ID");
				return PROTOCOL_ERROR;
			}

			response.type = CONFIRM;
			break;
		}

		case REPLY: { log("REPLY");
			int result = buffer[3];
			ref_message_id = get_msg_id(buffer + 4);

			if (b_rx < 5 || result < 0 || result > 1) {
				local_error("Invalid reply result or integrity");
				return PROTOCOL_ERROR;
			}

			response.status = result ? OK : NOK;

			std::string content(buffer + 6);

			response.content = (result ? "Action Success: " : "Action Failure: ") + content;

			response.type = REPLY;
			break;
		}

		case MSG: { log("MSG");
			if (get_msg_content(buffer + 3, msg_content)) {
				return MESSAGE_ERROR;
			}

			response.content = msg_content;
			response.type = MSG;
			break;
		}

		case ERR: { log("ERR");
			if (get_msg_content(buffer + 3, msg_content)) {
				return MESSAGE_ERROR;
			}

			response.content = msg_content;
			response.type = ERR;
			break;
		}

		case BYE: { log("BYE");
			response.type = BYE;
			break;
		}

		case PING: { log("PING");
			response.type = PING;
			break;
		}

		default: { log("INVALID MESSAGE TYPE");
			local_error("Invalid UDP server message: " + std::to_string(msg_type));
			response.type = UNKNOWN;
			break;
		}
	}

	log("[UDP] succesfuly processed message");

	return SUCCESS;
}

/* UDP error */
int UDP::error(std::string error) {
	if (send(error)) {
		return PROTOCOL_ERROR;
	}

	log("[UDP] error()");

	return SUCCESS;
}

/* UDP bye */
int UDP::disconnect() {
	if (client_r->get_state() != Client::State::START) {
		if (send(msg_factory->create_bye_msg(client_r->get_name()))) {
			return PROTOCOL_ERROR;
		}

		log("[UDP] disconnect()");
	}

	return SUCCESS;
}

/* UDP confirm, TODO move partialy to msg factory */
int UDP::confirm(uint16_t message_id) {
	std::string confirm_msg = {MsgType::CONFIRM, 0, 0};

	bind_msg_id(confirm_msg, message_id);

	for (int i = 0; i < confirm_msg.length(); ++i) std::cout << "[" << (int) confirm_msg[i] << "] ";

	if (direct_send(confirm_msg)) {
		return PROTOCOL_ERROR;
	}

	log("[UDP] sending confirm()");

	return SUCCESS;
}