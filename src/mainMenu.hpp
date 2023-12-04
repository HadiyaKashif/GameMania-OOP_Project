#include <fstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include "puzzle.hpp"
#include "pingpong.hpp"
#include "tetris.hpp"
#include "astrostrike.hpp"
#include "spookyChase.hpp"
using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 700;

// Represents a clickable menu option
struct MenuOption
{
    SDL_Rect rect;
    SDL_Texture *texture;

    MenuOption(SDL_Rect rect, SDL_Texture *texture) : rect(rect), texture(texture) {}
};

class MainMenu : public MindMaze, public PingPong, public Tetris, public SpookyChase, public AstroStrike
{
private:
    SDL_Texture *backgroundTexture, *textTexture;
    TTF_Font *font = nullptr;
    SDL_Color fontColor = {255, 255, 255,255};
    SDL_Rect textRect;
    vector<MenuOption *> gameOptions;
    vector<MenuOption *> submenuOptions;
    int currentSubMenu;
    bool gameRunning; // Flag to track if a game is running
    bool quit;
    bool displayText = false;

    bool initialize()
    {
        backgroundTexture = LoadTexture("images/mainBg.png", renderer);
        font = TTF_OpenFont("Oswald-Bold.ttf", 20);
        if (!backgroundTexture || !font)
        {
            cout << "Failed to initialize." << endl;
            return false;
        }
        createGameOptions();
        return true;
    }

    void cleanup()
    {
        SDL_DestroyTexture(backgroundTexture);
        for (MenuOption *option : gameOptions)
        {
            SDL_DestroyTexture(option->texture);
            delete option;
        }
        gameOptions.clear();

        for (MenuOption *option : submenuOptions)
        {
            SDL_DestroyTexture(option->texture);
            delete option;
        }
        submenuOptions.clear();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        SDL_DestroyTexture(textTexture);
    }

    void renderText(const string &text)
    {
        SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), fontColor, SCREEN_WIDTH);
        if (!surface)
        {
            cout << "Failed to create surface for text: " << text << endl;
        }

        if (textTexture)
        {
            SDL_DestroyTexture(textTexture);
            textTexture = nullptr;
        }

        textTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        SDL_QueryTexture(textTexture, nullptr, nullptr, &textRect.w, &textRect.h);
        textRect.x = (SCREEN_WIDTH - textRect.w) / 2;
        textRect.y = (SCREEN_HEIGHT - textRect.h) / 2;

        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    }

    void createGameOptions()
    {
        const char *gameOptionPaths[] = {
            "images/astrostrike.png",
            "images/spookychase.png",
            "images/mindmaze.png",
            "images/pingpong.png",
            "images/tetris.png"};

        const int gameOptionWidth = 150;
        const int gameOptionHeight = 150;
        const int gameOptionPadding = 20;
        const int optionsPerRow = 3;
        const int optionsPerColumn = 2;

        int totalOptions = 5;

        int optionsInFirstLine = totalOptions % optionsPerRow;
        int optionsInSecondLine = optionsPerRow - optionsInFirstLine;

        int totalWidthFirstLine = optionsInFirstLine * gameOptionWidth + (optionsInFirstLine - 1) * gameOptionPadding;
        int totalWidthSecondLine = optionsInSecondLine * gameOptionWidth + (optionsInSecondLine - 1) * gameOptionPadding;

        int startXFirstLine = (SCREEN_WIDTH - totalWidthFirstLine) / 3;
        int startXSecondLine = (SCREEN_WIDTH - totalWidthSecondLine) / 3;

        for (int i = 0; i < totalOptions; ++i)
        {
            int optionRow = i / optionsPerRow;
            int optionCol = i % optionsPerRow;

            int optionX, optionY;
            if (optionRow == 0)
            {
                optionX = startXFirstLine + optionCol * (gameOptionWidth + gameOptionPadding);
                optionY = 300;
            }
            else
            {
                optionX = startXSecondLine + optionCol * (gameOptionWidth + gameOptionPadding);
                optionY = 300 + gameOptionHeight + gameOptionPadding;
            }

            SDL_Rect rect{optionX, optionY, gameOptionWidth, gameOptionHeight};
            SDL_Texture *texture = LoadTexture(gameOptionPaths[i], renderer);
            gameOptions.push_back(new MenuOption(rect, texture));
        }
    }
    void createSubMenuOptions()
    {
        submenuOptions.clear();

        const char *submenuOptionPaths[] = {
            "images/play.png",
            "images/how to play.png",
            "images/back.png"};

        const int submenuOptionWidth = 200;
        const int submenuOptionHeight = 150;
        const int submenuOptionPadding = 30;

        for (int i = 0; i < 4; ++i)
        {
            int optionX = 100 + i * (submenuOptionWidth + submenuOptionPadding);
            SDL_Rect rect{optionX, 400, submenuOptionWidth, submenuOptionHeight};
            SDL_Texture *texture = LoadTexture(submenuOptionPaths[i], renderer);
            submenuOptions.push_back(new MenuOption(rect, texture));
        }
    }

    void renderMainMenu()
    {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);

        for (MenuOption *option : gameOptions)
        {
            SDL_RenderCopy(renderer, option->texture, nullptr, &option->rect);
        }

        SDL_RenderPresent(renderer);
    }
    void renderSubMenu()
    {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
        if (!displayText)
        {
            for (MenuOption *option : submenuOptions)
            {
                SDL_RenderCopy(renderer, option->texture, nullptr, &option->rect);
            }
        }
        else
        {
            SDL_Texture *imageTexture = LoadTexture("images/back.png", renderer);
            SDL_Rect imageRect = {600, 580, 200, 150};
            if (imageTexture)
            {
                SDL_RenderCopy(renderer, imageTexture, nullptr, &imageRect);
            }
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    for (int i = 0; i < gameOptions.size(); ++i)
                    {
                        SDL_Point tempPoint = {x, y};
                        if (SDL_PointInRect(&tempPoint, &imageRect))
                        {
                            displayText = false; // Reset the flag
                            createSubMenuOptions();
                        }
                    }
                }
            }
        }
        SDL_RenderPresent(renderer);
    }
    void handleMainMenuEvents(SDL_Event &event)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);

                for (int i = 0; i < gameOptions.size(); ++i)
                {
                    SDL_Point tempPoint = {x, y};
                    if (SDL_PointInRect(&tempPoint, &gameOptions[i]->rect))
                    {
                        currentSubMenu = i + 1;
                        createSubMenuOptions();
                        break;
                    }
                }
            }
        }
    }

    void handleSubMenuEvents(SDL_Event &event)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);

                for (int i = 0; i < submenuOptions.size(); ++i)
                {
                    SDL_Point tempPoint = {x, y};
                    if (SDL_PointInRect(&tempPoint, &submenuOptions[i]->rect))
                    {
                        if (i == 0) // "Instructions" option clicked
                        {
                            handleEvents();
                        }
                        else if (i == 1) // "Instructions" option clicked
                        {
                            displayText = true;               // Set the flag to true
                            handleSubMenuInstructionsClick(); // Render the text
                        }
                        else if (i == 2)
                        {
                            currentSubMenu = 0; // Go back to the main menu
                        }
                        break;
                    }
                }
            }
        }
    }

    void handleEvents()
    {
        if (currentSubMenu == 1)
        {
            AstroStrike astrostrike;
            astrostrike.run();
            gameRunning = true;
        }
        else if (currentSubMenu == 2)
        {
            SpookyChase spook;
            spook.run();
            gameRunning = true;
        }
        else if (currentSubMenu == 3)
        {
            MindMaze puzzle;
            puzzle.run();
            gameRunning = true; // Set the gameRunning flag to true
        }
        else if (currentSubMenu == 4)
        {
            PingPong game;
            game.run();
            gameRunning = true; // Set the gameRunning flag to true
        }
        else if (currentSubMenu == 5)
        {
            Tetris tetris;
            tetris.run();
            gameRunning = true; // Set the gameRunning flag to true
        }
    }
    void handleSubMenuInstructionsClick()
    {
        string fileName;
        if (currentSubMenu == 1)
        {
            fileName = "textFiles/astrostrike.txt";
        }
        else if (currentSubMenu == 2)
        {
            fileName = "textFiles/Spook.txt";
        }
        else if (currentSubMenu == 3)
        {
            fileName = "textFiles/mindmaze.txt";
        }
        else if (currentSubMenu == 4)
        {
            fileName = "textFiles/pingpong.txt";
        }
        else if (currentSubMenu == 5)
        {
            fileName = "textFiles/tetris.txt";
        }

        ifstream file(fileName);
        if (file.is_open())
        {
            string text;
            string line;
            while (getline(file, line))
            {
                text += line + "\n"; // Add bullet point and line break
            }
            file.close();
            renderText(text);
        }
        else
        {
            cout << "Failed to open file: " << fileName << endl;
        }
    }
    SDL_Texture *LoadTexture(const char *filename, SDL_Renderer *renderer)
    {
        SDL_Surface *surface = IMG_Load(filename);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        return texture;
    }

public:
    MainMenu() : Arcade("MainMenu", SCREEN_WIDTH, SCREEN_HEIGHT), currentSubMenu(0), gameRunning(false), quit(false)
    {
        createGameOptions();
    }
    void run()
    {
        bool running = initialize();
        if (!running)
        {
            cout << "Failed to initialize the game." << endl;
            return;
        }
        SDL_Event event;
        while (!quit)
        {
            SDL_RenderClear(renderer);
            if (currentSubMenu == 0)
            {
                renderMainMenu();
                handleMainMenuEvents(event);
            }
            else
            {
                renderSubMenu();
                handleSubMenuEvents(event);
            }
        }
        cleanup();
    }
};