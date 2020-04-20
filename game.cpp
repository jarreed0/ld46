#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
//#include <SDL2/SDL_Mixer.h>
#include <vector>
#include <iostream>

#define WIDTH 1600//1280//580
#define HEIGHT 900//720//620
#define FONT_SIZE 18
#define PI 3.14159265358979323846
#define SPEED 5
#define ARROW_SPEED 47//22
#define WAVE_REST 27
#define WOOD_TOTAL 200
#define STEEL_TOTAL 600

struct object {
 SDL_Rect dest, src;
 int img;
 double velX, velY, maxVel;
 int alpha=255;
 double angle=0;
 //double x, y;
 bool drawable=1;
 SDL_Point center={0,0};;
 SDL_RendererFlip flip=SDL_FLIP_NONE;
};

struct color {
    int r, g, b;
};

struct status_bar {
    SDL_Rect bar;
    SDL_Rect status;
    SDL_Rect change;
    SDL_Rect statusFull;
    double filled;
    double total;
    color c;
    bool active;
    bool full;
    int id=1;
} prayer;

struct arrow {
    SDL_Rect rect;
    int velX, velY;
    bool bounce=0;
    object o;
} tmpArrow;

struct enemy {
    object o;
    int velX, velY;
    int life;
    int spawn;
    int target;
    int speed = SPEED;
    double damage = .8;
    int frame, fC;
} tmpEnemy;

bool running;

int mouseX, mouseY;
bool left, right, up, down;
bool fire;
int quiver;
int reload, reloadTime;
bool dead, win, intro=1;
bool litFurnace=0, stoneInFurnace=0;

int wave=0;
int lastWaveEnd;

SDL_Renderer* renderer;
SDL_Window* window;
TTF_Font *font;
SDL_Color fcolor;

int frameCount, timerFPS, lastFrame, fps;

std::vector<SDL_Texture*> textures;

//Mix_Chunk *hit, *walk, *wisp;

object player, girl;
SDL_Rect statue, podium, col, knee;
std::vector<arrow> arrows;
double angle, rad;
bool praying, pressedQ, repairing;
object spawn[4];
status_bar barrier[4];
std::vector<enemy> enemies; 
//int enemyCount=0;
status_bar health;
int frame, frameDelay;

object map, statImg, cursor, quiverImg, bow, mapwall;
int mapPray, statPray, mapI, statI;

bool carryingWood, carryingStone, carryingSteel;
status_bar wood_status, stone_status, steel_status;
SDL_Rect wood, stone, steel;
SDL_Rect woods, stone_pile;
object holding, furnace;
object fireF;
int fireC;

void write(std::string text, int x, int y, int r, int g, int b) {
    if(!text.empty()) {
        SDL_Surface *surface;
        SDL_Texture *texture;
        if (font == NULL) {
        fprintf(stderr, "error: font not found\n");
        exit(EXIT_FAILURE);
        }
        fcolor.r = r;
        fcolor.g = g;
        fcolor.b = b;
        SDL_Rect rect;
        const char* t = text.c_str();
        surface = TTF_RenderText_Solid(font, t, fcolor);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        rect.w = surface->w;
        rect.h = surface->h;
        rect.x = x-rect.w;
        rect.y = y-rect.h;
        SDL_FreeSurface(surface);
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
    }
}

void writeHUD(std::string text, int x, int y, int r, int g, int b) {
    if(!text.empty()) {
        SDL_Surface *surface;
        SDL_Texture *texture;
        if (font == NULL) {
        fprintf(stderr, "error: font not found\n");
        exit(EXIT_FAILURE);
        }
        fcolor.r = r;
        fcolor.g = g;
        fcolor.b = b;
        SDL_Rect rect;
        const char* t = text.c_str();
        surface = TTF_RenderText_Solid(font, t, fcolor);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        rect.w = surface->w;
        rect.h = surface->h;
        rect.x = x;
        rect.y = y;
        SDL_FreeSurface(surface);
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
    }
}

void spawnEnemy() {
    //tmpEnemy.o.dest.w=tmpEnemy.o.dest.h=35;
    tmpEnemy.o.dest.w=56;tmpEnemy.o.dest.h=67;
    tmpEnemy.spawn = rand() % 4;
    tmpEnemy.target = rand() % 10;
    tmpEnemy.o.dest.x=spawn[tmpEnemy.spawn].dest.x+5+(rand() % 12);
    tmpEnemy.o.dest.y=spawn[tmpEnemy.spawn].dest.y+(rand() % (spawn[0].dest.h-40));
    tmpEnemy.life=12;
    if(rand() % 45 - wave <= 1 && wave > 7) tmpEnemy.life=13;
    if(rand() % 100 - wave <= 1 && wave > 14) tmpEnemy.life=14;
    if(rand() % 55 - wave <= 1 && wave > 9) tmpEnemy.speed=SPEED*1.2;
    if(rand() % 90 - wave <= 1 && wave > 16) tmpEnemy.speed=SPEED*1.6;
    if(rand() % 45 - wave <= 1 && wave > 10) tmpEnemy.damage=1;
    if(rand() % 60 - wave <= 1 && wave > 17) tmpEnemy.damage=2;
    if(rand() % 70 - wave <= 1 && wave > 11) {tmpEnemy.o.dest.w*=2; tmpEnemy.o.dest.h*=2;}
    enemies.push_back(tmpEnemy);
    //enemyCount++;
}

void startWave() {
    wave++;
    int spawnSize = 2 + (rand() % wave*.8);//(wave*.6);
    //if(spawnSize==0) spawnSize=2;
    for(int i=0; i<spawnSize; i++) {
        spawnEnemy();
    }
}

void updateEnemy() {
    //std::cout << enemyCount << std::endl;

    for(int i=0; i<enemies.size(); i++) {
        if(enemies[i].velX > 0) enemies[i].o.flip=SDL_FLIP_HORIZONTAL;
        if(enemies[i].velX < 0) enemies[i].o.flip=SDL_FLIP_NONE;
        if(enemies[i].fC>8) {
            enemies[i].frame++;
            if(enemies[i].frame>1) enemies[i].frame=0;
            enemies[i].o.src.x=enemies[i].frame* enemies[i].o.src.w;
            enemies[i].fC=0;
            /*if(SDL_HasIntersection(&enemies[i].o.dest, &col)) {
                enemies[i].o.src.x+=116;
                if(enemies[i].o.dest.x>WIDTH/2) {
                    enemies[i].o.flip=SDL_FLIP_HORIZONTAL;
                } else {
                    enemies[i].o.flip=SDL_FLIP_NONE;
                }
            }*/
        }
        //enemies[i].lastLife=enemies[i].life;
        if(enemies[i].target!=1)enemies[i].target = rand() % 1000;
        //std::cout << prayer.filled << std::endl;
        //if(prayer.filled!=0) enemies[i].target=1;
        //if(prayer.filled!=0) enemies[i].target=1;
        if(enemies[i].o.src.y!=0 && enemies[i].life!=0)enemies[i].o.src.y=0; 
        if(enemies[i].life<=0) {
            enemies.erase(enemies.begin()+i);
            ////Mix_PlayChannel(-1, hit, 0);
            i++;
            //enemyCount--;
            if(enemies.size()<=0) lastWaveEnd=SDL_GetTicks();
        }
        if(enemies[i].life<12) enemies[i].life--;
        if(SDL_HasIntersection(&enemies[i].o.dest, &spawn[enemies[i].spawn].dest) && barrier[enemies[i].spawn].filled!=0) {
            barrier[enemies[i].spawn].filled-=.2;
        } else {
            double r=0;
            if(enemies[i].target!=1 && !SDL_HasIntersection(&player.dest, &enemies[i].o.dest)) {
                r = atan2(((player.dest.y+(player.dest.h/2))-(enemies[i].o.dest.y+(enemies[i].o.dest.h/2))), ((player.dest.x+(player.dest.w/2))-(enemies[i].o.dest.x+(enemies[i].o.dest.w/2))));
            } else if(!SDL_HasIntersection(&podium, &enemies[i].o.dest)) {
                r = atan2(((podium.y+(podium.h/2))-(enemies[i].o.dest.y+(enemies[i].o.dest.h/2))), ((podium.x+(podium.w/2))-(enemies[i].o.dest.x+(enemies[i].o.dest.w/2))));
            }
            if(SDL_HasIntersection(&podium, &enemies[i].o.dest)) {
                if(prayer.filled>0){prayer.filled-=enemies[i].damage/2;}
                else {health.filled-=enemies[i].damage/4;girl.src.y=15;}
                prayer.active=1;
                ////Mix_PlayChannel(-1, hit, 0);
            }
            if(SDL_HasIntersection(&enemies[i].o.dest, &player.dest)) {
                health.filled-=enemies[i].damage*2;
                player.src.y=30;
                health.active=1;
                //Mix_PlayChannel(-1, hit, 0);
            } else {
                health.active=0;
            }
            enemies[i].velX=cos(r)*enemies[i].speed;
            enemies[i].velY=sin(r)*enemies[i].speed;
            if(!SDL_HasIntersection(&player.dest, &enemies[i].o.dest) && !SDL_HasIntersection(&podium, &enemies[i].o.dest)) {
                enemies[i].o.dest.x+=enemies[i].velX;
                enemies[i].o.dest.y+=enemies[i].velY;
                if((SDL_HasIntersection(&enemies[i].o.dest, &col) || SDL_HasIntersection(&enemies[i].o.dest, &furnace.dest)) && enemies[i].velX!=0) {
                    enemies[i].o.dest.x-=enemies[i].velX;
                    enemies[i].o.dest.y-=enemies[i].velY;
                }
            }
        }
        if((enemies[i].velX!=0 || enemies[i].velY!=0) && !&enemies[i].o.dest, &col)enemies[i].fC++;
    }
}

void updateStatus(status_bar* sb) {
    sb->full=0;
    sb->change=sb->bar;
    sb->change.x-=3;//sb->bar.w*.1;
    sb->change.y-=3;//sb->bar.h*.1;
    sb->change.w+=6;//sb->bar.w*.2;
    sb->change.h+=6;//sb->bar.h*.2;
    if(sb->filled>sb->total) {sb->filled=sb->total;sb->full=1;}
    if(sb->filled<0) sb->filled=0;
    sb->statusFull=sb->bar;
    sb->statusFull.x+=2;//sb->bar.w*.05;
    sb->statusFull.y+=2;//sb->bar.h*.1;
    sb->statusFull.w-=4;//sb->bar.w*.1;
    sb->statusFull.h-=4;//sb->bar.h*.2;
    sb->status=sb->statusFull;
    double w = ((double)sb->filled/(double)sb->total)*(double)sb->statusFull.w;
    sb->status.w=w;
}

void drawStatus(status_bar sb) {
    if(sb.active) {
        SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255);
        SDL_RenderFillRect(renderer, &sb.change);
    }
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderFillRect(renderer, &sb.bar);
    SDL_SetRenderDrawColor(renderer, sb.c.r/2, sb.c.g/2, sb.c.b/2, 100);
    SDL_RenderFillRect(renderer, &sb.statusFull);
    SDL_SetRenderDrawColor(renderer, sb.c.r, sb.c.g, sb.c.b, 255);
    SDL_RenderFillRect(renderer, &sb.status);
    //write(std::to_string(sb.id), sb.bar.x, sb.bar.y, 255, 255, 255);
}

bool outOfScreen(SDL_Rect o) {
    if(o.x+o.w<0)return 1;
    if(o.x>WIDTH)return 1;
    if(o.y+o.h<0)return 1;
    if(o.y>HEIGHT)return 1;
    return 0;
}
bool edge(SDL_Rect o) {
    if(o.x<0)return 1;
    if(o.x+o.w>WIDTH)return 1;
    if(o.y<105)return 1;
    if(o.y+o.h>HEIGHT)return 1;

    //EDGES
    //if(o.x < ((-150/(13*o.x+0.000001))+(24000/13))) return 1;
    //if(o.x > ((300/(29*o.x+0.000001))-(432000/29))) return 1;
    if(o.y < ((-12*o.x) + 1820)) return 1;
    if(o.y < (11.25*(o.x+o.w)) - 16300) return 1;
    //if(o.x > (((45/4)*o.x)-16200)) return 1;
    //

    return 0;
}

void updateBow() {
    rad = atan2((mouseY-tmpArrow.rect.y), (mouseX-tmpArrow.rect.x));
    angle=rad*(PI/180);
    if(!praying && !repairing) {bow.angle=rad*(180/PI)-45;tmpArrow.o.angle=rad*(180/PI);}
    if((mouseX<(player.dest.x+(player.dest.w/2))) && !praying && !repairing) {
        bow.dest.x=player.dest.x-bow.dest.w/2;
    } else if(!praying && !repairing) {
        bow.dest.x=player.dest.x+player.dest.w-bow.dest.w/2;
    }
    bow.dest.y=player.dest.y+15;
    if(praying || repairing) bow.dest.y+=10;
    quiverImg.src.x=0;
    if(quiver>=1) quiverImg.src.x=quiverImg.src.w;
    if(quiver>=3) quiverImg.src.x=quiverImg.src.w*2;
    if(quiver>=4) quiverImg.src.x=quiverImg.src.w*3;
    if(quiver>=6) quiverImg.src.x=quiverImg.src.w*4;
}

void updateArrows() {
    tmpArrow.rect.x=bow.dest.x+bow.dest.w/2;
    tmpArrow.rect.y=bow.dest.y+bow.dest.h/2;
    //if(quiver>0 && reload>reloadTime) {
        
        tmpArrow.rect.w=10;//44;
        tmpArrow.rect.h=10;//8;
        tmpArrow.velX=cos(rad)*(ARROW_SPEED);
        tmpArrow.velY=sin(rad)*(ARROW_SPEED);
        tmpArrow.o.dest=tmpArrow.rect;
        tmpArrow.o.dest.w=44; tmpArrow.o.dest.h=8;
        tmpArrow.o.src.x=0;
        tmpArrow.o.src.w=22;
        tmpArrow.o.src.h=4;
        tmpArrow.o.angle=rad * (180/PI);
        tmpArrow.o.flip=SDL_FLIP_NONE;
        if(mouseX<player.dest.x) {tmpArrow.o.flip=SDL_FLIP_VERTICAL;}
    //}
    for(int i=0; i<arrows.size(); i++) {
        arrows[i].rect.x+=arrows[i].velX;
        arrows[i].rect.y+=arrows[i].velY;
        if(edge(arrows[i].rect) || SDL_HasIntersection(&col, &arrows[i].rect) || SDL_HasIntersection(&arrows[i].rect, &furnace.dest)) {
            arrows[i].rect.x-=arrows[i].velX;
            arrows[i].rect.y-=arrows[i].velY;
            arrows[i].bounce=1;
            //FLIP ANGLE ALSO
            //arrows[i].o.angle+=180;
            arrows[i].velX=-arrows[i].velX;
            arrows[i].velY=-arrows[i].velY;
        }
        if(arrows[i].bounce) {
            arrows[i].velX=arrows[i].velX/2;
            arrows[i].velY=arrows[i].velY/2;
        }
        if(arrows[i].velX==0 && SDL_HasIntersection(&player.dest, &arrows[i].rect)) {
            arrows.erase(arrows.begin()+i);
            i++;
            quiver++;
        }
        if(arrows[i].velX!=0) {
            for(int j=0; j<enemies.size(); j++) {
                if(SDL_HasIntersection(&arrows[i].rect, &enemies[j].o.dest) && !arrows[i].bounce) {
                    enemies[j].life--;
                    enemies[j].o.src.y=73;
                    arrows[i].bounce=1;
                }//[i].velX=arrows[i].velY=0;};
            }
        }
    }
}

int setImage(std::string filename) {
  SDL_Texture* tex;

  SDL_Surface* surf;
  if(IMG_Load(filename.c_str()) < 0) {
    std::cout << "Failed at IMG_Load()" << IMG_GetError() << std::endl;
  } else {
    surf= IMG_Load(filename.c_str());
  }

  tex = IMG_LoadTexture(renderer, filename.c_str());
  textures.push_back(tex);
  return textures.size()-1;
}

void draw(object o) {
  SDL_Rect dest = o.dest;
  SDL_Rect src = o.src;
  SDL_SetTextureAlphaMod(textures[o.img], o.alpha);
  SDL_Point c=o.center;
  SDL_RendererFlip flip = o.flip; 
  SDL_RenderCopyEx(renderer, textures[o.img], &src, &dest, o.angle, &c, flip);
}
void draw(std::vector<object> o) {
for(int i=0; i<o.size(); i++) {draw(o[i]);}
}


void fireArrow() {
    if(quiver>0 && reload>reloadTime) {
        /*tmpArrow.rect.x=bow.dest.x+bow.dest.w/2;
        tmpArrow.rect.y=bow.dest.y+bow.dest.h/2;
        tmpArrow.rect.w=10;//44;
        tmpArrow.rect.h=10;//8;
        tmpArrow.velX=cos(rad)*(ARROW_SPEED);
        tmpArrow.velY=sin(rad)*(ARROW_SPEED);
        tmpArrow.o.dest=tmpArrow.rect;
        tmpArrow.o.dest.w=44; tmpArrow.o.dest.h=8;
        tmpArrow.o.src.x=0;
        tmpArrow.o.src.y=(rand() % 3) * 4;
        tmpArrow.o.src.w=22;
        tmpArrow.o.src.h=4;
        tmpArrow.o.angle=rad * (180/PI);
        tmpArrow.o.flip=SDL_FLIP_NONE;
        if(mouseX<player.dest.x) {tmpArrow.o.flip=SDL_FLIP_VERTICAL;}*/
        arrows.push_back(tmpArrow);
        //Mix_PlayChannel(-1, wisp, 0);
        player.dest.x-=cos(rad)*6;
        if(SDL_HasIntersection(&player.dest, &col) || edge(player.dest) || SDL_HasIntersection(&player.dest, &furnace.dest)) {
            player.dest.x+=cos(rad)*6;
        }
        player.dest.y-=sin(rad)*4;
        if(SDL_HasIntersection(&player.dest, &col) || edge(player.dest) || SDL_HasIntersection(&player.dest, &furnace.dest)) {
            player.dest.y+=sin(rad)*6;
        }
        quiver--;
        reload=0;
        tmpArrow.o.src.y=(rand() % 3) * 4;
    }
}

void update() {


    if(up || down || right || left) //Mix_PlayChannel(-1, walk, 0);

    girl.src.y=0;

    frameDelay++;
    if(!praying && !repairing) {
    if((right || left || up || down) && frameDelay>50){frame++; frameDelay=0;}
    else if(frameDelay>120 ){frame++; frameDelay=0;}
    if(frame>3) frame=0;
    if(!right && !left && !down && !up && frame>1) frame=0;
    player.src.x=player.src.w*frame;
    if(mouseX>player.dest.x+player.dest.w/2) player.src.x+=60;
    if(player.src.y>0) player.src.y=0;
    }
    if(praying) player.src.x=15;

    fireC++;
    if(fireC>30)fireC=0;
    fireF.src.x=0;
    if(fireC>15)fireF.src.x=43;

    if(praying || repairing) reload=0;

    if(SDL_GetTicks()>lastWaveEnd+(WAVE_REST*1000) && enemies.size()<=0) {
        startWave();
    }

    /*if(carryingSteel) std::cout << "steel" << std::endl;
    if(carryingWood) std::cout << "wood" << std::endl;
    if(carryingStone) std::cout << "stone" << std::endl;*/

    if(health.filled<=0) dead=1;
    if(!praying && !repairing) {
        if(left) player.dest.x-=SPEED;
        if(right) player.dest.x+=SPEED;
        if(SDL_HasIntersection(&player.dest, &col) || edge(player.dest) || SDL_HasIntersection(&player.dest, &furnace.dest)) {
            for(int i=0;i<SPEED; i++) {
                if(left) player.dest.x++;
                if(right) player.dest.x--;
                if(!SDL_HasIntersection(&player.dest, &col) && !edge(player.dest) && !SDL_HasIntersection(&player.dest, &furnace.dest)) {
                    i=SPEED+1;
                    break;
                }
            }
        }
        if(up) player.dest.y-=SPEED;
        if(down) player.dest.y+=SPEED;
        if(SDL_HasIntersection(&player.dest, &col) || edge(player.dest) || SDL_HasIntersection(&player.dest, &furnace.dest)) {
            if(up && !down && edge(player.dest) && player.dest.y>10 && player.dest.y>105) {
                player.dest.y-=SPEED/2;
                if(player.dest.x>WIDTH/2) {
                    player.dest.x--;
                    //player.dest.x=((player.dest.y+16300/11.15)-player.dest.w)+1;
                } else {
                    player.dest.x++;
                    //player.dest.x=(player.dest.y-1820)/-12+1;
                }
            } else {
                for(int i=0;i<player.dest.w; i++) {
                    if(up) player.dest.y++;
                    if(down) player.dest.y--;
                    if(!SDL_HasIntersection(&player.dest, &col) && !edge(player.dest) && !SDL_HasIntersection(&player.dest, &furnace.dest)) {
                        i=player.dest.w+1;
                        break;
                    }
                }
            }

        }
    }
    if(SDL_HasIntersection(&player.dest, &knee) && praying && (player.dest.y+player.dest.h)<knee.y+knee.h && player.dest.x>knee.x && player.dest.x+player.dest.w<knee.x+knee.w) {
        
        //write("press e to pray", player.dest.x+10, player.dest.y+10, 255, 255, 255);
        prayer.active=1;
    }else{
        prayer.active=0;
    }
    updateStatus(&prayer);
    updateStatus(&barrier[0]);
    updateStatus(&barrier[1]);
    updateStatus(&barrier[2]);
    updateStatus(&barrier[3]);
    updateStatus(&health);
    updateStatus(&stone_status);
    updateStatus(&wood_status);
    updateStatus(&steel_status);
    wood_status.filled++;
    stone_status.filled++;
    if(stoneInFurnace && litFurnace) steel_status.filled++;
    wood_status.active=stone_status.active=steel_status.active=0;
    if(wood_status.full) wood_status.active=1;
    if(steel_status.full) steel_status.active=1;
    if(stone_status.full) stone_status.active=1;

    for(int i=0;i<4;i++) {
        if(SDL_HasIntersection(&player.dest, &spawn[i].dest) && repairing) {
            barrier[i].active=1;
            barrier[i].filled+=2;
            if(barrier[i].filled>=barrier[i].total){
                repairing=0;
                carryingSteel=carryingWood=0;
            }
        } else {
            barrier[i].active=0;
        }
    }
    if(prayer.filled>=prayer.total && enemies.size()<=0) {
        praying=0;
        win=1;
        //std::cout << "you won" << std::endl;
    }

    updateBow();
    updateEnemy();
    updateArrows();
    //std::cout << angle << ":" << a << std::endl;
    if(prayer.active) prayer.filled+=.05;
    if(fire && !praying && !barrier[0].active && !barrier[1].active && !barrier[2].active && !barrier[3].active) fireArrow();
    reload++;
}


void input() {
    left=right=up=down=0;
    SDL_Event e;
    const Uint8 *keystates = SDL_GetKeyboardState(NULL);
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) running=false;
        if(e.type==SDL_MOUSEBUTTONUP) fire=0;
        if((e.type == SDL_MOUSEBUTTONDOWN) && e.button.button == SDL_BUTTON_LEFT) fire=1;
    }
    if(keystates[SDL_SCANCODE_ESCAPE]) running=false;
    if(keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_UP]) up=1;
    if(keystates[SDL_SCANCODE_A] || keystates[SDL_SCANCODE_LEFT]) left=1;
    if(keystates[SDL_SCANCODE_S] || keystates[SDL_SCANCODE_DOWN]) down=1;
    if(keystates[SDL_SCANCODE_D] || keystates[SDL_SCANCODE_RIGHT]) right=1;
    //if(keystates[SDL_SCANCODE_E]) fire=1;
    if(keystates[SDL_SCANCODE_Q] && SDL_HasIntersection(&player.dest, &knee) && !pressedQ && (player.dest.y+player.dest.h)<knee.y+knee.h && player.dest.x>knee.x && player.dest.x+player.dest.w<knee.x+knee.w) {praying=!praying;pressedQ=1;}
    for(int i=0; i<4; i++) {
        if(keystates[SDL_SCANCODE_Q] && SDL_HasIntersection(&player.dest, &spawn[i].dest) && !pressedQ && (carryingSteel || carryingWood)) {
            repairing=!repairing;
            pressedQ=1;
            if(carryingWood) {barrier[i].total=WOOD_TOTAL;barrier[i].id=1;}
            if(carryingSteel) {barrier[i].total=STEEL_TOTAL;barrier[i].id=2;}
            if(repairing){carryingWood=carryingSteel=0;}
        }
    }
    if(!keystates[SDL_SCANCODE_Q]) pressedQ=0;
    if(keystates[SDL_SCANCODE_0]) spawnEnemy();
    if(keystates[SDL_SCANCODE_E]) {
        if(stone_status.full && SDL_HasIntersection(&stone_pile, &player.dest) && !carryingWood && !carryingSteel && !carryingStone) {
            carryingStone=1;
            stone_status.filled=0;
        }
        if(wood_status.full && SDL_HasIntersection(&woods, &player.dest) && !carryingWood && !carryingSteel && !carryingStone) {
            carryingWood=1;
            wood_status.filled=0;
        }
        if(SDL_HasIntersection(&fireF.dest, &player.dest)) {
            if(carryingWood && !litFurnace) {
                carryingWood=0;
                litFurnace=1;
            }
            if(carryingStone) {
                stoneInFurnace=1;
                carryingStone=0;
            }
            if(!carryingStone && !carryingWood && !carryingSteel && steel_status.full) {
                stoneInFurnace=litFurnace=0;
                steel_status.filled=0;
                carryingSteel=1;
            }
        }
    }

    SDL_GetMouseState(&mouseX, &mouseY);

}

void render() {
    SDL_SetRenderDrawColor(renderer, 0x70, 0x70, 0x90, 255);
    SDL_RenderClear(renderer);

//    SDL_Rect rect;
//    rect.x=rect.y=0;
//    rect.w=WIDTH;
//    rect.h=HEIGHT;
//    SDL_RenderFillRect(renderer,&rect);

    frameCount++;
    int timerFPS = SDL_GetTicks()-lastFrame;
    if(timerFPS<(1000/60)) {
        SDL_Delay((1000/60)-timerFPS);
    }

    //BKG
    map.img=mapI;
    map.alpha=255;
    draw(map);
    //if(praying) {
        map.img=mapPray;
        map.alpha=(prayer.filled*255)/prayer.total;
        draw(map);
    //}

    //PADS
    SDL_SetRenderDrawColor(renderer, 100, 255, 255, 255);
    //SDL_RenderFillRect(renderer, &knee);
    SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
    /*SDL_RenderFillRect(renderer, &spawn[0].dest);
    SDL_RenderFillRect(renderer, &spawn[1].dest);
    SDL_RenderFillRect(renderer, &spawn[2].dest);
    SDL_RenderFillRect(renderer, &spawn[3].dest);*/

    //draw(spawn[1]);
    //draw(spawn[2]);
    //draw(spawn[3]);

    //SDL_SetRenderDrawColor(renderer, 30, 255, 30, 255);
    //SDL_RenderFillRect(renderer, &woods);
    //SDL_RenderFillRect(renderer, &stone_pile);
    //SDL_RenderFillRect(renderer, &furnace);


    //STATUE
    //SDL_SetRenderDrawColor(renderer, 100, 30, 255, 255);
    //SDL_RenderFillRect(renderer, &podium);
    //SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
    //SDL_RenderFillRect(renderer, &col);

    //ITEMS & ENEMIES
    for(int i=0; i<enemies.size(); i++) {
        /*SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderFillRect(renderer, &enemies[i].o.dest);
        SDL_SetRenderDrawColor(renderer, 255, 10, 10, 255);
        SDL_RenderDrawRect(renderer, &enemies[i].o.dest);*/
        draw(enemies[i].o);
    }

    for(int i=0; i<4; i++) {
        if(barrier[i].filled>0) {
            spawn[i].alpha=(barrier[i].filled*255)/barrier[i].total;
            //if(barrier[i].total<=WOOD_TOTAL) spawn[i].src.x=0;
            //if(barrier[i].total>WOOD_TOTAL) spawn[i].src.x=36/2;
            if(barrier[i].id==1)spawn[i].src.x=0;
            if(barrier[i].id==2)spawn[i].src.x=18;
            draw(spawn[i]);
        }
    }

    //SDL_SetRenderDrawColor(renderer, 30, 255, 100, 255);
    for(int i=0; i<arrows.size(); i++) {
        //SDL_RenderFillRect(renderer, &arrows[i].rect);
        arrows[i].o.dest.x=arrows[i].rect.x;
        arrows[i].o.dest.y=arrows[i].rect.y;
        draw(arrows[i].o);
    }

    draw(girl);

    //PLAYER
    //SDL_RenderFillRect(renderer, &bow);
    draw(bow); if(!praying && !repairing && quiver>0) {draw(tmpArrow.o);}
    //SDL_SetRenderDrawColor(renderer, 255, 100, 30, 255);
    //SDL_RenderFillRect(renderer, &player.dest);
    draw(player);
    //SDL_SetRenderDrawColor(renderer, 30, 255, 100, 255);
    if(bow.dest.x>player.dest.x) {draw(bow);if(!praying && !repairing && quiver>0){draw(tmpArrow.o);}}//SDL_RenderFillRect(renderer, &bow);
    //SDL_RenderFillRect(renderer, &statue);

    if(litFurnace)draw(fireF);
    if(!litFurnace)draw(furnace);

    //OVERLAY
    statImg.img=statI;
    statImg.alpha=255;
    draw(statImg);
    //if(praying) {
        statImg.img=statPray;
        statImg.alpha=map.alpha;
        draw(statImg);
    //}
    draw(mapwall);

    //HUD
    draw(quiverImg);
    if(carryingWood) {
        holding.src.x=0;
        draw(holding);
    }
    if(carryingStone) {
        holding.src.x=42;
        draw(holding);
    }
    if(carryingSteel) {
        holding.src.x=84;
        draw(holding);
    }

    if(prayer.active || SDL_HasIntersection(&player.dest, &knee))drawStatus(prayer);
    if(barrier[0].filled>0 || SDL_HasIntersection(&player.dest, &spawn[0].dest))drawStatus(barrier[0]);
    if(barrier[1].filled>0 || SDL_HasIntersection(&player.dest, &spawn[1].dest))drawStatus(barrier[1]);
    if(barrier[2].filled>0 || SDL_HasIntersection(&player.dest, &spawn[2].dest))drawStatus(barrier[2]);
    if(barrier[3].filled>0 || SDL_HasIntersection(&player.dest, &spawn[3].dest))drawStatus(barrier[3]);
    drawStatus(health);
    if(stone_status.filled>0 || SDL_HasIntersection(&player.dest, &stone_pile))drawStatus(stone_status);
    if(wood_status.filled>0 || SDL_HasIntersection(&player.dest, &woods))drawStatus(wood_status);
    if(steel_status.filled>0 || SDL_HasIntersection(&player.dest, &fireF.dest))drawStatus(steel_status);

    if(SDL_HasIntersection(&player.dest, &knee) && (player.dest.y+player.dest.h)<knee.y+knee.h && player.dest.x>knee.x && player.dest.x+player.dest.w<knee.x+knee.w && prayer.filled<prayer.total) {
        std::string msg = "press q to pray";
        if(praying) msg = "press q to stop praying";
        write(msg, player.dest.x+10+((FONT_SIZE*msg.size())/2), player.dest.y+1, 255, 255, 255);
    }
    for(int i=0; i<4; i++) {
        if(barrier[i].filled<barrier[i].total && SDL_HasIntersection(&player.dest, &spawn[i].dest)) {
            std::string msg = "press q to repair";
            if(repairing) msg = "press q to stop repairing";
            if(!carryingSteel && !carryingWood) msg = "collect material to repair";
            if(carryingStone) msg = "forge in furnace";
            if(player.dest.x>WIDTH/2) {
                write(msg, player.dest.x-10, player.dest.y+1, 255, 255, 255);
            } else {
                write(msg, player.dest.x+10+((FONT_SIZE*msg.size())/2), player.dest.y+10,255, 255, 255);
            }
        }
    }

    if(SDL_HasIntersection(&fireF.dest, &player.dest)) {
        std::string msg;

        if(steel_status.full) msg = "press e to collect steel";

        if(!litFurnace) msg = "needs wood";
        if(litFurnace && !stoneInFurnace) msg = "needs stone";

        if(carryingWood && !litFurnace) msg = "press e to light";

        if(carryingStone && !stoneInFurnace) msg = "press e to place stone";

        if(litFurnace && stoneInFurnace && !steel_status.full) msg = "wait for steel";

        if(player.dest.x>WIDTH/2) {

            write(msg, player.dest.x-10, player.dest.y+1, 255, 255, 255);
        } else {

            write(msg, player.dest.x+10+((FONT_SIZE*msg.size())/2), player.dest.y+1, 255, 255, 255);
        }

    }

    if( SDL_HasIntersection(&stone_pile, &player.dest) && !carryingWood && !carryingSteel && !carryingStone) {
        std::string msg = "press e to collect stone";
        if(!stone_status.full) msg = "wait for stone";
        if(player.dest.x>WIDTH/2) {
            write(msg, player.dest.x-10, player.dest.y+1, 255, 255, 255);
        } else {
            write(msg, player.dest.x+10+((FONT_SIZE*msg.size())/2), player.dest.y+1, 255, 255, 255);
        }
    }
    if(SDL_HasIntersection(&woods, &player.dest) && !carryingWood && !carryingSteel && !carryingStone) {
        std::string msg = "press e to collect wood";
        if(!wood_status.full) msg = "wait for wood";
        if(player.dest.x>WIDTH/2) {
            write(msg, player.dest.x-10, player.dest.y+1, 255, 255, 255);
        } else {
            write(msg, player.dest.x+10+((FONT_SIZE*msg.size())/2), player.dest.y+1, 255, 255, 255);
        }
    }
    writeHUD(std::to_string(quiver), quiverImg.dest.x, quiverImg.dest.y+(quiverImg.dest.h/2), 255, 255, 255);
    if(enemies.size()>0) {
        writeHUD("Wave: " + std::to_string(wave), quiverImg.dest.x-1, health.bar.y+health.bar.h-4, 255, 255, 255);
    } else if(wave!=0) {
        writeHUD(std::to_string(((lastWaveEnd+(WAVE_REST*1000))-SDL_GetTicks())/1000), 9, health.bar.y+health.bar.h-4, 255, 255, 255);
    }
    //if(enemyCount<=0) writeHUD("Time To Next Wave " + std::to_string(((lastWaveEnd+(WAVE_REST*1000))-SDL_GetTicks())/1000), WIDTH/2-90, HEIGHT*.7, 255, 255, 255);
    //write(std::to_string(mouseX) + ", " + std::to_string(mouseY), mouseX, mouseY, 255, 255, 255);


   
    draw(cursor);

    //MENU
    if(dead) {
        SDL_Rect bkg; bkg.x=bkg.y=0;bkg.w=WIDTH;bkg.h=HEIGHT;
        SDL_SetRenderDrawColor(renderer, 32, 15, 15, 200);
        SDL_RenderFillRect(renderer, &bkg);
        write("You died.", WIDTH/2+30, 500, 255, 30, 30);
        write("made by avery reed - Youtube: Avery Makes Games", WIDTH/2+180, HEIGHT-10, 255, 255, 255);
    }
    if(win) {
        SDL_Rect bkg; bkg.x=bkg.y=0;bkg.w=WIDTH;bkg.h=HEIGHT;
        SDL_SetRenderDrawColor(renderer, 250, 250, 255, 200);
        SDL_RenderFillRect(renderer, &bkg);
        write("she has woken.", WIDTH/2+30, 250, 30, 50, 255);
        write("made by avery reed - Youtube: Avery Makes Games", WIDTH/2+180, HEIGHT-10, 255, 255, 255);
    }

    //SDL_RenderDrawLine(renderer, 0, (-12*0) + 1820, 900, (-12*900) + 1820);
    //SDL_RenderDrawLine(renderer, 1440, (11.25*1440) - 16300, 1520, (11.25*1520) - 16300);
    ///STUFF

    SDL_RenderPresent(renderer);

}

void introScene() {
    SDL_Rect bkg; bkg.x=bkg.y=0;bkg.w=WIDTH;bkg.h=HEIGHT;
    for(int i=0; i<255; i+=22) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255-i);
    SDL_RenderFillRect(renderer, &bkg);
    write("take her to the north and pray for her", WIDTH/2+160, HEIGHT-250, 200, 250, 200);

    SDL_RenderPresent(renderer);
    SDL_Delay(120);
    }
}

void init() {
    lastWaveEnd=SDL_GetTicks();
    dead=win=0;
    player.dest.w=44; player.dest.h=70;
    player.dest.x=WIDTH/2-(player.dest.w/2); player.dest.y=HEIGHT*.65-player.dest.h*2;
    player.src.x=player.src.y=0;
    player.src.w=15;player.src.h=30;
    player.img=setImage("res/player.png");
    podium.w=866-731; podium.h=405-339;
    podium.x=731;//WIDTH/2-(podium.w/2); 
    podium.y=339;//HEIGHT*.3;
    girl.dest.w=player.dest.h+8; girl.dest.h=player.dest.w+8;
    girl.dest.x=WIDTH/2 - girl.dest.w/2 - 4; girl.dest.y=338;
    girl.img=setImage("res/girl.png");
    girl.src.x=girl.src.y=0;
    girl.src.w=30; girl.src.h=15;
    //statue.h=90; statue.w=podium.w*.8;
    statue.x=775;//WIDTH/2-(statue.w/2);
    statue.y=299;//podium.y-statue.h;
    statue.w=820-775;
    statue.h=339-299;
    col.x=podium.x; col.y=statue.y+(statue.h*.7);
    col.w=podium.w; col.h=podium.h*.8;
    col=podium;
    col.h=podium.h*.3;
    knee.w=824-771; knee.h=482-426;
    knee.x=771;//WIDTH/2-(knee.w/2);
    knee.y=426;//podium.y+podium.h+(player.dest.h*.3);
    prayer.bar.x=WIDTH/2-35;prayer.bar.y=205;
    prayer.bar.w=60; prayer.bar.h=12;
    prayer.filled=0;
    prayer.total=350;
    prayer.c.b=255; prayer.c.g=prayer.c.r=10;
    updateStatus(&prayer);
    bow.dest.w=player.dest.w; bow.dest.h=player.dest.h/2;
    bow.center={bow.dest.w/2, bow.dest.h/2};
    bow.img=setImage("res/bow.png");
    bow.src.x=bow.src.y=0;
    bow.src.w=bow.src.h=48;
    quiver=13;//13
    quiverImg.src.x=quiverImg.src.y=0;
    quiverImg.src.w=32; quiverImg.src.h=48;
    quiverImg.dest.w=32; quiverImg.dest.h=48;
    quiverImg.dest.x=110;    quiverImg.dest.y=HEIGHT-22-quiverImg.dest.h;
    quiverImg.alpha=180;
    quiverImg.img=setImage("res/quiver.png");
    tmpArrow.o.img=setImage("res/arrow.png");
    reload=0; reloadTime=16;//23
    cursor.dest.w=32;cursor.dest.h=20;
    cursor.src.w=32;cursor.src.h=20;
    cursor.src.y=0; cursor.src.x=0;
    cursor.img=setImage("res/cursor.png");
    cursor.center={cursor.dest.w/2, cursor.dest.h/2};
    spawn[0].img=setImage("res/barriers.png");
    spawn[0].src.x=spawn[0].src.y=0;
    spawn[0].src.w=36/2;spawn[0].src.h=42;
    spawn[0].dest.w=85;spawn[0].dest.h=179;
    spawn[0].dest.x=72; spawn[0].dest.y=138;
    spawn[1]=spawn[0];
    spawn[1].dest.x=38;spawn[1].dest.y=562;
    spawn[2]=spawn[0];
    spawn[2].flip=SDL_FLIP_HORIZONTAL;
    spawn[2].dest.x=WIDTH-spawn[2].dest.w-spawn[0].dest.x;
    spawn[3]=spawn[1];
    spawn[3].flip=SDL_FLIP_HORIZONTAL;
    spawn[3].dest.x=WIDTH-spawn[1].dest.x-spawn[3].dest.w;
    barrier[0].bar.x=spawn[0].dest.x+2;
    barrier[0].bar.y=spawn[0].dest.y;
    barrier[0].bar.w=40;barrier[0].bar.h=12;
    barrier[0].total=200;
    barrier[0].filled=40;
    barrier[0].c.r=barrier[0].c.b=30; barrier[0].c.g=200;
    barrier[0].id=1;
    barrier[1]=barrier[2]=barrier[3]=barrier[0];
    barrier[1].bar.x=spawn[1].dest.x+2;barrier[1].bar.y=spawn[1].dest.y;
    barrier[2].bar.x=spawn[2].dest.x+2;barrier[2].bar.y=spawn[2].dest.y;
    barrier[3].bar.x=spawn[3].dest.x+2;barrier[3].bar.y=spawn[3].dest.y;
    tmpEnemy.o.dest.w=58;tmpEnemy.o.dest.h=73;
    tmpEnemy.o.img=setImage("res/enemy.png");
    tmpEnemy.o.src.x=tmpEnemy.o.src.y=0;
    tmpEnemy.o.src.w=58;tmpEnemy.o.src.h=73;
    //enemyCount=0;
    health.bar=prayer.bar;
    health.bar.x=quiverImg.dest.x-4;
    health.bar.y=quiverImg.dest.y-8-health.bar.h;
    health.total=2000;
    health.filled=2000;
    health.c.b=health.c.g=30; health.c.r=255;
    updateStatus(&health);
    woods.w=woods.h=60;
    woods.x=360; woods.y=110;
    stone_pile=woods; stone_pile.x=954;
    fireF.dest=woods;
    fireF.dest.w=125;
    fireF.dest.h=75;
    fireF.dest.x=(WIDTH/2)-(fireF.dest.w/2);
    fireF.dest.y=HEIGHT-fireF.dest.h;
    fireF.img=setImage("res/furnace-f.png");
    fireF.src.x=fireF.src.y=0;
    fireF.src.w=43; fireF.src.h=22;
    furnace=fireF;
    furnace.src.w=44;furnace.src.h=14;
    furnace.dest.h=52;
    furnace.dest.y=HEIGHT-furnace.dest.h;
    furnace.img=setImage("res/furnace.png");
    wood_status.bar=health.bar;
    wood_status.bar.w=40;
    wood_status.bar.x=woods.x+5;
    wood_status.bar.y=woods.y+woods.h;
    wood_status.total=500;
    wood_status.filled=0;
    wood_status.c.r=101; wood_status.c.g=67; wood_status.c.b=33;
    stone_status=wood_status;
    stone_status.bar.x=stone_pile.x+5;
    stone_status.bar.y-=40;
    stone_status.c.r=76; stone_status.c.g=76; stone_status.c.b=76;
    stone_status.total=1000;
    steel_status=wood_status;
    steel_status.bar.y=furnace.dest.y-20;
    steel_status.bar.x=furnace.dest.x+5;
    steel_status.c.r=226; steel_status.c.g=88; steel_status.c.b=34;
    map.img=setImage("res/map.png");
    map.dest.x=0;map.dest.y=0;
    map.dest.w=WIDTH; map.dest.h=HEIGHT;
    map.src.x=map.src.y=0;
    map.src.w=347; map.src.h=195;
    mapwall=map;
    mapwall.img=setImage("res/mapwall.png");
    statImg=map;
    statImg.dest.y+=2;
    statImg.img=setImage("res/statue.png");
    mapI=map.img;
    statI=statImg.img;
    mapPray=setImage("res/map-p.png");
    statPray=setImage("res/statue-p.png");
    holding.img=setImage("res/items.png");
    holding.src.w=42; holding.src.h=28;
    holding.src.y=0;
    holding.alpha=180;
    holding.dest.w=64; holding.dest.h=36;
    holding.dest.x=quiverImg.dest.x+quiverImg.dest.w+15;
    holding.dest.y=HEIGHT-22-holding.dest.h;
}

int main() {
    srand(time(NULL));
    running=1;
    static int lastTime=0;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) std::cout << "Failed at SDL_Init()" << std::endl;
    if(SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_FULLSCREEN, &window, &renderer) < 0) std::cout << "Failed at SDL_CreateWindowAndRenderer()" << std::endl;
    SDL_SetWindowTitle(window, "LD 46");
    SDL_ShowCursor(0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    TTF_Init();
    font = TTF_OpenFont("res/Peepo.ttf", FONT_SIZE);
    if(font == NULL) std::cout << "f" << std::endl;

    //Mix_OpenAudio(44100, //Mix_DEFAULT_FORMAT, 2, 2048);
    //hit = //Mix_LoadWAV("res/damage.wav");
    //walk = //Mix_LoadWAV("res/walk.wav");
    //wisp = //Mix_LoadWAV("res/arrow.wav");

 
    init();

    while(running) {
        lastFrame=SDL_GetTicks();
        if(lastFrame>=(lastTime+1000)) {
            lastTime=lastFrame;
            fps=frameCount;
            frameCount=0;
        }
        if(intro) introScene();
        intro=0;
                render();

        //spawn[3].x=spawn[2].x;
        //barrier[3].bar.x=spawn[3].x+2;barrier[3].bar.y=spawn[3].y;
        if(!dead && !win)update();
        cursor.dest.x=mouseX-(cursor.dest.w/2);
        cursor.dest.y=mouseY-(cursor.dest.h/2);
        cursor.src.x=0;
        if(quiver<=0) cursor.src.x=32;
        cursor.angle=rad*(180/PI)+90;//++;//rad;//(180*PI);
        input();
    //carryingSteel=1;//KKKKKK

    }
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //Mix_FreeChunk(hit);
    //Mix_FreeChunk(wisp);
    //Mix_FreeChunk(walk);
    //Mix_CloseAudio();
    SDL_Quit();
}
