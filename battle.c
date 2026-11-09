#include <gb/gb.h>
#include <stdint.h>
#include <rand.h>

#include "game.h"
#include "player.h"
#include "map.h"
#include "battle.h"
#include "screen.h"
#include "sfx.h"
#include "save.h"

Monster monster;

/* ค่าคงที่ของมอนสเตอร์แต่ละชนิด */
static const Monster monster_table[] = {
    /* name, level, hp, max_hp, atk, def, exp, sprite, boss */
    {"SLIME",    1, 12, 12,  4, 1,   8, SPR_SLIME,    0},
    {"SKELETON", 3, 22, 22,  6, 2,  20, SPR_SKELETON, 0},
    {"BAT",      2, 16, 16,  5, 1,  15, SPR_BAT,      0},
    {"MALAKOR",  8, 80, 80, 12, 6, 100, SPR_BOSS,     1}
};

static uint8_t battle_ready;
static uint8_t cursor;
static uint8_t prev_keys;

#define MENU_ROW0 13
#define MSG_ROW0  6

static uint16_t calc_damage(uint16_t attack, uint16_t defense) {
    if (attack <= defense) {
        return 1;
    }
    return (uint16_t)(attack - defense);
}

static void battle_draw_enemy_hp(void) {
    uint8_t x;

    text_clear_line(1);
    x = text_put(1, 1, "HP ");
    x = text_put_u16(x, 1, monster.hp);
    x = text_put(x, 1, "/");
    text_put_u16(x, 1, monster.max_hp);
}

static void battle_draw_header(void) {
    uint8_t x;

    text_clear_line(0);

    x = text_put(1, 0, (monster.is_boss ? "" : "WILD "));
    x = text_put(x, 0, monster.name);
    x = text_put(x, 0, " LV ");
    text_put_u16(x, 0, monster.level);

    battle_draw_enemy_hp();
}

static void battle_draw_stats(void) {
    uint8_t x;

    text_clear_line(10);
    text_clear_line(11);

    x = text_put(0, 10, "HERO LV ");
    text_put_u16(x, 10, player.level);

    x = text_put(0, 11, "HP");
    x = text_put_u16(x, 11, player.hp);
    x = text_put(x, 11, "/");
    x = text_put_u16(x, 11, player.max_hp);
    x = text_put(x, 11, " MP");
    x = text_put_u16(x, 11, player.mp);
    x = text_put(x, 11, "/");
    text_put_u16(x, 11, player.max_mp);
}

static void battle_draw_menu(void) {
    uint8_t x;

    text_clear_line(MENU_ROW0 + 0);
    text_clear_line(MENU_ROW0 + 1);
    text_clear_line(MENU_ROW0 + 2);
    text_clear_line(MENU_ROW0 + 3);

    text_put(2, MENU_ROW0 + 0, "FIGHT");

    x = text_put(2, MENU_ROW0 + 1, "MAGIC ");
    x = text_put_u16(x, MENU_ROW0 + 1, 3);
    text_put(x, MENU_ROW0 + 1, "MP");

    x = text_put(2, MENU_ROW0 + 2, "ITEM X");
    text_put_u16(x, MENU_ROW0 + 2, player.potions);

    text_put(2, MENU_ROW0 + 3, "RUN");

    text_put(1, (uint8_t)(MENU_ROW0 + cursor), ">");
}

static void battle_clear_messages(void) {
    text_clear_line(MSG_ROW0);
    text_clear_line(MSG_ROW0 + 1);
}

static void battle_message(const char *line1, const char *line2) {
    battle_clear_messages();
    text_put(1, MSG_ROW0, line1);
    if (line2 && *line2) {
        text_put(1, MSG_ROW0 + 1, line2);
    }
    wait_a_press();
    battle_clear_messages();
}

static void battle_show_damage(const char *label, uint16_t damage) {
    battle_clear_messages();
    text_put(1, MSG_ROW0, label);
    text_put(1, MSG_ROW0 + 1, "DAMAGE ");
    text_put_u16(8, MSG_ROW0 + 1, damage);
    wait_a_press();
    battle_clear_messages();
}

/* ---------- จบการต่อสู้ ---------- */

static void battle_faint(void) {
    sfx_hurt();
    text_clear();
    hide_sprite(0);
    hide_sprite(1);
    text_put(2, 5, "YOU FAINTED...");
    text_put(2, 7, "RETURNING HOME");
    wait_a_press();

    player.hp = player.max_hp;
    player.mp = player.max_mp;

    if (current_area == AREA_PLAINS) {
        px = 32;
        py = 80;
    } else {
        px = 8;
        py = 68;
    }
    map_spawn_enemy(current_area);
    enter_overworld();
}

static void battle_victory(void) {
    uint8_t levels;

    sfx_victory();
    text_clear();
    hide_sprite(1);

    text_put(2, 4, "VICTORY!");
    text_put(2, 6, "EXP +");
    text_put_u16(7, 6, monster.exp_reward);

    player.wins++;
    levels = player_gain_exp(monster.exp_reward);

    if (levels) {
        sfx_levelup();
        text_put(2, 8, "LEVEL UP!");
        text_put(2, 9, "LEVEL ");
        text_put_u16(8, 9, player.level);
    }

    if (!monster.is_boss && (rand() & 3) == 0) {
        player.potions++;
        text_put(2, 11, "FOUND A POTION!");
    }
    text_put(2, 15, "PRESS A TO GO ON");

    wait_a_press();

    if (monster.is_boss) {
        save_erase();
        enter_ending();
        return;
    }
    map_spawn_enemy(current_area);
    save_game();
    enter_overworld();
}

/* ---------- เทิร์นของศัตรู ---------- */

static void enemy_attack(void) {
    uint16_t damage = calc_damage(monster.attack, player.defense);
    uint8_t x;

    player_take_damage(damage);
    sfx_hurt();
    battle_draw_stats();

    battle_clear_messages();
    x = text_put(1, MSG_ROW0, monster.name);
    text_put(x, MSG_ROW0, " ATTACKS!");
    text_put(1, MSG_ROW0 + 1, "DAMAGE ");
    text_put_u16(8, MSG_ROW0 + 1, damage);
    wait_a_press();
    battle_clear_messages();

    if (player.hp == 0) {
        battle_faint();
    }
}

/* ---------- เทิร์นของผู้เล่น ---------- */

static void player_attack_with(uint16_t attack, const char *label) {
    uint16_t damage = calc_damage(attack, monster.defense);

    sfx_attack();
    if (damage >= monster.hp) {
        monster.hp = 0;
    } else {
        monster.hp -= damage;
    }
    battle_draw_enemy_hp();
    battle_show_damage(label, damage);

    if (monster.hp == 0) {
        battle_victory();
        return;
    }
    enemy_attack();
}

static void do_fight(void) {
    player_attack_with(player.attack, "HERO ATTACKS!");
}

static void do_magic(void) {
    if (player.mp < 3) {
        battle_message("NOT ENOUGH MP!", "");
        return;
    }
    player.mp -= 3;
    battle_draw_stats();
    player_attack_with((uint16_t)(player.attack + 6), "FLAME!");
}

static void do_item(void) {
    if (player.potions == 0) {
        battle_message("NO POTIONS LEFT!", "");
        return;
    }
    if (player.hp == player.max_hp) {
        battle_message("HP IS ALREADY FULL!", "");
        return;
    }
    player.potions--;
    player_heal(12);
    sfx_heal();
    battle_draw_stats();
    battle_message("POTION USED!", "HP RESTORED");
    enemy_attack();
}

static void do_run(void) {
    if (monster.is_boss) {
        battle_message("MALAKOR BLOCKS", "YOUR ESCAPE!");
        enemy_attack();
        return;
    }
    if (rand() & 1) {
        sfx_confirm();
        battle_message("ESCAPED SAFELY!", "");
        map_spawn_enemy(current_area);
        enter_overworld();
    } else {
        battle_message("COULD NOT ESCAPE!", "");
        enemy_attack();
    }
}

static void battle_action(uint8_t choice) {
    switch (choice) {
        case 0: do_fight(); break;
        case 1: do_magic(); break;
        case 2: do_item();  break;
        default: do_run();  break;
    }

    if (game_state != STATE_BATTLE) {
        return;
    }
    battle_draw_enemy_hp();
    battle_draw_stats();
    battle_draw_menu();
    prev_keys = joypad();
}

/* ---------- เริ่ม/อัปเดตฉากต่อสู้ ---------- */

void enter_battle(void) {
    uint8_t kind = enemy_kind;

    if (kind > 3) {
        kind = 0;
    }
    monster = monster_table[kind];
    monster.hp = monster.max_hp;

    battle_ready = 0;
    game_state = STATE_BATTLE;
}

void battle_update(void) {
    uint8_t keys;
    uint8_t hit;

    if (!battle_ready) {
        text_clear();
        hide_sprite(2);
        set_sprite_tile(0, SPR_HERO);
        move_sprite(0, 72, 64);
        set_sprite_tile(1, monster.sprite_tile);
        move_sprite(1, 72, 24);

        cursor = 0;
        battle_draw_header();
        battle_draw_enemy_hp();
        battle_draw_stats();
        battle_draw_menu();

        battle_ready = 1;
        prev_keys = 0;
        return;
    }

    keys = joypad();
    hit = (uint8_t)(keys & ~prev_keys);
    prev_keys = keys;

    if (hit & (J_UP | J_DOWN)) {
        if (hit & J_UP) {
            cursor = (uint8_t)((cursor + 3) & 3);
        } else {
            cursor = (uint8_t)((cursor + 1) & 3);
        }
        sfx_menu();
        battle_draw_menu();
    }

    if (hit & J_A) {
        battle_action(cursor);
    }
}
