#include <iostream>
#include <chrono>
#include <Windows.h>
#include <vector>
#include <algorithm>
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
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#.......#####.##";
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
		if(GetAsyncKeyState(static_cast<unsigned short>('K')) && 0x8000)
		{
			PlayerAngle -= 0.1f * 15.f * deltaTime;
		}

		if (GetAsyncKeyState(static_cast<unsigned short>('L')) && 0x8000)
		{ 
			PlayerAngle += 0.1f * 15.f * deltaTime;
		}


		if (GetAsyncKeyState(static_cast<unsigned short>('A')) && 0x8000)
		{
			PlayerX -= cosf(PlayerAngle) * 5.0f * deltaTime;
			PlayerY += sinf(PlayerAngle) * 5.0f * deltaTime;

			if (map[static_cast<int>(PlayerY) * MapWidth + static_cast<int>(PlayerX)] == '#')
			{
				PlayerX += cosf(PlayerAngle) * 5.0f * deltaTime;
				PlayerY -= sinf(PlayerAngle) * 5.0f * deltaTime;
			}
		}

		if (GetAsyncKeyState(static_cast<unsigned short>('D')) && 0x8000)
		{
			PlayerX += cosf(PlayerAngle) * 5.0f * deltaTime;
			PlayerY -= sinf(PlayerAngle) * 5.0f * deltaTime;

			if (map[static_cast<int>(PlayerY) * MapWidth + static_cast<int>(PlayerX)] == '#')
			{
				PlayerX -= cosf(PlayerAngle) * 5.0f * deltaTime;
				PlayerY += sinf(PlayerAngle) * 5.0f * deltaTime;
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
			bool IsHitWallBoundry = false;

			float EyeX = sinf(RayAngle); //Unit vector for ray in player space
			float EyeY = cosf(RayAngle);

			while (!IsHitWall && DistanceToWall < MaxDepth)
			{
				DistanceToWall += 0.1f;

				//Coordinate on the map the ray currently is reaching to 
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

						//Accumulates for corners of walls
						std::vector<std::pair<float, float>> CornerDistances; //distance to wall corner / dot product (angle between player-to-wall ray and corner-to-player ray)

						for (int CornerTestX = 0; CornerTestX < 2; CornerTestX++)
						{
							for (int CornerTestY = 0; CornerTestY < 2; CornerTestY++)
							{
								//Vector from the corner to the player 
								float VectorX = static_cast<float>(TestX) + CornerTestX - PlayerX;
								float VectorY = static_cast<float>(TestY) + CornerTestY - PlayerY;
								float VectorMagnitude = sqrt(VectorX * VectorX + VectorY * VectorY);
								float VectorDotProduct = (EyeX * VectorX / VectorMagnitude) + (EyeY * VectorY / VectorMagnitude);
								CornerDistances.push_back(make_pair(VectorMagnitude, VectorDotProduct));
							}
						}

						//Sort pairs from closes to farthest
						std::sort(CornerDistances.begin(), CornerDistances.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first  < right.first; });

						float TestBound = 0.008f;
						IsHitWallBoundry = (acos(CornerDistances.at(0).second) < TestBound) || (acos(CornerDistances.at(1).second) < TestBound) || (acos(CornerDistances.at(2).second) < TestBound);
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

			if (IsHitWallBoundry)
			{
				Shade = ' ';
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

		//Display stats
		swprintf(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f", PlayerX, PlayerY, PlayerAngle, 1.0f / deltaTime);

		//Display map
		for (int MapX = 0; MapX < MapWidth; MapX++)
		{
			for (int MapY = 0; MapY < MapHeight; MapY++)
			{
				screen[(MapY + 1) * ScreenWidth + MapX] = map[MapY * MapWidth + MapX];
			}
		}
		screen[static_cast<int>(PlayerY + 1) * ScreenWidth + static_cast<int>(PlayerX)] = '@';

		screen[ScreenWidth * ScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, ScreenWidth * ScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}