#pragma once

#include "protocol.hpp"
#include "config.hpp"

class TCP : public Protocol {
	public:
		TCP(Config& config);
		~TCP();

		int connect() override;
		int send(std::string msg) override;
		int receive() override;
		int process(Response& response) override;
		int error(std::string err) override;
		int disconnect(std::string id) override;
};