#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define MAX_WIDTH 20
#define MAX_HEIGHT 10
#define PADDLE_MAX_HEIGHT 3

int ball_x = MAX_WIDTH / 2;
int ball_y = MAX_HEIGHT / 2;
int dir_x = 1, dir_y = 1;
int paddle_y = MAX_HEIGHT / 2;

void draw(void);
void update(void);
void move(void);

int main()
{

    printf("Hello, PingPong!\n");
    draw();
    while (1)
    {
        update();
        move();
        draw();
    }
    return 0;
}

void draw()
{
    system("cls");
    for (int y = 0; y < MAX_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_WIDTH; x++)
        {
            if (x == 0 && y >= paddle_y && y < paddle_y + PADDLE_MAX_HEIGHT)
                printf("|"); // Paddle
            else if (x == ball_x && y == ball_y)
                printf("O"); // Ball
            else
                printf(" ");
        }
        printf("\n");
    }
}
void move()
{
    if (_kbhit())
    {
        char c = _getch();
        if (c == 'w' && paddle_y < MAX_HEIGHT)
            paddle_y--;
        if (c == 's' && paddle_y > 0)
            paddle_y++;
    }
}

void update()
{
    ball_x += dir_x;
    ball_y += dir_y;

    if (ball_y <= 0 || ball_y >= MAX_HEIGHT - 1)
        dir_y *= -1;

    if (ball_x == 1 && ball_y >= paddle_y && ball_y < paddle_y + PADDLE_MAX_HEIGHT)
        dir_x *= -1;

    if (ball_x <= 0)
    {
        ball_x = MAX_WIDTH / 2;
        ball_y = MAX_HEIGHT / 2;
        dir_x = 1;
        dir_y = 1;
    }
    if (ball_x >= MAX_WIDTH - 1)
        dir_x *= -1;
}
