#include "error.hpp"
#include "args.hpp"
#include "config.hpp"
#include "client.hpp"
#include "error.hpp"

/* TODO:
 * incomming message queue
 * Proper ERR messages sending
 * Message length? 
 * /exit
 * /style
 * /clear
 */

int main(int argc, char **argv) {
	Config config;

	/* Parse program parameters */
	if (args_parse(argc, argv, config)) {
		local_error("Argument parsing failed");
		return PARSE_ERROR;
	}

	/* Select and setup given protocol */
	auto protocol = Protocol::protocol_setup(config);

	if (protocol == nullptr) {
		local_error("Protocol setup failed");
		return PROTOCOL_ERROR;
	}

	/* Launch and run client */
	Client client(std::move(protocol));

	if (client.client_run()) {
			local_error("Client runtime");
			return CLIENT_ERROR;
	}

	return 0;
}