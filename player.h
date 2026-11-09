#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

/* รีเซ็ตผู้เล่นเป็นค่าที่เริ่มเกม */
void player_init(void);

/* เพิ่ม EXP และเลเวลอัปอัตโนมัติ คืนจำนวนเลเวลที่เพิ่มขึ้น */
uint8_t player_gain_exp(uint16_t amount);

/* ฟื้นฟู HP (ไม่เกิน max) */
void player_heal(uint16_t amount);

/* หัก HP (ไม่ติดลบ) */
void player_take_damage(uint16_t amount);

#endif /* PLAYER_H */
