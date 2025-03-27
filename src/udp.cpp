#include "udp.hpp"
#include "msg_factory.hpp"
#include "protocol.hpp"
#include <cstdint>
#include <iostream>

UDP::UDP(Config& config)
	: Protocol(config)
	, message_id{0}
	, retransmission{config.udp_retransmission}
	, timeout{config.udp_timeout} {
}

int UDP::confirm(uint16_t message_id) {
	std::cout << MsgType::CONFIRM << message_id;
	return 0;
}