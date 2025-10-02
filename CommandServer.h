#pragma once

#include <Arduino.h>

class CommandServer
{
public:
    CommandServer();
    bool registerStream(Print &stream);
    bool registerCommand(const char *command, void (*handler)(const char *args));
    void assembleCommand();

private:
    static const uint8_t MAX_COMMANDS = 10;

    struct StreamInfo
    {
        Print *stream;
        uint8_t logLevel;
        StreamInfo *next;
    };
    StreamInfo *firstStream = nullptr, *lastStream = nullptr;

    String commandBuffer;
    void (*commandHandlers[MAX_COMMANDS])(const char *args);
    const char *registeredCommands[MAX_COMMANDS];
};
