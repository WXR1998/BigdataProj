#pragma once
#include <string>
#include "kvstore.hpp"

void server();
void* session(void *args);
bool parseCommand(std::string cmd);