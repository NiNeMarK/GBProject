#ifndef GAME_H
#define GAME_H

#include <gb/gb.h>
#include <stdint.h>

/* ---------- หน้าจอ/ไทล์ ---------- */
#define SCREEN_W 20
#define SCREEN_H 18
#define MAP_W    40              /* ความกว้างแผนที่จริง (ไทล์) */
#define MAP_MAX_X ((MAP_W * 8) - 8)
#define MAP_MAX_Y ((SCREEN_H * 8) - 8)

/* ลำดับไทล์พื้นหลัง (ต้องตรงกับ map_tiles ใน assets.c) */
#define TILE_GRASS     0
#define TILE_TREE      1
#define TILE_WATER     2
#define TILE_PATH      3
#define TILE_FLOWER    4
#define TILE_WALL      5
#define TILE_FLOOR     6
#define TILE_PILLAR    7
#define TILE_SIGN      8
#define TILE_DARKGRASS 9
#define TILE_BONES     10
#define TILE_CRYSTAL   11
#define TILE_BLACK     12
#define TILE_COUNT     13

/* ฟอนต์ 8x8 เริ่มที่ไทล์นี้ */
#define FONT_TILE_BASE 32

/* สไปรท์ (ต้องตรงกับ sprite_tiles ใน assets.c) */
#define SPR_HERO     0
#define SPR_SLIME    1
#define SPR_SKELETON 2
#define SPR_BAT      3
#define SPR_BOSS     4
#define SPR_ELDER    5
#define SPR_TILE_COUNT 6

/* ---------- สถานะเกม ---------- */
typedef enum {
    STATE_TITLE,
    STATE_OVERWORLD,
    STATE_BATTLE,
    STATE_MENU,
    STATE_ENDING
} GameState;

/* ---------- พื้นที่/บทของเกม ---------- */
typedef enum {
    AREA_PLAINS = 0,
    AREA_FOREST,
    AREA_CASTLE,
    AREA_COUNT
} AreaId;

/* ---------- ตัวละคร ---------- */
typedef struct {
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
} Player;

typedef struct {
    const char *name;
    uint8_t  level;
    uint16_t hp;
    uint16_t max_hp;
    uint8_t  attack;
    uint8_t  defense;
    uint8_t  exp_reward;
    uint8_t  sprite_tile;
    uint8_t  is_boss;
} Monster;

/* ---------- ตัวแปรกลาง ---------- */
extern Player player;
extern Monster monster;
extern GameState game_state;
extern uint8_t current_area;
extern uint16_t px, py;             /* ตำแหน่งผู้เล่น (พิกเซล) */
extern uint16_t enemy_px, enemy_py; /* ตำแหน่งศัตรูบนแผนที่ */
extern uint8_t enemy_kind;         /* 0=slime 1=skeleton 2=bat 3=boss */
extern uint8_t enemy_sprite_tile;
extern uint8_t map_cam_x;          /* กล้องแนวนอน (พิกเซล) */

/* ---------- ฟังก์ชันเปลี่ยนสถานะ ---------- */
void enter_title(void);
void enter_overworld(void);
void enter_battle(void);
void enter_status_menu(void);
void enter_ending(void);

/* ---------- ลูปของแต่ละสถานะ ---------- */
void title_update(void);
void overworld_update(void);
void battle_update(void);
void menu_update(void);
void ending_update(void);

#define ABS_DIFF(a, b) ((a) > (b) ? (uint16_t)((a) - (b)) : (uint16_t)((b) - (a)))

#endif /* GAME_H */
