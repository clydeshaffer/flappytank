#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "persist.h"
#include "banking.h"
#include "gen/assets/gfx.h"
#include "random.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/audio.h"

#define SUBFRAME_COUNT 2
#define FLAP_ANIMATION_START 0
#define FLAP_ANIMATION_END 8
#define EXPLODE_ANIMATION_START 9
#define EXPLODE_ANIMATION_END 15
#define GRAVITY 32
char anim_frame = 0;
char anim_subframe = 0;
char burd_alive;
char reset_timer;

int bird_y;
int bird_vy;

#define GAME_RESET_TIME 180
#define OBSTACLE_COUNT 2
#define OBSTACLE_WIDTH 32
#define BIRD_X 40
char opening_top[OBSTACLE_COUNT] = { 48, 48 };
char opening_bot[OBSTACLE_COUNT] = { 96, 96};
char opening_width = 48;
int obstacle_x[OBSTACLE_COUNT] = { (64 << 8) + (128 << 8), (128 << 8)  + (128 << 8)};

#define PHASE_TITLE 0
#define PHASE_MAIN 1
char game_phase = PHASE_TITLE;

void init_stuff() {
    static char i;
    for(i = 0; i < OBSTACLE_COUNT; ++i) {
        opening_top[i] = 48;
        opening_top[i] = 48;
        obstacle_x[i] = ((i+1) * 64) << 8;
    }
    burd_alive = 1;
    anim_frame = 0;
    anim_subframe = 0;
    bird_y = 64 << 8;
    bird_vy = 0;
    reset_timer = GAME_RESET_TIME;
    game_phase = PHASE_TITLE;
    play_song(&ASSET__audio__title_mid, REPEAT_NONE);
}

#define SPRITE_BANK_BURD 0
#define SPRITE_BANK_ENV 1
#define SPRITE_BANK_TITLE 2

int main () {
    static char i;
    static char tmp;

    init_graphics();
    init_dynawave();
    init_music();

    load_spritesheet(&ASSET__gfx__burd_bmp, SPRITE_BANK_BURD);
    load_spritesheet(&ASSET__gfx__environment_bmp, SPRITE_BANK_ENV);
    load_spritesheet(&ASSET__gfx__titlecard_bmp, SPRITE_BANK_TITLE);

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    while(1) {
        init_stuff();

        while (reset_timer) {                                     //  Run forever
            clear_screen(84);

            if(game_phase == PHASE_TITLE) {
                draw_sprite(0, 0, 127, 127, 0, 0, SPRITE_BANK_TITLE | bankflip);
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                    play_song(&ASSET__audio__gameloop_mid, REPEAT_LOOP);
                    game_phase = PHASE_MAIN;
                }
            }
            else if(game_phase == PHASE_MAIN) {
                if(burd_alive) {
                    draw_sprite_frame(&ASSET__gfx__burd_json, 40, bird_y >> 8, anim_frame, 0, SPRITE_BANK_BURD | BANK_CLIP_X | BANK_CLIP_Y);
                    if(anim_frame > FLAP_ANIMATION_START) {
                        if(++anim_subframe == SUBFRAME_COUNT) {
                            anim_subframe = 0;
                            if(++anim_frame == FLAP_ANIMATION_END+1) {
                                anim_frame = FLAP_ANIMATION_START;
                            } else if(anim_frame == EXPLODE_ANIMATION_END) {
                                burd_alive = 0;
                            }
                        }
                    }

                    
                    
                    if(anim_frame < EXPLODE_ANIMATION_START) {
                        bird_vy += GRAVITY;
                        bird_y += bird_vy;
                        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_A) {
                                bird_vy = -512;
                                anim_frame = 1;
                                play_sound_effect(&ASSET__audio__flap_bin, 0);
                        }
                    }
                } else {
                    --reset_timer;
                }

                for(i = 0; i < OBSTACLE_COUNT; ++i) {
                    tmp = obstacle_x[i] >> 8;
                    draw_sprite(tmp, 0, OBSTACLE_WIDTH, opening_top[i], 0, 127 - opening_top[i], SPRITE_BANK_ENV | bankflip | BANK_CLIP_X | BANK_CLIP_Y);
                    draw_sprite(tmp, opening_bot[i], OBSTACLE_WIDTH, 127 - opening_bot[i], 0, 0, SPRITE_BANK_ENV | bankflip | BANK_CLIP_X | BANK_CLIP_Y);

                    if(anim_frame < EXPLODE_ANIMATION_START) {
                        if((BIRD_X > tmp) && ((BIRD_X - OBSTACLE_WIDTH) < tmp)) {
                            tmp = bird_y >> 8;
                            if((tmp < opening_top[i]) || (tmp > opening_bot[i])) {
                                anim_frame = EXPLODE_ANIMATION_START;
                                stop_music();
                                
                                play_sound_effect(&ASSET__audio__hit_bin, 1);
                            }
                        }
                        obstacle_x[i] -= 256;
                        if(obstacle_x[i] < ((-OBSTACLE_WIDTH)<<8)) {
                            opening_top[i] = rnd_range(24, 72);
                            opening_bot[i] = opening_top[i] + opening_width;
                            obstacle_x[i] += (128+OBSTACLE_WIDTH) << 8;
                        }
                    }
                }
            }  

            clear_border(0);
            await_draw_queue();
            sleep(1);
            flip_pages();
            update_inputs();
            tick_music();
        }
    }

  return (0);                                     //  We should never get here!
}