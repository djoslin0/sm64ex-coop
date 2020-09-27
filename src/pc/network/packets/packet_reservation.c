#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"
#include "behavior_table.h"
#include "course_table.h"
#include "src/game/interaction.h"
#include "src/engine/math_util.h"

#define RESERVATION_COUNT 10

void network_send_reservation_request(void) {
    assert(gNetworkType == NT_CLIENT);

    struct Packet p;
    packet_init(&p, PACKET_RESERVATION_REQUEST, true, false);
    network_send(&p);
}

void network_receive_reservation_request(UNUSED struct Packet* p) {
    assert(gNetworkType == NT_SERVER);
    network_send_reservation();
}

void network_send_reservation(void) {
    assert(gNetworkType == NT_SERVER);
    u8 clientPlayerIndex = 1; // two-player hack

    // find all reserved objects
    u8 reservedObjs[RESERVATION_COUNT] = { 0 };
    u16 reservedIndex = 0;
    for (u16 i = 1; i < MAX_SYNC_OBJECTS; i++) {
        if (gSyncObjects[i].reserved == clientPlayerIndex) {
            reservedObjs[reservedIndex++] = i;
            if (reservedIndex >= RESERVATION_COUNT) { break; }
        }
    }

    if (reservedIndex < RESERVATION_COUNT) {
        // reserve the rest
        for (u16 i = MAX_SYNC_OBJECTS - 1; i > 0; i--) {
            if (gSyncObjects[i].o != NULL) { continue; }
            if (gSyncObjects[i].reserved != 0) { continue; }
            gSyncObjects[i].reserved = clientPlayerIndex;
            reservedObjs[reservedIndex++] = i;
            if (reservedIndex >= RESERVATION_COUNT) { break; }
        }
    }

    struct Packet p;
    packet_init(&p, PACKET_RESERVATION, true, false);
    packet_write(&p, reservedObjs, sizeof(u8) * RESERVATION_COUNT);
    network_send(&p);
}

void network_receive_reservation(struct Packet* p) {
    assert(gNetworkType == NT_CLIENT);
    u8 clientPlayerIndex = 1; // two-player hack

    // find all reserved objects
    u8 reservedObjs[RESERVATION_COUNT] = { 0 };
    packet_read(p, reservedObjs, sizeof(u8) * RESERVATION_COUNT);

    for (u16 i = 0; i < RESERVATION_COUNT; i++) {
        u16 index = reservedObjs[i];
        if (index == 0) { continue; }
        if (gSyncObjects[index].o != NULL) { continue; }
        gSyncObjects[index].reserved = clientPlayerIndex;
    }
}
