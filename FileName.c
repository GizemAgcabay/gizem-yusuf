#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_BLOCKS 10
#define MAX_ENEMIES 5
#define MAX_TRAJECTORY_POINTS 100
#define ENEMY_MAX_HEALTH 3

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool launched;
    float radius;
} Bird;

typedef struct {
    Rectangle rect;
    bool active;
    Vector2 velocity;
    bool falling;
    Rectangle startRect;
    float rotation;
    float angularVelocity;
    bool onGround;  // New field to track if block is on ground
} Block;

typedef struct {
    Vector2 position;
    float radius;
    bool active;
    Vector2 velocity;
    bool falling;
    bool landed;
    int health;          // Health points for enemy
    int maxHealth;       // Maximum health
    float hitTimer;      // Timer to prevent multiple hits in quick succession
} Enemy;

typedef struct {
    Vector2 points[MAX_TRAJECTORY_POINTS];
    int count;
} TrajectoryPoints;

typedef enum {
    MENU,
    GAME,
    SETTINGS
} GameState;

Enemy enemies[MAX_ENEMIES];
Block blocks[MAX_BLOCKS];
int enemyCount = 0;
int blockCount = 0;
bool soundMuted = false;  // Global sound mute state

// Function to calculate trajectory
TrajectoryPoints CalculateTrajectory(Vector2 startPos, Vector2 velocity, int numPoints, float timeStep) {
    TrajectoryPoints trajectory;
    trajectory.count = numPoints;
    const float gravity = 0.5f;

    for (int i = 0; i < numPoints; i++) {
        float t = i * timeStep;
        float x = startPos.x + velocity.x * t;
        float y = startPos.y + velocity.y * t + 0.5f * gravity * t * t;
        trajectory.points[i] = (Vector2){ x, y };
    }
    return trajectory;
}

// Function to initialize enemies
void InitializeEnemies(void) {
    enemyCount = 2;
    enemies[0] = (Enemy){
        {1000.0f, 390.0f}, 15.0f, true, {0.0f, 0.0f}, false, false,
        ENEMY_MAX_HEALTH, ENEMY_MAX_HEALTH, 0.0f
    };
    enemies[1] = (Enemy){
        {1000.0f, 505.0f}, 15.0f, true, {0.0f, 0.0f}, false, false,
        ENEMY_MAX_HEALTH, ENEMY_MAX_HEALTH, 0.0f
    };
}

// Function to initialize blocks
void InitializeBlocks(void) {
    blockCount = 8;

    Rectangle r1 = { 1000.0f, 300.0f, 46.0f, 120.0f };
    blocks[0] = (Block){ r1, true, {0.0f, 0.0f}, false, r1, 0.0f, 0.0f, false };

    Rectangle r2 = { 913.0f, 500.0f, 140.0f, 70.0f };
    blocks[1] = (Block){ r2, true, {0.0f, 0.0f}, false, r2, 0.0f, 0.0f, false };

    Rectangle r3 = { 1000.0f, 416.0f, 46.0f, 120.0f };
    blocks[2] = (Block){ r3, true, {0.0f, 0.0f}, false, r3, 0.0f, 0.0f, false };

    Rectangle r4 = { 913.0f, 270.0f, 140.0f, 70.0f };
    blocks[3] = (Block){ r4, true, {0.0f, 0.0f}, false, r4, 0.0f, 0.0f, false };

    Rectangle r5 = { 912.0f, 300.0f, 46.0f, 120.0f };
    blocks[4] = (Block){ r5, true, {0.0f, 0.0f}, false, r5, 0.0f, 0.0f, false };

    Rectangle r6 = { 913.0f, 384.0f, 140.0f, 70.0f };
    blocks[5] = (Block){ r6, true, {0.0f, 0.0f}, false, r6, 0.0f, 0.0f, false };

    Rectangle r7 = { 915.0f, 416.0f, 46.0f, 120.0f };
    blocks[6] = (Block){ r7, true, {0.0f, 0.0f}, false, r7, 0.0f, 0.0f, false };

    Rectangle r8 = { 913.0f, 250.0f, 140.0f, 70.0f };
    blocks[7] = (Block){ r8, true, {0.0f, 0.0f}, false, r8, 0.0f, 0.0f, false };
}

// Function to damage enemy
void DamageEnemy(int enemyIndex, int damage) {
    if (enemyIndex < 0 || enemyIndex >= enemyCount || !enemies[enemyIndex].active) return;

    enemies[enemyIndex].health -= damage;
    if (enemies[enemyIndex].health <= 0) {
        enemies[enemyIndex].active = false;
    }
}

// Function to check if all enemies are dead
bool AllEnemiesDead(void) {
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            return false;
        }
    }
    return true;
}

// Function to reset game
void ResetGame(Bird* bird, int* score, int* lives, bool* gameOver) {
    *bird = (Bird){ { 150.0f, 400.0f }, { 0.0f, 0.0f }, false, 15.0f };
    *score = 0;
    *lives = 3;
    *gameOver = false;

    // Reset enemies to original positions
    enemies[0].position = (Vector2){ 1000.0f, 390.0f };
    enemies[0].active = true;
    enemies[0].falling = false;
    enemies[0].landed = false;
    enemies[0].velocity = (Vector2){ 0.0f, 0.0f };
    enemies[0].health = ENEMY_MAX_HEALTH;
    enemies[0].hitTimer = 0.0f;

    enemies[1].position = (Vector2){ 1000.0f, 505.0f };
    enemies[1].active = true;
    enemies[1].falling = false;
    enemies[1].landed = false;
    enemies[1].velocity = (Vector2){ 0.0f, 0.0f };
    enemies[1].health = ENEMY_MAX_HEALTH;
    enemies[1].hitTimer = 0.0f;

    // Reset blocks
    for (int i = 0; i < blockCount; i++) {
        blocks[i].rect = blocks[i].startRect;
        blocks[i].active = true;
        blocks[i].falling = false;
        blocks[i].velocity = (Vector2){ 0.0f, 0.0f };
        blocks[i].angularVelocity = 0.0f;
        blocks[i].rotation = 0.0f;
        blocks[i].onGround = false;
    }
}

int main(void) {
    const int screenWidth = 1536;
    const int screenHeight = 800;
    const float gravity = 0.41f;
    const int maxLives = 3;

    InitWindow(screenWidth, screenHeight, "Angry Birds - Raylib C");
    SetTargetFPS(60);

    // Initialize audio
    InitAudioDevice();

    // Load textures
    Image bgImage = LoadImage("backpeace.jpg");
    ImageResize(&bgImage, 1536, 1024);
    Texture2D background = LoadTextureFromImage(bgImage);
    UnloadImage(bgImage);

    Texture2D ground = LoadTexture("ground.png");
    Texture2D slingTexture = LoadTexture("sling.png");
    Texture2D blockTexture1 = LoadTexture("blockd.png");
    Texture2D blockTexture2 = LoadTexture("blocky.png");
    Texture2D enemyTexture = LoadTexture("enemy.png");
    Texture2D menuBackground = LoadTexture("menu.png");
    Texture2D birdTexture = LoadTexture("angrybird.png");

    if (birdTexture.id == 0) {
        TraceLog(LOG_ERROR, "Bird texture failed to load!");
    }

    // Initialize game state
    GameState currentState = MENU;
    Bird bird = { { 150.0f, 400.0f }, { 0.0f, 0.0f }, false, 15.0f };
    int lives = maxLives;
    int score = 0;
    bool gameOver = false;
    bool dragging = false;
    bool victory = false;

    // Initialize enemies and blocks
    InitializeEnemies();
    InitializeBlocks();

    // Main game loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (currentState == MENU) {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexture(menuBackground, 0, 0, WHITE);

            Rectangle playButton = { 900, 180, 300, 100 };
            Rectangle settingsButton = { 900, 300, 300, 100 };
            Rectangle exitButton = { 900, 420, 300, 100 };

            Vector2 mousePoint = GetMousePosition();

            if (CheckCollisionPointRec(mousePoint, playButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentState = GAME;
                victory = false;
            }
            if (CheckCollisionPointRec(mousePoint, settingsButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentState = SETTINGS;
            }
            if (CheckCollisionPointRec(mousePoint, exitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Cleanup and close window
                UnloadTexture(background);
                UnloadTexture(ground);
                UnloadTexture(birdTexture);
                UnloadTexture(slingTexture);
                UnloadTexture(blockTexture1);
                UnloadTexture(blockTexture2);
                UnloadTexture(enemyTexture);
                UnloadTexture(menuBackground);
                CloseAudioDevice();
                CloseWindow();
                return 0;
            }

            Rectangle backButton = { 1450, 5, 100, 50 };
            DrawRectangleRec(backButton, LIGHTGRAY);
            DrawText("Back", backButton.x + 20, backButton.y + 10, 20, DARKGRAY);

            EndDrawing();
            continue;
        }

        if (currentState == SETTINGS) {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexture(menuBackground, 0, 0, WHITE);

            // Title
            DrawText("SETTINGS", screenWidth / 2 - 100, 150, 40, DARKGRAY);

            // Mute/Unmute button
            Rectangle muteButton = { 900, 300, 300, 100 };
            Color buttonColor = soundMuted ? RED : GREEN;
            DrawRectangleRec(muteButton, buttonColor);
            DrawRectangleLinesEx(muteButton, 3.0f, BLACK);

            const char* buttonText = soundMuted ? "SOUND: OFF" : "SOUND: ON";
            int textWidth = MeasureText(buttonText, 20);
            DrawText(buttonText,
                muteButton.x + (muteButton.width - textWidth) / 2,
                muteButton.y + 40,
                20, WHITE);

            // Back button
            Rectangle backButton = { 900, 450, 300, 100 };
            DrawRectangleRec(backButton, LIGHTGRAY);
            DrawRectangleLinesEx(backButton, 3.0f, BLACK);
            DrawText("BACK", backButton.x + 120, backButton.y + 40, 20, DARKGRAY);

            Vector2 mousePoint = GetMousePosition();

            if (CheckCollisionPointRec(mousePoint, muteButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                soundMuted = !soundMuted;
                if (soundMuted) {
                    SetMasterVolume(0.0f);
                }
                else {
                    SetMasterVolume(1.0f);
                }
            }

            if (CheckCollisionPointRec(mousePoint, backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentState = MENU;
            }

            EndDrawing();
            continue;
        }

        // Update enemy hit timers
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].hitTimer > 0.0f) {
                enemies[i].hitTimer -= deltaTime;
            }
        }

        // Block-enemy collision
        for (int i = 0; i < blockCount; i++) {
            if (blocks[i].active && blocks[i].falling) {
                for (int j = 0; j < enemyCount; j++) {
                    if (enemies[j].active && !enemies[j].falling && enemies[j].hitTimer <= 0.0f &&
                        CheckCollisionCircleRec(enemies[j].position, enemies[j].radius, blocks[i].rect)) {

                        // Damage enemy by 1 point from block collision
                        DamageEnemy(j, 1);
                        enemies[j].hitTimer = 0.5f; // Prevent multiple hits for 0.5 seconds

                        if (enemies[j].active) {
                            enemies[j].falling = true;
                            enemies[j].velocity = (Vector2){ 0.0f, -4.0f };
                        }
                        else {
                            score += 100; // Bonus points for killing enemy
                        }
                    }
                }
            }
        }

        // Enemy falling physics
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].active && enemies[i].falling) {
                enemies[i].velocity.y += gravity;
                enemies[i].position.y += enemies[i].velocity.y;

                float groundY = screenHeight - 250.0f;
                if (enemies[i].position.y + enemies[i].radius >= groundY) {
                    enemies[i].position.y = groundY - enemies[i].radius;
                    enemies[i].velocity.y = 0.0f;
                    enemies[i].falling = false;
                    enemies[i].landed = true;
                }
            }
        }

        // Mouse input for bird launching
        if (!bird.launched && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointCircle(GetMousePosition(), bird.position, bird.radius)) {
                dragging = true;
            }
        }

        if (dragging) {
            bird.position = GetMousePosition();
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                dragging = false;
                bird.velocity = (Vector2){ (150.0f - bird.position.x) * 0.2f, (400.0f - bird.position.y) * 0.2f };
                bird.launched = true;
            }
        }

        // Bird physics
        if (bird.launched) {
            bird.velocity.y += gravity;
            bird.position.x += bird.velocity.x;
            bird.position.y += bird.velocity.y;

            // Bird-enemy collision (instant kill)
            for (int i = 0; i < enemyCount; i++) {
                if (enemies[i].active &&
                    CheckCollisionCircles(bird.position, bird.radius, enemies[i].position, enemies[i].radius)) {

                    // Bird kills enemy instantly
                    enemies[i].active = false;
                    score += 150; // High score for direct bird hit
                }
            }

            // Ground collision
            float groundHeight = 250.0f;
            float groundY = screenHeight - groundHeight;

            if (bird.position.y + bird.radius >= groundY) {
                bird.position.y = groundY - bird.radius;
                bird.velocity.y *= -0.5f;

                if (fabs(bird.velocity.y) < 1.0f) {
                    bird.velocity.y = 0.0f;
                }
            }

            // Reset bird if stopped or out of bounds
            if ((fabs(bird.velocity.x) < 0.5f && fabs(bird.velocity.y) < 0.5f) ||
                bird.position.x > (float)screenWidth || bird.position.x < 0.0f || bird.position.y < 0.0f) {

                if (lives > 1) {
                    bird = (Bird){ { 150.0f, 400.0f }, { 0.0f, 0.0f }, false, 15.0f };
                    lives--;

                    // Check if all enemies are dead after bird reset
                    if (AllEnemiesDead()) {
                        victory = true;
                    }
                }
                else {
                    gameOver = true;
                }
            }

            // Bird-block collision
            for (int i = 0; i < blockCount; i++) {
                if (blocks[i].active && !blocks[i].falling &&
                    CheckCollisionCircleRec(bird.position, bird.radius, blocks[i].rect)) {
                    blocks[i].falling = true;
                    blocks[i].velocity = (Vector2){ 0.0f, -4.0f };
                    blocks[i].angularVelocity = ((float)GetRandomValue(-20, 20)) / 10.0f;

                    score += 10;
                    float randomX = GetRandomValue(-3, 3);
                    blocks[i].velocity = (Vector2){ (float)randomX, -4.0f };

                    score += 10;
                    bird.velocity.x *= -0.3f;
                    bird.velocity.y *= -0.3f;
                }
            }
        }

        // Check victory condition
        if (AllEnemiesDead() && !victory) {
            victory = true;
        }

        // Reset game
        if (IsKeyPressed(KEY_R)) {
            ResetGame(&bird, &score, &lives, &gameOver);
            victory = false;
        }

        // Improved block physics - reduces vibration
        for (int i = 0; i < blockCount; i++) {
            if (blocks[i].active && blocks[i].falling && !blocks[i].onGround) {
                float mass = 1.0f + (blocks[i].rect.width * blocks[i].rect.height) / 10000.0f;
                blocks[i].velocity.y += gravity * mass;
                blocks[i].velocity.x *= 0.98f; // Slightly more friction

                blocks[i].rect.x += blocks[i].velocity.x;
                blocks[i].rect.y += blocks[i].velocity.y;
                blocks[i].rotation += blocks[i].angularVelocity;
                blocks[i].angularVelocity *= 0.95f; // More angular friction

                float groundY = screenHeight - 250.0f;
                if (blocks[i].rect.y + blocks[i].rect.height >= groundY) {
                    blocks[i].rect.y = groundY - blocks[i].rect.height;
                    blocks[i].velocity = (Vector2){ 0.0f, 0.0f };
                    blocks[i].angularVelocity = 0.0f;
                    blocks[i].rotation = 0.0f;
                    blocks[i].falling = false;
                    blocks[i].onGround = true; // Mark as on ground to prevent further movement
                }
            }
        }

        // Block-block collision
        for (int i = 0; i < blockCount; i++) {
            if (!blocks[i].active || !blocks[i].falling || blocks[i].onGround) continue;

            for (int j = 0; j < blockCount; j++) {
                if (i == j || !blocks[j].active || blocks[j].falling || blocks[j].onGround) continue;

                if (CheckCollisionRecs(blocks[i].rect, blocks[j].rect)) {
                    blocks[j].falling = true;
                    blocks[j].velocity = (Vector2){ (float)GetRandomValue(-1, 1), -2.0f };
                    blocks[j].angularVelocity = ((float)GetRandomValue(-10, 10)) / 10.0f;
                }
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexture(background, 0, -200, WHITE);

        // Ground
        Rectangle sourceRec = { 0.0f, 0.0f, (float)ground.width, (float)ground.height };
        Rectangle destRec = { 0.0f, (float)(screenHeight - 600), (float)screenWidth, 700.0f };
        Vector2 groundOrigin = { 0.0f, 0.0f };
        DrawTexturePro(ground, sourceRec, destRec, groundOrigin, 0.0f, WHITE);

        DrawText("Angry Birds (Raylib)", 20, 20, 30, RED);
        DrawText(TextFormat("Score: %i", score), 20, 60, 20, DARKGRAY);
        DrawText(TextFormat("Lives: %d", lives), 20, 120, 20, DARKBLUE);
        DrawText("R to reset", 20, 90, 20, GRAY);

        if (gameOver && !victory) {
            DrawText("GAME OVER!", screenWidth / 2 - 100, screenHeight / 2, 40, RED);
            DrawText("R - Try again", screenWidth / 2 - 100, screenHeight / 2 + 50, 20, GRAY);
        }

        if (victory) {
            DrawText("YOU WIN!", screenWidth / 2 - 80, screenHeight / 2 - 40, 40, DARKGREEN);
            DrawText("R - Play again", screenWidth / 2 - 80, screenHeight / 2 + 10, 20, GRAY);
        }

        // Draw bird
        Vector2 birdOrigin = { birdTexture.width / 2.0f, birdTexture.height / 2.0f };
        DrawTexturePro(birdTexture,
            (Rectangle) {
            0.0f, 0.0f, (float)birdTexture.width, (float)birdTexture.height
        },
            (Rectangle) {
            bird.position.x, bird.position.y, (float)birdTexture.width, (float)birdTexture.height
        },
            birdOrigin,
            0.0f,
            WHITE
        );

        // Draw enemies with health bars (invisible)
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].active) {
                float enemyScale = 0.05f;
                float enemyWidth = enemyTexture.width * enemyScale;
                float enemyHeight = enemyTexture.height * enemyScale;

                Rectangle source = { 0.0f, 0.0f, (float)enemyTexture.width, (float)enemyTexture.height };
                Rectangle dest = {
                    enemies[i].position.x - enemyWidth / 1.2f,
                    enemies[i].position.y - enemyHeight / 2.0f,
                    enemyWidth,
                    enemyHeight
                };
                Vector2 origin = { 0.0f, 0.0f };

                DrawTexturePro(enemyTexture, source, dest, origin, 0.0f, WHITE);
            }
        }

        // Draw blocks
        for (int i = 0; i < blockCount; i++) {
            if (blocks[i].active) {
                Texture2D textureToDraw = (i % 2 == 0) ? blockTexture1 : blockTexture2;

                Rectangle source = { 0.0f, 0.0f, (float)textureToDraw.width, (float)textureToDraw.height };
                Rectangle dest = {
                    blocks[i].rect.x,
                    blocks[i].rect.y,
                    blocks[i].rect.width,
                    blocks[i].rect.height
                };

                Vector2 origin = { blocks[i].rect.width / 2.0f, blocks[i].rect.height / 2.0f };
                dest.x += origin.x;
                dest.y += origin.y;

                DrawTexturePro(textureToDraw, source, dest, origin, blocks[i].rotation, WHITE);
            }
        }

        // Draw slingshot rope
        if (!bird.launched) {
            DrawLineEx((Vector2) { 150.0f, 400.0f }, bird.position, 3.0f, GRAY);
        }

        // Draw slingshot
        Vector2 slingPos = { 150.0f, 400.0f };
        float scale = 0.18f;
        float newWidth = slingTexture.width * scale;
        float newHeight = slingTexture.height * scale;

        Vector2 drawPos = {
            slingPos.x - newWidth / 2.0f,
            slingPos.y - newHeight / 8.0f
        };

        DrawTexturePro(
            slingTexture,
            (Rectangle) {
            0.0f, 0.0f, (float)slingTexture.width, (float)slingTexture.height
        },
            (Rectangle) {
            drawPos.x, drawPos.y, newWidth, newHeight
        },
            (Vector2) {
            0.0f, 0.0f
        },
            0.0f,
            WHITE
        );

        // Draw trajectory
        if (!bird.launched && dragging) {
            Vector2 velocity = {
                (slingPos.x - bird.position.x) * 0.2f,
                (slingPos.y - bird.position.y) * 0.2f
            };
            TrajectoryPoints trajPoints = CalculateTrajectory(slingPos, velocity, 100, 0.9f);
            for (int i = 0; i < trajPoints.count; i++) {
                DrawCircleV(trajPoints.points[i], 2.0f, WHITE);
            }
        }

        // Draw game instructions
        DrawText("Bird: Instant kill | Blocks: 3 hits to kill", 20, 150, 16, DARKGRAY);

        EndDrawing();
    }

    // Cleanup
    UnloadTexture(background);
    UnloadTexture(ground);
    UnloadTexture(birdTexture);
    UnloadTexture(slingTexture);
    UnloadTexture(blockTexture1);
    UnloadTexture(blockTexture2);
    UnloadTexture(enemyTexture);
    UnloadTexture(menuBackground);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}