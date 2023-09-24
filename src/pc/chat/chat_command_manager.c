#include "chat_command_manager.h"
#include <string.h>

#define MAX_COMMANDS 512

typedef struct {
    const char* name;
    bool (*execute)(char* args);
    const char* description;
} CommandEntry;

static CommandEntry gCommands[MAX_COMMANDS];
static int gNumCommands = 0;

void register_chat_command(const char* commandName, bool (*execute)(char* args), const char* description) {
    if (gNumCommands < MAX_COMMANDS) {
        gCommands[gNumCommands].name = commandName;
        gCommands[gNumCommands].execute = execute;
        gCommands[gNumCommands].description = description;
        gNumCommands++;
    }
}

bool execute_chat_command(char* commandName, char* args) {
    for (int i = 0; i < gNumCommands; i++) {
        if (strcmp(gCommands[i].name, commandName) == 0) {
            return gCommands[i].execute(args);
        }
    }
    return false;
}

void display_all_chat_commands() {
    for (int i = 0; i < gNumCommands; i++) {
        djui_chat_message_create(gCommands[i].description);
    }
}
