// king_bobomb.c.inc

struct MarioState* king_bobomb_nearest_mario_state() {
    struct MarioState* nearest = NULL;
    f32 nearestDist = 0;
    u8 checkActive = TRUE;
    do {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (checkActive && !is_player_active(&gMarioStates[i])) { continue; }
            float ydiff = (o->oPosY - gMarioStates[i].marioObj->oPosY);
            if (ydiff >= 1200) { continue; }

            float dist = dist_between_objects(o, gMarioStates[i].marioObj);
            if (nearest == NULL || dist < nearestDist) {
                nearest = &gMarioStates[i];
                nearestDist = dist;
            }
        }
        if (!checkActive) { break; }
        checkActive = FALSE;
    } while (nearest == NULL);

    return nearest;
}

// Copy of geo_update_projectile_pos_from_parent
Gfx *geo_update_held_mario_pos(s32 run, UNUSED struct GraphNode *node, Mat4 mtx) {
    Mat4 sp20;
    struct Object *sp1C;

    if (run == TRUE) {
        sp1C = (struct Object *) gCurGraphNodeObject;
        if (sp1C->prevObj != NULL) {
            create_transformation_from_matrices(sp20, mtx, *gCurGraphNodeCamera->matrixPtr);
            obj_update_pos_from_parent_transformation(sp20, sp1C->prevObj);
            obj_set_gfx_pos_from_pos(sp1C->prevObj);
        }
    }
    return NULL;
}

void bhv_bobomb_anchor_mario_loop(void) {
    common_anchor_mario_behavior(50.0f, 50.0f, 64);
}

u8 king_bobomb_act_0_continue_dialog(void) { return o->oAction == 0 && o->oSubAction != 0; }

void king_bobomb_act_0(void) {
#ifndef VERSION_JP
    o->oForwardVel = 0;
    o->oVelY = 0;
#endif
    struct MarioState* marioState = nearest_mario_state_to_object(o);
    if (o->oSubAction == 0) {
        cur_obj_become_intangible();
        gSecondCameraFocus = o;
        cur_obj_init_animation_with_sound(5);
        cur_obj_set_pos_to_home();
        o->oHealth = 3;
        if (should_start_or_continue_dialog(marioState, o) && cur_obj_can_mario_activate_textbox_2(&gMarioStates[0], 500.0f, 100.0f)) {
            o->oSubAction++;
            func_8031FFB4(SEQ_PLAYER_LEVEL, 60, 40);
        }
    } else if (should_start_or_continue_dialog(marioState, o) && cur_obj_update_dialog_with_cutscene(&gMarioStates[0], 2, 1, CUTSCENE_DIALOG, DIALOG_017, king_bobomb_act_0_continue_dialog)) {
        o->oAction = 2;
        o->oFlags |= OBJ_FLAG_HOLDABLE;
    }
}

int mario_is_far_below_object(f32 arg0) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!is_player_active(&gMarioStates[i])) { continue; }
        if (arg0 >= o->oPosY - gMarioStates[i].marioObj->oPosY) { return FALSE; }
    }
    return TRUE;
}

void king_bobomb_act_2(void) {
    cur_obj_become_tangible();
    if (o->oPosY - o->oHomeY < -100.0f) { // Thrown off hill
        o->oAction = 5;
        cur_obj_become_intangible();
    }
    if (o->oKingBobombUnk100 == 0) {
        if (cur_obj_check_anim_frame(15))
            cur_obj_shake_screen(SHAKE_POS_SMALL);
        if (cur_obj_init_animation_and_check_if_near_end(4))
            o->oKingBobombUnk100++;
    } else {
        if (o->oKingBobombUnk100 == 1) {
            cur_obj_init_animation_and_anim_frame(11, 7);
            o->oKingBobombUnk100 = 2;
        } else
            cur_obj_init_animation_with_sound(11);
        if (o->oKingBobombUnk108 == 0) {
            o->oForwardVel = 3.0f;
            struct MarioState* marioState = king_bobomb_nearest_mario_state();
            if (marioState != NULL) {
                int angleToPlayer = obj_angle_to_object(o, marioState->marioObj);
                cur_obj_rotate_yaw_toward(angleToPlayer, 0x100);
            }
        } else {
            o->oForwardVel = 0.0f;
            o->oKingBobombUnk108--;
        }
    }
    if (cur_obj_check_grabbed_mario())
        o->oAction = 3;
    if (mario_is_far_below_object(1200.0f)) {
        o->oAction = 0;
        stop_background_music(SEQUENCE_ARGS(4, SEQ_EVENT_BOSS));
    }
}

void king_bobomb_act_3(void) {
    if (o->oSubAction == 0) {
        o->oForwardVel = 0;
        o->oKingBobombUnk104 = 0;
        o->oKingBobombUnkFC = 0;
        if (o->oTimer == 0)
            cur_obj_play_sound_2(SOUND_OBJ_UNKNOWN3);
        if (cur_obj_init_animation_and_check_if_near_end(0)) {
            o->oSubAction++;
            cur_obj_init_animation_and_anim_frame(1, 0);
        }
    } else {
        if (o->oSubAction == 1) {
            cur_obj_init_animation_with_sound(1);
            o->oKingBobombUnkFC += player_performed_grab_escape_action();
            print_debug_bottom_up("%d", o->oKingBobombUnkFC);
            if (o->oKingBobombUnkFC > 10) {
                o->oKingBobombUnk88 = 3;
                o->oAction = 2;
                o->oKingBobombUnk108 = 35;
                o->oInteractStatus &= ~(INT_STATUS_GRABBED_MARIO);
            } else {
                o->oForwardVel = 3.0f;
                if (o->oKingBobombUnk104 > 20 && cur_obj_rotate_yaw_toward(0, 0x400)) {
                    o->oSubAction++;
                    cur_obj_init_animation_and_anim_frame(9, 22);
                }
            }
            o->oKingBobombUnk104++;
        } else {
            cur_obj_init_animation_with_sound(9);
            if (cur_obj_check_anim_frame(31)) {
                o->oKingBobombUnk88 = 2;
                cur_obj_play_sound_2(SOUND_OBJ_UNKNOWN4);
            } else if (cur_obj_check_if_near_animation_end()) {
                o->oAction = 1;
                o->oInteractStatus &= ~(INT_STATUS_GRABBED_MARIO);
            }
        }
    }
}

void king_bobomb_act_1(void) {
    o->oForwardVel = 0;
    o->oVelY = 0;
    cur_obj_init_animation_with_sound(11);
    struct MarioState* marioState = king_bobomb_nearest_mario_state();
    int distanceToPlayer = (marioState != NULL) ? dist_between_objects(o, marioState->marioObj) : 3000;

    if (marioState != NULL) {
        int angleToPlayer = obj_angle_to_object(o, marioState->marioObj);
        o->oMoveAngleYaw = approach_s16_symmetric(o->oMoveAngleYaw, angleToPlayer, 512);
    }

    if (distanceToPlayer < 2500.0f)
        o->oAction = 2;

    if (mario_is_far_below_object(1200.0f)) {
        o->oAction = 0;
        stop_background_music(SEQUENCE_ARGS(4, SEQ_EVENT_BOSS));
    }
}

void king_bobomb_act_6(void) {
    if (o->oSubAction == 0) {
        if (o->oTimer == 0) {
            o->oKingBobombUnk104 = 0;
            cur_obj_play_sound_2(SOUND_OBJ_KING_BOBOMB);
            cur_obj_play_sound_2(SOUND_OBJ2_KING_BOBOMB_DAMAGE);
            cur_obj_shake_screen(SHAKE_POS_SMALL);
            spawn_mist_particles_variable(0, 0, 100.0f);
            o->oInteractType = 8;
            cur_obj_become_tangible();
        }
        if (cur_obj_init_animation_and_check_if_near_end(2))
            o->oKingBobombUnk104++;
        if (o->oKingBobombUnk104 > 3) {
            o->oSubAction++;
            ; // Needed to match
        }
    } else {
        if (o->oSubAction == 1) {
            if (cur_obj_init_animation_and_check_if_near_end(10)) {
                o->oSubAction++;
                o->oInteractType = 2;
                cur_obj_become_intangible();
            }
        } else {
            cur_obj_init_animation_with_sound(11);
            struct MarioState* marioState = king_bobomb_nearest_mario_state();
            if (marioState != NULL) {
                int angleToPlayer = obj_angle_to_object(o, marioState->marioObj);
                if (cur_obj_rotate_yaw_toward(angleToPlayer, 0x800) == 1) {
                    o->oAction = 2;
                }
            }
        }
    }
}

u8 king_bobomb_act_7_continue_dialog(void) { return o->oAction == 7; }

void king_bobomb_act_7(void) {
    cur_obj_init_animation_with_sound(2);

    struct MarioState* marioState = nearest_mario_state_to_object(o);
    u8 updateDialog = should_start_or_continue_dialog(marioState, o) || (gMarioStates[0].pos[1] >= o->oPosY - 100.0f);
    if (updateDialog && cur_obj_update_dialog_with_cutscene(&gMarioStates[0], 2, 2, CUTSCENE_DIALOG, DIALOG_116, king_bobomb_act_7_continue_dialog)) {
        o->oAction = 8;
        network_send_object(o);
    }
}

void king_bobomb_act_8(void) {
    if (!(o->header.gfx.node.flags & GRAPH_RENDER_INVISIBLE)) {
        create_sound_spawner(SOUND_OBJ_KING_WHOMP_DEATH);
        cur_obj_hide();
        cur_obj_become_intangible();
        spawn_mist_particles_variable(0, 0, 200.0f);
        spawn_triangle_break_particles(20, 138, 3.0f, 4);
        cur_obj_shake_screen(SHAKE_POS_SMALL);
#ifndef VERSION_JP
        cur_obj_spawn_star_at_y_offset(2000.0f, 4500.0f, -4500.0f, 200.0f);
#else
        o->oPosY += 100.0f;
        spawn_default_star(2000.0f, 4500.0f, -4500.0f);
#endif
    }
    if (o->oTimer == 60)
        stop_background_music(SEQUENCE_ARGS(4, SEQ_EVENT_BOSS));
}

void king_bobomb_act_4(void) { // bobomb been thrown
    if (o->oPosY - o->oHomeY > -100.0f) { // not thrown off hill
        if (o->oMoveFlags & OBJ_MOVE_LANDED) {
            o->oHealth--;
            o->oForwardVel = 0;
            o->oVelY = 0;
            cur_obj_play_sound_2(SOUND_OBJ_KING_BOBOMB);
            if (o->oHealth)
                o->oAction = 6;
            else
                o->oAction = 7;
        }
    } else {
        if (o->oSubAction == 0) {
            if (o->oMoveFlags & OBJ_MOVE_ON_GROUND) {
                o->oForwardVel = 0;
                o->oVelY = 0;
                o->oSubAction++;
            } else if (o->oMoveFlags & OBJ_MOVE_LANDED)
                cur_obj_play_sound_2(SOUND_OBJ_KING_BOBOMB);
        } else {
            if (cur_obj_init_animation_and_check_if_near_end(10))
                o->oAction = 5; // Go back to top of hill
            o->oSubAction++;
        }
    }
}

u8 king_bobomb_act_5_continue_dialog(void) { return o->oAction == 5 && o->oSubAction == 4; }

void king_bobomb_act_5(void) { // bobomb returns home
    struct MarioState* marioState = nearest_mario_state_to_object(o);
    switch (o->oSubAction) {
        case 0:
            if (o->oTimer == 0)
                cur_obj_play_sound_2(SOUND_OBJ_KING_BOBOMB_JUMP);
            o->oKingBobombUnkF8 = 1;
            cur_obj_init_animation_and_extend_if_at_end(8);
            o->oMoveAngleYaw =  cur_obj_angle_to_home();
            if (o->oPosY < o->oHomeY)
                o->oVelY = 100.0f;
            else {
                arc_to_goal_pos(&o->oHomeX, &o->oPosX, 100.0f, -4.0f);
                o->oSubAction++;
            }
            break;
        case 1:
            cur_obj_init_animation_and_extend_if_at_end(8);
            if (o->oVelY < 0 && o->oPosY < o->oHomeY) {
                o->oPosY = o->oHomeY;
                o->oVelY = 0;
                o->oForwardVel = 0;
                o->oGravity = -4.0f;
                o->oKingBobombUnkF8 = 0;
                cur_obj_init_animation_with_sound(7);
                cur_obj_play_sound_2(SOUND_OBJ_KING_BOBOMB);
                cur_obj_shake_screen(SHAKE_POS_SMALL);
                o->oSubAction++;
            }
            break;
        case 2:
            if (cur_obj_init_animation_and_check_if_near_end(7))
                o->oSubAction++;
            break;
        case 3:
            if (mario_is_far_below_object(1200.0f)) {
                o->oAction = 0;
                stop_background_music(SEQUENCE_ARGS(4, SEQ_EVENT_BOSS));
            }
            if (should_start_or_continue_dialog(marioState, o) && cur_obj_can_mario_activate_textbox_2(&gMarioStates[0], 500.0f, 100.0f))
                o->oSubAction++;
            break;
        case 4:
            if (should_start_or_continue_dialog(marioState, o) && cur_obj_update_dialog_with_cutscene(&gMarioStates[0], 2, 1, CUTSCENE_DIALOG, DIALOG_128, king_bobomb_act_5_continue_dialog))
                o->oAction = 2;
            break;
    }
}

void (*sKingBobombActions[])(void) = {
    king_bobomb_act_0, king_bobomb_act_1, king_bobomb_act_2, king_bobomb_act_3, king_bobomb_act_4,
    king_bobomb_act_5, king_bobomb_act_6, king_bobomb_act_7, king_bobomb_act_8,
};
struct SoundState sKingBobombSoundStates[] = {
    { 0, 0, 0, NO_SOUND },
    { 1, 1, 20, SOUND_OBJ_POUNDING1_HIGHPRIO },
    { 0, 0, 0, NO_SOUND },
    { 0, 0, 0, NO_SOUND },
    { 1, 15, -1, SOUND_OBJ_POUNDING1_HIGHPRIO },
    { 0, 0, 0, NO_SOUND },
    { 0, 0, 0, NO_SOUND },
    { 0, 0, 0, NO_SOUND },
    { 0, 0, 0, NO_SOUND },
    { 1, 33, -1, SOUND_OBJ_POUNDING1_HIGHPRIO },
    { 0, 0, 0, NO_SOUND },
    { 1, 1, 15, SOUND_OBJ_POUNDING1_HIGHPRIO },
};

void king_bobomb_move(void) {
    cur_obj_update_floor_and_walls();
    if (o->oKingBobombUnkF8 == 0)
        cur_obj_move_standard(-78);
    else
        cur_obj_move_using_fvel_and_gravity();
    cur_obj_call_action_function(sKingBobombActions);
    exec_anim_sound_state(sKingBobombSoundStates);
#ifndef NODRAWINGDISTANCE
    struct Object* player = nearest_player_to_object(o);
    int distanceToPlayer = dist_between_objects(o, player);
    if (distanceToPlayer < 5000.0f)
#endif
        cur_obj_enable_rendering();
#ifndef NODRAWINGDISTANCE
    else
        cur_obj_disable_rendering();
#endif
}

u8 king_bobomb_ignore_if_true(void) { return o->oAction == 8; }

void bhv_king_bobomb_loop(void) {
    if (!network_sync_object_initialized(o)) {
        struct SyncObject* so = network_init_object(o, 4000.0f);
        so->ignore_if_true = king_bobomb_ignore_if_true;
        network_init_object_field(o, &o->oKingBobombUnk88);
        network_init_object_field(o, &o->oFlags);
        network_init_object_field(o, &o->oHealth);
    }

    f32 sp34 = 20.0f;
    f32 sp30 = 50.0f;
    UNUSED u8 pad[8];
    o->oInteractionSubtype |= INT_SUBTYPE_GRABS_MARIO;
    switch (o->oHeldState) {
        case HELD_FREE:
            king_bobomb_move();
            break;
        case HELD_HELD:
            cur_obj_unrender_and_reset_state(6, 1);
            break;
        case HELD_THROWN:
        case HELD_DROPPED:
            cur_obj_get_thrown_or_placed(sp34, sp30, 4);
            cur_obj_become_intangible();
            o->oPosY += 20.0f;
            break;
    }
    o->oInteractStatus = 0;
}
