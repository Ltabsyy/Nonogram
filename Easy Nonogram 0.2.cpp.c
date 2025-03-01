#include <stdlib.h>
#include <time.h>
#include <graphics.h>

#define LimHeight 24
#define LimWidth 24
#define RefreshCycle 50
#define InvalidPosition -2147483648

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
void DrawWindow(int mode, int time, color_t timeColor);
void InitWindow(int mode);
void Operate(char operation, int r, int c);

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
//int AirWeaveLine(struct LinesIterator li);
int SolveStep();
int Solve();

int main()
{
	mouse_msg mouseMsg;
	key_msg keyMsg;
	char operation;
	int r, c, isEnd, t0;
	int r1 = InvalidPosition, c1 = InvalidPosition, r2 = InvalidPosition, c2 = InvalidPosition;
	// 设置难度
	InitWindow(0);
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
		delay_ms(RefreshCycle);
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
	}
	lengthOfRowNumber = (widthOfBoard+1)/2;
	lengthOfColumnNumber = (heightOfBoard+1)/2;
	// 游戏
	InitWindow(1);
	int newGame = 1;
	while(newGame == 1)
	{
		t0 = time(0);
		SummonBoard(t0);
		flushmouse();
		flushkey();
		while(1)
		{
			DrawWindow(0, time(0)-t0, WHITE);
			if(r1 != InvalidPosition && c1 != InvalidPosition && (r1 == r2 || c1 == c2))
			{
				setcolor(BLUE);
				setlinewidth(sideLength/16);
				ege_line((lengthOfRowNumber+c1)*sideLength+sideLength/2, (lengthOfColumnNumber+r1)*sideLength+sideLength/2,
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
				if(mouseMsg.is_wheel() && keystate(key_control))//调整显示大小
				{
					if(mouseMsg.wheel > 0) sideLength += 4;
					else if(sideLength > 16) sideLength -= 4;
					resizewindow((lengthOfRowNumber+widthOfBoard)*sideLength, (lengthOfColumnNumber+heightOfBoard)*sideLength);
					setfont(sideLength, 0, "Consolas");
					DrawWindow(0, time(0)-t0, WHITE);
				}
			}
			while(kbmsg())
			{
				keyMsg = getkey();
				if(keyMsg.msg == key_msg_down)
				{
					if(keyMsg.key == ' ')
					{
						if(CheckSign() == 1)
						{
							isEnd = 1;
							break;
						}
					}
					if(keyMsg.key == '\t')
					{
						SolveStep();
					}
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
				r1 = InvalidPosition;
				c1 = InvalidPosition;
				r2 = InvalidPosition;
				c2 = InvalidPosition;
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
			delay_ms(RefreshCycle);
		}
		if(isEnd == 1) DrawWindow(1, time(0)-t0, YELLOW);
		else DrawWindow(1, time(0)-t0, RED);
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
				if(mouseMsg.is_wheel() && keystate(key_control))//调整显示大小
				{
					if(mouseMsg.wheel > 0) sideLength += 4;
					else if(sideLength > 16) sideLength -= 4;
					resizewindow((lengthOfRowNumber+widthOfBoard)*sideLength, (lengthOfColumnNumber+heightOfBoard)*sideLength);
					setfont(sideLength, 0, "Consolas");
					DrawWindow(0, time(0)-t0, WHITE);
				}
			}
			delay_ms(RefreshCycle);
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
			ege_line(x+sideLength/4, y+sideLength/4, x+sideLength*3/4, y+sideLength*3/4);
			ege_line(x+sideLength/4, y+sideLength*3/4, x+sideLength*3/4, y+sideLength/4);
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

void DrawWindow(int mode, int time, color_t timeColor)
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
	//用时
	setcolor(timeColor);
	xyprintf(sideLength/4, 0, "%ds", time);
}

void InitWindow(int mode)
{
	int screenHeight, screenWidth;
	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) == 0)//无法获取显示屏分辨率
	{
		screenWidth = 1920;
		screenHeight = 1080;
	}
	else
	{
		screenWidth = dm.dmPelsWidth;
		screenHeight = dm.dmPelsHeight;
	}
	if(mode == 0)
	{
		if(screenHeight >= 2160) sideLength = 64;
		else if(screenHeight >= 1440) sideLength = 44;
		else if(screenHeight >= 1080) sideLength = 32;
		else sideLength = 24;
		setcaption("Easy Nonogram");
		SetProcessDPIAware();//避免Windows缩放造成模糊
		initgraph(10*sideLength, 5*sideLength, INIT_RENDERMANUAL);
		setbkcolor(EGERGB(18, 18, 18));
		setfont(sideLength, 0, "Consolas");
		ege_enable_aa(true);
	}
	else
	{
		while((widthOfBoard+lengthOfRowNumber)*sideLength > screenWidth
			|| (heightOfBoard+lengthOfColumnNumber+3)*sideLength > screenHeight)
		{
			sideLength -= 4;
		}
		if(sideLength < 16) sideLength = 16;
		resizewindow((lengthOfRowNumber+widthOfBoard)*sideLength, (lengthOfColumnNumber+heightOfBoard)*sideLength);
		setfont(sideLength, 0, "Consolas");
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
			if(pos + lineNumber[i] > lengthOfLine)//无解情况
			{
				free(mine);
				return NULL;
			}
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
			if(pos < 0)//无解情况
			{
				free(mine);
				return NULL;
			}
		}
	}
	return mine;
}

int SolveLine(struct LinesIterator li)
{
	int i, ni;
	//生成标准线
	RecoverLine(li.r, li.c, 0);
	//端向心分析
	int nipos, check, nstart, nend, nlimit;
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
						for(; i<lengthOfLine; i++)
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
								i+=2;
							}
							else if(lineOpen[i] == 0)
							{
								if(lineOpen[i+1] != 2)//连续两个未知方块或单个未知方块和翻开
								{
									i+=2;
								}
								else//未知方块尾随标记
								{
									i++;//定位到标记
									if(lineOpen[i-1] == 0) lineSolution[i-1] = 1;
									if(i+1 < lengthOfLine && lineOpen[i+1] == 0) lineSolution[i+1] = 1;
									i+=2;
								}
							}
						}
					}//需要i回溯或处理衔接，复杂且效益不高
					break;
				}
			}
			else//大数预置分析
			{
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
			//break;
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
						for(; i>=0; i--)
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
								i-=2;
							}
							else if(lineOpen[i] == 0)
							{
								if(lineOpen[i-1] != 2)//连续两个未知方块或单个未知方块和翻开
								{
									i-=2;
								}
								else//未知方块尾随标记
								{
									i--;//定位到标记
									if(i > 0 && lineOpen[i-1] == 0) lineSolution[i-1] = 1;
									if(lineOpen[i+1] == 0) lineSolution[i+1] = 1;
									i-=2;
								}
							}
						}
					}
					break;
				}
			}
			else//大数预置分析
			{
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
				if(check == 0) break;
				//break;
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
		}
	}
	//首末解交汇分析
	int* firstMine = LineFirstSolutionMine();//生成首解雷场
	int* lastMine = LineLastSolutionMine();//生成末解雷场
	if(firstMine != NULL && lastMine != NULL)
	{
		int matchFirstLast = 1;
		for(i=0; i<lengthOfLine; i++)
		{
			if(firstMine[i] != lastMine[i]) matchFirstLast = 0;
		}
		if(matchFirstLast == 1)//首解末解完全相同，可判断端收束顶满线
		{
			for(i=0; i<lengthOfLine; i++)
			{
				if(firstMine[i] == 1 && lineOpen[i] == 0) lineSolution[i] = 2;
				if(firstMine[i] == 0 && lineOpen[i] == 0) lineSolution[i] = 1;
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
				if(ni == 0 && n1pos > 0)//线首翻开
				{
					for(i=0; i<n1pos; i++)
					{
						if(lineOpen[i] == 0) lineSolution[i] = 1;
					}
				}
				if(ni == countOfLineNumber-1 && n2pos+lineNumber[ni] < lengthOfLine)//线尾翻开
				{
					for(i=n2pos+lineNumber[ni]; i<lengthOfLine; i++)
					{
						if(lineOpen[i] == 0) lineSolution[i] = 1;
					}
				}
				n1pos += lineNumber[ni];
				n2pos += lineNumber[ni];
			}
		}
	}
	else//存在错误标记
	{
		if(firstMine != NULL) free(firstMine);
		if(lastMine != NULL) free(lastMine);
		return 0;
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
	//标准线解写出
	RecoverLine(li.r, li.c, 1);
	if(firstMine != NULL) free(firstMine);
	if(lastMine != NULL) free(lastMine);
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
Easy Nonogram 0.2
——新增 基于Nonogram 0.6a算法生成可解地图
——新增 按Tab自动推进数织进度
——新增 游戏时按空格执行标记校验
——新增 显示大小适配各种屏幕分辨率
——新增 Ctrl+滚轮调整显示大小
——优化 错误标记线条
——优化 全局抗锯齿
--------------------------------*/
