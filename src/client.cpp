#include "client.hpp"
#include "command.hpp"
#include "error.hpp"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <csignal>
#include <poll.h>
#include <string>

static volatile sig_atomic_t interrupt;

void catch_signal(int signal) {
	interrupt = 1;
	std::cout << "CTRL + C --> ";
}

Client::Client(std::unique_ptr<Protocol> protocol)
	: state {Client::State::START}
	, display_name{"Display_Name"}
	, protocol{std::move(protocol)} {
}

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

void client_error(std::string err) {
	std::cout << "ERROR: " << err << std::endl;
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
				<< "Prints this help message with command description.\n"
				<<std::endl;
}

int Client::client_run() {
	signal(SIGINT, catch_signal); // Maybe use sigaction() instead
	
	//protocol->to_string();

	/* Bind client referrence to protocol */
	if (protocol->bind_client(this)) {
		std::cerr << "ERROR: bind_client() unable to get client reference" << std::endl;
		return 1;
	}

	/* Connect to server */
	if (protocol->connect()) {
		std::cerr << "ERROR: Connection failed" << std::endl;
		return PROTOCOL_ERROR;
	}

	/* POLLING to avoid BLOCKING */
	struct pollfd pfds[2] = {};
	int timeout = -1; // Endless

	/* STDIN */
	pfds[0].fd = STDIN_FILENO;
	pfds[0].events = POLLIN;

	/* NETWORK SOCKET */
	pfds[1].fd = protocol->get_socket();
	pfds[1].events = POLLIN;

	std::string input = "";

	/* Client core loop */
	while (state != State::END && !interrupt) {
		int ready = poll(pfds, 2, timeout);

		/* Poll ready and server connection */
		if (ready < 0 && errno != EINTR) {
			std::cerr << "error: poll error" << std::endl;
			return CLIENT_ERROR;
		}

		/* STDIN ready */
		if (pfds[0].revents & POLLIN) {
			std::getline(std::cin, input);
		
			if (std::cin.eof()) {
				std::cout << "CTRL + D --> ";
				break;
			}

			if(!input.empty()) {
				auto cmd = get_command(input);
				
				/* Command is valid */
				if (cmd != nullptr) {
					cmd->execute(*this);
				}
			}
		}

		/* Socket POLLIN */
		if (pfds[1].revents & POLLIN) {
			std::cout << "network receive data" << std::endl;
			
		}
	}

	// Move disconnect here

	return 0;
}

