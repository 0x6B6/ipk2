#include "signal.hpp"
#include "error.hpp"

#include <csignal>

volatile sig_atomic_t terminate;

/* Signal handler function */
void catch_signal(int signal) {
	terminate = 1;
}

/* SIGINT catch setup
 * source: https://man7.org/linux/man-pages/man2/sigaction.2.html
 */
int set_signal(){
	struct sigaction signal = {};
	
	signal.sa_flags = 0;
	signal.sa_handler = catch_signal;
	
	sigemptyset(&signal.sa_mask);

	if (sigaction(SIGINT, &signal, nullptr) == -1) {
		local_error("sigaction()");
		return GENERAL_ERROR;
	}

	return SUCCESS;
}