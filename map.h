#ifndef MAP_H
#define MAP_H

#include <stdint.h>

extern uint8_t enemy_px, enemy_py;
extern uint8_t enemy_kind;
extern uint8_t enemy_sprite_tile;

extern uint8_t npc_px, npc_py;
extern uint8_t npc_active;

/* โหลดแผนที่ของ Act นั้น ๆ ขึ้นจอ */
void map_load(uint8_t area);

/* วาดแถวของแผนที่ใหม่ (ใช้ล้างข้อความที่ทับอยู่) */
void map_draw_row(uint8_t row);

/* วางศัตรูของพื้นที่ (forest สุ่มโครงกระดูก/ค้างคาว) */
void map_spawn_enemy(uint8_t area);

/* วาง NPC เฒ่าผู้แก่ (เฉพาะ Green Plains) */
void map_setup_npc(uint8_t area);

/* ตรวจสิ่งกีดขวาง */
uint8_t map_is_solid_tile(uint8_t tx, uint8_t ty);
uint8_t map_is_solid_px(uint8_t x, uint8_t y);

const char *map_area_name(uint8_t area);
const char *map_area_act(uint8_t area);

#endif /* MAP_H */
