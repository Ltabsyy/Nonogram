#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define RefreshCycle 50

int isMine[20][20];
int isOpen[20][20];
int rowNumber[20][10];
int columnNumber[10][20];

int heightOfBoard = 6;
int widthOfBoard = 6;
int numberOfMine = 27;
int summonCheckMode = 0;

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
void clrscr()//清空屏幕
{
	HANDLE hdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hdout, &csbi);//获取标准输出设备的屏幕缓冲区属性
	DWORD size = csbi.dwSize.X * csbi.dwSize.Y, num = 0;//定义双字节变量
	COORD pos = {0, 0};
	//把窗口缓冲区全部填充为空格并填充为默认颜色
	FillConsoleOutputCharacter(hdout, ' ', size, pos, &num);
	FillConsoleOutputAttribute(hdout, csbi.wAttributes, size, pos, &num);
	SetConsoleCursorPosition(hdout, pos);//光标定位到窗口左上角
}
void showCursor(int visible)//显示或隐藏光标
{
	CONSOLE_CURSOR_INFO cursor_info = {20, visible};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}
void ColorNumber(int number, int color)//输出彩色数字
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	printf("%d", number);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
}
void SetConsoleMouseMode(int mode)//键鼠操作切换
{
	if(mode == 1)//切换到鼠标
	{
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
	}
	else if(mode == 0)//切换到键盘
	{
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT
			| ENABLE_MOUSE_INPUT | ENABLE_INSERT_MODE | ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS
			| ENABLE_AUTO_POSITION);
	}
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
			if(columnNumber[r][c] < 10) putchar(' ');
			if(columnNumber[r][c] == 0) putchar(' ');
			else if(columnNumber[r][c] == heightOfBoard)
			{
				ColorNumber(columnNumber[r][c], 0x01);
			}
			else if(columnNumber[r][c] > heightOfBoard/2)
			{
				ColorNumber(columnNumber[r][c], 0x02);
			}
			else if(columnNumber[r][c] > heightOfBoard/3)
			{
				ColorNumber(columnNumber[r][c], 0x04);
			}
			else
			{
				printf("%d", columnNumber[r][c]);
			}
		}
		printf("\n");
	}
	for(r=0; r<heightOfBoard; r++)
	{
		// 行数字
		for(c=0; c<(widthOfBoard+1)/2; c++)
		{
			if(rowNumber[r][c] < 10) putchar(' ');
			if(rowNumber[r][c] == 0) putchar(' ');
			else if(rowNumber[r][c] == widthOfBoard)
			{
				ColorNumber(rowNumber[r][c], 0x01);
			}
			else if(rowNumber[r][c] > widthOfBoard/2)
			{
				ColorNumber(rowNumber[r][c], 0x02);
			}
			else if(rowNumber[r][c] > widthOfBoard/3)
			{
				ColorNumber(rowNumber[r][c], 0x04);
			}
			else
			{
				printf("%d", rowNumber[r][c]);
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
						ColorStr(" #", 0x06);
					}
					else if(isOpen[r][c] == 1)
					{
						ColorStr(" @", 0x04);
					}
					else
					{
						ColorStr(" *", 0x0c);
					}
				}
				else
				{
					if(isOpen[r][c] == 2)
					{
						ColorStr(" _", 0x06);
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
					ColorStr(" #", 0x06);
				}
				else if(isOpen[r][c] == 1)
				{
					if(isMine[r][c] == 1) ColorStr(" @", 0x04);
					else printf("  ");
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
	while(1)
	{
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
		if(summonCheckMode == 0) break;
		else if(summonCheckMode == 1)
		{
			// 校验存在顶满边则退出，即存在一边的雷场不存在连续2空
			if(isMine[0][0] != 0 && isMine[heightOfBoard-1][0] != 0)
			{
				n = 0;
				for(r=0; r<heightOfBoard-1; r++)
				{
					if(isMine[r][0] == 0 && isMine[r+1][0] == 0)
					{
						n = 1;
						break;
					}
				}
				if(n == 0) break;
			}
			if(isMine[0][widthOfBoard-1] != 0 && isMine[heightOfBoard-1][widthOfBoard-1] != 0)
			{
				n = 0;
				for(r=0; r<heightOfBoard-1; r++)
				{
					if(isMine[r][widthOfBoard-1] == 0 && isMine[r+1][widthOfBoard-1] == 0)
					{
						n = 1;
						break;
					}
				}
				if(n == 0) break;
			}
			if(isMine[0][0] != 0 && isMine[0][widthOfBoard-1] != 0)
			{
				n = 0;
				for(c=0; c<widthOfBoard-1; c++)
				{
					if(isMine[0][c] == 0 && isMine[0][c+1] == 0)
					{
						n = 1;
						break;
					}
				}
				if(n == 0) break;
			}
			if(isMine[heightOfBoard-1][0] != 0 && isMine[heightOfBoard-1][widthOfBoard-1] != 0)
			{
				n = 0;
				for(c=0; c<widthOfBoard-1; c++)
				{
					if(isMine[heightOfBoard-1][c] == 0 && isMine[heightOfBoard-1][c+1] == 0)
					{
						n = 1;
						break;
					}
				}
				if(n == 0) break;
			}
		}
		else if(summonCheckMode == 2)
		{
			//校验存在空线则退出
			for(r=0; r<heightOfBoard; r++)
			{
				n = 0;
				for(c=0; c<widthOfBoard; c++)
				{
					if(isMine[r][c] == 1)
					{
						n = 1;
						break;
					}
				}
				if(n == 0) break;
			}
			if(n == 0) break;
			for(c=0; c<widthOfBoard; c++)
			{
				n = 0;
				for(r=0; r<heightOfBoard; r++)
				{
					if(isMine[r][c] == 1)
					{
						n = 1;
						break;
					}
				}
				if(n == 0) break;
			}
			if(n == 0) break;
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

int CheckSign()
{
	int r, c;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if((isMine[r][c] == 0 && isOpen[r][c] == 2)//错误标记
				|| (isMine[r][c] == 1 && isOpen[r][c] == 0))//未标记的雷
			{
				return 0;
			}
		}
	}
	return 1;//胜利
}

int main()
{
	int choiceMode;
	int seed, r, c, isSigning;
	int isEnd, temp, remainder;
	HANDLE hdin = GetStdHandle(STD_INPUT_HANDLE);
	COORD mousePos = {0, 0};
	COORD mouseOperatedPos = {0, 0};//鼠标已操作坐标，屏蔽双击
	INPUT_RECORD rcd;
	DWORD rcdnum;
	int isReadyRefreshMouseOperatedPos = 0;
	while(1)
	{
		//clrscr();
		printf("*******************************\n"//宽31
			   "(1)新游戏\n"
			   "(2)设置\n"
			   "(3)退出\n"
			   "*******************************\n");
		printf(">");
		choiceMode = 0;
		scanf("%d", &choiceMode);
		if(choiceMode == 1)
		{
			clrscr();
			//isSigning = 0;
			seed = time(0);
			remainder = numberOfMine;
			SummonBoard(seed);
			SetConsoleMouseMode(1);
			showCursor(0);
			while(1)
			{
				gotoxy(0, 0);
				PrintBoard(0);
				//getchar();
				isEnd = 0;
				while(1)
				{
					gotoxy(0, (heightOfBoard+1)/2+heightOfBoard);
					printf("剩余雷数：%d 用时：%d \n", remainder, time(0)-seed);
					GetNumberOfConsoleInputEvents(hdin, &rcdnum);
					if(rcdnum == 0)
					{
						showCursor(0);
						Sleep(RefreshCycle);
						continue;
					}
					ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
					isReadyRefreshMouseOperatedPos = 1;
					if(rcd.EventType == MOUSE_EVENT)
					{
						mousePos = rcd.Event.MouseEvent.dwMousePosition;
						r = mousePos.Y - (heightOfBoard+1)/2;
						c = (mousePos.X-1)/2 - (widthOfBoard+1)/2;
						if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)
						{
							if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
							{
								if(isOpen[r][c] == 0) isOpen[r][c] = 1;
								break;
							}
							else if(rcd.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
							{
								isReadyRefreshMouseOperatedPos = 0;
								if(mousePos.X >= mouseOperatedPos.X-1 && mousePos.X <= mouseOperatedPos.X+1 && mousePos.Y == mouseOperatedPos.Y);
								else
								{
									if(isOpen[r][c] == 0)
									{
										isOpen[r][c] = 2;
										remainder--;
									}
									else if(isOpen[r][c] == 2)
									{
										isOpen[r][c] = 0;
										remainder++;
									}
									mouseOperatedPos = mousePos;
									break;
								}
							}
						}
					}
					else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
					{
						if(rcd.Event.KeyEvent.wVirtualKeyCode == ' ')
						{
							if(CheckSign() == 1)
							{
								isEnd = 1;
								break;
							}
						}
					}
					if(isReadyRefreshMouseOperatedPos == 1)
					{
						mouseOperatedPos.X = 0;
						mouseOperatedPos.Y = 0;
					}
					showCursor(0);
					Sleep(RefreshCycle);
				}
				if(isMine[r][c] == 1 && isOpen[r][c] == 1)
				{
					break;//翻开雷失败
				}
				if(isEnd == 1) break;//标记校验成功
				isEnd = 1;//翻开所有非雷方块成功
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(isMine[r][c] == 0 && isOpen[r][c] != 1)
						{
							isEnd = 0;//存在未翻开的非雷方块
						}
					}
				}
				if(isEnd == 1) break;
			}
			clrscr();
			gotoxy(0, 0);
			PrintBoard(1);
			if(isEnd == 1) printf(":)\nYou Win!\n");
			else printf(":(\nGame Over!\n");
			printf("用时：%d\n", time(0)-seed);
			SetConsoleMouseMode(0);
			showCursor(1);
			fflush(stdin);
		}
		else if(choiceMode == 2)
		{
			printf("*******************************\n");//宽31
			printf("(1)初级： 6*6  - 27\n");
			printf("(2)中级：10*10 - 64\n");
			printf("(3)高级：12*15 - 90\n");
			printf("(4)专家：15*20 -148\n");
			printf("(5)自定义\n");
			printf("*******************************\n");
			printf("当前难度:%d*%d-%d|密度:%.2f\n", heightOfBoard, widthOfBoard, numberOfMine, (float)numberOfMine/heightOfBoard/widthOfBoard);
			printf(">");
			scanf("%d", &temp);
			if(temp == 1)
			{
				heightOfBoard = 6;
				widthOfBoard = 6;
				numberOfMine = 27;
			}
			else if(temp == 2)
			{
				heightOfBoard = 10;
				widthOfBoard = 10;
				numberOfMine = 64;
			}
			else if(temp == 3)
			{
				heightOfBoard = 12;
				widthOfBoard = 15;
				numberOfMine = 90;
			}
			else if(temp == 4)
			{
				heightOfBoard = 15;
				widthOfBoard = 20;
				numberOfMine = 148;
			}
			else if(temp == 5)
			{
				printf("[行数]>");
				scanf("%d", &heightOfBoard);
				printf("[列数]>");
				scanf("%d", &widthOfBoard);
				printf("[雷数]>");
				scanf("%d", &numberOfMine);
				if(heightOfBoard < 1) heightOfBoard = 1;
				if(heightOfBoard > 20) heightOfBoard = 20;
				if(widthOfBoard < 1) widthOfBoard = 1;
				if(widthOfBoard > 20) widthOfBoard = 20;
				if(numberOfMine < 0) numberOfMine = 0;
				if(numberOfMine > heightOfBoard * widthOfBoard) numberOfMine = heightOfBoard * widthOfBoard;
				while(numberOfMine < (heightOfBoard+1)/2 || numberOfMine < (widthOfBoard+1)/2) numberOfMine++;
			}
			clrscr();
		}
		else if(choiceMode == 4)
		{
			printf("[地图生成校验：0关闭/1必存在顶满边/2必存在空线]>");
			scanf("%d", &summonCheckMode);
		}
		else// if(choiceMode == 3)
		{
			break;
		}
	}
	return 0;
}

/*--------------------------------
更新日志：
Nonogram 0.2
——新增 大数字彩色显示
——优化 刷新周期从100ms提高到50ms
——优化 调整错误标记显示
——优化 地图列数上限提升到20
——修复 错误标记可能异常停止游戏
Nonogram 0.3
——新增 设置内显示当前难度
——新增 游戏时按空格执行标记校验
——新增 显示剩余雷数
——新增 可按4设置地图生成校验必存在顶满边或必存在空线
——优化 高级和专家难度改为横向
——优化 数字配色
——修复 游戏时调整窗口大小显示控制台光标
//——新增 拖动标记根据起始操作统一标记/取消标记
--------------------------------*/
