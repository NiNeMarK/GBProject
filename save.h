#ifndef SAVE_H
#define SAVE_H

#include <stdint.h>

/* ตรวจว่ามีเซฟที่ถูกต้องในตลับหรือไม่ */
uint8_t save_exists(void);

/* บันทึกสถานะปัจจุบันลง SRAM */
void save_game(void);

/* โหลดเซฟ คืน 1 ถ้าสำเร็จ */
uint8_t load_game(void);

/* ลบเซฟ (หลังจบเกม) */
void save_erase(void);

#endif /* SAVE_H */
