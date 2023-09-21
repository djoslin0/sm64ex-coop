#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pc/network/network.h"
#include "pc/lua/smlua_hooks.h"
#include "pc/chat_commands.h"
#include "djui.h"

struct DjuiChatBox* gDjuiChatBox = NULL;
bool gDjuiChatBoxFocus = false;
static bool sDjuiChatBoxClearText = false;

#define MAX_HISTORY_SIZE 1000
#define MAX_MSG_LENGTH 201

typedef struct {
    int initialized;
    int size;
    char messages[MAX_HISTORY_SIZE][MAX_MSG_LENGTH];
    int currentIndex;
    char currentMessage[MAX_MSG_LENGTH];
} ArrayList;

ArrayList sentHistory;

static int iTabCompletionIndex = -1;
static char sTabCompletionOriginalText[MAX_MSG_LENGTH];

//static int iTabCompletionPlayernamesIndex = -1;
//static char sTabCompletionPlayernamesOriginalText[MAX_MSG_LENGTH];

//void resetTabCompletionCommands(void) {
//    iTabCompletionIndex = -1;
//    strncpy(sTabCompletionOriginalText, "", MAX_MSG_LENGTH - 1);
//}

//void resetTabCompletionPlayernames(void) {
//    iTabCompletionPlayernamesIndex = -1;
//    strncpy(sTabCompletionPlayernamesOriginalText, "", MAX_MSG_LENGTH - 1);
//}

//void resetTabCompletionAll(void) {
//    resetTabCompletionCommands();
//    resetTabCompletionPlayernames();
//}

void resetTabCompletion(void) {
    iTabCompletionIndex = -1;
    strncpy(sTabCompletionOriginalText, "", MAX_MSG_LENGTH - 1);
}

void sentHistoryInit(ArrayList *arrayList) {
    if (!arrayList->initialized) {
        arrayList->size = 0;
		arrayList->initialized = 1;
        arrayList->currentIndex = -1;
        strncpy(arrayList->currentMessage, "", MAX_MSG_LENGTH - 1);
    }
}

void sentHistoryAddMessage(ArrayList *arrayList, const char *newMessage) {
    if (arrayList->size == MAX_HISTORY_SIZE) {
        for (int i = 1; i < MAX_HISTORY_SIZE; i++) {
            strncpy(arrayList->messages[i-1], arrayList->messages[i], MAX_MSG_LENGTH);
        }
        arrayList->size--;
    }
	
    strncpy(arrayList->messages[arrayList->size], newMessage, MAX_MSG_LENGTH - 1);
    arrayList->messages[arrayList->size][MAX_MSG_LENGTH - 1] = '\0';
    arrayList->size++;
}

void sentHistoryUpdateCurrentMessage(ArrayList *arrayList, const char *message) {
    if (arrayList->currentIndex == -1) {
        strncpy(arrayList->currentMessage, message, MAX_MSG_LENGTH - 1);
    }
}

void sentHistoryNavigate(ArrayList *arrayList, bool navigateUp) {
    if (navigateUp) {
        if (arrayList->currentIndex == -1) {
            arrayList->currentIndex = arrayList->size - 1;
        } else if (arrayList->currentIndex > 1) {
            arrayList->currentIndex = arrayList->currentIndex - 1;
        } else {
            arrayList->currentIndex = 0;
        }
    } else {
        if (arrayList->currentIndex == -1 || arrayList->currentIndex == arrayList->size - 1) {
            arrayList->currentIndex = -1;
        } else if (arrayList->currentIndex > -1) {
            arrayList->currentIndex = arrayList->currentIndex + 1;
        }
    }
    djui_inputbox_set_text(gDjuiChatBox->chatInput, arrayList->currentIndex == -1 ? arrayList->currentMessage : arrayList->messages[arrayList->currentIndex]);
    djui_inputbox_move_cursor_to_end(gDjuiChatBox->chatInput);
}

void sentHistoryResetNavigation(ArrayList *arrayList) {
    strncpy(arrayList->currentMessage, "", MAX_MSG_LENGTH - 1);
    arrayList->currentIndex = -1;
}

bool djui_chat_box_render(struct DjuiBase* base) {
    struct DjuiChatBox* chatBox = (struct DjuiChatBox*)base;
    struct DjuiBase* ccBase = &chatBox->chatContainer->base;
    djui_base_set_size(ccBase, 1.0f, chatBox->base.comp.height - 32 - 8);
    if (sDjuiChatBoxClearText) {
        sDjuiChatBoxClearText = false;
        djui_inputbox_set_text(gDjuiChatBox->chatInput, "");
        djui_inputbox_select_all(gDjuiChatBox->chatInput);
    }
    return true;
}

static void djui_chat_box_destroy(struct DjuiBase* base) {
    struct DjuiChatBox* chatBox = (struct DjuiChatBox*)base;
    free(chatBox);
}

static void djui_chat_box_set_focus_style(void) {
    djui_base_set_visible(&gDjuiChatBox->chatInput->base, gDjuiChatBoxFocus);
    if (gDjuiChatBoxFocus) {
        djui_interactable_set_input_focus(&gDjuiChatBox->chatInput->base);
    }

    djui_base_set_color(&gDjuiChatBox->chatFlow->base, 0, 0, 0, gDjuiChatBoxFocus ? 128 : 0);
}

static void djui_chat_box_input_enter(struct DjuiInputbox* chatInput) {
    djui_interactable_set_input_focus(NULL);

    if (strlen(chatInput->buffer) != 0) {
        sentHistoryAddMessage(&sentHistory, chatInput->buffer);
        if (chatInput->buffer[0] == '/') {
            if (strcmp(chatInput->buffer, "/help") == 0 || strcmp(chatInput->buffer, "/?") == 0) {
                display_chat_commands();
            } else if (!exec_chat_command(chatInput->buffer)) {
                char extendedUnknownCommandMessage[MAX_MSG_LENGTH];
                snprintf(extendedUnknownCommandMessage, sizeof(extendedUnknownCommandMessage), "%s (/help)", DLANG(CHAT, UNRECOGNIZED));
                djui_chat_message_create(extendedUnknownCommandMessage);
            }
        } else {
            djui_chat_message_create_from(gNetworkPlayerLocal->globalIndex, chatInput->buffer);
            network_send_chat(chatInput->buffer, gNetworkPlayerLocal->globalIndex);
        }
    }

    djui_inputbox_set_text(chatInput, "");
    djui_inputbox_select_all(chatInput);
    if (gDjuiChatBoxFocus) { djui_chat_box_toggle(); }
}

static void djui_chat_box_input_escape(struct DjuiInputbox* chatInput) {
    djui_interactable_set_input_focus(NULL);
    djui_inputbox_set_text(chatInput, "");
    djui_inputbox_select_all(chatInput);
    if (gDjuiChatBoxFocus) { djui_chat_box_toggle(); }
}

static char* get_main_command_from_input(const char* input) {
    char* spacePos = strrchr(input, ' ');
    if (spacePos == NULL) {
        return NULL;
    }
    size_t len = spacePos - input;
    char* command = (char*) malloc(len + 1);
    strncpy(command, input, len);
    command[len] = '\0';
    return command;
}

static bool complete_subcommand(const char* mainCommand, const char* subCommandPrefix) {
    char** subcommands = smlua_get_chat_subcommands_list(mainCommand);
    if (!subcommands || !subcommands[0]) {
        if (subcommands) {
            free(subcommands);
        }
        return false;
    }

    int foundSubCommandsCount = 0;
    for (int i = 0; subcommands[i] != NULL; i++) {
        if (strncmp(subcommands[i], subCommandPrefix, strlen(subCommandPrefix)) == 0) {
            foundSubCommandsCount++;
        }
    }

    bool completionSuccess = false;
    if (foundSubCommandsCount > 0) {
        iTabCompletionIndex = (iTabCompletionIndex + 1) % foundSubCommandsCount;
        int currentIndex = 0;

        for (int i = 0; subcommands[i] != NULL; i++) {
            if (strncmp(subcommands[i], subCommandPrefix, strlen(subCommandPrefix)) == 0) {
                if (currentIndex == iTabCompletionIndex) {
                    char completion[MAX_MSG_LENGTH];
                    snprintf(completion, MAX_MSG_LENGTH, "/%s %s", mainCommand, subcommands[i]);
                    djui_inputbox_set_text(gDjuiChatBox->chatInput, completion);
                    djui_inputbox_move_cursor_to_end(gDjuiChatBox->chatInput);
                    completionSuccess = true;
                    break;
                }
                currentIndex++;
            }
        }
    }

    for (int i = 0; subcommands[i] != NULL; i++) {
        free(subcommands[i]);
    }
    free(subcommands);
    
    return completionSuccess;
}

typedef struct {
    char word[MAX_MSG_LENGTH];
    int index;
} CurrentWordInfo;

CurrentWordInfo get_current_word_info(char* buffer, int position) {
    CurrentWordInfo info;
    memset(info.word, 0, MAX_MSG_LENGTH);
    info.index = -1;

    int currentWordStart = position;
    int currentWordEnd = position;
    
    while (currentWordStart > 0 && buffer[currentWordStart - 1] != ' ') { currentWordStart--; }
    while (buffer[currentWordEnd] != '\0' && buffer[currentWordEnd] != ' ') { currentWordEnd++; }

    strncpy(info.word, &buffer[currentWordStart], currentWordEnd - currentWordStart);

    int wordCount = 0;
    for (int i = 0; i <= currentWordStart; i++) {
        if (buffer[i] == ' ' || i == 0) {
            wordCount++;
        }
    }
    info.index = wordCount;

    return info;
}


/*static bool complete_playername(const char* namePrefix) {
    char** players = smlua_get_chat_player_list();
    if (!players || !players[0]) {
        if (players) {
            free(players);
        }
        return false;
    }

    int foundPlayerCount = 0;
    for (int i = 0; players[i] != NULL; i++) {
        if (strncmp(players[i], namePrefix, strlen(namePrefix)) == 0) {
            foundPlayerCount++;
        }
    }

    bool completionSuccess = false;
    if (foundPlayerCount > 0) {
        iTabCompletionPlayernamesIndex = (iTabCompletionPlayernamesIndex + 1) % foundPlayerCount;
        int currentIndex = 0;

        for (int i = 0; players[i] != NULL; i++) {
            if (strncmp(players[i], namePrefix, strlen(namePrefix)) == 0) {
                if (currentIndex == iTabCompletionPlayernamesIndex) {
                    char completion[MAX_MSG_LENGTH];
                    char preCompletion[MAX_MSG_LENGTH];
                    strncpy(preCompletion, gDjuiChatBox->chatInput->buffer, namePrefix - gDjuiChatBox->chatInput->buffer);
                    preCompletion[namePrefix - gDjuiChatBox->chatInput->buffer] = '\0';
                    snprintf(completion, MAX_MSG_LENGTH, "%s%s", preCompletion, players[i]);
                    strcat(completion, namePrefix + strlen(namePrefix));
                    djui_inputbox_set_text(gDjuiChatBox->chatInput, completion);
                    completionSuccess = true;
                    break;
                }
                currentIndex++;
            }
        }
    }

    for (int i = 0; players[i] != NULL; i++) {
        free(players[i]);
    }
    free(players);
    
    return completionSuccess;
}*/

static bool djui_chat_box_input_on_key_down(struct DjuiBase* base, int scancode) {
    sentHistoryInit(&sentHistory);

    if (gDjuiChatBox == NULL) { return false; }
    f32 yMax = gDjuiChatBox->chatContainer->base.elem.height - gDjuiChatBox->chatFlow->base.height.value;

    f32* yValue = &gDjuiChatBox->chatFlow->base.y.value;
    bool canScrollUp   = (*yValue > yMax);
    bool canScrollDown = (*yValue < 0);
    f32 pageAmount = gDjuiChatBox->chatContainer->base.elem.height * 3.0f / 4.0f;

    char previousText[MAX_MSG_LENGTH];
    strncpy(previousText, gDjuiChatBox->chatInput->buffer, MAX_MSG_LENGTH - 1);

    switch (scancode) {
        case SCANCODE_UP:
            sentHistoryUpdateCurrentMessage(&sentHistory, gDjuiChatBox->chatInput->buffer);
            sentHistoryNavigate(&sentHistory, true);
            if (strcmp(previousText, gDjuiChatBox->chatInput->buffer) != 0) {
                resetTabCompletion();
            }
            return true;
        case SCANCODE_DOWN:
            sentHistoryUpdateCurrentMessage(&sentHistory, gDjuiChatBox->chatInput->buffer);
            sentHistoryNavigate(&sentHistory, false);
            if (strcmp(previousText, gDjuiChatBox->chatInput->buffer) != 0) {
                resetTabCompletion();
            }
            return true;
        case SCANCODE_PAGE_UP:
            gDjuiChatBox->scrolling = true;
            if (canScrollDown) { *yValue = fmin(*yValue + 15, 0); }
            return true;
        case SCANCODE_PAGE_DOWN:
            gDjuiChatBox->scrolling = true;
            if (canScrollUp) { *yValue = fmax(*yValue - 15, yMax); }
            return true;
        case SCANCODE_POS1:
            gDjuiChatBox->scrolling = true;
            if (canScrollDown) { *yValue = fmin(*yValue + pageAmount, 0); }
            return true;
        case SCANCODE_END:
            gDjuiChatBox->scrolling = true;
            if (canScrollUp) { *yValue = fmax(*yValue - pageAmount, yMax); }
            return true;
        case SCANCODE_TAB:
            bool alreadyTabCompleted = false;
            if (gDjuiChatBox->chatInput->buffer[0] == '/') {
                char* spacePosition = strrchr(sTabCompletionOriginalText, ' ');
                if (spacePosition != NULL) {
                    char* mainCommand = get_main_command_from_input(sTabCompletionOriginalText);
                    if (mainCommand) {
                        if (!complete_subcommand(mainCommand + 1, spacePosition + 1)) {
                            resetTabCompletion();
                        } else {
                            alreadyTabCompleted = true;
                        }
                        free(mainCommand);
                    }
                } else {
                    if (iTabCompletionIndex == -1) {
                        strncpy(sTabCompletionOriginalText, gDjuiChatBox->chatInput->buffer, MAX_MSG_LENGTH - 1);
                    }
                    
                    char* buffer_without_slash = sTabCompletionOriginalText + 1;
                    char** commands = smlua_get_chat_maincommands_list();
                    int foundCommandsCount = 0;
                    
                    for (int i = 0; commands[i] != NULL; i++) {
                        if (strncmp(commands[i], buffer_without_slash, strlen(buffer_without_slash)) == 0) {
                            foundCommandsCount++;
                        }
                    }
                    
                    if (foundCommandsCount > 0) {
                        iTabCompletionIndex = (iTabCompletionIndex + 1) % foundCommandsCount;
                        int currentIndex = 0;
                        
                        for (int i = 0; commands[i] != NULL; i++) {
                            if (strncmp(commands[i], buffer_without_slash, strlen(buffer_without_slash)) == 0) {
                                if (currentIndex == iTabCompletionIndex) {
                                    char completion[MAX_MSG_LENGTH];
                                    snprintf(completion, MAX_MSG_LENGTH, "/%s", commands[i]);
                                    djui_inputbox_set_text(gDjuiChatBox->chatInput, completion);
                                    djui_inputbox_move_cursor_to_end(gDjuiChatBox->chatInput);
                                    alreadyTabCompleted = true;
                                }
                                currentIndex++;
                            }
                        }
                    } else {
                        char* spacePositionB = strrchr(sTabCompletionOriginalText, ' ');
                        if (spacePositionB != NULL) {
                            char* mainCommandB = get_main_command_from_input(sTabCompletionOriginalText);
                            if (mainCommandB) {
                                if (!complete_subcommand(mainCommandB + 1, spacePositionB + 1)) {
                                    resetTabCompletion();
                                } else {
                                    alreadyTabCompleted = true;
                                }
                                free(mainCommandB);
                            }
                        }
                    }
                    
                    for (int i = 0; commands[i] != NULL; i++) {
                        free(commands[i]);
                    }
                    free(commands);
                }
            }
            if (!alreadyTabCompleted) {
                if (gDjuiChatBox->chatInput->selection[0] != gDjuiChatBox->chatInput->selection[1]) {
                    alreadyTabCompleted = true;
                }
            }
            if (!alreadyTabCompleted) {
                CurrentWordInfo wordCurrent = get_current_word_info(gDjuiChatBox->chatInput->buffer, gDjuiChatBox->chatInput->selection[0]);
                if (gDjuiChatBox->chatInput->buffer[0] == '/') {
                    if (wordCurrent.index == 1 && smlua_maincommand_exists(wordCurrent.word + 1)) {
                        alreadyTabCompleted = true;
                    } else if (wordCurrent.index == 2) {
                        CurrentWordInfo worldMainCommand = get_current_word_info(gDjuiChatBox->chatInput->buffer, 0);
                        if (smlua_maincommand_exists(worldMainCommand.word + 1) && smlua_subcommand_exists(worldMainCommand.word + 1, wordCurrent.word)) {
                            alreadyTabCompleted = true;
                        }
                    }
                }
            }
            if (!alreadyTabCompleted) {
                //[IMPORTANT INFO/TODO] => SOMETHING IS WRONG HERE, IT WILL CRASH UPON TAB-COMPLETION OF PLAYER NAMES. IT KINDA WORKED WHILE AGO BUT IT WAS KINDA BUGGY, BUT ME TRYING TO FIX IT JUST COMPLETLY BROKE IT. NOW ITS NO LONGER WORKING AT ALL BUT CRASHING THE GAME. THATS WHY I PUT // BEFORE THOSE LINES FOR NOW TO AT LEAST PREVENT THE CRASH UNTIL SOMEBODY WILL FIX IT...
                
                /* {{{CRASH-PREVENTING WORKAROUND CODE}}} */ 
                printf("\n{PLAYER-TAB-ERROR}  [CLASS]=djui_chat_box.c [METHOD]=djui_chat_box_input_on_key_down [LINE]=425  {PLAYER-TAB-ERROR}");
					//djui_inputbox_set_text(gDjuiChatBox->chatInput, "{PLAYER-TAB-ERROR} [CLASS]=djui_chat_box.c [METHOD]=djui_chat_box_input_on_key_down [LINE]=425 {PLAYER-TAB-ERROR}");
					//djui_inputbox_move_cursor_to_end(gDjuiChatBox->chatInput);
                
                /* {{{CRASH-CAUSING BUGGY CODE}}} */
                    /*if (complete_playername(get_current_word_info(gDjuiChatBox->chatInput->buffer, gDjuiChatBox->chatInput->selection[0]).word)) {
                        alreadyTabCompleted = true;
                    } else {
                        resetTabCompletionAll();
                    }*/
            }
            return true;
        case SCANCODE_ENTER:
            resetTabCompletion();
            sentHistoryResetNavigation(&sentHistory);
            djui_chat_box_input_enter(gDjuiChatBox->chatInput);
            return true;
        case SCANCODE_ESCAPE:
            resetTabCompletion();
            sentHistoryResetNavigation(&sentHistory);
            djui_chat_box_input_escape(gDjuiChatBox->chatInput);
            return true;
        default:
            bool returnValueOnOtherKeyDown = djui_inputbox_on_key_down(base, scancode);
            if (strcmp(previousText, gDjuiChatBox->chatInput->buffer) != 0) {
                resetTabCompletion();
            }
            return returnValueOnOtherKeyDown;
    }
}

static void djui_chat_box_input_on_text_input(struct DjuiBase *base, char* text) {
    char previousText[MAX_MSG_LENGTH];
    strncpy(previousText, gDjuiChatBox->chatInput->buffer, MAX_MSG_LENGTH - 1);
    djui_inputbox_on_text_input(base, text);
    if (strcmp(previousText, gDjuiChatBox->chatInput->buffer) != 0) {
        resetTabCompletion();
    }
}

void djui_chat_box_toggle(void) {
    if (gDjuiChatBox == NULL) { return; }
    if (!gDjuiChatBoxFocus) { sDjuiChatBoxClearText = true; }
    gDjuiChatBoxFocus = !gDjuiChatBoxFocus;
    djui_chat_box_set_focus_style();
    gDjuiChatBox->scrolling = false;
    gDjuiChatBox->chatFlow->base.y.value = gDjuiChatBox->chatContainer->base.elem.height - gDjuiChatBox->chatFlow->base.height.value;
}

struct DjuiChatBox* djui_chat_box_create(void) {
    if (gDjuiChatBox != NULL) {
        djui_base_destroy(&gDjuiChatBox->base);
        gDjuiChatBox = NULL;
    }

    struct DjuiChatBox* chatBox = calloc(1, sizeof(struct DjuiChatBox));
    struct DjuiBase* base = &chatBox->base;

    djui_base_init(&gDjuiRoot->base, base, djui_chat_box_render, djui_chat_box_destroy);
    djui_base_set_size_type(base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(base, 600, 400);
    djui_base_set_alignment(base, DJUI_HALIGN_LEFT, DJUI_VALIGN_BOTTOM);
    djui_base_set_color(base, 0, 0, 0, 0);
    djui_base_set_padding(base, 0, 8, 8, 8);

    struct DjuiRect* chatContainer = djui_rect_create(base);
    struct DjuiBase* ccBase = &chatContainer->base;
    djui_base_set_size_type(ccBase, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(ccBase, 1.0f, 0);
    djui_base_set_color(ccBase, 0, 0, 0, 0);
    chatBox->chatContainer = chatContainer;

    struct DjuiFlowLayout* chatFlow = djui_flow_layout_create(ccBase);
    struct DjuiBase* cfBase = &chatFlow->base;
    djui_base_set_location(cfBase, 0, 0);
    djui_base_set_size_type(cfBase, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(cfBase, 1.0f, 2);
    djui_base_set_color(cfBase, 0, 0, 0, 128);
    djui_base_set_padding(cfBase, 2, 2, 2, 2);
    djui_flow_layout_set_margin(chatFlow, 2);
    djui_flow_layout_set_flow_direction(chatFlow, DJUI_FLOW_DIR_UP);
    cfBase->addChildrenToHead = true;
    cfBase->abandonAfterChildRenderFail = true;
    chatBox->chatFlow = chatFlow;

    struct DjuiInputbox* chatInput = djui_inputbox_create(base, 200);
    struct DjuiBase* ciBase = &chatInput->base;
    djui_base_set_size_type(ciBase, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(ciBase, 1.0f, 32);
    djui_base_set_alignment(ciBase, DJUI_HALIGN_LEFT, DJUI_VALIGN_BOTTOM);
    djui_interactable_hook_key(&chatInput->base, djui_chat_box_input_on_key_down, djui_inputbox_on_key_up);
    djui_interactable_hook_text_input(&chatInput->base, djui_chat_box_input_on_text_input);
    chatBox->chatInput = chatInput;

    gDjuiChatBox = chatBox;
    djui_chat_box_set_focus_style();
    return chatBox;
}
