#pragma once

#include "client.hpp"
#include "config.hpp"
#include "message.hpp"
#include "msg_factory.hpp"

#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory>
#include <queue>

class Client; // declaration forwarding

/* Abstraction of protocol, 
 * encapsulates both transport layer and IPK25-CHAT protocols
 */
class Protocol {
	public:
		/* Create and setup protocol (static) class method */
		static std::unique_ptr<Protocol> protocol_setup(Config &config);
		
		/* Protocol utilities */
		Protocol(Config &config);
		void bind_client(Client* client);
		MsgFactory& get_msg_factory();
		int create_socket();
		void to_string();
		int get_socket();
		std::queue<Response>& get_msg_queue();
		int get_address(const char* ip_hname);


		/* Protocol AWAIT response method in request states */
		int await_response(uint16_t timeout, int expected, Response& response);

		/* Virtual methods, implemented by concrete protocols */
		virtual ~Protocol();
		virtual int connect() = 0;
		virtual int send(std::string msg) = 0;
		virtual int receive() = 0;
		virtual int process(Response& response) = 0;
		virtual int error(std::string err) = 0;
		virtual int disconnect(std::string id) = 0;

	protected:
		/* Type of used protocol */
		Config::Protocol protocol_type;

		/* Message factory */
		std::unique_ptr<MsgFactory> msg_factory;

		/* Socket info */
		int socket_fd;
		int socket_type;

		/* Server address & port info */
		struct sockaddr_in server_address;
		uint16_t dyn_port;

		/* Reply timeout, 5000 milliseconds */
		const uint16_t timeout = 5000;

		/* Receive buffer */
		char buffer[2048];
		int b_rx;

		/* Message queue */
		std::queue<Response> msg_queue;
};