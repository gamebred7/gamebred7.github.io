#include "stdafx.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <list>
#include <dos.h>
#include <sstream>
#include <windows.h>
using namespace std;
using namespace sf;
#define H 20
#define W 150
float offsetX,offsetY;
int tileMap[H][W];

class Map
{
public:
	FILE *tiles;

	Map()
	{
		offsetX=0;
		offsetY=0;
	}

	void CreateMap()
	{
		tiles = fopen("tiles.txt", "r");	
		while(!feof(tiles))
		{   
			for(int i=0; i<H; i++)
			{
				for(int j=0; j<W; j++)
				{
					fscanf(tiles, "%d", &tileMap[i][j]);
				}
			}
		}
		fclose(tiles);
	}
};

class Caveman 
{
	float currentFrame0,currentFrame1,currentFrame2, time;
	Texture cavemanTexture;
	int dir;   
public:
	bool life,isFallen,onGround;
	float dx,dy;
	Sprite cavemanSprite;
	FloatRect rect;

	Caveman()
	{
		isFallen=0;
		dir=1;
		life=true;
		cavemanTexture.loadFromFile("8.png");
		time=20;
		currentFrame0=0;
		currentFrame1=0;
		currentFrame2=0;
		dx=dy=0.1;
		cavemanSprite.setTexture(cavemanTexture);
		rect = FloatRect(2*32,15*32,39,45);
	}

	void Move()
	{	
		Control();
		rect.left += dx * time;	
		Collision(0);

		if (!onGround) dy=dy+0.002*time;
		rect.top += dy*time;
		onGround=false;
		Collision(1);

		currentFrame0 += 0.006*time;
		currentFrame1 += 0.02*time;

		if (currentFrame0 > 4) currentFrame0 -=4;
		if (currentFrame1 > 11) currentFrame1 -=11;

		if(dx>0 && dy==0)                                                   //run right
		{
			cavemanSprite.setTextureRect(IntRect(39*int(currentFrame1),45,39,45)); 
			dir=1;
		}

		if(dx>0 && (dy<0 || dy>0))                                          //jump right
		{
			cavemanSprite.setTextureRect(IntRect(39*int(currentFrame0),0,39,45));
			dir=1;
		}

		if (dx<0 && dy==0)                                                  //run left
		{
			cavemanSprite.setTextureRect(IntRect(39*int(currentFrame1)+39,45,-39,45));
			dir=0;
		}

		if(dx<0 && (dy<0 || dy>0))                                          //jump left
		{
			cavemanSprite.setTextureRect(IntRect(39*int(currentFrame0)+39,0,-39,45));
			dir=0;
		}

		if(dx==0 && (dy>0 || dy<0))                                         //simple jump
		{
			if(dir==1)
				cavemanSprite.setTextureRect(IntRect(39*int(currentFrame0),0,39,45));
			else
				cavemanSprite.setTextureRect(IntRect(39*int(currentFrame0)+39,0,-39,45));
		}

		if(dx==0 && dy==0)                                                   //stay
		{
			if(dir==1)
				cavemanSprite.setTextureRect(IntRect(39,45,39,45));
			else
				cavemanSprite.setTextureRect(IntRect(39+39,45,-39,45));
		}

		if(life==0 && dy==0)
			Death();

		cavemanSprite.setPosition(rect.left - offsetX, rect.top - offsetY);
		dx=0;
	}

	void Death()
	{
		currentFrame2 += 0.02*time;

		if(currentFrame2>4)
		{
			currentFrame2 -= 0.02*time;
			isFallen=1;
		}

		rect.left += dx * time;	
		Collision(0);

		if (!onGround) dy=dy+0.002*time;
		rect.top += dy*time;
		onGround=false;
		Collision(1);

		//death
		if(dir==1)
			cavemanSprite.setTextureRect(IntRect(53*int(currentFrame2),90,53,45));    
		else
			cavemanSprite.setTextureRect(IntRect(53*int(currentFrame2)+53,90,-53,45));  
	}

	void Collision(int dir)
	{
		for (int i = rect.top/32 ; i<(rect.top+rect.height)/32; i++)
			for (int j = rect.left/32; j<(rect.left+rect.width)/32; j++)
			{ 
				if (tileMap[i][j]>1 || tileMap[i][j]==-2) 
				{ 
					if ((dx>0) && (dir==0))
						rect.left =  j*32 - rect.width; 
					if ((dx<0) && (dir==0)) 
						rect.left =  j*32 + 32;
					if ((dy>0) && (dir==1)) 
					{
						rect.top =   i*32 -  rect.height;  
						dy=0;   
						onGround=true; 
					}
					if ((dy<0) && (dir==1)) 
					{ 
						rect.top = i*32 + 32; 
						dy=0;
					}
				}
			}
	}

	void Control()
	{
		if(life)
		{
			if (Keyboard::isKeyPressed(Keyboard::Left)) 
				dx = -0.4;
			if (Keyboard::isKeyPressed(Keyboard::Right)) 
			{
				/*SoundBuffer buffer;
				buffer.loadFromFile("Mario_Theme.ogg");
				Sound bufferShoot(buffer);
				bufferShoot.play();*/
				dx = 0.4;
			}
			if (Keyboard::isKeyPressed(Keyboard::Up)) 
			{
				if (onGround) 
				{ 
					dy=-0.7; 
					onGround=false;
				}
			}
			if (Keyboard::isKeyPressed(Keyboard::Down)) 
			{
				if (!onGround) 
				{ 
					dy=0.3; 
					onGround=true;
				}
			}
		}

		if (rect.left>150) 
			offsetX = rect.left - 150;
		offsetY = rect.top - 212;
	}

	void Recover()
	{
		rect = FloatRect(2*32,15*32,39,45);
		isFallen=0;
		life=true;
		dx=0.4;
		currentFrame2=0;
		offsetX=0;
		offsetY=0;
	}
};

class Entity
{
protected:
	float currentFrame,moveTimer;
	bool onGround;
	int frameNumber,time;
public:               
	float dx,dy;
	FloatRect rect;
	int width,heigth;
	Sprite sprite;

	void virtual Move() = 0;

	void Collision(int dir)
	{
		for (int i = rect.top/32 ; i<(rect.top+rect.height)/32; i++)
			for (int j = rect.left/32; j<(rect.left+rect.width)/32; j++)
			{ 
				if (tileMap[i][j]>1 || tileMap[i][j]==-1) 
				{ 
					if (dx<0)
					{
						rect.left =  j*32 + 32;
						dx = 0.2;
					}

					if ((dx>0) && (dir==0))
					{
						rect.left =  j*32 - rect.width;
						dx = -0.2;
					}

					if ((dy>0) && (dir==1)) 
					{
						rect.top =   i*32 -  rect.height;  
						dy=0;   
						onGround=true; 
					}
					if ((dy<0) && (dir==1)) 
					{ 
						rect.top = i*32 + 32; 
						dy=0;
					}
				}
			}
	}
};

class Drogon : public Entity
{
public:
	Drogon(int x, int y, int width, int heigth, float dx, int time, int frameNumber, Texture &image)
	{
		this->width=width;
		this->heigth=heigth;
		this->frameNumber=frameNumber;
		this->time=time;
		this->dx=dx;
		moveTimer=0;
		dy=0;
		rect = FloatRect(x*32,y*32,width,heigth);
		currentFrame = 0;
		sprite.setTexture(image);
	}

	Drogon()
	{
		memset(this, 0, sizeof(Drogon));
	}

	void Move() override
	{	
		Collision(0);
		rect.left += dx * time;	

		rect.top += dy*time;
		onGround=false;
		Collision(1);

		currentFrame += 0.02*time;
		if (currentFrame > frameNumber) currentFrame -=frameNumber;

		if (dx>0) sprite.setTextureRect(IntRect(width*int(currentFrame),0,width,heigth));
		if (dx<0) sprite.setTextureRect(IntRect(width*int(currentFrame)+width,0,-width,heigth));

		sprite.setPosition(rect.left - offsetX, rect.top - offsetY);
	}
};

class Dino : public Entity
{
public:
	Dino(int x, int y, float dx,  int frameNumber, int time, int width, int heigth, Texture &image)
	{
		this->width=width;
		this->heigth=heigth;
		this->frameNumber=frameNumber;
		this->moveTimer=time;
		this->time=time;
		this->dx=dx;
		dy=0.1;
		rect = FloatRect(x*32,y*32,width,heigth);
		currentFrame = 0;
		sprite.setTexture(image);
	}

	Dino()
	{
		memset(this, 0, sizeof(Dino));
	}

	void Move() override
	{	
		Collision(0);
		rect.left += dx * time;	

		rect.top += dy*time;
		onGround=false;
		Collision(1);

		currentFrame += 0.02*time;
		if (currentFrame > frameNumber) currentFrame -=frameNumber;

		if (dx>0) sprite.setTextureRect(IntRect(width*int(currentFrame),0,width,heigth));
		if (dx<0) sprite.setTextureRect(IntRect(width*int(currentFrame)+width,0,-width,heigth));

		sprite.setPosition(rect.left - offsetX, rect.top - offsetY);
	}
};

class Flower : public Entity
{
public:
	Flower(int x, int y,int width, int heigth,  int frameNumber, int time, Texture &image)
	{
		this->width=width;
		this->heigth=heigth;
		this->frameNumber=frameNumber;
		this->time=time;
		moveTimer=0;
		dx=dy=0.0000001;
		rect = FloatRect(x*32,y*32,width,heigth);
		currentFrame = 0;
		sprite.setTexture(image);
	}

	Flower()
	{
		memset(this, 0, sizeof(Flower));
	}

	void Move() override
	{	
		moveTimer += time;
		if (moveTimer>2000)
		{ 
			dx *= -1; 
			moveTimer = 0; 
		}

		Collision(0);
		rect.left += dx * time;	

		rect.top += dy*time;
		onGround=false;
		Collision(1);

		currentFrame += 0.02*time;
		if (currentFrame > frameNumber) currentFrame -=frameNumber;

		if (dx>0) sprite.setTextureRect(IntRect(width*int(currentFrame),0,width,heigth));
		if (dx<0) sprite.setTextureRect(IntRect(width*int(currentFrame)+width,0,-width,heigth));

		sprite.setPosition(rect.left - offsetX, rect.top - offsetY);
	}
};

class Food 
{
	Texture foodTexture;
public:               
	FloatRect rect;
	int width,heigth;
	Sprite sprite;
	bool available;

	Food(int x, int y)
	{
		available=true;
		heigth=29;
		width=32;
		foodTexture.loadFromFile("meat.png");
		rect = FloatRect(x*32,y*32,width,heigth);
		sprite.setTexture(foodTexture);
	}

	Food()
	{
		memset(this, 0, sizeof(Food));
	}

	void Display() 
	{		
		sprite.setPosition(rect.left - offsetX, rect.top - offsetY);
	}
};

class Game {
	Image icon;
	Texture menuBackground, menuPlaySm, menuPlayLg, menuControlSm, menuControlLg,  tileset, dinoTexture, drogonTexture, menuImage, rex, endrestart, endhome;
	Music gameMusic, menuMusic;
	bool isMenu,endGame,isHelp, isQuit, isRecord, isPlayer, isLand, isEnd, isRestart, isWinter, isSummer;
	int menuNum,score,quitYes, gamerecord;
	VideoMode videoMode;
	RenderWindow* window;
	Event sfmlEvent;
	Sprite Tileset;
	Caveman p;
	Map m;
	Font font;
	list<Entity*>  entities;
	list<Entity*>::iterator it, it2;
	list<Food*> food;
	list<Food*>::iterator it3; 
	String cavemanScore, gameRecord, newRecord;
	Sprite HelpStone,QuitStone, RecordStone, closeButton,yesButton, backButton, PlayerStone, axeButton, LandStone, EndStone, endRestart, endHome, ScoreStone, sosButton;
	Texture helpStone,quitStone, recordStone, closebutton,yesbutton, helpBG,  backbutton, playerStone, axebutton, landStone, endWin, endLose, scoreStone, sosbutton;
	FILE *record;
public:	
	Game()
	{
		this->initVariables();
		this->Window();
		icon.loadFromFile("icon.jpg");
		window->setIcon(32, 32, icon.getPixelsPtr());
	}

	~Game()
	{
		delete this->window;
	}

	void Window()
	{
		this->videoMode = VideoMode(480, 320);
		this->window = new RenderWindow(this->videoMode, "Caveman");
		this->window->setFramerateLimit(20);
	}

	const bool Running() const
	{
		return this->window->isOpen();
	}

	void Menu() 
	{
		Sprite Play, Record, Help, Exit, menuBg(menuImage);
		menuBg.setPosition(0, 0);

		while(isMenu)
		{
			menuNum = 0;
			window->clear();

			Play.setPosition(185, 238);
			Play.setTexture(menuPlaySm);
			Play.setTextureRect(IntRect(0,0,104,40));

			Record.setPosition(185, 281);
			Record.setTexture(menuControlSm);
			Record.setTextureRect(IntRect(116,0,31,35));

			Help.setPosition(221, 281);
			Help.setTexture(menuControlSm);
			Help.setTextureRect(IntRect(78,0,31,35));

			Exit.setPosition(258, 281);
			Exit.setTexture(menuControlSm);
			Exit.setTextureRect(IntRect(0,0,31,35));

			if (IntRect(210, 250, 60, 30).contains(Mouse::getPosition(*window)))
			{
				Play.setPosition(185-6, 238-2);
				Play.setTexture(menuPlayLg);
				Play.setTextureRect(IntRect(0,0,117,45));
				menuNum = 1; 
			}

			if (IntRect(185, 281, 31, 35).contains(Mouse::getPosition(*window)))
			{
				Record.setPosition(185-2,281-2);
				Record.setTexture(menuControlLg);
				Record.setTextureRect(IntRect(133,0,36,40));
				menuNum = 2; 
			}

			if (IntRect(221, 281, 31, 35).contains(Mouse::getPosition(*window))) 
			{
				Help.setPosition(221-2, 281-2);
				Help.setTexture(menuControlLg);
				Help.setTextureRect(IntRect(89,0,36,40));
				menuNum = 3; 
			}

			if (IntRect(258, 281, 31, 35).contains(Mouse::getPosition(*window))) 
			{
				Exit.setPosition(258-2, 281-2);
				Exit.setTexture(menuControlLg);
				Exit.setTextureRect(IntRect(0,0,36,40));
				menuNum = 4; 
			}

			if (Mouse::isButtonPressed(Mouse::Left))
			{
				if (menuNum == 1) 
				{
					isMenu=false;
					Restart();
					PlayerChoose(); 
					//isMenu=true;
				}

				if (menuNum == 2) 
				{
					isMenu=false;
					GameRecord(); 
					isMenu=true;
				}
				if (menuNum == 3)
				{
					isMenu=false;
					HelpGuide(); 
					isMenu=true;
				}
				if (menuNum == 4)
				{
					isMenu=false;
					Quit(); 
					isMenu=true;
				}
			}

			if(isMenu)
			{
				window->draw(menuBg);
				window->draw(Play);
				window->draw(Record);
				window->draw(Help);
				window->draw(Exit);
				window->display();
			}
		}
		//isMenu=1;
	}

	void HelpGuide() 
	{
		helpBG.loadFromFile("helpBg.png");
		Sprite helpBg(helpBG);
		helpBg.setPosition(0, 0);
		isHelp=true;
		while(isHelp)
		{
			int helpNum=0;

			HelpStone.setPosition(50, 26);
			HelpStone.setTexture(helpStone);

			if (IntRect(385, 75, 38, 38).contains(Mouse::getPosition(*window)))
				helpNum = 1; 

			if (Mouse::isButtonPressed(Mouse::Left))
			{
				if (helpNum) 
				{
					isHelp=false;

				}
			}

			window->draw(helpBg);
			window->draw(HelpStone);
			if(helpNum)
				window->draw(closeButton);
			window->display();
		}
	}

	void Quit() 
	{
		helpBG.loadFromFile("helpBg.png");
		Sprite quitBg(helpBG);
		quitBg.setPosition(0, 0);
		isQuit=true;

		while(isQuit)
		{
			quitYes=0;
			int quitNum=0;

			QuitStone.setPosition(50, 26);
			QuitStone.setTexture(quitStone);

			if (IntRect(385, 75, 38, 38).contains(Mouse::getPosition(*window)))
				quitNum = 1; 
			if (IntRect(196, 216, 80, 50).contains(Mouse::getPosition(*window)))
				quitYes= 1; 

			if (Mouse::isButtonPressed(Mouse::Left))
			{
				if (quitNum) 
					isQuit=false;
				if (quitYes) 
					window->close();
			}

			window->draw(quitBg);
			window->draw(QuitStone);
			if(quitNum)
				window->draw(closeButton);
			if(quitYes)
				window->draw(yesButton);
			window->display();
		}
	}

	void GameRecord() 
	{
		helpBG.loadFromFile("helpBg.png");
		gameRecord = to_string(gamerecord);
		Text record(" ",font,36);
		record.setColor(Color::White);
		record.setString(gameRecord);
		record.setStyle(Text::Bold);
		record.setPosition(252,189);


		Sprite helpBg(helpBG);
		helpBg.setPosition(0, 0);
		isRecord=true;
		while(isRecord)
		{
			int recordNum=0;

			RecordStone.setPosition(50, 26);
			RecordStone.setTexture(recordStone);

			if (IntRect(385, 75, 38, 38).contains(Mouse::getPosition(*window)))
				recordNum = 1; 

			if (Mouse::isButtonPressed(Mouse::Left))
			{
				if (recordNum) 
				{
					isRecord=false;

				}
			}

			window->draw(helpBg);
			window->draw(RecordStone);
			if(recordNum)
				window->draw(closeButton);
			window->draw(record);
			window->display();
		}
	}

	void PlayerChoose() 
	{
		helpBG.loadFromFile("helpBg.png");
		Sprite helpBg(helpBG);
		helpBg.setPosition(0, 0);
		isPlayer=true;
		while(isPlayer)
		{
			int playerNum=0;

			PlayerStone.setPosition(50, 0);
			PlayerStone.setTexture(playerStone);

			if (IntRect(46, 47, 64, 47).contains(Mouse::getPosition(*window)))
				playerNum = 1; 
			if (IntRect(197, 175, 51, 51).contains(Mouse::getPosition(*window)))
			{
				axeButton.setPosition(195,172);
				playerNum = 2; 
			}
			if (IntRect(368, 175, 51, 51).contains(Mouse::getPosition(*window)))
			{
				playerNum = 3; 
				axeButton.setPosition(371,172);
			}

			if (Mouse::isButtonPressed(Mouse::Left))
			{
				if (playerNum==1) 
				{
					isPlayer=false;
					isMenu=true;
				}
				if (playerNum==2) 
				{

					isPlayer=false;
					isLand=true;
					LandChoose();
					//isMenu=false;
				}
				if (playerNum==3) 
				{
					isPlayer=false;
					isLand=true;
					LandChoose();
					//isMenu=false;
				}
			}

			if(isPlayer)
			{

				window->draw(helpBg);
				window->draw(PlayerStone);
				if(playerNum==1)
				{
					backButton.setPosition(46, 47);
					window->draw(backButton);
				}
				if(playerNum==2 || playerNum==3)
					window->draw(axeButton);
				window->display();
			}
		}
	}

	void LandChoose() 
	{
		helpBG.loadFromFile("helpBg.png");
		Sprite helpBg(helpBG);
		helpBg.setPosition(0, 0);
		isPlayer=true;
		while(isLand)
		{
			int landNum=0;

			LandStone.setPosition(50, 26);
			LandStone.setTexture(landStone);

			if (IntRect(66, 93, 50, 30).contains(Mouse::getPosition(*window)))
				landNum = 1;  
			if (IntRect(122, 242, 51, 51).contains(Mouse::getPosition(*window)))
			{
				axeButton.setPosition(120,240);
				landNum = 2; 
			}
			if (IntRect(314, 242, 51, 51).contains(Mouse::getPosition(*window)))
			{
				landNum = 3; 
				axeButton.setPosition(312,240);
			}

			if (Mouse::isButtonPressed(Mouse::Left))
			{
				if (landNum==1) 
				{
					isLand=false;
					PlayerChoose();
				}
				if (landNum==2) 
				{
					isLand=false;
					isPlayer=false;
					isMenu=false;
					menuBackground.loadFromFile("Ice.jpg");
					tileset.loadFromFile("tilesetIce.png");
					ScoreStone.setTextureRect(IntRect(0,54,200,54));
					isWinter=1;
					isSummer=0;
					sosButton.setPosition(118,14);
				}
				if (landNum==3) 
				{
					isLand=false;
					isPlayer=false;
					isMenu=false;
					isWinter=0;
					isSummer=1;
					menuBackground.loadFromFile("helpBG.png");
					tileset.loadFromFile("tilesetJumanji.png");
					ScoreStone.setTextureRect(IntRect(0,0,200,54));
					sosButton.setPosition(146,12);
				}
			}

			if(isLand)
			{

				window->draw(helpBg);
				window->draw(LandStone);
				if(landNum==1)
				{
					backButton.setPosition(50, 76);
					window->draw(backButton);
				}
				if(landNum==2 || landNum==3)
					window->draw(axeButton);
				window->display();
			}
		}
	}

	void GameEnd()
	{
		if(isWinter)
			helpBG.loadFromFile("Ice.jpg");
		if(isSummer)
			helpBG.loadFromFile("helpBg.png");
		Sprite helpBg(helpBG);
		helpBg.setPosition(0, 0);

		isEnd=true;

		newRecord = to_string(score);
		gameRecord = to_string(gamerecord);

		Text NewRecord(" ",font,32);
		NewRecord.setColor(Color::White);
		NewRecord.setString(newRecord);
		NewRecord.setStyle(Text::Bold);

		Text OldRecord(" ",font,17);
		OldRecord.setColor(Color::White);
		OldRecord.setString(gameRecord);
		OldRecord.setStyle(Text::Bold);

		if(score>gamerecord)
		{
			record = fopen("record.txt", "w");			
			fprintf(record, "%d", score);
			fclose(record);

			EndStone.setPosition(100, 0);
			EndStone.setTexture(endWin);
			endHome.setPosition(161,235);
			endRestart.setPosition(257,235);
			NewRecord.setPosition(246,184);
			OldRecord.setPosition(286,149);
		}
		else
		{
			EndStone.setPosition(100, 0);
			EndStone.setTexture(endLose);
			endHome.setPosition(166,238);
			endRestart.setPosition(260,238);
			NewRecord.setPosition(249,183);
			OldRecord.setPosition(299,163);
		}


		while(isEnd)
		{
			int endNum=0;

			if (IntRect(167, 237, 40, 20).contains(Mouse::getPosition(*window)))
				endNum = 1;  
			if (IntRect(270, 237, 40, 20).contains(Mouse::getPosition(*window)))
				endNum = 2; 


			if (Mouse::isButtonPressed(Mouse::Left))
			{
				if (endNum==1) 
				{
					isEnd=0;
					isMenu=1;
					Menu();
				}
				if (endNum==2) 
				{
					isEnd=0;
					Restart();
				}
			}

			if(isEnd)
			{
				window->draw(helpBg);
				window->draw(EndStone);
				if(endNum==1)
				{
					window->draw(endHome);
				}
				if(endNum==2)
					window->draw(endRestart);
				window->draw(NewRecord);

				window->draw(OldRecord);
				window->display();
			}
		}
	}

	void DrawMap()
	{
		Sprite menuBg(menuBackground);
		menuBg.setPosition(0, 0);
		this->window->draw(menuBg);

		for (int i=0; i<H; i++)
		{
			for (int j=0; j<W ; j++)
			{ 
				if (tileMap[i][j]==2)
					Tileset.setTextureRect(IntRect(0,0,32,32));
				if (tileMap[i][j]==3)
					Tileset.setTextureRect(IntRect(32,0,32,32));
				if (tileMap[i][j]==4)
					Tileset.setTextureRect(IntRect(64,0,32,32));
				if (tileMap[i][j]==5)
					Tileset.setTextureRect(IntRect(96,0,32,32));
				if (tileMap[i][j]==6)
					Tileset.setTextureRect(IntRect(32*4,0,32,32));
				if (tileMap[i][j]==7)
					Tileset.setTextureRect(IntRect(32*5,0,32,32));
				if (tileMap[i][j]==8)
					Tileset.setTextureRect(IntRect(32*6,0,32,32));
				if (tileMap[i][j]==9)
					Tileset.setTextureRect(IntRect(32*7,0,32,32));
				if (tileMap[i][j]==22)
					Tileset.setTextureRect(IntRect(0,32,32,32));
				if (tileMap[i][j]==33)
					Tileset.setTextureRect(IntRect(32,32,32,32));
				if (tileMap[i][j]==44)
					Tileset.setTextureRect(IntRect(64,32,32,32));
				if (tileMap[i][j]==55)
					Tileset.setTextureRect(IntRect(96,32,32,32));
				if (tileMap[i][j]==66)
					Tileset.setTextureRect(IntRect(32*4,32,32,32));
				if (tileMap[i][j]==77)
					Tileset.setTextureRect(IntRect(32*5,32,32,32));
				if (tileMap[i][j]==88)
					Tileset.setTextureRect(IntRect(32*6,32,32,32));
				if (tileMap[i][j]==99)
					Tileset.setTextureRect(IntRect(32*7,32,32,32));
				if (tileMap[i][j]==10)
					Tileset.setTextureRect(IntRect(32,96,176-32,160));


				if (tileMap[i][j]==1 || tileMap[i][j]==-1 || tileMap[i][j]==-2)
					continue;
				Tileset.setPosition(j*32-offsetX, i*32-offsetY);
				window->draw(Tileset);
			}
		}
	}

	void Score()
	{
		newRecord = to_string(score);
		gameRecord = to_string(gamerecord);

		ScoreStone.setTexture(scoreStone);
		ScoreStone.setPosition(2,2);

		Text gamescore(" ",font,25);
		gamescore.setColor(Color::White);
		gamescore.setString(cavemanScore);
		gamescore.setStyle(Text::Bold);


		Text record(" ",font,25);
		record.setColor(Color::White);
		if(score<=gamerecord)
			record.setString("/"+gameRecord);
		else
			record.setString("/"+cavemanScore);
		record.setStyle(Text::Bold);

		int scoreNum=0;

		if(isWinter)
		{
			if (IntRect(118, 14, 27, 27).contains(Mouse::getPosition(*window)))
				scoreNum=1;  
			if(score>9)
			{
				gamescore.setPosition(59,11);
				record.setPosition(84,11);
			}
			else
			{
				gamescore.setPosition(59,11);
				record.setPosition(74,11);
			}
		}
		if(isSummer)
		{
			if (IntRect(146, 12, 27, 27).contains(Mouse::getPosition(*window)))
				scoreNum=1;  
			if(score>9)
			{
				gamescore.setPosition(61,12);
				record.setPosition(86,12);
			}
			else
			{
				gamescore.setPosition(61,12);
				record.setPosition(76,12);
			}
		}


		if (Mouse::isButtonPressed(Mouse::Left))
		{
			if (scoreNum==1) 
			{
				GameEnd();
				scoreNum=0;
			}
		}

		window->draw(ScoreStone);
		window->draw(gamescore);
		window->draw(record);
		if(scoreNum==1)
			window->draw(sosButton);

	}

	void Music()
	{
		gameMusic.openFromFile("menuMusic.ogg");
		gameMusic.play();
		gameMusic.setLoop(true);
	}

	void initVariables()
	{
		isWinter=0;
		m.CreateMap();
		isRestart=0;
		score=0;
		record = fopen("record.txt", "r");			
		fscanf(record, "%d", &gamerecord);
		fclose(record);

		font.loadFromFile("CyrilicOld.ttf");
		quitYes=0;

		closeButton.setPosition(387, 77);
		closeButton.setTexture(closebutton);
		closeButton.setTextureRect(IntRect(0,0,43,43));

		yesButton.setPosition(198, 218);
		yesButton.setTexture(yesbutton);
		yesButton.setTextureRect(IntRect(0,0,82,52));


		isHelp=1;
		isMenu = 1;
		menuNum = 0;
		endGame = false;

		axebutton.loadFromFile("Xe.png");
		axeButton.setTexture(axebutton);



		Tileset.setTexture(tileset);

		dinoTexture.loadFromFile("kroko.png");
		drogonTexture.loadFromFile("drogon.png");

		menuPlaySm.loadFromFile("playSm.png");
		menuPlayLg.loadFromFile("playLg.png");
		menuControlSm.loadFromFile("controlSm.png");
		menuControlLg.loadFromFile("controlLg.png");

		helpStone.loadFromFile("HelpStone.png");
		recordStone.loadFromFile("RecordStone.png");
		closebutton.loadFromFile("Close.png");
		yesbutton.loadFromFile("Yes.png");
		quitStone.loadFromFile("QuitStone.png");
		backbutton.loadFromFile("Back.png");
		playerStone.loadFromFile("CavemanStone.png");
		landStone.loadFromFile("LandStone.png");
		endhome.loadFromFile("endHome.png");
		endrestart.loadFromFile("endRestart.png");
		endWin.loadFromFile("EndWin.png");
		endLose.loadFromFile("EndLose.png");
		scoreStone.loadFromFile("scoreStone.png");

		endHome.setTexture(endhome);


		endRestart.setTexture(endrestart);

		sosbutton.loadFromFile("sos.png");
		sosButton.setTexture(sosbutton);





		backButton.setTexture(backbutton);


		helpBG.loadFromFile("helpBg.png");


		menuImage.loadFromFile("caveman.jpg");
		rex.loadFromFile("greyrex.png");

		for (int i = 0; i < 1; i++)
		{
			//entities.push_back(new Drogon(60,12,49,33,0.9,22,3,drogonTexture));
			entities.push_back(new Dino(20,16,0.8,2,25,84,58,dinoTexture));
			entities.push_back(new Dino(25,16,-0.8,3,25,49,31,rex));

			food.push_back(new Food(16,16));
			food.push_back(new Food(26,15));
			food.push_back(new Food(36,17));
			food.push_back(new Food(16,15));
			food.push_back(new Food(56,15));
			food.push_back(new Food(66,14));
			food.push_back(new Food(20,16));
			food.push_back(new Food(46,15));
			food.push_back(new Food(11,15));
			food.push_back(new Food(6,15));

		}
	}

	void Start()
	{
		p.Move();
		cavemanScore = to_string(score);

		for (it = entities.begin(); it != entities.end(); it++)
		{

			(*it)->Move();

			if ((*it)->rect.intersects(p.rect))
			{
				if ((*it)->dx>0)
				{
					(*it)->rect.left = p.rect.left - (*it)->rect.width; 

				}
				if ((*it)->dx < 0)
				{
					(*it)->rect.left = p.rect.left + p.rect.width;
				}
				End();
			}
			for (it2 = entities.begin(); it2 != entities.end(); it2++)
			{
				if((*it)->rect!=(*it2)->rect)
				{
					if ((*it)->rect.intersects((*it2)->rect))
						(*it)->dx*=-1;
				}
			}
		}
		FoodDisplay();

		if (p.dx < 0) { p.rect.left = (*it)->rect.left + p.rect.width; }
		if (p.dx > 0) { p.rect.left = (*it)->rect.left - p.rect.width; }


		if(p.isFallen)
		{
			Sleep(2000);
			GameEnd();
		}
	}

	void End()
	{
		(*it)->dx=0;
		p.life=0;
	}

	void Display()
	{
		this->window->clear();
		this->DrawMap();
		this->Score();
		this->window->draw(p.cavemanSprite);
		for (it = entities.begin(); it != entities.end(); it++)
		{
			window->draw((*it)->sprite); 
		}
		for (it3 = food.begin(); it3 != food.end(); it3++)
		{
			window->draw((*it3)->sprite); 
		}
		this->window->display();
	}

	void Restart()
	{
		p.Recover();
		score=0;
		isRestart=1;

		for (it = entities.begin(); it != entities.end(); it++)
		{
			if(isRestart && (*it)->dx==0)
			{
				(*it)->dx=0.1;
				isRestart=0;
			}
		}
		FoodDisplay();
	}

	void FoodDisplay()
	{

		for (it3 = food.begin(); it3 != food.end();)
		{
			(*it3)->Display();
			if ((*it3)->rect.intersects(p.rect))
			{
				score++;
				(*it3)->available=false;
				it3=food.erase(it3);
			}
			else it3++;
		}
	}
};

int main()
{
	Game game;
	//game.Music();
	game.Menu();


	while (game.Running())
	{
		game.Start();
		game.Display();
	}

	system("pause");
	return 0;
}
