#include "client.hpp"
#include "command.hpp"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <csignal>
#include <poll.h>

static volatile sig_atomic_t interrupt;

void catch_signal(int signal) {
	interrupt = 1;
}

Client::Client(std::unique_ptr<Protocol> protocol)
	: state {Client::State::START}
	, display_name{"Display_Name"}
	, protocol{std::move(protocol)} {
}

void Client::set_state(State new_state) {
	state = new_state;
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
	
	// set stdin to be polled so it doesnt block
	struct pollfd pfds[1];
	pfds[0].fd = STDIN_FILENO;
	pfds[0].events = POLLIN;

	while (state != State::END) {
		std::string input;

		int ret = poll(pfds, 1, -1);

		if (ret < 0 && errno != EINTR) {
			std::cerr << "poll error" << std::endl;
			return 1;
		}

		if (pfds[0].revents & POLLIN) {
			std::getline(std::cin, input);
		}

		if (std::cin.eof() || interrupt) {
			std::cout << "CTRL or C/CTRL + D --> bye";
			break;
		}

		if(!input.empty()) {
			auto cmd = get_command(input);
		
			if (cmd != nullptr) {
				cmd->execute(*this);
			}
		}
	}

	return 0;
}

