#ifndef TANK_H
#define TANK_H

class Tank
{
private:
	int Direction;
	int point[2];//����
	int Speed;
	int Bulletspeed;//����

public:
	Tank(int dir, int hot1, int hot2, int typ, int bulletspee)//���캯��
	{
		Direction = dir;
		point[0] = hot1;
		point[1] = hot2;
		Type = typ;
		Bulletspeed = bulletspee;
	}
	int Type;//̹�˵�����
	int ID;
	int FireEnable;//�Ƿ�������
	int Life;//����
	void Enemy1move();
	void Enemy2move();
	int Judge(int x, int y, int ID);//�ж��Ƿ��ܻ�̹��
	void DrawTank();//��̹��
	void Redraw();//����̹��
	
	int GetBulletspeed()
	{
		return Bulletspeed;
	}

	int GetDirection()
	{
		return Direction;
	}

	int GetpointX()//��ȡ����
	{
		return point[0];
	}

	int GetpointY()
	{
		return point[1];
	}

	void IncreaseBulletspeed()//��������
	{
		Bulletspeed++;
	}
	
	void ChangeDirection(int newD)//�ı䷽��
	{
		Direction = newD;
	}

	void ChangePos(int x, int y)//�ı�����
	{
		point[0] = x;
		point[1] = y;
	}
};


#endif