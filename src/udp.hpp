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
		int receive(std::string msg) override;
		int process(std::string msg) override;
		int error(std::string err) override;
		int disconnect() override;

	private:
		uint16_t message_id;
		uint16_t timeout;
		uint8_t retransmission;

		void assign_message_id(uint16_t message_id);
		int confirm(uint16_t message_id);
};