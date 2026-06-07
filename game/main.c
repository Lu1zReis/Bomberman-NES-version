#include <allegro5/allegro.h>
#include <allegro5/keyboard.h>
#include <allegro5/display.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define WEIGHT 210
#define HEIGHT 210
#define COL (WEIGHT / 16)
#define ROW (HEIGHT / 16)

#define FPS 30.f
#define SPEED 2

enum blocks {
    GROUND          = 0,
    HARD_BLOCK      = 1,
    BROKEN_BLOCK    = 2,
    FIRED_BLOCK     = 3,
    DESTROYED_BLOCK = 4,
    OUT             = 5
};

typedef struct _SPRITE{
    int x, y, w, h, frame_y, amount_x, amount_y;
    float frame;
    ALLEGRO_BITMAP *skin;
} SPRITE;

typedef struct _BOMB{
    int x, y, w, h, frame_y, amount_x, amount_y, timer, tile_x, tile_y;
    float frame;
    bool explosion;
    ALLEGRO_BITMAP *bomb;
    struct _BOMB *next;
    int cordenadas_bloco[4];
    bool solid;
} BOMB;

typedef struct _ENEMY {
    int x, y, w, h, frame_y, amount_x, amount_y, tile_x, tile_y, die, direction;
    float frame;
    ALLEGRO_BITMAP *enemy;
    struct _ENEMY *next;
} ENEMY;

ALLEGRO_BITMAP *bomba = NULL;
ALLEGRO_BITMAP *explosao = NULL;
ALLEGRO_BITMAP *inimigo = NULL;

// metodos gerais
void renderMap(int map[][COL], SPRITE obj, SPRITE saida);
SPRITE makeSprite(char *path_sprite, int x, int y, int w, int h, int frame_y, float frame, int amount_x, int amount_y);
void createMapping(int map[][COL], ENEMY **enemys, SPRITE *saida);
bool canMove(int map[][COL], int next_x, int next_y, int w, int h, BOMB *bomb);

// lista encadeada de bomba
void addBomb(BOMB **bombs, int x, int y, int timer);
void listBomb(BOMB **bombs, ENEMY **enemys, int map[][COL]); // aqui vamos exibir e se a bomba passar do tempo eh apagada
void removeAllBombs(BOMB **bombs);
int killForBomb(int x, int y, ENEMY **enemy);

// lista encadeada de inimigos
void addEnemy(ENEMY **enemy, int x, int y);
void listEnemy(ENEMY **enemy, BOMB *bombs, int map[][COL]);
void removeEnemy(ENEMY **enemys, int map[][COL]);
void removeAllEnemys(ENEMY **enemys);
int killForEnemy(ENEMY **enemy);
bool canMoveEnemy(int map[][COL], int next_x, int next_y, int w, int h, BOMB *bomb);


int player_x = 0;
int player_y = 0;
int end_game = 0;
int gameOver = 0;
int win      = 0;

int main () {

    al_init();
    al_init_image_addon();
    al_install_keyboard();
    al_init_font_addon();

    // variaveis primarias/auxiliares
    bool keys[ALLEGRO_KEY_MAX] = {false};
    int map[ROW][COL] = {0};

    ALLEGRO_FONT *font = al_create_builtin_font();

    ALLEGRO_DISPLAY *display = al_create_display(WEIGHT, HEIGHT);
    al_set_window_position(display, 700, 300);
    
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1 / FPS);
    al_start_timer(timer);
    
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    
    
    SPRITE block     = makeSprite("img/./sprite_blocks.png", 0, 0, 16, 16, 0, 0, 8, 1);
    SPRITE bomberman = makeSprite("img/./sprite_bomberman.png", 20, 15, 15, 16, 0, 0, 7, 3);
    SPRITE saida     = makeSprite("img/./saida.png", -1, -1, 15, 14, 0, 0, 1, 1);

    ENEMY *enemy     = NULL;
    BOMB *bomb       = NULL;
    
    
    // usando dessa forma para nao ficar sempre criando novo sprite sendo que podemos reutilizar
    bomba = al_load_bitmap("img/./bomba.png");
    explosao = al_load_bitmap("img/./explosao.png");
    inimigo = al_load_bitmap("img/./enemy.png");
    

    createMapping(map, &enemy, &saida); 

    // pelo sprite de andar para esq e para baixo estar na mesma linha, usar essa variavel para multiplicar quando vai ser um ou outro
    // sprite de andar para esq vem 3 sprites primeiro, depois 3 para baixo (mesma logica para andar direita e cima)
    int offset_x = 0;
    char texto[50];
    int tempo = FPS * 202;

    while (true) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);
        
        // caso o usuario clique para fechar ou tenha dado game over esta liberado para somente clicar enter
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || 
            (
                gameOver &&
                event.type == ALLEGRO_EVENT_KEY_DOWN &&
                event.keyboard.keycode == ALLEGRO_KEY_ENTER
            ))
        {
            break;
        } else if (!gameOver) {
            
            if (event.type == ALLEGRO_EVENT_KEY_UP) keys[event.keyboard.keycode] = false;
            else if (event.type == ALLEGRO_EVENT_KEY_DOWN && !end_game) {
                keys[event.keyboard.keycode] = true;

                if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    addBomb(&bomb, bomberman.x, bomberman.y, FPS * 2);
                }
            }
            
            if (event.type == ALLEGRO_EVENT_TIMER) {
                
                bool is_moving = false;

                if (keys[ALLEGRO_KEY_UP]) {
                    if (canMove(map, bomberman.x, bomberman.y - SPEED, bomberman.w, bomberman.h, bomb)) {
                        bomberman.y -= SPEED;
                        bomberman.frame_y = 1;
                        offset_x = 3;
                        is_moving = true;
                    }                    

                }
                else if (keys[ALLEGRO_KEY_DOWN]) {
                    if (canMove(map, bomberman.x, bomberman.y + SPEED, bomberman.w, bomberman.h, bomb)) {
                        bomberman.y += SPEED;
                        bomberman.frame_y = 0;
                        offset_x = 3;
                        is_moving = true;
                    }
                }
                else if (keys[ALLEGRO_KEY_RIGHT]) {
                    if (canMove(map, bomberman.x + SPEED, bomberman.y, bomberman.w, bomberman.h, bomb)) {
                        bomberman.x += SPEED;
                        bomberman.frame_y = 1;
                        offset_x = 0;
                        is_moving = true;
                    }
                }
                else if (keys[ALLEGRO_KEY_LEFT]) {
                    if (canMove(map, bomberman.x - SPEED, bomberman.y, bomberman.w, bomberman.h, bomb)) {
                        bomberman.x -= SPEED;
                        bomberman.frame_y = 0;
                        offset_x = 0;
                        is_moving = true;
                    }
                }
                
                if (end_game) {
                    bomberman.frame_y = 2;
                    bomberman.frame += 0.1f;
                    if (bomberman.frame >= 6) {
                        gameOver = 1;
                    }
                } 
                else if (tempo <= 0) {
                    end_game = 1;
                }
                
                else if (is_moving) {
                    bomberman.frame += 0.2f;
                    
                    if (bomberman.frame >= 3)
                    bomberman.frame = 0;
                    
                } 
                else {
                    bomberman.frame = 1;
                }
                
                al_clear_to_color(al_map_rgb(0, 100, 0));
                renderMap(map, block, saida);

                al_draw_bitmap_region(
                    bomberman.skin,
                    ((int)bomberman.frame + offset_x) * bomberman.w,
                    bomberman.frame_y * bomberman.h,
                    bomberman.w,
                    bomberman.h,
                    bomberman.x,
                    bomberman.y,
                    0
                );

                sprintf(texto, "Tempo: %d", (tempo--)/60);

                al_draw_text(
                    font,
                    al_map_rgb(0, 0, 0),
                    1,
                    2,
                    0,
                    texto
                );

                // if (keys[ALLEGRO_KEY_SPACE]) {
                //     addBomb(&bomb, bomberman.x, bomberman.y, FPS*3);
                // }
                
                player_x = bomberman.x;
                player_y = bomberman.y;

                listBomb(&bomb, &enemy, map);
                listEnemy(&enemy, bomb, map);
                killForEnemy(&enemy);
                al_flip_display();

            }

        } else if (gameOver) {

            // tela preta de game over
            al_clear_to_color(al_map_rgb(0,0,0));
            if (win)
                sprintf(texto,  "  WIN! ");
            else sprintf(texto, "GAME OVER");
            al_draw_text(
                font,
                al_map_rgb(255, 255, 255),
                (WEIGHT/2) - 35,
                (HEIGHT/2) - 20,
                0,
                texto
            );

            al_draw_text(
                font,
                al_map_rgb(255,255,255),
                (WEIGHT/2),
                (HEIGHT/2) - 10,
                ALLEGRO_ALIGN_CENTER,
                "- PRESS ENTER -"
            );

            al_flip_display();
            
        }
    }

    // tirando os nos restantes da lista encadeada
    removeAllBombs(&bomb);
    removeAllEnemys(&enemy);

    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);

    return 0;
}

// -=-=-=-=-=-=-=-=-= bombas e explosoes -=-=-=-=-=-=-=-=-=

void addBomb(BOMB **bombs, int x, int y, int timer) {

    BOMB *aux        = NULL;
    aux              = (BOMB*) malloc(sizeof(BOMB));
    aux->bomb        = bomba;
    aux->tile_x      = (x + 8) / 16;
    aux->tile_y      = (y + 8) / 16;
    aux->x           = aux->tile_x * 16;
    aux->y           = aux->tile_y * 16;
    aux->w           = 16;
    aux->h           = 16;
    aux->frame_y     = 0;
    aux->frame       = 0;
    aux->amount_x    = 3;
    aux->amount_y    = 1;
    aux->timer       = timer;
    aux->explosion   = false;
    aux->solid       = false;
    aux->next        = NULL;
    
    // estrutura de fila,
    if (*bombs == NULL) *bombs = aux;
    else {
        aux->next = *bombs;
        *bombs = aux;
    }
}

int calculoParedes(BOMB **b, int map[][COL]) {
    int x = (*b)->tile_x; 
    int y = (*b)->tile_y; 
    // printf("%d %d %d\n", x, y, COL);
    for (int i = x; x+2 < COL && i < x+2; i++) {
        int bloco = map[y][i];

        if (bloco == HARD_BLOCK || bloco == BROKEN_BLOCK) {
            break;
        }
        (*b)->cordenadas_bloco[0]++;
    }
    for (int i = x; x-2 >= 0 && i > x-2; i--) {
        int bloco = map[y][i];

        if (bloco == HARD_BLOCK || bloco == BROKEN_BLOCK) {
            break;
        }
        (*b)->cordenadas_bloco[1]++;

    }
    for (int i = y; y+2 < ROW && i < y+2; i++) {
        int bloco = map[i][x];
        if (bloco == HARD_BLOCK || bloco == BROKEN_BLOCK) {
            break;
        }
        (*b)->cordenadas_bloco[2]++;

    }
    for (int i = y; y-2 >= 0 && i > y-2; i--) {
        int bloco = map[i][x];
        if (bloco == HARD_BLOCK || bloco == BROKEN_BLOCK) {
            break;
        }
        (*b)->cordenadas_bloco[3]++;
    }

}

void toExplosion(BOMB **b, int map[][COL]) {
    (*b)->bomb      = explosao;
    (*b)->w         = 79;
    (*b)->h         = 79;
    (*b)->x         = (*b)->x - ((*b)->w / 2) +8;
    (*b)->y         = (*b)->y - ((*b)->h / 2) +8;
    (*b)->frame_y   = 0;
    (*b)->frame     = 0;
    (*b)->amount_x  = 2;
    (*b)->amount_y  = 2;
    (*b)->explosion = true;
    (*b)->timer = FPS * 2;
    for (int i = 0; i < 4; i++) {
        (*b)->cordenadas_bloco[i] = 0;
    }

    calculoParedes(&(*b), map);
} 

void verifyFrame(BOMB **aux) {
    if (!(*aux)->explosion) {
        if ((*aux)->timer >= FPS * 2) (*aux)->frame = 2;
        else if ((*aux)->timer >= FPS) (*aux)->frame = 1;
        else (*aux)->frame = 0;
    } else {
        if ((*aux)->timer >= FPS) (*aux)->frame = 1;
        else (*aux)->frame = 0;           
    }
}

void listBomb(BOMB **bombs, ENEMY **enemys, int map[][COL]) {

    BOMB *current = *bombs;
    BOMB *prev = NULL;

    while (current != NULL) {

        verifyFrame(&current);

        al_draw_bitmap_region(
            current->bomb,
            (int) current->frame * current->w,
            current->frame_y,
            current->w,
            current->h,
            current->x,
            current->y,
            0
        );

        // tempo ate a bomba explodir
        current->timer--;

        // virou explosão
        if (current->timer <= 0 && !current->explosion) {
            toExplosion(&current, map);
        }
        
        // remove da lista
        else if (current->timer <= 0 && current->explosion) {
            
            BOMB *to_delete = current;

            if (prev == NULL) {
                *bombs = current->next;
                current = current->next;
            } else {
                prev->next = current->next;
                current = current->next;
            }
            
            free(to_delete);
            continue;
        }
        
        if (current->explosion) {
            int x = current->tile_x;
            int y = current->tile_y;

            int bloco_escolhido;
            if (current->timer >= FPS * 2) {
                bloco_escolhido = FIRED_BLOCK;
            } else if (current->timer >= FPS * 1) {
                bloco_escolhido = DESTROYED_BLOCK;
            } else {
                bloco_escolhido = GROUND;
            }

            for (int i = x; x+current->cordenadas_bloco[0] < COL && i <= x+current->cordenadas_bloco[0]; i++) {
                int bloco = map[y][i];
                if (bloco == HARD_BLOCK) break;
                
                // caso nao seja bloco para quebrar, nao exibir um bloco pegando fogo onde eh grama
                map[y][i] = bloco != GROUND ? bloco_escolhido : bloco;
                killForBomb(i, y, &(*enemys));
                if (bloco == BROKEN_BLOCK) break;
                
            }
            for (int i = x; x-current->cordenadas_bloco[1] >= 0 && i >= x-current->cordenadas_bloco[1]; i--) {
                int bloco = map[y][i];
                if (bloco == HARD_BLOCK) break;
                
                // caso nao seja bloco para quebrar, nao exibir um bloco pegando fogo onde eh grama
                map[y][i] = bloco != GROUND ? bloco_escolhido : bloco;
                killForBomb(i, y, &(*enemys));

                if (bloco == BROKEN_BLOCK) break;
            }
            for (int i = y; y+current->cordenadas_bloco[2] < ROW && i <= y+current->cordenadas_bloco[2]; i++) {
                int bloco = map[i][x];
                if (bloco == HARD_BLOCK) break;

                // caso nao seja bloco para quebrar, nao exibir um bloco pegando fogo onde eh grama
                map[i][x] = bloco != GROUND ? bloco_escolhido : bloco;
                killForBomb(x, i, &(*enemys));

                if (bloco == BROKEN_BLOCK) break;
            }
            for (int i = y; y-current->cordenadas_bloco[3] >= 0 && i >= y-current->cordenadas_bloco[3]; i--) {
                int bloco = map[i][x];
                if (map[i][x] == HARD_BLOCK) break;

                // caso nao seja bloco para quebrar, nao exibir um bloco pegando fogo onde eh grama
                map[i][x] = bloco != GROUND ? bloco_escolhido : bloco;
                killForBomb(x, i, &(*enemys));

                if (bloco == BROKEN_BLOCK) break;
            }
            
            
            
        }

        prev = current;
        current = current->next;
    }
}

void removeAllBombs(BOMB **bombs) {
    while (*bombs != NULL) {
        BOMB *aux = *bombs;
        *bombs = (*bombs)->next;
        free(aux);
    }
    *bombs = NULL;
}

// -=-=-=-=-=-=-=-=-= inimigos -=-=-=-=-=-=-=-=-=

void addEnemy(ENEMY **enemys, int x, int y) {
    ENEMY *aux        = NULL;
    aux              = (ENEMY*) malloc(sizeof(ENEMY));
    aux->enemy        = inimigo;
    aux->tile_x      = x; // transformando em tile
    aux->tile_y      = y;
    aux->x           = aux->tile_x * 16;
    aux->y           = aux->tile_y * 16;
    aux->w           = 16;
    aux->h           = 16;
    aux->frame_y     = 0;
    aux->frame       = 0;
    aux->amount_x    = 11;
    aux->amount_y    = 4;
    aux->next        = NULL;
    aux->die         = 0;
    aux->direction   = rand() % 4 + 1;
    
    // estrutura de fila,
    if (*enemys == NULL) *enemys = aux;
    else {
        aux->next = *enemys;
        *enemys = aux;
    }
}

void listEnemy(ENEMY **enemys, BOMB *bombs, int map[][COL]) {
    ENEMY *current = *enemys;
    while (current != NULL) 
    {
        int x = current->x;
        int y = current->y;

        int choice = current->direction;
        int velocidade = (SPEED-1);

        if (!current->die) {
            if (choice == 1 && canMoveEnemy(map, x + velocidade, y, current->w, current->h, bombs)) {
                current->x += velocidade;
                current->frame_y = 0;
                current->frame = current->frame > 3 ? 0 : current->frame + 0.2f;  
            } else if (choice == 1) {
                current->direction = rand() % 4 + 1;
                choice = current->direction;
            }
            if (choice == 2 && canMoveEnemy(map, x - velocidade, y, current->w, current->h, bombs)) {
                current->x -= velocidade;
                current->frame_y = 0;
                current->frame = current->frame < 4 || current->frame >= 6 ? 4 : current->frame + 0.2f;  
            } else if (choice == 2) {
                current->direction = rand() % 4 + 1;
                choice = current->direction;
            }
            if (choice == 3 && canMoveEnemy(map, x, y + velocidade, current->w, current->h, bombs)) {
                current->y += velocidade;
                current->frame_y = 0;
            } else if (choice == 3) {
                current->direction = rand() % 4 + 1;
                choice = current->direction;
            }
            
            if (choice == 4 && canMoveEnemy(map, x, y - velocidade, current->w, current->h, bombs)) {
                current->y -= velocidade;
                current->frame_y = 0;
            } else if (choice == 4) {
                current->direction = rand() % 4 + 1;
                choice = current->direction;
            }

        } else {
            current->frame = current->frame < 7 ? 7 : current->frame + 0.2f;
            if (current->frame > 10) {
                removeEnemy(&(*enemys), map);
            }  
        }

        al_draw_bitmap_region(
            current->enemy,
            ((int)current->frame) * current->w,
            current->frame_y * current->h,
            current->w,
            current->h,
            current->x,
            current->y,
            0
        );

        current = current->next;
    }
    
}

void removeEnemy(ENEMY **enemys, int map[][COL]) {
    
    ENEMY *current = *enemys;
    ENEMY *prev = current;

    while (current != NULL) {

        // se estiver no inicio
        if ((*enemys)->die) {
            ENEMY *aux = (*enemys);
            if ((*enemys)->next == NULL) {
                (*enemys) = NULL;
                break;
            } else {
                (*enemys) = (*enemys)->next;
            }
            current = (*enemys);
            free(aux);

        }

        // depois do inicio
        else if (current->die) {
            ENEMY *aux = current;
            prev->next = current->next;
            current = current->next == NULL ? prev : current->next;
            free(aux);
        }
        prev = current;
        current = current->next;
    }
}

void removeAllEnemys(ENEMY **enemys) {
    while (*enemys != NULL) {
        ENEMY *aux = *enemys;
        *enemys = (*enemys)->next;
        free(aux);
    }
    *enemys = NULL;
}

// -=-=-=-=-=-=-=-=-= jogador e game em geral -=-=-=-=-=-=-=-=-=

SPRITE makeSprite(char *path_sprite, int x, int y, int w, int h, int frame_y, float frame, int amount_x, int amount_y) {
    SPRITE sprite;
    sprite.skin       = al_load_bitmap(path_sprite);
    sprite.x          = x;
    sprite.y          = y;
    sprite.w          = w;
    sprite.h          = h;
    sprite.frame_y    = frame_y;
    sprite.frame      = frame;
    sprite.amount_x   = amount_x;
    sprite.amount_y   = amount_y;
    return sprite;
}


int killForEnemy(ENEMY **enemys) {
    // convertendo para tile pois player_x esta em pixels 
    // (poderiamos converter a explosao para pixel fazendo * 16)
    int player_tile_x = (player_x + 8) / 16;
    int player_tile_y = (player_y + 8) / 16;

    ENEMY *curr = *enemys;
    while (curr != NULL) {
        int enemy_x = (curr->x + 8) / 16, enemy_y = (curr->y + 8) / 16;
        // caso o inimigo tenha se encontrado com nosso player
        if (enemy_x == player_tile_x && enemy_y == player_tile_y) {
            end_game = 1;
            // printf("inimigo matou o player\n");
            return 1;
        }
        curr = curr->next;
    }

    return 0;
}

int killForBomb(int b_x, int b_y, ENEMY **enemys) {
    // convertendo para tile pois player_x esta em pixels 
    // (poderiamos converter a explosao para pixel fazendo * 16)
    int player_tile_x = (player_x + 8) / 16;
    int player_tile_y = (player_y + 8) / 16;
    // printf("posicoes: %d %d = %d %d\n", player_tile_x, player_tile_y, b_x, b_y);
    if (player_tile_x == b_x && player_tile_y == b_y) {
        // como vamos utilizar essa variavel para ver se o player morreu, precisamos antes verificar se esse metodo nao foi chamado por um inimigo
        end_game = 1;  
        // printf("FIM DE JOGOOOO\n");
        return 1;
    }


    ENEMY *curr = *enemys;
    while (curr != NULL) {
        int enemy_x = (curr->x + 8) / 16, enemy_y = (curr->y + 8) / 16;
        //printf("inimigo %d %d = %d %d = %d %d\n", enemy_x, enemy_y, b_x, b_y, player_tile_x, player_tile_y);
        // caso o inimigo tenha encostado na explosao
        if (enemy_x == b_x && enemy_y == b_y) {
            // como vamos utilizar essa variavel para ver se o player morreu, precisamos antes verificar se esse metodo nao foi chamado por um inimigo
            curr->die = 1;
            return 1;
        }

        curr = curr->next;
    }

    return 0;
}

void createMapping(int map[][COL], ENEMY **enemys, SPRITE *saida) {
    srand(time(NULL));
    int existe_saida = 0;
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            int aleatory_block = rand() % 100;
            if (i == 0 || i == ROW-1 || j == 0 || j == COL-1 || (i % 2 == 0 && j % 2 == 0)) map[i][j] = HARD_BLOCK;
            else if ((i == 1 && j == 1) || (i == 1 && j == 2) || (i == 2 && j == 1)) map[i][j] = GROUND;
            else if (aleatory_block < 55) {
                int chances = rand() % 3;
                if (!existe_saida && i >= ROW / 2 && j >= COL / 2 && chances == 1) {
                    saida->x = i;
                    saida->y = j;
                    // printf("SAIDA %d %d\n", i, j);
                    existe_saida = 1;
                } else if (i == ROW -1 && !existe_saida) {
                    saida->x = i;
                    saida->y = j;
                    existe_saida = 1;
                    // printf("SAIDA ULTIMA LINHA %d %d\n", i, j);
                }
                map[i][j] = BROKEN_BLOCK;
            }
            else {
                int spawn = rand() % 4;
                if (spawn == 1 && (i > 1 && j > 1) && (i > 1 && j > 2) && (i > 2 && j > 1)) {
                    addEnemy(&(*enemys), j, i);
                }
                map[i][j] = GROUND;
            }
        }
    }
}

void renderMap(int map[][COL], SPRITE obj, SPRITE saida) {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (map[i][j] == HARD_BLOCK) {
                al_draw_bitmap_region(obj.skin, obj.frame, obj.frame_y*0, obj.w, obj.h, obj.w*j, obj.h*i, 0);
            } else if (map[i][j] == BROKEN_BLOCK) {
                al_draw_bitmap_region(obj.skin, obj.frame+obj.amount_x*2, obj.frame_y*0, obj.w, obj.h, obj.w*j, obj.h*i, 0);
            } else if (map[i][j] == FIRED_BLOCK) {
                al_draw_bitmap_region(obj.skin, obj.frame+obj.amount_x*4, obj.frame_y*0, obj.w, obj.h, obj.w*j, obj.h*i, 0);
            } else if (map[i][j] == DESTROYED_BLOCK) {
                al_draw_bitmap_region(obj.skin, obj.frame+obj.amount_x*6, obj.frame_y*0, obj.w, obj.h, obj.w*j, obj.h*i, 0);
            } else if ((map[i][j] == GROUND || map[i][j] == OUT) && saida.x == i && saida.y == j) {
                map[i][j] = OUT;
                al_draw_bitmap_region(saida.skin, 0, 0, obj.w, obj.h, obj.w*j, obj.h*i, 0);
            }
        }
    }
}

bool canMove(int map[][COL], int next_x, int next_y, int w, int h, BOMB *bomb) {

    int left   = (next_x + 1) / 16;
    int right  = (next_x + w - 3) / 16;

    int top    = (next_y + 1) / 16;
    int bottom = (next_y + h - 3) / 16;

        
    if (
            map[top][left] == HARD_BLOCK ||
            map[top][right] == HARD_BLOCK ||
            map[bottom][left] == HARD_BLOCK ||
            map[bottom][right] == HARD_BLOCK ||
            map[top][left] == BROKEN_BLOCK ||
            map[top][right] == BROKEN_BLOCK ||
            map[bottom][left] == BROKEN_BLOCK ||
            map[bottom][right] == BROKEN_BLOCK 
    ) 
    {
        return false;
    }
    
    if (
            map[top][left] == OUT ||
            map[top][right] == OUT ||
            map[bottom][left] == OUT ||
            map[bottom][right] == OUT
    ) {
        win      = 1;
        gameOver = 1;
        return true;
    }
        
    // solid eh uma var auxiliar para que o player nao ficque preso dentro da bomba
    int player_tile_x = (next_x + 8) / 16;
    int player_tile_y = (next_y + 8) / 16;
    while (bomb != NULL) {
        int bomb_tile_x = (bomb->x + 8) / 16;
        int bomb_tile_y = (bomb->y + 8) / 16;
        if (bomb->solid && player_tile_x == bomb_tile_x && player_tile_y == bomb_tile_y)
        {
            return false;
        }
        
        if (!bomb->solid) {
            
            if (player_tile_x != bomb_tile_x ||
                player_tile_y != bomb_tile_y) {
                    bomb->solid = true;
            }
        }
        bomb = bomb->next;
    }
    return true;
}

bool canMoveEnemy(int map[][COL], int next_x, int next_y, int w, int h, BOMB *bomb) {

    int left   = (next_x + 1) / 16;
    int right  = (next_x + w - 3) / 16;

    int top    = (next_y + 1) / 16;
    int bottom = (next_y + h - 3) / 16;

        
    if (
            map[top][left] == HARD_BLOCK ||
            map[top][right] == HARD_BLOCK ||
            map[bottom][left] == HARD_BLOCK ||
            map[bottom][right] == HARD_BLOCK ||
            map[top][left] == BROKEN_BLOCK ||
            map[top][right] == BROKEN_BLOCK ||
            map[bottom][left] == BROKEN_BLOCK ||
            map[bottom][right] == BROKEN_BLOCK
    ) 
    {
        return false;
    }
        
        
    // solid eh uma var auxiliar para que o player nao ficque preso dentro da bomba
    int player_tile_x = (next_x + 8) / 16;
    int player_tile_y = (next_y + 8) / 16;
    while (bomb != NULL) {
        int bomb_tile_x = (bomb->x + 8) / 16;
        int bomb_tile_y = (bomb->y + 8) / 16;
        if (bomb->solid && player_tile_x == bomb_tile_x && player_tile_y == bomb_tile_y)
        {
            return false;
        }
        
        bomb = bomb->next;
    }
    return true;
}
