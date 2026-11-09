#include <gb/gb.h>
#include <stdint.h>

#include "game.h"
#include "assets.h"
#include "screen.h"
#include "sfx.h"

#define FONT_COUNT 54

/* แปลงตัวอักษรเป็น index ในตารางฟอนต์ (ตัวเล็กถือเป็นตัวใหญ่) */
static uint8_t font_index(char c) {
    uint8_t i = 0;

    if (c >= 'a' && c <= 'z') {
        c = (char)(c - 32);
    }
    while (font_chars[i]) {
        if (font_chars[i] == c) {
            return i;
        }
        i++;
    }
    return 0; /* ไม่รู้จัก -> ช่องว่าง */
}

static uint8_t font_tile(char c) {
    return (uint8_t)(FONT_TILE_BASE + font_index(c));
}

/* สร้างไทล์ฟอนต์ 8x8 จากบิตแมป 5x7 แล้วโหลดลง VRAM */
static void load_font(void) {
    uint8_t tile[16];
    uint8_t i, row;

    for (i = 0; i < FONT_COUNT; i++) {
        for (row = 0; row < 8; row++) {
            uint8_t bits = (row < 7) ? font_glyphs[(uint16_t)i * 7 + row] : 0;
            uint8_t mask = (uint8_t)(bits << 1); /* จัดกลาง 5 พิกเซล */
            tile[row * 2] = mask;
            tile[row * 2 + 1] = mask;
        }
        set_bkg_data(FONT_TILE_BASE + i, 1, tile);
    }
}

void screen_init(void) {
    BGP_REG = 0xE4;
    OBP0_REG = 0xE4;
    OBP1_REG = 0xE4;

    set_bkg_data(0, TILE_COUNT, map_tiles);
    load_font();
    set_sprite_data(0, SPR_TILE_COUNT, sprite_tiles);

    SPRITES_8x8;
    SCX_REG = 0;
    SCY_REG = 0;
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    text_clear();
}

void text_clear(void) {
    HIDE_WIN;
    SCX_REG = 0;
    SCY_REG = 0;
    text_box(0, 0, SCREEN_W, SCREEN_H);
}

uint8_t text_put(uint8_t x, uint8_t y, const char *s) {
    uint8_t buf[SCREEN_W];
    uint8_t n = 0;

    while (*s && (uint8_t)(x + n) < SCREEN_W) {
        buf[n] = font_tile(*s);
        n++;
        s++;
    }
    if (n) {
        set_bkg_tiles(x, y, n, 1, buf);
    }
    return (uint8_t)(x + n);
}

uint8_t text_put_u16(uint8_t x, uint8_t y, uint16_t value) {
    uint8_t digits[5];
    uint8_t buf[5];
    uint8_t n = 0;
    uint8_t i = 0;

    if (value == 0) {
        digits[n++] = 0;
    } else {
        while (value && n < 5) {
            digits[n++] = (uint8_t)(value % 10);
            value /= 10;
        }
    }
    while (n) {
        buf[i++] = (uint8_t)(FONT_TILE_BASE + 27 + digits[--n]);
    }
    if (i && (uint8_t)(x + i) <= SCREEN_W) {
        set_bkg_tiles(x, y, i, 1, buf);
    }
    return (uint8_t)(x + i);
}

void text_clear_line(uint8_t y) {
    uint8_t buf[SCREEN_W];
    uint8_t i;

    for (i = 0; i < SCREEN_W; i++) {
        buf[i] = TILE_BLACK;
    }
    set_bkg_tiles(0, y, SCREEN_W, 1, buf);
}

void text_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    uint8_t buf[SCREEN_W];
    uint8_t i;

    if (x >= SCREEN_W || y >= SCREEN_H) {
        return;
    }
    if ((uint8_t)(x + w) > SCREEN_W) {
        w = (uint8_t)(SCREEN_W - x);
    }
    if ((uint8_t)(y + h) > SCREEN_H) {
        h = (uint8_t)(SCREEN_H - y);
    }
    for (i = 0; i < w; i++) {
        buf[i] = TILE_BLACK;
    }
    while (h--) {
        set_bkg_tiles(x, y, w, 1, buf);
        y++;
    }
}

#define WIN_W SCREEN_W
#define WIN_H 5
#define WIN_X 7
#define WIN_Y 104

/* กล่องบทสนทนาล่างจอ (window layer) จะไม่เลื่อนตามแผนที่ */
void win_dialog_begin(void) {
    uint8_t buf[WIN_W];
    uint8_t i, y;

    for (i = 0; i < WIN_W; i++) {
        buf[i] = TILE_BLACK;
    }
    for (y = 0; y < WIN_H; y++) {
        set_win_tiles(0, y, WIN_W, 1, buf);
    }
    WX_REG = WIN_X;
    WY_REG = WIN_Y;
    SHOW_WIN;
}

uint8_t win_text_put(uint8_t x, uint8_t y, const char *s) {
    uint8_t buf[WIN_W];
    uint8_t n = 0;

    while (*s && (uint8_t)(x + n) < WIN_W) {
        buf[n] = font_tile(*s);
        n++;
        s++;
    }
    if (n) {
        set_win_tiles(x, y, n, 1, buf);
    }
    return (uint8_t)(x + n);
}

void win_dialog_end(void) {
    HIDE_WIN;
}

void wait_a_press(void) {
    delay_frames(8);
    while (!(joypad() & J_A)) {
        bgm_update();
        wait_vbl_done();
    }
    delay_frames(8);
}
