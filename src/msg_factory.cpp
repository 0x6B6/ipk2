#include "msg_factory.hpp"
#include <iostream>

constexpr const char* CRLF = "\r\n";

/**
 *	TCP Messages
 */
std::string TCPMsgFactory::create_auth_msg(std::string id, std::string dname, std::string secret) const {
	return "AUTH " + id + " AS " + dname + " USING " + secret + CRLF;
}

std::string TCPMsgFactory::create_join_msg(std::string id, std::string dname) const {
	return "JOIN " + id + " AS " + dname + CRLF;
}

std::string TCPMsgFactory::create_chat_msg(std::string dname, std::string content) const {
	return "MSG FROM " + dname + " IS " + content + CRLF;
}

std::string TCPMsgFactory::create_err_msg(std::string dname, std::string content) const {
	return "ERR FROM " + dname + " IS " + content + CRLF;
}

std::string TCPMsgFactory::create_bye_msg(std::string dname) const {
	return "BYE FROM " + dname + CRLF;
}

/**
 *	UDP Messages
 */

void print_udp_msg(std::string msg) {
	for (int i = 0; i < msg.length(); ++i) {
		char c = msg[i];
		uint8_t x = c;
		std::cout << c << "(" << x << ")";
	}
}

std::string UDPMsgFactory::create_auth_msg(std::string id, std::string dname, std::string secret) const {
	std::string content = id + '\0' + dname + '\0' + secret + '\0';

	std::string auth_msg = std::string(1, MsgType::AUTH) + std::string(2, 0x00) + content; 

	//print_udp_msg(auth_msg);

	return "hello";
}

std::string UDPMsgFactory::create_join_msg(std::string id, std::string dname) const {
	return "UDP JOIN CID AS DNAME";
}

std::string UDPMsgFactory::create_chat_msg(std::string dname, std::string content) const {
	return "UDP MSG FROM DNAME IS CONTENT";
}

std::string UDPMsgFactory::create_err_msg(std::string dname, std::string content) const {
	return "UDP ERR FROM DNAME IS CONTENT";
}

std::string UDPMsgFactory::create_bye_msg(std::string dname) const {
	return "UDP BYE FROM DNAME";
}
