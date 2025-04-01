#include "command.hpp"
#include "message.hpp"
#include "msg_factory.hpp"
#include "protocol.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <cctype>

bool auth_param_valid(std::string username, std::string secret, std::string display_name) {
	/* Check if its not empty */
	if (username.empty() || secret.empty() || display_name.empty()) {
		return false;
	}

	/* Check valid chars */
	if (valid_char(username) == false || valid_char(secret) == false || valid_printable(display_name) == false) {
		return false;
	}

	/* Check length */
	if (username.length() > MAX_ID_LEN || secret.length() > MAX_SECRET_LEN || display_name.length() > MAX_DN_LEN) {
		return false;
	}

	return true;
}

bool join_param_valid(std::string channel_id) {
	if (channel_id.empty()) {
		return false;
	}

	if (valid_char(channel_id) == false) {
		return false;
	}

	if (channel_id.length() > MAX_CID_LEN) {
		return false;
	}

	return true;
}

bool rename_param_valid(std::string display_name) {
	if (display_name.empty()) {
		return false;
	}

	if (valid_printable(display_name) == false) {
		return false;
	}

	if (display_name.length() > MAX_DN_LEN) {
		return false;
	}

	return true;
}

/**
 * Command parse function
 */
std::unique_ptr<Command> get_command(std::string input) {
	using namespace std;

	std::unique_ptr<Command> command;
	istringstream input_stream(input);
	string cmd, username, secret, display_name, channel_id; 

	input_stream >> cmd;

	if (cmd.front() == '/') {
		if (cmd == "/auth") {
			input_stream >> username;
			input_stream >> secret;
			input_stream >> display_name;

			if (auth_param_valid(username, secret, display_name) == false || !input_stream.eof()) {
				cerr << "Invalid parameters\n";
				return nullptr;
			}

			command = std::make_unique<AuthCommand>(username, secret, display_name);
		}
		else if (cmd == "/join") {
			input_stream >> channel_id;

			if (join_param_valid(channel_id) == false || !input_stream.eof()) {
				cerr << "Invalid parameters\n";
				return nullptr;
			}

			command = std::make_unique<JoinCommand>(channel_id);
		}
		else if (cmd == "/rename") {
			input_stream >> display_name;

			if (rename_param_valid(display_name) == false || !input_stream.eof()) {
				cerr << "Invalid parameters\n";
				return nullptr;
			}

			command = std::make_unique<RenameCommand>(display_name);
		}
		else if (cmd == "/help") {
			if (!input_stream.eof()) {
				cerr << "'/help' does not require any additional parameters!\n";
				return nullptr;
			}

			command = std::make_unique<HelpCommand>();
		}
		else {
			cerr << "Invalid command\n";
			return nullptr;
		}
	}
	else {
		if (valid_printable_msg(input) == false) {
			return nullptr;
		}

		command = std::make_unique<MsgCommand>(input);
	}

	return command;
}

/** 
 *	Command constructors
 */
AuthCommand::AuthCommand(std::string username, std::string secret, std::string display_name)
	: username{username}
	, secret{secret}
	, display_name{display_name} {
}

JoinCommand::JoinCommand(std::string channel_id): channel_id{channel_id} {
}

RenameCommand::RenameCommand(std::string display_name): display_name{display_name} {
}

MsgCommand::MsgCommand(std::string message): message{message} {	
}

/**
 *	Command public methods
 */
bool Command::undefined() {
	return type == Type::UNDEF; 
}

/* debug function */
void print_msg(std::string msg) {
	for (int i = 0; i < msg.length(); ++i) {
		char c = msg[i];
		int x = c;
		std::cout << c << "(" << x << ")";
	}
	std::cout << std::endl;
}

/* /auth command */
int AuthCommand::execute(Client& client) {
	Protocol& p = client.get_protocol();
	MsgFactory& f = p.get_msg_factory();

	client.set_name(display_name);

	std::string msg = f.create_auth_msg(username, display_name, secret);

	p.send(msg);

	// receive & error
	
	return 0;
}

/* /join command */
int JoinCommand::execute(Client& client) {
	Protocol& p = client.get_protocol();
	MsgFactory& f = p.get_msg_factory();

	std::string msg = f.create_join_msg(channel_id, client.get_name());

	p.send(msg);

	// receive & error

	return 0;
}

/* /rename command */
int RenameCommand::execute(Client& client) {
	client.set_name(display_name);

	return 0;
}

/* /help command */
int HelpCommand::execute(Client& client) {
	client.help();

	return 0;
}

/* standard chat message */
int MsgCommand::execute(Client& client) {
	Protocol& p = client.get_protocol();
	MsgFactory& f = p.get_msg_factory();

	std::string msg = f.create_chat_msg(client.get_name(), message); 

	p.send(msg);

	return 0;
}