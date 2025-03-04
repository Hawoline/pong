#include "windows.h"
#include "math.h"

typedef struct {
	float x, y, width, height, rad, dx, dy, speed;
	HBITMAP hBitmap;//����� � ������� ������ 
	bool active;
} sprite;

sprite racket;
sprite ball;
const int block_columns = 20;
const int block_rows = 6;
sprite blocks[block_columns][block_rows];

struct {
	int score, balls;
	bool action = false;//��������� - �������� (����� ������ ������ ������) ��� ����
} game;

struct {
	HWND hWnd;//����� ����
	HDC device_context, context;// ��� ��������� ���������� (��� �����������)
	int width, height;
} window;
// TODO fix read and write index out of bounds exception
// ��� ���� 90 �������� ���� �� ����� �� ���������� ����� ����
HBITMAP hBack;

void InitGame()
{
	//� ���� ������ ��������� ������� � ������� ������� gdi
	//���� ������������� - ����� ������ ������ ����� � .exe 
	//��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
	ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	int block_width = window.width / block_columns;
	int block_height = window.height / 20;
	const int offset = window.height / 3;
	for (int i = 0; i < block_columns; i++) {
		for (int j = 0; j < block_rows; j++) {
			blocks[i][j].x = block_width * i;
			blocks[i][j].y = block_height * j + offset;
			blocks[i][j].width = block_width;
			blocks[i][j].height = block_height;
			blocks[i][j].active = true;
			blocks[i][j].hBitmap = racket.hBitmap;
		}
	}
	//------------------------------------------------------

	racket.width = 300;
	racket.height = 50;
	racket.speed = 50;
	racket.x = window.width / 2;
	racket.y = window.height - racket.height;

	ball.dy = (rand() % 65 + 35) / 100.;
	ball.dx = -(1 - ball.dy);
	ball.speed = 250;
	ball.rad = 20;
	ball.x = racket.x;
	ball.y = racket.y - ball.rad;

	game.score = 0;
	game.balls = 9;
}

void ProcessSound(const char* name)//������������ ���������� � ������� .wav, ���� ������ ������ � ��� �� ����� ��� � ���������
{
	PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//���������� name �������� ��� �����. ���� ASYNC ��������� ����������� ���� ���������� � ����������� ���������
}

void ShowScoreAndHealth()
{
	SetTextColor(window.context, RGB(160, 160, 160));
	SetBkColor(window.context, RGB(0, 0, 0));
	SetBkMode(window.context, TRANSPARENT);
	auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
	auto hTmp = (HFONT)SelectObject(window.context, hFont);

	char txt[32];//����� ��� ������
	_itoa_s(game.score, txt, 10);//�������������� �������� ���������� � �����. ����� �������� � ���������� txt
	TextOutA(window.context, 10, 10, "Score", 5);
	TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

	_itoa_s(game.balls, txt, 10);
	TextOutA(window.context, 10, 100, "Balls", 5);
	TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

void ProcessInput()
{
	if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
	if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;

	if (!game.action && GetAsyncKeyState(VK_SPACE))
	{
		game.action = true;
	}
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
	HBITMAP hbm, hOldbm;
	HDC hMemDC;
	BITMAP bm;

	hMemDC = CreateCompatibleDC(hDC); // ������� �������� ������, ����������� � ���������� �����������
	hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// �������� ����������� bitmap � �������� ������

	if (hOldbm) // ���� �� ���� ������, ���������� ������
	{
		GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // ���������� ������� �����������

		if (alpha)
		{
			TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//��� ������� ������� ����� ����� ��������������� ��� ����������
		}
		else
		{
			StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // ������ ����������� bitmap
		}

		SelectObject(hMemDC, hOldbm);// ��������������� �������� ������
	}

	DeleteDC(hMemDC); // ������� �������� ������
}

void ShowBackgroundAndRacketAndBall()
{
	ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//������ ���
	ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);

	ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);
}

void ShowBlocks()
{
	for (int i = 0; i < block_columns; i++) {
		for (int j = 0; j < block_rows; j++) {
			if (blocks[i][j].active) {
				ShowBitmap(window.context, blocks[i][j].x, blocks[i][j].y, blocks[i][j].width, blocks[i][j].height, blocks[i][j].hBitmap);
			}
		}
	}
}

void LimitRacket()
{
	racket.x = max(racket.x, racket.width / 2.);//���� ��������� ������ ���� ������� ������ ����, �������� �� ����
	racket.x = min(racket.x, window.width - racket.width / 2.);//���������� ��� ������� ����
}

void CheckWalls()
{
	if (ball.x < ball.rad || ball.x > window.width - ball.rad)
	{
		ball.dx *= -1;
	}
}

void CheckRoof()
{
	if (ball.y < ball.rad)
	{
		ball.dy *= -1;
	}
}

bool tail = false;

void CheckFloor()
{
	if (ball.y > window.height - ball.rad - racket.height)//����� ������� ����� ������� - ����������� �������
	{
		if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//����� �����, � �� �� � ������ ��������� ������
		{
			game.score++;
			ball.speed += 5. / game.score;
			ball.dy *= -1;
			racket.width -= 10. / game.score;
		}
		else
		{//����� �� �����

			tail = true;//����� ������ ������ ���� �������

			if (ball.y - ball.rad > window.height);
			{
				game.balls--;

				if (game.balls < 0) {

					MessageBoxA(window.hWnd, "game over", "", MB_OK);//������ �� �������� ����� ��������� � ����� ����?
					InitGame();
				}

				ball.dy = (rand() % 65 + 35) / 100.;//������ ����� ��������� ������ ��� ������
				ball.dx = -(1 - ball.dy);
				ball.x = racket.x;
				ball.y = racket.y - ball.rad;
				game.action = false;
				tail = false;
			}
		}
	}
}
void CheckBLocks()
{
	int l = sqrt(ball.dx * ball.speed * ball.dx * ball.speed + ball.dy * ball.speed * ball.dy * ball.speed);
	for (int t = 0; t < l; t++) {
		float current_x = ball.x + ball.dx * ball.speed * t / (float)l;
		float current_y = ball.y + ball.dy * ball.speed * t / (float)l;

		for (int i = 0; i < block_columns; i++) {
			for (int j = 0; j < block_rows; j++) {
				if (current_x >= blocks[i][j].x && current_x <= blocks[i][j].x + blocks[i][j].width &&
					current_y >= blocks[i][j].y && current_y <= blocks[i][j].y + blocks[i][j].height &&
					blocks[i][j].active) {
					blocks[i][j].active = false;
					int rebound_left_x = current_x - blocks[i][j].x;
					int rebound_right_x = blocks[i][j].x + blocks[i][j].width - current_x;
					int rebound_small_x = min(rebound_left_x, rebound_right_x);

					int rebound_top_y = current_y - blocks[i][j].y;
					int rebound_bottom_y = blocks[i][j].y + blocks[i][j].height - current_y;
					int rebound_small_y = min(rebound_top_y, rebound_bottom_y);

					if (rebound_small_x < rebound_small_y) {
						ball.dx *= -1;
					}
					else {
						ball.dy *= -1;
					}
					return;
				}
			}
		}
	}
}

void ProcessRoom()
{
	CheckWalls();
	CheckRoof();
	CheckFloor();
	CheckBLocks();
}
void ProcessBall()
{
	if (game.action)
	{
		ball.x += ball.dx * ball.speed;
		ball.y += ball.dy * ball.speed;
	}
	else
	{
		ball.x = racket.x;
	}
}

void InitWindow()
{
	SetProcessDPIAware();
	window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

	RECT r;
	GetClientRect(window.hWnd, &r);
	window.device_context = GetDC(window.hWnd);//�� ������ ���� ������� ����� ��������� ���������� ��� ���������
	window.width = r.right - r.left;//���������� ������� � ���������
	window.height = r.bottom - r.top;
	window.context = CreateCompatibleDC(window.device_context);//������ �����
	SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//����������� ���� � ���������
	GetClientRect(window.hWnd, &r);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	InitWindow();
	InitGame();

	ShowCursor(NULL);

	while (!GetAsyncKeyState(VK_ESCAPE))
	{
		ShowBackgroundAndRacketAndBall();
		ShowBlocks();
		ShowScoreAndHealth();

		BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
		Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)

		ProcessInput();//����� ����������
		LimitRacket();//���������, ����� ������� �� ������� �� �����
		ProcessBall();
		ProcessRoom();//������������ ������� �� ���� � �������, ��������� ������ � ��������
	}
}
