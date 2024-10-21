#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

int isMine[20][15];
int isOpen[20][15];
int rowNumber[20][8];
int columnNumber[10][15];

// 初级6*6-27 中级10*10-64 高级15*12-90 专家20*15-148
int heightOfBoard = 6;
int widthOfBoard = 6;
int numberOfMine = 27;

void ColorStr(const char* content, int color)//输出彩色字符
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	printf("%s", content);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
}
void gotoxy(short int x, short int y)
{
	COORD coord = {x, y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void PrintBoard(int mode)
{
	int r, c;
	// 列数字
	for(r=0; r<(heightOfBoard+1)/2; r++)
	{
		for(c=0; c<(widthOfBoard+1)/2; c++)
		{
			printf("  ");
		}
		for(c=0; c<widthOfBoard; c++)
		{
			if(columnNumber[r][c] == 0)
			{
				printf("  ");
			}
			else
			{
				printf("%2d", columnNumber[r][c]);
			}
		}
		printf("\n");
	}
	for(r=0; r<heightOfBoard; r++)
	{
		// 行数字
		for(c=0; c<(widthOfBoard+1)/2; c++)
		{
			if(rowNumber[r][c] == 0)
			{
				printf("  ");
			}
			else
			{
				printf("%2d", rowNumber[r][c]);
			}
		}
		for(c=0; c<widthOfBoard; c++)
		{
			if(mode == 1)
			{
				if(isMine[r][c] == 1)
				{
					if(isOpen[r][c] == 2)
					{
						//printf(" #");
						ColorStr(" #", 0x06);
					}
					else if(isOpen[r][c] == 1)
					{
						//printf(" @");
						ColorStr(" @", 0x04);
					}
					else
					{
						//printf(" *");
						ColorStr(" *", 0x0c);
					}
				}
				else
				{
					if(isOpen[r][c] == 2)
					{
						//printf("_#");
						ColorStr("_#", 0x04);
					}
					else if(isOpen[r][c] == 1)
					{
						printf("  ");
					}
					else
					{
						printf(" %%");
					}
				}
			}
			else
			{
				if(isOpen[r][c] == 2)
				{
					//printf(" #");
					ColorStr(" #", 0x06);
				}
				else if(isOpen[r][c] == 1)
				{
					/*if(isMine[r][c] == 1) ColorStr(" @", 0x04);
					else */printf("  ");
				}
				else
				{
					printf(" %%");
				}
			}
		}
		printf("\n");
	}
}

void SummonBoard(int seed)
{
	int r, c, i, n;
	srand(seed);
	// 初始化
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			isMine[r][c] = 0;
			isOpen[r][c] = 0;
		}
	}
	// 生成雷
	for(i=0; i<numberOfMine; )
	{
		r = rand() % heightOfBoard;
		c = rand() % widthOfBoard;
		if(isMine[r][c] == 0)
		{
			isMine[r][c] = 1;
			i++;
		}
	}
	// 初始化数字
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<(widthOfBoard+1)/2; c++)
		{
			rowNumber[r][c] = 0;
		}
	}
	for(r=0; r<(heightOfBoard+1)/2; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			columnNumber[r][c] = 0;
		}
	}
	// 计算行数字
	for(r=0; r<heightOfBoard; r++)
	{
		n = 0;
		i = (widthOfBoard+1)/2-1;
		for(c=widthOfBoard-1; c>=0; c--)
		{
			if(isMine[r][c] == 0)
			{
				if(n != 0)
				{
					rowNumber[r][i] = n;
					n = 0;
					i--;
				}
			}
			else
			{
				n++;
			}
		}
		if(n != 0)
		{
			rowNumber[r][i] = n;
			n = 0;
			i--;
		}
	}
	// 计算列数字
	for(c=0; c<widthOfBoard; c++)
	{
		n = 0;
		i = (heightOfBoard+1)/2-1;
		for(r=heightOfBoard-1; r>=0; r--)
		{
			if(isMine[r][c] == 0)
			{
				if(n != 0)
				{
					columnNumber[i][c] = n;
					n = 0;
					i--;
				}
			}
			else
			{
				n++;
			}
		}
		if(n != 0)
		{
			columnNumber[i][c] = n;
			n = 0;
			i--;
		}
	}
}

int main()
{
	int choiceMode = 1;
	int seed;
	int r, c;
	int isOpening, isSigning, rStart, cStart, rEnd, cEnd;
	int isEnd;
	HANDLE hdin = GetStdHandle(STD_INPUT_HANDLE);
	COORD mousePos = {0, 0};
	INPUT_RECORD rcd;
	DWORD rcdnum;
	while(1)
	{
		if(choiceMode == 1)
		{
			seed = time(0);
			SummonBoard(seed);
			SetConsoleMode(hdin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
			while(1)
			{
				gotoxy(0, 0);
				PrintBoard(0);
				//getchar();
				while(1)
				{
					ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
					if(rcd.EventType == MOUSE_EVENT)
					{
						mousePos = rcd.Event.MouseEvent.dwMousePosition;
						if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
						{
							r = mousePos.Y - (heightOfBoard+1)/2;
							c = mousePos.X/2 - (widthOfBoard+1)/2;
							if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)
							{
								if(isOpen[r][c] == 0) isOpen[r][c] = 1;
								break;
							}
						}
						else if(rcd.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
						{
							r = mousePos.Y - (heightOfBoard+1)/2;
							c = mousePos.X/2 - (widthOfBoard+1)/2;
							if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)
							{
								if(isOpen[r][c] == 0) isOpen[r][c] = 2;
								else if(isOpen[r][c] == 2) isOpen[r][c] = 0;
								break;
							}
						}
					}
					Sleep(100);
				}
				isEnd = 0;
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(isMine[r][c] == 1 && isOpen[r][c] == 1)
						{
							isEnd = 1;
						}
						if(isMine[r][c] == 0 && isOpen[r][c] == 1)
						{
							
						}
					}
				}
				if(isEnd == 1) break;
			}
			gotoxy(0, 0);
			PrintBoard(1);
			getchar();
		}
	}
	return 0;
}
