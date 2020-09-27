#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"
#include "sm64.h"
#include "game/interaction.h"
#include "game/mario.h"
#include "audio/external.h"

#define SET_BIT(val, num) ((((u8)(val)) & 0x01) << (num));
#define GET_BIT(val, num) (((val) >> (num)) & 0x01)

void network_send_player(void) {
    if (gMarioStates[0].marioObj == NULL) { return; }
    u32 heldSyncID = (gMarioStates[0].heldObj != NULL)
                   ? gMarioStates[0].heldObj->oSyncID
                   : 0;
    u32 heldBySyncID = (gMarioStates[0].heldByObj != NULL)
                     ? gMarioStates[0].heldByObj->oSyncID
                     : 0;

    u8 customFlags = SET_BIT((gMarioStates[0].freeze > 0), 0);

    struct Packet p;
    packet_init(&p, PACKET_PLAYER, false);
    packet_write(&p, &gMarioStates[0], sizeof(u32) * 24);
    packet_write(&p, gMarioStates[0].controller, 20);
    packet_write(&p, gMarioStates[0].marioObj->rawData.asU32, sizeof(u32) * 80);
    packet_write(&p, &gMarioStates[0].health, sizeof(s16));
    packet_write(&p, &gMarioStates[0].marioObj->header.gfx.node.flags, sizeof(s16));
    packet_write(&p, &gMarioStates[0].actionState, sizeof(u16));
    packet_write(&p, &gMarioStates[0].actionTimer, sizeof(u16));
    packet_write(&p, &gMarioStates[0].actionArg, sizeof(u32));
    packet_write(&p, &gMarioStates[0].currentRoom, sizeof(s16));
    packet_write(&p, &gMarioStates[0].squishTimer, sizeof(u8));
    packet_write(&p, &gMarioStates[0].peakHeight, sizeof(f32));
    packet_write(&p, &customFlags, sizeof(u8));
    packet_write(&p, &heldSyncID, sizeof(u32));
    packet_write(&p, &heldBySyncID, sizeof(u32));
    network_send(&p);
}

void network_receive_player(struct Packet* p) {
    if (gMarioStates[1].marioObj == NULL) { return; }

    // save previous state
    u32 heldSyncID = 0;
    u32 heldBySyncID = 0;
    u16 playerIndex = gMarioStates[1].playerIndex;
    u8 customFlags = 0;
    u32 oldAction = gMarioStates[1].action;
    u16 oldActionState = gMarioStates[1].actionState;
    u16 oldActionArg = gMarioStates[1].actionArg;
    u32 oldBehParams = gMarioStates[1].marioObj->oBehParams;

    // load mario information from packet
    packet_read(p, &gMarioStates[1], sizeof(u32) * 24);
    packet_read(p, gMarioStates[1].controller, 20);
    packet_read(p, &gMarioStates[1].marioObj->rawData.asU32, sizeof(u32) * 80);
    packet_read(p, &gMarioStates[1].health, sizeof(s16));
    packet_read(p, &gMarioStates[1].marioObj->header.gfx.node.flags, sizeof(s16));
    packet_read(p, &gMarioStates[1].actionState, sizeof(u16));
    packet_read(p, &gMarioStates[1].actionTimer, sizeof(u16));
    packet_read(p, &gMarioStates[1].actionArg, sizeof(u32));
    packet_read(p, &gMarioStates[1].currentRoom, sizeof(s16));
    packet_read(p, &gMarioStates[1].squishTimer, sizeof(u8));
    packet_read(p, &gMarioStates[1].peakHeight, sizeof(f32));
    packet_read(p, &customFlags, sizeof(u8));
    packet_read(p, &heldSyncID, sizeof(u32));
    packet_read(p, &heldBySyncID, sizeof(u32));

    // read custom flags
    gMarioStates[1].freeze = GET_BIT(customFlags, 0);

    // reset player index
    gMarioStates[1].playerIndex = playerIndex;
    gMarioStates[1].marioObj->oBehParams = oldBehParams;

    // reset mario sound play flag so that their jump sounds work
    if (gMarioStates[1].action != oldAction) {
        gMarioStates[1].flags &= ~(MARIO_ACTION_SOUND_PLAYED | MARIO_MARIO_SOUND_PLAYED);
    }

    // find and set their held object
    if (heldSyncID != 0 && gSyncObjects[heldSyncID].o != NULL) {
        // TODO: do we have to move graphics nodes around to make this visible?
        struct Object* heldObj = gSyncObjects[heldSyncID].o;
        if (gMarioStates[0].heldObj == heldObj && gNetworkType == NT_CLIENT) { // two-player hack: needs priority
            mario_drop_held_object(&gMarioStates[0]);
            force_idle_state(&gMarioStates[0]);
        }
        gMarioStates[1].heldObj = heldObj;
        heldObj->oHeldState = HELD_HELD;
        heldObj->heldByPlayerIndex = 1;
    } else {
        gMarioStates[1].heldObj = NULL;
    }

    // find and set their held-by object
    if (heldBySyncID != 0 && gSyncObjects[heldBySyncID].o != NULL) {
        // TODO: do we have to move graphics nodes around to make this visible?
        gMarioStates[1].heldByObj = gSyncObjects[heldBySyncID].o;
    } else {
        gMarioStates[1].heldByObj = NULL;
    }

    // jump kicking: restore action state, otherwise it won't play
    if (gMarioStates[1].action == ACT_JUMP_KICK) {
        gMarioStates[1].actionState = oldActionState;
    }

    // punching:
    if ((gMarioStates[1].action == ACT_PUNCHING || gMarioStates[1].action == ACT_MOVE_PUNCHING)) {
        // play first punching sound, otherwise it will be missed
        if (gMarioStates[1].action != oldAction) {
            play_sound(SOUND_MARIO_PUNCH_YAH, gMarioStates[1].marioObj->header.gfx.cameraToObject);
        }
        // make the first punch large, otherwise it will be missed
        if (gMarioStates[1].actionArg == 2 && oldActionArg == 1) {
            gMarioStates[1].marioBodyState->punchState = (0 << 6) | 4;
        }
    }

    // action changed, reset timer
    if (gMarioStates[1].action != oldAction) {
        gMarioStates[1].actionTimer = 0;
    }
}

void network_update_player(void) {
    network_send_player();
}
