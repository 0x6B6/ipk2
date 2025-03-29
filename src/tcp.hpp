#pragma once

#include "protocol.hpp"
#include "config.hpp"

class TCP : public Protocol {
	public:
		TCP(Config& config);
		~TCP();

		int connect() override;
		int send(std::string msg) override;
		int receive(std::string msg) override;
		int process(std::string msg) override;
		int error(std::string err) override;
		int disconnect() override;
};