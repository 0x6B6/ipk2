#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory>

#include "client.hpp"
#include "config.hpp"
#include "msg_factory.hpp"

class Client;

class Protocol {
	public:
		Protocol(Config &config);
		
		static std::unique_ptr<Protocol> protocol_setup(Config &config);
		
		MsgFactory& get_msg_factory();
		int create_socket();
		int get_socket();
		int get_address(const char* ip_hname);
		int bind_client(Client* client);
		void to_string();

		virtual ~Protocol();
		virtual int connect() = 0;
		virtual int send(std::string msg) = 0;
		virtual int receive(std::string msg) = 0;
		virtual int process(std::string msg) = 0;
		virtual int error(std::string err) = 0;
		virtual int disconnect() = 0;
	
	protected:
		Config::Protocol protocol_type;
		Client* client_r;
		std::unique_ptr<MsgFactory> msg_factory;

		int socket_fd;
		int socket_type;

		struct sockaddr_in server_address;
		uint16_t dyn_port;

		int await_response(int expected);
};