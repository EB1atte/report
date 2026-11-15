#include "raylib.h"
#include <math.h>
#include <stdbool.h>

#define SCREEN_W 800
#define SCREEN_H 600
#define MAX_BULLETS 35
#define MAX_ENEMIES 20

// 총알 위치와 사용 여부를 저장하는 구조체
typedef struct {
    Vector2 position;
    bool active;
} Bullet;

// 적의 위치, 속도, 크기와 사용 여부를 저장하는 구조체
typedef struct {
    Vector2 position;
    float speed;
    float radius;
    bool active;
} Enemy;

// 우주선이 화면 밖으로 나가지 않도록 값을 제한
static float ClampPosition(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// 게임리셋
static void ResetGame(Bullet bullets[], Enemy enemies[], Vector2 *player,
                      int *score, int *lives, float *spawnTimer, float *shotTimer) {
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    *player = (Vector2){ SCREEN_W / 2.0f, SCREEN_H - 70.0f };
    *score = 0;
    *lives = 3;
    *spawnTimer = 0.5f;
    *shotTimer = 0.0f;
}

// 빈 총알 칸에 투사체를 생성
static void FireBullet(Bullet bullets[], Vector2 player) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].position = (Vector2){ player.x, player.y - 24.0f };
            break;
        }
    }
}

// 빈 적 칸에  적을 생성
static void SpawnEnemy(Enemy enemies[], int score) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].radius = (float)GetRandomValue(15, 23);
            enemies[i].position = (Vector2){
                (float)GetRandomValue(30, SCREEN_W - 30), -25.0f
            };
            enemies[i].speed = (float)GetRandomValue(105, 175) +
                               (float)(score / 100) * 10.0f;
            break;
        }
    }
}

// 삼각형과 불꽃으로 플레이어 우주선을 그림.
static void DrawPlayer(Vector2 player) {
    DrawTriangle((Vector2){ player.x, player.y - 27 },
                 (Vector2){ player.x - 22, player.y + 22 },
                 (Vector2){ player.x + 22, player.y + 22 }, SKYBLUE);
    DrawRectangle((int)player.x - 5, (int)player.y - 3, 10, 17, BLUE);
    DrawTriangle((Vector2){ player.x - 8, player.y + 22 },
                 (Vector2){ player.x + 8, player.y + 22 },
                 (Vector2){ player.x, player.y + 36 }, ORANGE);
}

int main(void) {
    // 필요한 자료들 초기회
    InitWindow(SCREEN_W, SCREEN_H, "SPACE SHOOTER - raylib");
    SetTargetFPS(60);

    Bullet bullets[MAX_BULLETS] = { 0 };
    Enemy enemies[MAX_ENEMIES] = { 0 };
    Vector2 player = { 0 };
    int score = 0;
    int lives = 3;
    float spawnTimer = 0.0f;
    float shotTimer = 0.0f;
    bool gameOver = false;
    ResetGame(bullets, enemies, &player, &score, &lives, &spawnTimer, &shotTimer);

    // 
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;

        if (gameOver) {
            // 게임 오버 상태에서 R 키로 초기화 함수
            if (IsKeyPressed(KEY_R)) {
                ResetGame(bullets, enemies, &player, &score, &lives,
                          &spawnTimer, &shotTimer);
                gameOver = false;
            }
        } else {
            // 이동 함수 연결
            Vector2 direction = { 0, 0 };
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) direction.x -= 1;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) direction.x += 1;
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) direction.y -= 1;
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) direction.y += 1;
            float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
            if (length > 0) {
                direction.x /= length;
                direction.y /= length;
            }
            player.x = ClampPosition(player.x + direction.x * 310.0f * dt, 25, SCREEN_W - 25);
            player.y = ClampPosition(player.y + direction.y * 310.0f * dt, 35, SCREEN_H - 38);

            // 총알 발사
            shotTimer -= dt;
            if (IsKeyDown(KEY_SPACE) && shotTimer <= 0.0f) {
                FireBullet(bullets, player);
                shotTimer = 0.18f;
            }

            // 적 생성 타임 함수
            spawnTimer -= dt;
            if (spawnTimer <= 0.0f) {
                SpawnEnemy(enemies, score);
                float interval = 0.85f - score * 0.0015f;
                spawnTimer = interval > 0.28f ? interval : 0.28f;
            }

            // 총알 판정
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) continue;
                bullets[i].position.y -= 530.0f * dt;
                if (bullets[i].position.y < -12) bullets[i].active = false;
            }

            // 적 로직
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].active) continue;
                enemies[i].position.y += enemies[i].speed * dt;
                if (enemies[i].position.y - enemies[i].radius > SCREEN_H) {
                    enemies[i].active = false;
                    lives--;
                    continue;
                }

                // 총알 적 판정
                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (!bullets[j].active) continue;
                    if (CheckCollisionCircles(enemies[i].position, enemies[i].radius,
                                              bullets[j].position, 5.0f)) {
                        enemies[i].active = false;
                        bullets[j].active = false;
                        score += 10;
                        break;
                    }
                }
                if (!enemies[i].active) continue;

                // 생명 로직
                if (CheckCollisionCircles(enemies[i].position, enemies[i].radius,
                                          player, 18.0f)) {
                    enemies[i].active = false;
                    lives--;
                }
            }

            // 게임오버
            if (lives <= 0) {
                lives = 0;
                gameOver = true;
            }
        }

        // 배경과 별을 그려 우주 공간을 표현 ai 생성 파트
        BeginDrawing();
        ClearBackground((Color){ 9, 13, 32, 255 });
        for (int i = 0; i < 65; i++) {
            int x = (i * 137 + 43) % SCREEN_W;
            int y = (i * 83 + 97) % SCREEN_H;
            DrawCircle(x, y, (i % 3 == 0) ? 2 : 1, (Color){ 115, 136, 180, 210 });
        }

        // 살아 있는 총알, 적, 우주선을 화면에 그림.
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active)
                DrawRectangle((int)bullets[i].position.x - 3,
                              (int)bullets[i].position.y - 11, 6, 18, YELLOW);
        }
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) continue;
            DrawCircleV(enemies[i].position, enemies[i].radius, MAGENTA);
            DrawCircleV(enemies[i].position, enemies[i].radius * 0.58f, PURPLE);
            DrawCircle((int)enemies[i].position.x - 6,
                       (int)enemies[i].position.y - 3, 3, WHITE);
            DrawCircle((int)enemies[i].position.x + 6,
                       (int)enemies[i].position.y - 3, 3, WHITE);
        }
        if (!gameOver) DrawPlayer(player);

        // 점수, 남은 생명, 조작법을 화면 위아래에 표시함.
        DrawText(TextFormat("SCORE  %d", score), 20, 18, 26, RAYWHITE);
        DrawText(TextFormat("LIVES  %d", lives), SCREEN_W - 170, 18, 26, PINK);
        DrawText("MOVE: WASD / ARROWS     SHOOT: SPACE", 20, SCREEN_H - 25, 17, LIGHTGRAY);

        // 게임 종료 ui표시
        if (gameOver) {
            DrawRectangle(135, 190, 530, 200, (Color){ 15, 19, 45, 240 });
            DrawRectangleLines(135, 190, 530, 200, PINK);
            DrawText("GAME OVER", 270, 215, 44, PINK);
            DrawText(TextFormat("FINAL SCORE: %d", score), 280, 284, 25, WHITE);
            DrawText("PRESS R TO RESTART", 259, 340, 22, SKYBLUE);
        }
        EndDrawing();
    }

    // 게임 창 닫기
    CloseWindow();
    return 0;
}
