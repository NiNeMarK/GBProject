#include <gb/gb.h>
#include <stdint.h>
#include <rand.h>

#include "game.h"
#include "assets.h"
#include "map.h"

uint16_t enemy_px, enemy_py;
uint8_t enemy_kind;
uint8_t enemy_sprite_tile;

uint16_t npc_px, npc_py;
uint8_t npc_active;

uint8_t map_cam_x;

/* แผนที่ทั้ง Act ถูกเก็บใน WRAM เพื่อให้เลื่อนกล้องได้ */
static uint8_t map_buf[MAP_W * SCREEN_H];

/* ตำแหน่งเกิดศัตรูเริ่มต้น และตำแหน่งสำรองเมื่อผู้เล่นยืนอยู่ตรงจุดเกิด */
static const uint16_t spawn_x[AREA_COUNT]     = {120, 120, 272};
static const uint16_t spawn_y[AREA_COUNT]     = { 32, 104, 104};
static const uint16_t alt_spawn_x[AREA_COUNT] = { 24,  16, 144};
static const uint16_t alt_spawn_y[AREA_COUNT] = {120,  88, 104};

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

/* กล้องตามผู้เล่น แต่ไม่เลยขอบแผนที่ */
static uint8_t camera_for_px(uint16_t x) {
    uint8_t cam;

    if (x <= 72) {
        return 0;
    }
    cam = (uint8_t)(x - 72);
    if (cam > (uint8_t)((MAP_W - SCREEN_W) * 8)) {
        cam = (uint8_t)((MAP_W - SCREEN_W) * 8);
    }
    return cam;
}

/*
 * แผนที่พื้นหลังกว้าง 32 ไทล์แบบวนรอบ
 * - กล้องช่วงซ้าย (0-95 px) ใช้คอลัมน์ 0-31 = โลกคอลัมน์ 0-31
 * - กล้องช่วงขวา (96-160 px) ใช้คอลัมน์ 0-7 = โลกคอลัมน์ 32-39
 * จึงเขียนใหม่เฉพาะ 8 คอลัมน์เมื่อกล้องข้ามเส้นแบ่ง
 */
static void write_world_column(uint8_t col) {
    uint8_t buf[SCREEN_H];
    uint8_t y;

    for (y = 0; y < SCREEN_H; y++) {
        buf[y] = map_buf[(uint16_t)y * MAP_W + col];
    }
    set_bkg_tiles((uint8_t)(col & 31), 0, 1, SCREEN_H, buf);
}

static void write_ring(uint8_t high) {
    uint8_t i;
    uint8_t start = high ? 32 : 0;

    for (i = 0; i < 8; i++) {
        write_world_column((uint8_t)(start + i));
    }
}

void map_load(uint8_t area) {
    uint8_t x, y;
    uint8_t target;

    if (area >= AREA_COUNT) {
        area = AREA_PLAINS;
    }
    for (y = 0; y < SCREEN_H; y++) {
        for (x = 0; x < MAP_W; x++) {
            map_buf[(uint16_t)y * MAP_W + x] = char_to_tile(area_maps[area][y][x]);
        }
    }

    target = camera_for_px(px);

    /* เขียนแผนที่ทั้งหน้าจอครั้งเดียวตอนเปลี่ยนพื้นที่ */
    DISPLAY_OFF;
    for (x = 0; x < 32; x++) {
        write_world_column(x);
    }
    if (target >= 96) {
        for (x = 32; x < MAP_W; x++) {
            write_world_column(x);
        }
    }
    map_cam_x = target;
    SCX_REG = target;
    SCY_REG = 0;
    DISPLAY_ON;
}

void map_scroll_sync(void) {
    uint8_t target = camera_for_px(px);
    uint8_t was_high;

    if (target == map_cam_x) {
        return;
    }
    was_high = (uint8_t)(map_cam_x >= 96);
    map_cam_x = target;

    if ((uint8_t)(target >= 96) != was_high) {
        write_ring((uint8_t)(target >= 96));
    }
    SCX_REG = target;
}

uint8_t map_is_solid_tile(uint8_t tx, uint8_t ty) {
    uint8_t tile;

    if (tx >= MAP_W || ty >= SCREEN_H) {
        return 1;
    }
    tile = map_buf[(uint16_t)ty * MAP_W + tx];

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

uint8_t map_is_solid_px(uint16_t x, uint16_t y) {
    /* ตรวจทั้งสี่มุมของสไปรท์ 8x8 */
    return (uint8_t)(map_is_solid_tile((uint8_t)(x >> 3), (uint8_t)(y >> 3)) ||
                     map_is_solid_tile((uint8_t)((x + 7) >> 3), (uint8_t)(y >> 3)) ||
                     map_is_solid_tile((uint8_t)(x >> 3), (uint8_t)((y + 7) >> 3)) ||
                     map_is_solid_tile((uint8_t)((x + 7) >> 3), (uint8_t)((y + 7) >> 3)));
}

void map_spawn_enemy(uint8_t area) {
    uint16_t sx, sy;

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
