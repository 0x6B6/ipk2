#pragma once

#include <memory>
#include <string>

#include "protocol.hpp"
#include "message.hpp"

class Protocol;

class Client {
	public:
		enum class State {
			START,
			OPEN,
			AWAITING,
			END,
			ERR
		};

		/* Client */
		Client(std::unique_ptr<Protocol> protocol);
		int client_run();

		/* Client info */
		void client_output(std::string msg);
		void help();

		/* Client state */
		void set_state(State new_state);
		State get_state();

		/* Display name */
		void set_name(std::string name);
		std::string get_name();
		
		/* IPK25 & Transport protocol */
		Protocol& get_protocol();

	private:
		/* Client info */
		State state;
		std::string display_name;

		/* IPK25 & Transport protocol */
		std::unique_ptr<Protocol> protocol;

		void process_msg(Response& response);
		void process_msg_queue();
};