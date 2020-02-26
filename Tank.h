#ifndef TANK_H
#define TANK_H

class Tank
{
private:
	int Direction;
	int point[2];//坐标
	int Speed;
	int Bulletspeed;//火力

public:
	Tank(int dir, int hot1, int hot2, int typ, int bulletspee)//构造函数
	{
		Direction = dir;
		point[0] = hot1;
		point[1] = hot2;
		Type = typ;
		Bulletspeed = bulletspee;
	}
	int Type;//坦克的种类
	int ID;
	int FireEnable;//是否可以射击
	int Life;//生命
	void Enemy1move();
	void Enemy2move();
	int Judge(int x, int y, int ID);//判断是否能画坦克
	void DrawTank();//画坦克
	void Redraw();//消除坦克
	
	int GetBulletspeed()
	{
		return Bulletspeed;
	}

	int GetDirection()
	{
		return Direction;
	}

	int GetpointX()//获取坐标
	{
		return point[0];
	}

	int GetpointY()
	{
		return point[1];
	}

	void IncreaseBulletspeed()//增加射速
	{
		Bulletspeed++;
	}
	
	void ChangeDirection(int newD)//改变方向
	{
		Direction = newD;
	}

	void ChangePos(int x, int y)//改变坐标
	{
		point[0] = x;
		point[1] = y;
	}
};


#endif