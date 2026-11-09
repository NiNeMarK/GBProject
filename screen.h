#ifndef SCREEN_H_INCLUDED
#define SCREEN_H_INCLUDED

#include <stdint.h>

/* ตั้งค่า palette, โหลดไทล์/ฟอนต์/สไปรท์ และเปิดจอ */
void screen_init(void);

/* ล้างจอเป็นสีดำ (สำหรับหน้า title/battle/menu) */
void text_clear(void);

/* เขียนข้อความ/ตัวเลข คืนค่าคอลัมน์ถัดไป */
uint8_t text_put(uint8_t x, uint8_t y, const char *s);
uint8_t text_put_u16(uint8_t x, uint8_t y, uint16_t value);

/* ล้างเฉพาะบรรทัด / วาดกล่องดำ */
void text_clear_line(uint8_t y);
void text_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

/* รอจนกว่าจะกด A (พร้อม debounce) */
void wait_a_press(void);

#endif /* SCREEN_H_INCLUDED */
