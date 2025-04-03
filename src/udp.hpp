#pragma once

#include "protocol.hpp"
#include "config.hpp"
#include <cstdint>

class UDP : public Protocol {
	public:
		UDP(Config& config);
		~UDP();	

		int connect() override;
		int send(std::string msg) override;
		int receive() override;
		int process(Response& response) override;
		int error(std::string err) override;
		int disconnect() override;

	private:
		uint16_t message_id;
		uint16_t udp_timeout;
		uint8_t retransmission;

		void assign_message_id(uint16_t message_id);
		int direct_send(std::string msg);
		int confirm(uint16_t message_id);
};