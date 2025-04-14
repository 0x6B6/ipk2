#include "msg_factory.hpp"
#include "message.hpp"
#include <cstdint>

/**
 *	TCP Messages
 */

/* TCP AUTH message */
std::string TCPMsgFactory::create_auth_msg(std::string id, std::string dname, std::string secret) const {
	return "AUTH " + id + " AS " + dname + " USING " + secret + CRLF;
}

/* TCP JOIN message */
std::string TCPMsgFactory::create_join_msg(std::string id, std::string dname) const {
	return "JOIN " + id + " AS " + dname + CRLF;
}

/* TCP MSG message */
std::string TCPMsgFactory::create_chat_msg(std::string dname, std::string msg) const {
	return "MSG FROM " + dname + " IS " + msg + CRLF;
}

/* TCP ERR message */
std::string TCPMsgFactory::create_err_msg(std::string dname, std::string msg) const {
	return "ERR FROM " + dname + " IS " + msg + CRLF;
}

/* TCP BYE message */
std::string TCPMsgFactory::create_bye_msg(std::string dname) const {
	return "BYE FROM " + dname + CRLF;
}

/**
 *	UDP Messages
 */

/* UDP message serializer
 *
 * The serializer formats the IPK25 protocol message for the UDP transport layer protocol.
 * It is important to note that the MessageID is only set when sending,
 * for now it has a default value of zero.
 *
 *  1 byte       2 bytes           n bytes
 * +--------+--------+--------+-------~~------+---+
 * |  0x04  |    MessageID    |    Content    | 0 |
 * +--------+--------+--------+-------~~------+---+
 */
std::string udp_serialize(uint8_t msg_type, std::string content) {
	std::string msg = std::string(1, msg_type) + std::string(2, 0x00) + content;

	return msg;
}

/* UDP AUTH message */
std::string UDPMsgFactory::create_auth_msg(std::string id, std::string dname, std::string secret) const {
	std::string content = id + '\0' + dname + '\0' + secret + '\0';

	return udp_serialize(MsgType::AUTH, content);
}

/* UDP JOIN message */
std::string UDPMsgFactory::create_join_msg(std::string id, std::string dname) const {
	std::string content = id + '\0' + dname + '\0';

	return udp_serialize(MsgType::JOIN, content);
}

/* UDP MSG message */
std::string UDPMsgFactory::create_chat_msg(std::string dname, std::string msg) const {
	std::string content = dname + '\0' + msg + '\0';

	return udp_serialize(MsgType::MSG, content);
}

/* UDP ERR message */
std::string UDPMsgFactory::create_err_msg(std::string dname, std::string msg) const {
	std::string content = dname + '\0' + msg + '\0';

	return udp_serialize(MsgType::ERR, content);
}

/* UDP BYE message */
std::string UDPMsgFactory::create_bye_msg(std::string dname) const {
	std::string content = dname + '\0';

	return udp_serialize(MsgType::BYE, content);
}