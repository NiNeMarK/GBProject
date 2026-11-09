#include <stdint.h>

#include "game.h"
#include "player.h"

Player player;
uint16_t px = 32;
uint16_t py = 80;

void player_init(void) {
    player.level = 1;
    player.max_hp = 24;
    player.hp = 24;
    player.exp = 0;
    player.next_level_exp = 10;
    player.attack = 6;
    player.defense = 2;
    player.max_mp = 5;
    player.mp = 5;
    player.potions = 3;
    player.wins = 0;

    px = 32; /* เริ่มที่ Green Plains */
    py = 80;
}

uint8_t player_gain_exp(uint16_t amount) {
    uint8_t levels = 0;

    player.exp += amount;

    while (player.exp >= player.next_level_exp) {
        player.exp -= player.next_level_exp;
        player.level++;
        player.max_hp += 5;
        player.hp = player.max_hp;
        player.attack += 2;
        player.defense += 1;
        player.max_mp += 2;
        player.mp = player.max_mp;
        player.next_level_exp += 10;
        levels++;
    }

    return levels;
}

void player_heal(uint16_t amount) {
    if (amount >= player.max_hp - player.hp) {
        player.hp = player.max_hp;
    } else {
        player.hp += amount;
    }
}

void player_take_damage(uint16_t amount) {
    if (amount >= player.hp) {
        player.hp = 0;
    } else {
        player.hp -= amount;
    }
}
