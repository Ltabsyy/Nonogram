#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define LimLength 24
#define LimHeight LimLength
#define LimWidth LimLength
#define RefreshCycle 50

int isMine[LimHeight][LimWidth];
int isOpen[LimHeight][LimWidth];
int solution[LimHeight][LimWidth];
int rowNumber[LimHeight][(LimWidth+1)/2];
int columnNumber[(LimHeight+1)/2][LimWidth];
int rowNumberColor[LimHeight][(LimWidth+1)/2];
int columnNumberColor[(LimHeight+1)/2][LimWidth];

// 标准线
int lineMine[LimLength];
int lineOpen[LimLength];
int lineSolution[LimLength];
int lineNumber[(LimLength+1)/2];
int lengthOfLine;
int countOfLineNumber;

int heightOfBoard = 6;
int widthOfBoard = 6;
int numberOfMine = 27;
int summonCheckMode = 3;

int lengthOfRowNumber = 3;
int lengthOfColumnNumber = 3;

int debug = 0;
int countStep = 0;

void PrintBoard(int mode);
void SummonBoard(int seed);
int CheckSign();
int LiS();
void RecoverLine(int r, int c, int mode);
struct LinesIterator//标准线组迭代器
{
	int r, c;
};
struct LinesIterator LinesIteratorBegin();
int IsLinesIteratorEnd(struct LinesIterator li);
void LinesIteratorNext(struct LinesIterator* li);
//int PutNumber(int* mine, int pos, int number);
int* LineFirstSolutionPosList();
int* LineLastSolutionPosList();
int SolveLine(struct LinesIterator li);
//int AirWeaveLine(struct LinesIterator li);
int SolveStep();
int Solve();

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

int main()
{
	int choiceMode;
	int seed, r, c/*, isSigning*/;
	int isEnd, temp, remainder, t0, t1, lis;
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
			t0 = time(0);
			remainder = numberOfMine;
			SummonBoard(t0);
			lis = LiS();
			temp = lis;
			SetConsoleMouseMode(1);
			//FlushConsoleInputBuffer(hdin);
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
					t1 = time(0);
					printf("剩余雷数：%d 用时：%d LiS：%d/%d \n", remainder, t1-t0, lis-temp, lis);
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
						r = mousePos.Y - lengthOfColumnNumber;
						c = (mousePos.X-1)/2 - lengthOfRowNumber;
						if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)
						{
							if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
							{
								if(isOpen[r][c] == 0) isOpen[r][c] = 1;
								temp = LiS();
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
						else if(r>=0 && r<heightOfBoard && c < 0)//行数字
						{
							if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED
								|| rcd.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
							{
								if(rowNumberColor[r][c+lengthOfRowNumber] != 0x08)
								{
									rowNumberColor[r][c+lengthOfRowNumber] = 0x08;
								}
								else if(rowNumber[r][c+lengthOfRowNumber] == widthOfBoard)
								{
									rowNumberColor[r][c+lengthOfRowNumber] = 0x01;
								}
								else if(rowNumber[r][c+lengthOfRowNumber] > widthOfBoard/2)
								{
									rowNumberColor[r][c+lengthOfRowNumber] = 0x02;
								}
								else if(rowNumber[r][c+lengthOfRowNumber] > widthOfBoard/3)
								{
									rowNumberColor[r][c+lengthOfRowNumber] = 0x04;
								}
								else
								{
									rowNumberColor[r][c+lengthOfRowNumber] = 0x07;
								}
								break;
							}
						}
						else if(r < 0 && c>=0 && c<widthOfBoard)//列数字
						{
							if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED
								|| rcd.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
							{
								if(columnNumberColor[r+lengthOfColumnNumber][c] != 0x08)
								{
									columnNumberColor[r+lengthOfColumnNumber][c] = 0x08;
								}
								else if(columnNumber[r+lengthOfColumnNumber][c] == heightOfBoard)
								{
									columnNumberColor[r+lengthOfColumnNumber][c] = 0x01;
								}
								else if(columnNumber[r+lengthOfColumnNumber][c] > heightOfBoard/2)
								{
									columnNumberColor[r+lengthOfColumnNumber][c] = 0x02;
								}
								else if(columnNumber[r+lengthOfColumnNumber][c] > heightOfBoard/3)
								{
									columnNumberColor[r+lengthOfColumnNumber][c] = 0x04;
								}
								else
								{
									columnNumberColor[r+lengthOfColumnNumber][c] = 0x07;
								}
								break;
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
						else if(rcd.Event.KeyEvent.wVirtualKeyCode == '\t')
						{
							SolveStep();
							temp = LiS();
							break;
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
				if(isEnd == 1) break;//标记校验成功
				isEnd = 1;
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(isMine[r][c] == 1 && isOpen[r][c] == 1)
						{
							isEnd = 0;
						}
					}
				}
				if(isEnd == 0) break;//翻开雷失败
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
				if(isEnd == 1) break;//翻开所有非雷方块成功
			}
			clrscr();
			gotoxy(0, 0);
			PrintBoard(1);
			t1 = time(0);
			if(isEnd == 1)
			{
				printf(":)\nYou Win!\n");
				printf("用时：%d LiS：%d 速度：%.2f\n", t1-t0, lis, 10.0*lis/(t1-t0));
			}
			else
			{
				printf(":(\nGame Over!\n");
				temp = LiS();
				printf("用时：%d LiS：%d/%d 速度：%.2f\n", t1-t0, lis-LiS(), lis, 10.0*(lis-LiS())/(t1-t0));
			}
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
				if(heightOfBoard > LimHeight) heightOfBoard = LimHeight;
				if(widthOfBoard < 1) widthOfBoard = 1;
				if(widthOfBoard > LimWidth) widthOfBoard = LimWidth;
				if(numberOfMine < 0) numberOfMine = 0;
				if(numberOfMine > heightOfBoard * widthOfBoard) numberOfMine = heightOfBoard * widthOfBoard;
				//while(numberOfMine < (heightOfBoard+1)/2 || numberOfMine < (widthOfBoard+1)/2) numberOfMine++;
				if(summonCheckMode == 3 && numberOfMine < heightOfBoard * widthOfBoard / 2)
				{
					printf("密度过低，难以生成可解地图，是否维持地图可解？\n[1:维持可解/0放弃可解]>");
					scanf("%d", &temp);
					if(temp != 1) summonCheckMode = 0;
				}
			}
			lengthOfRowNumber = (widthOfBoard+1)/2;
			lengthOfColumnNumber = (heightOfBoard+1)/2;
			clrscr();
		}
		else if(choiceMode == 4)
		{
			printf("[地图生成校验：0关闭/1必存在顶满边/2必存在空线/3地图可解]>");
			scanf("%d", &summonCheckMode);
		}
		else if(choiceMode == 5)//标准十万局测试
		{
			int countWin = 0;
			int count[2][3] = {0};
			countStep = 0;
			t0 = time(0);
			temp = summonCheckMode;
			summonCheckMode = 0;
			for(seed=0; seed<100000; seed++)
			{
				/*printf("\nseed=%d\n", seed);
				SummonBoard(seed);
				while(SolveStep())
				{
					//PrintBoard(0);
					//getchar();
				}
				PrintBoard(1);
				*/
				if(seed % 64 == 0) printf("\rseed=%d", seed);//进度显示总耗时0.4秒
				SummonBoard(seed);
				countWin += Solve();
				//PrintBoard(1);
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						count[isMine[r][c]][isOpen[r][c]]++;
					}
				}
			}
			printf("\rseed=%d", seed-1);
			printf("\n胜利数：%d 用时：%d 步数：%d\n", countWin, time(0)-t0, countStep);
			printf("正确翻开%d 正确标记%d 翻开雷%d 错误标记%d\n", count[0][1], count[1][2], count[1][1], count[0][2]);
			summonCheckMode = temp;
		}
		else if(choiceMode == 6)//测试LiS
		{
			temp = summonCheckMode;
			summonCheckMode = 0;
			debug = 1;
			for(seed=0; seed<10; seed++)
			{
				SummonBoard(seed);
				printf("seed=%d\n", seed);
				//PrintBoard(1);
				LiS();
			}
			summonCheckMode = temp;
			debug = 0;
		}
		else if(choiceMode == 7)//随机求解测试
		{
			int n, i;
			srand(0);
			t0 = time(0);
			for(seed=0; seed<10000000; seed++)
			{
				if(seed % 8192 == 0) printf("\r%d", seed);
				//n = seed;
				for(c=0; c<widthOfBoard; c++)
				{
					/*isMine[0][c] = n%6/2;
					isOpen[0][c] = n%6%3;
					n/=6;*/
					isMine[0][c] = rand()%2;
					isOpen[0][c] = rand()%3;
				}
				for(c=0; c<lengthOfRowNumber; c++)
				{
					rowNumber[0][c] = 0;
				}
				n = 0;
				i = lengthOfRowNumber-1;
				for(c=widthOfBoard-1; c>=0; c--)
				{
					if(isMine[0][c] == 0)
					{
						if(n != 0)
						{
							rowNumber[0][i] = n;
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
					rowNumber[0][i] = n;
					n = 0;
					i--;
				}
				SolveLine(LinesIteratorBegin());
			}
			printf("\r%d\n", seed-1);
			printf("稳定性通过！标准线长：%d 用时：%d\n", lengthOfLine, time(0)-t0);
		}
		else// if(choiceMode == 3)
		{
			break;
		}
	}
	return 0;
}

void PrintBoard(int mode)
{
	int r, c;
	// 列数字
	for(r=0; r<lengthOfColumnNumber; r++)
	{
		for(c=0; c<lengthOfRowNumber; c++)
		{
			printf("  ");
		}
		for(c=0; c<widthOfBoard; c++)
		{
			if(columnNumber[r][c] < 10) putchar(' ');
			if(columnNumber[r][c] == 0) putchar(' ');
			else ColorNumber(columnNumber[r][c], columnNumberColor[r][c]);
		}
		printf("\n");
	}
	for(r=0; r<heightOfBoard; r++)
	{
		// 行数字
		for(c=0; c<lengthOfRowNumber; c++)
		{
			if(rowNumber[r][c] < 10) putchar(' ');
			if(rowNumber[r][c] == 0) putchar(' ');
			else ColorNumber(rowNumber[r][c], rowNumberColor[r][c]);
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
						//printf(" %%");
						if(r%3 == 0 || c%3 == 0) ColorStr(" %", 0x08);
						else printf(" %%");
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
					//printf(" %%");
					if(r%3 == 0 || c%3 == 0) ColorStr(" %", 0x08);
					else printf(" %%");
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
		// 初始化数字
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<lengthOfRowNumber; c++)
			{
				rowNumber[r][c] = 0;
			}
		}
		for(r=0; r<lengthOfColumnNumber; r++)
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
			i = lengthOfRowNumber-1;
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
			i = lengthOfColumnNumber-1;
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
		// 初始化颜色
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<lengthOfRowNumber; c++)
			{
				rowNumberColor[r][c] = 0x07;
				if(rowNumber[r][c] != 0)
				{
					if(rowNumber[r][c] == widthOfBoard)
					{
						rowNumberColor[r][c] = 0x01;
					}
					else if(rowNumber[r][c] > widthOfBoard/2)
					{
						rowNumberColor[r][c] = 0x02;
					}
					else if(rowNumber[r][c] > widthOfBoard/3)
					{
						rowNumberColor[r][c] = 0x04;
					}
				}
			}
		}
		for(r=0; r<lengthOfColumnNumber; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				columnNumberColor[r][c] = 0x07;
				if(columnNumber[r][c] != 0)
				{
					if(columnNumber[r][c] == heightOfBoard)
					{
						columnNumberColor[r][c] = 0x01;
					}
					else if(columnNumber[r][c] > heightOfBoard/2)
					{
						columnNumberColor[r][c] = 0x02;
					}
					else if(columnNumber[r][c] > heightOfBoard/3)
					{
						columnNumberColor[r][c] = 0x04;
					}
				}
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
			n = 0;
			for(r=0; r<heightOfBoard; r++)
			{
				if(rowNumber[r][lengthOfRowNumber-1] == 0)
				{
					n = 1;
					break;
				}
			}
			for(c=0; c<widthOfBoard; c++)
			{
				if(columnNumber[lengthOfColumnNumber-1][c] == 0)
				{
					n = 1;
					break;
				}
			}
			if(n == 1) break;
		}
		else if(summonCheckMode == 3)
		{
			if(Solve() == 1)//校验地图可解
			{
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						isOpen[r][c] = 0;
					}
				}
				break;
			}
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

int LiS()//计算剩余空含线数，衡量地图难度和游戏进度
{
	int r, c, r1, c1, r2, c2, isRising;
	int lis = 0, hlis, vlis;
	int** space =(int**) calloc(heightOfBoard, sizeof(int*));
	int** link =(int**) calloc(heightOfBoard, sizeof(int*));
	int** chain = (int**) calloc(heightOfBoard, sizeof(int*));
	//申请内存
	for(r=0; r<heightOfBoard; r++)
	{
		space[r] =(int*) calloc(widthOfBoard, sizeof(int));
		link[r] =(int*) calloc(widthOfBoard, sizeof(int));
		chain[r] =(int*) calloc(widthOfBoard, sizeof(int));
	}
	//标记未翻开空格
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isMine[r][c] == 0 && isOpen[r][c] != 1)
			{
				space[r][c] = 1;
			}
		}
	}
	//计算空格连通性
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(space[r][c] == 1)//8421标记上右下左是否为空格
			{
				if(r > 0 && space[r-1][c] == 1) link[r][c] |= 8;
				if(c+1 < widthOfBoard && space[r][c+1] == 1) link[r][c] |= 4;
				if(r+1 < heightOfBoard && space[r+1][c] == 1) link[r][c] |= 2;
				if(c > 0 && space[r][c-1] == 1) link[r][c] |= 1;
			}
		}
	}
	//必然计算
	for(r1=0; r1<heightOfBoard; r1++)
	{
		for(c1=0; c1<widthOfBoard; c1++)
		{
			if(space[r1][c1] == 1)
			{
				r = r1;
				c = c1;
				//处理无连空格
				if(link[r1][c1] == 0)
				{
					lis++;
					space[r][c] = 2;
				}
				//处理单连空格
				else if(link[r1][c1] == 8)
				{
					lis++;
					for(; r>=0 && space[r][c]; r--)
					{
						space[r][c] = 2;
					}
				}
				else if(link[r1][c1] == 4)
				{
					lis++;
					for(; c<widthOfBoard && space[r][c]; c++)
					{
						space[r][c] = 2;
					}
				}
				else if(link[r1][c1] == 2)
				{
					lis++;
					for(; r<heightOfBoard && space[r][c]; r++)
					{
						space[r][c] = 2;
					}
				}
				else if(link[r1][c1] == 1)
				{
					lis++;
					for(; c>=0 && space[r][c]; c--)
					{
						space[r][c] = 2;
					}
				}
				//处理双连空格
				if(link[r1][c1] == 10)
				{
					lis++;
					for(; r>=0 && space[r][c]; r--)
					{
						space[r][c] = 2;
					}
					for(r=r1+1; r<heightOfBoard && space[r][c]; r++)
					{
						space[r][c] = 2;
					}
				}
				else if(link[r1][c1] == 5)
				{
					lis++;
					for(; c<widthOfBoard && space[r][c]; c++)
					{
						space[r][c] = 2;
					}
					for(c=c1-1; c>=0 && space[r][c]; c--)
					{
						space[r][c] = 2;
					}
				}
			}
		}
	}
	//策略计算
	for(r1=0; r1<heightOfBoard; r1++)
	{
		for(c1=0; c1<widthOfBoard; c1++)
		{
			if(space[r1][c1] == 1)
			{
				//生成空链
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						chain[r][c] = 0;
					}
				}
				chain[r1][c1] = 1;
				isRising = 1;
				while(isRising == 1)
				{
					isRising = 0;
					for(r2=0; r2<heightOfBoard; r2++)
					{
						for(c2=0; c2<widthOfBoard; c2++)
						{
							if(space[r2][c2] == 1 && chain[r2][c2] == 1)
							{
								r = r2;
								c = c2;
								for(; r>=0 && space[r][c]; r--)//向上
								{
									if(chain[r][c] == 0)
									{
										chain[r][c] = 1;
										isRising = 1;
									}
								}
								for(r=r2+1; r<heightOfBoard && space[r][c]; r++)//向下
								{
									if(chain[r][c] == 0)
									{
										chain[r][c] = 1;
										isRising = 1;
									}
								}
								for(r=r2; c<widthOfBoard && space[r][c]; c++)//向右
								{
									if(chain[r][c] == 0)
									{
										chain[r][c] = 1;
										isRising = 1;
									}
								}
								for(c=c1-1; c>=0 && space[r][c]; c--)//向左
								{
									if(chain[r][c] == 0)
									{
										chain[r][c] = 1;
										isRising = 1;
									}
								}
							}
						}
					}
				}
				//计算横向线数
				hlis = 0;
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(chain[r][c] == 1 && space[r][c] == 1)
						{
							hlis++;
							for(; c<widthOfBoard && chain[r][c]; c++);
						}
					}
				}
				//计算纵向线数
				vlis = 0;
				for(c=0; c<widthOfBoard; c++)
				{
					for(r=0; r<heightOfBoard; r++)
					{
						if(chain[r][c] == 1 && space[r][c] == 1)
						{
							vlis++;
							for(; r<heightOfBoard && chain[r][c]; r++);
						}
					}
				}
				//增加较小值
				if(hlis <= vlis) lis += hlis;
				else lis += vlis;
				//处理空格
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(chain[r][c] == 1 && space[r][c] == 1)
						{
							space[r][c] = 2;
						}
					}
				}
			}
		}
	}
	//调试
	if(debug)
	{
		printf("LiS = %d\n", lis);
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(space[r][c] == 0) printf("  ");
				else
				{
					putchar(' ');
					if(space[r][c] == 1) ColorNumber(1, 0x04);
					else if(space[r][c] == 2) ColorNumber(2, 0x02);
				}
			}
			printf("\n");
		}
	}
	for(r=0; r<heightOfBoard; r++)
	{
		free(space[r]);
		free(link[r]);
		free(chain[r]);
	}
	free(space);
	free(link);
	free(chain);
	return lis;
}

void RecoverLine(int r, int c, int mode)//0生成line，1写出
{
	int i, i0;
	if(r != -1 && c == -1)//行操作，r有效
	{
		if(mode == 0)
		{
			lengthOfLine = widthOfBoard;
			for(i=0; i<lengthOfLine; i++)
			{
				lineMine[i] = isMine[r][i];
				lineOpen[i] = isOpen[r][i];
				lineSolution[i] = 0;
			}
			countOfLineNumber = (lengthOfLine+1)/2;
			for(i=0; i<(lengthOfLine+1)/2; i++)
			{
				if(rowNumber[r][i] == 0) countOfLineNumber--;
				else break;
			}
			for(i0=i; i<(lengthOfLine+1)/2; i++)
			{
				lineNumber[i-i0] = rowNumber[r][i];
			}
		}
		else if(mode == 1)
		{
			for(i=0; i<lengthOfLine; i++)
			{
				if(lineSolution[i] != 0)
				{
					solution[r][i] = lineSolution[i];
					isOpen[r][i] = lineSolution[i];
				}
			}
		}
	}
	else if(r == -1 && c != -1)//列操作，c有效
	{
		if(mode == 0)
		{
			lengthOfLine = heightOfBoard;
			for(i=0; i<lengthOfLine; i++)
			{
				lineMine[i] = isMine[i][c];
				lineOpen[i] = isOpen[i][c];
				lineSolution[i] = 0;
			}
			countOfLineNumber = (lengthOfLine+1)/2;
			for(i=0; i<(lengthOfLine+1)/2; i++)
			{
				if(columnNumber[i][c] == 0) countOfLineNumber--;
				else break;
			}
			for(i0=i; i<(lengthOfLine+1)/2; i++)
			{
				lineNumber[i-i0] = columnNumber[i][c];
			}
		}
		else if(mode == 1)
		{
			for(i=0; i<lengthOfLine; i++)
			{
				if(lineSolution[i] != 0)
				{
					solution[i][c] = lineSolution[i];
					isOpen[i][c] = lineSolution[i];
				}
			}
		}
	}
}

struct LinesIterator LinesIteratorBegin()
{
	struct LinesIterator li;
	li.r = 0;
	li.c = -1;
	return li;
}

int IsLinesIteratorEnd(struct LinesIterator li)//判断超尾
{
	return li.r == -1 && li.c == -1;
}

void LinesIteratorNext(struct LinesIterator* li)
{
	if(li->r != -1 && li->c == -1)
	{
		li->r++;
		if(li->r == heightOfBoard)
		{
			li->r = -1;
			li->c = 0;
		}
	}
	else if(li->r == -1 && li->c != -1)
	{
		li->c++;
		if(li->c == widthOfBoard)
		{
			li->c = -1;
		}
	}
}
/*
int PutNumber(int* mine, int pos, int number)//尝试放置数字，基于标准线信息
{
	int i;
	//判断无法放置情况
	if(pos + number > lengthOfLine) return 0;//超出边界
	if(pos + number < lengthOfLine && lineOpen[pos+number] == 2) return 0;//尾部为标记
	if(pos > 0 && lineOpen[pos-1] == 2) return 0;//头部为标记
	for(i=pos; i<pos+number; i++)
	{
		if(lineOpen[i] == 1) return 0;//中部被翻开
	}
	//放置
	for(i=pos; i<pos+number; i++)
	{
		mine[i] = 1;
	}
	//if(pos + number < lengthOfLine) mine[i] = 0;
	return 1;
}
*/
int* LineFirstSolutionPosList()//生成标准线首解位置表
{
	int* posList =(int*) calloc(countOfLineNumber, sizeof(int));
	int i, pos = 0, put, number, j;
	for(i=0; i<countOfLineNumber; )
	{
		number = lineNumber[i];
		//判断无法放置情况
		if(pos + number > lengthOfLine//超出边界，pos + number-1 >= lengthOfLine
		   || (pos + number < lengthOfLine && lineOpen[pos+number] == 2)//尾部为标记
		   || (pos > 0 && lineOpen[pos-1] == 2))//头部为标记
		{
			pos++;//单步跳过该位置
			if(pos + number > lengthOfLine)//无解情况
			{
				free(posList);
				return NULL;
			}
		}
		else
		{
			put = 1;
			for(j=pos+number-1; j>=pos; j--)//逆序检查
			{
				if(lineOpen[j] == 1)//中部被翻开
				{
					put = 0;
					pos = j+1;//多步连跳到翻开位置
					if(pos + number > lengthOfLine)//无解情况
					{
						free(posList);
						return NULL;
					}
					break;
				}
			}
			if(put == 1)
			{
				//放置
				posList[i] = pos;
				pos += number+1;
				i++;
			}
		}
	}
	return posList;
}

int* LineLastSolutionPosList()//生成标准线末解位置表
{
	int* posList =(int*) calloc(countOfLineNumber, sizeof(int));
	int i, pos = lengthOfLine-1, put, number, j;
	for(i=countOfLineNumber-1; i>=0; )
	{
		number = lineNumber[i];
		//判断无法放置情况
		if(pos - number + 1 < 0//超出边界
		   || (pos - number >= 0 && lineOpen[pos-number] == 2)//尾部为标记
		   || (pos+1 < lengthOfLine && lineOpen[pos+1] == 2))//头部为标记
		{
			pos--;//单步跳过该位置
			if(pos - number + 1 < 0)//无解情况
			{
				free(posList);
				return NULL;
			}
		}
		else
		{
			put = 1;
			for(j=pos-number+1; j<=pos; j++)//逆序检查
			{
				if(lineOpen[j] == 1)//中部被翻开
				{
					put = 0;
					pos = j-1;//多步连跳到翻开位置
					if(pos - number + 1 < 0)//无解情况
					{
						free(posList);
						return NULL;
					}
				}
			}
			if(put == 1)
			{
				//放置
				posList[i] = pos-number+1;
				pos -= number+1;
				i--;
			}
		}
	}
	return posList;
}

int SolveLine(struct LinesIterator li)
{
	int i, ni, nipos, check;
	//生成标准线
	RecoverLine(li.r, li.c, 0);
	//端向心分析
	int nstart, nend, nlimit;
	//头向尾分析
	ni = 0;
	for(i=0; i<lengthOfLine; i++)
	{
		if(ni == countOfLineNumber)
		{
			for(; i<lengthOfLine; i++)//持续翻开
			{
				if(lineOpen[i] == 0) lineSolution[i] = 1;
			}
			break;
		}
		if(lineOpen[i] == 0)
		{
			if(lineNumber[ni] == 1)//次1判断
			{
				if(i+1 < lengthOfLine && lineOpen[i+1] == 2)
				{
					lineSolution[i] = 1;
					i += 2;
					if(i < lengthOfLine && lineOpen[i] == 0) lineSolution[i] = 1;
					ni++;
				}
				else// break;//该位置可为1，不确定，退出
				{
					int nj;
					check = 1;//记录连续1数
					for(nj=ni+1; nj<countOfLineNumber; nj++)
					{
						if(lineNumber[nj] == 1) check++;
						else break;
					}
					if(nj == countOfLineNumber)//全1判断
					{
						for(i+=2; i<lengthOfLine; i++)
						{
							if(lineOpen[i] == 2)//前后翻开
							{
								if(lineOpen[i-1] == 0) lineSolution[i-1] = 1;
								if(i+1 < lengthOfLine && lineOpen[i+1] == 0) lineSolution[i+1] = 1;
							}
						}
					}
					else if(check > 1)//非虚悬连1判断
					{
						for(; check>0; check--)
						{
							while(i<lengthOfLine && lineOpen[i] == 1) i++;//滑过翻开方块
							if(lineOpen[i] == 2)//前后翻开
							{
								if(lineOpen[i-1] == 0) lineSolution[i-1] = 1;
								if(i+1 < lengthOfLine && lineOpen[i+1] == 0) lineSolution[i+1] = 1;
								i += 2;
							}
							else if(lineOpen[i] == 0)
							{
								if(lineOpen[i+1] != 2)//连续两个未知方块或单个未知方块和翻开
								{
									i += 2;
								}
								else//未知方块尾随标记
								{
									i++;//定位到标记
									if(lineOpen[i-1] == 0) lineSolution[i-1] = 1;
									if(i+1 < lengthOfLine && lineOpen[i+1] == 0) lineSolution[i+1] = 1;
									i += 2;
								}
							}
						}
					}
					break;
				}
			}
			else//大数预置分析
			{
				if(i+lineNumber[ni]-1 >= lengthOfLine) break;//无解情况
				nipos = i;
				check = 0;
				for(i = nipos+lineNumber[ni]-1; i > nipos; i--)
				{
					if(lineOpen[i] == 1)//未知方块连块不能容纳该数
					{
						check = 1;
						for(; i >= nipos; i--)
						{
							if(lineOpen[i] == 0) lineSolution[i] = 1;
						}
					}
				}
				if(check == 1)
				{
					for(i = nipos+lineNumber[ni]-1; lineOpen[i] != 1; i--);
					continue;//定位到最后一个翻开并继续
				}
				for(i = nipos+1; i < nipos+lineNumber[ni]; i++)
				{
					if(lineOpen[i] == 2)
					{
						check = 2;
						nstart = i;
						for(; i < nipos+lineNumber[ni]; i++)//顺延标记
						{
							if(lineOpen[i] == 0) lineSolution[i] = 2;
						}
					}
				}
				if(check == 0 && nipos+lineNumber[ni] < lengthOfLine && lineOpen[nipos+lineNumber[ni]] == 2)
				{
					check = 3;//存在超尾标记
					nstart = nipos+lineNumber[ni];
					lineSolution[nipos] = 1;
					nipos++;
				}
				if(check == 0) break;
				/*标记连块数a = nend-nstart+1，边缘区长n-a
				后部为nend+1(头)到(尾)nend+n-a = nstart+n-1
				前部为nstart-1(尾)到(头)nstart-n+a = nend-n+1
				限制偏移量b = nstart+n-1-nlimit+1 = nstart+n-nlimit
				回顶标记区域nstart-1(尾)到(头)nstart-b = nlimit-n
				前向翻开区域nipos(头)到(尾)nend-n*/
				//计算标记连块尾位置和限制位置
				nend = nipos+lineNumber[ni]-1;
				for(i = nipos+lineNumber[ni]; i<lengthOfLine; i++)
				{
					if(lineOpen[i] == 2) nend = i;
					else break;
				}
				if(nend-nstart+1 == lineNumber[ni]) nlimit = nend+1;//数字确定
				else
				{
					nlimit = nstart+lineNumber[ni];
					for(i = nend+1; i < nstart+lineNumber[ni]; i++)
					{
						if(i == lengthOfLine)
						{
							nlimit = lengthOfLine;
							break;
						}
						else if(lineOpen[i] == 1)
						{
							nlimit = i;
							break;
						}
					}
				}
				//执行操作
				for(i = nipos; i <= nend-lineNumber[ni]; i++)//前向翻开
				{
					if(lineOpen[i] == 0) lineSolution[i] = 1;
				}
				for(i = nlimit-lineNumber[ni]; i < nstart; i++)//回顶标记
				{
					if(lineOpen[i] == 0) lineSolution[i] = 2;
				}
				if(nend-nstart+1 == lineNumber[ni] && nend+1 < lengthOfLine)//数字确定尾部翻开
				{
					if(lineOpen[nend+1] == 0) lineSolution[nend+1] = 1;
				}
				//判断是否继续
				ni++;
				i = nlimit;
				//break;//直接退出
				if(nend-nstart+1 == lineNumber[ni-1]) continue;
				else if(nlimit < lengthOfLine && lineOpen[nlimit] == 1)//确保限制为翻开
				{
					if(nlimit <= nend+2) continue;//数字确定或空悬一格
					else if(ni < countOfLineNumber && lineNumber[ni] > nlimit-nend-2)
					{
						//剩余空间无法容纳新数字，可放置位置从nend+2到nlimit-1，长度nlimit-1-nend-2+1
						continue;
					}
					else break;
				}
				else break;
			}
		}
		else if(lineOpen[i] == 2)
		{
			if(i+lineNumber[ni]-1 >= lengthOfLine) break;//无解情况
			for(nipos = i; i < nipos+lineNumber[ni]; i++)//顺延标记
			{
				if(lineOpen[i] == 0) lineSolution[i] = 2;
			}
			if(nipos+lineNumber[ni] < lengthOfLine && lineOpen[nipos+lineNumber[ni]] == 0)//尾部翻开
			{
				lineSolution[nipos+lineNumber[ni]] = 1;
			}
			ni++;
			//i定位到超尾之后
		}
	}
	if(ni < countOfLineNumber)
	{
		if(lineNumber[ni] == 1)
		{
			int nj;
			check = 1;//记录连续1数
			for(nj=ni+1; nj<countOfLineNumber; nj++)
			{
				if(lineNumber[nj] == 1) check++;
				else break;
			}
			if(nj == countOfLineNumber)//全1判断
			{
				for(; i<lengthOfLine; i++)
				{
					if(lineOpen[i] == 2)//前后翻开
					{
						if(lineOpen[i-1] == 0) lineSolution[i-1] = 1;
						if(i+1 < lengthOfLine && lineOpen[i+1] == 0) lineSolution[i+1] = 1;
					}
				}
			}
			/*else if(check > 1)//连1判断
			{
				for(; check>0; check--)
				{
					while(i<lengthOfLine && lineOpen[i] == 1) i++;//滑过翻开方块
					if(lineOpen[i] == 2)//前后翻开
					{
						if(lineOpen[i-1] == 0) lineSolution[i-1] = 1;
						if(i+1 < lengthOfLine && lineOpen[i+1] == 0) lineSolution[i+1] = 1;
						i += 2;
					}
					else if(lineOpen[i] == 0)
					{
						if(lineOpen[i+1] != 2)//连续两个未知方块或单个未知方块和翻开
						{
							i += 2;
						}
						else//未知方块尾随标记
						{
							i++;//定位到标记
							if(lineOpen[i-1] == 0) lineSolution[i-1] = 1;
							if(i+1 < lengthOfLine && lineOpen[i+1] == 0) lineSolution[i+1] = 1;
							i += 2;
						}
					}
				}
			}*/
			/*大数预置分析退出后连1判断需要i回溯或处理衔接，还有虚悬1，复杂且效益不高，故仅在次1判断退出后进行
			虚悬1：数字1 1 4标记1 1 2，中1虚悬，左1可判*/
		}
	}
	//尾向头分析
	ni = countOfLineNumber-1;
	for(i=lengthOfLine-1; i>=0; i--)
	{
		if(ni < 0)
		{
			for(; i>=0; i--)//持续翻开
			{
				if(lineOpen[i] == 0) lineSolution[i] = 1;
			}
			break;
		}
		if(lineOpen[i] == 0)
		{
			if(lineNumber[ni] == 1)//次1判断
			{
				if(i > 0 && lineOpen[i-1] == 2)
				{
					lineSolution[i] = 1;
					i -= 2;
					if(i >= 0 && lineOpen[i] == 0) lineSolution[i] = 1;
					ni--;
				}
				else// break;//该位置可为1，不确定，退出
				{
					int nj;
					check = 1;//记录连续1数
					for(nj=ni-1; nj>=0; nj--)
					{
						if(lineNumber[nj] == 1) check++;
						else break;
					}
					if(nj == -1)//全1判断
					{
						for(i-=2; i>=0; i--)
						{
							if(lineOpen[i] == 2)//前后翻开
							{
								if(i > 0 && lineOpen[i-1] == 0) lineSolution[i-1] = 1;
								if(lineOpen[i+1] == 0) lineSolution[i+1] = 1;
							}
						}
					}
					else if(check > 1)//非虚悬连1判断
					{
						for(; check>0; check--)
						{
							while(i>=0 && lineOpen[i] == 1) i--;//滑过翻开方块
							if(lineOpen[i] == 2)//前后翻开
							{
								if(i > 0 && lineOpen[i-1] == 0) lineSolution[i-1] = 1;
								if(lineOpen[i+1] == 0) lineSolution[i+1] = 1;
								i -= 2;
							}
							else if(lineOpen[i] == 0)
							{
								if(lineOpen[i-1] != 2)//连续两个未知方块或单个未知方块和翻开
								{
									i -= 2;
								}
								else//未知方块尾随标记
								{
									i--;//定位到标记
									if(i > 0 && lineOpen[i-1] == 0) lineSolution[i-1] = 1;
									if(lineOpen[i+1] == 0) lineSolution[i+1] = 1;
									i -= 2;
								}
							}
						}
					}
					break;
				}
			}
			else//大数预置分析
			{
				if(i-lineNumber[ni]+1 < 0) break;//无解情况
				nipos = i;
				check = 0;
				for(i = nipos-lineNumber[ni]+1; i < nipos; i++)
				{
					if(lineOpen[i] == 1)//未知方块连块不能容纳该数
					{
						check = 1;
						for(; i <= nipos; i++)
						{
							if(lineOpen[i] == 0) lineSolution[i] = 1;
						}
					}
				}
				if(check == 1)
				{
					for(i = nipos-lineNumber[ni]+1; lineOpen[i] != 1; i++);
					continue;//定位到最后一个翻开并继续
				}
				for(i = nipos-1; i > nipos-lineNumber[ni]; i--)
				{
					if(lineOpen[i] == 2)
					{
						check = 2;
						nstart = i;
						for(; i > nipos-lineNumber[ni]; i--)//顺延标记
						{
							if(lineOpen[i] == 0) lineSolution[i] = 2;
						}
					}
				}
				if(check == 0 && nipos-lineNumber[ni] >= 0 && lineOpen[nipos-lineNumber[ni]] == 2)
				{
					check = 3;//存在超尾标记
					nstart = nipos-lineNumber[ni];
					lineSolution[nipos] = 1;
					nipos--;
				}
				if(check == 0) break;
				//计算标记连块尾位置和限制位置
				nend = nipos-lineNumber[ni]+1;
				for(i = nipos-lineNumber[ni]; i>=0; i--)
				{
					if(lineOpen[i] == 2) nend = i;
					else break;
				}
				if(nend-nstart+1 == lineNumber[ni]) nlimit = nend-1;//数字确定
				else
				{
					nlimit = nstart-lineNumber[ni];
					for(i = nend-1; i > nstart-lineNumber[ni]; i--)
					{
						if(i == -1)
						{
							nlimit = -1;
							break;
						}
						else if(lineOpen[i] == 1)
						{
							nlimit = i;
							break;
						}
					}
				}
				//执行操作
				for(i = nipos; i >= nend+lineNumber[ni]; i--)//前向翻开
				{
					if(lineOpen[i] == 0) lineSolution[i] = 1;
				}
				for(i = nlimit+lineNumber[ni]; i > nstart; i--)//回顶标记
				{
					if(lineOpen[i] == 0) lineSolution[i] = 2;
				}
				if(nend-nstart+1 == lineNumber[ni] && nend > 0)//数字确定尾部翻开
				{
					if(lineOpen[nend-1] == 0) lineSolution[nend-1] = 1;
				}
				//判断是否继续
				ni--;
				i = nlimit;
				//break;//直接退出
				if(nend-nstart+1 == lineNumber[ni+1]) continue;
				else if(nlimit >= 0 && lineOpen[nlimit] == 1)//确保限制为翻开
				{
					if(nlimit >= nend-2) continue;//数字确定或空悬一格
					else if(ni >= 0 && lineNumber[ni] > nlimit+nend+2)
					{
						//剩余空间无法容纳新数字，可放置位置从nend+2到nlimit-1，长度nlimit-1-nend-2+1
						continue;
					}
					else break;
				}
				else break;
			}
		}
		else if(lineOpen[i] == 2)
		{
			if(i-lineNumber[ni]+1 < 0) break;//无解情况
			for(nipos = i; i > nipos-lineNumber[ni]; i--)//顺延标记
			{
				if(lineOpen[i] == 0) lineSolution[i] = 2;
			}
			if(nipos-lineNumber[ni] >= 0 && lineOpen[nipos-lineNumber[ni]] == 0)//头部翻开
			{
				lineSolution[nipos-lineNumber[ni]] = 1;
			}
			ni--;
		}
	}
	if(ni >= 0)
	{
		if(lineNumber[ni] == 1)
		{
			int nj;
			check = 1;//记录连续1数
			for(nj=ni-1; nj>=0; nj--)
			{
				if(lineNumber[nj] == 1) check++;
				else break;
			}
			if(nj == -1)//全1判断
			{
				for(; i>=0; i--)
				{
					if(lineOpen[i] == 2)//前后翻开
					{
						if(i > 0 && lineOpen[i-1] == 0) lineSolution[i-1] = 1;
						if(lineOpen[i+1] == 0) lineSolution[i+1] = 1;
					}
				}
			}
			/*else if(check > 1)//连1判断
			{
				for(; check>0; check--)
				{
					while(i>=0 && lineOpen[i] == 1) i--;//滑过翻开方块
					if(lineOpen[i] == 2)//前后翻开
					{
						if(i > 0 && lineOpen[i-1] == 0) lineSolution[i-1] = 1;
						if(lineOpen[i+1] == 0) lineSolution[i+1] = 1;
						i -= 2;
					}
					else if(lineOpen[i] == 0)
					{
						if(lineOpen[i-1] != 2)//连续两个未知方块或单个未知方块和翻开
						{
							i -= 2;
						}
						else//未知方块尾随标记
						{
							i--;//定位到标记
							if(i > 0 && lineOpen[i-1] == 0) lineSolution[i-1] = 1;
							if(lineOpen[i+1] == 0) lineSolution[i+1] = 1;
							i -= 2;
						}
					}
				}
			}*/
		}
	}
	//首末解交汇分析
	int* firstPosList = LineFirstSolutionPosList();//生成首解
	int* lastPosList = LineLastSolutionPosList();//生成末解
	if(firstPosList != NULL && lastPosList != NULL)
	{
		//简单数字分析，可判断端收束顶满线、大数半满偏移
		int n1pos = 0, n2pos = 0;
		/*if(countOfLineNumber > 0)//线首翻开
		{
			for(i=0; i<firstPosList[0]; i++)
			{
				if(lineOpen[i] == 0) lineSolution[i] = 1;
			}
		}*/
		for(ni=0; ni<countOfLineNumber; ni++)
		{
			n1pos = firstPosList[ni];
			/*if(ni > 0 && n2pos+lineNumber[ni-1] < n1pos)//间分判断，末解前数尾和首解后数头之间翻开
			{
				for(i = n2pos+lineNumber[ni-1]; i < n1pos; i++)
				{
					if(lineOpen[i] == 0) lineSolution[i] = 1;
				}
			}*/
			n2pos = lastPosList[ni];
			if(n1pos == n2pos)//数字确定
			{
				if(n1pos > 0 && lineOpen[n1pos-1] == 0)//头部翻开
				{
					lineSolution[n1pos-1] = 1;
				}
				for(i=n1pos; i < n1pos+lineNumber[ni]; i++)//中部标记
				{
					if(lineOpen[i] == 0) lineSolution[i] = 2;
				}
				if(n1pos+lineNumber[ni] < lengthOfLine && lineOpen[n1pos+lineNumber[ni]] == 0)//尾部翻开
				{
					lineSolution[n1pos+lineNumber[ni]] = 1;
				}
			}
			else if(n2pos - n1pos < lineNumber[ni])//数字中部确定
			{
				for(i=n2pos; i < n1pos+lineNumber[ni]; i++)//中部标记
				{
					if(lineOpen[i] == 0) lineSolution[i] = 2;
				}
			}
		}
		/*if(lastPosList[countOfLineNumber-1]+lineNumber[countOfLineNumber-1] < lengthOfLine)//线尾翻开
		{
			for(i=lastPosList[countOfLineNumber-1]+lineNumber[countOfLineNumber-1]; i<lengthOfLine; i++)
			{
				if(lineOpen[i] == 0) lineSolution[i] = 1;
			}
		}*/
	}
	else//存在错误标记
	{
		if(firstPosList != NULL) free(firstPosList);
		if(lastPosList != NULL) free(lastPosList);
		return 0;
	}
	//完成分析，判断首末解不完全相同的完成情况
	check = 1;
	nipos = 0;
	for(ni=0; ni<countOfLineNumber; ni++)
	{
		while(nipos < lengthOfLine && (lineOpen[nipos] != 2 && lineSolution[nipos] != 2)) nipos++;
		if(nipos+lineNumber[ni]-1 >= lengthOfLine)
		{
			check = 0;
			break;
		}
		for(i = nipos; i < nipos+lineNumber[ni]; i++)
		{
			if(lineOpen[i] != 2 && lineSolution[i] != 2)
			{
				check = 0;
				break;
			}
		}
		if(check == 0) break;
		nipos += lineNumber[ni];
	}
	if(check == 1)
	{
		for(i=0; i<lengthOfLine; i++)
		{
			if(lineOpen[i] == 0 && lineSolution[i] == 0)
			{
				lineSolution[i] = 1;
			}
		}
	}
	if(debug)
	{
		printf("定位标准线(%d,%d)\n", li.r, li.c);
		printf("lengthOfLine=%d\n", lengthOfLine);
		printf("countOfLineNumber=%d\n", countOfLineNumber);
		printf("lineMine：");
		for(i=0; i<lengthOfLine; i++)
		{
			printf("%d ", lineMine[i]);
		}
		printf("\n");
		printf("lineOpen：");
		for(i=0; i<lengthOfLine; i++)
		{
			printf("%d ", lineOpen[i]);
		}
		printf("\n");
		printf("lineNumber：");
		for(i=0; i<countOfLineNumber; i++)
		{
			printf("%d ", lineNumber[i]);
		}
		printf("\n");
		if(firstPosList != NULL)
		{
			printf("首解：");
			for(i=0; i<countOfLineNumber; i++)
			{
				printf("%d ", firstPosList[i]);
			}
			printf("\n");
		}
		if(lastPosList != NULL)
		{
			printf("末解：");
			for(i=0; i<countOfLineNumber; i++)
			{
				printf("%d ", lastPosList[i]);
			}
			printf("\n");
		}
		printf("  解：");
		for(i=0; i<lengthOfLine; i++)
		{
			if(lineOpen[i] == 2 || lineSolution[i] == 2) 
			{
				ColorNumber(lineSolution[i], 0x06);
				putchar(' ');
			}
			else printf("%d ", lineSolution[i]);
		}
		printf("\n");
	}
	//标准线解写出
	RecoverLine(li.r, li.c, 1);
	if(firstPosList != NULL) free(firstPosList);
	if(lastPosList != NULL) free(lastPosList);
	//返回是否存在解
	for(i=0; i<lengthOfLine; i++)
	{
		if(lineSolution[i] != 0) return 1;
	}
	return 0;
}
/*
int AirWeaveLine(struct LinesIterator li)
{
	int i, ni;
	int* hasFirstSolution =(int*) calloc(countOfLineNumber, sizeof(int));//数字首解存在栈，用于回溯定位
	int* npos =(int*) calloc(countOfLineNumber, sizeof(int));
	int* sumMine =(int*) calloc(lengthOfLine, sizeof(int));
	//生成标准线
	RecoverLine(li.r, li.c, 0);
	//遍历所有解
	int* mine =(int*) calloc(lengthOfLine, sizeof(int));
	int numberOfPossibility = 0;
	int t;
	while(1)
	{
		//生成下一个解
		t = 0;
		while(1)
		{
			if(PutNumber(mine, npos[t], lineNumber[t]) == 1)
			{
				if(t == countOfLineNumber-1)
				{
					//检查剩余方块无标记
					
					//写入和字典
					numberOfPossibility++;
					for(i=0; i<lengthOfLine; i++)
					{
						sumMine[i] += mine[i];
					}
					for(i=0; i<countOfLineNumber; i++)
					{
						hasFirstSolution[i] = 1;
					}
				}
				else
				{
					npos[t+1] = npos[t] + lineNumber[t]+1;
					t++;
				}
			}
			else
			{
				npos[t]++;
				if(npos[t] + lineNumber[t] > lengthOfLine)//无解
				{
					
				}
			}
		}
		
		for(i=0; i<lengthOfLine; i++)
		{
			sumMine[i] += mine[i];
			mine[i] = 0;
		}
	}
	
	//标准线解写出
	RecoverLine(li.r, li.c, 1);
	free(hasFirstSolution);
	//返回是否存在解
	for(i=0; i<lengthOfLine; i++)
	{
		if(lineSolution[i] != 0) return 1;
	}
	return 0;
}
*/
int SolveStep()
{
	int isSolving = 0;
	struct LinesIterator li;
	for(li = LinesIteratorBegin(); !IsLinesIteratorEnd(li); LinesIteratorNext(&li))//通过标准线组迭代器遍历
	{
		isSolving += SolveLine(li);
	}
	/*if(isSolving == 0)
	{
		for(li = LinesIteratorBegin(); !IsLinesIteratorEnd(li); LinesIteratorNext(&li))
		{
			isSolving += AirWeaveLine(li);
		}
	}*/
	countStep++;
	return isSolving != 0;
}

int Solve()
{
	int r, c;
	int isSolving = 1;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			solution[r][c] = 0;
		}
	}
	while(isSolving)
	{
		isSolving = SolveStep();
	}
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isOpen[r][c] == 0 && solution[r][c] == 0) return 0;
		}
	}
	return 1;
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
Nonogram 0.4
——新增 鼠标点击数字时切换数字暗淡/明亮
——新增 地图辅助线
——新增 可按Tab自动推进一定数织进度
——新增 地图生成可校验地图可解
——优化 统一计算数字长度
——优化 颜色仅在地图生成时计算
——优化 地图大小上限从20*20提升到24*24
Nonogram 0.5
——新增 端向心分析的次1判断、大数预置分析、全1判断
——新增 可按5执行标准万局测试
——修复 地图求解可能标记已翻开方块
——修复 错误标记时首末解交汇分析引起的闪退
Nonogram 0.6
——新增 端向心分析的非虚悬连1判断
——新增 大数预置分析后继续分析
——新增 大数预置分析对超尾标记的处理
——优化 现在默认校验地图可解
——优化 不再自动提升过低雷数
——优化 不再单独判断首末解完全相同的情况
Nonogram 0.7
——新增 标准测试统计步数
——新增 显示LiS和速度
——优化 标准万局测试改为标准十万局测试
——优化 重构首末解生成
——优化 首末解交汇分析不再翻开线首和线尾
——优化 重构完成分析
——修复 错误标记Tab后不失败
Nonogram 0.8
——优化 首末解以位置表代替雷场提升性能
——优化 标准线使用固定内存
——修复 非法情况端向心分析可能闪退
//——新增 拖动标记根据起始操作统一标记/取消标记
//——新增 按空格执行标记校验改为翻开全部未标记方块
//——新增 首末解交汇分析的间分判断
//——优化 动态内存分配
//——修复 再次进入游戏时可能持续翻开方块
--------------------------------*/
