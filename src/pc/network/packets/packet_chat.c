#include <stdio.h>
#include "../network.h"
#include "../reservation_area.h"
#include "pc/djui/djui.h"
#include "pc/debuglog.h"

#ifdef DEVELOPMENT
#include "behavior_table.h"

static void print_sync_object_table(void) {
    LOG_INFO("Sync Object Table");
    for (int i = 0; i < MAX_SYNC_OBJECTS; i++) {
        if (gSyncObjects[i].o == NULL) { continue; }
        u16 behaviorId = get_id_from_behavior(gSyncObjects[i].behavior);
        LOG_INFO("%03d: %04X", i, behaviorId);
    }
    LOG_INFO(" ");
}

static void print_network_player_table(void) {
    u8 globalIndex;
    // struct NetworkPlayer* nph = network_player_from_global_index(globalIndex);
// char levelname;
    fprintf (stdout, "\nNetwork Player Table\n");
    fprintf (stdout, "\n%s %5s %5s %8s %8s %8s %8s %8s", "Name", "gID", "lID", "course", "act", "level", "levelName", "area", "valid", "model", "palette", "palette name", "\n");
    for (int i = 0; i < MAX_PLAYERS; i++) {
        
        struct NetworkPlayer* np = &gNetworkPlayers[i];
        if (!np->connected) { continue; }
        // findlevelname();
char* levelname;
levelname = "Unknown";
if (np->currLevelNum == 17) {
levelname = "Bowser In the Dark-World";
} else if (np->currLevelNum == 4) {
levelname = "Big Boo's Haunt";
} else if (np->currLevelNum == 5) {
levelname = "Cool, Cool Mountain";
} else if (np->currLevelNum == 6) {
levelname = "Inside Peach's Castle";
} else if (np->currLevelNum == 7) {
levelname = "Hazy Maze Cave";
} else if (np->currLevelNum == 8) {
levelname = "Shifting Sand Land";
} else if (np->currLevelNum == 9) {
levelname = "Bob-omb Battlefield";
} else if (np->currLevelNum == 10) {
levelname = "Snowman's Land";
} else if (np->currLevelNum == 11) {
levelname = "Wet-Dry World";
} else if (np->currLevelNum == 12) {
levelname = "Jolly Roger Bay";
} else if (np->currLevelNum == 13) {
levelname = "Tiny-Huge Island";
} else if (np->currLevelNum == 14) {
levelname = "Tick Tock Clock";
} else if (np->currLevelNum == 15) {
levelname = "Rainbow Ride";
} else if (np->currLevelNum == 16) {
levelname = "Outside the Castle";
} else if (np->currLevelNum == 18) {
levelname = "Vanish Cap Under the Moat";
} else if (np->currLevelNum == 19) {
levelname = "Bowser in the Fire Sea";
} else if (np->currLevelNum == 20) {
levelname = "The Secret Aquarium";
} else if (np->currLevelNum == 21) {
levelname = "Bowser in the Sky";
} else if (np->currLevelNum == 22) {
levelname = "Lethal Lava Land";
} else if (np->currLevelNum == 23) {
levelname = "Dire Dire Docks";
} else if (np->currLevelNum == 24) {
levelname = "Whomp's Fortress";
} else if (np->currLevelNum == 25) {
levelname = "The End Image";
} else if (np->currLevelNum == 26) {
levelname = "Castle Courtyard";
} else if (np->currLevelNum == 27) {
levelname = "The Princess's Secret Slide";
} else if (np->currLevelNum == 28) {
levelname = "Cavern of the Metal Cap";
} else if (np->currLevelNum == 29) {
levelname = "Tower of the Wing Cap";
} else if (np->currLevelNum == 30) {
levelname = "Bowser in the Dark World (Boss)";
} else if (np->currLevelNum == 31) {
levelname = "Wing Mario Over the Rainbow";
} else if (np->currLevelNum == 33) {
levelname = "Bowser in the Fire Sea (Boss)";
} else if (np->currLevelNum == 34) {
levelname = "Bowser in the Sky (Boss)";
} else if (np->currLevelNum == 36) {
levelname = "Tall Tall Mountain";

};
char* modelname;
if (np->modelIndex == 0) {
modelname = "Mario";
} else if (np->modelIndex == 1) {
modelname = "Luigi";
} else if (np->modelIndex == 2) {
modelname = "Toad";
} else if (np->modelIndex == 3) {
modelname = "Waluigi";
};

char* palettename;
if (np->paletteIndex == 0) {
palettename = "Mario";
};
if (np->paletteIndex == 1) {
palettename = "Luigi";
};
if (np->paletteIndex == 2) {
palettename = "Waluigi";
};
if (np->paletteIndex == 3) {
palettename = "Wario";
};
if (np->paletteIndex == 4) {
palettename = "Cobalt";
};
if (np->paletteIndex == 5) {
palettename = "Hot Pink";
};
if (np->paletteIndex == 6) {
palettename = "Seafoam";
};if (np->paletteIndex == 7) {
palettename = "Lilac";
};
if (np->paletteIndex == 8) {
palettename = "Copper";
};
if (np->paletteIndex == 9) {
palettename = "Azure";
};
if (np->paletteIndex == 10) {
palettename = "Burgundy";
};
if (np->paletteIndex == 11) {
palettename = "Mint";
};
if (np->paletteIndex == 12) {
palettename = "Eggplant";
};
if (np->paletteIndex == 13) {
palettename = "Orange";
};
if (np->paletteIndex == 14) {
palettename = "Arctic";
};
if (np->paletteIndex == 15) {
palettename = "Fire";
};




        fprintf (stdout, "\n%s %5d %5d %8d %8d %8d %s %8d %8d %8d %s %8d %s", np->name, np->globalIndex, np->localIndex, np->currCourseNum, np->currActNum, np->currLevelNum, levelname, np->currAreaIndex, np->currAreaSyncValid, np->modelIndex, modelname, np->paletteIndex, palettename);
    fprintf (stdout, " \n");
    // char* text;
    // char* name = np->name;
    // text = "%s %s %s %s", name, levelname, modelname, palettename;
    // djui_popup_create(text, sizeof(char*));


    u8* rgb = get_player_color(np->paletteIndex, 0);
    char popupMsg[128] = { 0 };
    snprintf(popupMsg, 128, "\\#%02x%02x%02x\\%s\\#dcdcdc\\ %s", rgb[0], rgb[1], rgb[2], np->name, levelname);
    djui_popup_create(popupMsg, 1);


    // fprintf (stdout, "\n%s\n", levelname);
    fprintf (stdout, " \n");

    }
    fprintf (stdout, " \n");
}
#endif

void network_send_chat(char* message, u8 globalIndex) {
    u16 messageLength = strlen(message);
    struct Packet p;
    
    if (strstr(message, "/print"))
{
                print_network_player_table();
    } else {
    fprintf(stdout, message);
    fprintf(stdout, "\n");
    packet_init(&p, PACKET_CHAT, true, PLMT_NONE);
    packet_write(&p, &globalIndex, sizeof(u8));
    packet_write(&p, &messageLength, sizeof(u16));
    packet_write(&p, message, messageLength * sizeof(u8));
    network_send(&p);
    };
    
#ifdef DEVELOPMENT
    print_network_player_table();
    //reservation_area_debug();
    //print_sync_object_table();
#endif
}

void network_receive_chat(struct Packet* p) {
    u16 remoteMessageLength = 0;
    char remoteMessage[256] = { 0 };
    u8 globalIndex;

    packet_read(p, &globalIndex, sizeof(u8));
    packet_read(p, &remoteMessageLength, sizeof(u16));
    if (remoteMessageLength > 256) { remoteMessageLength = 255; }
    packet_read(p, &remoteMessage, remoteMessageLength * sizeof(u8));

    // add the message
    djui_chat_message_create_from(globalIndex, remoteMessage);
    struct NetworkPlayer* np = network_player_from_global_index(globalIndex);
    fprintf(stdout, (np != NULL) ? np->name : "Player");
    fprintf (stdout, ": ");
    fprintf (stdout, remoteMessage);
    fprintf (stdout, " ");
    fprintf( stdout, "\n" );
    
    LOG_INFO("rx chat: %s", remoteMessage);
    /*
#ifdef DEVELOPMENT
    print_network_player_table();
#endif
    */
}
