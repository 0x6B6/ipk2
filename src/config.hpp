#pragma once
#include <cstdint>

/* Chat client configuration structure */
struct Config {
	/* Protocol types */
	enum class Protocol {
	TCP,
	UDP,
	UND
	};

	Protocol protocol;          // Chosen transport protocol
	char *ip_hostname;          // Server IPv4/Hostname address
	uint16_t server_port;       // Server port
	uint16_t udp_timeout;       // UDP confirmation timeout
	uint8_t udp_retransmission; // Number of udp packet retransmissions

	/* Default constructor */
	Config() {
		protocol = Protocol::UND;
		ip_hostname = nullptr;
		/* Default value configuration */
		server_port = 4567;
		udp_timeout = 250;
		udp_retransmission = 3;
	}
};