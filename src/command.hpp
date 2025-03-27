#pragma once

#include "client.hpp"
#include <string>

struct Command {
	enum class Type {
		AUTH,
		JOIN,
		RENAME,
		HELP,
		MSG,
		UNDEF
	};

	Type type;
	bool undefined();
	virtual ~Command() = default;
	virtual int execute(Client& client) = 0;
};

struct AuthCommand : public Command {
	std::string username,
				secret,
				display_name;

	AuthCommand(std::string username, std::string secret, std::string display_name);
	int execute(Client& client) override;
};

struct JoinCommand : public Command {
	std::string channel_id;

	JoinCommand(std::string channel_id);
	int execute(Client& client) override;
};

struct RenameCommand : public Command {
	std::string display_name;

	RenameCommand(std::string display_name);
	int execute(Client& client) override;
};

struct HelpCommand : public Command {
	int execute(Client& client) override;
};

struct MsgCommand : public Command {
	std::string message;

	MsgCommand(std::string message);
	int execute(Client& client) override;
};

/**
 * Command parse and get function
 */
std::unique_ptr<Command> get_command(std::string input);