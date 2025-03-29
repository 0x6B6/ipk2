#pragma once

#include <memory>
#include <string>

#include "protocol.hpp"

class Protocol;

class Client {
	public:
		enum class State {
			START,
			AUTH,
			OPEN,
			JOIN,
			END
		};

		Client(std::unique_ptr<Protocol> protocol);
		int client_run();
		void client_error(std::string err);
		void help();
		void set_state(State new_state);
		State get_state();
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