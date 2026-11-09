#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>

// กำหนดสถานะของเกม (Game States)
typedef enum {
    STATE_TITLE,
    STATE_OVERWORLD,
    STATE_BATTLE,
    STATE_MENU
} GameState;

GameState current_state = STATE_TITLE;

// โครงสร้างข้อมูลตัวละคร (Player Stats)
typedef struct {
    UINT8 level;
    UINT16 hp;
    UINT16 max_hp;
    UINT16 exp;
    UINT16 next_level_exp;
    UINT8 attack;
    UINT8 defense;
} Player;

Player player = {1, 20, 20, 0, 10, 5, 2};

// โครงสร้างข้อมูลมอนสเตอร์ (เพิ่ม const ป้องกัน warning)
typedef struct {
    const char* name;
    UINT8 level;
    UINT16 hp;
    UINT16 max_hp;
    UINT8 attack;
    UINT8 exp_reward;
} Monster;

Monster current_monster;

// กราฟิกตัวละครและมอนสเตอร์แบบย่อ
const unsigned char player_tile[] = {0x3C, 0x42, 0x99, 0xA5, 0xA5, 0x99, 0x42, 0x3C};
const unsigned char monster_tile[] = {0x3C, 0x7E, 0xFF, 0xDB, 0xFF, 0x7E, 0x3C, 0x18};

UINT8 px = 80, py = 72;
UINT8 mx = 40, my = 40;
UINT8 monster_move_delay = 0;
UINT8 battle_initialized = 0;

void init_game(void) {
    set_sprite_data(0, 1, player_tile);
    set_sprite_tile(0, 0);
    set_sprite_data(1, 1, monster_tile);
    set_sprite_tile(1, 1);
    SHOW_SPRITES;
    DISPLAY_ON;
}

void update_title_screen(void) {
    gotoxy(3, 5);
    printf("== GB FANTASY RPG ==");
    gotoxy(4, 9);
    printf("PRESS START (A) ");
    
    if (joypad() & J_A) {
        for(UINT8 i=0; i<18; i++) {
            gotoxy(0, i);
            printf("                    ");
        }
        current_state = STATE_OVERWORLD;
        waitpadup();
    }
}

void update_battle(void) {
    if (!battle_initialized) {
        for(UINT8 i=0; i<18; i++) { gotoxy(0, i); printf("                    "); }
        gotoxy(2, 2);
        printf("WILD %s LV.%u", current_monster.name, current_monster.level);
        gotoxy(2, 4);
        printf("MON HP: %u/%u    ", current_monster.hp, current_monster.max_hp);
        gotoxy(2, 9);
        printf("PLAYER HP: %u/%u ", player.hp, player.max_hp);
        gotoxy(2, 13);
        printf("A:ATTACK   B:RUN ");
        battle_initialized = 1;
    }

    UINT8 keys = joypad();
    
    if (keys & J_A) {
        if (current_monster.hp > player.attack) {
            current_monster.hp -= player.attack;
        } else {
            current_monster.hp = 0;
        }

        gotoxy(2, 4);
        printf("MON HP: %u/%u    ", current_monster.hp, current_monster.max_hp);

        delay(300); 
        waitpadup();

        if (current_monster.hp == 0) {
            player.exp += current_monster.exp_reward;
            if (player.exp >= player.next_level_exp) {
                player.exp -= player.next_level_exp;
                player.level++;
                player.max_hp += 5;
                player.hp = player.max_hp;
                player.attack += 2;
                player.next_level_exp += 10;
            }
            
            for(UINT8 i=0; i<18; i++) { gotoxy(0, i); printf("                    "); }
            gotoxy(2, 5);
            printf("VICTORY! EXP +%u", current_monster.exp_reward);
            gotoxy(2, 7);
            printf("PRESS A TO CONTINUE");
            
            waitpadup();
            while(!(joypad() & J_A)) { wait_vbl_done(); }
            waitpadup();
            
            // เคลียร์หน้าจอให้เกลี้ยงก่อนกลับแมพ
            for(UINT8 i=0; i<18; i++) { gotoxy(0, i); printf("                    "); }

            battle_initialized = 0;
            mx = (px > 80) ? 20 : 140;
            my = (py > 64) ? 20 : 100;
            current_state = STATE_OVERWORLD;
            return;
        } else {
            if (player.hp > current_monster.attack) {
                player.hp -= current_monster.attack;
            } else {
                player.hp = player.max_hp; 
                for(UINT8 i=0; i<18; i++) { gotoxy(0, i); printf("                    "); }
                gotoxy(2, 5);
                printf("YOU DIED... RESPAWN");
                
                waitpadup();
                while(!(joypad() & J_A)) { wait_vbl_done(); }
                waitpadup();
                
                // เคลียร์หน้าจอให้เกลี้ยงก่อนกลับแมพ
                for(UINT8 i=0; i<18; i++) { gotoxy(0, i); printf("                    "); }

                battle_initialized = 0;
                current_state = STATE_OVERWORLD;
                return;
            }
            
            gotoxy(2, 9);
            printf("PLAYER HP: %u/%u ", player.hp, player.max_hp);
        }
    }
    
    if (keys & J_B) {
        for(UINT8 i=0; i<18; i++) { gotoxy(0, i); printf("                    "); }
        battle_initialized = 0;
        current_state = STATE_OVERWORLD;
        waitpadup();
    }
}

void update_overworld(void) {
    UINT8 keys = joypad();

    if (keys & J_UP)    { if (py > 16) py--; }
    if (keys & J_DOWN)  { if (py < 112) py++; }
    if (keys & J_LEFT)  { if (px > 8) px--; }
    if (keys & J_RIGHT) { if (px < 160) px++; }

    monster_move_delay++;
    if (monster_move_delay >= 15) {
        monster_move_delay = 0;
        
        if (mx < px) mx++;
        else if (mx > px) mx--;

        if (my < py) my++;
        else if (my > py) my--;
    }

    move_sprite(0, px, py);
    move_sprite(1, mx, my);

    if (keys & J_SELECT) {
        current_state = STATE_MENU;
        move_sprite(0, 0, 0);
        move_sprite(1, 0, 0);
        waitpadup();
        return;
    }

    INT8 dx = (INT8)px - (INT8)mx;
    INT8 dy = (INT8)py - (INT8)my;
    if (dx > -8 && dx < 8 && dy > -8 && dy < 8) {
        current_monster.name = "SLIME";
        current_monster.level = 1;
        current_monster.hp = 12;
        current_monster.max_hp = 12;
        current_monster.attack = 4;
        current_monster.exp_reward = 6;

        current_state = STATE_BATTLE;
        move_sprite(1, 0, 0); 
        
        for(UINT8 i=0; i<18; i++) { gotoxy(0, i); printf("                    "); }
        waitpadup();
    }
}

void update_menu(void) {
    gotoxy(3, 2);
    printf("=== STATUS ===");
    gotoxy(3, 4);
    printf("LEVEL : %u", player.level);
    gotoxy(3, 6);
    printf("HP    : %u/%u", player.hp, player.max_hp);
    gotoxy(3, 8);
    printf("ATTACK: %u", player.attack);
    gotoxy(3, 10);
    printf("EXP   : %u/%u", player.exp, player.next_level_exp);
    
    gotoxy(3, 14);
    printf("PRESS SELECT TO BACK");

    if (joypad() & J_SELECT) {
        for(UINT8 i=0; i<18; i++) { gotoxy(0, i); printf("                    "); }
        current_state = STATE_OVERWORLD;
        waitpadup();
    }
}

void main(void) {
    init_game();

    while (1) {
        switch (current_state) {
            case STATE_TITLE:
                update_title_screen();
                break;
            case STATE_OVERWORLD:
                update_overworld();
                break;
            case STATE_BATTLE:
                update_battle();
                break;
            case STATE_MENU:
                update_menu();
                break;
        }
        wait_vbl_done();
    }
}