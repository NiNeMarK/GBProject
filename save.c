#include <gb/gb.h>
#include <stdint.h>

#include "game.h"
#include "player.h"
#include "save.h"

#define SAVE_ADDR   ((SaveData *)0xA000)
#define SAVE_MAGIC0 'C'
#define SAVE_MAGIC1 'C'
#define SAVE_VERSION 1

typedef struct {
    uint8_t  magic0;
    uint8_t  magic1;
    uint8_t  version;

    uint8_t  level;
    uint16_t hp;
    uint16_t max_hp;
    uint16_t exp;
    uint16_t next_level_exp;
    uint8_t  attack;
    uint8_t  defense;
    uint8_t  mp;
    uint8_t  max_mp;
    uint8_t  potions;
    uint16_t wins;

    uint8_t  area;
    uint8_t  x;
    uint8_t  y;
} SaveData;

static void save_enable(void) {
    ENABLE_RAM;
    SWITCH_RAM(0);
}

static void save_disable(void) {
    DISABLE_RAM;
}

uint8_t save_exists(void) {
    uint8_t ok;

    save_enable();
    ok = (uint8_t)(SAVE_ADDR->magic0 == SAVE_MAGIC0 &&
                   SAVE_ADDR->magic1 == SAVE_MAGIC1 &&
                   SAVE_ADDR->version == SAVE_VERSION &&
                   SAVE_ADDR->area < AREA_COUNT &&
                   SAVE_ADDR->level > 0 &&
                   SAVE_ADDR->max_hp > 0);
    save_disable();
    return ok;
}

void save_game(void) {
    SaveData *save = SAVE_ADDR;

    save_enable();
    save->magic0 = SAVE_MAGIC0;
    save->magic1 = SAVE_MAGIC1;
    save->version = SAVE_VERSION;

    save->level = player.level;
    save->hp = player.hp;
    save->max_hp = player.max_hp;
    save->exp = player.exp;
    save->next_level_exp = player.next_level_exp;
    save->attack = player.attack;
    save->defense = player.defense;
    save->mp = player.mp;
    save->max_mp = player.max_mp;
    save->potions = player.potions;
    save->wins = player.wins;

    save->area = current_area;
    save->x = px;
    save->y = py;
    save_disable();
}

uint8_t load_game(void) {
    const SaveData *save = SAVE_ADDR;

    if (!save_exists()) {
        return 0;
    }
    save_enable();

    player.level = save->level;
    player.hp = save->hp;
    player.max_hp = save->max_hp;
    player.exp = save->exp;
    player.next_level_exp = save->next_level_exp;
    player.attack = save->attack;
    player.defense = save->defense;
    player.mp = save->mp;
    player.max_mp = save->max_mp;
    player.potions = save->potions;
    player.wins = save->wins;

    current_area = save->area;
    px = save->x;
    py = save->y;
    save_disable();

    if (player.hp > player.max_hp) {
        player.hp = player.max_hp;
    }
    if (player.mp > player.max_mp) {
        player.mp = player.max_mp;
    }
    return 1;
}

void save_erase(void) {
    save_enable();
    SAVE_ADDR->magic0 = 0;
    SAVE_ADDR->magic1 = 0;
    SAVE_ADDR->version = 0;
    save_disable();
}
