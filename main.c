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
    int y;
    int x;
}coord;

typedef struct color{
    int r;
    int g;
    int b;
}color;

typedef struct rayreturn{
    coord coord;
    double direction;
    color color;
}rayreturn;

char obj[Y][X] = {};
int drawarray[Y][X][3] = {};
coord mousepos = {};
int lastmaxbrightness = 1;

color converts[256] = {(color){0,0,0},(color){100,100,100},(color){300,0,0},(color){0,300,0},(color){0,0,300}};

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

void drawcircle(coord pos, int radius, char filltype, char array[Y][X]){
    for(int y = pos.y-radius; y < pos.y+radius; y++){
        for(int x = pos.x-radius; x < pos.x+radius; x++){
            if (sqrt((pos.x-x)*(pos.x-x) + (pos.y-y)*(pos.y-y))<radius){
                array[y][x] = filltype;
            }
        }
    }
}

double checksurround(coord pos,double dir){
    int rdir = (int)dir+180;
    if (dir<0){
        dir+=360;
    }
    dir = fmod(dir,360);
    if(dir >= 0 && dir < 90){
        if(obj[pos.y-1][pos.x] && obj[pos.y][pos.x-1]) rdir = 225; 
        else if(obj[pos.y-1][pos.x]) rdir = 270;
        else if(obj[pos.y][pos.x-1]) rdir = 180;
        else if(obj[pos.y+1][pos.x]) rdir = 270;
        else if(obj[pos.y][pos.x+1]) rdir = 180;
        
    }
    else if(dir > 90 && dir <= 180){
        if(obj[pos.y+1][pos.x] && obj[pos.y][pos.x-1]) rdir = 315; 
        else if(obj[pos.y+1][pos.x]) rdir = 270;
        else if(obj[pos.y][pos.x-1]) rdir = 0;
        else if(obj[pos.y-1][pos.x]) rdir = 270;
        else if(obj[pos.y][pos.x+1]) rdir = 0;
    }
    else if(dir > 180 && dir <= 270){
        if(obj[pos.y+1][pos.x] && obj[pos.y][pos.x+1]) rdir = 45; 
        else if(obj[pos.y+1][pos.x]) rdir = 90;
        else if(obj[pos.y][pos.x+1]) rdir = 0;
        else if(obj[pos.y-1][pos.x]) rdir = 90;
        else if(obj[pos.y][pos.x-1]) rdir = 0;
    }
    else if(dir > 270 && dir <= 360){
        if(obj[pos.y-1][pos.x] && obj[pos.y][pos.x+1]) rdir = 135; 
        if(obj[pos.y-1][pos.x]) rdir = 90;
        else if(obj[pos.y][pos.x+1]) rdir = 180;
        else if(obj[pos.y+1][pos.x]) rdir = 90;
        else if(obj[pos.y][pos.x-1]) rdir = 180;
    }
    else{
        //printf("%f\n",dir);
    }

    

    if(rdir == 0 || rdir == 180){
        dir = -dir;
    }
    if(rdir == 90 || rdir == 270){
        dir += 90;
        dir = -dir;
        dir -= 90;
    }
    
    if(dir<0) dir+=360;
    dir = fmod(dir,360);

    int randomness = 180 - fabs(fmod(fabs(dir - rdir), 2*180) - 180);
    if(randomness > 180){
        printf("%f,%d\n",dir,rdir);
    }
    
    if(randomness < 2){
        randomness = 2;
    }
    return dir+180+((double)(random()%(randomness*5)))/10-((double)randomness)/4;
}

rayreturn computeray(coord start, double dir, char array[Y][X], color lightcolor){
    dir = degToRad(dir);
    double offset[2] = {sin(dir), cos(dir)};

    double pos[2] = {start.y,start.x};

    while((int)pos[0]>=0 && (int)pos[0]<X && (int)pos[1]>=0 && (int)pos[1]<Y){
        if(array[(int)pos[1]][(int)pos[0]]){
            dir = radToDeg(dir);
            dir = checksurround((coord){(int)pos[1],(int)pos[0]},dir);

            color rcolor = lightcolor;

            rcolor.r += converts[array[(int)pos[1]][(int)pos[0]]].r;
            rcolor.g += converts[array[(int)pos[1]][(int)pos[0]]].g;
            rcolor.b += converts[array[(int)pos[1]][(int)pos[0]]].b;
            rcolor.r /= 2;
            rcolor.g /= 2;
            rcolor.b /= 2;

            return (rayreturn){(coord){(int)(pos[0]-offset[0]),(int)(pos[1]-offset[1])},dir,rcolor};
        }
        pos[0]+=offset[0];
        pos[1]+=offset[1];

        drawarray[(int)pos[0]][(int)pos[1]][0]+=lightcolor.r;
        drawarray[(int)pos[0]][(int)pos[1]][1]+=lightcolor.g;
        drawarray[(int)pos[0]][(int)pos[1]][2]+=lightcolor.b;
    }

    return (rayreturn){(coord){-1,-1},-1.0};
}

void on_mouse(S2D_Event e) {
    switch (e.type) {
        case S2D_MOUSE_MOVE:

            if(e.x > 0 && e.x < X) mousepos.x = e.x;
            if(e.y > 0 && e.y < Y) mousepos.y = e.y;
        break;
    }
}

void update(){
    memset(obj,0,X*Y*sizeof(char));
    drawrect((coord){0,0},(coord){2,512},1,obj); //draws 1 pixel line on left border
    drawrect((coord){0,0},(coord){512,2},1,obj); //draws 1 pixel line on top border
    drawrect((coord){0,510},(coord){512,512},1,obj); //draws 1 pixel line on bottom border
    drawrect((coord){510,0},(coord){512,512},1,obj); //draws 1 pixel line on right border
    drawrect((coord){320,320},(coord){321,340},1,obj);
    drawrect((coord){100,320},(coord){150,370},2,obj);

    drawrect((coord){100,400},(coord){400,420},1,obj);

    drawrect((coord){100,100},(coord){150,102},3,obj);
    drawrect((coord){100,100},(coord){102,150},3,obj);
    drawrect((coord){150,100},(coord){152,150},3,obj);
    drawrect((coord){100,150},(coord){130,152},3,obj);

    drawcircle((coord){256,256},50,1,obj);
    drawcircle((coord){256,256},40,0,obj);
    drawrect((coord){256,206},(coord){306,306},0,obj);
}

void render(){

    memset(drawarray,0,X*Y*sizeof(int)*3);
    double frameclock = omp_get_wtime();
    int raydegc = 100;
    #pragma omp parallel for reduction (+:drawarray)
    for(int i = 0; i < (360*raydegc); i+=1){
        coord pastbounce = (coord){mousepos.y,mousepos.x};
        double direction = (double)i/raydegc+frameclock*50;
        color pastcolor = (color){100,100,100};
        for(int b = 0; b< 5; b++){
            rayreturn computerayreturn = computeray(pastbounce,direction,obj,pastcolor);
            pastbounce = computerayreturn.coord;
            direction = computerayreturn.direction;
            pastcolor = computerayreturn.color;
            if(pastbounce.x == -1 && pastbounce.y == -1){
                break;
            }
        }
    }

    int newmaxbrightness = 0;
    for(int y = 0; y<Y; y++){
        for(int x = 0; x<X; x++){
            int brightness = (drawarray[y][x][0] + drawarray[y][x][1] + drawarray[y][x][2]) / 3;
            if(brightness > newmaxbrightness) newmaxbrightness = brightness;
            double red = (double)(drawarray[y][x][0])/lastmaxbrightness;
            double green = (double)(drawarray[y][x][1])/lastmaxbrightness;
            double blue = (double)(drawarray[y][x][2])/lastmaxbrightness;

            S2D_DrawQuad(x,y,red,green,blue,1,x+1,y,
            red,green,blue,1,x+1,y+1,
            red,green,blue,1,x,y+1,
            red,green,blue,1);
        }
    }
    lastmaxbrightness = newmaxbrightness/10;
}

int main(){
    
    S2D_Window *window = S2D_CreateWindow(
        "Raytracing", 512, 512, update, render, 0
    );
    
    S2D_HideCursor();
    window->on_mouse = on_mouse;
    S2D_Show(window);
    return 0;
}