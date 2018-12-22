#include "pch.h"
#include "Game.h"

Game::Game()
{
	mWindow = nullptr;
	mRenderer = nullptr;
	mIsRunning = true;
}

bool Game::Initialize()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);

	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());

		return false;
	}

	mWindow = SDL_CreateWindow(
		"SDL Pong Clone",    //Window title
		100,                      // Top left x-coordinate
		100,                        // Top left y-coordinate
		screenWidth,                // Width of window
		screenHeight,               // Height of window
		0                           // Flags (0 for no flags set)
	);

	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());

		return false;
	}

	mRenderer = SDL_CreateRenderer(
		mWindow,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());

		return false;
	}

	ResetGame();

	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::Shutdown()
{
	SDL_DestroyWindow(mWindow);
	SDL_DestroyRenderer(mRenderer);

	SDL_Quit();
}

void Game::ProcessInput()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;
		default:
			break;
		}

		const Uint8* state = SDL_GetKeyboardState(NULL);

		// The esc key ends the game
		if (state[SDL_SCANCODE_ESCAPE])
		{
			mIsRunning = false;
		}

		// The enter or return key starts the game and toggles the pause
		if (state[SDL_SCANCODE_RETURN])
		{
			if (!mIsStarted)
			{
				StartGame();
			}
			else
			{
				PauseGame();
			}
		}

		// if game isn't started don't allow players to move the paddles.
		if (mIsStarted && !mIsPaused)
		{
			mLeftPaddleDir = 0;
			mRightPaddleDir = 0;
			if (state[SDL_SCANCODE_W])
			{
				mLeftPaddleDir -= 1;
			}

			if (state[SDL_SCANCODE_S])
			{
				mLeftPaddleDir += 1;
			}

			if (state[SDL_SCANCODE_I])
			{
				mRightPaddleDir -= 1;
			}

			if (state[SDL_SCANCODE_K])
			{
				mRightPaddleDir += 1;
			}
		}
	}
}

void Game::UpdateGame()
{
	if (mIsPaused || !mIsStarted) return;

	// limit to 60 FPS (16ms since last frame)
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		;

	// delta time is the difference in ticks from the last frame (converted to seconds)
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;

	// clamp the maximum delta time value
	if (deltaTime > maxDelta)
	{
		deltaTime = maxDelta;
	}

	// Update tick counts (for next frame)
	mTicksCount = SDL_GetTicks();

	// Todo: Update objects in game world as function of delta time
	MovePaddles(deltaTime);
	CheckBallCollision(deltaTime);
	CheckBallOffscreen();
}

void Game::GenerateOutput()
{
	SetDefaultDrawColor();
	SDL_RenderClear(mRenderer);

	DrawWalls();
	DrawBall();
	DrawPaddles();

	SDL_RenderPresent(mRenderer);
}

void Game::DrawWalls()
{
	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);

	SDL_RenderFillRect(mRenderer, &GetTopWall());
	SDL_RenderFillRect(mRenderer, &GetBottomWall());
	/*SDL_RenderFillRect(mRenderer, &GetLeftWall());
	SDL_RenderFillRect(mRenderer, &GetRightWall());*/

	SetDefaultDrawColor();
}

void Game::DrawBall()
{
	SDL_Rect ball{
		static_cast<int>(mBallPos.x - thickness / 2),
		static_cast<int>(mBallPos.y - thickness / 2),
		thickness,
		thickness
	};

	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
	SDL_RenderFillRect(mRenderer, &ball);

}

void Game::DrawPaddles()
{
	SDL_Rect leftPaddle{
		static_cast<int>(mLeftPaddlePos.x - thickness / 2),
		static_cast<int>(mLeftPaddlePos.y - thickness / 2),
		thickness,
		paddleSize
	};

	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
	SDL_RenderFillRect(mRenderer, &leftPaddle);

	SDL_Rect rightPaddle{
		static_cast<int>(mRightPaddlePos.x - thickness / 2),
		static_cast<int>(mRightPaddlePos.y - thickness / 2),
		thickness,
		paddleSize
	};

	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
	SDL_RenderFillRect(mRenderer, &rightPaddle);
}

void Game::SetDefaultDrawColor()
{
	SDL_SetRenderDrawColor(
		mRenderer,
		0,  // R
		0,  // G
		25,  // B
		255 // A
	);
}

void Game::CheckBallCollision(float deltaTime)
{
	mBallPos.x += mBallVel.x * deltaTime;
	mBallPos.y += mBallVel.y * deltaTime;
	if ((mBallPos.y <= thickness && mBallVel.y < 0.0f) || (mBallPos.y >= screenHeight - thickness && mBallVel.y > 0.0f))
	{
		mBallVel.y *= -1;
	}

	if (mBallPos.y >= mLeftPaddlePos.y - (paddleSize / 2) && mBallPos.y <= mLeftPaddlePos.y + (paddleSize / 2) &&
		mBallPos.x <= mLeftPaddlePos.x + thickness / 2 &&
		mBallVel.x < 0.0f)
	{
		mBallPos.x = mLeftPaddlePos.x + thickness / 2;
		mBallVel.x *= -1.0f;
		mBallVel.x += 0.5f;
	}

	if (mBallPos.y >= mRightPaddlePos.y - (paddleSize / 2) && mBallPos.y <= mRightPaddlePos.y + (paddleSize / 2) &&
		mBallPos.x >= mRightPaddlePos.x - thickness / 2 &&
		mBallVel.x > 0.0f)
	{
		mBallPos.x = mRightPaddlePos.x - thickness / 2;
		mBallVel.x *= -1.0f;
		mBallVel.x -= 0.5f;
	}
}

void Game::CheckBallOffscreen()
{
	if (mBallPos.x < -100.0f || mBallPos.x > screenWidth + 100.0f) {
		//mIsRunning = false;
		/*mBallVel.x = 0.0f;
		mBallVel.yt = 0.0f;*/
		mIsStarted = false;
		ResetGame();
	}
}

void Game::MovePaddles(float deltaTime)
{
	// if paddle direction is not zero, move it at a rate of 300 pixels / second
	if (mLeftPaddleDir != 0)
	{
		mLeftPaddlePos.y += mLeftPaddleDir * 300.0f * deltaTime;

		// make sure the paddle stays in bounds
		if (mLeftPaddlePos.y < thickness + 10)
		{
			mLeftPaddlePos.y = thickness + 10.0f;
		}
		else if (mLeftPaddlePos.y >(screenHeight - paddleSize - thickness))
		{
			mLeftPaddlePos.y = (float)(screenHeight - paddleSize - thickness);
		}
	}

	// if paddle direction is not zero, move it at a rate of 300 pixels / second
	if (mRightPaddleDir != 0)
	{
		mRightPaddlePos.y += mRightPaddleDir * 300.0f * deltaTime;

		// make sure the paddle stays in bounds
		if (mRightPaddlePos.y < thickness + 10)
		{
			mRightPaddlePos.y = thickness + 10.0f;
		}
		else if (mRightPaddlePos.y >(screenHeight - paddleSize - thickness))
		{
			mRightPaddlePos.y = (float)(screenHeight - paddleSize - thickness);
		}
	}

}

void Game::StartGame()
{
	mIsStarted = true;

	/*mBallVel.x = -200.0f;
	mBallVel.y = 100.0f;*/
}

void Game::ResetGame()
{
	mIsStarted = false;
	mIsPaused = false;

	mBallPos.x = screenWidth / 2.0f;
	mBallPos.y = screenHeight / 2.0f;

	mLeftPaddlePos.x = thickness + 20.0f;
	mLeftPaddlePos.y = (screenHeight - paddleSize) / 2.0f;

	mRightPaddlePos.x = screenWidth - 20.0f - thickness;
	mRightPaddlePos.y = (screenHeight - paddleSize) / 2.0f;

	mBallVel.x = -200.0f;
	mBallVel.y = 235.0f;

}

void Game::PauseGame()
{
	mIsPaused = !mIsPaused;
}

SDL_Rect Game::GetTopWall()
{
	SDL_Rect wall{ 0, 0, screenWidth, thickness };

	return wall;
}

SDL_Rect Game::GetBottomWall()
{
	SDL_Rect wall{ 0, screenHeight - thickness, screenWidth, thickness };

	return wall;
}

SDL_Rect Game::GetLeftWall()
{
	SDL_Rect wall{ 0, 0, thickness, screenHeight };

	return wall;
}

SDL_Rect Game::GetRightWall()
{
	SDL_Rect wall{ screenWidth - thickness, 0, thickness, screenHeight };

	return wall;
}
