/**
 * @file: args.hpp
 */

#pragma once

#include "config.hpp"

char* get_arg(int argc, char** argv, int idx);

int to_int(char* arg);

int args_parse(int argc, char** argv, Config& config);

Config::Protocol parse_protocol(char* protocol);

int in_range(int value, int max);

void help(char *pname);