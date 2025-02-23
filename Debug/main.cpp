#include "windows.h"
#include "math.h"

// секция данных игры  
typedef struct {
	float x, y, width, height, rad, dx, dy, speed;
	HBITMAP hBitmap;//хэндл к спрайту шарика 
	bool active;
} sprite;
//efefefefefefe

sprite racket;//ракетка игрока
sprite ball;//шарик
const int block_columns = 20;
const int block_rows = 6;
sprite blocks[block_columns][block_rows];

struct {
	int score, balls;//количество набранных очков и оставшихся "жизней"
	bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
	HWND hWnd;//хэндл окна
	HDC device_context, context;// два контекста устройства (для буферизации)
	int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения

//cекция кода

void InitGame()
{
	//в этой секции загружаем спрайты с помощью функций gdi
	//пути относительные - файлы должны лежать рядом с .exe 
	//результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
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
	racket.speed = 50;//скорость перемещения ракетки
	racket.x = window.width / 2.;//ракетка посередине окна
	racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки

	ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
	ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
	ball.speed = 25;
	ball.rad = 20;
	ball.x = racket.x;//x координата шарика - на середие ракетки
	ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки

	game.score = 0;
	game.balls = 9;


}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
	//PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
}

void ShowScore()
{
	//поиграем шрифтами и цветами
	SetTextColor(window.context, RGB(160, 160, 160));
	SetBkColor(window.context, RGB(0, 0, 0));
	SetBkMode(window.context, TRANSPARENT);
	auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
	auto hTmp = (HFONT)SelectObject(window.context, hFont);

	char txt[32];//буфер для текста
	_itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
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
		ProcessSound("bounce.wav");
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
			TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
		}
		else
		{
			StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
		}

		SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
	}

	DeleteDC(hMemDC); // Удаляем контекст памяти
}

void ShowRacketAndBall()
{
	ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
	ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ракетка игрока

	ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик
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
	racket.x = max(racket.x, racket.width / 2.);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
	racket.x = min(racket.x, window.width - racket.width / 2.);//аналогично для правого угла
}

void CheckWalls()
{
	if (ball.x < ball.rad || ball.x > window.width - ball.rad)
	{
		ball.dx *= -1;
		ProcessSound("bounce.wav");
	}
}

void CheckRoof()
{
	if (ball.y < ball.rad)
	{
		ball.dy *= -1;
		ProcessSound("bounce.wav");
	}
}

bool tail = false;

void CheckFloor()
{
	if (ball.y > window.height - ball.rad - racket.height)//шарик пересек линию отскока - горизонталь ракетки
	{
		if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//шарик отбит, и мы не в режиме обработки хвоста
		{
			game.score++;//за каждое отбитие даем одно очко
			ball.speed += 5. / game.score;//но увеличиваем сложность - прибавляем скорости шарику
			ball.dy *= -1;//отскок
			racket.width -= 10. / game.score;//дополнительно уменьшаем ширину ракетки - для сложности
			ProcessSound("bounce.wav");//играем звук отскока
		}
		else
		{//шарик не отбит

			tail = true;//дадим шарику упасть ниже ракетки

			if (ball.y - ball.rad > window.height)//если шарик ушел за пределы окна
			{
				game.balls--;//уменьшаем количество "жизней"

				ProcessSound("fail.wav");//играем звук

				if (game.balls < 0) { //проверка условия окончания "жизней"

					MessageBoxA(window.hWnd, "game over", "", MB_OK);//выводим сообщение о проигрыше
					InitGame();//переинициализируем игру
				}

				ball.dy = (rand() % 65 + 35) / 100.;//задаем новый случайный вектор для шарика
				ball.dx = -(1 - ball.dy);
				ball.x = racket.x;//инициализируем координаты шарика - ставим его на ракетку
				ball.y = racket.y - ball.rad;
				game.action = false;//приостанавливаем игру, пока игрок не нажмет пробел
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
	//обрабатываем стены, потолок и пол. принцип - угол падения равен углу отражения, а значит, для отскока мы можем просто инвертировать часть вектора движения шарика
	CheckWalls();
	CheckRoof();
	CheckFloor();
	CheckBLocks();
}
void ProcessBall()
{
	if (game.action)
	{

		//если игра в активном режиме - перемещаем шарик
		ball.x += ball.dx * ball.speed;
		ball.y += ball.dy * ball.speed;
	}
	else
	{
		//иначе - шарик "приклеен" к ракетке
		ball.x = racket.x;
	}
}

void DrawBresenhamLine() {
	/*
	1. Нарисовать линию DONE
	2. Нарисовать линию так, чтобы был инвертированы начала x0, x1, y0, y1
	3. Нарисовать линию вокруг заданной точки
	*/
	int center_of_line = 400;
	int x0 = center_of_line;
	int y0 = center_of_line;
	int x1 = center_of_line + 100;
	int y1 = center_of_line + 50;

	int delta_x = abs(x1 - x0);
	int delta_y = abs(y1 - y0);

	float error = 0;
	float delta_error = (float) (delta_y + 1) / (delta_x + 1);

	int direction_y = y1 - y0;
	if (direction_y < 0) {
		direction_y = -1;
	}
	else {
		direction_y = 1;
	}
	int current_y = y0;
	int direction_x = x1 - x0;
	if (direction_x < 0) {
		direction_x = -1;
	}
	if (direction_x < 0) {
		direction_x = 1;
	}
	if (direction_x > 0) {
		for (int current_x = x0; current_x < x1; current_x++) {
			SetPixel(window.context, current_x, current_y, RGB(255, 255, 255));
			error += delta_error;
			if (error >= 1) {
				current_y += direction_y;
				error -= 1;
			}
		}
	}
	else {
		for (int current_x = x0; current_x > x1; current_x--) {
			SetPixel(window.context, current_x, current_y, RGB(255, 255, 255));
			error += delta_error;
			if (error >= 1) {
				current_y += direction_y;
				error -= 1;
			}
		}
	}

	// Не работает TODO пофиксить
	for (int current_x = x0; (direction_x > 0) ? current_x < x1 : current_x > x1; current_x += direction_x) {
		SetPixel(window.context, current_x, current_y, RGB(255, 255, 255));
		error += delta_error;
		if (error >= 1) {
			current_y += direction_y;
			error -= 1;
		}
	}
}

void InitWindow()
{
	SetProcessDPIAware();
	window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

	RECT r;
	GetClientRect(window.hWnd, &r);
	window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
	window.width = r.right - r.left;//определяем размеры и сохраняем
	window.height = r.bottom - r.top;
	window.context = CreateCompatibleDC(window.device_context);//второй буфер
	SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
	GetClientRect(window.hWnd, &r);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	InitWindow();//здесь инициализируем все что нужно для рисования в окне
	InitGame();//здесь инициализируем переменные игры

	ShowCursor(NULL);

	while (!GetAsyncKeyState(VK_ESCAPE))
	{
		ShowRacketAndBall();//рисуем фон, ракетку и шарик
		ShowBlocks();
		ShowScore(); //рисуем очик и жизни

		DrawBresenhamLine();

		BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
		Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)

		ProcessInput();//опрос клавиатуры
		LimitRacket();//проверяем, чтобы ракетка не убежала за экран
		ProcessBall();//перемещаем шарик
		ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
	}
}
