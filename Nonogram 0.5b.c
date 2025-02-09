#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define LimHeight 24
#define LimWidth 24
#define RefreshCycle 50

int isMine[LimHeight][LimWidth];
int isOpen[LimHeight][LimWidth];
int solution[LimHeight][LimWidth];
int rowNumber[LimHeight][(LimWidth+1)/2];
int columnNumber[(LimHeight+1)/2][LimWidth];
int rowNumberColor[LimHeight][(LimWidth+1)/2];
int columnNumberColor[(LimHeight+1)/2][LimWidth];

// 标准线
int* lineMine = 0;
int* lineOpen = 0;
int* lineSolution = 0;
int* lineNumber = 0;
int lengthOfLine;
int countOfLineNumber;

int heightOfBoard = 6;
int widthOfBoard = 6;
int numberOfMine = 27;
int summonCheckMode = 0;

int lengthOfRowNumber = 3;
int lengthOfColumnNumber = 3;

int debug = 0;

void PrintBoard(int mode);
void SummonBoard(int seed);
int CheckSign();
void RecoverLine(int r, int c, int mode);
struct LinesIterator//标准线组迭代器
{
	int r, c;
};
struct LinesIterator LinesIteratorBegin();
int IsLinesIteratorEnd(struct LinesIterator li);
void LinesIteratorNext(struct LinesIterator* li);
int PutNumber(int* mine, int pos, int number);
int* LineFirstSolutionMine();
int* LineLastSolutionMine();
int SolveLine(struct LinesIterator li);
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
			//seed = 7;
			remainder = numberOfMine;
			SummonBoard(seed);
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
						r = mousePos.Y - lengthOfColumnNumber;
						c = (mousePos.X-1)/2 - lengthOfRowNumber;
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
				if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard
					&& isMine[r][c] == 1 && isOpen[r][c] == 1)
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
				if(heightOfBoard > LimHeight) heightOfBoard = LimHeight;
				if(widthOfBoard < 1) widthOfBoard = 1;
				if(widthOfBoard > LimWidth) widthOfBoard = LimWidth;
				if(numberOfMine < 0) numberOfMine = 0;
				if(numberOfMine > heightOfBoard * widthOfBoard) numberOfMine = heightOfBoard * widthOfBoard;
				while(numberOfMine < (heightOfBoard+1)/2 || numberOfMine < (widthOfBoard+1)/2) numberOfMine++;
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

void RecoverLine(int r, int c, int mode)//0生成line，1写出
{
	int i, i0;
	if(r != -1 && c == -1)//行操作，r有效
	{
		if(mode == 0)
		{
			lengthOfLine = widthOfBoard;
			if(lineMine != 0) free(lineMine);
			if(lineOpen != 0) free(lineOpen);
			if(lineSolution != 0) free(lineSolution);
			lineMine =(int*) calloc(lengthOfLine, sizeof(int));
			lineOpen =(int*) calloc(lengthOfLine, sizeof(int));
			lineSolution =(int*) calloc(lengthOfLine, sizeof(int));
			for(i=0; i<lengthOfLine; i++)
			{
				lineMine[i] = isMine[r][i];
				lineOpen[i] = isOpen[r][i];
				//lineSolution[i] = solution[r][i];
			}
			countOfLineNumber = (lengthOfLine+1)/2;
			for(i=0; i<(lengthOfLine+1)/2; i++)
			{
				if(rowNumber[r][i] == 0) countOfLineNumber--;
				else break;
			}
			if(lineNumber != 0) free(lineNumber);
			lineNumber =(int*) calloc(countOfLineNumber, sizeof(int));
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
			if(lineMine != 0) free(lineMine);
			if(lineOpen != 0) free(lineOpen);
			if(lineSolution != 0) free(lineSolution);
			lineMine =(int*) calloc(lengthOfLine, sizeof(int));
			lineOpen =(int*) calloc(lengthOfLine, sizeof(int));
			lineSolution =(int*) calloc(lengthOfLine, sizeof(int));
			for(i=0; i<lengthOfLine; i++)
			{
				lineMine[i] = isMine[i][c];
				lineOpen[i] = isOpen[i][c];
				//lineSolution[i] = solution[i][c];
			}
			countOfLineNumber = (lengthOfLine+1)/2;
			for(i=0; i<(lengthOfLine+1)/2; i++)
			{
				if(columnNumber[i][c] == 0) countOfLineNumber--;
				else break;
			}
			if(lineNumber != 0) free(lineNumber);
			lineNumber =(int*) calloc(countOfLineNumber, sizeof(int));
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

int* LineFirstSolutionMine()//生成标准线首解雷场
{
	int* mine =(int*) calloc(lengthOfLine, sizeof(int));
	int i, pos = 0;
	for(i=0; i<countOfLineNumber; )
	{
		if(PutNumber(mine, pos, lineNumber[i]) == 1)
		{
			pos += lineNumber[i]+1;
			i++;
		}
		else
		{
			pos++;
			//if(pos + lineNumber[i] > lengthOfLine)//无解情况，暂不考虑
		}
	}
	return mine;
}

int* LineLastSolutionMine()//生成标准线末解雷场
{
	int* mine =(int*) calloc(lengthOfLine, sizeof(int));
	int i, pos = lengthOfLine-1;
	for(i=countOfLineNumber-1; i>=0; )
	{
		if(PutNumber(mine, pos, lineNumber[i]) == 1)
		{
			if(i > 0) pos -= lineNumber[i-1]+1;
			i--;
		}
		else
		{
			pos--;
			//if(pos < 0)//无解情况，暂不考虑
		}
	}
	return mine;
}

int SolveLine(struct LinesIterator li)
{
	int i, ni;
	//生成标准线
	RecoverLine(li.r, li.c, 0);
	if(debug)
	{
		printf("定位标准线(%d,%d)\n", li.r, li.c);
		/*printf("lengthOfLine=%d\n", lengthOfLine);
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
		printf("lineSolution：");
		for(i=0; i<lengthOfLine; i++)
		{
			printf("%d ", lineSolution[i]);
		}
		printf("\n");*/
		printf("lineNumber：");
		for(i=0; i<countOfLineNumber; i++)
		{
			printf("%d ", lineNumber[i]);
		}
		printf("\n");
	}
	//端向心分析
	int nipos;
	ni = 0;
	for(i=0; i<lengthOfLine; i++)//头向尾分析
	{
		if(lineOpen[i] == 0) break;
		else if(lineOpen[i] == 2)
		{
			for(nipos = i; i < nipos+lineNumber[ni]; i++)//顺延标记
			{
				if(lineOpen[i] != 2) lineSolution[i] = 2;
			}
			if(nipos+lineNumber[ni] < lengthOfLine && lineOpen[nipos+lineNumber[ni]] != 1)//尾部翻开
			{
				lineSolution[nipos+lineNumber[ni]] = 1;
			}
			ni++;
			//i定位到超尾之后
			if(ni == countOfLineNumber)
			{
				for(; i<lengthOfLine; i++)//持续翻开
				{
					if(lineOpen[i] != 1) lineSolution[i] = 1;
				}
			}
		}
	}
	ni = countOfLineNumber-1;
	for(i=lengthOfLine-1; i>=0; i--)//尾向头分析
	{
		if(lineOpen[i] == 0) break;
		else if(lineOpen[i] == 2)
		{
			for(nipos = i; i > nipos-lineNumber[ni]; i--)//顺延标记
			{
				if(lineOpen[i] != 2) lineSolution[i] = 2;
			}
			if(nipos-lineNumber[ni] >= 0 && lineOpen[nipos-lineNumber[ni]] != 1)//头部翻开
			{
				lineSolution[nipos-lineNumber[ni]] = 1;
			}
			ni--;
			if(ni < 0)
			{
				for(; i>=0; i--)//持续翻开
				{
					if(lineOpen[i] != 1) lineSolution[i] = 1;
				}
			}
		}
	}
	//首末解交汇分析
	int* firstMine = LineFirstSolutionMine();//生成首解雷场
	int* lastMine = LineLastSolutionMine();//生成末解雷场
	int matchFirstLast = 1;
	for(i=0; i<lengthOfLine; i++)
	{
		if(firstMine[i] != lastMine[i]) matchFirstLast = 0;
	}
	if(matchFirstLast == 1)//首解末解完全相同，可判断端收束顶满线
	{
		for(i=0; i<lengthOfLine; i++)
		{
			if(firstMine[i] == 1 && lineOpen[i] != 2) lineSolution[i] = 2;
			if(firstMine[i] == 0 && lineOpen[i] != 1) lineSolution[i] = 1;
		}
	}
	else
	{
		//简单数字分析，可判断大数半满偏移
		int n1pos = 0, n2pos = 0;
		for(ni=0; ni<countOfLineNumber; ni++)
		{
			while(n1pos+lineNumber[ni] < lengthOfLine && firstMine[n1pos] == 0) n1pos++;
			while(n2pos+lineNumber[ni] < lengthOfLine && lastMine[n2pos] == 0) n2pos++;
			if(n1pos == n2pos)//数字确定
			{
				if(n1pos > 0 && lineOpen[n1pos-1] != 1)//头部翻开
				{
					lineSolution[n1pos-1] = 1;
				}
				for(i=n1pos; i < n1pos+lineNumber[ni]; i++)//中部标记
				{
					if(lineOpen[i] != 2) lineSolution[i] = 2;
				}
				if(n1pos+lineNumber[ni] < lengthOfLine && lineOpen[n1pos+lineNumber[ni]] != 1)//尾部翻开
				{
					lineSolution[n1pos+lineNumber[ni]] = 1;
				}
			}
			else if(n2pos - n1pos < lineNumber[ni])//数字中部确定
			{
				for(i=n2pos; i < n1pos+lineNumber[ni]; i++)//中部标记
				{
					if(lineOpen[i] != 2) lineSolution[i] = 2;
				}
			}
			if(ni == 0 && n1pos > 0)//线首翻开
			{
				for(i=0; i<n1pos; i++)
				{
					if(lineOpen[i] != 1) lineSolution[i] = 1;
				}
			}
			if(ni == countOfLineNumber-1 && n2pos+lineNumber[ni] < lengthOfLine)//线尾翻开
			{
				for(i=n2pos+lineNumber[ni]; i<lengthOfLine; i++)
				{
					if(lineOpen[i] != 1) lineSolution[i] = 1;
				}
			}
			n1pos += lineNumber[ni];
			n2pos += lineNumber[ni];
		}
	}
	//完成分析//中级测试种子7
	int sum1 = 0, sum2 = 0;
	for(i=0; i<lengthOfLine; i++)
	{
		if(lineOpen[i] == 2) sum1++;
	}
	for(i=0; i<countOfLineNumber; i++)
	{
		sum2 += lineNumber[i];
	}
	if(sum1 == sum2)
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
		printf("首解：");
		for(i=0; i<lengthOfLine; i++)
		{
			printf("%d ", firstMine[i]);
		}
		printf("\n");
		printf("末解：");
		for(i=0; i<lengthOfLine; i++)
		{
			printf("%d ", lastMine[i]);
		}
		printf("\n");
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
	free(firstMine);
	free(lastMine);
	//返回是否存在解
	for(i=0; i<lengthOfLine; i++)
	{
		if(lineSolution[i] != 0) return 1;
	}
	return 0;
}

int SolveStep()
{
	int isSolving = 0;
	struct LinesIterator li;
	for(li = LinesIteratorBegin(); !IsLinesIteratorEnd(li); LinesIteratorNext(&li))//通过标准线组迭代器遍历
	{
		isSolving += SolveLine(li);
	}
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
//——新增 拖动标记根据起始操作统一标记/取消标记
//——修复 再次进入游戏时可能持续翻开方块
--------------------------------*/
