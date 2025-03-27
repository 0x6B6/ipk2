#pragma once
#include <cstdint>

struct Config {
	enum class Protocol {
	TCP,
	UDP,
	UND
	};

	Protocol protocol;
	char *ip_hostname;
	uint16_t server_port;
	uint16_t udp_timeout;
	uint8_t udp_retransmission;

	Config() {
		protocol = Protocol::UND;
		ip_hostname = nullptr;
		/* Default value configuration */
		server_port = 4567;
		udp_timeout = 250;
		udp_retransmission = 3;
	}
};