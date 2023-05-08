#include <iostream>
#include <chrono>
#include <Windows.h>
using namespace std;

const int ScreenWidth = 120;
const int ScreenHeight = 40;

float PlayerX = 8.0f;
float PlayerY = 8.0f;
float PlayerAngle = 0.0f;

const int MapHeight = 16;
const int MapWidth = 16;

const float FOV = 3.14159f / 4.0f;

const float MaxDepth = 16;

int main()
{
	wchar_t* screen = new wchar_t[ScreenWidth * ScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	//Game loop
	while (true)
	{

		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float deltaTime = elapsedTime.count();

		//Controls
		//Handle CCW Rotation
		if(GetAsyncKeyState(static_cast<unsigned short>('Q')) && 0x8000)
		{
			PlayerAngle -= 0.1f * 15.f * deltaTime;
		}

		if (GetAsyncKeyState(static_cast<unsigned short>('E')) && 0x8000)
		{ 
			PlayerAngle += 0.1f * 15.f * deltaTime;
		}

		if (GetAsyncKeyState(static_cast<unsigned short>('A')) && 0x8000)
		{
			PlayerX += tanf(PlayerAngle) * 5.0f * deltaTime;
			PlayerY += (cosf(PlayerAngle) / sinf(PlayerAngle)) * 5.0f * deltaTime;

			if (map[static_cast<int>(PlayerY) * MapWidth + static_cast<int>(PlayerX)] == '#')
			{
				PlayerX -= tanf(PlayerAngle) * 5.0f * deltaTime;
				PlayerY -= cosf(PlayerAngle) / sinf(PlayerAngle) * 5.0f * deltaTime;
			}
		}

		if (GetAsyncKeyState(static_cast<unsigned short>('D')) && 0x8000)
		{
			PlayerX -= sinf(PlayerAngle) * 5.0f * deltaTime;
			PlayerY -= cosf(PlayerAngle) * 5.0f * deltaTime;

			if (map[static_cast<int>(PlayerY) * MapWidth + static_cast<int>(PlayerX)] == '#')
			{
				PlayerX += sinf(PlayerAngle) * 5.0f * deltaTime;
				PlayerY += cosf(PlayerAngle) * 5.0f * deltaTime;
			}
		}

		if (GetAsyncKeyState(static_cast<unsigned short>('W')) && 0x8000)
		{
			PlayerX += sinf(PlayerAngle) * 5.0f * deltaTime;
			PlayerY += cosf(PlayerAngle) * 5.0f * deltaTime;

			if (map[static_cast<int>(PlayerY) * MapWidth + static_cast<int>(PlayerX)] == '#')
			{
				PlayerX -= sinf(PlayerAngle) * 5.0f * deltaTime;
				PlayerY -= cosf(PlayerAngle) * 5.0f * deltaTime;
			}
		}

		if (GetAsyncKeyState(static_cast<unsigned short>('S')) && 0x8000)
		{
			PlayerX -= sinf(PlayerAngle) * 5.0f * deltaTime;
			PlayerY -= cosf(PlayerAngle) * 5.0f * deltaTime;

			if (map[static_cast<int>(PlayerY) * MapWidth + static_cast<int>(PlayerX)] == '#')
			{
				PlayerX += sinf(PlayerAngle) * 5.0f * deltaTime;
				PlayerY += cosf(PlayerAngle) * 5.0f * deltaTime;
			}
		}

		for (int x = 0; x < ScreenWidth; x++)
		{
			//For each column, calculate the projected ray angle into world space
			float RayAngle = (PlayerAngle - FOV * 0.5f) + (static_cast<float>(x) / static_cast<float>(ScreenWidth)) * FOV;

			float DistanceToWall = 0.f;
			bool IsHitWall = false; 

			float EyeX = sinf(RayAngle); //Unit vector for ray in player space
			float EyeY = cosf(RayAngle);

			while (!IsHitWall && DistanceToWall < MaxDepth)
			{
				DistanceToWall += 0.1f;

				int TestX = static_cast<int>(PlayerX + EyeX * DistanceToWall);
				int TestY = static_cast<int>(PlayerY + EyeY * DistanceToWall);

				//Test if ray is out of bounds
				if (TestX < 0 || TestX > MapWidth || TestY < 0 || TestY >= MapHeight)
				{
					IsHitWall = true;
					DistanceToWall = MaxDepth;
				}
				else
				{
					//Ray is inbounds so test to see if the ray cell is a wall block
					if (map[TestY * MapWidth + TestX] == '#')
					{
						IsHitWall = true;
					}
				}
			}

			//Calculate distance to ceiling and floor
			int CeilingDistance = static_cast<float>(ScreenHeight * 0.5f) - ScreenHeight / static_cast<float>(DistanceToWall);
			int FloorDistance = ScreenHeight - CeilingDistance;

			short Shade = ' ';
			if (DistanceToWall <= MaxDepth / 4.f)
			{
				Shade = 0x2588; //Very close
			}
			else if (DistanceToWall <= MaxDepth / 3.0f)
			{
				Shade = 0x2593;
			}
			else if (DistanceToWall <= MaxDepth / 2.f)
			{
				Shade = 0x2592;
			}
			else if (DistanceToWall < MaxDepth)
			{
				Shade = 0x2592;
			}
			else
			{
				Shade = ' '; //Too far away
			}

			short FloorShade = ' ';

			for (int y = 0; y < ScreenHeight; y++)
			{
				if (y < CeilingDistance)
				{
					screen[y * ScreenWidth + x] = ' ';
				}
				else if(y > CeilingDistance && y <= FloorDistance)
				{
					screen[y * ScreenWidth + x] = Shade;
				}
				else
				{
					//Shade floor based on distance
					float b = 1.0f - ((static_cast<float>(y) - ScreenHeight * 0.5f) / (static_cast<float>(ScreenHeight) * 0.5f));
					if (b < 0.25f)
					{
						FloorShade = '#';
					}
					else if (b < 0.5f) 
					{
						FloorShade = 'x';
					}
					else if (b < 0.75f)
					{
						FloorShade = '.';
					}
					else if (b < 0.9f)
					{
						FloorShade = '-';
					}
					else
					{
						FloorShade = ' ';
					}

					screen[y * ScreenWidth + x] = FloorShade;
				} 
			}
		}

		screen[ScreenWidth * ScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, ScreenWidth * ScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}