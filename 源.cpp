#include<iostream>
#include<windows.h>
#include<stdlib.h>
#include<time.h>
#include<conio.h>
#include"Tank.h"
using namespace std;

int GameOver = 0;
int map[25][50];

//坦克和子弹不同种类
#define Player1 1
#define Player2 2
#define Player1Bullet 3
#define Player2Bullet 4
#define Enemy1 5
#define Enemy2 6
#define Enemy1Bullet 7
#define Enemy2Bullet 8

//坦克种类
#define FriendTank 0
#define EnemyTank 1

//方向
#define Up 0
#define Down 1
#define Left 2
#define Right 3

int Player1Kill;
int Player2Kill;
int Enemy1Exist;
int Enemy2Exist;
int speed = 50;

//互斥对象
HANDLE Mutex = CreateMutex(NULL, FALSE, NULL);
//子弹函数申明
DWORD WINAPI Bullet(LPVOID lpParameter);
//更新界面
void Updata();
void Settle();

Tank player1(Up, 0, 0, FriendTank, 1);
Tank player2(Up, 0, 0, FriendTank, 1);
Tank enemy1(Up, 0, 0, EnemyTank, 1);
Tank enemy2(Down, 0, 0, EnemyTank, 1);

//设定光标位置
void SetPos(int i, int j)
{
	COORD pos = { i,j };
	HANDLE Out = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(Out, pos);
}

//隐藏光标
void HideCurSor()
{
	CONSOLE_CURSOR_INFO info = { 1,0 };
	HANDLE Out = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(Out, &info);
}

int shape[4][12] =
{
	{0,1,1,0,1,1,1,2,2,0,2,2},
	{0,0,0,2,1,0,1,1,1,2,2,1},
	{0,1,0,2,1,0,1,1,2,1,2,2},
	{0,0,0,1,1,1,1,2,2,0,2,1},
};

//画坦克
void Tank::DrawTank()
{
	int nx, ny;
	if (Type == EnemyTank)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
	if (ID == Player1)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	if (ID == Player2)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
	for (int i = 0; i < 6; i++)
	{
		nx = point[0] + shape[Direction][i * 2];
		ny = point[1] + shape[Direction][i * 2 + 1];
		SetPos((ny + 1) * 2, nx + 1);//利用shape数组相对于点x,y绘制形状
		map[nx][ny] = ID;
		cout << "■";
	}
}

//消除坦克
void Tank::Redraw()
{
	int nx, ny;
	for (int i = 0; i < 6; i++)
	{
		nx = point[0] + shape[Direction][i * 2];
		ny = point[1] + shape[Direction][i * 2 + 1];
		map[nx][ny] = 0;
		SetPos((ny + 1) * 2, nx + 1);
		cout << " ";
	}
}

//判断是否能画坦克
int Tank::Judge(int x, int y, int dir)
{
	int nx, ny;
	for (int i = 0; i < 6; i++)
	{
		nx = x + shape[dir][i * 2];
		ny = y + shape[dir][i * 2 + 1];
		if (nx < 0 || nx >= 25 || ny < 0 || ny >= 50 || map[nx][ny] != 0)
			return 1;//不能绘制，返回1
	}
	return 0;
}

//第一个敌方坦克自动移动和射击
void Tank::Enemy1move()
{
	int m_dir;
	while (1)
	{
		if (Life == 0)
		{
			Enemy1Exist = 0;
			return;
		}

		if (GameOver == 2)
			return;
		if (FireEnable == 1 && GameOver != 2)//如果可以射击
		{
			WaitForSingleObject(Mutex, INFINITE);
			FireEnable = 0;//0为不能射击
			HANDLE bullet = CreateThread(NULL, 0, Bullet, &ID, 0, NULL);//创建子弹线程
			CloseHandle(bullet);
			ReleaseMutex(Mutex);
			Sleep(100);
		}
		WaitForSingleObject(Mutex, INFINITE);//线程拥有互斥对象
		srand(time(0));
		m_dir = rand() % 4;

		if (m_dir == Up)//随机出方向并重画坦克
		{
			Redraw();
			if (Judge(point[0] - 1, point[1], m_dir) == 0)
			{
				point[0]--;
				Direction = m_dir;
			}
			else
			{
				if (Judge(point[0], point[1], m_dir) == 0)
					Direction = m_dir;
			}
		}
		if (m_dir == Down)
		{
			Redraw();
			if (Judge(point[0] + 1, point[1], m_dir) == 0)
			{
				point[0]++;
				Direction = m_dir;
			}
			else
			{
				if (Judge(point[0], point[1], m_dir) == 0)
					Direction = m_dir;
			}
		}
		if (m_dir == Left)
		{
			Redraw();
			if (Judge(point[0], point[1] - 1, m_dir) == 0)
			{
				point[1]--;
				Direction = m_dir;
			}
			else
			{
				if (Judge(point[0], point[1], m_dir) == 0)
					Direction = m_dir;
			}
		}
		if (m_dir == Right)
		{
			Redraw();
			if (Judge(point[0], point[1] + 1, m_dir) == 0)
			{
				point[1]++;
				Direction = m_dir;
			}
			else
			{
				if (Judge(point[0], point[1], m_dir) == 0)
					Direction = m_dir;
			}
		}
		if (GameOver != 2 && Life != 0)
			DrawTank();
		ReleaseMutex(Mutex);//释放互斥对象
		Sleep(500 - 80 * Speed);
	}
}

//第二个敌方坦克自动移动和射击
void Tank::Enemy2move()
{
	int m_dir;
	while (1)
	{
		if (Life == 0)
		{
			Enemy2Exist = 0;
			return;
		}

		if (GameOver == 2)
			return;
		if (FireEnable == 1 && GameOver != 2)
		{
			WaitForSingleObject(Mutex, INFINITE);
			FireEnable = 0;
			HANDLE bullet = CreateThread(NULL, 0, Bullet, &ID, 0, NULL);
			CloseHandle(bullet);
			ReleaseMutex(Mutex);
			Sleep(100);
		}
		WaitForSingleObject(Mutex, INFINITE);
		srand(time(0));
		m_dir = rand() % 4;

		if (m_dir == 2)
		{
			Redraw();
			if (Judge(point[0] - 1, point[1], 0) == 0)
			{
				point[0]--;
				Direction = 0;
			}
			else
			{
				if (Judge(point[0], point[1], 0) == 0)
					Direction = 0;
			}
		}
		if (m_dir == 0)
		{
			Redraw();
			if (Judge(point[0] + 1, point[1], 1) == 0)
			{
				point[0]++;
				Direction = 1;
			}
			else
			{
				if (Judge(point[0], point[1], 1) == 0)
					Direction = 1;
			}
		}
		if (m_dir == 3)
		{
			Redraw();
			if (Judge(point[0], point[1] - 1, 2) == 0)
			{
				point[1]--;
				Direction = 2;
			}
			else
			{
				if (Judge(point[0], point[1], 2) == 0)
					Direction = 2;
			}
		}
		if (m_dir == 1)
		{
			Redraw();
			if (Judge(point[0], point[1] + 1, 3) == 0)
			{
				point[1]++;
				Direction = 3;
			}
			else
			{
				if (Judge(point[0], point[1], 3) == 0)
					Direction = 3;
			}
		}
		if (GameOver != 2 && Life != 0)
			DrawTank();
		ReleaseMutex(Mutex);
		Sleep(500 - 80 * Speed);
	}
}

//子弹线程函数
DWORD WINAPI Bullet(LPVOID lpParameter)
{
	int *shooter = (int *)lpParameter;
	int Pos[2];
	int direction;//子弹发射的方向和坦克发射子弹是的方向相同
	int Speed;//子弹的射速
	int type;//用来判断敌我子弹
	int hit = 0;//用来判断子弹是否击中
	int oldx, oldy;//子弹发射点坐标
	int flag = 0;//用来判断子弹是否能移动
	if (*shooter == Player1)
	{
		type = Player1Bullet;
		direction = player1.GetDirection();
		Speed = player1.GetBulletspeed();
		Pos[0] = player1.GetpointX();
		Pos[1] = player1.GetpointY();
	}
	if (*shooter == Player2)
	{
		type = Player2Bullet;
		direction = player2.GetDirection();
		Speed = player2.GetBulletspeed();
		Pos[0] = player2.GetpointX();
		Pos[1] = player2.GetpointY();
	}
	if (*shooter == Enemy1)
	{
		type = Enemy1Bullet;
		direction = enemy1.GetDirection();
		Speed = enemy1.GetBulletspeed();
		Pos[0] = enemy1.GetpointX();
		Pos[1] = enemy1.GetpointY();
	}
	if (*shooter == Enemy2)
	{
		type = Enemy2Bullet;
		direction = enemy2.GetDirection();
		Speed = enemy2.GetBulletspeed();
		Pos[0] = enemy2.GetpointX();
		Pos[1] = enemy2.GetpointY();
	}
	if (direction == Up)//根据坦克的位置和方向确定子弹的初始坐标
	{
		Pos[0]--;
		Pos[1]++;
	}
	if (direction == Down)
	{
		Pos[0] += 3;
		Pos[1]++;
	}
	if (direction == Left)
	{
		Pos[0]++;
		Pos[1]--;
	}
	if (direction == Right)
	{
		Pos[0]++;
		Pos[1] += 3;
	}
	while (1)
	{
		WaitForSingleObject(Mutex, INFINITE);
		if (flag == 1 && hit != 1)//消除子弹原来的位置
		{
			map[oldx][oldy] = 0;
			SetPos((oldy + 1) * 2, oldx + 1);
			cout << " ";
		}
		if (GameOver == 2)
			return 0;
		if (hit == 1 || Pos[0] < 0 || Pos[0]>24 || Pos[1] < 0 || Pos[1]>49)//如果击中
		{
			ReleaseMutex(Mutex);
			Sleep(500);
			if (type == Player1Bullet)
			{
				player1.FireEnable = 1;
			}
			if (type == Player2Bullet)
			{
				player2.FireEnable = 1;
			}
			if (type = Enemy1Bullet)
			{
				enemy1.FireEnable = 1;
			}
			if (type = Enemy2Bullet)
			{
				enemy2.FireEnable = 1;
			}
			break;
		}
		switch (map[Pos[0]][Pos[1]])
		{

		case 0://如果是空位置就画出子弹
			map[Pos[0]][Pos[1]] = type;
			SetPos((Pos[1] + 1) * 2, Pos[0] + 1);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_RED);
			cout << "◎";
			break;
		case Player1://玩家1
			if (type == Enemy1Bullet || type == Enemy2Bullet)
			{
				player1.Life--;
				if (player1.Life <= 0)
				{
					GameOver++;
					player1.Redraw();
				}
			}
			Updata();
			hit = 1;
			break;

		case Player2://玩家2
			if (type == Enemy1Bullet || type == Enemy2Bullet)
			{
				player2.Life--;
				if (player2.Life <= 0)
				{
					GameOver++;
					player2.Redraw();
				}
			}
			Updata();
			hit = 1;
			break;

		case Enemy1://敌人1
			if (type == Player1Bullet)
			{
				hit = 1;
				Player1Kill++;
				if (Player1Kill % 10 == 0 && player1.Life < 5)
				{
					player1.Life++;
				}

				if (Player1Kill % 5 == 0 && player1.GetBulletspeed() < 5)
				{
					player1.IncreaseBulletspeed();
				}
				enemy1.Redraw();
				enemy1.Life = 0;
			}
			if (type == Player2Bullet)
			{
				hit = 1;
				Player2Kill++;
				if (Player2Kill % 10 == 0 && player2.Life < 5)
				{
					player2.Life++;
				}

				if (Player2Kill % 5 == 0 && player2.GetBulletspeed() < 5)
				{
					player2.IncreaseBulletspeed();
				}
				enemy1.Redraw();
				enemy1.Life = 0;
			}
			Updata();
			hit = 1;
			break;
		case Enemy2://敌人2
			if (type == Player1Bullet)
			{
				hit = 1;
				Player1Kill++;
				if (Player1Kill % 10 == 0 && player1.Life < 5)
				{
					player1.Life++;
				}

				if (Player1Kill % 5 == 0 && player1.GetBulletspeed() < 5)
				{
					player1.IncreaseBulletspeed();
				}
				enemy2.Redraw();
				enemy2.Life = 0;
			}
			if (type == Player2Bullet)
			{
				hit = 1;
				Player2Kill++;
				if (Player2Kill % 10 == 0 && player2.Life < 5)
				{
					player2.Life++;
				}

				if (Player2Kill % 5 == 0 && player2.GetBulletspeed() < 5)
				{
					player2.IncreaseBulletspeed();
				}
				enemy2.Redraw();
				enemy2.Life = 0;
			}
			Updata();
			hit = 1;
			break;
		}
		oldx = Pos[0];
		oldy = Pos[1];
		if (direction == Up)
			Pos[0]--;
		if (direction == Down)
			Pos[0]++;
		if (direction == Left)
			Pos[1]--;
		if (direction == Right)
			Pos[1]++;
		ReleaseMutex(Mutex);
		flag = 1;
		Sleep(60 - 10 * Speed);
	}
	return 0;
}

//敌人1线程函数
DWORD WINAPI FirstEnemy(LPVOID lpParameter)
{
	Sleep(400);
	int random1;
	int birthplace1[2];//敌人起始坐标
	int typ;
	int bulletspe;

	while (1)
	{
		if (GameOver == 2)
			return 0;
		srand(time(0));
		random1 = rand() % 3;
		if (random1 == 0)
		{
			birthplace1[0] = 2;
			birthplace1[0] = 2;
		}
		if (random1 == 1)
		{
			birthplace1[0] = 20;
			birthplace1[1] = 22;
		}
		if (random1 == 2)
		{
			birthplace1[0] = 2;
			birthplace1[1] = 42;
		}

		if (player1.Judge(birthplace1[0], birthplace1[1], Down) == 0)
			break;
		if (player2.Judge(birthplace1[0], birthplace1[1], Down) == 0)
			break;
	}
	WaitForSingleObject(Mutex, INFINITE);
	typ = 1;
	bulletspe = 3;

	enemy1 = Tank(Right, birthplace1[0], birthplace1[1], typ, bulletspe);//再次生成敌人1
	enemy1.ID = Enemy1;
	enemy1.Life = 1;
	enemy1.FireEnable = 1;
	ReleaseMutex(Mutex);

	enemy1.Enemy1move();
	return 0;
}

//敌人2线程函数
DWORD WINAPI SecondEnemy(LPVOID lpParameter)
{
	Sleep(400);
	int random2;
	int birthplace2[2];
	int typ;
	int bulletspe;

	while (1)
	{
		if (GameOver == 2)
			return 0;
		srand(time(0));
		random2 = rand() % 3;
		if (random2 == 0)
		{
			birthplace2[0] = 22;
			birthplace2[0] = 2;
		}
		if (random2 == 1)
		{
			birthplace2[0] = 2;
			birthplace2[1] = 20;
		}
		if (random2 == 2)
		{
			birthplace2[0] = 18;
			birthplace2[1] = 40;
		}

		if (player1.Judge(birthplace2[0], birthplace2[1], Down) == 0)
			break;
		if (player2.Judge(birthplace2[0], birthplace2[1], Down) == 0)
			break;
	}

	WaitForSingleObject(Mutex, INFINITE);
	typ = 1;
	bulletspe = 3;

	enemy2 = Tank(Down, birthplace2[0], birthplace2[1], typ, bulletspe);
	enemy2.ID = Enemy2;
	enemy2.Life = 1;
	enemy2.FireEnable = 1;
	ReleaseMutex(Mutex);

	enemy2.Enemy2move();
	return 0;
}

//初始化数据
void Init()
{
	Player1Kill = 0;
	Player2Kill = 0;
	player1 = Tank(Up, 11, 12, FriendTank, 1);
	player2 = Tank(Up, 11, 32, FriendTank, 1);
	enemy1 = Tank(Up, 0, 0, EnemyTank, 1);
	enemy2 = Tank(Left, 0, 0, EnemyTank, 1);
	player1.Life = 2;
	player2.Life = 2;
	player1.FireEnable = 1;
	player2.FireEnable = 1;
	enemy1.Life = 0;
	enemy2.Life = 0;
	enemy1.FireEnable = 1;
	enemy2.FireEnable = 1;
	player1.ID = Player1;
	player2.ID = Player2;
	enemy1.ID = Enemy1;
	enemy2.ID = Enemy2;
	Enemy1Exist = 0;
	Enemy2Exist = 0;
}

//更新界面
void Updata()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	int i;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	SetPos(105, 1);
	cout << "P1生命值：";
	SetPos(105, 2);
	for (i = 0; i < 5; i++)
	{
		if (i < player1.Life)
			cout << "■";
		else
			cout << " ";
	}
	SetPos(105, 3);
	cout << "P1杀敌数：";
	SetPos(105, 4);
	cout << Player1Kill;
	SetPos(105, 5);
	cout << "P1射速：";
	SetPos(105, 6);
	for (i = 0; i < 5; i++)
	{
		if (i < player1.GetBulletspeed())
			cout << "■";
		else
			cout << " ";
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
	SetPos(105, 11);
	cout << "P2生命值：";
	SetPos(105, 12);
	for (i = 0; i < 5; i++)
	{
		if (i < player2.Life)
			cout << "■";
		else
			cout << " ";
	}
	SetPos(105, 13);
	cout << "P2杀敌数：";
	SetPos(105, 14);
	cout << Player2Kill;
	SetPos(105, 15);
	cout << "P2射速：";
	SetPos(105, 16);
	for (i = 0; i < 5; i++)
	{
		if (i < player2.GetBulletspeed())
			cout << "■";
		else
			cout << " ";
	}
}

//画地图
void DrawMap()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	system("cls");
	for (int i = 0; i < 52; i++)
	{
		SetPos(i * 2, 0);
		cout << "■";
	}
	for (int i = 1; i < 27; i++)
	{
		SetPos(0, i);
		cout << "■";
		SetPos(102, i);
		cout << "■";
	}
	for (int i = 0; i < 52; i++)
	{
		SetPos(i * 2, 26);
		cout << "■";
	}
	SetPos(5, 28);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN);
	cout << "每击杀五个敌方坦克增加一格射速     每击杀十个敌方坦克增加一格生命值";

	Updata();
}

//开始界面
void Startgame()
{
	system("cls");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	SetPos(10, 5);
	for (int i = 0; i < 50; i++)
		cout << "■";
	SetPos(10, 6);
	cout << "■                                                                                                ■";
	SetPos(10, 7);
	cout << "■                                           坦克大战双人版                                       ■";
	SetPos(10, 8);
	cout << "■                                                                                                ■";
	SetPos(10, 9);
	for (int i = 0; i < 50; i++)
		cout << "■";
	SetPos(10, 10);
	cout << "■                                                                                                ■";
	SetPos(10, 11);
	cout << "■       ■■■■■■■              ■               ■          ■          ■     ■           ■";
	SetPos(10, 12);
	cout << "■             ■                  ■  ■             ■■        ■          ■   ■             ■";
	SetPos(10, 13);
	cout << "■             ■                 ■    ■            ■  ■      ■          ■ ■               ■";
	SetPos(10, 14);
	cout << "■             ■                ■■■■■           ■    ■    ■          ■  ■              ■";
	SetPos(10, 15);
	cout << "■             ■               ■        ■          ■      ■  ■          ■    ■            ■";
	SetPos(10, 16);
	cout << "■             ■              ■          ■         ■        ■■          ■       ■         ■";
	SetPos(10, 17);
	cout << "■                                                                                                ■";
	SetPos(10, 18);
	for (int i = 0; i < 50; i++)
		cout << "■";
	SetPos(10, 19);
	cout << "■                                                 ■                                             ■";
	SetPos(10, 20);
	cout << "■          P1:WASD键移动 空格键攻击               ■                 谯钰霖                      ■";
	SetPos(10, 21);
	cout << "■                                                 ■                                             ■";
	SetPos(10, 22);
	cout << "■          P2:方向键移动 “.”键攻击              ■                 何岷勋   作                 ■";
	SetPos(10, 23);
	cout << "■                                                 ■                                             ■";
	SetPos(10, 24);
	for (int i = 0; i < 50; i++)
		cout << "■";
	SetPos(52, 26);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
	cout << "按空格键开始游戏";
	while (1)
	{
		if (GetAsyncKeyState(VK_SPACE))
		{
			break;
		}
	}
}


//结算界面
void Settle()
{
	system("cls");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	SetPos(10, 3);
	for (int i = 0; i < 50; i++)
		cout << "■";
	SetPos(10, 4);
	cout << "■                                                                                                ■";
	SetPos(10, 5);
	cout << "■           ■■■■               ■                ■          ■          ■■■■■■        ■";
	SetPos(10, 6);
	cout << "■         ■        ■           ■  ■              ■■      ■■          ■                  ■";
	SetPos(10, 7);
	cout << "■        ■                     ■    ■             ■ ■    ■ ■          ■■■■■■        ■";
	SetPos(10, 8);
	cout << "■        ■       ■■         ■■■■■            ■  ■  ■  ■          ■                  ■";
	SetPos(10, 9);
	cout << "■         ■         ■       ■        ■           ■  ■  ■  ■          ■                  ■";
	SetPos(10, 10);
	cout << "■           ■■■■■       ■          ■          ■   ■■   ■          ■■■■■■        ■";
	SetPos(10, 11);
	cout << "■                                                                                                ■";
	SetPos(10, 12);
	cout << "■           ■■■■         ■          ■          ■■■■■■■          ■■■■■          ■";
	SetPos(10, 13);
	cout << "■         ■        ■        ■        ■           ■                      ■       ■         ■";
	SetPos(10, 14);
	cout << "■       ■            ■       ■      ■            ■■■■■■■          ■ ■■■           ■";
	SetPos(10, 15);
	cout << "■       ■            ■        ■    ■             ■                      ■  ■              ■";
	SetPos(10, 16);
	cout << "■        ■          ■          ■  ■              ■                      ■    ■            ■";
	SetPos(10, 17);
	cout << "■           ■■■■               ■                ■■■■■■■          ■       ■         ■";
	SetPos(10, 18);
	cout << "■                                                                                                ■";
	SetPos(10, 19);
	for (int i = 0; i < 50; i++)
		cout << "■";
	SetPos(10, 20);
	cout << "■                                                 ■                                             ■";
	SetPos(10, 21);
	cout << "            P1杀敌:";
	cout << Player1Kill;
	cout << "              总杀敌：";
	SetPos(10, 22);
	cout << "■                                                 ■                                             ■";
	SetPos(10, 23);
	cout << "            P2杀敌:";
	cout << Player2Kill;
	cout << "                 ";
	cout << Player1Kill + Player2Kill;
	SetPos(10, 24);
	cout << "■                                                 ■                                             ■";
	SetPos(10, 25);
	for (int i = 0; i < 50; i++)
		cout << "■";
	SetPos(74, 22);
}


int main()
{
	Init();
	HideCurSor();
	Startgame();
	DrawMap();
	HANDLE temp1;
	HANDLE temp2;
	int m_dir;
	player1.DrawTank();
	player2.DrawTank();
	while (GameOver != 2)
	{
		if (GetAsyncKeyState('W'))//玩家1上移
		{
			if (player1.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				m_dir = Up;
				player1.Redraw();
				if (player1.Judge(player1.GetpointX() - 1, player1.GetpointY(), m_dir) == 0)
				{
					player1.ChangePos(player1.GetpointX() - 1, player1.GetpointY());
					player1.ChangeDirection(m_dir);
				}
				else
				{
					if (player1.Judge(player1.GetpointX(), player1.GetpointY(), m_dir) == 0)
						player1.ChangeDirection(m_dir);
				}
				if (GameOver != 2)
					player1.DrawTank();
				ReleaseMutex(Mutex);
			}
		}

		if (GetAsyncKeyState(VK_UP))//玩家2上移
		{
			if (player2.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				m_dir = Up;
				player2.Redraw();
				if (player2.Judge(player2.GetpointX() - 1, player2.GetpointY(), m_dir) == 0)//移动玩家坦克，原理和敌人函数一样
				{
					player2.ChangePos(player2.GetpointX() - 1, player2.GetpointY());
					player2.ChangeDirection(m_dir);
				}
				else
				{
					if (player2.Judge(player2.GetpointX(), player2.GetpointY(), m_dir) == 0)
						player2.ChangeDirection(m_dir);
				}
				if (GameOver != 2)
					player2.DrawTank();
				ReleaseMutex(Mutex);
			}
		}

		if (GetAsyncKeyState('S'))//玩家1下移
		{
			if (player1.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				m_dir = Down;
				player1.Redraw();
				if (player1.Judge(player1.GetpointX() + 1, player1.GetpointY(), m_dir) == 0)
				{
					player1.ChangePos(player1.GetpointX() + 1, player1.GetpointY());
					player1.ChangeDirection(m_dir);
				}
				else
				{
					if (player1.Judge(player1.GetpointX(), player1.GetpointY(), m_dir) == 0)
						player1.ChangeDirection(m_dir);
				}
				if (GameOver != 2)
					player1.DrawTank();
				ReleaseMutex(Mutex);
			}
		}

		if (GetAsyncKeyState(VK_DOWN))//玩家2下移
		{
			if (player2.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				m_dir = Down;
				player2.Redraw();
				if (player2.Judge(player2.GetpointX() + 1, player2.GetpointY(), m_dir) == 0)
				{
					player2.ChangePos(player2.GetpointX() + 1, player2.GetpointY());
					player2.ChangeDirection(m_dir);
				}
				else
				{
					if (player2.Judge(player2.GetpointX(), player2.GetpointY(), m_dir) == 0)
						player2.ChangeDirection(m_dir);
				}
				if (GameOver != 2)
					player2.DrawTank();
				ReleaseMutex(Mutex);
			}
		}

		if (GetAsyncKeyState('D'))//玩家1右移
		{
			if (player1.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				m_dir = Right;
				player1.Redraw();
				if (player1.Judge(player1.GetpointX(), player1.GetpointY() + 1, m_dir) == 0)
				{
					player1.ChangePos(player1.GetpointX(), player1.GetpointY() + 1);
					player1.ChangeDirection(m_dir);
				}
				else
				{
					if (player1.Judge(player1.GetpointX(), player1.GetpointY(), m_dir) == 0)
						player1.ChangeDirection(m_dir);
				}
				if (GameOver != 2)
					player1.DrawTank();
				ReleaseMutex(Mutex);
			}
		}

		if (GetAsyncKeyState(VK_RIGHT))//玩家2右移
		{
			if (player2.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				m_dir = Right;
				player2.Redraw();
				if (player2.Judge(player2.GetpointX(), player2.GetpointY() + 1, m_dir) == 0)
				{
					player2.ChangePos(player2.GetpointX(), player2.GetpointY() + 1);
					player2.ChangeDirection(m_dir);
				}
				else
				{
					if (player2.Judge(player2.GetpointX(), player2.GetpointY(), m_dir) == 0)
						player2.ChangeDirection(m_dir);
				}
				if (GameOver != 2)
					player2.DrawTank();
				ReleaseMutex(Mutex);
			}
		}

		if (GetAsyncKeyState('A'))//玩家1左移
		{
			if (player1.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				m_dir = Left;
				player1.Redraw();
				if (player1.Judge(player1.GetpointX(), player1.GetpointY() - 1, m_dir) == 0)
				{
					player1.ChangePos(player1.GetpointX(), player1.GetpointY() - 1);
					player1.ChangeDirection(m_dir);
				}
				else
				{
					if (player1.Judge(player1.GetpointX(), player1.GetpointY(), m_dir) == 0)
						player1.ChangeDirection(m_dir);
				}
				if (GameOver != 2)
					player1.DrawTank();
				ReleaseMutex(Mutex);
			}
		}

		if (GetAsyncKeyState(VK_LEFT))//玩家2左移
		{
			if (player2.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				m_dir = Left;
				player2.Redraw();
				if (player2.Judge(player2.GetpointX(), player2.GetpointY() - 1, m_dir) == 0)
				{
					player2.ChangePos(player2.GetpointX(), player2.GetpointY() - 1);
					player2.ChangeDirection(m_dir);
				}
				else
				{
					if (player2.Judge(player2.GetpointX(), player2.GetpointY(), m_dir) == 0)
						player2.ChangeDirection(m_dir);
				}
				if (GameOver != 2)
					player2.DrawTank();
				ReleaseMutex(Mutex);
			}
		}

		if (GetAsyncKeyState(VK_SPACE))//射击
		{
			if (player1.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				if (player1.FireEnable == 1)//如果可以射击
				{
					HANDLE bullet = CreateThread(NULL, 0, Bullet, &(player1.ID), 0, NULL);//创建玩家子弹进程
					CloseHandle(bullet);
					player1.FireEnable = 0;
				}
				ReleaseMutex(Mutex);
			}
		}

		if (GetAsyncKeyState(VK_DECIMAL))//射击
		{
			if (player2.Life != 0)
			{
				WaitForSingleObject(Mutex, INFINITE);
				if (player2.FireEnable == 1)//如果可以射击
				{
					HANDLE bullet = CreateThread(NULL, 0, Bullet, &(player2.ID), 0, NULL);//创建玩家子弹进程
					CloseHandle(bullet);
					player2.FireEnable = 0;
				}
				ReleaseMutex(Mutex);
			}
		}

		if (Enemy1Exist == 0 && GameOver != 2)//如果敌人1不存在生成新敌人
		{
			WaitForSingleObject(Mutex, INFINITE);
			Enemy1Exist = 1;
			temp1 = CreateThread(NULL, 0, FirstEnemy, NULL, 0, NULL);//创建敌人线程
			CloseHandle(temp1);
			ReleaseMutex(Mutex);
		}

		if (Enemy2Exist == 0 && GameOver != 2)//如果敌人2不存在生成新敌人
		{
			WaitForSingleObject(Mutex, INFINITE);
			Enemy2Exist = 1;
			temp2 = CreateThread(NULL, 0, SecondEnemy, NULL, 0, NULL);//创建敌人线程
			CloseHandle(temp2);
			ReleaseMutex(Mutex);
		}

		Sleep(200 - speed);
	}

	Settle();//结算

	system("pause");
	return 0;
}

