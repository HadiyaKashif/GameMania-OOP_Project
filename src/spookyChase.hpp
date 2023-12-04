#include "abstract.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <SDL2\SDL.h>
#include <SDL2\SDL_image.h>
#include <SDL2\SDL_ttf.h>
#include <SDL2\SDL_mixer.h>
#include <SDL2\SDL_scancode.h>
#include <random>
#include <cmath>
using namespace std;

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 700;
const int GRIM_WIDTH = 150;
const int GRIM_HEIGHT = 150;
const int OBSTACLE_WIDTH = 110;
const int OBSTACLE_HEIGHT = 110;
const int COLLECTIBLE_WIDTH = 110;
const int COLLECTIBLE_HEIGHT = 110;
const int MAX_LIVES = 3;
const int POINTS_PER_COLLECTIBLE = 5;
const int WINNING_POINTS = 100;
const int NUM_POWERUPS = 3;
const int POWERUP_SPAWN_DISTANCE = 300;

class SpookyChase : virtual public Arcade
{
private:
    SDL_Texture *backgroundTexture;
    Mix_Chunk *collectSound;
    Mix_Chunk *collisionSound;
    Mix_Chunk *powerUpSound;
    SDL_Rect backgroundRect;
    bool quit;
    struct Grim
    {
        int x;
        int y;
        int velocity;
        SDL_Texture *texture;
    };

    struct Obstacle
    {
        int x;
        int y;
        int velocity;
        SDL_Texture *texture;
    };

    struct PowerUp
    {
        int x;
        int y;
        string type;
        bool collected;
        int duration; // Duration in frames
        int timer;    // Timer for tracking the duration
        SDL_Texture *texture;
    };

    struct Collectible
    {
        int x;
        int y;
        float angle;
        float radius;
        bool collected;
        int velocity;
        SDL_Texture *texture;
    };
    bool initialize()
    {
        backgroundTexture = loadTexture("images/backgroundSpook.jpg");
        if (!backgroundTexture)
        {
            cleanup();
            return false;
        }

        backgroundRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, &backgroundRect);
        // Load music and sound effects
        backgroundMusic = Mix_LoadMUS("sound/horrorBg.mp3");
        collectSound = Mix_LoadWAV("sound/points.wav");
        collisionSound = Mix_LoadWAV("sound/collision.wav");
        powerUpSound = Mix_LoadWAV("sound/powerupSound.wav");
        if (!backgroundMusic || !collectSound || !collisionSound || !powerUpSound)
        {
            cout << "Failed to load audio files: " << Mix_GetError() << endl;
            return false;
        }

        return true;
    }

    void cleanup()
    {
        Mix_FreeChunk(collectSound);
        Mix_FreeChunk(collisionSound);
        Mix_FreeChunk(powerUpSound);
    }

    void renderText(const string &text, int x, int y, const SDL_Color &color)
    {
        SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), color);
        if (!surface)
        {
            cout << "Failed to render text surface: " << TTF_GetError() << endl;
            return;
        }

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture)
        {
            cout << "Failed to create text texture: " << SDL_GetError() << endl;
            return;
        }

        int textWidth = 0;
        int textHeight = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, &textWidth, &textHeight);

        SDL_Rect dstRect = {x, y, textWidth, textHeight};
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }

    SDL_Texture *loadTexture(const string &fileName)
    {
        SDL_Surface *surface = IMG_Load(fileName.c_str());
        if (!surface)
        {
            cout << "Failed to load image: " << IMG_GetError() << endl;
            return nullptr;
        }

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture)
        {
            cout << "Failed to create texture: " << SDL_GetError() << endl;
            return nullptr;
        }

        return texture;
    }
    void handleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            {
                quit = true; // Set the quit member variable to true when Escape key is pressed
            }
            // Play sound effects for specific events
            if (event.type == SDL_USEREVENT)
            {
                if (event.user.code == 0) // Collect sound event
                {
                    Mix_PlayChannel(-1, collectSound, 0);
                }
                else if (event.user.code == 1) // Collision sound event
                {
                    Mix_PlayChannel(-1, collisionSound, 0);
                }
                else if (event.user.code == 2) // Power-up sound event
                {
                    Mix_PlayChannel(-1, powerUpSound, 0);
                }
            }
        }
    }

    void updateGrim(Grim &grim)
    {
        // updates the position of the Grim character based on keyboard input.
        // It checks the state of keyboard keys using SDL_GetKeyboardState and moves the character accordingly.
        // The Grim character cannot move outside the game window boundaries.

        const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);

        if (currentKeyStates[SDL_SCANCODE_LEFT])
        {
            // subtracts the velocity value from the x coordinate of the grim character,
            //  causing it to move left.Additionally, it checks if the x coordinate goes below 0

            //  (the left boundary of the game window) and,
            // if so, resets it to 0 to prevent the character from moving outside the window.

            grim.x -= grim.velocity;
            if (grim.x < 0)
            {
                grim.x = 0;
            }
        }
        else if (currentKeyStates[SDL_SCANCODE_RIGHT])
        {
            // it adds the velocity value to the x coordinate of the grim character,
            // causing it to move right.Additionally, it checks if the x coordinate exceeds the window width
            // minus the width of the Grim character(GRIM_WIDTH).If it does, it resets the x coordinate to the maximum allowed value,
            //  preventing the character from moving outside the window on the right side.

            grim.x += grim.velocity;
            if (grim.x > WINDOW_WIDTH - GRIM_WIDTH)
            {
                grim.x = WINDOW_WIDTH - GRIM_WIDTH;
            }
        }

        if (currentKeyStates[SDL_SCANCODE_UP])
        {
            // it subtracts the velocity value from the y coordinate of the grim character,
            //  causing it to move up.Additionally, it checks if the y coordinate goes below 0(the top boundary of the game window) and,
            // if so, resets it to 0 to prevent the character from moving outside the window.
            grim.y -= grim.velocity;
            if (grim.y < 0)
            {
                grim.y = 0;
            }
        }
        else if (currentKeyStates[SDL_SCANCODE_DOWN])
        {
            // it adds the velocity value to the y coordinate of the grim character, causing it to move down.Additionally,
            // it checks if the y coordinate exceeds the window height minus the height of the Grim character(GRIM_HEIGHT).
            // If it does, it resets the y coordinate to the maximum allowed value,
            //  preventing the character from moving outside the window on the bottom side.
            grim.y += grim.velocity;
            if (grim.y > WINDOW_HEIGHT - GRIM_HEIGHT)
            {
                grim.y = WINDOW_HEIGHT - GRIM_HEIGHT;
            }
        }
    }

    void updateCollectibles(vector<Collectible> &collectibles, const Grim &grim, int &points)
    {
        for (Collectible &collectible : collectibles)
        {
            if (!collectible.collected)
            {
                if (collectible.angle >= 360.0f)
                {
                    collectible.angle = 0.0f;
                }

                // Calculate the new position of the collectible based on the circular animation
                collectible.x = collectible.x + static_cast<int>(collectible.radius * cos(collectible.angle * M_PI / 180.0f));
                collectible.y = collectible.y + static_cast<int>(collectible.radius * sin(collectible.angle * M_PI / 180.0f));

                // Check for collision with the grim
                if (collectible.y + COLLECTIBLE_HEIGHT >= grim.y &&
                    collectible.y <= grim.y + GRIM_HEIGHT &&
                    collectible.x + COLLECTIBLE_WIDTH >= grim.x &&
                    collectible.x <= grim.x + GRIM_WIDTH)
                {
                    collectible.collected = true;
                    points += POINTS_PER_COLLECTIBLE;

                    SDL_Event soundEvent;
                    soundEvent.type = SDL_USEREVENT;
                    soundEvent.user.code = 0;
                    SDL_PushEvent(&soundEvent);

                    // Move the collectible to a new random position on the screen
                    collectible.x = rand() % (WINDOW_WIDTH - COLLECTIBLE_WIDTH);
                    collectible.y = rand() % (WINDOW_HEIGHT - COLLECTIBLE_HEIGHT);
                    collectible.collected = false;
                }

                // Update the angle for the circular animation
                collectible.angle += static_cast<float>(collectible.velocity);

                // Render the collectible on the screen
                SDL_Rect collectibleRect = {collectible.x, collectible.y, COLLECTIBLE_WIDTH, COLLECTIBLE_HEIGHT};
                SDL_RenderCopy(renderer, collectible.texture, nullptr, &collectibleRect);
            }
        }
    }

    void updateObstacles(vector<Obstacle> &obstacles, Grim &grim, int &lives)
    {
        for (Obstacle &obstacle : obstacles)
        {
            obstacle.y += obstacle.velocity;

            // Check for collision with the grim
            if (obstacle.y + OBSTACLE_HEIGHT >= grim.y &&
                obstacle.y <= grim.y + GRIM_HEIGHT &&
                obstacle.x + OBSTACLE_WIDTH >= grim.x &&
                obstacle.x <= grim.x + GRIM_WIDTH)
            {
                lives--; // Decrement lives on collision with an obstacle

                // Reset the position of the obstacle
                obstacle.x = rand() % (WINDOW_WIDTH - OBSTACLE_WIDTH);
                obstacle.y = -(rand() % 1000 + 100);
                obstacle.velocity = rand() % 5 + 1;

                SDL_Event soundEvent;
                soundEvent.type = SDL_USEREVENT;
                soundEvent.user.code = 1;
                SDL_PushEvent(&soundEvent);
            }

            // Reset the position of the obstacle when it goes off the screen
            if (obstacle.y > WINDOW_HEIGHT)
            {
                obstacle.x = rand() % (WINDOW_WIDTH - OBSTACLE_WIDTH);
                obstacle.y = -(rand() % 1000 + 100);
                obstacle.velocity = rand() % 5 + 1;
            }
        }
    }

    void updatePowerUps(vector<PowerUp> &powerUps, Grim &grim, int &lives, vector<Obstacle> &obstacles)
    {
        for (PowerUp &powerUp : powerUps)
        {
            if (!powerUp.collected)
            {
                powerUp.y += 3;

                // Check for collision with the grim
                if (powerUp.y + COLLECTIBLE_HEIGHT >= grim.y &&
                    powerUp.y <= grim.y + GRIM_HEIGHT &&
                    powerUp.x + COLLECTIBLE_WIDTH >= grim.x &&
                    powerUp.x <= grim.x + GRIM_WIDTH)
                {
                    powerUp.collected = true;
                    powerUp.timer = powerUp.duration; // Start the timer for the power-up

                    // Dispatch power-up sound event
                    SDL_Event soundEvent;
                    soundEvent.type = SDL_USEREVENT;
                    soundEvent.user.code = 2;
                    SDL_PushEvent(&soundEvent);

                    if (powerUp.type == "SpeedBoost")
                    {
                        grim.velocity *= 2; // Double the grim's velocity
                    }
                    else if (powerUp.type == "Invincibility")
                    {
                        lives = MAX_LIVES; // Set lives to the maximum value
                    }
                }

                SDL_Rect powerUpRect = {powerUp.x, powerUp.y, COLLECTIBLE_WIDTH, COLLECTIBLE_HEIGHT};
                SDL_RenderCopy(renderer, powerUp.texture, nullptr, &powerUpRect);
            }
            else
            {
                // Update the timer for the power-up
                if (powerUp.timer > 0)
                {
                    powerUp.timer--;
                }
                else
                {
                    powerUp.collected = false; // Reset the power-up after the duration is over

                    // Revert the effects of the power-up
                    if (powerUp.type == "SpeedBoost")
                    {
                        grim.velocity /= 2; // Restore the original grim velocity
                    }
                    else if (powerUp.type == "Invincibility")
                    {
                        // No action needed since lives were set to the maximum immediately
                    }
                }
            }
        }
    }
    void renderPowerUpType(SDL_Renderer *renderer, TTF_Font *font, const string &powerUpType, int windowWidth, int windowHeight)
    {
        SDL_Color textColor = {255, 0, 0}; // Red color
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, powerUpType.c_str(), textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        int textWidth = textSurface->w;
        int textHeight = textSurface->h;

        SDL_Rect textRect;
        textRect.x = (windowWidth - textWidth) / 2;   // Center the text horizontally
        textRect.y = (windowHeight - textHeight) / 2; // Center the text vertically
        textRect.w = textWidth;
        textRect.h = textHeight;

        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    void spawnPowerUp(vector<PowerUp> &powerUps)
    {
        if (powerUps.size() < NUM_POWERUPS)
        {
            PowerUp powerUp;
            powerUp.x = rand() % (WINDOW_WIDTH - COLLECTIBLE_WIDTH);
            powerUp.y = -COLLECTIBLE_HEIGHT;
            powerUp.collected = false;
            powerUp.duration = 300; // Duration of 300 frames
            powerUp.timer = 0;
            powerUp.texture = IMG_LoadTexture(renderer, "images/powerup.png");

            // Randomly choose a power-up type
            srand(time(NULL));
            int powerUpType = rand() % 3;
            if (powerUpType == 0)
            {
                powerUp.type = "SpeedBoost";
            }
            else if (powerUpType == 1)
            {
                powerUp.type = "Invincibility";
            }
            powerUps.push_back(powerUp);
        }
    }

public:
    SpookyChase() : Arcade("SpookyChase", WINDOW_WIDTH, WINDOW_HEIGHT) {}
    void run()
    {
        if (!initialize())
        {
            cleanup();
            return;
        }
        srand(static_cast<unsigned int>(time(nullptr)));
        font = TTF_OpenFont("spooky.ttf", 44);

        Grim grim;
        grim.x = WINDOW_WIDTH / 2 - GRIM_WIDTH / 2;
        grim.y = WINDOW_HEIGHT - GRIM_HEIGHT - 10;
        grim.velocity = 5;
        grim.texture = loadTexture("images/grimSpook.png");
        if (!grim.texture)
        {
            cleanup();
            return;
        }

        vector<Obstacle> obstacles;
        vector<Collectible> collectibles;
        vector<PowerUp> powerUps;

        int lives = MAX_LIVES;
        int points = 0;

        // Play background music on loop
        Mix_PlayMusic(backgroundMusic, -1);

        quit = false;
        SDL_Event event;
        int frameCount = 0;
        const int SPAWN_INTERVAL = 200; // Adjust as needed 
        int powerUpTimer = 0;

        while (!quit)
        {
            handleEvents();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            backgroundRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            SDL_RenderCopy(renderer, backgroundTexture, nullptr, &backgroundRect);

            if (lives > 0 && points < WINNING_POINTS)
            {
                if (obstacles.empty())
                {
                    Obstacle obstacle;
                    obstacle.x = rand() % (WINDOW_WIDTH - OBSTACLE_WIDTH);
                    obstacle.y = -OBSTACLE_HEIGHT;
                    obstacle.velocity = rand() % 3 + 1;
                    obstacle.texture = loadTexture("images/ghosts.png");
                    if (!obstacle.texture)
                    {
                        cleanup();
                        return;
                    }
                    obstacles.push_back(obstacle);
                }
                if (collectibles.empty())
                {
                    Collectible collectible;
                    collectible.x = rand() % (WINDOW_WIDTH - COLLECTIBLE_WIDTH);
                    collectible.y = rand() % (WINDOW_HEIGHT - COLLECTIBLE_HEIGHT);
                    collectible.collected = false;
                    collectible.velocity = rand() % 4 + 2; // Assign a random velocity
                    collectible.texture = loadTexture("images/collectible.png");
                    if (!collectible.texture)
                    {
                        cleanup();
                        return;
                    }
                    collectibles.push_back(collectible);
                }

                if (powerUps.empty())
                {
                    PowerUp powerUp;
                    powerUp.x = rand() % (WINDOW_WIDTH - COLLECTIBLE_WIDTH);
                    powerUp.y = -COLLECTIBLE_HEIGHT;
                    powerUp.collected = false;
                    powerUp.duration = 300; // Duration of 300 frames 
                    powerUp.timer = 0;
                    powerUp.texture = IMG_LoadTexture(renderer, "images/powerup.png");

                    // Randomly choose a power-up type
                    int powerUpType = rand() % 3;
                    if (powerUpType == 0)
                    {
                        powerUp.type = "SpeedBoost";
                    }
                    else if (powerUpType == 1)
                    {
                        powerUp.type = "Invincibility";
                    }
                    powerUps.push_back(powerUp);
                }

                for (Obstacle &obstacle : obstacles)
                {
                    obstacle.y += obstacle.velocity;
                    if (obstacle.y > WINDOW_HEIGHT)
                    {
                        obstacles.clear();
                    }

                    SDL_Rect obstacleRect = {obstacle.x, obstacle.y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};
                    SDL_RenderCopy(renderer, obstacle.texture, nullptr, &obstacleRect);
                }

                for (Collectible &collectible : collectibles)
                {
                    collectible.y += collectible.velocity;

                    // Reset the position of the collectible when it goes off the screen
                    if (collectible.y > WINDOW_HEIGHT)
                    {
                        collectible.x = rand() % (WINDOW_WIDTH - COLLECTIBLE_WIDTH);
                        collectible.y = -(rand() % 1000 + 100);
                        collectible.collected = false;
                    }

                    if (!collectible.collected)
                    {
                        SDL_Rect collectibleRect = {collectible.x, collectible.y, COLLECTIBLE_WIDTH, COLLECTIBLE_HEIGHT};
                        SDL_RenderCopy(renderer, collectible.texture, nullptr, &collectibleRect);
                    }
                }

                updateGrim(grim);
                updateObstacles(obstacles, grim, lives);
                updateCollectibles(collectibles, grim, points);

                // updatePowerUps(powerUps, grim, lives, obstacles);
                updatePowerUps(powerUps, grim, lives, obstacles);

                // Periodically spawn power-ups randomly
                if (frameCount % SPAWN_INTERVAL == 0)
                {
                    spawnPowerUp(powerUps);
                }

                renderText("Lives: " + to_string(lives), 10, 10, {255, 255, 0, 255});
                renderText("Points: " + to_string(points), WINDOW_WIDTH - 140, 10, {255, 255, 0, 255});

                SDL_Rect carRect = {grim.x, grim.y, GRIM_WIDTH, GRIM_HEIGHT};
                SDL_RenderCopy(renderer, grim.texture, nullptr, &carRect);

                frameCount++;
            }
            else
            {
                string message;
                SDL_Color color;

                if (lives <= 0)
                {
                    message = "YOU LOST!";
                    color = {0, 192, 192, 192};
                }
                else
                {
                    message = "YOU WON!";
                    color = {0, 128, 192, 255};
                }

                renderText(message, WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 20, color);
                renderText("Final Points: " + to_string(points), WINDOW_WIDTH / 2 - 70, WINDOW_HEIGHT / 2 + 20, color);
            }
            // Clear the power-ups vector when the game ends
            if (lives == 0 || points == WINNING_POINTS)
            {
                powerUps.clear();
            }
            // Render the power-up type on the game window
            bool anyPowerUpCollected = false;
            for (const PowerUp &powerUp : powerUps)
            {
                if (powerUp.collected)
                {
                    anyPowerUpCollected = true;
                    break;
                }
            }

            if (anyPowerUpCollected)
            {
                string powerUpTypeText;
                if (powerUps[0].type == "SpeedBoost")
                {
                    powerUpTypeText = "Speed Boost";
                }
                else if (powerUps[0].type == "Invincibility")
                {
                    powerUpTypeText = "Invincibility";
                }
                renderPowerUpType(renderer, font, powerUpTypeText, WINDOW_WIDTH, WINDOW_HEIGHT);
            }

            SDL_RenderPresent(renderer);
        }

        for (PowerUp &powerUp : powerUps)
        {
            SDL_DestroyTexture(powerUp.texture);
        }

        cleanup();
    }
};