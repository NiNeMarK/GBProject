#ifndef ASSETS_H
#define ASSETS_H

#include <stdint.h>
#include "game.h"

/* ไทล์พื้นหลัง 2bpp (ไทล์ละ 16 ไบต์) */
extern const uint8_t map_tiles[];

/* ไทล์สไปรท์ 8x8 (ไทล์ละ 16 ไบต์) */
extern const uint8_t sprite_tiles[];

/* ฟอนต์ 5x7 ขยายเป็น 8x8 ตอน init (ตัวอักษรละ 7 แถว) */
extern const uint8_t font_glyphs[];
extern const char font_chars[];

/* แผนที่แต่ละ Act: 18 แถว x 20 คอลัมน์ (+ null) */
extern const char area_maps[AREA_COUNT][SCREEN_H][SCREEN_W + 1];

#endif /* ASSETS_H */
