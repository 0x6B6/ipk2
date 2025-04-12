#include "client.hpp"
#include "command.hpp"
#include "error.hpp"
#include "message.hpp"
#include "msg_factory.hpp"
#include "signal.hpp"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <csignal>
#include <poll.h>
#include <string>
#include <unistd.h>

Client::Client(std::unique_ptr<Protocol> protocol)
	: state {Client::State::START}
	, display_name{"unknown"}
	, protocol{std::move(protocol)} {}

void Client::set_state(State new_state) {
	state = new_state;
}

Client::State Client::get_state() {
	return state;
}

void Client::set_name(std::string name) {
	display_name = name;
}

std::string Client::get_name() {
	return display_name;
}

Protocol& Client::get_protocol() {
	return *protocol.get();
}

void Client::client_output(std::string msg) {
	std::cout << msg << std::endl;
}

void Client::help() {
	constexpr int cmd_w = 15;
	constexpr int param_w = 40;

	std::cout 	<< std::left
				<< "\033[1mIPK25 Chat client command line interface\033[0m\n"
				<< "Usage: /command [PARAM]...\n"
				<< "Required arguments are enclosed within curly braces.\n\n"
				<< std::setw(23) << "\033[4mCommand\033[0m"
				<< std::setw(48) << "\033[4mParameter(s)\033[0m"
				<< "\033[4mClient behaviour\033[0m"
				<< std::endl
				<< std::setw(cmd_w) << "/auth"
				<< std::setw(param_w) << "{Username} {Secret} {DisplayName}"
				<< "Sends authorization request to the chat server and sets initial display name.\n"
				<< std::setw(cmd_w) << "/join"
				<< std::setw(param_w) << "{ChannelID}"
				<< "Joins the given chat channel.\n"
				<< std::setw(cmd_w) << "/rename"
				<< std::setw(param_w) << "{DisplayName}"
				<< "Changes display name.\n"
				<< std::setw(cmd_w) << "/help"
				<< std::setw(param_w) << "None"
				<< "Prints this help message with command description."
				<<std::endl;
}

void Client::process_msg(Response& response) { 
	switch (response.type) {
		case MSG: {
			client_output(response.content);
			break;
		}

		case ERR: {
			client_output(response.content);
			set_state(State::ERR);
			break;
		}

		case BYE: {
			set_state(State::END);
			break;
		}

		default: // Ping, etc. --> skip
			break;
	}
}

void Client::process_msg_queue() {
	auto& msg_queue = protocol->get_msg_queue();

	while (!msg_queue.empty()) {
		Response response = msg_queue.front();

		if (response.type == REPLY) {
			client_output(response.content);

			if (response.status == OK) {
				set_state(Client::State::OPEN);
			}
		}
		else {
			process_msg(response);
		} 

		msg_queue.pop();
	}
}

int Client::client_run() {
	/* SIGINT signal catch */
	if (set_signal()) {
		return CLIENT_ERROR;
	}

	/* Bind client referrence to protocol */
	protocol->bind_client(this);

	/* Connect to server */
	if (protocol->connect()) {
		local_error("Connection failed");
		return PROTOCOL_ERROR;
	}

	/* POLLING to avoid BLOCKING I/O operations */
	struct pollfd pfds[2] = {{STDIN_FILENO, POLLIN, 0}, {protocol->get_socket(), POLLIN, 0}};
	int timeout = -1; // Endless

	std::string input = "";
	Response response;
	MsgFactory& factory = protocol->get_msg_factory();

	/* Client core loop */
	while (state != State::END && state != State::ERR && !terminate) {
		int ready = poll(pfds, 2, timeout);

		/* Poll ready and server connection */
		if (ready < 0 && errno != EINTR) {
			local_error("poll() failure");
			return CLIENT_ERROR;
		}

		/* STDIN ready --> Command */
		if (pfds[0].revents & POLLIN) {
			std::getline(std::cin, input);
			
			/* EOF reached --> exit */
			if (std::cin.eof()) {
				terminate = 1;
				set_state(State::END);
			}

			/* Input is not empty */
			if(!input.empty()) {
				auto cmd = get_command(input);
				
				/* Command is invalid, skip */
				if (cmd == nullptr) {
					continue;
				}

				/* Execute command routine */
				if (cmd->execute(*this)) {
					local_error("Command action unsuccessful");
					return CLIENT_ERROR;
				}

				/* Proccess message queue after finishing command */
				process_msg_queue();
			}
		}

		/* Socket POLLIN */
		if (pfds[1].revents & POLLIN) {
			/* Receive the message from the socket */
			if (protocol->receive()) {
				local_error("Message could not be received");
				
				return CLIENT_ERROR;
			}

			/* Process and parse the message  */
			if (protocol->process(response)) {
				local_error("Message could not be processed");
				
				protocol->error(factory.create_err_msg(get_name(), "Received a malformed message from the server"));
				
				return CLIENT_ERROR;
			}

			/* Process and handle the message */
			process_msg(response);
		}
	}

	/* Disconnect logic on terminate signal (CTRL + (C | D)) */
	if (terminate) {
		if (protocol->disconnect(get_name())) {
			return CLIENT_ERROR;
		}
	}

	return SUCCESS;
}