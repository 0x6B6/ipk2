#include "message.hpp"

/**
 *	Input validation helper functions
 */
bool valid_char(std::string string) {
	char c;

	for (int i = 0; i < string.length(); ++i) {
		c = string[i];

		/* Alphanumeric chars, underscore, dash */
		if (!isalnum(c) && c != '_' && c != '-') {
			return false;
		}
	}

	return true;
}

bool valid_printable(std::string string) {
	char c;

	for (int i = 0; i < string.length(); ++i) {
		c = string[i];

		/* Printable ASCII chars from ! to ~ */
		if (c < 0x21 || c > 0x7E) {
			return false;
		}
	}

	return true;	
}

bool valid_printable_msg(std::string string) {
	char c;

	for (int i = 0; i < string.length(); ++i) {
		c = string[i];

		/* Printable ASCII chars from ! to ~, space, linefeed */
		if ((c < 0x20 || c > 0x7E) && c != 0x0A) {
			return false;
		}
	}

	return true;	
}