#pragma once

#include <Arduino.h>

class CommandServer {
public:
    CommandServer();
    bool registerCommand(const char* command, bool (*handler)(bool firstCall, const char** args, uint8_t argCount));
    void checkForCommand();

private:
    static const uint8_t MAX_COMMANDS = 10, MAX_ARGS = 4;
    int8_t activeHandler = -1;

    String commandBuffer;
    bool (*commandHandlers[MAX_COMMANDS])(bool firstCall, const char** args, uint8_t argCount) = { nullptr };
    const char* registeredCommands[MAX_COMMANDS];
    const char* commandArgs[MAX_ARGS];
    uint8_t argCount = 0;

    void processLine();
    void splitTokens();
};
