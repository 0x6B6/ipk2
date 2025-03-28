#pragma once

#include "protocol.hpp"
#include "config.hpp"

class TCP : public Protocol {
	public:
		TCP(Config& config);
		~TCP();
	/*	int send() override;
		int receive() override;
		int process() override;
		*/
	private:
		int connect();
};