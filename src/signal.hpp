#include <csignal>
#include <unistd.h>

extern volatile sig_atomic_t terminate;

void catch_signal(int signal);

int set_signal();