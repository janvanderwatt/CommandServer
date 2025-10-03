#include "CommandServer.h"

// #define DEBUG_COMMAND_SERVER

#if defined(DEBUG_COMMAND_SERVER)
bool DebugTestCommand(bool firstCall, const char **args, uint8_t argCount) {
    Serial.printf("[%d] commands passed\n", argCount);
    for (uint8_t i = 0; i < argCount; i++) {
        Serial.printf("arg[%d] = [%s]\n", i, args[i]);
    }
    return true;
}
#endif // defined(DEBUG_COMMAND_SERVER)

CommandServer::CommandServer() {
#if defined(DEBUG_COMMAND_SERVER)
    commandBuffer = "CMD par1 p2 \"p3\" par4";
    registerCommand("CMD", DebugTestCommand);
#endif // defined(DEBUG_COMMAND_SERVER)
}

bool CommandServer::registerCommand(const char *command, bool (*handler)(bool firstCall, const char **args, uint8_t argCount)) {
    // Register a command with its handler
    for (int i = 0; i < MAX_COMMANDS; ++i) {
        if (commandHandlers[i] == nullptr) {
            commandHandlers[i] = handler;
            registeredCommands[i] = command;
            Serial.printf("Registered command handler for [%s] at position [%d]\n", command, i);
            return true;
            break;
        }
    }
    return false; // No space to register more commands
}

void CommandServer::checkForCommand() {
    // If currently processing a command, keep invoking handler
    if (activeHandler >= 0) {
        if (commandHandlers[activeHandler](false, this->commandArgs, this->argCount)) {
            // Finished
            activeHandler = -1;
        }
        return;
    }

    // Otherwise, scan serial
#if defined(DEBUG_COMMAND_SERVER)
    processLine();
#else
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n') {
            processLine();
            commandBuffer = "";
        } else if (c != '\r') {
            commandBuffer += c;
        }
    }
#endif // defined(DEBUG_COMMAND_SERVER)
}

void CommandServer::processLine() {
    if (commandBuffer.length() == 0)
        return;

    // Split tokens
    splitTokens();
    if (this->argCount == 0) {
        return;
    }

    bool done = commandHandlers[activeHandler](false, this->commandArgs, this->argCount);
    if (done) {
        activeHandler = -1;
    }
}

void CommandServer::splitTokens() {
    bool inQuotes = false, foundCommand = false;
    uint8_t argLength = 0, startOfArg = 0;

    this->argCount = 0;

    for (uint8_t i = 0; i < commandBuffer.length(); i++) {
        char c = commandBuffer[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            commandBuffer[i] = 0; // terminate the string at this point
            // We have found a space, and we're not inside quotes
            if (!foundCommand) {
                // This is the first argument, check if it matches any commands (CASE SENSITIVE!)
                for (uint8_t j = 0; j < MAX_COMMANDS; j++) {
                    if (commandHandlers[j] != nullptr) {
                        const char *cmpCmd = commandBuffer.c_str();
                        if (strcmp(registeredCommands[j], cmpCmd) == 0) {
                            activeHandler = j;
                            startOfArg = i + 1;
                            foundCommand = true;
                            argLength = 0;
                            break;
                        }
                    }
                }
                if (!foundCommand) {
                    return;
                }
            } else {
                // We already have a matched command, check if the argument isn't zero length, and if not, add it to the command list
                if (argLength) {
                    this->commandArgs[this->argCount] = commandBuffer.c_str() + startOfArg;
                    this->argCount++;
                    argLength = 0;
                }
                startOfArg = i + 1;
            }
        } else {
            argLength++;
        }
    }
    if (foundCommand) {
        // Check for the last parameter (it's null-terminated, and doesn't have a SPACE after it)
        if (argLength) {
            this->commandArgs[this->argCount] = &commandBuffer.c_str()[startOfArg];
            this->argCount++;
            argLength = 0;
        }
    }
}
