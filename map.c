#include <gb/gb.h>
#include <stdint.h>
#include <rand.h>

#include "game.h"
#include "assets.h"
#include "map.h"

uint8_t enemy_px, enemy_py;
uint8_t enemy_kind;
uint8_t enemy_sprite_tile;

uint8_t npc_px, npc_py;
uint8_t npc_active;

static uint8_t map_buf[SCREEN_W * SCREEN_H];

/* ตำแหน่งเกิดศัตรูเริ่มต้น และตำแหน่งสำรองเมื่อผู้เล่นยืนอยู่ตรงจุดเกิด */
static const uint8_t spawn_x[AREA_COUNT]     = {120, 120, 128};
static const uint8_t spawn_y[AREA_COUNT]     = { 32, 104, 104};
static const uint8_t alt_spawn_x[AREA_COUNT] = { 24,  16, 128};
static const uint8_t alt_spawn_y[AREA_COUNT] = {120,  88, 104};

static uint8_t char_to_tile(char c) {
    switch (c) {
        case '#': return TILE_TREE;
        case '~': return TILE_WATER;
        case '=': return TILE_PATH;
        case '*': return TILE_FLOWER;
        case 'W': return TILE_WALL;
        case ':': return TILE_FLOOR;
        case 'O': return TILE_PILLAR;
        case 'S': return TILE_SIGN;
        case 'd': return TILE_DARKGRASS;
        case 'b': return TILE_BONES;
        case 'X': return TILE_CRYSTAL;
        case '.': return TILE_GRASS;
        default:  return TILE_GRASS;
    }
}

void map_load(uint8_t area) {
    uint8_t x, y;

    if (area >= AREA_COUNT) {
        area = AREA_PLAINS;
    }
    for (y = 0; y < SCREEN_H; y++) {
        for (x = 0; x < SCREEN_W; x++) {
            map_buf[(uint16_t)y * SCREEN_W + x] = char_to_tile(area_maps[area][y][x]);
        }
    }
    set_bkg_tiles(0, 0, SCREEN_W, SCREEN_H, map_buf);
}

void map_draw_row(uint8_t row) {
    if (row >= SCREEN_H) {
        return;
    }
    set_bkg_tiles(0, row, SCREEN_W, 1, &map_buf[(uint16_t)row * SCREEN_W]);
}

uint8_t map_is_solid_tile(uint8_t tx, uint8_t ty) {
    uint8_t tile;

    if (tx >= SCREEN_W || ty >= SCREEN_H) {
        return 1;
    }
    tile = map_buf[(uint16_t)ty * SCREEN_W + tx];

    switch (tile) {
        case TILE_TREE:
        case TILE_WATER:
        case TILE_WALL:
        case TILE_PILLAR:
        case TILE_SIGN:
        case TILE_CRYSTAL:
            return 1;
        default:
            return 0;
    }
}

uint8_t map_is_solid_px(uint8_t x, uint8_t y) {
    /* ตรวจทั้งสี่มุมของสไปรท์ 8x8 */
    return (uint8_t)(map_is_solid_tile((uint8_t)(x >> 3), (uint8_t)(y >> 3)) ||
                     map_is_solid_tile((uint8_t)((x + 7) >> 3), (uint8_t)(y >> 3)) ||
                     map_is_solid_tile((uint8_t)(x >> 3), (uint8_t)((y + 7) >> 3)) ||
                     map_is_solid_tile((uint8_t)((x + 7) >> 3), (uint8_t)((y + 7) >> 3)));
}

void map_spawn_enemy(uint8_t area) {
    uint8_t sx, sy;

    if (area >= AREA_COUNT) {
        area = AREA_PLAINS;
    }
    sx = spawn_x[area];
    sy = spawn_y[area];

    /* อย่าให้ศัตรูโผล่ทับผู้เล่นทันทีหลังจบการต่อสู้ */
    if (ABS_DIFF(px, sx) < 24 && ABS_DIFF(py, sy) < 24) {
        sx = alt_spawn_x[area];
        sy = alt_spawn_y[area];
    }
    enemy_px = sx;
    enemy_py = sy;

    if (area == AREA_PLAINS) {
        enemy_kind = 0;
        enemy_sprite_tile = SPR_SLIME;
    } else if (area == AREA_FOREST) {
        if (rand() & 1) {
            enemy_kind = 1;
            enemy_sprite_tile = SPR_SKELETON;
        } else {
            enemy_kind = 2;
            enemy_sprite_tile = SPR_BAT;
        }
    } else {
        enemy_kind = 3;
        enemy_sprite_tile = SPR_BOSS;
    }
}

void map_setup_npc(uint8_t area) {
    if (area == AREA_PLAINS) {
        npc_active = 1;
        npc_px = 48;
        npc_py = 88;
    } else {
        npc_active = 0;
    }
}

const char *map_area_name(uint8_t area) {
    switch (area) {
        case AREA_PLAINS: return "GREEN PLAINS";
        case AREA_FOREST: return "WHISPERING WOODS";
        case AREA_CASTLE: return "THE VOID CASTLE";
        default:          return "AETHELGARD";
    }
}

const char *map_area_act(uint8_t area) {
    switch (area) {
        case AREA_PLAINS: return "ACT 1";
        case AREA_FOREST: return "ACT 2";
        case AREA_CASTLE: return "ACT 3";
        default:          return "CHRONO CRYSTAL";
    }
}
