#pragma once

#include "protocol.hpp"
#include "config.hpp"

class TCP : public Protocol {
	public:
		TCP(Config& config);
/*		int send() override;
		int receive() override;
		int process() override;
		~TCP() override;*/
	private:
		int connect();
};