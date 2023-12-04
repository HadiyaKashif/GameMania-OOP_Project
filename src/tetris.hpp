#include <iostream>
#include <time.h>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "abstract.hpp"
using namespace std;
class Tetris : virtual public Arcade
{
private:
	enum
	{
		BlockW = 42,
		BlockH = 32
	};
	enum
	{
		Lines = 20,
		Cols = 10
	};
	SDL_Texture *background = NULL, *blocks = NULL;
	SDL_Rect srcR = {0, 0, BlockW, BlockH}, destR = {0, 0, BlockW, BlockH};
	Mix_Chunk *rowCompletedSound;
	SDL_Color textColor = {255, 255, 255}, gameOverColor = {255, 255, 255};
	SDL_Event e;

	bool running = false;
	int field[Lines][Cols] = {0}; // field[9][19]
	static const int figures[7][4];
	struct Point
	{
		int x, y;
	} items[4], backup[4], upcomingItems[4];
	int color = 1, upcomingColor = 1;
	int dx = 0;
	bool rotate = false;
	unsigned int delay = 300;
	Uint32 startTime = 0, currentTime = 0;
	int score = 0;

	void cleanup()
	{
		SDL_DestroyTexture(blocks);
		SDL_DestroyTexture(background);
		SDL_DestroyRenderer(renderer);
		Mix_FreeChunk(rowCompletedSound);
	}

	void setCurrentTime(Uint32 t)
	{
		// sets the current time to be used in the game loop
		currentTime = t;
	}

	bool isrunning()
	{
		// checks if the game is still running
		return running;
	}

	bool initialize()
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		int imgFlags = IMG_INIT_PNG;
		int initted = IMG_Init(imgFlags);
		if ((initted & imgFlags) != imgFlags)
		{
			cout << "Failed to initialize required png support\n"
				 << "IMG_Init() Error : " << IMG_GetError() << endl;
				 return false;
		}
		SDL_Surface *loadSurf = IMG_Load("images/tetris_background.png");
		background = SDL_CreateTextureFromSurface(renderer, loadSurf);
		SDL_FreeSurface(loadSurf);
		loadSurf = IMG_Load("images/blocks.png");
		blocks = SDL_CreateTextureFromSurface(renderer, loadSurf);
		backgroundMusic = Mix_LoadMUS("sound/tetris-sounds.mp3");
		rowCompletedSound = Mix_LoadWAV("sound/success.mp3");
		if (!backgroundMusic || !rowCompletedSound)
		{
			cout << "Failed to load music: " << Mix_GetError() << endl;
			return false;
		}
		SDL_FreeSurface(loadSurf);
		TTF_Init();
		font = TTF_OpenFont("Oswald-Bold.ttf", 30);
		msgfont = TTF_OpenFont("Oswald-Bold.ttf", 100);
		if (font == NULL || msgfont == NULL)
		{
			cout << "Failed to load font!" << TTF_GetError() << endl;
			return false;
		}
		nextTetrimino();
		Mix_PlayMusic(backgroundMusic, -1);

		running = true;
		return true;
	}
	void firstTetrimino()
	{
		// Generate the first block at the start of the game
		color = 1 + rand() % 7;
		int n = rand() % 7;
		for (int i = 0; i < 4; i++)
		{
			items[i].x = figures[n][i] % 4;
			items[i].y = int(figures[n][i] / 4);
		}
	}
	void nextTetrimino()
	{
		// Generate the upcoming block
		upcomingColor = 1 + rand() % 7;
		int n = rand() % 7; //  2,6,5,4,
		for (int i = 0; i < 4; i++)
		{
			upcomingItems[i].x = figures[n][i] % 4;		 // 2 2 1 0
			upcomingItems[i].y = int(figures[n][i] / 4); // 0 1 1 1
		}
	}
	void currentTetrimino()
	{
		// Generate the current falling block
		color = upcomingColor;
		for (int i = 0; i < 4; i++)
		{
			items[i].x = upcomingItems[i].x;
			items[i].y = upcomingItems[i].y;
		}
		nextTetrimino();
	}
	void handleEvents()
	{
		// Handles keyboard input events, such as moving the tetrimino left or right and rotating it
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					running = false;
					break;
				case SDLK_UP:
					rotate = true;
					break;
				case SDLK_LEFT:
					dx = -1;
					break;
				case SDLK_RIGHT:
					dx = 1;
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		const Uint8 *state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_DOWN])
			delay = 50;
	}
	bool isvalid()
	{
		// checks if the current tetrimino's position is valid within the game field
		for (int i = 0; i < 4; i++) //  2 6 5 4
			if (items[i].x < 0 || items[i].x >= Cols || items[i].y >= Lines)
				return false;
			else if (field[items[i].y][items[i].x]) // 0 1 1 1  // 2 2 1 0
				return false;
		return true;
	}
	void gameplay()
	{
		// backup
		for (int i = 0; i < 4; i++)
			backup[i] = items[i];
		// move
		if (dx)
		{
			for (int i = 0; i < 4; i++)
			{
				items[i].x += dx;
			}
			if (!isvalid())
				for (int i = 0; i < 4; i++)
					items[i] = backup[i];
		}
		// rotate
		if (rotate)
		{						//  2 6 5 4
			Point p = items[2]; // center of rotation   // p.x = 1 , p.y = 1  ,items[i].x = 2 2 1 0 , items[i].y = 0 1 1 1
			for (int i = 0; i < 4; i++)
			{
				int x = items[i].y - p.y; // -1 0 0  0
				int y = items[i].x - p.x; //  1 1 0 -1
				items[i].x = p.x - x;	  // 2 1 1 1
				items[i].y = p.y + y;	  // 2 2 1 0
			}
			if (!isvalid())
				for (int i = 0; i < 4; i++)
					items[i] = backup[i];
		}
		// tick
		if (currentTime - startTime > delay)
		{
			for (int i = 0; i < 4; i++)
				backup[i] = items[i];
			for (int i = 0; i < 4; i++)
				items[i].y++;
			if (!isvalid())
			{
				for (int i = 0; i < 4; i++)
					field[backup[i].y][backup[i].x] = color;
				currentTetrimino();
			}
			startTime = currentTime;
		}
		// Check for completed lines
		int completedLines = 0;
		for (int i = 0; i < Lines; i++)
		{
			bool lineComplete = true;
			for (int j = 0; j < Cols; j++)
			{
				if (field[i][j] == 0)
				{
					lineComplete = false;
					break;
				}
			}
			if (lineComplete)
			{
				completedLines++;
				for (int j = i; j > 0; j--)
				{
					for (int k = 0; k < Cols; k++)
					{
						field[j][k] = field[j - 1][k];
					}
				}
			}
		}
		// Update the score
		if (completedLines > 0)
		{
			score += completedLines * 10;
			Mix_PlayChannel(-1, rowCompletedSound, 0); // Play the row completed sound
		}
		dx = 0;
		rotate = false;
		delay = 300;
	}
	void setRectPos(SDL_Rect &rect, int x = 0, int y = 0, int w = BlockW, int h = BlockH)
	{
		rect = {x, y, w, h};
	}
	void moveRectPos(SDL_Rect &rect, int x, int y)
	{
		rect.x += x;
		rect.y += y;
	}
	void delay_()
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
		running = false;
	}
	void renderUpcomingBlock()
	{
		SDL_RenderCopy(renderer, background, NULL, NULL);
		for (int i = 0; i < 4; i++)
		{																									  // items[i].x = 2 2 1 0, items[i].y = 0 1 1 1
			setRectPos(srcR, upcomingColor * BlockW);														  // 2 * 42 = 84
			setRectPos(destR, (upcomingItems[i].x + Cols + 2.5) * BlockW, (upcomingItems[i].y + 8) * BlockH); // 609 609 567 525, 256 288 288 288
			SDL_RenderCopy(renderer, blocks, &srcR, &destR);
		}
	}
	void renderGameField()
	{
		for (int i = 0; i < Lines; i++)
			for (int j = 0; j < Cols; j++)
				if (field[i][j])
				{
					setRectPos(srcR, field[i][j] * BlockW);	   // 42
					setRectPos(destR, j * BlockW, i * BlockH); //
					moveRectPos(destR, BlockW, Height - (Lines + 1) * BlockH);
					SDL_RenderCopy(renderer, blocks, &srcR, &destR);
				}
	}
	void renderFallingBlock()
	{
		for (int i = 0; i < 4; i++)
		{																 // items[i].x = 2 2 1 0, items[i].y = 0 1 1 1
			setRectPos(srcR, color * BlockW);							 // 2 * 42 = 84
			setRectPos(destR, items[i].x * BlockW, items[i].y * BlockH); // 84 84 42 0, 0 32 32 32
			moveRectPos(destR, BlockW, Height - (Lines + 1) * BlockH);	 // 42 , 700 - 21 * 32 = 28
			SDL_RenderCopy(renderer, blocks, &srcR, &destR);
		}
	}
	void renderScore()
	{
		string scoreText = "Score: " + to_string(score);
		SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
		SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
		SDL_Rect scoreRect;
		scoreRect.x = Width - scoreSurface->w - 40;	  // Adjust the position as needed
		scoreRect.y = Height - scoreSurface->h - 100; // Adjust the position as needed
		scoreRect.w = scoreSurface->w;
		scoreRect.h = scoreSurface->h;
		SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
		SDL_FreeSurface(scoreSurface);
		SDL_DestroyTexture(scoreTexture);
	}
	void renderNextBlock()
	{
		string Text = "Next Block";
		SDL_Surface *TextSurface = TTF_RenderText_Solid(font, Text.c_str(), textColor);
		SDL_Texture *TextTexture = SDL_CreateTextureFromSurface(renderer, TextSurface);
		SDL_Rect TextRect;
		TextRect.x = Width - TextSurface->w - 36;	// Adjust the position as needed
		TextRect.y = Height - TextSurface->h - 510; // Adjust the position as needed
		TextRect.w = TextSurface->w;
		TextRect.h = TextSurface->h;
		SDL_RenderCopy(renderer, TextTexture, NULL, &TextRect);
		SDL_FreeSurface(TextSurface);
		SDL_DestroyTexture(TextTexture);
	}
	void renderGameOver()
	{
		// Check if the game is over
		bool gameOver = false;
		for (int i = 0; i < Cols; i++)
		{
			if (field[0][i] != 0)
			{
				gameOver = true;
				running = false;
				break;
			}
		}
		if (gameOver)
		{
			string gameOverText = "Game Over!";
			SDL_Surface *gameOverSurface = TTF_RenderText_Solid(msgfont, gameOverText.c_str(), gameOverColor);
			SDL_Texture *gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
			SDL_Rect gameOverRect;
			gameOverRect.x = (Width - gameOverSurface->w) / 2;
			gameOverRect.y = (Height - gameOverSurface->h) / 2;
			gameOverRect.w = gameOverSurface->w;
			gameOverRect.h = gameOverSurface->h;
			SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
			SDL_FreeSurface(gameOverSurface);
			SDL_DestroyTexture(gameOverTexture);
		}
	}
	void updateRender()
	{
		renderUpcomingBlock();
		renderGameField();
		renderFallingBlock();
		renderScore();
		renderNextBlock();
		renderGameOver();
		SDL_RenderPresent(renderer);
	}

public:
	Tetris() : Arcade("Tetris") {}
	void run()
	{
		srand(time(0));
		const char *title = "Tetris game";
		if (initialize())
		{
			firstTetrimino();
			while (isrunning())
			{
				setCurrentTime(SDL_GetTicks());
				handleEvents();
				gameplay();
				updateRender();
			}
		}
		delay_();
		cleanup();
	}
};

/*
	0	1	2	3
	4	5	6	7
*/
const int Tetris::figures[7][4] =
	{
		0, 1, 2, 3, // I
		0, 4, 5, 6, // J
		2, 6, 5, 4, // L
		1, 2, 5, 6, // O
		2, 1, 5, 4, // S
		1, 4, 5, 6, // T
		0, 1, 5, 6, // Z
};