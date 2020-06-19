#include <iostream>
#include <SFML/Network.hpp>
#include <vector>
#include <string>
#include <thread>

using namespace std;
using namespace sf;


enum Checker
{
	BLANK = 0,
	RED = 1,
	RED_Q = 2,
	BLACK = 3,
	BLACK_Q = 4
};


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

int starting_field[8][8] =
{
	{0,3,0,3,0,3,0,3},
	{3,0,3,0,3,0,3,0},
	{0,3,0,3,0,3,0,3},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{1,0,1,0,1,0,1,0},
	{0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0},
};

int field[8][8] = {};

void resetField()
{
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			field[i][j] = starting_field[i][j];
}

Packet& operator<<(sf::Packet& packet, int coords[8][8])
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			packet << coords[i][j];
		}
	}
	return packet;
}

const int port = 2020;

struct Player
{
	int regular;
	int q;
	bool isPlayersTurn;
};

Player players[2];
TcpSocket* room[2] = { nullptr, nullptr };
int kill_counters[2] = { 0, 0 };


void sendField(Packet& packet, int turn)
{
	if (kill_counters[turn] >= 12)
	{
		packet.clear();
		field[7][7] = -1;
		packet << true << field;
		room[turn]->send(packet);
		swap(players[0].isPlayersTurn, players[1].isPlayersTurn);

		packet.clear();
		field[7][7] = -2;
		packet << field;
		room[turn == 0 ? 1 : 0]->send(packet);
	}

	packet.clear();
	packet << true << field;
	room[turn]->send(packet);
	swap(players[0].isPlayersTurn, players[1].isPlayersTurn);

	packet.clear();
	packet << field;
	room[turn == 0 ? 1 : 0]->send(packet);
}

void game()
{
	CheckerCoords selected, desired;
	Packet packet;
	while (true)
	{
		int turn = (players[0].isPlayersTurn ? 0 : 1);

		room[turn]->receive(packet);
		packet >> selected >> desired;

		std::cout << "SELECTED: " << selected.x << " " << selected.y << '\n';
		std::cout << "DESIRED: " << desired.x << " " << desired.y << '\n';

		int& selectedF = field[selected.y][selected.x];
		int& desiredF = field[desired.y][desired.x];

		std::cout << "selectedF: " << selectedF << '\n';
		std::cout << "desiredF: " << desiredF << '\n';

		packet.clear();

		// Если игрок красных
		if (players[turn].regular == RED)
		{	// красная шашка
			if (selectedF == RED)
			{
				if ((desiredF == BLANK) && (selected.y - 1 == desired.y)
					&& (selected.x == desired.x - 1 || selected.x == desired.x + 1))
				{
					if (desired.y == 0)
					{
						selectedF = BLANK;
						desiredF = RED_Q;

						sendField(packet, turn);
					}
					else
					{
						selectedF = BLANK;
						desiredF = RED;

						sendField(packet, turn);
					}
				}
				else if (desiredF == BLACK || desiredF == BLACK_Q)
				{
					short side = selected.x - desired.x;
					short up = selected.y - desired.y;

					if (up == 1)
					{
						if (side == 1)
						{
							if ((field[(desired.y) - 1][(desired.x) - 1] == BLANK) && ((desired.y) - 1 >= 0) && ((desired.x) - 1 >= 0))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								if ((desired.y) - 1 == 0)
								{
									field[(desired.y) - 1][(desired.x) - 1] = RED_Q;
								}
								else
								{
									field[(desired.y) - 1][(desired.x) - 1] = RED;
								}

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
						else if (side == -1)
						{
							if ((field[(desired.y) - 1][(desired.x) + 1] == BLANK) && ((desired.x) - 1 >= 0) && ((desired.y) + 1 <= 7))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								if ((desired.y) - 1 == 0)
								{
									field[(desired.y) - 1][(desired.x) + 1] = RED_Q;
								}
								else
								{
									field[(desired.y) - 1][(desired.x) + 1] = RED;
								}

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
						else
						{
							packet.clear();
							packet << false;
							room[turn]->send(packet);
						}
					}
					else
					{
						packet.clear();
						packet << false;
						room[turn]->send(packet);
					}
				}
				else if ((selected.y == desired.y) && (selected.x == desired.x))
				{
					packet.clear();
					packet << false;
					room[turn]->send(packet);
				}
				else
				{
					packet.clear();
					packet << false;
					room[turn]->send(packet);
				}
			}
			// ход красной дамки
			else if (selectedF == RED_Q)
			{
				if ((desiredF == BLANK) && ((selected.y - 1 == desired.y) || (selected.y + 1 == desired.y))
					&& (selected.x == desired.x - 1 || selected.x == desired.x + 1))

				{
					selectedF = BLANK;
					desiredF = RED_Q;

					sendField(packet, turn);
				}
				else if (desiredF == BLACK || desiredF == BLACK_Q)
				{
					short side = selected.x - desired.x;
					short up = selected.y - desired.y;

					if (up == 1)
					{
						if (side == 1)
						{
							if ((field[(desired.y) - 1][(desired.x) - 1] == BLANK) && ((desired.y) - 1 >= 0) && ((desired.x) - 1 >= 0))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								field[(desired.y) - 1][(desired.x) - 1] = RED_Q;

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
						else if (side == -1)
						{
							if ((field[(desired.y) - 1][(desired.x) + 1] == BLANK) && ((desired.x) - 1 >= 0) && ((desired.y) + 1 <= 7))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								field[(desired.y) - 1][(desired.x) + 1] = RED_Q;

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
						else
						{
							packet.clear();
							packet << false;
							room[turn]->send(packet);
						}
					}
					else if (up == -1)
					{
						if (side == 1)
						{
							if ((field[(desired.y) + 1][(desired.x) - 1] == BLANK) && ((desired.y) + 1 <= 7) && ((desired.x) - 1 >= 0))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								field[(desired.y) + 1][(desired.x) - 1] = RED_Q;

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
						else if (side == -1)
						{
							if ((field[(desired.y) + 1][(desired.x) + 1] == BLANK) && ((desired.x) + 1 <= 7) && ((desired.y) + 1 <= 7))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								field[(desired.y) + 1][(desired.x) + 1] = RED_Q;

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
					}
					else
					{
						packet.clear();
						packet << false;
						room[turn]->send(packet);
					}
				}
				else if ((selected.y == desired.y) && (selected.x == desired.x))
				{
					packet.clear();
					packet << false;
					room[turn]->send(packet);
				}
				else
				{
					packet.clear();
					packet << false;
					room[turn]->send(packet);
				}
			}
		}
		// Если игрок чёрных
		else
		{
			// ход черной шашки
			if (selectedF == BLACK)
			{
				if ((desiredF == BLANK) && (selected.y + 1 == desired.y)
					&& (selected.x == desired.x - 1 || selected.x == desired.x + 1))
				{
					if (desired.y == 7)
					{
						selectedF = BLANK;
						desiredF = BLACK_Q;

						sendField(packet, turn);
					}
					else
					{
						selectedF = BLANK;
						desiredF = BLACK;


						sendField(packet, turn);
					}
				}
				else if (desiredF == RED || desiredF == RED_Q)
				{
					short side = selected.x - desired.x;
					short up = selected.y - desired.y;

					if (up == -1)
					{
						if (side == 1)
						{
							if ((field[(desired.y) + 1][(desired.x) - 1] == BLANK) && ((desired.y) + 1 <= 7) && ((desired.x) - 1 >= 0))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								if ((desired.y) + 1 == 7)
								{
									field[(desired.y) + 1][(desired.x) - 1] = BLACK_Q;
								}
								else
								{
									field[(desired.y) + 1][(desired.x) - 1] = BLACK;
								}


								kill_counters[turn]++;


								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
						else if (side == -1)
						{
							if ((field[(desired.y) + 1][(desired.x) + 1] == BLANK) && ((desired.x) + 1 <= 7) && ((desired.y) + 1 <= 7))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								if ((desired.y) + 1 == 7)
								{
									field[(desired.y) + 1][(desired.x) + 1] = BLACK_Q;
								}
								else
								{
									field[(desired.y) + 1][(desired.x) + 1] = BLACK;
								}

								kill_counters[turn]++;

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}

						}
						else
						{
							packet.clear();
							packet << false;
							room[turn]->send(packet);
						}
					}
					else
					{
						packet.clear();
						packet << false;
						room[turn]->send(packet);
					}
				}
				else
				{
					packet.clear();
					packet << false;
					room[turn]->send(packet);
				}

			}
			// черная дамка 
			else if (selectedF == BLACK_Q)
			{
				if ((desiredF == BLANK) && ((selected.y + 1 == desired.y) || (selected.y - 1 == desired.y))
					&& (selected.x == desired.x - 1 || selected.x == desired.x + 1))
				{

					selectedF = BLANK;
					desiredF = BLACK_Q;

					sendField(packet, turn);

				}
				else if (desiredF == RED || desiredF == RED_Q)
				{
					short side = selected.x - desired.x;
					short up = selected.y - desired.y;

					if (up == -1)
					{
						if (side == 1)
						{
							if ((field[(desired.y) + 1][(desired.x) - 1] == BLANK) && ((desired.y) + 1 <= 7) && ((desired.x) - 1 >= 0))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								field[(desired.y) + 1][(desired.x) - 1] = BLACK_Q;

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
						else if (side == -1)
						{
							if ((field[(desired.y) + 1][(desired.x) + 1] == BLANK) && ((desired.x) + 1 <= 7) && ((desired.y) + 1 <= 7))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								field[(desired.y) + 1][(desired.x) + 1] = BLACK_Q;

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}

						}
						else if (up == 1)
						{

						}
						else
						{
							packet.clear();
							packet << false;
							room[turn]->send(packet);
						}
					}
					else if (up == 1)
					{
						if (side == 1)
						{
							if ((field[(desired.y) - 1][(desired.x) - 1] == BLANK) && ((desired.y) - 1 >= 0) && ((desired.x) - 1 >= 0))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								field[(desired.y) - 1][(desired.x) - 1] = BLACK_Q;

								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
						else if (side == -1)
						{
							if ((field[(desired.y) - 1][(desired.x) + 1] == BLANK) && ((desired.x) - 1 >= 0) && ((desired.y) + 1 <= 7))
							{
								desiredF = BLANK;
								selectedF = BLANK;

								kill_counters[turn]++;

								field[(desired.y) - 1][(desired.x) + 1] = BLACK_Q;


								sendField(packet, turn);
							}
							else
							{
								packet.clear();
								packet << false;
								room[turn]->send(packet);
							}
						}
						else
						{
							packet.clear();
							packet << false;
							room[turn]->send(packet);
						}
					}
					else
					{
						packet.clear();
						packet << false;
						room[turn]->send(packet);
					}
				}
				else
				{
					packet.clear();
					packet << false;
					room[turn]->send(packet);
				}
			}
		}

		if (kill_counters[turn] >= 12)
		{
			packet.clear();
			field[0][0] = -1;
			room[turn]->send(packet);

			packet.clear();
			field[0][0] = -2;
			room[turn == 0 ? 1 : 0]->send(packet);
			break;
		}
	}
	cout << "\nИгра окончена.\n";
}

int main()
{
	setlocale(LC_ALL, "Russian");
	cout << "Checkers server v1.0 (beta)\n";
	vector<TcpSocket*> sockets;

	SocketSelector selector;
	TcpListener listener;

	if (listener.listen(port) != Socket::Done)
	{
		cout << "Не могу прослушать порт.\n";
		return 1;
	}
	cout << "Сервер запущен на " << port << "\n<3\n";

	selector.add(listener);

	//-------------------------------------
	while (true)
	{
		if (selector.wait())
		{
			if (selector.isReady(listener))
			{
				TcpSocket* sock = new TcpSocket;
				if (listener.accept(*sock) != Socket::Done)
				{
					cout << "Ошибка при подключении клиента\n";
					return 1;
				}
				else
				{
					cout << "Новый сокет\n";
					selector.add(*sock);
					sockets.push_back(sock);
				}
			}
			else
			{
				for (int i = 0; i < sockets.size(); i++)
				{
					if (selector.isReady(*sockets[i]))
					{
						Packet packet;
						string action;
						switch (sockets[i]->receive(packet))
						{
						case Socket::Done:
							packet >> action;

							if (action == "connect")
							{
								packet.clear();
								if (room[0] == nullptr)
								{
									cout << "Зашел первый игрок.\n";
									room[0] = sockets[i];
									packet << true;
									sockets[i]->send(packet);
								}
								else if (room[1] == nullptr)
								{
									cout << "Зашел второй игрок.\n";
									room[1] = sockets[i];
									packet << true;
									sockets[i]->send(packet);

									// Инициализируем игру
									packet.clear();
									resetField();
									packet << field << RED << RED_Q << true;
									players[0] = { RED, RED_Q, true };
									room[0]->send(packet);

									packet.clear();
									packet << field << BLACK << BLACK_Q << false;
									players[1] = { BLACK, BLACK_Q, false };
									room[1]->send(packet);

									selector.remove(*room[0]);
									selector.remove(*room[1]);
									// Начинаем игру на отдельном потоке
									thread game_thread(game);
									game_thread.detach();
								}
								else
								{
									cout << "Третий игрок попытался подключиться!\n";
									packet << false;
									sockets[i]->send(packet);
								}
							}

							break;

						case Socket::Disconnected:
							selector.remove(*sockets[i]);
							delete sockets[i];
							sockets.erase(sockets.begin() + i);
							break;
						}
					}
				}
			}
		}
	}

	return 0;
}
