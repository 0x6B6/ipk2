#pragma once

#include "protocol.hpp"
#include "config.hpp"
#include <cstdint>

class UDP : public Protocol {
	public:
		UDP(Config& config);
		~UDP();	
	/*	int connect() override;
		int send() override;
		int receive() override;
		int process() override;
		*/
		int confirm(uint16_t message_id);
	private:
		uint16_t message_id;
		uint16_t timeout;
		uint8_t retransmission;

		void assign_message_id(uint16_t message_id);
};