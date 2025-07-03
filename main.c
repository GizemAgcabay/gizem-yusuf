#include "raylib.h"
#include <cmath>
#include <vector>
#include <string>

struct Bird {
    Vector2 position;
    Vector2 velocity;
    bool launched;
    float radius;
};

struct Block {
    Rectangle rect;
    bool active;
    Vector2 velocity;
    bool falling;
    Rectangle startRect; // başlangıç konumu
    float rotation;
    float angularVelocity = 0.0f; // Açısal hız
};


struct Enemy {
    Vector2 position;
    float radius;
    bool active;
    Vector2 velocity;
    bool falling;
    bool landed;
};

std::vector<Enemy> enemies = {
    { {1000.0f, 400.0f}, 15.0f, true, {0.0f, 0.0f}, false, false },
    { {1000.0f, 520.0f}, 15.0f, true, {0.0f, 0.0f}, false, false }
};


// Trajectory hesaplama fonksiyonu
std::vector<Vector2> CalculateTrajectory(Vector2 startPos, Vector2 velocity, int numPoints, float timeStep) {
    std::vector<Vector2> points;
    const float gravity = 0.5f;
    for (int i = 0; i < numPoints; i++) {
        float t = i * timeStep;
        float x = startPos.x + velocity.x * t;
        float y = startPos.y + velocity.y * t + 0.5f * gravity * t * t;
        points.push_back({ x, y });
    }
    return points;
}

const float gravity = 0.41f;
enum GameState { MENU, GAME };
GameState currentState = MENU;

int main() {
    const int screenWidth = 1536;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Angry Birds - Raylib");
    SetTargetFPS(60);

    // Arka plan
    Image bgImage = LoadImage("backpeace.jpg");
    ImageResize(&bgImage, 1536, 1024);
    Texture2D background = LoadTextureFromImage(bgImage);
    UnloadImage(bgImage);

    // Zemin resmi
    Texture2D ground = LoadTexture("ground.png");

    Texture2D slingTexture = LoadTexture("sling.png");

    Texture2D blockTexture1 = LoadTexture("blockd.png");  // Tahta 1
    Texture2D blockTexture2 = LoadTexture("blocky.png");  // Tahta 2
    Texture2D enemyTexture = LoadTexture("enemy.png");
    Texture2D menuBackground = LoadTexture("menu.png");  // senin pembe görselin
 ;


    // Kuş resmi
    Texture2D birdTexture = LoadTexture("angrybird.png");
    if (birdTexture.id == 0) {
        TraceLog(LOG_ERROR, "Bird texture failed to load!");
    }

    const int maxLives = 3;
    int lives = maxLives;
    bool gameOver = false;

    Bird bird = { { 150.0f, 400.0f }, { 0.0f, 0.0f }, false, 15.0f };

    
    std::vector<Block> blocks;

    // Kat 3
    Rectangle r1 = { 1000.0f, 300.0f, 46.0f, 120.0f };
    blocks.push_back({ r1, true, {0.0f, 0.0f}, false, r1, 0.0f });   // Y (blocky)

    Rectangle r2 = { 913.0f, 500.0f, 140.0f, 70.0f };
    blocks.push_back({ r2, true, {0.0f, 0.0f}, false, r2, 0.0f });   // Y (blocky)

    Rectangle r3 = { 1000.0f, 416.0f, 46.0f, 120.0f };
    blocks.push_back({ r3, true, {0.0f, 0.0f}, false, r3, 0.0f });   // D (blockd)

    Rectangle r4 = { 913.0f, 270.0f, 140.0f, 70.0f };
    blocks.push_back({ r4, true, {0.0f, 0.0f}, false, r4, 0.0f });   // D (blockd)

    Rectangle r5 = { 912.0f, 300.0f, 46.0f, 120.0f };
    blocks.push_back({ r5, true, {0.0f, 0.0f}, false, r5, 0.0f });   // Y (blocky)

    Rectangle r6 = { 913.0f, 384.0f, 140.0f, 70.0f };
    blocks.push_back({ r6, true, {0.0f, 0.0f}, false, r6, 0.0f });   // Y (blocky)

    Rectangle r7 = { 915.0f, 416.0f, 46.0f, 120.0f };
    blocks.push_back({ r7, true, {0.0f, 0.0f}, false, r7, 0.0f });   // D (blockd)

    Rectangle r8 = { 913.0f, 250.0f, 140.0f, 70.0f };
    blocks.push_back({ r8, true, {0.0f, 0.0f}, false, r8, 0.0f });   // D (blockd)

    int score = 0;
    bool dragging = false;

    while (!WindowShouldClose()) {
        if (currentState == MENU) {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexture(menuBackground, 0, 0, WHITE);

            // Görünür buton çizimi (sadece geçici)
            Rectangle playButton = { 900, 180, 300, 100 };

           

            // Butona tıklanırsa oyuna geç
            if (CheckCollisionPointRec(GetMousePosition(), playButton) &&
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentState = GAME;
            }

            EndDrawing();
            continue; // Menüdeyken oyunu çizme
        }
        // Eğer oyun başladıysa (yani currentState == GAME)
       

        // === GÜNCELLEME ===
        for (auto& block : blocks) {
            if (block.active && block.falling) {
                for (auto& enemy : enemies) {
                    if (enemy.active && !enemy.falling &&
                        CheckCollisionCircleRec(enemy.position, enemy.radius, block.rect)) {
                        enemy.falling = true;
                        enemy.velocity = { 0.0f, -4.0f };
                    }
                }
            }
        }
        for (auto& enemy : enemies) {
            if (enemy.active && enemy.falling) {
                enemy.velocity.y += gravity;
                enemy.position.y += enemy.velocity.y;

                float groundY = screenHeight - 250.0f;
                if (enemy.position.y + enemy.radius >= groundY) {
                    enemy.position.y = groundY - enemy.radius;
                    enemy.velocity.y = 0.0f;
                    enemy.falling = false;
                    enemy.landed = true;
                }
            }
        }
        for (auto& enemy : enemies) {
            if (enemy.active && enemy.landed &&
                CheckCollisionCircles(bird.position, bird.radius, enemy.position, enemy.radius)) {
                enemy.active = false;
                score += 50;
            }
        }

        if (!bird.launched && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointCircle(GetMousePosition(), bird.position, bird.radius)) {
                dragging = true;
            }
        }

        if (dragging) {
            bird.position = GetMousePosition();
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                dragging = false;
                bird.velocity = Vector2{ (150.0f - bird.position.x) * 0.2f, (400.0f - bird.position.y) * 0.2f };
                bird.launched = true;
            }
        }

        if (bird.launched) {
            bird.velocity.y += gravity;
            bird.position.x += bird.velocity.x;
            bird.position.y += bird.velocity.y;

            for (auto& enemy : enemies) {
                // Kuş düşmana çarparsa düşmeye başlasın:
                if (enemy.active && !enemy.falling && CheckCollisionCircles(bird.position, bird.radius, enemy.position, enemy.radius)) {
                    enemy.falling = true;
                    enemy.velocity = { 0.0f, -4.0f };
                }

                // Yere düştüyse düşme dursun
                if (enemy.active && enemy.falling) {
                    enemy.velocity.y += gravity;
                    enemy.position.y += enemy.velocity.y;

                    float groundY = screenHeight - 250.0f;
                    if (enemy.position.y + enemy.radius >= groundY) {
                        enemy.position.y = groundY - enemy.radius;
                        enemy.velocity.y = 0.0f;
                        enemy.falling = false;
                        enemy.landed = true;
                    }
                }

                // Eğer yere indikten sonra kuş çarparsa düşman ölür
                if (enemy.active && enemy.landed &&
                    CheckCollisionCircles(bird.position, bird.radius, enemy.position, enemy.radius)) {
                    enemy.active = false;
                    score += 50;
                }
            }
           
            float groundHeight = 250.0f; // Zemin görselinin yüksekliği (bunu kullandığın değerle eş yap!)
            float groundY = screenHeight - groundHeight;

            if (bird.position.y + bird.radius >= groundY) {
                bird.position.y = groundY - bird.radius;
                bird.velocity.y *= -0.5f;

                // Eğer yavaşladıysa durdur
                if (fabs(bird.velocity.y) < 1.0f) {
                    bird.velocity.y = 0.0f;
                }
            }


            if ((fabs(bird.velocity.x) < 0.5f && fabs(bird.velocity.y) < 0.5f) ||
                bird.position.x > (float)screenWidth || bird.position.x < 0.0f || bird.position.y < 0.0f) {

                if (lives > 1) {
                    bird = { { 150.0f, 400.0f }, { 0.0f, 0.0f }, false, 15.0f };
                    lives--;
                }
                else {
                    gameOver = true;
                }
            }

            for (auto& block : blocks) {
                if (block.active && !block.falling && CheckCollisionCircleRec(bird.position, bird.radius, block.rect)) {
                    block.falling = true;
                    block.velocity = { 0.0f, -4.0f };
                    block.angularVelocity = ((float)GetRandomValue(-20, 20)) / 10.0f;  // Rastgele dönme

                    score += 10;
                    float randomX = GetRandomValue(-3, 3);  // -3 ile 3 arasında rastgele x yönü
                    block.velocity = { (float)randomX, -4.0f };

                    score += 10;
                    bird.velocity.x *= -0.3f;
                    bird.velocity.y *= -0.3f;
                }

            }
        }

        if (IsKeyPressed(KEY_R)) {
            bird = { { 150.0f, 400.0f }, { 0.0f, 0.0f }, false, 15.0f };
            for (auto& block : blocks) block.active = true;
            score = 0;
            lives = maxLives;
            gameOver = false;
            for (auto& enemy : enemies) enemy.active = true;
            for (auto& block : blocks) {
                block.rect = block.startRect;
                block.active = true;
                block.falling = false;
                block.velocity = { 0.0f, 0.0f };
                block.angularVelocity = 0.0f;
                block.rotation = 0.0f;
            }


        }

        // === ÇİZİM ===
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexture(background, 0, -200, WHITE);
		// Arka planı çiz (yüksekliği ayarlayabilirsin)
       

        // Zemin (ground.png görseli, hatasız float ile)
        Rectangle sourceRec = { 0.0f, 0.0f, (float)ground.width, (float)ground.height };
        Rectangle destRec = { 0.0f, (float)(screenHeight - 600), (float)screenWidth, 700.0f };
        Vector2 groundOrigin = { 0.0f, 0.0f };
        DrawTexturePro(ground, sourceRec, destRec, groundOrigin, 0.0f, WHITE);

        DrawText("Angry Birds (Raylib)", 20, 20, 30, RED);
        DrawText(TextFormat("Score: %i", score), 20, 60, 20, DARKGRAY);
        DrawText(TextFormat("Lives: %d", lives), 20, 120, 20, DARKBLUE);
        DrawText("R to reset", 20, 90, 20, GRAY);

        if (gameOver) {
            DrawText("GAME OVER!", screenWidth / 2 - 100, screenHeight / 2, 40, RED);
            DrawText("R - Try again", screenWidth / 2 - 100, screenHeight / 2 + 50, 20, GRAY);
        }

        bool allEnemiesDead = true;
        for (const auto& enemy : enemies) {
            if (enemy.active) {
                allEnemiesDead = false;
                break;
            }
        }
        if (allEnemiesDead && !gameOver) {
            DrawText("YOU WIN!", screenWidth / 2 - 80, screenHeight / 2 - 40, 40, DARKGREEN);
        }

        // Kuş çizimi
        Vector2 birdOrigin = { birdTexture.width / 2.0f, birdTexture.height / 2.0f };
        DrawTexturePro(birdTexture,
            Rectangle{ 0.0f, 0.0f, (float)birdTexture.width, (float)birdTexture.height },
            Rectangle{ bird.position.x, bird.position.y, (float)birdTexture.width, (float)birdTexture.height },
            birdOrigin,
            0.0f,
            WHITE
        );

      

        for (const auto& enemy : enemies) {
            if (enemy.active) {
              
            }
        }

        // Düşmanlar
        
        for (const auto& enemy : enemies) {
            if (enemy.active) {
                Vector2 enemyDrawPos = {
                    enemy.position.x - enemyTexture.width / 2.0f,
                    enemy.position.y - enemyTexture.height / 2.0f
                };
                float enemyScale = 0.05f;  // Oranı küçült (deneyerek ayarlayabilirsin)
                float enemyWidth = enemyTexture.width * enemyScale;
                float enemyHeight = enemyTexture.height * enemyScale;

                Rectangle source = { 0.0f, 0.0f, (float)enemyTexture.width, (float)enemyTexture.height };
                Rectangle dest = {
                    enemy.position.x - enemyWidth / 1.2f,
                    enemy.position.y - enemyHeight / 2.0f,
                    enemyWidth,
                    enemyHeight
                };
                Vector2 origin = { 0.0f, 0.0f };

                DrawTexturePro(enemyTexture, source, dest, origin, 0.0f, WHITE);

            }
        }


        // Bloklar
        Color semiTransparanBrown = { 101, 67, 33, 255 };
        for (size_t i = 0; i < blocks.size(); ++i) {
            if (blocks[i].active) {
                Texture2D textureToDraw = (i % 2 == 0) ? blockTexture1 : blockTexture2;

                Rectangle source = { 0.0f, 0.0f, (float)textureToDraw.width, (float)textureToDraw.height };
                Rectangle dest = {
                    blocks[i].rect.x,
                    blocks[i].rect.y,
                    blocks[i].rect.width,
                    blocks[i].rect.height
                };

                // Dönme için merkez
                Vector2 origin = { blocks[i].rect.width / 2.0f, blocks[i].rect.height / 2.0f };

                // Blokların merkezinden dönebilmesi için konum düzelt
                dest.x += origin.x;
                dest.y += origin.y;

                DrawTexturePro(textureToDraw, source, dest, origin, blocks[i].rotation, WHITE);
            }
        }

        for (auto& block : blocks) {
            if (block.active && block.falling) {
                // Yerçekimi etkisi
                float mass = 1.0f + (block.rect.width * block.rect.height) / 10000.0f;
                block.velocity.y += gravity * mass;

                block.velocity.x *= 0.99f; // hava sürtünmesi

                // Konum güncelle
                block.rect.x += block.velocity.x;
                if (fabs(block.velocity.y) < 0.5f) {
                    block.velocity.y = 0.0f;
                }

                block.rect.y += block.velocity.y;

                // Rotasyon
                block.rotation += block.angularVelocity;
                block.angularVelocity *= 0.99f;

                // Zemine çarptı mı?
                float groundY = screenHeight - 250.0f;
                if (block.rect.y + block.rect.height >= groundY) {
                    block.rect.y = groundY - block.rect.height;

                    // Bu çok önemli: hareketi tamamen durdur
                    block.velocity = { 0.0f, 0.0f };
                    block.angularVelocity = 0.0f;
                    block.rotation = roundf(block.rotation); // hafif dönmeyi durdur

                    block.falling = false;
                }

            }
        }
        // Bloklar birbirine çarparsa diğerini de düşür
        for (size_t i = 0; i < blocks.size(); i++) {
            if (!blocks[i].active || !blocks[i].falling) continue;

            for (size_t j = 0; j < blocks.size(); j++) {
                if (i == j || !blocks[j].active || blocks[j].falling) continue;

                if (CheckCollisionRecs(blocks[i].rect, blocks[j].rect)) {
                    blocks[j].falling = true;
                    blocks[j].velocity = { (float)GetRandomValue(-1, 1), -2.0f };  // daha az hareket
                    blocks[j].angularVelocity = ((float)GetRandomValue(-10, 10)) / 10.0f; // daha az dönme

                }
            }
        }






        // Sapan ipi
        
        if (!bird.launched) {
            DrawLineEx({ 150.0f, 400.0f }, bird.position, 3.0f, GRAY);
        }
            Vector2 slingPos = { 150.0f, 400.0f };
            Vector2 slingOrigin = { slingTexture.width / 2.0f, slingTexture.height / 2.0f }; // merkez noktası

            // Sapan sabit pozisyonda (örnek)
        // Sapan ipi
            

                // Sapan görseli boyutlandırılmış şekilde
                float scale = 0.18f;
                float newWidth = slingTexture.width * scale;
                float newHeight = slingTexture.height * scale;

                // Sapanın ekran konumu (ortalanmış)
                Vector2 drawPos = {
                    slingPos.x - newWidth / 2.0f,
                    slingPos.y - newHeight / 8.0f
                };

                DrawTexturePro(
                    slingTexture,
                    Rectangle{ 0.0f, 0.0f, (float)slingTexture.width, (float)slingTexture.height }, // kaynak boyut
                    Rectangle{ drawPos.x, drawPos.y, newWidth, newHeight }, // hedef boyut ve konum
                    Vector2{ 0.0f, 0.0f },
                    0.0f,
                    WHITE
                );
            


        

        // Nişangah
       
        if (!bird.launched && dragging) {
            Vector2 velocity = {
                (slingPos.x - bird.position.x) * 0.2f,
                (slingPos.y - bird.position.y) * 0.2f
            };
            std::vector<Vector2> trajPoints = CalculateTrajectory(slingPos, velocity, 100, 0.9f);
            for (const auto& point : trajPoints) {
                DrawCircleV(point, 2.0f, WHITE);
            }
        }

        EndDrawing();
    }

    // Kaynakları boşalt
    UnloadTexture(background);
    UnloadTexture(ground);
    UnloadTexture(birdTexture);
    UnloadTexture(slingTexture);
    UnloadTexture(blockTexture1);
    UnloadTexture(blockTexture2);
    UnloadTexture(enemyTexture);
    UnloadTexture(menuBackground);
   


    CloseWindow();
    return 0;
}
