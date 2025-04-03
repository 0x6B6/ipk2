#include "client.hpp"
#include "command.hpp"
#include "error.hpp"
#include "message.hpp"
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
	, display_name{"Display_Name"}
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

	/* POLLING to avoid BLOCKING */
	struct pollfd pfds[2] = {{STDIN_FILENO, POLLIN, 0}, {protocol->get_socket(), POLLIN, 0}};
	int timeout = -1; // Endless

	/* STDIN & network response */
	std::string input = "";
	Response response;

	/* Client core loop */
	while (state != State::END && state != State::ERR && !interrupt) {
		int ready = poll(pfds, 2, timeout);

		/* Poll ready and server connection */
		if (ready < 0 && errno != EINTR) {
			local_error("poll failure");
			return CLIENT_ERROR;
		}

		/* STDIN ready */
		if (pfds[0].revents & POLLIN) {
			std::getline(std::cin, input);
		
			if (std::cin.eof()) {
				log("CTRL + D --> ");
				set_state(State::END);
			}

			if(!input.empty()) {
				auto cmd = get_command(input);
				
				/* Command is invalid, skip */
				if (cmd == nullptr) {
					continue;
				}

				/* Execute command routine */
				if (cmd->execute(*this)) {
					local_error("command-action unsuccessful");
					return CLIENT_ERROR;
				}
			}
		}

		/* Socket POLLIN */
		if (pfds[1].revents & POLLIN) {
			log("network: main loop received data");

			if (protocol->receive() || protocol->process(response)) {
				local_error("Message could not be received or processed");
				return CLIENT_ERROR;
			}

			switch (response.type) {
				case MSG:
					client_output(response.content);
					break;

				case BYE:
					set_state(State::END);
					break;

				case ERR:
					client_output(response.content);
					set_state(State::ERR);
					break;

				default: // Ping, etc. --> skip
					break;
			}
		}
	}

	if (state == State::ERR) {
		return CLIENT_ERROR;
	}

	return SUCCESS;
}