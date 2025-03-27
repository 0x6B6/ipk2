#pragma once

#include <cstdint>
#include <string>

/* UDP message type */
enum MsgType : uint8_t {
	CONFIRM,	// UDP only
	REPLY,		// Server response O 
	AUTH,		// Request message
	JOIN,		// Request message
	MSG,		// Client chat message
	PING,		// UDP only, server query message
	ERR,		// Two-way communication error
	BYE			// Two-way communication
};

class MsgFactory {
	public:
		virtual ~MsgFactory(){};
		virtual std::string create_auth_msg(std::string id, std::string dname, std::string secret)	const = 0;
		virtual std::string create_join_msg(std::string id, std::string dname)	const = 0;
		virtual std::string create_chat_msg(std::string dname, std::string content)	const = 0;
		virtual std::string create_err_msg(std::string dname, std::string content)	const = 0;
		virtual std::string create_bye_msg(std::string dname)	const = 0;
};

class TCPMsgFactory : public MsgFactory {
	public:
		std::string create_auth_msg(std::string id, std::string dname, std::string secret)	const override;
		std::string create_join_msg(std::string id, std::string dname)	const override;
		std::string create_chat_msg(std::string dname, std::string content)	const override;
		std::string create_err_msg(std::string dname, std::string content)	const override;
		std::string create_bye_msg(std::string dname)	const override;
};

class UDPMsgFactory : public MsgFactory {
	public:
		std::string create_auth_msg(std::string id, std::string dname, std::string secret)	const override;
		std::string create_join_msg(std::string id, std::string dname)	const override;
		std::string create_chat_msg(std::string dname, std::string content)	const override;
		std::string create_err_msg(std::string dname, std::string content)	const override;
		std::string create_bye_msg(std::string dname)	const override;
};