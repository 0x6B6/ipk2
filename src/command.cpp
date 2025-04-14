#include "command.hpp"
#include "client.hpp"
#include "error.hpp"
#include "message.hpp"
#include "msg_factory.hpp"
#include "protocol.hpp"

#include <cstdlib>
#include <cstring>
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

	/* UNCOMMENT!!! */
/*	if (valid_char(channel_id) == false) {
		return false;
	}*/

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
				local_error("Invalid '/auth' parameters");
				return nullptr;
			}

			command = std::make_unique<AuthCommand>(username, secret, display_name);
		}
		else if (cmd == "/join") {
			input_stream >> channel_id;

			if (join_param_valid(channel_id) == false || !input_stream.eof()) {
				local_error("Invalid '/join' parameters");
				return nullptr;
			}

			command = std::make_unique<JoinCommand>(channel_id);
		}
		else if (cmd == "/rename") {
			input_stream >> display_name;

			if (rename_param_valid(display_name) == false || !input_stream.eof()) {
				local_error("Invalid '/rename' parameters");
				return nullptr;
			}

			command = std::make_unique<RenameCommand>(display_name);
		}
		else if (cmd == "/help") {
			if (!input_stream.eof()) {
				local_error("'/help' does not require any additional parameters!");
				return nullptr;
			}

			command = std::make_unique<HelpCommand>();
		}
		else {
			local_error("Invalid command '" + cmd + "', try /help");
			return nullptr;
		}
	}
	else {
		if (valid_printable_msg(input) == false) {
			local_error("Invalid chat message contents");
			return nullptr;
		}

		if (input.length() > MAX_MSG_LEN) {
			local_error("Chat message exceeds the 60 000 character limit - message will be truncated");
			input = input.substr(0, MAX_MSG_LEN);
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
	, display_name{display_name} {}

JoinCommand::JoinCommand(std::string channel_id): channel_id{channel_id} {}

RenameCommand::RenameCommand(std::string display_name): display_name{display_name} {}

MsgCommand::MsgCommand(std::string message): message{message} {}

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

/* AUTH /auth command */
int AuthCommand::execute(Client& client) {
	Protocol& p = client.get_protocol();
	MsgFactory& f = p.get_msg_factory();
	Response response = {.type = UNKNOWN, .status = NONE, .duplicate = false};
	int result = SUCCESS;

	if (client.get_state() != Client::State::OPEN) {
		client.set_name(display_name);

		if ((result = p.send(f.create_auth_msg(username, display_name, secret)))) {
			/* Handle server exit message later */
			if (result == SERVER_EXIT) {
				return SUCCESS;
			}

			local_error("send() - Unable to reach server");	
			
			return result;
		}

		if ((result = p.await_response(5000, MsgType::REPLY, response))) {
			local_error("Invalid message, format or response timeout");

			if (result == TIMEOUT) {
				return PROTOCOL_ERROR;
			}
			
			return result;
		}
	}
	else {
		local_error("Already authenthicated");
	}

	return SUCCESS;
}

/* JOIN /join command */
int JoinCommand::execute(Client& client) {
	Protocol& p = client.get_protocol();
	MsgFactory& f = p.get_msg_factory();
	Response response = {.type = UNKNOWN, .status = NONE, .duplicate = false};
	int result = SUCCESS;

	if (client.get_state() == Client::State::OPEN) {
		if ((result = p.send(f.create_join_msg(channel_id, client.get_name())))) {
			/* Handle server exit message later */
			if (result == SERVER_EXIT) {
				return SUCCESS;
			}

			local_error("send() - Unable to reach server");	

			return result;
		}

		if ((result = p.await_response(5000, MsgType::REPLY, response))) {
			local_error("Invalid message, format or response timeout");

			if (result == TIMEOUT) {
				return PROTOCOL_ERROR;
			}

			return result;
		}
	}
	else {
		local_error("Authentication required to join a channel.");
	}

	return SUCCESS;
}

/* RENAME /rename command */
int RenameCommand::execute(Client& client) {
	client.set_name(display_name);

	return SUCCESS;
}

/* HELP /help command */
int HelpCommand::execute(Client& client) {
	client.help();
	
	return SUCCESS;
}

/* MSG standard chat message */
int MsgCommand::execute(Client& client) {
	Protocol& p = client.get_protocol();
	MsgFactory& f = p.get_msg_factory();
	int result = SUCCESS;

	if (client.get_state() == Client::State::OPEN) {
		if ((result = p.send(f.create_chat_msg(client.get_name(), message)))) {
			/* Handle server exit message later */
			if (result == SERVER_EXIT) {
				return SUCCESS;
			}

			local_error("send() - Unable to reach server");	

			return result;
		}
	}
	else {
		local_error("Authentication required to send chat messages");
	}

	return SUCCESS;
}