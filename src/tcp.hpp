#pragma once

#include "protocol.hpp"
#include "config.hpp"

class TCP : public Protocol {
	public:
		TCP(Config& config);
		~TCP();

		/* Transport protocol overriden methods */
		int connect() override;
		int send(std::string msg) override;
		int receive() override;
		int process(Response& response) override;
		int error(std::string err) override;
		int disconnect(std::string id) override;

		/* Segmentation */
		int offset_segment = 0;
		bool segmentation = false;
};