#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
using namespace std;

// Constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_WIDTH = 40;
const int PLAYER_HEIGHT = 30;
const int ENEMY_WIDTH = 30;
const int ENEMY_HEIGHT = 30;
const int BULLET_WIDTH = 4;
const int BULLET_HEIGHT = 12;
const int PLAYER_SPEED = 5;
const int BULLET_SPEED = 7;
const int ENEMY_BULLET_SPEED = 4;
const int ENEMY_SPACING_X = 60;
const int ENEMY_SPACING_Y = 50;

// Enemy formation patterns
enum Pattern {
    PATTERN_CLASSIC,    // Traditional rows
    PATTERN_DIAMOND,    // Diamond formation
    PATTERN_V_SHAPE,    // V formation
    PATTERN_CIRCLE,     // Circular formation
    PATTERN_WAVE        // Wave pattern
};

// Entity structures
struct Entity {
    float x, y;
    int width, height;
    bool active;
    
    Entity(float x = 0, float y = 0, int w = 0, int h = 0) 
        : x(x), y(y), width(w), height(h), active(true) {}
    
    bool collidesWith(const Entity& other) const {
        return active && other.active &&
               x < other.x + other.width &&
               x + width > other.x &&
               y < other.y + other.height &&
               y + height > other.y;
    }
};

struct Player : Entity {
    int lives;
    Player(float x, float y) : Entity(x, y, PLAYER_WIDTH, PLAYER_HEIGHT), lives(3) {}
};

struct Enemy : Entity {
    int type;
    float originalX, originalY;  // For pattern movement
    Enemy(float x, float y, int t = 0) 
        : Entity(x, y, ENEMY_WIDTH, ENEMY_HEIGHT), type(t), originalX(x), originalY(y) {}
};

struct Bullet : Entity {
    bool fromPlayer;
    Bullet(float x, float y, bool fp) 
        : Entity(x, y, BULLET_WIDTH, BULLET_HEIGHT), fromPlayer(fp) {}
};

// Game class
class SpaceInvaders {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* largeFont;
    bool running;
    
    Player player;
    vector<Enemy> enemies;
    vector<Bullet> bullets;
    
    float enemyDirection;
    float enemySpeed;
    int score;
    int frameCount;
    bool gameOver;
    bool victory;
    
    // Level system
    int level;
    int enemiesKilledThisLevel;
    bool levelTransition;
    int transitionTimer;
    Pattern currentPattern;
    
    void initEnemies() {
        enemies.clear();
        
        // Determine pattern based on level
        currentPattern = (Pattern)(level % 5);
        
        // Calculate number of enemies (increases with level)
        int rows = 4 + (level / 3);  // More rows every 3 levels
        int cols = 8 + (level / 2);  // More columns every 2 levels
        
        // Cap maximum enemies
        if (rows > 8) rows = 8;
        if (cols > 12) cols = 12;
        
        float centerX = SCREEN_WIDTH / 2;
        float centerY = 150;
        
        switch(currentPattern) {
            case PATTERN_CLASSIC:
                createClassicPattern(rows, cols);
                break;
            case PATTERN_DIAMOND:
                createDiamondPattern();
                break;
            case PATTERN_V_SHAPE:
                createVPattern();
                break;
            case PATTERN_CIRCLE:
                createCirclePattern();
                break;
            case PATTERN_WAVE:
                createWavePattern(rows, cols);
                break;
        }
        
        // Reset enemy movement
        enemyDirection = 1.0f;
        enemySpeed = 0.5f + (level * 0.15f);  // Faster each level
        enemiesKilledThisLevel = 0;
    }
    
    void createClassicPattern(int rows, int cols) {
        float startX = (SCREEN_WIDTH - (cols * ENEMY_SPACING_X)) / 2;
        float startY = 80;
        
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                float x = startX + col * ENEMY_SPACING_X;
                float y = startY + row * ENEMY_SPACING_Y;
                int type = row % 4;
                enemies.push_back(Enemy(x, y, type));
            }
        }
    }
    
    void createDiamondPattern() {
        float centerX = SCREEN_WIDTH / 2;
        float startY = 80;
        int size = 5 + level / 2;
        if (size > 8) size = 8;
        
        for (int row = 0; row < size; row++) {
            int enemiesInRow = (row < size/2) ? (row * 2 + 1) : ((size - row - 1) * 2 + 1);
            float rowWidth = enemiesInRow * ENEMY_SPACING_X;
            float startX = centerX - rowWidth / 2;
            
            for (int i = 0; i < enemiesInRow; i++) {
                float x = startX + i * ENEMY_SPACING_X;
                float y = startY + row * ENEMY_SPACING_Y;
                enemies.push_back(Enemy(x, y, row % 4));
            }
        }
    }
    
    void createVPattern() {
        float centerX = SCREEN_WIDTH / 2;
        float startY = 80;
        int size = 6 + level / 2;
        if (size > 10) size = 10;
        
        for (int row = 0; row < size; row++) {
            // Left arm of V
            float leftX = centerX - row * 30;
            float y = startY + row * ENEMY_SPACING_Y;
            enemies.push_back(Enemy(leftX, y, row % 4));
            
            // Right arm of V
            float rightX = centerX + row * 30;
            enemies.push_back(Enemy(rightX, y, row % 4));
        }
    }
    
    void createCirclePattern() {
        float centerX = SCREEN_WIDTH / 2;
        float centerY = 150;
        float radius = 100 + level * 10;
        int numEnemies = 12 + level * 2;
        if (numEnemies > 30) numEnemies = 30;
        
        for (int i = 0; i < numEnemies; i++) {
            float angle = (2 * M_PI * i) / numEnemies;
            float x = centerX + radius * cos(angle) - ENEMY_WIDTH / 2;
            float y = centerY + radius * sin(angle) - ENEMY_HEIGHT / 2;
            enemies.push_back(Enemy(x, y, i % 4));
        }
    }
    
    void createWavePattern(int rows, int cols) {
        float startX = (SCREEN_WIDTH - (cols * ENEMY_SPACING_X)) / 2;
        float startY = 80;
        
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                float x = startX + col * ENEMY_SPACING_X;
                // Create sine wave
                float waveOffset = sin((col / (float)cols) * M_PI * 2) * 30;
                float y = startY + row * ENEMY_SPACING_Y + waveOffset;
                enemies.push_back(Enemy(x, y, row % 4));
            }
        }
    }
    
    void renderText(const string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse = nullptr) {
        if (fontToUse == nullptr) fontToUse = font;
        if (!fontToUse) return;
        
        SDL_Surface* surface = TTF_RenderText_Solid(fontToUse, text.c_str(), color);
        if (!surface) return;
        
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            SDL_FreeSurface(surface);
            return;
        }
        
        SDL_Rect destRect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &destRect);
        
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
    
    void handleInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                if (gameOver) {
                    // Restart game
                    score = 0;
                    level = 1;
                    player.lives = 3;
                    player.x = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
                    player.active = true;
                    bullets.clear();
                    initEnemies();
                    gameOver = false;
                    victory = false;
                    levelTransition = false;
                } else if (levelTransition) {
                    // Skip level transition
                    levelTransition = false;
                    initEnemies();
                } else if (victory) {
                    // Continue to next level
                    victory = false;
                    level++;
                    levelTransition = true;
                    transitionTimer = 0;
                } else {
                    // Shoot
                    bullets.push_back(Bullet(player.x + PLAYER_WIDTH/2 - BULLET_WIDTH/2, 
                                            player.y, true));
                }
            }
        }
        
        if (!gameOver && !victory && !levelTransition) {
            const Uint8* keyState = SDL_GetKeyboardState(NULL);
            
            if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A]) {
                player.x -= PLAYER_SPEED;
                if (player.x < 0) player.x = 0;
            }
            if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D]) {
                player.x += PLAYER_SPEED;
                if (player.x > SCREEN_WIDTH - PLAYER_WIDTH) 
                    player.x = SCREEN_WIDTH - PLAYER_WIDTH;
            }
        }
    }
    
    void updateEnemies() {
        if (enemies.empty()) {
            victory = true;
            return;
        }
        
        bool shouldMoveDown = false;
        
        // Check if any enemy hit screen edge
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            
            if ((enemyDirection > 0 && enemy.x + ENEMY_WIDTH >= SCREEN_WIDTH - 10) ||
                (enemyDirection < 0 && enemy.x <= 10)) {
                shouldMoveDown = true;
                break;
            }
        }
        
        // Move enemies
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            
            enemy.x += enemyDirection * enemySpeed;
            
            if (shouldMoveDown) {
                enemy.y += ENEMY_HEIGHT / 2;
                
                // Check if enemies reached player
                if (enemy.y + ENEMY_HEIGHT >= player.y) {
                    gameOver = true;
                }
            }
        }
        
        if (shouldMoveDown) {
            enemyDirection *= -1;
        }
        
        // Random enemy shooting (more frequent at higher levels)
        int shootFrequency = max(30, 60 - level * 3);
        if (frameCount % shootFrequency == 0 && !enemies.empty()) {
            vector<Enemy*> activeEnemies;
            for (auto& enemy : enemies) {
                if (enemy.active) activeEnemies.push_back(&enemy);
            }
            
            if (!activeEnemies.empty()) {
                // Multiple enemies can shoot at higher levels
                int numShooters = 1 + (level / 4);
                if (numShooters > 3) numShooters = 3;
                
                for (int i = 0; i < numShooters && i < (int)activeEnemies.size(); i++) {
                    Enemy* shooter = activeEnemies[rand() % activeEnemies.size()];
                    bullets.push_back(Bullet(shooter->x + ENEMY_WIDTH/2 - BULLET_WIDTH/2,
                                            shooter->y + ENEMY_HEIGHT, false));
                }
            }
        }
    }
    
    void updateBullets() {
        for (auto& bullet : bullets) {
            if (!bullet.active) continue;
            
            if (bullet.fromPlayer) {
                bullet.y -= BULLET_SPEED;
                if (bullet.y < 0) bullet.active = false;
            } else {
                bullet.y += ENEMY_BULLET_SPEED;
                if (bullet.y > SCREEN_HEIGHT) bullet.active = false;
            }
        }
        
        // Remove inactive bullets
        bullets.erase(remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b) { return !b.active; }), bullets.end());
    }
    
    void checkCollisions() {
        // Player bullets vs enemies
        for (auto& bullet : bullets) {
            if (!bullet.active || !bullet.fromPlayer) continue;
            
            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                
                if (bullet.collidesWith(enemy)) {
                    bullet.active = false;
                    enemy.active = false;
                    // Score increases with level
                    int baseScore = (4 - enemy.type) * 10;
                    score += baseScore * level;
                    enemiesKilledThisLevel++;
                    break;
                }
            }
        }
        
        // Enemy bullets vs player
        for (auto& bullet : bullets) {
            if (!bullet.active || bullet.fromPlayer) continue;
            
            if (bullet.collidesWith(player)) {
                bullet.active = false;
                player.lives--;
                
                if (player.lives <= 0) {
                    gameOver = true;
                    player.active = false;
                }
                break;
            }
        }
        
        // Remove dead enemies
        enemies.erase(remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e) { return !e.active; }), enemies.end());
    }
    
    void drawPlayer() {
        if (!player.active) return;
        
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        
        // Draw player ship
        SDL_Rect body = {(int)player.x, (int)player.y + 10, PLAYER_WIDTH, 20};
        SDL_RenderFillRect(renderer, &body);
        
        // Draw cockpit
        SDL_Rect cockpit = {(int)player.x + 15, (int)player.y, 10, 15};
        SDL_RenderFillRect(renderer, &cockpit);
    }
    
    void drawEnemies() {
        for (const auto& enemy : enemies) {
            if (!enemy.active) continue;
            
            // Different colors for different types
            switch(enemy.type) {
                case 0: SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); break;
                case 1: SDL_SetRenderDrawColor(renderer, 255, 128, 0, 255); break;
                case 2: SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); break;
                case 3: SDL_SetRenderDrawColor(renderer, 128, 255, 0, 255); break;
            }
            
            // Draw enemy body
            SDL_Rect body = {(int)enemy.x, (int)enemy.y, ENEMY_WIDTH, ENEMY_HEIGHT};
            SDL_RenderFillRect(renderer, &body);
            
            // Draw eyes
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect eye1 = {(int)enemy.x + 8, (int)enemy.y + 10, 4, 4};
            SDL_Rect eye2 = {(int)enemy.x + 18, (int)enemy.y + 10, 4, 4};
            SDL_RenderFillRect(renderer, &eye1);
            SDL_RenderFillRect(renderer, &eye2);
        }
    }
    
    void drawBullets() {
        for (const auto& bullet : bullets) {
            if (!bullet.active) continue;
            
            if (bullet.fromPlayer) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
            }
            
            SDL_Rect rect = {(int)bullet.x, (int)bullet.y, BULLET_WIDTH, BULLET_HEIGHT};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    
    void drawUI() {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color yellow = {255, 255, 0, 255};
        
        // Draw lives text and count
        string livesText = "Lives: " + to_string(player.lives);
        renderText(livesText, 10, 10, white);
        
        // Draw score
        string scoreText = "Score: " + to_string(score);
        renderText(scoreText, SCREEN_WIDTH / 2 - 60, 10, white);
        
        // Draw level
        string levelText = "Level: " + to_string(level);
        renderText(levelText, SCREEN_WIDTH - 120, 10, yellow);
    }
    
    void renderTextCentered(const string& text, int centerX, int y, SDL_Color color, TTF_Font* fontToUse = nullptr) {
        if (fontToUse == nullptr) fontToUse = font;
        if (!fontToUse) return;
        
        SDL_Surface* surface = TTF_RenderText_Solid(fontToUse, text.c_str(), color);
        if (!surface) return;
        
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            SDL_FreeSurface(surface);
            return;
        }
        
        // Center the text
        int textWidth = surface->w;
        SDL_Rect destRect = {centerX - textWidth/2, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &destRect);
        
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
    
    void drawLevelTransition() {
        // Draw semi-transparent overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        
        // Draw box
        SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
        SDL_Rect box = {SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 100, 400, 200};
        SDL_RenderFillRect(renderer, &box);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect innerBox = {SCREEN_WIDTH/2 - 195, SCREEN_HEIGHT/2 - 95, 390, 190};
        SDL_RenderFillRect(renderer, &innerBox);
        
        // Draw text
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color cyan = {0, 255, 255, 255};
        
        string levelText = "LEVEL " + to_string(level);
        string patternText = "Pattern: ";
        
        switch(currentPattern) {
            case PATTERN_CLASSIC: patternText += "Classic"; break;
            case PATTERN_DIAMOND: patternText += "Diamond"; break;
            case PATTERN_V_SHAPE: patternText += "V-Formation"; break;
            case PATTERN_CIRCLE: patternText += "Circle"; break;
            case PATTERN_WAVE: patternText += "Wave"; break;
        }
        
        string readyText = "Press SPACE to continue";
        
        renderTextCentered(levelText, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 60, cyan, largeFont);
        renderTextCentered(patternText, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 10, white);
        renderTextCentered(readyText, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 40, white);
    }
    
    void drawGameOver() {
        // Draw semi-transparent overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        
        // Draw box
        SDL_Color boxColor = gameOver ? (SDL_Color){255, 0, 0, 255} : (SDL_Color){0, 255, 0, 255};
        SDL_SetRenderDrawColor(renderer, boxColor.r, boxColor.g, boxColor.b, 255);
        SDL_Rect box = {SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 120, 400, 240};
        SDL_RenderFillRect(renderer, &box);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect innerBox = {SCREEN_WIDTH/2 - 195, SCREEN_HEIGHT/2 - 115, 390, 230};
        SDL_RenderFillRect(renderer, &innerBox);
        
        // Draw text
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color bright = {255, 255, 100, 255};  // Brighter yellow for main text
        string mainText = gameOver ? "GAME OVER!" : "LEVEL COMPLETE!";
        string levelText = "Level Reached: " + to_string(level);
        string scoreText = "Final Score: " + to_string(score);
        string restartText = gameOver ? "Press SPACE to restart" : "Press SPACE for next level";
        
        // Use large font only for GAME OVER, regular font for LEVEL COMPLETE
        TTF_Font* titleFont = gameOver ? largeFont : font;
        renderTextCentered(mainText, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 70, bright, titleFont);
        renderTextCentered(levelText, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 20, white);
        renderTextCentered(scoreText, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 10, white);
        renderTextCentered(restartText, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 60, white);
    }
    
public:
    SpaceInvaders() : window(nullptr), renderer(nullptr), font(nullptr), largeFont(nullptr),
                      running(true), player(SCREEN_WIDTH/2 - PLAYER_WIDTH/2, SCREEN_HEIGHT - 80),
                      enemyDirection(1.0f), enemySpeed(0.5f), score(0), 
                      frameCount(0), gameOver(false), victory(false),
                      level(1), enemiesKilledThisLevel(0), levelTransition(false), 
                      transitionTimer(0), currentPattern(PATTERN_CLASSIC) {
        srand(time(NULL));
    }
    
    bool init() {
        cout << "Initializing SDL..." << endl;
        
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cerr << "SDL_Init Error: " << SDL_GetError() << endl;
            return false;
        }
        
        if (TTF_Init() < 0) {
            cerr << "TTF_Init Error: " << TTF_GetError() << endl;
            SDL_Quit();
            return false;
        }
        
        cout << "SDL and SDL_ttf initialized successfully" << endl;
        
        window = SDL_CreateWindow("Space Invaders - Multi-Level Edition", 
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
            TTF_Quit();
            SDL_Quit();
            return false;
        }
        
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            return false;
        }
        
        // Try to load system fonts
        const char* fontPaths[] = {
            "/System/Library/Fonts/Helvetica.ttc",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
            "/Library/Fonts/Arial.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
        };
        
        for (const char* path : fontPaths) {
            font = TTF_OpenFont(path, 24);
            if (font) {
                cout << "Font loaded: " << path << endl;
                break;
            }
        }
        
        for (const char* path : fontPaths) {
            largeFont = TTF_OpenFont(path, 48);
            if (largeFont) break;
        }
        
        initEnemies();
        levelTransition = true;  // Start with level intro
        cout << "Game initialized. Starting level 1..." << endl;
        return true;
    }
    
    void run() {
        cout << "Game running!" << endl;
        
        while (running) {
            handleInput();
            
            if (levelTransition) {
                transitionTimer++;
            } else if (!gameOver && !victory) {
                updateEnemies();
                updateBullets();
                checkCollisions();
                frameCount++;
            }
            
            // Render
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            
            drawPlayer();
            drawEnemies();
            drawBullets();
            drawUI();
            
            if (levelTransition) {
                drawLevelTransition();
            } else if (gameOver || victory) {
                drawGameOver();
            }
            
            SDL_RenderPresent(renderer);
            SDL_Delay(16); // ~60 FPS
        }
        
        cout << "Game ended. Final score: " << score << " Level: " << level << endl;
    }
    
    void cleanup() {
        if (largeFont) TTF_CloseFont(largeFont);
        if (font) TTF_CloseFont(font);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    cout << "=== Space Invaders - Multi-Level Edition ===" << endl;
    
    SpaceInvaders game;
    
    if (!game.init()) {
        cerr << "Failed to initialize game!" << endl;
        return 1;
    }
    
    game.run();
    game.cleanup();
    
    cout << "Thanks for playing!" << endl;
    return 0;
}