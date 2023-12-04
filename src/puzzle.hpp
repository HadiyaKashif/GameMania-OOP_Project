#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <algorithm>
#include <random>
#include <string>
#include "abstract.hpp"
using namespace std;

const int IMAGE_WIDTH = 500;                    // setting the image width
const int IMAGE_HEIGHT = 500;                   // setting the image height
const int GRID_SIZE = 4;                        // setting the puzzle grid size
const int NUM_PIECES = GRID_SIZE * GRID_SIZE;   // setting the number of pieces the grid will have
const int PIECE_SIZE = IMAGE_WIDTH / GRID_SIZE; // setting the size of each piece
const int GAME_DURATION = 100;                  // 2 minutes in seconds

class MindMaze : virtual public Arcade
{
private:
    SDL_Texture *backgroundTexture;
    SDL_Texture *texture;
    SDL_Color textColor, msgColor;
    int grid[GRID_SIZE][GRID_SIZE]; // setting a 2D array, which will be used as puzzle grid
    SDL_Rect pieces[NUM_PIECES];
    random_device rd;  //it is a random number generator (RNG) provided by the C++ standard library
    mt19937 rng;  //mt19937 is a pseudo-random number generator (PRNG) from the C++ standard library's <random> header
    bool running, puzzleSolved;
    Uint32 startTime, endTime, currentTime;
    SDL_Event e;

    bool initialize()
    {
        SDL_RenderClear(renderer);
        // Load the background image
        SDL_Surface *backgroundSurface = IMG_Load("images/bg.png");
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);

        texture = IMG_LoadTexture(renderer, "images/grid.PNG");
        int pieceIndex = 0;
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                pieces[pieceIndex].x = j * PIECE_SIZE;
                pieces[pieceIndex].y = i * PIECE_SIZE;
                pieces[pieceIndex].w = PIECE_SIZE;
                pieces[pieceIndex].h = PIECE_SIZE;
                grid[i][j] = pieceIndex;
                pieceIndex++;
            }
        }

        // Shuffle the positions of the pieces randomly
        rng = mt19937(rd());
        shuffle(&grid[0][0], &grid[GRID_SIZE - 1][GRID_SIZE - 1] + 1, rng);
        // Variables for timer
        startTime = SDL_GetTicks();
        endTime = startTime + (GAME_DURATION * 1000); // Convert to milliseconds
        font = TTF_OpenFont("Oswald-Bold.ttf", 40);
        textColor = {0, 0, 0, 255};
        msgfont = TTF_OpenFont("Oswald-Bold.ttf", 100);
        msgColor = {0, 0, 0};
        backgroundMusic = Mix_LoadMUS("sound/puzzle.mp3");
        if (backgroundTexture == nullptr || backgroundMusic == nullptr || font == nullptr || msgfont == nullptr)
        {
            cout << "Failed to initialize" << endl;
            return false;
        }
        // Play the background music
        Mix_PlayMusic(backgroundMusic, -1);
        return true;
    }
    void cleanup()
    {
        SDL_DestroyTexture(texture);
        SDL_DestroyTexture(backgroundTexture);
    }

    void handleEvents()
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;

            if (e.type == SDL_KEYDOWN)
            {
                int emptyPieceX = 0;
                int emptyPieceY = 0;
                bool foundEmptyPiece = false;

                // Find empty piece
                for (int i = 0; i < GRID_SIZE; i++)
                {
                    for (int j = 0; j < GRID_SIZE; j++)
                    {
                        if (grid[i][j] == NUM_PIECES - 1)
                        {
                            emptyPieceX = j;
                            emptyPieceY = i;
                            foundEmptyPiece = true;
                            break;
                        }
                    }
                    if (foundEmptyPiece)
                        break;
                }

                int clickedPieceX = emptyPieceX;
                int clickedPieceY = emptyPieceY;

                switch (e.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                case SDLK_UP:
                    clickedPieceY++;
                    break;
                case SDLK_DOWN:
                    clickedPieceY--;
                    break;
                case SDLK_LEFT:
                    if (emptyPieceX == GRID_SIZE - 1)
                    {
                        if (emptyPieceY == 0)
                        {
                            swap(grid[1][0], grid[emptyPieceY][emptyPieceX]);
                        }
                        else if (emptyPieceY == 1)
                        {
                            swap(grid[2][0], grid[emptyPieceY][emptyPieceX]);
                        }
                        else if (emptyPieceY == 2)
                        {
                            swap(grid[3][0], grid[emptyPieceY][emptyPieceX]);
                        }
                    }
                    else
                    {
                        clickedPieceX++;
                    }
                    break;
                case SDLK_RIGHT:
                    if (emptyPieceX == 0)
                    {
                        if (emptyPieceY == 1)
                        {
                            swap(grid[0][3], grid[emptyPieceY][emptyPieceX]);
                        }
                        else if (emptyPieceY == 2)
                        {
                            swap(grid[1][3], grid[emptyPieceY][emptyPieceX]);
                        }
                        else if (emptyPieceY == 3)
                        {
                            swap(grid[2][3], grid[emptyPieceY][emptyPieceX]);
                        }
                    }
                    else
                    {
                        clickedPieceX--;
                    }
                    break;
                default:
                    break;
                }

                // Check if the clicked piece is adjacent to the empty piece
                if (((clickedPieceX == emptyPieceX && abs(clickedPieceY - emptyPieceY) == 1) ||
                     (clickedPieceY == emptyPieceY && abs(clickedPieceX - emptyPieceX) == 1)) &&
                    clickedPieceX >= 0 && clickedPieceX < GRID_SIZE && clickedPieceY >= 0 && clickedPieceY < GRID_SIZE)
                {
                    // Swap positions in the grid
                    swap(grid[clickedPieceY][clickedPieceX], grid[emptyPieceY][emptyPieceX]);
                }
            }
        }
    }

    bool isPuzzleSolved()
    {  //checking if all the puzzle pieces are in correct positions
        int expectedValue = 0;
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                if (grid[i][j] != expectedValue)
                {
                    return false;
                }
                expectedValue++;
            }
        }
        return true;
    }
    string formatTime(Uint32 milliseconds)
    {  //converting the time to a string
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

    void render()
    {
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);

        for (int i = 0; i < NUM_PIECES; i++)
        {
            int pieceX = i % GRID_SIZE;
            int pieceY = i / GRID_SIZE;

            SDL_Rect destRect;

            destRect.x = pieceX * PIECE_SIZE + 100;
            destRect.y = pieceY * PIECE_SIZE + 100;
            destRect.w = PIECE_SIZE;
            destRect.h = PIECE_SIZE;

            SDL_RenderCopy(renderer, texture, &pieces[grid[pieceY][pieceX]], &destRect);
        }
        // Render the timer
        currentTime = SDL_GetTicks();
        Uint32 remainingTime = endTime > currentTime ? endTime - currentTime : 0;
        string timeStr = formatTime(remainingTime);
        SDL_Surface *timeSurface = TTF_RenderText_Solid(font, timeStr.c_str(), textColor);
        SDL_Texture *timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
        SDL_Rect timeRect;
        timeRect.x = Width - timeSurface->w - 110; // Adjust the position as needed
        timeRect.y = 10;                           // Adjust the position as needed
        timeRect.w = timeSurface->w;
        timeRect.h = timeSurface->h;
        SDL_RenderCopy(renderer, timeTexture, nullptr, &timeRect);
        SDL_FreeSurface(timeSurface);
        SDL_DestroyTexture(timeTexture);

        SDL_RenderPresent(renderer);
    }

    void displayWonMessage()
    {  //displaying won message on screen

        SDL_Surface *surface = TTF_RenderText_Solid(msgfont, "You won!", msgColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect textRect;
        textRect.x = (Width - surface->w) / 2;
        textRect.y = (Height - surface->h) / 2;
        textRect.w = surface->w;
        textRect.h = surface->h;
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(textTexture);

        // Update the renderer
        SDL_RenderPresent(renderer);
    }
    void delay()
    {
        Uint32 delayStartTime = SDL_GetTicks();
        while (SDL_GetTicks() - delayStartTime < 2000) // Delay for 2 seconds
        {
            // Keep the event loop running to process events
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT)
                    running = false;
            }
        }
        running = false; // Stop the game
    }
    void displayGameOverMessage()
    {
        SDL_Surface *surface = TTF_RenderText_Solid(msgfont, "Game Over", msgColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect textRect;
        textRect.x = (Width - surface->w) / 2;
        textRect.y = (Height - surface->h) / 2;
        textRect.w = surface->w;
        textRect.h = surface->h;
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(textTexture);

        // Update the renderer
        SDL_RenderPresent(renderer);
    }
    void update()
    {
        puzzleSolved = isPuzzleSolved();
        if (puzzleSolved)
        {
            displayWonMessage();
            delay();
        }
        else if (currentTime >= endTime)
        {
            displayGameOverMessage();
            delay();
        }
    }

public:
    MindMaze() : Arcade("MindMaze"), running(true), puzzleSolved(false) {}
    void run()
    {  //method controlling the whole game
        initialize();
        while (running)
        {
            handleEvents();
            update();
            render();
        }
        cleanup();
    }
};