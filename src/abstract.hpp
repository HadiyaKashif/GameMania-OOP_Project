#ifndef ARCADE_H
#define ARCADE_H
// Above are preprocessor directives that guard against multiple inclusion of the same header file.

#include <string>
using namespace std;
class Arcade
{
public:
    Arcade(const char *n = "", int w = 700, int h = 700) : gameName(n), Width(w), Height(h), window(nullptr), renderer(nullptr)
    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // SDL_Init initializes the SDL. The parameter specifies what part(s)/subsystems of SDL to initialize.

        window = SDL_CreateWindow(gameName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_SHOWN);
        // SDL_CreateWindow creates a window for the game with the title "PONG" and dimensions specified by WIDTH and HEIGHT constants.

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        // SDL_CreateRenderer creates a hardware-accelerated renderer associated with the game window.
        IMG_Init(IMG_INIT_PNG);                            // This initializes the SDL_image extension with support for PNG image loading.
        IMG_Init(IMG_INIT_JPG);                            // This initializes the SDL_image extension with support for PNG image loading.
        TTF_Init();                                        // TTF_Init() initializes SDL_ttf which allows to load and render TrueType fonts in SDL applications.
        Mix_Init(MIX_INIT_MP3);                            // This line initializes specific components of the SDL_mixer library for handling MP3 audio format.
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); // This initializes the SDL_mixer extension for audio mixing. It specifies the audio format and parameters
    }
    ~Arcade()
    {
        TTF_CloseFont(font);
        TTF_CloseFont(msgfont);
        Mix_FreeMusic(backgroundMusic);
        Mix_CloseAudio();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
    }
    virtual void run() = 0;
    virtual bool initialize() = 0;
    virtual void handleEvents() = 0;
    virtual void cleanup() = 0;

protected:
    const char *gameName;       // this will store the name for each window
    SDL_Window *window;         // SDL_Window is a structure in the SDL library that represents a window or a graphical windowing element in a graphical user interface.
                                // It acts as a container or an area on the screen where you can display your graphics, render images, and receive input events.
    SDL_Renderer *renderer;     // SDL_Renderer is a structure in the SDL library that represents a rendering context.It allows to perform various rendering operations
    Mix_Music *backgroundMusic; // Mix_Music is a structure that represents a piece of music, something that can be played for an extended period of time, usually repeated.
    // backgroundMusic is a pointer used to store the background music for the game.
    TTF_Font *font = NULL, *msgfont = NULL; // TTF_Font represents a font object that can be used for rendering TrueType fonts in SDL applications.
                                            // It is a structure that encapsulates the necessary data and settings to handle font rendering operations.
    int Width, Height;                      // this will control the width and height of window
};

#endif // concluding preprocessor directive
