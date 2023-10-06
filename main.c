#include <stdio.h>
#include <simple2d.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define degToRad(angleInDegrees) ((angleInDegrees) * 3.14159 / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians) * 180.0 / 3.14159)

#define X 512
#define Y 512

typedef struct coord{
    int x;
    int y;
}coord;

typedef struct rayreturn{
    coord coord;
    double direction;
}rayreturn;

char obj[Y][X] = {};
int drawarray[Y][X] = {};

void swapints(int *a, int *b){
    int *c = a;
    *a = *b;
    *b = *c;
}

void drawrect(coord a, coord b, char filltype, char array[Y][X]){
    if(a.x < 0) a.x = 0;
    if(a.x >= X) a.x = X-1;
    if(a.y < 0) a.y = 0;
    if(a.y >= Y) a.y = Y-1;

    if(b.x < 0) b.x = 0;
    if(b.x >= X) b.x = X-1;
    if(b.y < 0) b.y = 0;
    if(b.y >= Y) b.y = Y-1;

    if(a.x > b.x) swapints(&a.x,&b.x);
    if(a.y > b.y) swapints(&a.y,&b.y);

    for(int y = a.y; y<b.y; y++){
        for(int x = a.x; x<b.x; x++){
            array[y][x] = filltype;
        }
    }
}

rayreturn computeray(coord start, double dir, char array[Y][X]){
    dir = degToRad(dir);
    double offset[2] = {sin(dir), cos(dir)};

    double pos[2] = {start.x,start.y};

    while((int)pos[0]>0 && (int)pos[0]<X && (int)pos[1]>0 && (int)pos[1]<Y){
        if(array[(int)pos[0]][(int)pos[1]]){
            return (rayreturn){(coord){(int)(pos[0]-offset[0]),(int)(pos[1]-offset[1])},radToDeg(dir)};
        }
        pos[0]+=offset[0];
        pos[1]+=offset[1];
        drawarray[(int)pos[0]][(int)pos[1]]++;
    }

    return (rayreturn){(coord){-1,-1},-1.0};
}

void update(){
    drawrect((coord){0,0},(coord){2,512},1,obj); //draws 1 pixel line on left border
    drawrect((coord){0,0},(coord){512,2},1,obj); //draws 1 pixel line on top border
    drawrect((coord){0,510},(coord){512,512},1,obj); //draws 1 pixel line on bottom border
    drawrect((coord){510,0},(coord){512,512},1,obj); //draws 1 pixel line on right border
    drawrect((coord){320,320},(coord){321,340},1,obj);
    drawrect((coord){100,320},(coord){150,370},1,obj);
}

void render(){
    memset(drawarray,0,X*Y*sizeof(int));
    double frameclock = omp_get_wtime();
    int raydegc = 100;
    #pragma omp parallel for
    for(int i = 0; i < (360*raydegc); i+=1){
        coord pastbounce = (coord){(int)(sin(frameclock)*128+256),256};
        double direction = (double)i/raydegc;
        for(int b = 0; b< 5; b++){
            rayreturn computerayreturn = computeray(pastbounce,direction,obj);
            pastbounce = computerayreturn.coord;
            direction = computerayreturn.direction + 180 +(double)(random()%900)/10-45;
            if(pastbounce.x == -1 && pastbounce.y == -1){
                break;
            }
        }
    }
    for(int y = 0; y<Y; y++){
        for(int x = 0; x<X; x++){
            double intensity = (double)(drawarray[x][y])/1000;
            S2D_DrawQuad(x,y,1,1,1,intensity,x+1,y,1,1,1,intensity,x+1,y+1,1,1,1,intensity,x,y+1,1,1,1,intensity);
        }
    }
}

int main(){
    
    S2D_Window *window = S2D_CreateWindow(
        "Raytracing", 512, 512, update, render, 0
    );

    S2D_Show(window);
    return 0;
}