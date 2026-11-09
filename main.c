#include <gb/gb.h>
#include <stdint.h>
#include <rand.h>

#include "game.h"
#include "screen.h"
#include "sfx.h"
#include "player.h"
#include "map.h"
#include "battle.h"
#include "save.h"

GameState game_state;
uint8_t current_area;

static uint8_t prev_joy;
static uint8_t banner_timer;
static uint8_t enemy_move_timer;
static uint8_t enemy_cooldown;

/* -------------------------------------------------------------------------
 * จัดวางสไปรท์บนแผนที่
 * ---------------------------------------------------------------------- */
static void place_overworld_sprites(void) {
    set_sprite_tile(0, SPR_HERO);
    move_sprite(0, px, py);

    set_sprite_tile(1, enemy_sprite_tile);
    move_sprite(1, enemy_px, enemy_py);

    if (npc_active) {
        set_sprite_tile(2, SPR_ELDER);
        move_sprite(2, npc_px, npc_py);
    } else {
        hide_sprite(2);
    }
}

static void show_area_banner(void) {
    text_box(0, 1, SCREEN_W, 2);
    text_put(1, 1, map_area_act(current_area));
    text_put(1, 2, map_area_name(current_area));
    banner_timer = 100;
}

static void start_world(void) {
    map_load(current_area);
    map_spawn_enemy(current_area);
    map_setup_npc(current_area);
    place_overworld_sprites();
    SHOW_SPRITES;

    game_state = STATE_OVERWORLD;
    prev_joy = joypad();
    enemy_move_timer = 0;
    enemy_cooldown = 30;
}

/* -------------------------------------------------------------------------
 * หน้า Title / Ending
 * ---------------------------------------------------------------------- */
void enter_title(void) {
    game_state = STATE_TITLE;
    banner_timer = 0;

    hide_sprite(0);
    hide_sprite(1);
    hide_sprite(2);

    text_clear();
    text_put(2, 3, "CHRONO CRYSTAL");
    text_put(1, 5, "A LAND OUT OF TIME");
    text_put(1, 8, "MALAKOR STOLE THE");
    text_put(1, 9, "SACRED CRYSTAL...");

    if (save_exists()) {
        text_put(2, 12, "A: CONTINUE");
        text_put(2, 14, "B: NEW GAME");
    } else {
        text_put(2, 12, "PRESS A TO START");
    }
    text_put(1, 16, "AETHELGARD NEEDS YOU");

    waitpadup();
    prev_joy = 0;
}

void title_update(void) {
    uint8_t keys = joypad();

    if (keys & J_A) {
        uint8_t has_save = save_exists();

        sfx_confirm();
        player_init();
        current_area = AREA_PLAINS;
        if (has_save) {
            load_game();
            if (current_area >= AREA_COUNT) {
                current_area = AREA_PLAINS;
            }
        }
        waitpadup();
        start_world();
        show_area_banner();
    } else if ((keys & J_B) && save_exists()) {
        sfx_confirm();
        save_erase();
        player_init();
        current_area = AREA_PLAINS;
        waitpadup();
        start_world();
        show_area_banner();
    }
}

void enter_ending(void) {
    game_state = STATE_ENDING;

    hide_sprite(0);
    hide_sprite(1);
    hide_sprite(2);
    text_clear();

    sfx_victory();
    text_put(1, 3, "THE CHRONO CRYSTAL");
    text_put(1, 5, "IS RESTORED!");
    text_put(1, 7, "TIME FLOWS AGAIN");
    text_put(1, 8, "ACROSS AETHELGARD.");
    text_put(2, 11, "THANK YOU, HERO!");
    text_put(2, 14, "PRESS A");
}

void ending_update(void) {
    if (joypad() & J_A) {
        waitpadup();
        player_init();
        current_area = AREA_PLAINS;
        enter_title();
    }
}

/* -------------------------------------------------------------------------
 * Overworld (เดิน, ชนสิ่งกีดขวาง, เปลี่ยน Act, ชนศัตรู)
 * ---------------------------------------------------------------------- */
static void try_move(int8_t dx, int8_t dy) {
    int16_t nx = (int16_t)px + dx;
    int16_t ny = (int16_t)py + dy;

    if (nx < 0 || ny < 0 || nx > 152 || ny > 136) {
        return;
    }
    if (map_is_solid_px((uint8_t)nx, (uint8_t)ny)) {
        return;
    }
    px = (uint8_t)nx;
    py = (uint8_t)ny;
}

static void move_enemy(void) {
    uint8_t nx = enemy_px;
    uint8_t ny = enemy_py;

    if (enemy_px < px) {
        nx++;
    } else if (enemy_px > px) {
        nx--;
    }
    if (enemy_py < py) {
        ny++;
    } else if (enemy_py > py) {
        ny--;
    }

    if (nx <= 152 && ny <= 136 && !map_is_solid_px(nx, ny)) {
        enemy_px = nx;
        enemy_py = ny;
    }
}

static void change_area(int8_t direction) {
    uint8_t next = (uint8_t)((int8_t)current_area + direction);

    if (next >= AREA_COUNT) {
        return;
    }
    current_area = next;

    /* เข้าจากด้านตรงข้ามเสมอ และอยู่บนทางเดินแถว 8-9 */
    px = (direction > 0) ? 8 : 152;
    if (py < 64) {
        py = 64;
    } else if (py > 72) {
        py = 72;
    }

    sfx_confirm();
    map_spawn_enemy(current_area);
    map_setup_npc(current_area);

    map_load(current_area);
    place_overworld_sprites();
    game_state = STATE_OVERWORLD;
    enemy_move_timer = 0;
    enemy_cooldown = 40;
    prev_joy = joypad();

    show_area_banner();
    save_game();
}

static void talk_to_elder(void) {
    player.hp = player.max_hp;
    player.mp = player.max_mp;
    sfx_heal();

    text_box(0, 13, SCREEN_W, 4);
    text_put(1, 13, "ELDER: REST, HERO.");
    text_put(1, 14, "THE CRYSTAL CALLS.");
    text_put(1, 15, "HP AND MP RESTORED.");
    wait_a_press();

    map_draw_row(13);
    map_draw_row(14);
    map_draw_row(15);
    map_draw_row(16);

    save_game();
    prev_joy = joypad();
}

void enter_overworld(void) {
    map_load(current_area);
    map_setup_npc(current_area);
    place_overworld_sprites();
    SHOW_SPRITES;

    game_state = STATE_OVERWORLD;
    enemy_move_timer = 0;
    enemy_cooldown = 45;
    prev_joy = joypad();
}

void overworld_update(void) {
    uint8_t keys = joypad();
    uint8_t hit = (uint8_t)(keys & ~prev_joy);

    prev_joy = keys;

    if (hit & J_SELECT) {
        sfx_confirm();
        enter_status_menu();
        return;
    }

    if (hit & J_A) {
        if (npc_active &&
            ABS_DIFF(px, npc_px) <= 10 &&
            ABS_DIFF(py, npc_py) <= 10) {
            talk_to_elder();
            return;
        }
    }

    /* Phase 3: เดินชนกำแพง/น้ำ/ต้นไม้ไม่ได้ */
    if (keys & J_UP)    { try_move(0, -1); }
    if (keys & J_DOWN)  { try_move(0, 1); }
    if (keys & J_LEFT)  { try_move(-1, 0); }
    if (keys & J_RIGHT) { try_move(1, 0); }

    move_sprite(0, px, py);

    /* ทางออกระหว่าง Act */
    if ((keys & J_RIGHT) && px >= 152 && current_area < AREA_CASTLE) {
        change_area(1);
        return;
    }
    if ((keys & J_LEFT) && px <= 8 && current_area > AREA_PLAINS) {
        change_area(-1);
        return;
    }

    /* ศัตรูเดินไล่ผู้เล่น (บอสอยู่กับที่) */
    if (enemy_cooldown) {
        enemy_cooldown--;
    } else if (current_area != AREA_CASTLE) {
        enemy_move_timer++;
        if (enemy_move_timer >= 8) {
            enemy_move_timer = 0;
            move_enemy();
        }
    }
    move_sprite(1, enemy_px, enemy_py);

    /* ชนศัตรู -> เข้าฉากต่อสู้ */
    if (enemy_cooldown == 0 &&
        ABS_DIFF(px, enemy_px) <= 8 &&
        ABS_DIFF(py, enemy_py) <= 8) {
        sfx_attack();
        enter_battle();
        return;
    }

    /* ลบป้ายชื่อพื้นที่หลังแสดงครบเวลา */
    if (banner_timer) {
        banner_timer--;
        if (!banner_timer) {
            map_draw_row(1);
            map_draw_row(2);
        }
    }
}

/* -------------------------------------------------------------------------
 * เมนูสถานะ
 * ---------------------------------------------------------------------- */
void enter_status_menu(void) {
    uint8_t x;

    game_state = STATE_MENU;
    hide_sprite(0);
    hide_sprite(1);
    hide_sprite(2);
    text_clear();

    text_put(2, 0, "=== STATUS ===");
    text_put(2, 1, map_area_act(current_area));
    x = text_put(2, 2, "LEVEL ");
    text_put_u16(x, 2, player.level);

    x = text_put(2, 4, "HP ");
    x = text_put_u16(x, 4, player.hp);
    x = text_put(x, 4, "/");
    text_put_u16(x, 4, player.max_hp);

    x = text_put(2, 5, "MP ");
    x = text_put_u16(x, 5, player.mp);
    x = text_put(x, 5, "/");
    text_put_u16(x, 5, player.max_mp);

    text_put(2, 7, "ATTACK");
    text_put_u16(11, 7, player.attack);
    text_put(2, 8, "DEFENSE");
    text_put_u16(11, 8, player.defense);

    x = text_put(2, 9, "EXP ");
    x = text_put_u16(x, 9, player.exp);
    x = text_put(x, 9, "/");
    text_put_u16(x, 9, player.next_level_exp);

    text_put(2, 10, "POTIONS");
    text_put_u16(11, 10, player.potions);
    text_put(2, 11, "WINS");
    text_put_u16(11, 11, player.wins);

    text_put(2, 14, "B: BACK");
}

void menu_update(void) {
    if (joypad() & J_B) {
        waitpadup();
        enter_overworld();
    }
}

/* -------------------------------------------------------------------------
 * Entry point
 * ---------------------------------------------------------------------- */
void main(void) {
    initrand((uint16_t)DIV_REG);

    screen_init();
    sfx_init();
    player_init();
    current_area = AREA_PLAINS;

    enter_title();

    while (1) {
        switch (game_state) {
            case STATE_TITLE:
                title_update();
                break;
            case STATE_OVERWORLD:
                overworld_update();
                break;
            case STATE_BATTLE:
                battle_update();
                break;
            case STATE_MENU:
                menu_update();
                break;
            case STATE_ENDING:
                ending_update();
                break;
            default:
                break;
        }
        bgm_update();
        wait_vbl_done();
    }
}
