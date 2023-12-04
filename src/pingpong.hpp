#include "abstract.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <string>

using namespace std;

const int BALL_SIZE = 35;
const int BALL_SPEED = 4;
const int PADDLE_WIDTH = 30;
const int PADDLE_HEIGHT = 150;
const int PADDLE_SPEED = 7;
const string BALL_IMAGE_PATH = "images/ball.png";     // providing the path for rendering ball image
const string PADDLE_IMAGE_PATH = "images/paddle.png"; // providing the path for rendering ball image
const int SCORE_X_OFFSET = 20;
const int SCORE_Y_OFFSET = 20;
// SCORE_X_OFFSET and SCORE_Y_OFFSET are specifying the distance from edges of window where the score text should be displayed
const int maxScore = 10; // sets the maximum score

SDL_Color textColor = {0, 192, 249, 255}; // SDL_Color is a structure that represents a color using RGB (Red, Green, Blue) components.

class PingPong : virtual public Arcade
{
public:
    PingPong() : Arcade("PingPong"), ballTexture(nullptr), ballVelX(BALL_SPEED), ballVelY(BALL_SPEED), lScore(0), rScore(0), running(false), scoreTexture(nullptr), paddleHitSound(nullptr) {}
    // default constructor
    void run() // controls the running of the game
    {
        running = initialize();
        if (!running)
        {
            cout << "Failed to initialize the game." << endl;
            return;
        }

        loadMedia();

        ball.x = Width / 2 - BALL_SIZE / 2;
        ball.y = Height / 2 - BALL_SIZE / 2;
        ball.w = BALL_SIZE;
        ball.h = BALL_SIZE;
        SDL_RenderCopy(renderer, ballTexture, NULL, &ball);
        SDL_RenderCopy(renderer, lScoreTexture, NULL, &lScoreRect);
        SDL_RenderCopy(renderer, rScoreTexture, NULL, &rScoreRect);
        SDL_RenderPresent(renderer);

        Mix_PlayMusic(backgroundMusic, -1);

        while (running)
        {
            handleEvents();
            update();
            render();
        }
        SDL_Delay(3000);
        cleanup();
    }

private:
    struct Paddle
    {
        SDL_Rect rect;        // SDL_Rect is a structure that represents a rectangle. It is commonly used to define the position and dimensions of objects
                              // or areas within a GUI or when working with graphics and rendering in SDL-based applications.
        SDL_Texture *texture; // SDL_Texture is a structure in the SDL library that represents an optimized texture for rendering on the GPU(Graphics Processing Unit).
    };
    SDL_Texture *ballTexture;
    SDL_Texture *lScoreTexture;
    SDL_Texture *rScoreTexture;
    SDL_Texture *backgroundTexture;

    Mix_Chunk *paddleHitSound; // The Mix_Chunk is a structure that represents a sound effect.
    // paddleHitSound is a pointer used to store the sound effect when the paddle hits ball.
    SDL_Rect ball;
    Paddle lPaddle; // creating a left paddle using Paddle structure
    Paddle rPaddle; // creating a right paddle using Paddle structure
    int ballVelX;
    int ballVelY;
    int lScore;   // variable for storing left player's score
    int rScore;   // variable for storing right player's score
    bool running; // variable for checking the state of game(running or not)
    SDL_Texture *scoreTexture;
    SDL_Rect lScoreRect;
    SDL_Rect rScoreRect;
    bool initialize() // This method initializes SDL and other necessary components.
    {
        font = TTF_OpenFont("Oswald-Bold.ttf", 75); // loading the Oswald-Bold font file and setting its size

        lPaddle.rect.x = 20;                             // specifying the horizontal position of the paddle on the game screen.
        lPaddle.rect.y = Height / 2 - PADDLE_HEIGHT / 2; // specifying the vertical position of the paddle on the game screen.
        lPaddle.rect.w = PADDLE_WIDTH;                   // specifying the width of the paddle on the game screen.
        lPaddle.rect.h = PADDLE_HEIGHT;                  //  specifying the height of the paddle on the game screen.

        rPaddle.rect.x = Width - 20 - PADDLE_WIDTH;
        rPaddle.rect.y = Height / 2 - PADDLE_HEIGHT / 2;
        rPaddle.rect.w = PADDLE_WIDTH;
        rPaddle.rect.h = PADDLE_HEIGHT;

        return true;
    }
    void handleEvents() // This method handles SDL events, such as keyboard input.
    {
        SDL_Event event;              // SDL_Event is a structure used to represent an event that occurs in program, such as a key press, mouse movement, window event, or user-defined event.
        while (SDL_PollEvent(&event)) // SDL_PollEvent() is a function used to check for pending events in the event queue and retrieve the next event, if available.
                                      //  The parameter is a pointer to an SDL_Event structure where the retrieved event will be stored.
        {
            if (event.type == SDL_QUIT) // If a quit event((e.g. by clicking the close button) is detected, the program is exited.
            {
                cleanup();
                exit(0);
            }
        }

        const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL); // SDL_GetKeyboardState is a function used to get the current state of the keyboard.(Uint8 = unsigned 8 bit integer)
        // The function returns a pointer to an array of Uint8 values, where each element represents the state of a specific key on the keyboard.
        // Scan codes are unique identifiers assigned to each key on a keyboard, regardless of the physical layout or keyboard language.
        if (currentKeyStates[SDL_SCANCODE_W] && lPaddle.rect.y > 0)
        {
            lPaddle.rect.y -= PADDLE_SPEED;
        }
        if (currentKeyStates[SDL_SCANCODE_S] && lPaddle.rect.y + lPaddle.rect.h < Height)
        {
            lPaddle.rect.y += PADDLE_SPEED;
        }
        if (currentKeyStates[SDL_SCANCODE_UP] && rPaddle.rect.y > 0)
        {
            rPaddle.rect.y -= PADDLE_SPEED;
        }
        if (currentKeyStates[SDL_SCANCODE_DOWN] && rPaddle.rect.y + rPaddle.rect.h < Height)
        {
            rPaddle.rect.y += PADDLE_SPEED;
        }
        if (currentKeyStates[SDL_SCANCODE_ESCAPE])
        {
            cleanup();
            exit(0);
        }
    }
    void cleanup() // This method releases the memory resources used by SDL and other components.
    {
        if (ballTexture)
        {
            SDL_DestroyTexture(ballTexture);
            ballTexture = nullptr;
        }
        if (lPaddle.texture)
        {
            SDL_DestroyTexture(lPaddle.texture);
            lPaddle.texture = nullptr;
        }
        if (rPaddle.texture)
        {
            SDL_DestroyTexture(rPaddle.texture);
            rPaddle.texture = nullptr;
        }
        if (lScoreTexture)
        {
            SDL_DestroyTexture(lScoreTexture);
            lScoreTexture = nullptr;
        }

        if (rScoreTexture)
        {
            SDL_DestroyTexture(rScoreTexture);
            rScoreTexture = nullptr;
        }

        if (paddleHitSound)
        {
            Mix_FreeChunk(paddleHitSound);
            paddleHitSound = nullptr;
        }
    }

    void loadMedia() // This method loads images, fonts, and sounds.
    {
        SDL_Surface *ballSurface = IMG_Load(BALL_IMAGE_PATH.c_str()); // SDL_Surface is a structure in the SDL library that represents a two-dimensional image surface.
        // ballSurface is a pointer used to store the result of loading the ball image using IMG_Load.
        if (!ballSurface)
        {
            cout << "Failed to load ball image: " << IMG_GetError() << endl;
            cleanup();
            exit(1);
        }

        ballTexture = SDL_CreateTextureFromSurface(renderer, ballSurface); // SDL_CreateTextureFromSurface allows you to convert an SDL surface into a texture
        // creating texture of ball

        SDL_FreeSurface(ballSurface); // SDL_FreeSurface is a function used to free the memory allocated for an SDL_Surface object.
        // freeing the memory because after creating texture, surface is no longer needed
        if (!ballTexture)
        {
            cout << "Failed to create ball texture: " << SDL_GetError() << endl;
            cleanup();
            exit(1);
        }
        ball.w = ballSurface->w; // setting the width of the ball based on the width of the loaded image surface (ballSurface).
        ball.h = ballSurface->h; // setting height of the ball based on the height of the loaded image surface (ballSurface).
        SDL_Surface *paddleSurface = IMG_Load(PADDLE_IMAGE_PATH.c_str());
        // paddleSurface is used to store the result of loading the paddle image using IMG_Load.
        if (!paddleSurface)
        {
            cout << "Failed to load paddle image: " << IMG_GetError() << endl;
            return;
        }

        lPaddle.texture = SDL_CreateTextureFromSurface(renderer, paddleSurface); // creating texture of left paddle
        if (!lPaddle.texture)
        {
            cout << "Failed to create left paddle texture: " << SDL_GetError() << endl;
            SDL_FreeSurface(paddleSurface);
            return;
        }

        rPaddle.texture = SDL_CreateTextureFromSurface(renderer, paddleSurface); // creating texture of right paddle
        if (!rPaddle.texture)
        {
            cout << "Failed to create right paddle texture: " << SDL_GetError() << endl;
            SDL_FreeSurface(paddleSurface);
            return;
        }

        SDL_FreeSurface(paddleSurface);

        SDL_Surface *lScoreSurface = TTF_RenderText_Solid(font, "0", textColor);
        if (!lScoreSurface)
        {
            cout << "Failed to render left score text: " << TTF_GetError() << endl;
            cleanup();
            exit(1);
        }
        SDL_Surface *rScoreSurface = TTF_RenderText_Solid(font, "0", textColor);
        if (!rScoreSurface)
        {
            cout << "Failed to render right score text: " << TTF_GetError() << endl;
            cleanup();
            exit(1);
        }

        lScoreTexture = SDL_CreateTextureFromSurface(renderer, lScoreSurface);
        if (!lScoreTexture)
        {
            cout << "Failed to create left score texture: " << SDL_GetError() << endl;
            cleanup();
            exit(1);
        }

        rScoreTexture = SDL_CreateTextureFromSurface(renderer, rScoreSurface);
        if (!rScoreTexture)
        {
            cout << "Failed to create right score texture: " << SDL_GetError() << endl;
            cleanup();
            exit(1);
        }

        SDL_FreeSurface(lScoreSurface);
        SDL_FreeSurface(rScoreSurface);

        SDL_Surface *backgroundSurface = IMG_Load("images/pongBg.jpg");
        if (!backgroundSurface)
        {
            cout << "Failed to load background image: " << IMG_GetError() << endl;
            cleanup();
            exit(1);
        }
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);
        if (!backgroundTexture)
        {
            cout << "Failed to create background texture: " << SDL_GetError() << endl;
            cleanup();
            exit(1);
        }
        backgroundMusic = Mix_LoadMUS("sound/ping-pong.mp3"); // Mix_LoadMUS() is used to load a supported audio format into a music object.

        paddleHitSound = Mix_LoadWAV("sound/paddle-hit.mp3"); // Mix_LoadMUS() is used to load a supported audio format into a chunk.
        if (!paddleHitSound)
        {
            cout << "Failed to load paddle hit sound: " << Mix_GetError() << endl;
            cleanup();
            exit(1);
        }
    }

    void update() // This method updates the game state, such as moving the ball and paddles, checking collisions, and updating scores.
    {
        ball.x += ballVelX;
        ball.y += ballVelY;
        // moving the ball by adding the current velocity

        if (lScore >= maxScore || rScore >= maxScore) // checking if either player has reached max score
        {
            running = false;
        }

        if (ball.y <= 0 || ball.y + BALL_SIZE >= Height) // checks if the ball collides with screen edges, and if it does reverses vertical velocity, causing the ball to bounce off edges.
        {
            ballVelY = -ballVelY;
        }

        if (ball.x <= lPaddle.rect.x + lPaddle.rect.w && ball.y + BALL_SIZE >= lPaddle.rect.y && ball.y <= lPaddle.rect.y + lPaddle.rect.h)
        // checks if ball collides with left paddle and if it does, the horizontal velocity is reversed, and hitting sound is played
        {
            ballVelX = -ballVelX;
            Mix_PlayChannel(-1, paddleHitSound, 0); // Mix_PlayChannel() is used to play an audio chunk on a specific channel.
            // 1st parameter is "channel". Here its value (-1) allows SDL_mixer choose an available channel automatically.
            // 2nd parameter is pointer to "Mix_Chunk". Here its value (paddleHitSound) tells whichb sound is to be played.
            // 3rd parameter is "loops". Here its value (0) means the sound will be played once.
        }

        if (ball.x + BALL_SIZE >= rPaddle.rect.x && ball.y + BALL_SIZE >= rPaddle.rect.y && ball.y <= rPaddle.rect.y + rPaddle.rect.h)
        // checks if ball collides with right paddle and if it does, the horizontal velocity is reversed, and hitting sound is played
        {
            ballVelX = -ballVelX;
            Mix_PlayChannel(-1, paddleHitSound, 0);
        }

        if (ball.x <= 0) // checks if ball has reached left edge, if it does the right player's score is incremented and ball is resetted to it's original position
        {
            rScore++;
            ball.x = Width / 2 - BALL_SIZE / 2;
            ball.y = Height / 2 - BALL_SIZE / 2;
            ballVelX = BALL_SPEED;
            ballVelY = BALL_SPEED;
        }

        else if (ball.x + BALL_SIZE >= Width) // checks if ball has reached right edge, if it does the left player's score is incremented and ball is resetted to it's original position
        {
            lScore++;
            ball.x = Width / 2 - BALL_SIZE / 2;
            ball.y = Height / 2 - BALL_SIZE / 2;
            ballVelX = -BALL_SPEED;
            ballVelY = -BALL_SPEED;
        }

        string lScoreStr = to_string(lScore);
        string rScoreStr = to_string(rScore);
        // converting scores from integer to string to render on screen

        SDL_Surface *lScoreSurface = TTF_RenderText_Solid(font, lScoreStr.c_str(), textColor);
        SDL_Surface *rScoreSurface = TTF_RenderText_Solid(font, rScoreStr.c_str(), textColor);
        // creating surfaces to render the left and right scores as text
        // TTF_RenderText_Solid() is used to render solid text onto an SDL surface. It takes the specified text, font, and color as input and produces a rendered SDL surface containing the rendered text.

        SDL_DestroyTexture(lScoreTexture);
        SDL_DestroyTexture(rScoreTexture);
        // destroying the existing textures for rendering the left and right scores.

        lScoreTexture = SDL_CreateTextureFromSurface(renderer, lScoreSurface);
        rScoreTexture = SDL_CreateTextureFromSurface(renderer, rScoreSurface);
        // creating new textures from the updated score surfaces

        SDL_FreeSurface(lScoreSurface);
        SDL_FreeSurface(rScoreSurface);
        // freeing the memory used by the score surfaces after the textures have been created
    }

    void render() // This method renders the game on the screen, including paddles, ball, scores, and a game won / game over message.
    {
        SDL_RenderClear(renderer); // SDL_RenderClear() clears the entire renderer

        // SDL_RenderCopy() is a function used to copy a texture onto the rendering target (usually a window or screen) during the rendering process. Allows to display an SDL_Texture on the screen at a specific position, with optional scaling and rotation.
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        SDL_RenderCopy(renderer, lPaddle.texture, NULL, &lPaddle.rect); // rendering left paddle on screen
        SDL_RenderCopy(renderer, rPaddle.texture, NULL, &rPaddle.rect); // rendering right paddle on screen

        SDL_RenderCopy(renderer, ballTexture, NULL, &ball); // rendering ball on screen

        SDL_QueryTexture(lScoreTexture, NULL, NULL, &lScoreRect.w, &lScoreRect.h); // SDL_QueryTexture() is used to retrieve important information about a texture.
        lScoreRect.x = (Width / 2) - SCORE_X_OFFSET - lScoreRect.w;                // setting position of left player's score
        lScoreRect.y = SCORE_Y_OFFSET;
        SDL_RenderCopy(renderer, lScoreTexture, NULL, &lScoreRect); // rendering left player's score on screen

        SDL_QueryTexture(rScoreTexture, NULL, NULL, &rScoreRect.w, &rScoreRect.h);
        rScoreRect.x = (Width / 2) + SCORE_X_OFFSET; // setting position of right player's score
        rScoreRect.y = SCORE_Y_OFFSET;
        SDL_RenderCopy(renderer, rScoreTexture, NULL, &rScoreRect); // rendering right player's score on screen

        int partitionX = (Width / 2) - 5;                     // variable setting the x component for partition line
        SDL_SetRenderDrawColor(renderer, 235, 242, 240, 255); // setting the color of the partition line
        for (int i = 0; i < Height; i += 20)                  // drawing the line on the whole window having a distance of 20 px
        {
            SDL_RenderDrawLine(renderer, partitionX, i, partitionX, i + 10); // creats a 10px line segment
        }
        if (!running)
        { // this block implements if the game is not running
            msgfont = TTF_OpenFont("Oswald-Bold.ttf", 35);
            SDL_Surface *gameOverSurface = TTF_RenderText_Solid(msgfont, "GAME OVER", textColor); // creating a surface containing rendered game over text
            SDL_Surface *winnerSurface = TTF_RenderText_Solid(msgfont, (lScore >= 10 ? std::string("LEFT PLAYER WINS!") : std::string("RIGHT PLAYER WINS!")).c_str(), textColor);
            // creating a surface containing rendered winner text

            SDL_Texture *gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
            SDL_Texture *winnerTexture = SDL_CreateTextureFromSurface(renderer, winnerSurface);
            // creating texture from surface to render text

            SDL_Rect gameOverRect;
            gameOverRect.x = Width / 2 - gameOverSurface->w / 2;
            gameOverRect.y = Height / 2 - gameOverSurface->h;
            gameOverRect.w = gameOverSurface->w;
            gameOverRect.h = gameOverSurface->h;

            SDL_Rect winnerRect;
            winnerRect.x = Width / 2 - winnerSurface->w / 2;
            winnerRect.y = Height / 2;
            winnerRect.w = winnerSurface->w;
            winnerRect.h = winnerSurface->h;
            // defining the rectangles that represent the positions and dimensions of the rendered text on the screen.

            SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect); // render the game over text on the screen.
            SDL_RenderCopy(renderer, winnerTexture, NULL, &winnerRect);     // render the winner text on the screen.

            SDL_FreeSurface(gameOverSurface);
            SDL_FreeSurface(winnerSurface);
            SDL_DestroyTexture(gameOverTexture);
            SDL_DestroyTexture(winnerTexture);
        }
        SDL_RenderPresent(renderer); // presents the rendered frame on the screen, making it visible to the user.
    }
};
