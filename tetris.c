#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define PF_W 10
#define PF_H 24

#define TileSize 16

#define SCALE 2

#define SCREEN_W (PF_W * TileSize)
#define SCREEN_H (PF_H * TileSize)

SDL_Window* win = NULL;
SDL_Renderer* render = NULL;
SDL_Texture* tex = NULL;
int playing = 0;

int playfield[PF_H][PF_W];

typedef struct {
    int x;
    int y;
    int data[4][4];
    int placed;

} Piece;

Piece piece;
Piece pieceBack;

int speed;

char* PieceL = "\
    \
 #  \
 #  \
 ## ";
	   
char* PieceO = "\
    \
 ## \
 ## \
    ";
char* PieceS = "\
    \
 ## \
##  \
    ";

char* PieceZ = "\
    \
 ## \
  ##\
    ";
char* PieceI = "\
 #  \
 #  \
 #  \
 #  ";

char* PieceT = "\
    \
### \
 #  \
    ";

char* pieces[6];

Uint32 delta;

void loadPiece(char* p) {
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
            if (p[y * 4 + x] == '#') piece.data[y][x] = 1;
	    else piece.data[y][x] = 0;
	}
    }
    piece.x = (PF_W / 2) - 2;
    piece.y = -3;
    piece.placed = 1;
}
void drawPiece() {
	if (!piece.placed) return;
        SDL_Rect cell, frame;
	cell.w = cell.h = frame.w = frame.h = TileSize;
	frame.y = 0;
	for(int sy = 0; sy < 4; sy++) {
            cell.y = (piece.y + sy) * TileSize;
            for(int sx = 0; sx < 4; sx++) {
                cell.x = (piece.x + sx) * TileSize;
		frame.x = piece.data[sy][sx] * TileSize;
		if (frame.x != 0)
		    SDL_RenderCopy(render, tex, &frame, &cell);
	    }
	}
}

int checkColision() {
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
	    int pfX = x + piece.x;
	    int pfY = y + piece.y;
	    if (piece.data[y][x]) {
		if (pfY < 0) continue;
                if(playfield[pfY][pfX] || pfY >= PF_H) return 1;
	    }
	}
    }
    return 0;
}

int outLeftBound() {
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
	    int pfX = x + piece.x;
	    if (piece.data[y][x]) {
                if(pfX < 0) return 1;
	    }
	}
    }
    return 0;
}

int outRightBound() {
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
	    int pfX = x + piece.x;
	    if (piece.data[y][x]) {
                if(pfX > PF_W) return 1;
	    }
	}
    }
    return 0;
}

void deleteRow(int r) {
    for(int x = 0; x < PF_W; x++) {
	playfield[r][x] = 0;
        for(int y = r; y > 0; y--) {
            playfield[y][x] = playfield[y-1][x];
	}
    }
}

void blitPiece() {
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
            playfield[y + piece.y][x + piece.x] += piece.data[y][x];
	}
    }
    piece.placed = 0;
    int y = PF_H - 1;
    while(y >= 0) {
	int x;
        for(x = 0; x < PF_W && playfield[y][x]; x++);
	if (x == PF_W) deleteRow(y);
	else y--;
    }
}
void clearPlayfield() {
    for(int y = 0; y < PF_H; y++)
        for(int x = 0; x < PF_W; x++) playfield[y][x] = 0;
}

void drawPlayfield() {
    SDL_Rect cell;
    SDL_Rect frame;
    frame.x = frame.y = 0;
    frame.w = frame.h = TileSize;
    cell.w = cell.h = TileSize;
    cell.y = 0;

    for(int y = 0; y < PF_H; y++) {
        cell.x = 0;
	for(int x = 0; x < PF_W; x++) {
	    int tile = playfield[y][x];
	    frame.x = (tile % 4) * TileSize;
	    SDL_RenderCopy(render, tex, &frame, &cell);
	    cell.x += TileSize;
	}
        cell.y += TileSize;
    }
}

void quit() {
    if (render) SDL_DestroyRenderer(render);
    if (win) SDL_DestroyWindow(win);
    exit(0);
}

void moveLeft() {
    if(!piece.placed) return;
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
            if(piece.data[y][x] && x + piece.x <= 0) return;
	}
    }

    piece.x--;
    if (checkColision()) {
        piece.x++;
    }
}

void moveRight() {
    if(!piece.placed) return;
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
            if(piece.data[y][x] && x + piece.x >= PF_W - 1) return;
	}
    }

    piece.x++;
    if (checkColision()) {
        piece.x--;
    }
}

void savePieceData() {
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
            pieceBack.data[y][x] = piece.data[y][x]; 
	}
    }
}

void restorePieceData() {
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
            piece.data[y][x] = pieceBack.data[y][x]; 
	}
    }
}

void rotateLeft() {
    savePieceData();
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
	    piece.data[3 - x][y] = pieceBack.data[y][x];
	}
    }
    while(outLeftBound()) piece.x++;
    while(outRightBound()) piece.x--;
    if (checkColision()) restorePieceData();
}

void rotateRight() {
    savePieceData();
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
	    piece.data[ x][3 - y] = pieceBack.data[y][x];
	}
    }
    while(outLeftBound()) piece.x++;
    while(outRightBound()) piece.x--;
    if (checkColision()) restorePieceData();
}

void moveDown() {
    if (!piece.placed) return;
    piece.y++;
    if (checkColision()) {
        piece.y--;
        blitPiece();
    }
}


void loop() {
    SDL_Event ev;
    while(SDL_PollEvent(&ev)) {
        switch(ev.type) {
            case SDL_QUIT:
	       playing = 0;
	       return;
	    case SDL_KEYDOWN:
	       if (ev.key.keysym.scancode == SDL_SCANCODE_LEFT) moveLeft();
	       if (ev.key.keysym.scancode == SDL_SCANCODE_RIGHT) moveRight();
	       if (ev.key.keysym.scancode == SDL_SCANCODE_DOWN) moveDown();
	       if (ev.key.keysym.scancode == SDL_SCANCODE_UP) rotateLeft();
	       if (ev.key.keysym.scancode == SDL_SCANCODE_Q) rotateLeft();
	       if (ev.key.keysym.scancode == SDL_SCANCODE_E) rotateRight();
	}

    }
    if (SDL_GetTicks() - delta > 1000 / speed) {
        delta = SDL_GetTicks();

        if (!piece.placed) {
             piece.placed = 1;
	     loadPiece(pieces[rand() % 6]);
        }

	moveDown();
    }

    SDL_RenderClear(render);
    drawPlayfield();
    drawPiece();
    SDL_RenderPresent(render);
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    win = SDL_CreateWindow("Tetris",
        SDL_WINDOWPOS_CENTERED,
	SDL_WINDOWPOS_CENTERED,
	SCREEN_W * SCALE,
	SCREEN_H * SCALE,
	SDL_WINDOW_SHOWN);
    if (!win) {
        printf("Cannot create window\n");
	quit();
    }

    render = SDL_CreateRenderer(win, -1, 0);
    if (!render) {
        printf("Cannot create renderer\n");
	quit();
    }

    SDL_RenderSetLogicalSize(render, SCREEN_W, SCREEN_H);
    delta = SDL_GetTicks();

    SDL_Surface* sur = SDL_LoadBMP("graphics.bmp");
    if (!sur) {
        printf("Cannot load graphics from 'graphics.bmp'\n");
	quit();
    }
    tex = SDL_CreateTextureFromSurface(render, sur);
    pieces[0] = PieceL;
    pieces[1] = PieceO;
    pieces[2] = PieceS;
    pieces[3] = PieceZ;
    pieces[4] = PieceI;
    pieces[5] = PieceT;

    playing = 1;
    speed = 4;

    piece.placed = 0;
    clearPlayfield();
    while(playing) {
        loop();
    }


    quit();
}
