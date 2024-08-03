#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "raylib.h"

#define CALC_WIN_SIZE(ATTR) (BOARD_##ATTR * CELL_SIZE + (BOARD_##ATTR - 1) * SPACING + 2 * PADDING)

#define FPS 60
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 10
#define MINE_COUNT 10
#define CELL_SIZE 30
#define SPACING 2
#define PADDING 10
#define MARGIN_TOP 40
#define WINDOW_WIDTH CALC_WIN_SIZE(WIDTH)
#define WINDOW_HEIGHT CALC_WIN_SIZE(HEIGHT) + MARGIN_TOP
#define FONT_SIZE 20

#define WIN_TEXT "You Win!"
#define LOSE_TEXT "Game Over!"
#define MINE_INFO "Mines: %d"
#define TIME_INFO "Time: %.1f"

#define TEXT_COLOR BLACK
#define BG_COLOR LIGHTGRAY
#define UNREVEALED_COLOR GRAY
#define REVEALED_COLOR WHITE
#define MINE_COLOR RED

#define CALC_POS_X(x) (PADDING + x * (CELL_SIZE + SPACING))
#define CALC_POS_Y(y) (PADDING + y * (CELL_SIZE + SPACING) + MARGIN_TOP)
#define MEASURE_TEXT(text) MeasureText(text, FONT_SIZE)
#define CALC_CENTER(whole, text) ((whole - MEASURE_TEXT(text)) / 2)
#define DRAW_CELL(posX, posY, color) DrawRectangle(posX, posY, CELL_SIZE, CELL_SIZE, color)
#define DRAW_MINE(posX, posY) DrawCircle(posX + CELL_SIZE * 0.5, posY + CELL_SIZE * 0.5, CELL_SIZE * 0.25, MINE_COLOR)
#define DRAW_TEXT(text, posX, posY) DrawText(text, posX, posY, FONT_SIZE, TEXT_COLOR)
#define FORMAT_TEXT(TEXT, ...) sprintf(buffer, TEXT, __VA_ARGS__)

typedef struct {
    bool isRevealed;
    bool isFlagged;
} Cell;

typedef struct {
    Cell board[BOARD_WIDTH][BOARD_HEIGHT];
    bool mines[BOARD_WIDTH][BOARD_HEIGHT];
    bool isGameOver;
    bool isGameWon;
    double time;
    bool started;
} Game;

void InitGame(Game *game);

void DrawGame(Game *game);

void HandleInput(Game *game);

int countSurroundingMines(Game *game, int x, int y);

void handleReveal(Game *game, int x, int y);

int main(void) {
    Game game;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "MineSweeper");
    SetTargetFPS(FPS);
    InitGame(&game);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BG_COLOR);
        DrawGame(&game);
        HandleInput(&game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void InitGame(Game *game) {
    game->time = 0;
    game->isGameWon = game->isGameOver = game->started = false;
    memset(game->board, 0, sizeof(game->board));
    memset(game->mines, 0, sizeof(game->mines));

    for (int i = 0; i < MINE_COUNT; i++) {
        int x, y;
        do {
            x = GetRandomValue(0, BOARD_WIDTH - 1);
            y = GetRandomValue(0, BOARD_HEIGHT - 1);
        } while (game->mines[x][y]);
        game->mines[x][y] = true;
    }
}

void DrawGame(Game *game) {
    int posX, posY, minesFlagged, surroundingMines;
    char buffer[20];

    if (game->isGameOver)
        DRAW_TEXT(LOSE_TEXT, CALC_CENTER(WINDOW_WIDTH, LOSE_TEXT), PADDING * 1.5);

    minesFlagged = 0;

    for (int x = 0; x < BOARD_WIDTH; x++)
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            posX = CALC_POS_X(x), posY = CALC_POS_Y(y);
            if (game->board[x][y].isRevealed) {
                DRAW_CELL(posX, posY, REVEALED_COLOR);
                if (!game->mines[x][y]) {
                    surroundingMines = countSurroundingMines(game, x, y);
                    if (surroundingMines > 0) {
                        FORMAT_TEXT("%d", surroundingMines);
                        DRAW_TEXT(buffer, posX + CALC_CENTER(CELL_SIZE, buffer), posY + CELL_SIZE / 2 - 10);
                    }
                } else
                    DRAW_MINE(posX, posY);
            } else {
                DRAW_CELL(posX, posY, UNREVEALED_COLOR);
                if (game->board[x][y].isFlagged) {
                    DRAW_MINE(posX, posY);
                    minesFlagged++;
                }
            }
        }

    FORMAT_TEXT(MINE_INFO, MINE_COUNT - minesFlagged);
    DRAW_TEXT(buffer, PADDING * 1.5, PADDING * 1.5);

    if (minesFlagged == MINE_COUNT) {
        game->isGameWon = true;
        DRAW_TEXT(WIN_TEXT, CALC_CENTER(WINDOW_WIDTH, WIN_TEXT), PADDING * 1.5);
    }

    if (game->started && !game->isGameOver && !game->isGameWon) game->time += GetFrameTime();

    if (game->started) {
        FORMAT_TEXT(TIME_INFO, game->time);
        DRAW_TEXT(buffer, WINDOW_WIDTH - MEASURE_TEXT(buffer) - PADDING * 1.5, PADDING * 1.5);
    }
}

void HandleInput(Game *game) {
    if (IsKeyPressed(KEY_R)) {
        InitGame(game);
        return;
    }

    if (game->isGameOver || game->isGameWon) return;

    int x = (GetMouseX() - PADDING) / (CELL_SIZE + SPACING);
    int y = (GetMouseY() - PADDING - MARGIN_TOP) / (CELL_SIZE + SPACING);

    if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) return;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (!game->started) game->started = true;
        if (game->board[x][y].isRevealed || game->board[x][y].isFlagged) return;
        if (game->mines[x][y]) game->isGameOver = true;
        handleReveal(game, x, y);
    } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        if (game->board[x][y].isRevealed) return;
        game->board[x][y].isFlagged = !game->board[x][y].isFlagged;
    }
}

int countSurroundingMines(Game *game, int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++) {
            if (x + i < 0 || x + i >= BOARD_WIDTH || y + j < 0 || y + j >= BOARD_HEIGHT) continue;
            if (game->mines[x + i][y + j]) count++;
        }
    return count;
}

void handleReveal(Game *game, int x, int y) {
    game->board[x][y].isRevealed = true;
    if (countSurroundingMines(game, x, y) > 0) return;

    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++) {
            if (x + i < 0 || x + i >= BOARD_WIDTH || y + j < 0 || y + j >= BOARD_HEIGHT) continue;
            if (!game->board[x + i][y + j].isRevealed) handleReveal(game, x + i, y + j);
        }
}
