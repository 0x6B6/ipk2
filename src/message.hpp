#pragma once

#include <cstdint>
#include <string>

/* Message type */
enum MsgType : uint8_t {
	CONFIRM,      // UDP only
	REPLY,        // Server response O 
	AUTH,         // Request message
	JOIN,         // Request message
	MSG,          // Client chat message
	PING = 0xFD,  // UDP only, server query message
	ERR = 0xFE,   // Two-way communication error
	BYE = 0xFF,   // Two-way communication
	UNKNOWN = 101 // Unknown message type
};

/* Response status type */
enum ResponseStatus : uint8_t {
	NONE,  // Reply has *no* status
	OK,    // OK
	NOK    // NOT OK
};

/* Response message container struct */
struct Response {
	MsgType type = UNKNOWN;       // Message type
	ResponseStatus status = NONE; // Response status, if it has any
	std::string content;          // Formated content of the message
	bool duplicate = false;       // Flag for duplicate message
	bool incomplete = false;      // Flag for incomplete message when segmentation occurrs
};

/* Maximum length of parameters */
inline constexpr int MAX_ID_LEN = 20;      // Username - 20 bytes
inline constexpr int MAX_CID_LEN = 20;     // Channel ID - 20 bytes
inline constexpr int MAX_SECRET_LEN = 128; // Secret - 128 bytes
inline constexpr int MAX_DN_LEN = 20;      // Display name - 20 bytes
inline constexpr int MAX_MSG_LEN = 60000;  // Message - 60 000 bytes

/* Every TCP message must be followed by CRLF */
constexpr const char* CRLF = "\r\n";

/* Message validation helper functions */
bool valid_char(std::string string);
bool valid_printable(std::string string);
bool valid_printable_msg(std::string string);

/* Message conversion helper functions */
char to_upper(char c);
std::string str_up(std::string str);