// Catch the Stars - simple 3DS homebrew game
// Move the paddle with the D-Pad or Circle Pad and catch the falling stars.
// Missing a star costs a life. START exits.

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_W    400
#define SCREEN_H    240
#define PADDLE_W    64
#define PADDLE_H    12
#define PADDLE_Y    (SCREEN_H - 24)
#define STAR_SIZE   14
#define NUM_STARS   5
#define START_LIVES 3

typedef struct {
    float x, y, vy;
} Star;

static Star  stars[NUM_STARS];
static float paddleX;
static int   score, best, lives;
static bool  gameOver;

static void spawnStar(Star *s)
{
    float extra = score * 0.05f;
    if (extra > 3.5f) extra = 3.5f;

    s->x  = (float)(rand() % (SCREEN_W - STAR_SIZE));
    s->y  = -(float)(rand() % 240) - STAR_SIZE;
    s->vy = 1.5f + (rand() % 100) / 100.0f + extra;
}

static void resetGame(void)
{
    score    = 0;
    lives    = START_LIVES;
    gameOver = false;
    paddleX  = (SCREEN_W - PADDLE_W) / 2.0f;
    for (int i = 0; i < NUM_STARS; i++)
        spawnStar(&stars[i]);
}

static void drawHud(void)
{
    printf("\x1b[2J");
    printf("\x1b[2;1H  === CATCH THE STARS ===\n\n");
    printf("  Score: %d\n", score);
    printf("  Best:  %d\n", best);
    printf("  Lives: %d\n\n", lives);
    if (gameOver)
        printf("  GAME OVER!\n  Press A to play again\n\n");
    else
        printf("  D-Pad / Circle Pad: move\n\n");
    printf("  START: exit\n");
}

int main(int argc, char *argv[])
{
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    consoleInit(GFX_BOTTOM, NULL);

    C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    const u32 clrBg     = C2D_Color32(0x10, 0x14, 0x30, 0xFF);
    const u32 clrStar   = C2D_Color32(0xFF, 0xD8, 0x30, 0xFF);
    const u32 clrPaddle = C2D_Color32(0x40, 0xC8, 0xFF, 0xFF);
    const u32 clrOver   = C2D_Color32(0xFF, 0x50, 0x50, 0xFF);

    srand((unsigned int)time(NULL));
    best = 0;
    resetGame();
    drawHud();

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_START)
            break;

        bool hudDirty = false;

        if (!gameOver) {
            circlePosition cp;
            hidCircleRead(&cp);

            float move = 0.0f;
            if (kHeld & KEY_LEFT)  move -= 5.0f;
            if (kHeld & KEY_RIGHT) move += 5.0f;
            if (abs(cp.dx) > 15)   move += (cp.dx / 156.0f) * 6.0f;

            paddleX += move;
            if (paddleX < 0)                paddleX = 0;
            if (paddleX > SCREEN_W - PADDLE_W) paddleX = SCREEN_W - PADDLE_W;

            for (int i = 0; i < NUM_STARS; i++) {
                Star *s = &stars[i];
                s->y += s->vy;

                bool caught = (s->y + STAR_SIZE >= PADDLE_Y) &&
                              (s->y <= PADDLE_Y + PADDLE_H) &&
                              (s->x + STAR_SIZE >= paddleX) &&
                              (s->x <= paddleX + PADDLE_W);

                if (caught) {
                    score++;
                    if (score > best) best = score;
                    spawnStar(s);
                    hudDirty = true;
                } else if (s->y > SCREEN_H) {
                    lives--;
                    spawnStar(s);
                    hudDirty = true;
                    if (lives <= 0) {
                        gameOver = true;
                        break;
                    }
                }
            }
        } else if (kDown & KEY_A) {
            resetGame();
            hudDirty = true;
        }

        if (hudDirty)
            drawHud();

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, clrBg);
        C2D_SceneBegin(top);

        for (int i = 0; i < NUM_STARS; i++)
            C2D_DrawRectSolid(stars[i].x, stars[i].y, 0.0f,
                              STAR_SIZE, STAR_SIZE, clrStar);

        C2D_DrawRectSolid(paddleX, PADDLE_Y, 0.0f,
                          PADDLE_W, PADDLE_H, gameOver ? clrOver : clrPaddle);

        C3D_FrameEnd(0);
    }

    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
