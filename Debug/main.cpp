#include "windows.h"
#include "math.h"

class Sprite {
protected:
	float x, y;
	HBITMAP bitmap;
public:
	void setX(float someX) {
		x = someX;
	}
	float getX() {
		return x;
	}
	void setY(float someY) {
		y = someY;
	}

	float getY() {
		return y;
	}
	void setBitmap(HBITMAP someBitmap) {
		bitmap = someBitmap;
	}
	HBITMAP getBitmap() {
		return bitmap;
	}
};
class RectangleSprite : public Sprite {
protected:
	int width, height;

public:
	int getWidth() {
		return width;
	}

	int getHeight() {
		return height;
	}

	void setWidth(int someWidth) {
		width = someWidth;
	}

	void setHeight(int someHeight) {
		height = someHeight;
	}

	void decreaseWidth(int someWidth) {
		width -= someWidth;
	}
};

class Movable {
protected:
	float dx, dy, speed;
public:
	void move(int someSpeed) {

	}

	void setDx(float someDx) {
		dx = someDx;
	}
	float getDx() {
		return dx;
	}

	void setDy(float someDy) {
		dy = someDy;
	}

	float getDy() {
		return dy;
	}

	void setSpeed(float someSpeed) {
		speed = someSpeed;
	}
	float getSpeed() {
		return speed;
	}

	void changeXDirection() {
		changeXDirection();
	}

	void changeYDirection() {
		changeYDirection();
	}
};

class Ball:public Sprite, public Movable {
private: 
	float radius;
public:
	Ball() {
		radius = 1;
	}
	Ball(int someX, int someY, float someRadius, float someSpeed, HBITMAP someBitmap) {
		x = someX;
		y = someY;
		radius = someRadius;
		speed = someSpeed;
		bitmap = someBitmap;
	}

	void move(float someSpeed) {
		x += someSpeed;
	}

	void setRadius(float someRadius) {
		radius = someRadius;
	}

	float getRadius() {
		return radius;
	}
	
	void increaseSpeed(float someSpeed) {
		speed += someSpeed;
	}
};
class Racket : public RectangleSprite, public Movable {
public:
	Racket() {

	}
	Racket(int someX, int someY, int someWidth, int someHeight, int someSpeed, HBITMAP someBitmap) {
		speed = someSpeed;
	}
};

class Block : public RectangleSprite {
private:
	bool active;
public:
	void setActive(bool someActive) {
		active = someActive;
	}
	bool isActive() {
		return active;
	}
};

class Window: public RectangleSprite {
private:
	HWND hwnd;
	HDC deviceContext, context;// два контекста устройства (для буферизации)
public: 
	Window() {

	}
	Window(HWND hwnd) {
		hwnd = hwnd;
	}

	HWND getHWindow() {
		return hwnd;
	}

	void setHWindow(HWND someHwnd) {
		hwnd = someHwnd;
	}

	void setDeviceContext(HDC someDeviceContext) {
		deviceContext = someDeviceContext;
	}

	HDC getDeviceContext() {
		return deviceContext;
	}

	void setContext(HDC someContext) {
		context = someContext;
	}

	HDC getContext() {
		return context;
	}
};

class Game {
public:
	int score, balls;
	bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
	Game() {

	}
	Game(int someScore, int someBalls) {
		score = someScore;
		balls = someBalls;
	}
};

const int block_columns = 20;
const int block_rows = 6;

HBITMAP hBack;
Block blocks[block_columns][block_rows];
Window window;
Ball ball;
Racket racket;
Game game;

void InitGame()
{
	//в этой секции загружаем спрайты с помощью функций gdi
	//пути относительные - файлы должны лежать рядом с .exe 
	//результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
	racket = Racket(window.getWidth() / 2, window.getHeight() - 50, 300, 50, 250, (HBITMAP) LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	ball = Ball(racket.getX(), 0, 20, 250, (HBITMAP) LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	int block_width = window.getWidth() / block_columns;
	int block_height = window.getHeight() / 20;
	const int offset = window.getHeight() / 3;
	for (int i = 0; i < block_columns; i++) {
		for (int j = 0; j < block_rows; j++) {
			blocks[i][j].setX(block_width * i);
			blocks[i][j].setY(block_height * j + offset);
			blocks[i][j].setWidth(block_width);
			blocks[i][j].setHeight(block_height);
			blocks[i][j].setActive(true);
			blocks[i][j].setBitmap(racket.getBitmap());
		}
	}
	//------------------------------------------------------

	racket.setWidth(300);
	racket.setHeight(50);
	racket.setSpeed(50);
	racket.setX(window.getWidth() / 2);
	racket.setY(window.getHeight() - racket.getHeight());

	ball.setDy((rand() % 65 + 35) / 100.);
	ball.setDx(ball.getDy() - 1);

	game = Game(0, 9);
}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
	PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
}

void ShowScoreAndHealth()
{
	SetTextColor(window.getContext(), RGB(160, 160, 160));
	SetBkColor(window.getContext(), RGB(0, 0, 0));
	SetBkMode(window.getContext(), TRANSPARENT);
	auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
	auto hTmp = (HFONT)SelectObject(window.getContext(), hFont);

	char txt[32];//буфер для текста
	_itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
	TextOutA(window.getContext(), 10, 10, "Score", 5);
	/*TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));*/

	_itoa_s(game.balls, txt, 10);
	TextOutA(window.getContext(), 10, 100, "Balls", 5);
	/*TextOutA(window.getContext(), 200, 100, (LPCSTR)txt, strlen(txt));*/
}

void ProcessInput()
{
	if (GetAsyncKeyState(VK_LEFT)) racket.move(-racket.getSpeed());
	if (GetAsyncKeyState(VK_RIGHT)) racket.move(racket.getSpeed());

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

	hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
	hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

	if (hOldbm) // Если не было ошибок, продолжаем работу
	{
		GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

		if (alpha)
		{
			TransparentBlt(window.getContext(), x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
		}
		else
		{
			StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
		}

		SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
	}

	DeleteDC(hMemDC); // Удаляем контекст памяти
}

void ShowBackgroundAndRacketAndBall()
{
	ShowBitmap(window.getContext(), 0, 0, window.getWidth(), window.getHeight(), hBack);//задний фон
	ShowBitmap(window.getContext(), racket.getX() - racket.getWidth() / 2., racket.getY(), racket.getWidth(), racket.getHeight(), racket.getBitmap());

	ShowBitmap(window.getContext(), ball.getX() - ball.getRadius(), ball.getY() - ball.getRadius(), 2 * ball.getRadius(), 2 * ball.getRadius(), ball.getBitmap(), true);
}

void ShowBlocks()
{
	for (int i = 0; i < block_columns; i++) {
		for (int j = 0; j < block_rows; j++) {
			if (blocks[i][j].isActive()) {
				ShowBitmap(window.getContext(), blocks[i][j].getX(), blocks[i][j].getY(), blocks[i][j].getWidth(), blocks[i][j].getHeight(), blocks[i][j].getBitmap());
			}
		}
	}
}

void LimitRacket()
{
	racket.setX(max(racket.getX(), racket.getWidth() / 2.)) ;//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
	racket.setX(min(racket.getX(), window.getWidth() - racket.getWidth() / 2.));//аналогично для правого угла
}

void CheckWalls()
{
	if (ball.getX() < ball.getRadius() || ball.getX() > window.getWidth() - ball.getRadius())
	{
		ball.changeXDirection();
	}
}

void CheckRoof()
{
	if (ball.getY() < ball.getRadius())
	{
		ball.changeYDirection();
	}
}

bool tail = false;

void CheckFloor()
{
	if (ball.getY() > window.getHeight() - ball.getRadius() - racket.getHeight())//шарик пересек линию отскока - горизонталь ракетки
	{
		if (!tail && ball.getX() >= racket.getX() - racket.getWidth() / 2. - ball.getRadius() && ball.getX() <= racket.getX() + racket.getWidth() / 2. + ball.getRadius())//шарик отбит, и мы не в режиме обработки хвоста
		{
			game.score++;
			ball.increaseSpeed(5. / game.score);
			ball.setDx(ball.getDy() * -1);
			racket.decreaseWidth(10. / game.score);
		}
		else
		{//шарик не отбит

			tail = true;//дадим шарику упасть ниже ракетки

			if (ball.getY() - ball.getRadius() > window.getHeight());
			{
				game.balls--;

				if (game.balls < 0) {

					MessageBoxA(window.getHWindow(), "game over", "", MB_OK);//Почему не работает вывод сообщения о конце игры?
					InitGame();
				}

				ball.setDy((rand() % 65 + 35) / 100.);//задаем новый случайный вектор для шарика
				ball.setDx(ball.getDy() - 1);
				ball.setX(racket.getX());
				ball.setY(racket.getY() - ball.getRadius());
				game.action = false;
				tail = false;
			}
		}
	}
}
void CheckBLocks()
{
	float ball_path_per_frame = sqrt(ball.getDx() * ball.getSpeed() * ball.getDx() * ball.getSpeed() + ball.getDy() * ball.getSpeed() * ball.getDy() * ball.getSpeed());
	float rebound = 0;
	for (int point_of_path = 0; point_of_path < ball_path_per_frame; point_of_path++) {
		float current_x = ball.getX() + ball.getDx() * ball.getSpeed() * (point_of_path - rebound) / ball_path_per_frame - rebound;
		float current_y = ball.getY() + ball.getDy() * ball.getSpeed() * point_of_path / ball_path_per_frame;

		for (int i = 0; i < block_columns; i++) {
			for (int j = 0; j < block_rows; j++) {
				if (blocks[i][j].isActive() && current_x >= blocks[i][j].getX() - 1 && current_x <= blocks[i][j].getX() + blocks[i][j].getWidth() + 1 &&
					current_y >= blocks[i][j].getY() - 1 && current_y <= blocks[i][j].getY() + blocks[i][j].getHeight() + 1) {
					
					blocks[i][j].setActive(false);
					int rebound_left_x = current_x - blocks[i][j].getX();
					int rebound_right_x = blocks[i][j].getX() + blocks[i][j].getWidth() - current_x;
					int rebound_small_x = min(rebound_left_x, rebound_right_x);

					int rebound_top_y = current_y - blocks[i][j].getY();
					int rebound_bottom_y = blocks[i][j].getY() + blocks[i][j].getHeight() - current_y;
					int rebound_small_y = min(rebound_top_y, rebound_bottom_y);

					if (rebound_small_x < rebound_small_y) {
						ball.changeXDirection();
					}
					else {
						ball.changeYDirection();
					}
					ball.setX(current_x);
					ball.setY(current_y);
					rebound = point_of_path;
					i = block_columns;
					break;
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
		ball.setX(ball.getX() + ball.getDx() * ball.getSpeed());
		ball.setY(ball.getY() + ball.getDy() * ball.getSpeed());
	}
	else
	{
		ball.setX(racket.getX());
	}
}

void InitWindow()
{
	SetProcessDPIAware();
	window = Window(CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0));

	RECT r;
	GetClientRect(window.getHWindow(), &r);
	HDC device_context = GetDC(window.getHWindow());//из хэндла окна достаем хэндл контекста устройства для рисования
	window.setDeviceContext(device_context);
	window.setWidth(r.right - r.left);//определяем размеры и сохраняем
	window.setHeight(r.bottom - r.top);
	HDC context = CreateCompatibleDC(device_context);//второй буфер
	window.setContext(context);
	SelectObject(context, CreateCompatibleBitmap(device_context, window.getWidth(), window.getHeight()));//привязываем окно к контексту
	GetClientRect(window.getHWindow(), &r);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	const int block_columns = 20;
	const int block_rows = 6;
	Sprite blocks[block_columns][block_rows];
	InitWindow();
	InitGame();

	ShowCursor(NULL);

	while (!GetAsyncKeyState(VK_ESCAPE))
	{
		ShowBackgroundAndRacketAndBall();
		ShowBlocks();
		ShowScoreAndHealth();

		BitBlt(window.getDeviceContext(), 0, 0, window.getWidth(), window.getHeight(), window.getContext(), 0, 0, SRCCOPY);//копируем буфер в окно
		Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)

		ProcessInput();//опрос клавиатуры
		LimitRacket();//проверяем, чтобы ракетка не убежала за экран
		ProcessBall();
		ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
	}
}
