#pragma once

#include <memory>
#include <string>

#include "protocol.hpp"

class Client {
	enum class State {
		START,
		AUTH,
		OPEN,
		JOIN,
		END
	};

	public:
		Client(std::unique_ptr<Protocol> protocol);
		int client_run();
		void help();
		void set_state(State new_state);
		void set_name(std::string name);
		std::string get_name();
		Protocol& get_protocol();

	private:
		/* Client info */
		State state;
		std::string display_name;

		/* IPK25 & Transport protocol */
		std::unique_ptr<Protocol> protocol;
};