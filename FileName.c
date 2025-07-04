#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_BLOCKS 10
#define MAX_ENEMIES 5
#define MAX_TRAJECTORY_POINTS 100
#define ENEMY_MAX_HEALTH 3
#define DARKRED (Color){139, 0, 0, 255}
#define DARKBLUE (Color){0, 0, 139, 255}
#define DARKGREEN (Color){0, 100, 0, 255}

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
    bool onGround;
    float mass;
    float friction;
    float bounciness;
} Block;

typedef struct {
    Vector2 position;
    float radius;
    bool active;
    Vector2 velocity;
    bool falling;
    bool landed;
    int health;
    int maxHealth;
    float hitTimer;
} Enemy;

typedef struct {
    Vector2 points[MAX_TRAJECTORY_POINTS];
    int count;
} TrajectoryPoints;

typedef enum {
    MENU,
    GAME,
    SETTINGS,
    LEVEL_COMPLETE
} GameState;

Enemy enemies[MAX_ENEMIES];
Block blocks[MAX_BLOCKS];
int enemyCount = 0;
int blockCount = 0;
bool soundMuted = false;
int currentLevel = 1;
int totalLevels = 2;

// Settings window variables
bool settingsWindowOpen = false;
Rectangle settingsWindow = { 400, 200, 400, 300 };
float masterVolume = 1.0f;
bool showTrajectory = true;
int difficultyLevel = 1; // 1 = Easy, 2 = Medium, 3 = Hard

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

// Function to initialize enemies for different levels
void InitializeEnemies(int level) {
    if (level == 1) {
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
    else if (level == 2) {
        enemyCount = 3;
        enemies[0] = (Enemy){
            {1000.0f, 390.0f}, 15.0f, true, {0.0f, 0.0f}, false, false,
            ENEMY_MAX_HEALTH, ENEMY_MAX_HEALTH, 0.0f
        };
        enemies[1] = (Enemy){
            {1000.0f, 505.0f}, 15.0f, true, {0.0f, 0.0f}, false, false,
            ENEMY_MAX_HEALTH, ENEMY_MAX_HEALTH, 0.0f
        };
        enemies[2] = (Enemy){
            {920.0f, 390.0f}, 15.0f, true, {0.0f, 0.0f}, false, false,
            ENEMY_MAX_HEALTH, ENEMY_MAX_HEALTH, 0.0f
        };
    }
}

// Function to initialize blocks with improved physics properties
void InitializeBlocks(int level) {
    blockCount = 8;

    if (level == 1) {
        Rectangle r1 = { 1000.0f, 300.0f, 46.0f, 120.0f };
        blocks[0] = (Block){ r1, true, {0.0f, 0.0f}, false, r1, 0.0f, 0.0f, false, 2.0f, 0.8f, 0.3f };

        Rectangle r2 = { 913.0f, 500.0f, 140.0f, 70.0f };
        blocks[1] = (Block){ r2, true, {0.0f, 0.0f}, false, r2, 0.0f, 0.0f, false, 3.0f, 0.9f, 0.2f };

        Rectangle r3 = { 1000.0f, 416.0f, 46.0f, 120.0f };
        blocks[2] = (Block){ r3, true, {0.0f, 0.0f}, false, r3, 0.0f, 0.0f, false, 2.0f, 0.8f, 0.3f };

        Rectangle r4 = { 913.0f, 270.0f, 140.0f, 70.0f };
        blocks[3] = (Block){ r4, true, {0.0f, 0.0f}, false, r4, 0.0f, 0.0f, false, 3.0f, 0.9f, 0.2f };

        Rectangle r5 = { 912.0f, 300.0f, 46.0f, 120.0f };
        blocks[4] = (Block){ r5, true, {0.0f, 0.0f}, false, r5, 0.0f, 0.0f, false, 2.0f, 0.8f, 0.3f };

        Rectangle r6 = { 913.0f, 384.0f, 140.0f, 70.0f };
        blocks[5] = (Block){ r6, true, {0.0f, 0.0f}, false, r6, 0.0f, 0.0f, false, 3.0f, 0.9f, 0.2f };

        Rectangle r7 = { 915.0f, 416.0f, 46.0f, 120.0f };
        blocks[6] = (Block){ r7, true, {0.0f, 0.0f}, false, r7, 0.0f, 0.0f, false, 2.0f, 0.8f, 0.3f };

        Rectangle r8 = { 913.0f, 250.0f, 140.0f, 70.0f };
        blocks[7] = (Block){ r8, true, {0.0f, 0.0f}, false, r8, 0.0f, 0.0f, false, 3.0f, 0.9f, 0.2f };
    }
    else if (level == 2) {
        // Level 2 has same block layout but different positions
        Rectangle r1 = { 1000.0f, 300.0f, 46.0f, 120.0f };
        blocks[0] = (Block){ r1, true, {0.0f, 0.0f}, false, r1, 0.0f, 0.0f, false, 2.5f, 0.7f, 0.4f };

        Rectangle r2 = { 913.0f, 500.0f, 140.0f, 70.0f };
        blocks[1] = (Block){ r2, true, {0.0f, 0.0f}, false, r2, 0.0f, 0.0f, false, 3.5f, 0.8f, 0.3f };

        Rectangle r3 = { 1000.0f, 416.0f, 46.0f, 120.0f };
        blocks[2] = (Block){ r3, true, {0.0f, 0.0f}, false, r3, 0.0f, 0.0f, false, 2.5f, 0.7f, 0.4f };

        Rectangle r4 = { 913.0f, 270.0f, 140.0f, 70.0f };
        blocks[3] = (Block){ r4, true, {0.0f, 0.0f}, false, r4, 0.0f, 0.0f, false, 3.5f, 0.8f, 0.3f };

        Rectangle r5 = { 850.0f, 300.0f, 46.0f, 120.0f };
        blocks[4] = (Block){ r5, true, {0.0f, 0.0f}, false, r5, 0.0f, 0.0f, false, 2.5f, 0.7f, 0.4f };

        Rectangle r6 = { 850.0f, 384.0f, 140.0f, 70.0f };
        blocks[5] = (Block){ r6, true, {0.0f, 0.0f}, false, r6, 0.0f, 0.0f, false, 3.5f, 0.8f, 0.3f };

        Rectangle r7 = { 850.0f, 416.0f, 46.0f, 120.0f };
        blocks[6] = (Block){ r7, true, {0.0f, 0.0f}, false, r7, 0.0f, 0.0f, false, 2.5f, 0.7f, 0.4f };

        Rectangle r8 = { 850.0f, 250.0f, 140.0f, 70.0f };
        blocks[7] = (Block){ r8, true, {0.0f, 0.0f}, false, r8, 0.0f, 0.0f, false, 3.5f, 0.8f, 0.3f };
    }
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

    // Reset enemies based on current level
    InitializeEnemies(currentLevel);

    // Reset blocks based on current level
    InitializeBlocks(currentLevel);
}

// Function to advance to next level
void NextLevel(Bird* bird, int* score, int* lives, bool* gameOver) {
    currentLevel++;
    if (currentLevel > totalLevels) {
        currentLevel = 1; // Loop back to first level
    }

    *bird = (Bird){ { 150.0f, 400.0f }, { 0.0f, 0.0f }, false, 15.0f };
    *lives = 3;
    *gameOver = false;

    // Initialize new level
    InitializeEnemies(currentLevel);
    InitializeBlocks(currentLevel);
}

// Function to draw settings window
void DrawSettingsWindow(void) {
    if (!settingsWindowOpen) return;

    // Draw semi-transparent overlay
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));

    // Draw settings window
    DrawRectangleRec(settingsWindow, LIGHTGRAY);
    DrawRectangleLinesEx(settingsWindow, 3.0f, DARKGRAY);

    // Title
    DrawText("SETTINGS", settingsWindow.x + 130, settingsWindow.y + 20, 24, DARKGRAY);

    // Volume slider
    DrawText("Master Volume:", settingsWindow.x + 20, settingsWindow.y + 70, 16, DARKGRAY);
    Rectangle volumeSlider = { settingsWindow.x + 20, settingsWindow.y + 100, 200, 20 };
    DrawRectangleRec(volumeSlider, WHITE);
    DrawRectangleLinesEx(volumeSlider, 2.0f, DARKGRAY);

    float sliderPos = volumeSlider.x + (masterVolume * volumeSlider.width);
    DrawCircle((int)sliderPos, (int)(volumeSlider.y + volumeSlider.height / 2), 8, RED);

    // Trajectory toggle
    Rectangle trajectoryToggle = { settingsWindow.x + 20, settingsWindow.y + 140, 20, 20 };
    DrawRectangleRec(trajectoryToggle, showTrajectory ? GREEN : RED);
    DrawRectangleLinesEx(trajectoryToggle, 2.0f, DARKGRAY);
    DrawText("Show Trajectory", settingsWindow.x + 50, settingsWindow.y + 143, 16, DARKGRAY);

    // Difficulty selector
    DrawText("Difficulty:", settingsWindow.x + 20, settingsWindow.y + 180, 16, DARKGRAY);
    Rectangle easyBtn = { settingsWindow.x + 20, settingsWindow.y + 200, 60, 30 };
    Rectangle mediumBtn = { settingsWindow.x + 90, settingsWindow.y + 200, 60, 30 };
    Rectangle hardBtn = { settingsWindow.x + 160, settingsWindow.y + 200, 60, 30 };

    DrawRectangleRec(easyBtn, difficultyLevel == 1 ? DARKGREEN : LIGHTGRAY);
    DrawRectangleRec(mediumBtn, difficultyLevel == 2 ? ORANGE : LIGHTGRAY);
    DrawRectangleRec(hardBtn, difficultyLevel == 3 ? RED : LIGHTGRAY);

    DrawText("Easy", easyBtn.x + 15, easyBtn.y + 8, 12, WHITE);
    DrawText("Medium", mediumBtn.x + 8, mediumBtn.y + 8, 12, WHITE);
    DrawText("Hard", hardBtn.x + 15, hardBtn.y + 8, 12, WHITE);

    // Close button
    Rectangle closeBtn = { settingsWindow.x + settingsWindow.width - 80, settingsWindow.y + settingsWindow.height - 50, 60, 30 };
    DrawRectangleRec(closeBtn, GRAY);
    DrawText("Close", closeBtn.x + 12, closeBtn.y + 8, 16, WHITE);

    // Handle input
    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, closeBtn)) {
            settingsWindowOpen = false;
        }

        if (CheckCollisionPointRec(mousePos, volumeSlider)) {
            masterVolume = (mousePos.x - volumeSlider.x) / volumeSlider.width;
            masterVolume = fmaxf(0.0f, fminf(1.0f, masterVolume));
            SetMasterVolume(masterVolume);
        }

        if (CheckCollisionPointRec(mousePos, trajectoryToggle)) {
            showTrajectory = !showTrajectory;
        }

        if (CheckCollisionPointRec(mousePos, easyBtn)) {
            difficultyLevel = 1;
        }
        if (CheckCollisionPointRec(mousePos, mediumBtn)) {
            difficultyLevel = 2;
        }
        if (CheckCollisionPointRec(mousePos, hardBtn)) {
            difficultyLevel = 3;
        }
    }
}

// Improved block physics with realistic motion
void UpdateBlockPhysics(float deltaTime) {
    const float gravity = 0.5f;
    const float groundY = GetScreenHeight() - 250.0f;

    for (int i = 0; i < blockCount; i++) {
        if (!blocks[i].active || blocks[i].onGround) continue;

        if (blocks[i].falling) {
            // Apply gravity based on mass
            blocks[i].velocity.y += gravity * blocks[i].mass * deltaTime * 60.0f;

            // Apply air resistance
            blocks[i].velocity.x *= (1.0f - 0.02f * deltaTime * 60.0f);
            blocks[i].velocity.y *= (1.0f - 0.01f * deltaTime * 60.0f);

            // Update position
            blocks[i].rect.x += blocks[i].velocity.x * deltaTime * 60.0f;
            blocks[i].rect.y += blocks[i].velocity.y * deltaTime * 60.0f;

            // Update rotation
            blocks[i].rotation += blocks[i].angularVelocity * deltaTime * 60.0f;
            blocks[i].angularVelocity *= (1.0f - 0.05f * deltaTime * 60.0f);

            // Ground collision with bounce
            if (blocks[i].rect.y + blocks[i].rect.height >= groundY) {
                blocks[i].rect.y = groundY - blocks[i].rect.height;
                blocks[i].velocity.y *= -blocks[i].bounciness;
                blocks[i].velocity.x *= blocks[i].friction;
                blocks[i].angularVelocity *= 0.7f;

                // Stop if velocity is too low
                if (fabs(blocks[i].velocity.y) < 1.0f && fabs(blocks[i].velocity.x) < 0.5f) {
                    blocks[i].velocity = (Vector2){ 0.0f, 0.0f };
                    blocks[i].angularVelocity = 0.0f;
                    blocks[i].falling = false;
                    blocks[i].onGround = true;
                }
            }

            // Side boundary collision
            if (blocks[i].rect.x < 0) {
                blocks[i].rect.x = 0;
                blocks[i].velocity.x *= -0.5f;
            }
            if (blocks[i].rect.x + blocks[i].rect.width > GetScreenWidth()) {
                blocks[i].rect.x = GetScreenWidth() - blocks[i].rect.width;
                blocks[i].velocity.x *= -0.5f;
            }
        }
    }
}

int main(void) {
    const int screenWidth = 1536;
    const int screenHeight = 800;
    const float gravity = 0.41f;
    const int maxLives = 3;

    InitWindow(screenWidth, screenHeight, "Angry Birds - Enhanced Edition");
    SetTargetFPS(60);

    // Initialize audio
    InitAudioDevice();
    SetMasterVolume(masterVolume);

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

    // Initialize first level
    InitializeEnemies(currentLevel);
    InitializeBlocks(currentLevel);

    // Main game loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (currentState == MENU) {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexture(menuBackground, 0, 0, WHITE);

            Rectangle playButton = { 800, 150, 300, 100 };
            Rectangle settingsButton = { 950 , 500, 300, 100 };
            Rectangle exitButton = { 450, 440, 300, 100 };

            Vector2 mousePoint = GetMousePosition();

            // Draw buttons
           

            if (CheckCollisionPointRec(mousePoint, playButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentState = GAME;
                victory = false;
            }
            if (CheckCollisionPointRec(mousePoint, settingsButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                settingsWindowOpen = true;
            }
            if (CheckCollisionPointRec(mousePoint, exitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                break;
            }

            // Draw settings window if open
            DrawSettingsWindow();

            EndDrawing();
            continue;
        }

        if (currentState == LEVEL_COMPLETE) {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexture(background, 0, -200, WHITE);

            DrawText("LEVEL COMPLETE!", screenWidth / 2 - 150, screenHeight / 2 - 100, 40, DARKGREEN);
            DrawText(TextFormat("Final Score: %d", score), screenWidth / 2 - 100, screenHeight / 2 - 50, 24, DARKGRAY);

            Rectangle nextLevelBtn = { screenWidth / 2 - 100, screenHeight / 2, 200, 50 };
            Rectangle menuBtn = { screenWidth / 2 - 100, screenHeight / 2 + 70, 200, 50 };

            DrawRectangleRec(nextLevelBtn, DARKGREEN);
            DrawRectangleRec(menuBtn, DARKBLUE);

            DrawText("NEXT LEVEL", nextLevelBtn.x + 50, nextLevelBtn.y + 15, 20, WHITE);
            DrawText("MAIN MENU", menuBtn.x + 50, menuBtn.y + 15, 20, WHITE);

            Vector2 mousePoint = GetMousePosition();
            if (CheckCollisionPointRec(mousePoint, nextLevelBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                NextLevel(&bird, &score, &lives, &gameOver);
                currentState = GAME;
                victory = false;
            }
            if (CheckCollisionPointRec(mousePoint, menuBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentState = MENU;
                currentLevel = 1;
                ResetGame(&bird, &score, &lives, &gameOver);
                victory = false;
            }

            EndDrawing();
            continue;
        }

        // Game logic (existing code with improvements)

        // Update enemy hit timers
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].hitTimer > 0.0f) {
                enemies[i].hitTimer -= deltaTime;
            }
        }

        // Improved block physics
        UpdateBlockPhysics(deltaTime);

        // Block-enemy collision
        for (int i = 0; i < blockCount; i++) {
            if (blocks[i].active && blocks[i].falling) {
                for (int j = 0; j < enemyCount; j++) {
                    if (enemies[j].active && !enemies[j].falling && enemies[j].hitTimer <= 0.0f &&
                        CheckCollisionCircleRec(enemies[j].position, enemies[j].radius, blocks[i].rect)) {

                        DamageEnemy(j, 1);
                        enemies[j].hitTimer = 0.5f;

                        if (enemies[j].active) {
                            enemies[j].falling = true;
                            enemies[j].velocity = (Vector2){ 0.0f, -4.0f };
                        }
                        else {
                            score += 100;
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

            // Bird-enemy collision
            for (int i = 0; i < enemyCount; i++) {
                if (enemies[i].active &&
                    CheckCollisionCircles(bird.position, bird.radius, enemies[i].position, enemies[i].radius)) {
                    enemies[i].active = false;
                    score += 150;
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

                    if (AllEnemiesDead()) {
                        victory = true;
                    }
                }
                else {
                    gameOver = true;
                }
            }

            // Bird-block collision with improved physics
            for (int i = 0; i < blockCount; i++) {
                if (blocks[i].active && !blocks[i].falling &&
                    CheckCollisionCircleRec(bird.position, bird.radius, blocks[i].rect)) {

                    blocks[i].falling = true;

                    // Calculate impact force based on bird velocity
                    float impactForce = sqrt(bird.velocity.x * bird.velocity.x + bird.velocity.y * bird.velocity.y);

                    blocks[i].velocity = (Vector2){
                        bird.velocity.x * 0.3f + ((float)GetRandomValue(-2, 2)),
                        -impactForce * 0.2f
                    };
                    blocks[i].angularVelocity = ((float)GetRandomValue(-30, 30)) / 10.0f;

                    score += 10;

                    // Reduce bird velocity after impact
                    bird.velocity.x *= 0.7f;
                    bird.velocity.y *= 0.7f;
                }
            }
        }

        // Check victory condition
        if (AllEnemiesDead() && !victory) {
            victory = true;
            if (currentLevel < totalLevels) {
                currentState = LEVEL_COMPLETE;
            }
        }

        // Reset game
        if (IsKeyPressed(KEY_R)) {
            ResetGame(&bird, &score, &lives, &gameOver);
            victory = false;
        }

        // Block-block collision with improved physics
        for (int i = 0; i < blockCount; i++) {
            if (!blocks[i].active || !blocks[i].falling || blocks[i].onGround) continue;

            for (int j = 0; j < blockCount; j++) {
                if (i == j || !blocks[j].active || blocks[j].falling || blocks[j].onGround) continue;

                if (CheckCollisionRecs(blocks[i].rect, blocks[j].rect)) {
                    blocks[j].falling = true;

                    // Transfer some momentum
                    blocks[j].velocity = (Vector2){
                        blocks[i].velocity.x * 0.5f + ((float)GetRandomValue(-1, 1)),
                        -3.0f + ((float)GetRandomValue(-1, 1))
                    };
                    blocks[j].angularVelocity = ((float)GetRandomValue(-15, 15)) / 10.0f;

                    // Reduce original block's velocity
                    blocks[i].velocity.x *= 0.8f;
                    blocks[i].velocity.y *= 0.8f;
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

        // UI
        DrawText("Angry Birds - Enhanced Edition", 20, 20, 30, RED);
        DrawText(TextFormat("Score: %i", score), 20, 60, 20, DARKGRAY);
        DrawText(TextFormat("Lives: %d", lives), 20, 90, 20, DARKBLUE);
        DrawText(TextFormat("Level: %d/%d", currentLevel, totalLevels), 20, 120, 20, DARKGREEN);
        DrawText("R to reset", 20, 150, 20, GRAY);

        if (gameOver && !victory) {
            DrawText("GAME OVER!", screenWidth / 2 - 100, screenHeight / 2, 40, RED);
            DrawText("R - Try again", screenWidth / 2 - 100, screenHeight / 2 + 50, 20, GRAY);
        }

        if (victory && currentLevel >= totalLevels) {
            DrawText("ALL LEVELS COMPLETE!", screenWidth / 2 - 150, screenHeight / 2 - 40, 40, DARKGREEN);
            DrawText("R - Play again", screenWidth / 2 - 100, screenHeight / 2 + 10, 20, GRAY);
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
            birdOrigin, 0.0f, WHITE);

        // Draw enemies with health indication
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].active) {
                float enemyScale = 0.05f;
                float enemyWidth = enemyTexture.width * enemyScale;
                float enemyHeight = enemyTexture.height * enemyScale;

                Rectangle source = { 0.0f, 0.0f, (float)enemyTexture.width, (float)enemyTexture.height };
                Rectangle dest = {
                    enemies[i].position.x - enemyWidth / 1.2f,
                    enemies[i].position.y - enemyHeight / 2.0f,
                    enemyWidth, enemyHeight
                };
                Vector2 origin = { 0.0f, 0.0f };

                DrawTexturePro(enemyTexture, source, dest, origin, 0.0f, WHITE);

                // Draw health bar
                Rectangle healthBar = {
                    enemies[i].position.x - 20,
                    enemies[i].position.y - 25,
                    40, 6
                };
                DrawRectangleRec(healthBar, RED);

                Rectangle healthFill = {
                    healthBar.x, healthBar.y,
                    healthBar.width * ((float)enemies[i].health / (float)enemies[i].maxHealth),
                    healthBar.height
                };
                DrawRectangleRec(healthFill, GREEN);
                DrawRectangleLinesEx(healthBar, 1.0f, BLACK);
            }
        }

        // Draw blocks with improved rotation
        for (int i = 0; i < blockCount; i++) {
            if (blocks[i].active) {
                Texture2D textureToDraw = (i % 2 == 0) ? blockTexture1 : blockTexture2;

                Rectangle source = { 0.0f, 0.0f, (float)textureToDraw.width, (float)textureToDraw.height };
                Rectangle dest = {
                    blocks[i].rect.x + blocks[i].rect.width / 2.0f,
                    blocks[i].rect.y + blocks[i].rect.height / 2.0f,
                    blocks[i].rect.width,
                    blocks[i].rect.height
                };

                Vector2 origin = { blocks[i].rect.width / 2.0f, blocks[i].rect.height / 2.0f };

                DrawTexturePro(textureToDraw, source, dest, origin, blocks[i].rotation * RAD2DEG, WHITE);
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

        DrawTexturePro(slingTexture,
            (Rectangle) {
            0.0f, 0.0f, (float)slingTexture.width, (float)slingTexture.height
        },
            (Rectangle) {
            drawPos.x, drawPos.y, newWidth, newHeight
        },
            (Vector2) {
            0.0f, 0.0f
        }, 0.0f, WHITE);

        // Draw trajectory if enabled
        if (!bird.launched && dragging && showTrajectory) {
            Vector2 velocity = {
                (slingPos.x - bird.position.x) * 0.2f,
                (slingPos.y - bird.position.y) * 0.2f
            };
            TrajectoryPoints trajPoints = CalculateTrajectory(slingPos, velocity, 50, 0.9f);
            for (int i = 0; i < trajPoints.count; i++) {
                float alpha = 1.0f - ((float)i / (float)trajPoints.count);
                DrawCircleV(trajPoints.points[i], 2.0f, Fade(YELLOW, alpha));
            }
        }

        // Draw game instructions
        DrawText("Bird: Instant kill | Blocks: 3 hits to kill", 20, 180, 16, DARKGRAY);
        DrawText("Use mouse to aim and shoot", 20, 200, 16, DARKGRAY);

        // Draw settings button in game
        Rectangle settingsBtn = { screenWidth - 100, 20, 80, 30 };
        DrawRectangleRec(settingsBtn, LIGHTGRAY);
        DrawText("Settings", settingsBtn.x + 10, settingsBtn.y + 8, 16, DARKGRAY);

        if (CheckCollisionPointRec(GetMousePosition(), settingsBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            settingsWindowOpen = true;
        }

        // Draw settings window if open
        DrawSettingsWindow();

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
