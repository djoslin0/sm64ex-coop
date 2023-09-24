#ifndef CHAT_COMMAND_H
#define CHAT_COMMAND_H
#include <stdbool.h>

typedef struct ChatCommand {
    char* commandName;
    bool (*execute)(char* command);
    void (*display)();
} ChatCommand;

#endif
