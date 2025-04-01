#pragma once

#include <cstdint>
#include <string>

/* Message type */
enum MsgType : uint8_t {
	CONFIRM,	// UDP only
	REPLY,		// Server response O 
	AUTH,		// Request message
	JOIN,		// Request message
	MSG,		// Client chat message
	PING,		// UDP only, server query message
	ERR,		// Two-way communication error
	BYE			// Two-way communication
};

/* Response message container struct */
struct Response {
	MsgType type;
	std::string content;
};

/* Maximum length of parameters */
inline constexpr int MAX_ID_LEN = 20;			// Username - 20 bytes
inline constexpr int MAX_CID_LEN = 20;			// Channel ID - 20 bytes
inline constexpr int MAX_SECRET_LEN = 128;		// Secret - 128 bytes
inline constexpr int MAX_DN_LEN = 20;			// Display name - 20 bytes
inline constexpr int MAX_MSG_LEN = 20;			// Message - 60 000 bytes

/* Input validation helper functions */
bool valid_char(std::string string);
bool valid_printable(std::string string);
bool valid_printable_msg(std::string string);