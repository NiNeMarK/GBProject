#ifndef SFX_H
#define SFX_H

#include <stdint.h>

void sfx_init(void);

/* เอฟเฟกต์เสียงสั้น ๆ */
void sfx_menu(void);
void sfx_confirm(void);
void sfx_attack(void);
void sfx_hit(void);
void sfx_hurt(void);
void sfx_heal(void);
void sfx_levelup(void);
void sfx_victory(void);

/* เพลงประกอบแบบลูป เรียกทุกเฟรม */
void bgm_update(void);

/* หน่วงเวลาหลายเฟรม พร้อมอัปเดตเพลง */
void delay_frames(uint8_t frames);

#endif /* SFX_H */
