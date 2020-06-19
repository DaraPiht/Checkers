#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <thread>

using namespace std;
using namespace sf;

enum Checker
{
	LOSE = -2,
	WIN = -1,
	BLANK = 0,
	RED = 1,
	RED_Q = 2,
	BLACK = 3,
	BLACK_Q = 4
};


int field[8][8] = {};

const int cell_size = 75;

struct CheckerCoords
{
	int x;
	int y;
};
Packet& operator<<(sf::Packet& packet, const CheckerCoords& coords)
{
	return packet << coords.x << coords.y;
}
Packet& operator>>(sf::Packet& packet, CheckerCoords& coords)
{
	return packet >> coords.x >> coords.y;
}

Packet& operator>>(sf::Packet& packet, int coords[8][8])
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			packet >> coords[i][j];
		}
	}
	return packet;
}


void checkStatus(Socket::Status st)
{
	if (st == Socket::Disconnected)
	{
		cout << "Disc";
	}
	else if (st == Socket::Done)
	{
		return;
	}
	else
	{
		cout << "Omegaoof";
	}
}

// Игровые параметры
bool is_my_turn = true;
struct { int regular; int q; } my_side = { RED, RED_Q };
bool is_enemy(int checker)
{
	return checker != my_side.regular && checker != my_side.q && checker != BLANK;
}
bool is_checker_selected = false; // выбрана ли шашка пользователем при его ходе
CheckerCoords selected_checker; // безымянная структура, хранящая выбранную шашку

void drawChecker(RenderWindow& window, CircleShape checker, int x, int y)
{
	if (is_checker_selected && (x / cell_size) == selected_checker.x && (y / cell_size) == selected_checker.y)
	{
		checker.setOutlineThickness(5);
		checker.setOutlineColor(Color(0, 200, 0));
	}

	checker.setPosition(x, y);
	window.draw(checker);
}

bool is_game_over = false;

bool is_my_turn_done = false;
CheckerCoords desired_checker;

void network_stuff(TcpSocket* socket)
{
	while (!is_game_over)
	{
		if (is_my_turn)
		{
			bool is_okay = false;
			do
			{
				while (!is_my_turn_done);
				is_my_turn_done = false;
				Packet packet;
				packet << selected_checker << desired_checker;


				socket->send(packet);


				packet.clear();
				socket->receive(packet);
				packet >> is_okay;
				if (!is_okay)
					cout << "Так ходить нельзя!\n";
				else
					packet >> field;
			} while (!is_okay);

			cout << "Передаём ход противнику...\n";
			is_my_turn = false;
		}
		else
		{
			Packet packet;
			socket->receive(packet);
			packet >> field;
			is_my_turn = true;
		}


		if (field[7][7] == WIN)
		{
			cout << "Поздравляю, вы выиграли!\n";
			is_game_over = true;
			is_my_turn = false;
			system("pause");
			return;
		}
		else if (field[7][7] == LOSE)
		{
			cout << "Вы проиграли(\n";
			is_game_over = true;
			is_my_turn = false;
			system("pause");
			return;
		}		
	}
}

string ip = "localhost";
int port = 2020;

int main()
{
	setlocale(LC_ALL, "Russian");
	cout << "Checkers v1.0 (beta)\n<3\n";


	cout << "Подключаюсь к серверу...\n";
	TcpSocket socket;
	while (socket.connect(ip, port) != Socket::Done) {};

	// Проверяем, если есть созданная комната
	Packet packet;
	packet << "connect";
	socket.send(packet);

	packet.clear();
	socket.receive(packet);
	bool response;
	packet >> response;


	if (!response)
	{
		cout << "Сейчас уже идёт игра!\n";
		return 1;
	}

	cout << "Ожидаем второго игрока.\n";
	packet.clear();
	socket.receive(packet);
	packet >> field;


	// Коннект к комнате успешен, получаем информацию об игре
	cout << "Успешно вошли в игру!\n";
	packet >> my_side.regular >> my_side.q >> is_my_turn;
	if (my_side.regular == BLACK) cout << "Вы играете за чёрных\n";
	else cout << "Вы играете за красных\n";

	if (is_my_turn) cout << "Вы ходите!\n";
	else cout << "Противник ходит первым!\n";

	thread network_thread(network_stuff, &socket);

	const int size = 600;
	ContextSettings settings;
	settings.antialiasingLevel = 8;

	RenderWindow window(VideoMode(size, size), "Checkers", Style::Default, settings);
	window.setVerticalSyncEnabled(true);

	// Конструктор прямоугольника---------------------
	RectangleShape rect_w;
	rect_w.setSize(Vector2f(75, 75));
	rect_w.setFillColor(Color(128, 128, 128));
	rect_w.setOutlineColor(Color(128, 128, 128));
	rect_w.setOutlineThickness(1);

	RectangleShape rect;
	rect.setSize(Vector2f(75, 75));
	rect.setFillColor(Color(255, 255, 255));
	rect.setOutlineColor(Color(255, 255, 255));
	rect.setOutlineThickness(1);
	//----------------------------------------------------

	// Конструктор чёрной шашки---------------------------
	CircleShape Checker_Black;
	Checker_Black.setRadius(30);
	Checker_Black.setOutlineColor(Color(0, 0, 0));
	Checker_Black.setOutlineThickness(1);
	Checker_Black.setFillColor(Color(0, 0, 0));
	//----------------------------------------------------

	// Конструктор красной шашки--------------------------
	CircleShape Checker_Red;
	Checker_Red.setRadius(30);																																						
	Checker_Red.setOutlineColor(Color(128, 0, 0));
	Checker_Red.setOutlineThickness(1);
	Checker_Red.setFillColor(Color(128, 0, 0));
	//----------------------------------------------------

	// Конструктор дамки чёрной шашки---------------------
	CircleShape Checker_Black_Q;
	Checker_Black_Q.setRadius(30);
	Checker_Black_Q.setOutlineColor(Color(0, 10, 54));
	Checker_Black_Q.setOutlineThickness(1);
	Checker_Black_Q.setFillColor(Color(0, 10, 54));

	//-----------------------------------------------------

	// Конструктор дамки красной шашки---------------------
	CircleShape Checker_Red_Q;
	Checker_Red_Q.setRadius(30);
	Checker_Red_Q.setOutlineColor(Color(255, 0, 0));
	Checker_Red_Q.setOutlineThickness(1);
	Checker_Red_Q.setFillColor(Color(255, 0, 0));

	//----------------------------------------------------

	//цикл вывода-----------------------------------------
	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case Event::Closed:
				window.close();
				break;

			case Event::MouseButtonPressed:
				if (event.mouseButton.button == Mouse::Left)
				{
					int cellX = event.mouseButton.x / cell_size;
					int cellY = event.mouseButton.y / cell_size;

					int selected = field[cellY][cellX];

					if (is_my_turn && (selected == my_side.regular || selected == my_side.q))
					{
						cout << "Выбрана шашка " << cellX << " " << cellY << '\n';
						selected_checker.x = cellX;
						selected_checker.y = cellY;
						is_checker_selected = true;
					}
					else if (is_my_turn && is_checker_selected && selected == BLANK)
					{
						cout << "Желаемая клетка" << cellX << " " << cellY << '\n';
						desired_checker.x = cellX;
						desired_checker.y = cellY;
						is_checker_selected = false;
						is_my_turn_done = true;
					}
					else if (is_my_turn && is_checker_selected && is_enemy(selected))
					{
						cout << "Желаемая клетка" << cellX << " " << cellY << '\n';
						desired_checker.x = cellX;
						desired_checker.y = cellY;
						is_checker_selected = false;
						is_my_turn_done = true;
					}
				}
				break;
			}
		}
		window.clear();

		//Отрисовка доски------------------------------

		//Рисовать сейчас белую или серую? 
		bool isWhite = false;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				// 1) Посчитать координаты
				int x = j * cell_size;
				int y = i * cell_size;

				// 2) Нарисовать клетку
				if (isWhite)
				{
					rect_w.setPosition(x, y);
					window.draw(rect_w);
				}
				else
				{
					rect.setPosition(x, y);
					window.draw(rect);
				}

				// Выравниваем по центру
				x += 6;
				y += 6;
				// 3) Если есть, нарисовать шашку
				switch (field[i][j])
				{
				case RED:
					drawChecker(window, Checker_Red, x, y);
					break;

				case RED_Q:
					drawChecker(window, Checker_Red_Q, x, y);
					break;

				case BLACK:
					drawChecker(window, Checker_Black, x, y);
					break;

				case BLACK_Q:
					drawChecker(window, Checker_Black_Q, x, y);
					break;

				case BLANK:
					break;

				case WIN:
				case LOSE:
					break;

				default:
					cout << "Ошибка (" << field[i][j] << ")\n";
					break;
				}
				isWhite = !isWhite;
			}
			isWhite = !isWhite;
		}

		window.display();
	}

	is_game_over = true;
	network_thread.join();
	return 0;
}
