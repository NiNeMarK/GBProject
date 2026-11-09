#include <gb/gb.h>
#include <stdint.h>

#include "sfx.h"

/* ค่าความถี่ของช่องเสียง (NR13/NR14) */
#define NOTE_REST 0x0000
#define NOTE_A4   0x06D6
#define NOTE_C5   0x0706
#define NOTE_D5   0x0721
#define NOTE_E5   0x0739
#define NOTE_F5   0x0744
#define NOTE_G5   0x0759
#define NOTE_A5   0x076B
#define NOTE_B5   0x077B
#define NOTE_C6   0x0783

/* ทำนองหลักของเกม (16 โน้ต ลูปไปเรื่อย ๆ) */
static const uint16_t bgm_notes[] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_E5,
    NOTE_A4, NOTE_C5, NOTE_E5, NOTE_D5,
    NOTE_F5, NOTE_A5, NOTE_G5, NOTE_E5,
    NOTE_D5, NOTE_C5, NOTE_A4, NOTE_REST
};
#define BGM_NOTE_COUNT 16
#define BGM_NOTE_FRAMES 12

static uint16_t bgm_timer;
static uint8_t bgm_pos;

static void tone(uint16_t freq, uint8_t dur) {
    NR10_REG = 0x00;
    NR11_REG = 0x80;
    NR12_REG = 0xA4;
    NR13_REG = (uint8_t)(freq & 0xFF);
    NR14_REG = (uint8_t)(0x80 | (freq >> 8));
    delay(dur);
    NR12_REG = 0x00;
}

static void noise(uint8_t period, uint8_t dur) {
    NR41_REG = 0x00;
    NR42_REG = 0xF3;
    NR43_REG = period;
    NR44_REG = 0x80;
    delay(dur);
    NR42_REG = 0x00;
}

void sfx_init(void) {
    NR52_REG = 0x80; /* เปิดระบบเสียง */
    NR51_REG = 0xFF; /* ทุกช่องออกทั้งซ้าย/ขวา */
    NR50_REG = 0x55; /* ระดับเสียงกลาง ๆ */
    NR10_REG = 0x00;
    NR12_REG = 0x00;
    NR22_REG = 0x00;
    NR32_REG = 0x00;
    NR42_REG = 0x00;
    bgm_timer = 0;
    bgm_pos = 0;
}

void sfx_menu(void) {
    tone(NOTE_E5, 3);
}

void sfx_confirm(void) {
    tone(NOTE_C6, 6);
}

void sfx_attack(void) {
    noise(0x44, 12);
}

void sfx_hit(void) {
    noise(0x33, 10);
}

void sfx_hurt(void) {
    noise(0x66, 16);
}

void sfx_heal(void) {
    tone(NOTE_C5, 6);
    tone(NOTE_G5, 8);
}

void sfx_levelup(void) {
    tone(NOTE_C5, 6);
    tone(NOTE_E5, 6);
    tone(NOTE_G5, 10);
}

void sfx_victory(void) {
    tone(NOTE_C5, 6);
    tone(NOTE_E5, 6);
    tone(NOTE_G5, 6);
    tone(NOTE_C6, 14);
}

void bgm_update(void) {
    uint16_t note;

    if (bgm_timer) {
        bgm_timer--;
        return;
    }
    note = bgm_notes[bgm_pos];
    bgm_pos++;
    if (bgm_pos >= BGM_NOTE_COUNT) {
        bgm_pos = 0;
    }
    bgm_timer = BGM_NOTE_FRAMES;

    if (note == NOTE_REST) {
        NR22_REG = 0x00;
        return;
    }
    NR21_REG = 0x80;
    NR22_REG = 0xA2;
    NR23_REG = (uint8_t)(note & 0xFF);
    NR24_REG = (uint8_t)(0x80 | (note >> 8));
}

void delay_frames(uint8_t frames) {
    while (frames--) {
        bgm_update();
        wait_vbl_done();
    }
}
