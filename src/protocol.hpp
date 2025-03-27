#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory>

#include "config.hpp"
#include "msg_factory.hpp"

class Protocol {
	public:
		Protocol(Config &config);
		~Protocol();

		static std::unique_ptr<Protocol> protocol_setup(Config &config);
		MsgFactory& get_msg_factory();
		/*
		virtual int connect() = 0;
		virtual int send() = 0;
		virtual int receive() = 0;
		virtual int process() = 0;
		virtual int disconnect() = 0;
		*/
		int create_socket();
		int get_address(const char* ip_hname);
		void to_string();
	
	protected:
		Config::Protocol protocol_type;
		std::unique_ptr<MsgFactory> msg_factory;

		int socket_fd;
		int socket_type;

		struct sockaddr_in server_address;
		uint16_t dyn_port;
};