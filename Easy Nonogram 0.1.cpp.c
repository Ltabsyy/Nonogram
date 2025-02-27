#include <stdlib.h>
#include <time.h>
#include <graphics.h>

#define LimHeight 24
#define LimWidth 24
#define RefreshCycle 50

int isMine[LimHeight][LimWidth];
int isOpen[LimHeight][LimWidth];
int solution[LimHeight][LimWidth];
int rowNumber[LimHeight][(LimWidth+1)/2];
int columnNumber[(LimHeight+1)/2][LimWidth];
color_t rowNumberColor[LimHeight][(LimWidth+1)/2];
color_t columnNumberColor[(LimHeight+1)/2][LimWidth];

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
//int summonCheckMode = 3;

int lengthOfRowNumber = 3;
int lengthOfColumnNumber = 3;

int sideLength = 32;

//int debug = 0;

void DrawBlock(int r, int c, int isMine, int isOpen);
void DrawWindow(int mode);
void Operate(char operation, int r, int c);

void SummonBoard(int seed);

int main()
{
	mouse_msg mouseMsg;
	char operation;
	int r, c, r1, c1, r2, c2;
	int isEnd, t0;
	// 设置难度
	setcaption("Easy Nonogram");
	SetProcessDPIAware();//避免Windows缩放造成模糊
	initgraph(10*sideLength, 5*sideLength, INIT_RENDERMANUAL);
	setbkcolor(EGERGB(18, 18, 18));
	setfont(sideLength, 0, "Consolas");
	setcolor(WHITE);
	xyprintf(0, 0*sideLength, "初级： 6*6  - 27");
	xyprintf(0, 1*sideLength, "中级：10*10 - 64");
	xyprintf(0, 2*sideLength, "高级：12*15 - 90");
	xyprintf(0, 3*sideLength, "专家：15*20 - 148");
	xyprintf(0, 4*sideLength, "最大：24*24 - 288");
	int difficulty = -1;
	while(difficulty == -1)
	{
		while(mousemsg())
		{
			mouseMsg = getmouse();
			if(mouseMsg.is_left() && mouseMsg.is_up())
			{
				difficulty = mouseMsg.y/sideLength;
				break;
			}
		}
		delay_ms(50);
	}
	if(difficulty == 0)
	{
		heightOfBoard = 6;
		widthOfBoard = 6;
		numberOfMine = 27;
	}
	else if(difficulty == 1)
	{
		heightOfBoard = 10;
		widthOfBoard = 10;
		numberOfMine = 64;
	}
	else if(difficulty == 2)
	{
		heightOfBoard = 12;
		widthOfBoard = 15;
		numberOfMine = 90;
	}
	else if(difficulty == 3)
	{
		heightOfBoard = 15;
		widthOfBoard = 20;
		numberOfMine = 148;
	}
	else if(difficulty == 4)
	{
		heightOfBoard = 24;
		widthOfBoard = 24;
		numberOfMine = 288;
		sideLength = 24;
		setfont(sideLength, 0, "Consolas");
	}
	lengthOfRowNumber = (widthOfBoard+1)/2;
	lengthOfColumnNumber = (heightOfBoard+1)/2;
	// 游戏
	initgraph((lengthOfRowNumber+widthOfBoard)*sideLength, (lengthOfColumnNumber+heightOfBoard)*sideLength);
	int newGame = 1;
	while(newGame == 1)
	{
		t0 = time(0);
		SummonBoard(t0);
		while(1)
		{
			DrawWindow(0);
			setcolor(WHITE);
			xyprintf(sideLength/4, 0, "%ds", time(0)-t0);
			if(r1 != -2147483648 && c1 != -2147483648 && (r1 == r2 || c1 == c2))
			{
				setcolor(BLUE);
				setlinewidth(sideLength/16);
				line((lengthOfRowNumber+c1)*sideLength+sideLength/2, (lengthOfColumnNumber+r1)*sideLength+sideLength/2,
					(lengthOfRowNumber+c2)*sideLength+sideLength/2, (lengthOfColumnNumber+r2)*sideLength+sideLength/2);
			}
			operation = 0;
			isEnd = 0;
			while(mousemsg())
			{
				mouseMsg = getmouse();
				r = mouseMsg.y / sideLength - lengthOfColumnNumber;
				c = mouseMsg.x / sideLength - lengthOfRowNumber;
				r2 = r;
				c2 = c;
				if(mouseMsg.is_down())
				{
					r1 = r;
					c1 = c;
				}
				if(mouseMsg.is_up())
				{
					if(mouseMsg.is_left()) operation = '@';
					if(mouseMsg.is_right()) operation = '#';
				}
			}
			if(operation != 0 && (r1 == r2 || c1 == c2))
			{
				if(operation == '#')//根据起点统一标记/取消标记
				{
					if(isOpen[r1][c1] == 2) operation = '%';
				}
				if(r1 == r2)
				{
					if(c1 > c2)
					{
						c = c1;
						c1 = c2;
						c2 = c;
					}
					for(c=c1; c<=c2; c++)
					{
						if(operation == '#' && isOpen[r1][c] == 2) continue;
						if(operation == '%' && isOpen[r1][c] == 0) continue;
						if(operation == '%') Operate('#', r1, c);
						else Operate(operation, r1, c);
					}
				}
				else if(c1 == c2)
				{
					if(r1 > r2)
					{
						r = r1;
						r1 = r2;
						r2 = r;
					}
					for(r=r1; r<=r2; r++)
					{
						if(operation == '#' && isOpen[r][c1] == 2) continue;
						if(operation == '%' && isOpen[r][c1] == 0) continue;
						if(operation == '%') Operate('#', r, c1);
						else Operate(operation, r, c1);
					}
				}
			}
			if(operation != 0)
			{
				r1 = -2147483648;
				c1 = -2147483648;
				r2 = -2147483648;
				c2 = -2147483648;
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
			delay_ms(50);
		}
		DrawWindow(1);
		if(isEnd == 1) setcolor(YELLOW);
		else setcolor(RED);
		xyprintf(sideLength/4, 0, "%ds", time(0)-t0);
		setfont(sideLength/2, 0, "黑体");
		setcolor(RED);
		xyprintf(sideLength/4, sideLength, "左键新游戏");
		xyprintf(sideLength/4, sideLength*3/2, "右键关闭窗口");
		setfont(sideLength, 0, "Consolas");
		newGame = -1;
		while(newGame == -1)
		{
			while(mousemsg())
			{
				mouseMsg = getmouse();
				if(mouseMsg.is_up())
				{
					if(mouseMsg.is_left()) newGame = 1;
					else newGame = 0;
					break;
				}
			}
			delay_ms(50);
		}
	}
	return 0;
}

void DrawBlock(int r, int c, int isMine, int isOpen)
{
	int x = (lengthOfRowNumber+c)*sideLength;
	int y = (lengthOfColumnNumber+r)*sideLength;
	if(isOpen == 1)//纯黑底纹
	{
		if(isMine == 1) setfillcolor(RED);
		else setfillcolor(BLACK);
		ege_fillrect(x, y, sideLength, sideLength);
	}
	else if(isOpen == 2)
	{
		setfillcolor(GOLD);
		ege_fillrect(x, y, sideLength, sideLength);
		setcolor(RED);
		setlinewidth(sideLength/16);
		if(isMine == 0)//错误标记
		{
			line(x+sideLength/4, y+sideLength/4, x+sideLength*3/4, y+sideLength*3/4);
			line(x+sideLength/4, y+sideLength*3/4, x+sideLength*3/4, y+sideLength/4);
		}
		else
		{
			ege_ellipse(x+sideLength/4, y+sideLength/4, sideLength/2, sideLength/2);
		}
	}
	else
	{
		setfillcolor(DIMGRAY);
		ege_fillrect(x, y, sideLength, sideLength);
		if(isMine == 1)
		{
			setcolor(RED);
			setlinewidth(sideLength/16);
			ege_ellipse(x+sideLength/4, y+sideLength/4, sideLength/2, sideLength/2);
		}
	}
}

void DrawWindow(int mode)
{
	int r, c;
	cleardevice();
	//列数字
	for(r=0; r<lengthOfColumnNumber; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(columnNumber[r][c] != 0)
			{
				setcolor(columnNumberColor[r][c]);
				if(columnNumber[r][c] < 10)
				{
					xyprintf((lengthOfRowNumber+c)*sideLength+sideLength/4, r*sideLength, "%d", columnNumber[r][c]);
				}
				else
				{
					xyprintf((lengthOfRowNumber+c)*sideLength, r*sideLength, "%d", columnNumber[r][c]);
				}
			}
		}
	}
	//行数字
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<lengthOfRowNumber; c++)
		{
			if(rowNumber[r][c] != 0)
			{
				setcolor(rowNumberColor[r][c]);
				if(rowNumber[r][c] < 10)
				{
					xyprintf(c*sideLength+sideLength/4, (lengthOfColumnNumber+r)*sideLength, "%d", rowNumber[r][c]);
				}
				else
				{
					xyprintf(c*sideLength, (lengthOfColumnNumber+r)*sideLength, "%d", rowNumber[r][c]);
				}
			}
		}
	}
	//地图
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(mode == 1)
			{
				DrawBlock(r, c, isMine[r][c], isOpen[r][c]);
			}
			else
			{
				if(isOpen[r][c] == 2)
				{
					DrawBlock(r, c, 1, isOpen[r][c]);
				}
				else if(isOpen[r][c] == 0)
				{
					DrawBlock(r, c, 0, isOpen[r][c]);
				}
				else
				{
					DrawBlock(r, c, isMine[r][c], isOpen[r][c]);
				}
			}
		}
	}
	//临时辅助线
	setcolor(GRAY);
	setlinewidth(sideLength/32);
	for(r=0; r<heightOfBoard; r+=3)
	{
		line(0, (lengthOfColumnNumber+r)*sideLength, (lengthOfRowNumber+widthOfBoard)*sideLength, (lengthOfColumnNumber+r)*sideLength);
	}
	for(c=0; c<widthOfBoard; c+=3)
	{
		line((lengthOfRowNumber+c)*sideLength, 0, (lengthOfRowNumber+c)*sideLength, (lengthOfColumnNumber+heightOfBoard)*sideLength);
	}
}

void Operate(char operation, int r, int c)
{
	if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)
	{
		if(operation == '@' && isOpen[r][c] == 0) isOpen[r][c] = 1;
		if(operation == '#' && isOpen[r][c] != 1) isOpen[r][c] = 2-isOpen[r][c];
	}
	else if(r>=0 && r<heightOfBoard && c < 0)//行数字
	{
		if(rowNumberColor[r][c+lengthOfRowNumber] != GRAY)
		{
			rowNumberColor[r][c+lengthOfRowNumber] = GRAY;
		}
		else if(rowNumber[r][c+lengthOfRowNumber] == widthOfBoard)
		{
			rowNumberColor[r][c+lengthOfRowNumber] = DEEPSKYBLUE;
		}
		else if(rowNumber[r][c+lengthOfRowNumber] > widthOfBoard/2)
		{
			rowNumberColor[r][c+lengthOfRowNumber] = SPRINGGREEN;
		}
		else if(rowNumber[r][c+lengthOfRowNumber] > widthOfBoard/3)
		{
			rowNumberColor[r][c+lengthOfRowNumber] = LIGHTRED;
		}
		else
		{
			rowNumberColor[r][c+lengthOfRowNumber] = WHITE;
		}
	}
	else if(r < 0 && c>=0 && c<widthOfBoard)//列数字
	{
		if(columnNumberColor[r+lengthOfColumnNumber][c] != GRAY)
		{
			columnNumberColor[r+lengthOfColumnNumber][c] = GRAY;
		}
		else if(columnNumber[r+lengthOfColumnNumber][c] == heightOfBoard)
		{
			columnNumberColor[r+lengthOfColumnNumber][c] = DEEPSKYBLUE;
		}
		else if(columnNumber[r+lengthOfColumnNumber][c] > heightOfBoard/2)
		{
			columnNumberColor[r+lengthOfColumnNumber][c] = SPRINGGREEN;
		}
		else if(columnNumber[r+lengthOfColumnNumber][c] > heightOfBoard/3)
		{
			columnNumberColor[r+lengthOfColumnNumber][c] = LIGHTRED;
		}
		else
		{
			columnNumberColor[r+lengthOfColumnNumber][c] = WHITE;
		}
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
				rowNumberColor[r][c] = WHITE;
				if(rowNumber[r][c] != 0)
				{
					if(rowNumber[r][c] == widthOfBoard)
					{
						rowNumberColor[r][c] = DEEPSKYBLUE;
					}
					else if(rowNumber[r][c] > widthOfBoard/2)
					{
						rowNumberColor[r][c] = SPRINGGREEN;
					}
					else if(rowNumber[r][c] > widthOfBoard/3)
					{
						rowNumberColor[r][c] = LIGHTRED;
					}
				}
			}
		}
		for(r=0; r<lengthOfColumnNumber; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				columnNumberColor[r][c] = WHITE;
				if(columnNumber[r][c] != 0)
				{
					if(columnNumber[r][c] == heightOfBoard)
					{
						columnNumberColor[r][c] = DEEPSKYBLUE;
					}
					else if(columnNumber[r][c] > heightOfBoard/2)
					{
						columnNumberColor[r][c] = SPRINGGREEN;
					}
					else if(columnNumber[r][c] > heightOfBoard/3)
					{
						columnNumberColor[r][c] = LIGHTRED;
					}
				}
			}
		}
		/*if(summonCheckMode == 0) */break;
		/*else if(summonCheckMode == 1)
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
		}*/
	}
}
