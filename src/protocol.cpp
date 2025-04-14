#include "protocol.hpp"
#include "tcp.hpp"
#include "udp.hpp"
#include "error.hpp"
#include "config.hpp"
#include "message.hpp"
#include "msg_factory.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <memory>
#include <string>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

Protocol::Protocol(Config& config) : protocol_type{config.protocol}, socket_fd{-1}, dyn_port{config.server_port},
	b_rx{0} {
	if (protocol_type == Config::Protocol::TCP) {
		socket_type = SOCK_STREAM;
	}
	else {
		socket_type = SOCK_DGRAM;
	}

	std::memset(&server_address, 0, sizeof(server_address));
	std::memset(buffer, 0, sizeof(buffer));
}

Protocol::~Protocol() {
	close(socket_fd);
}

std::unique_ptr<Protocol> Protocol::protocol_setup(Config& config) {
	std::unique_ptr<Protocol> protocol;

	try {
		/* Select transport protocol to be used */
		switch (config.protocol) {  
			case Config::Protocol::TCP:
				protocol = std::make_unique<TCP>(config);	// TCP transport protocol
				protocol->msg_factory = std::make_unique<TCPMsgFactory>(); // Message factory for TCP protocol
				break;
			
			case Config::Protocol::UDP: 
				protocol = std::make_unique<UDP>(config);	// UDP transport protocol
				protocol->msg_factory = std::make_unique<UDPMsgFactory>();	// Message factory for UDP protocol
				break;

			case Config::Protocol::UND:
			default:
				local_error("Undefined transport protocol type");
				return nullptr;
				break;
		}
	} catch (const std::exception& e) {
		local_error(std::string("Caught memory exception - ") + e.what());
		return nullptr;
	}

	if (protocol->create_socket() != 0) {
		local_error("Failed to create a socket");
		return nullptr;
	}

	if (protocol->get_address(config.ip_hostname) != 0) {
		local_error("Failed to retrieve server address");
		return nullptr;
	}

	return protocol;
}

MsgFactory& Protocol::get_msg_factory() {
	return *msg_factory.get();
}

std::queue<Response>& Protocol::get_msg_queue() {
	return msg_queue;
}

int Protocol::create_socket() {
	/* Create socket */
	int fd = socket(AF_INET, socket_type, 0);

	if (fd < 0) {
		local_error("socket()");
		return -1;
	}

	/* Set Non-blocking Network I/O */
	int flags = fcntl(fd, F_GETFL, 0);

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		close(fd);
		local_error(" socket()");
		return -1;
	}

	socket_fd  = fd;

	return 0;
}

int Protocol::get_address(const char* ip_hname) {
	struct addrinfo *addrinfo, hints{};

	/* IPv4 and protocol filter */
	hints.ai_family = AF_INET;
	hints.ai_socktype = socket_type;

	/* Get address information */
	if (getaddrinfo(ip_hname, std::to_string(dyn_port).c_str(), &hints, &addrinfo) != 0) {
		local_error(std::string("Unable to get host (") + std::string(ip_hname) + ") address information");
		return 1;
	}

	server_address = *reinterpret_cast<struct sockaddr_in*>(addrinfo->ai_addr);

	freeaddrinfo(addrinfo);

	return 0;
}

int Protocol::get_socket() {
	return socket_fd;
}

void Protocol::to_string() {
	std::string prot = protocol_type == Config::Protocol::TCP ? "tcp" : "udp";

	std::cout 	<< "\033[4mIPK25-CHAT Protocol\033[0m\n"
				<< "Transport protocol: " << prot << "\n"
				<< "Server: " << inet_ntoa(server_address.sin_addr) << ":" << ntohs(server_address.sin_port) << "\n"
				<< "Dynamic port: " << dyn_port << "\n"
				<< "Socket file descriptor: " << socket_fd << "\n"
				<< "Socket type: " << socket_type
				<< std::endl;
}

int Protocol::await_response(uint16_t timeout, int expected, Response& response) {
	struct pollfd pfd = {socket_fd, POLLIN, 0};
	
	while (true) {
		int ready = poll(&pfd, 1, timeout);

		if (ready < 0 && errno != EINTR) {
			local_error("poll() failure");
			return GENERAL_ERROR;
		}

		if (ready == 0) {
			return TIMEOUT;
		}

		if (pfd.revents & POLLIN) {
			/* Receive from the socket */
			if (receive()) {
				local_error("Await - receive()");
				return NETWORK_ERROR;
			}

			/* Process and parse the message */
			if (process(response)) {
				local_error("Await - process()");
				return PROTOCOL_ERROR;
			}

			/* Duplicate msg --> skip */
			if (response.duplicate) {
				continue;
			}

			/* Buffer the message to process later, if needed */
			msg_queue.push(response);

			/* Expected response successfuly arrived */
			if (response.type == expected) {
				break;
			}

			/* If the message requires it, gracefuly exit */
			if (response.type == ERR || response.type == BYE) {
				break;
			}
		}
	}

	return SUCCESS;
}