#include <SDL2\SDL.h>
#include <SDL2\SDL_image.h>
#include <SDL2\SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <unistd.h>
#include <SDL2\SDL_mixer.h>
#include <fstream>
#include "abstract.hpp"
using namespace std;
int hs;
class AstroStrike : virtual public Arcade
{
private:
    const int screen_width = 800;
    const int screen_height = 600;
    const int GAME_DURATION = 20;
    Mix_Chunk *bulletSound = nullptr;
    SDL_Event event;

    struct Position
    {
        int x, y;
    };

    struct Player
    {
        SDL_Texture *texture;
        SDL_Rect position;
    };

    struct Bullet
    {
        SDL_Texture *texture;
        SDL_Rect position;
        int speed;
        bool active;
    };

    struct Enemy
    {
        SDL_Texture *texture;
        SDL_Rect position;
        int speed;
        bool active;
    };

    struct LargeEnemy
    {
        SDL_Texture *texture;
        SDL_Rect position;
        int health;
        int speed;
        bool active;
    };

    Player player;
    vector<Bullet> bullets;
    vector<Enemy> enemies;
    vector<LargeEnemy> largeEnemies;

    int score;
    bool displayWoah;
    bool displayCheckPoint;
    bool gameover;
    SDL_Texture *backgroundTexture = nullptr;

    Uint32 startTime;
    Uint32 endTime;
    Uint32 currentTime;

    bool initialize()
    {

        startTime = SDL_GetTicks();
        endTime = startTime + (GAME_DURATION * 1000);

        font = TTF_OpenFont("ariali.ttf", 40);

        return true;
    }

    SDL_Texture *loadTexture(const string &filePath)
    {
        SDL_Surface *surface = IMG_Load(filePath.c_str());
        if (!surface)
        { // checking if the surface pointer is null indicating that the image failed to load.
            cout << "Failed to load image: " << IMG_GetError() << endl;
            return nullptr;
        }

        // setinng the color key as white
        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture)
        {
            cout << "Failed to create texture: " << SDL_GetError() << endl;
            return nullptr;
        }

        return texture;
    }

    bool loadMedia()
    {
        player.texture = loadTexture("images/player.png"); // loading the  player texture
        if (!player.texture)
            return false;

        for (int i = 0; i < 1; ++i)
        {
            Bullet bullet;
            bullet.texture = loadTexture("images/bullet.png"); // loading the bullet texture
            if (!bullet.texture)
                return false;

            bullet.active = false;
            bullets.push_back(bullet); // adding the bullet to the bullets vector
        }

        backgroundMusic = Mix_LoadMUS("sound/background_astro.mp3");

        bulletSound = Mix_LoadWAV("sound/bullet_sound.mp3"); // loading the bullet sound effect
        if (!bulletSound)
        {
            cout << "Failed to load bullet sound effect: " << Mix_GetError() << endl;
            return false;
        }

        backgroundTexture = loadTexture("images/astro_background.jpg"); // this loads the background texture
        if (!backgroundTexture)
            return false;

        return true;
    }

    void cleanup()
    {   // this function is responsible for freeing the
        // resources that were allocated during the execution of the game.

        // the if conditions are checking if the pointer is null, if not then it means
        // that the texture was loaded
        if (player.texture)
            SDL_DestroyTexture(player.texture);

        for (vector<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ++it)
        {
            if (it->texture)
                SDL_DestroyTexture(it->texture);
        }

        for (vector<Enemy>::iterator it = enemies.begin(); it != enemies.end(); ++it)
        {
            if (it->texture)
                SDL_DestroyTexture(it->texture);
        }

        for (vector<LargeEnemy>::iterator it = largeEnemies.begin(); it != largeEnemies.end(); ++it)
        {
            if (it->texture)
                SDL_DestroyTexture(it->texture);
        }

        if (backgroundTexture)
            SDL_DestroyTexture(backgroundTexture);

        if (bulletSound)
            Mix_FreeChunk(bulletSound);
    }

    void handleEvents()
    {
        // this loop continues as long as there are events to process.
        // SDL_PollEvent() gets the next event from the queue and stores it in the event variable.
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) // when we click on window close button it should exit
                exit(0);
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    // when we click on esc key it should exit
                    exit(0);
                }
            }
        }
        // handling input from array keys
        const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);
        if (keyboardState[SDL_SCANCODE_LEFT])
        {
            if (player.position.x > 0)
                player.position.x -= 5;
        }
        if (keyboardState[SDL_SCANCODE_RIGHT])
        {
            if (player.position.x < screen_width - player.position.w)
                player.position.x += 5;
        }
        // handling input from space bar
        if (keyboardState[SDL_SCANCODE_SPACE])
        {
            for (int i = 0; i < bullets.size(); ++i)
            {
                Bullet &bullet = bullets[i];
                if (!bullet.active)
                {
                    bullet.position.x = player.position.x + player.position.w / 2 - bullet.position.w / 2;
                    bullet.position.y = player.position.y;
                    bullet.active = true;
                    break;
                }
            }
        }
    }

    // this game pver function is called when time is over
    // it display game over message and fimal score
    void displayGameOverMessage()
    {
        SDL_Surface *surface = TTF_RenderText_Solid(font, "Game Over", {255, 255, 0});
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect textRect;
        textRect.x = (screen_width - surface->w) / 2;
        textRect.y = (screen_height - surface->h) / 2;
        textRect.w = surface->w;
        textRect.h = surface->h;
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(textTexture);

        // Render the score
        string scoreText = "Your Score: " + to_string(score);
        SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), {255, 255, 255});
        SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_Rect scoreRect;
        scoreRect.x = (screen_width - scoreSurface->w) / 2;
        scoreRect.y = textRect.y + textRect.h + 20;
        scoreRect.w = scoreSurface->w;
        scoreRect.h = scoreSurface->h;
        SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);

        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);

        SDL_RenderPresent(renderer);
    }

    // function that displays time
    string formatTime(Uint32 milliseconds)
    {
        int seconds = milliseconds / 1000;
        int minutes = seconds / 60;
        seconds %= 60;

        string timeString = to_string(minutes) + ":";

        if (seconds < 10)
        {
            timeString += "0";
        }

        timeString += to_string(seconds);

        return timeString;
    }

    // function that displays text on exe window
    void renderText(const string &text, int x, int y)
    {
        SDL_Color color = {0, 200, 255, 255};

        SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), color);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect position = {x, y, surface->w, surface->h};

        SDL_RenderCopy(renderer, texture, nullptr, &position);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    // this function is responsible for updating the game state during each frame
    void update()
    {
        currentTime = SDL_GetTicks();

        if (currentTime >= endTime)
        { // when time is over
            SDL_Delay(3000);
            gameover = true;
            return;
        }

        for (int i = 0; i < bullets.size(); ++i)
        {
            Bullet &bullet = bullets[i];
            if (bullet.active)
            {
                bullet.position.y -= bullet.speed;

                if (bullet.position.y < 0)
                {
                    bullet.active = false;
                }
                else
                {
                    for (int j = 0; j < enemies.size(); ++j)
                    {
                        Enemy &enemy = enemies[j];
                        if (enemy.active && SDL_HasIntersection(&bullet.position, &enemy.position))
                        {
                            bullet.active = false;
                            enemy.active = false;

                            score++;
                            // bullet sound when bullet hits the enemy
                            Mix_PlayChannel(-1, bulletSound, 0);

                            if (score >= 10 && score % 10 == 0)
                            {
                                displayCheckPoint = true;
                            }
                            else
                            {
                                displayCheckPoint = false;
                            }
                        }
                    }

                    for (int j = 0; j < largeEnemies.size(); ++j)
                    {
                        LargeEnemy &largeEnemy = largeEnemies[j];
                        if (largeEnemy.active && SDL_HasIntersection(&bullet.position, &largeEnemy.position))
                        {
                            bullet.active = false;
                            largeEnemy.health--;
                            Mix_PlayChannel(-1, bulletSound, 0);

                            if (largeEnemy.health <= 0)
                            {
                                largeEnemy.active = false;
                                score += 10;
                                displayWoah = true;
                            }
                            else
                            {
                                displayWoah = false;
                            }
                        }
                    }
                }
            }
        }

        for (int i = 0; i < enemies.size(); ++i)
        {
            Enemy &enemy = enemies[i];
            if (enemy.active)
            {
                enemy.position.y += enemy.speed;
                if (enemy.position.y > screen_height)
                {
                    enemy.position.y = -(rand() % 500);
                    enemy.position.x = rand() % (screen_width - enemy.position.w);
                }
            }
        }

        // for large enemy(alien) its occurence is less than small enemies
        for (int i = 0; i < largeEnemies.size(); ++i)
        {
            LargeEnemy &largeEnemy = largeEnemies[i];
            if (largeEnemy.active)
            {
                largeEnemy.position.y += largeEnemy.speed;
                if (largeEnemy.position.y > screen_height)
                {
                    largeEnemy.position.y = -(rand() % 500);
                    largeEnemy.position.x = rand() % (screen_width - largeEnemy.position.w);
                    largeEnemy.health = 3; // Reset health when repositioning
                }
            }
        }
    }

    // this function is responsible for rendering the game elements on the screen.
    void render()
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);

        // rendering the player's texture to the renderer,
        //  displaying the player character on the screen
        SDL_RenderCopy(renderer, player.texture, nullptr, &player.position);

        for (int i = 0; i < bullets.size(); ++i)
        {
            const Bullet &bullet = bullets[i];
            if (bullet.active) // if a bullet is active its texture is rendered to the renderer
                SDL_RenderCopy(renderer, bullet.texture, nullptr, &bullet.position);
        }

        for (int i = 0; i < enemies.size(); ++i)
        {
            const Enemy &enemy = enemies[i];
            if (enemy.active) // same logic as above
                SDL_RenderCopy(renderer, enemy.texture, nullptr, &enemy.position);
        }

        for (int i = 0; i < largeEnemies.size(); ++i)
        {
            const LargeEnemy &largeEnemy = largeEnemies[i];
            if (largeEnemy.active) // same logic as above
                SDL_RenderCopy(renderer, largeEnemy.texture, nullptr, &largeEnemy.position);
        }

        // conditions to display message on screen
        if (gameover)
        {
            SDL_Delay(3000);
            renderText("Game Over!", 70, 70);
        }

        if (displayCheckPoint)
        {
            renderText("CHECK POINT", 330, 200);
        }

        if (displayWoah)
        {
            renderText("GOOD JOB!!", 330, 250);
        }

        renderText("Score: " + to_string(score), 10, 10);
        ifstream file("Score.txt");
        int displayHS;
        file >> displayHS;
        renderText("HighScore: " + to_string(displayHS), 10, 30);
        file.close();

        string timeString = "Time remaining: " + formatTime(endTime - currentTime);
        renderText(timeString, 450, 10);

        SDL_RenderPresent(renderer);
    }

public:
    AstroStrike() : Arcade("Astrostrike", 800, 600), score(0), displayWoah(false), displayCheckPoint(false) {}
    void run()
    {
        if (!initialize())
            return;

        if (!loadMedia())
            return;

        player.position = {screen_width / 2 - 50, screen_height - 100, 100, 100};

        for (int i = 0; i < bullets.size(); ++i)
        {
            Bullet &bullet = bullets[i];
            bullet.position = {0, 0, 20, 50};
            bullet.speed = 5;
        }

        srand(time(nullptr));

        Mix_PlayMusic(backgroundMusic, -1);

        displayWoah = false;       // Initializing display Woah/good job message variaible to false
        displayCheckPoint = false; // Initializing checkPoint to false

        while (true)
        {
            handleEvents();
            update();
            render();

            int activeEnemies = 0;
            for (int i = 0; i < enemies.size(); ++i)
            {
                const Enemy &enemy = enemies[i];
                if (enemy.active)
                    activeEnemies++;
            }

            int activeLargeEnemies = 0;
            for (int i = 0; i < largeEnemies.size(); ++i)
            {
                const LargeEnemy &largeEnemy = largeEnemies[i];
                if (largeEnemy.active)
                    activeLargeEnemies++;
            }

            const int MAX_ENEMIES = 20;
            const int MAX_LARGE_ENEMIES = 2; // large enemies come rarely as compared to small enemies

            if (activeEnemies < MAX_ENEMIES)
            {
                Enemy enemy;
                enemy.texture = loadTexture("images/enemy.png");
                if (!enemy.texture)
                    return;

                enemy.position = {rand() % (screen_width - 50), -(rand() % 500), 50, 50};
                enemy.speed = rand() % 5 + 1;
                enemy.active = true;
                enemies.push_back(enemy);
            }

            if (activeLargeEnemies < MAX_LARGE_ENEMIES)
            {
                LargeEnemy largeEnemy;
                largeEnemy.texture = loadTexture("images/large_enemy.png");
                if (!largeEnemy.texture)
                    return;

                largeEnemy.position = {rand() % (screen_width - 100), -(rand() % 500), 100, 100}; // Adjusting size
                largeEnemy.speed = 1;                                                             // Adjusting speed
                largeEnemy.active = true;
                largeEnemy.health = 3; // Setting initial health
                largeEnemies.push_back(largeEnemy);
            }

            currentTime = SDL_GetTicks();
            if (currentTime >= endTime)
            {
                ifstream file("Score.txt");
                file >> hs;
                file.close();
                if(hs<=score){
                  ofstream file("Score.txt");
                  file << score << endl;
                  file.close();
                }
                displayGameOverMessage();
                SDL_Delay(3000);
                break;
            }
        }

        cleanup();
    }
};