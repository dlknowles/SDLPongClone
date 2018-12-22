#pragma once
#include "SDL.h"
#undef main

class Game
{
public:
	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();
private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	void DrawWalls();
	void DrawBall();
	void DrawPaddles();
	void SetDefaultDrawColor();
	void CheckBallCollision(float deltaTime);
	void CheckBallOffscreen();
	void MovePaddles(float deltaTime);
	void StartGame();
	void ResetGame();
	void PauseGame();
	SDL_Rect GetTopWall();
	SDL_Rect GetBottomWall();
	SDL_Rect GetLeftWall();
	SDL_Rect GetRightWall();

	// Window created by SDL
	SDL_Window* mWindow;
	// Renderer for the screen
	SDL_Renderer* mRenderer;

	const int thickness = 15;
	const int screenWidth = 1024;
	const int screenHeight = 768;
	const int paddleSize = 125;
	const float maxDelta = 0.05f;

	// continue to run?
	bool mIsRunning;
	bool mIsStarted;
	bool mIsPaused;
	Uint32 mTicksCount;
	int mLeftPaddleDir;
	int mRightPaddleDir;

	struct Vector2 { float x; float y; };

	Vector2 mBallPos;
	Vector2 mLeftPaddlePos;
	Vector2 mRightPaddlePos;
	Vector2 mBallVel;
};

