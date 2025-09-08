/* 
Future dev: 
    -Needs a multiple thread + delta time.
    -Fix player movement, to slow. (Prob cuz no delta time)
    -Re-use some of the 2048 game components.
Compile with: gcc spacewars.c -o spacewars

*/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

#define WIDTH 30
#define HEIGHT 20
#define MAX_BULLETS 5
#define MAX_ENEMIES 5


typedef struct {
    int x, y;
    int active;
} Enemy;
typedef struct {
    int x, y;
    int active;
} Bullet;
typedef struct {
    int x,y;
} Player;

int playerX;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
int score = 0;
int gameOver = 0;

void hideCursor() {
    CONSOLE_CURSOR_INFO ci = {100, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}
void draw();
void updateBullets();
void updateEnemies();
void checkCollisions();
void shoot();
void handleInput();

int main() {
    srand(time(0));
    hideCursor();
    playerX = WIDTH / 2;
    while (!gameOver) {
        draw();
        handleInput();
        updateBullets();
        updateEnemies();
        checkCollisions();
        Sleep(200); 
    }

    system("cls");
    printf("\nGame Over!\nFinal Score: %d\n", score);
    return 0;
}
void updateBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y--;
            if (bullets[i].y < 0) {
                bullets[i].active = 0;
            }
        }
    }
}

void updateEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].y++;
            if (enemies[i].y >= HEIGHT) {
                enemies[i].active = 0;
            }
            if (enemies[i].y == HEIGHT - 1 && enemies[i].x == playerX) {
                gameOver = 1;
            }
        } else {
            if (rand() % 20 == 0) {
                enemies[i].x = rand() % WIDTH;
                enemies[i].y = 0;
                enemies[i].active = 1;
            }
        }
    }
}

void checkCollisions() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;
            if (bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y) {
                bullets[i].active = 0;
                enemies[j].active = 0;
                score++;
            }
        }
    }
}



void shoot() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = playerX;
            bullets[i].y = HEIGHT - 2;
            bullets[i].active = 1;
            break;
        }
    }
}

void handleInput() {
    if (_kbhit()) {
        char key = _getch();
        if (key == 'a' && playerX > 0) playerX--;
        if (key == 'd' && playerX < WIDTH - 1) playerX++;
        if (key == ' ') shoot();
        if (key == 27) gameOver = 1;
    }
}

void draw() {
    system("cls"); 
    for (int i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\n");

    for (int y = 0; y < HEIGHT; y++) {
        printf("#");
        for (int x = 0; x < WIDTH; x++) {
            int drawn = 0;
            if (x == playerX && y == HEIGHT - 1) {
                printf("/\\");
                drawn = 1;
            }

            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i].active && bullets[i].x == x && bullets[i].y == y) {
                    printf("|");
                    drawn = 1;
                }
            }
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active && enemies[i].x == x && enemies[i].y == y) {
                    printf("X");
                    drawn = 1;
                }
            }

            if (!drawn) printf(" ");
        }
        printf("#\n");
    }
    for (int i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\nScore: %d\n", score);
}