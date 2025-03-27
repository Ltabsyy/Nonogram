//#define _WIN32_WINNT 0x0600//使用SetProcessDPIAware()
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <graphics.h>//链接参数-mwindows
/**
 * 数织 Easy Nonogram
 *
 * by Ltabsyy
 * 
 * https://github.com/Ltabsyy/Nonogram
 **/
#define LimLength 24
#define LimHeight LimLength
#define LimWidth LimLength
#define RefreshCycle 50
#define DragDeviation 1
#define InvalidPosition -2147483648

int isMine[LimHeight][LimWidth];
int isOpen[LimHeight][LimWidth];
int solution[LimHeight][LimWidth];
int rowNumber[LimHeight][(LimWidth+1)/2];
int columnNumber[(LimHeight+1)/2][LimWidth];
color_t rowNumberColor[LimHeight][(LimWidth+1)/2];
color_t columnNumberColor[(LimHeight+1)/2][LimWidth];

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
//int summonCheckMode = 3;

int lengthOfRowNumber = 3;
int lengthOfColumnNumber = 3;

int sideLength = 32;

int cursorR = InvalidPosition, cursorC = InvalidPosition;

//int debug = 0;

void DrawBlock(int r, int c, int isMine, int isOpen);
void DrawLineA(int x0, int y0, int r, int angle);
void DrawClock(int x0, int y0, int r);
void DrawWindow(int mode, int mstime, color_t timeColor);
void DrawTipLine(int r1, int c1, int r2, int c2);
void InitWindow(int mode);
void Operate(char operation, int r, int c);
int IsMousePosOutside();

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
int* LineFirstSolutionPosList();
int* LineLastSolutionPosList();
int SolveLine(struct LinesIterator li);
//int AirWeaveLine(struct LinesIterator li);
int SolveStep();
int Solve();

int main()
{
	mouse_msg mouseMsg;
	key_msg keyMsg;
	char operation;
	int r, c, isEnd, seed, t0, t1;
	int r1 = InvalidPosition, c1 = InvalidPosition, r2 = InvalidPosition, c2 = InvalidPosition;
	// 设置难度
	InitWindow(0);
	setcolor(WHITE);
	xyprintf(0, 0*sideLength, "初级： 6*6  - 27");
	xyprintf(0, 1*sideLength, "中级：10*10 - 64");
	xyprintf(0, 2*sideLength, "高级：12*15 - 90");
	xyprintf(0, 3*sideLength, "专家：15*20 - 148");
	xyprintf(0, 4*sideLength, "自定义");
	/*setfont(sideLength*15/16, 0, "黑体");
	xyprintf(0, 0*sideLength, "初级：");
	xyprintf(0, 1*sideLength, "中级：");
	xyprintf(0, 2*sideLength, "高级：");
	xyprintf(0, 3*sideLength, "专家：");
	xyprintf(0, 4*sideLength, "自定义");
	setfont(sideLength, 0, "Consolas");
	xyprintf(3*sideLength, 0*sideLength, " 6*6  - 27");
	xyprintf(3*sideLength, 1*sideLength, "10*10 - 64");
	xyprintf(3*sideLength, 2*sideLength, "12*15 - 90");
	xyprintf(3*sideLength, 3*sideLength, "15*20 - 148");*/
	int difficulty = -1;
	while(difficulty == -1)
	{
		while(mousemsg())
		{
			mouseMsg = getmouse();
			if(mouseMsg.is_up())
			{
				difficulty = mouseMsg.y/sideLength+1;
			}
		}
		while(kbmsg())
		{
			keyMsg = getkey();
			if(keyMsg.msg == key_msg_down)
			{
				if(keyMsg.key >= '0' && keyMsg.key <= '9')//选择难度
				{
					difficulty = keyMsg.key-'0'+0;
				}
				else if(keyMsg.key >= key_num0 && keyMsg.key <= key_num9)//小键盘识别
				{
					difficulty = keyMsg.key-key_num0+0;
				}
				else if(keyMsg.key == 'B') difficulty = 1;
				else if(keyMsg.key == 'I') difficulty = 2;
				else if(keyMsg.key == 'E') difficulty = 3;
				else if(keyMsg.key == 'G') difficulty = 4;
				else if(keyMsg.key == 'C') difficulty = 5;
			}
		}
		delay_ms(RefreshCycle);
	}
	if(difficulty == 1)
	{
		heightOfBoard = 6;
		widthOfBoard = 6;
		numberOfMine = 27;
	}
	else if(difficulty == 2)
	{
		heightOfBoard = 10;
		widthOfBoard = 10;
		numberOfMine = 64;
	}
	else if(difficulty == 3)
	{
		heightOfBoard = 12;
		widthOfBoard = 15;
		numberOfMine = 90;
	}
	else if(difficulty == 4)
	{
		heightOfBoard = 15;
		widthOfBoard = 20;
		numberOfMine = 148;
	}
	else
	{
		char str[64];
		resizewindow(13*32, 10*32);
		inputbox_getline("自定义难度输入框",
			"[行数] [列数] [雷数]\n注意空格，输入后回车。\n"
			"最大地图24*24，某些难度可能难以生成可解地图。\n"
			"什么？输入框太丑？请到https://github.com/wysaid/xege反馈！", str, 64);
		sscanf(str, "%d%d%d", &heightOfBoard, &widthOfBoard, &numberOfMine);
		if(heightOfBoard < 1) heightOfBoard = 1;
		if(heightOfBoard > LimHeight) heightOfBoard = LimHeight;
		if(widthOfBoard < 1) widthOfBoard = 1;
		if(widthOfBoard > LimWidth) widthOfBoard = LimWidth;
		if(numberOfMine < 0) numberOfMine = 0;
		if(numberOfMine > heightOfBoard * widthOfBoard) numberOfMine = heightOfBoard * widthOfBoard;
	}
	lengthOfRowNumber = (widthOfBoard+1)/2;
	lengthOfColumnNumber = (heightOfBoard+1)/2;
	if(lengthOfRowNumber < 2) lengthOfRowNumber = 2;
	if(lengthOfColumnNumber < 2) lengthOfColumnNumber = 2;
	// 游戏
	InitWindow(1);
	int newGame = 1;
	while(newGame == 1)
	{
		seed = time(0);
		SummonBoard(seed);
		flushmouse();
		flushkey();
		t0 = clock();
		while(1)
		{
			t1 = clock();
			DrawWindow(0, t1-t0, WHITE);
			DrawTipLine(r1, c1, r2, c2);
			operation = 0;
			isEnd = 0;
			if(IsMousePosOutside())
			{
				cursorR = InvalidPosition;
				cursorC = InvalidPosition;
			}
			while(mousemsg())
			{
				mouseMsg = getmouse();
				r = mouseMsg.y / sideLength - lengthOfColumnNumber;
				c = mouseMsg.x / sideLength - lengthOfRowNumber;
				r2 = r;
				c2 = c;
				cursorR = r;
				cursorC = c;
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
					DrawWindow(0, t1-t0, WHITE);
				}
				if(r1 != r2 && c1 != c2)//修正拖动偏差
				{
					int dr = r1-r2;
					int dc = c1-c2;
					if(dr < 0) dr *= -1;
					if(dc < 0) dc *= -1;
					if(dr > dc && dc <= DragDeviation) c2 = c1;
					if(dr < dc && dr <= DragDeviation) r2 = r1;
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
					else if(keyMsg.key == 'R')
					{
						if(seed != time(0))
						{
							seed = time(0);
							SummonBoard(seed);
							t0 = clock();
						}
					}
					else if(keyMsg.key == '\t')
					{
						SolveStep();
					}
				}
			}
			if(operation != 0 && (r1 == r2 || c1 == c2))
			{
				if(operation == '#')//根据起点统一标记/取消标记
				{
					if(r1>=0 && r1<heightOfBoard && c1>=0 && c1<widthOfBoard)
					{
						if(isOpen[r1][c1] == 2) operation = '%';
					}
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
						Operate(operation, r1, c);
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
						Operate(operation, r, c1);
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
		t1 = clock();
		if(isEnd == 1) DrawWindow(1, t1-t0, YELLOW);
		else DrawWindow(1, t1-t0, RED);
		delay_ms(1000);
		flushmouse();
		newGame = -1;
		while(newGame == -1)
		{
			if(isEnd == 1) DrawWindow(1, t1-t0, YELLOW);
			else DrawWindow(1, t1-t0, RED);
			setfont(sideLength/2, 0, "黑体");
			setcolor(RED);
			xyprintf(sideLength/4, sideLength*2, "左键新游戏");
			xyprintf(sideLength/4, sideLength*5/2, "右键关闭窗口");
			setfont(sideLength, 0, "Consolas");
			if(IsMousePosOutside())
			{
				cursorR = InvalidPosition;
				cursorC = InvalidPosition;
			}
			while(mousemsg())
			{
				mouseMsg = getmouse();
				cursorR = mouseMsg.y / sideLength - lengthOfColumnNumber;
				cursorC = mouseMsg.x / sideLength - lengthOfRowNumber;
				if(mouseMsg.is_up())
				{
					if(mouseMsg.is_left()) newGame = 1;
					else newGame = 0;
				}
				if(mouseMsg.is_wheel() && keystate(key_control))//调整显示大小
				{
					if(mouseMsg.wheel > 0) sideLength += 4;
					else if(sideLength > 16) sideLength -= 4;
					resizewindow((lengthOfRowNumber+widthOfBoard)*sideLength, (lengthOfColumnNumber+heightOfBoard)*sideLength);
					setfont(sideLength, 0, "Consolas");
					if(isEnd == 1) DrawWindow(1, t1-t0, YELLOW);
					else DrawWindow(1, t1-t0, RED);
				}
			}
			while(kbmsg())
			{
				keyMsg = getkey();
				if(keyMsg.msg == key_msg_down)
				{
					if(keyMsg.key == 'R') newGame = 1;
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

void DrawLineA(int x0, int y0, int r, int angle)//绘制时钟指针
{
	float rad;
	int x1, y1;
	rad = angle*PI/180;
	x1 = x0 + r*cos(rad);
	y1 = y0 + r*sin(rad);
	ege_line(x0, y0, x1, y1);
}

void DrawClock(int x0, int y0, int r)//绘制时钟
{
	int second, minute, hour;
	int t = time(0);
	// 获取时间
	second = t%60;
	minute = t/60;
	hour = minute/60+8;
	minute = minute%60;
	hour = hour%24;
	// 绘制时钟(r=20)
	setfillcolor(WHITE);
	ege_fillellipse(x0-r, y0-r, 2*r, 2*r);
	setlinewidth(r/10);
	setcolor(BLACK);
	//circle(x0, y0, r);
	ege_ellipse(x0-r, y0-r, 2*r, 2*r);
	//秒针
	setlinewidth(r/20);
	setcolor(RED);
	DrawLineA(x0, y0, r*4/5, 270+second*6);
	setcolor(BLACK);
	//分针
	setlinewidth(r/20);
	DrawLineA(x0, y0, r*3/4, 270+minute*6);
	//时针
	setlinewidth(r/10);
	DrawLineA(x0, y0, r/2, 270+hour%12*30+minute/12*6);//每12分钟跳一格
	//转轴
	setfillcolor(RED);
	ege_fillellipse(x0-r/10, y0-r/10, r/5, r/5);
}

void DrawWindow(int mode, int mstime, color_t timeColor)
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
	//剩余雷数
	int remainder = numberOfMine;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isOpen[r][c] == 2) remainder--;
		}
	}
	setcolor(RED);
	setlinewidth(sideLength/16);
	ege_ellipse(sideLength/4, sideLength/4, sideLength/2, sideLength/2);
	if(remainder == 0) setcolor(SPRINGGREEN);
	else setcolor(WHITE);
	xyprintf(sideLength, 0, "%d", remainder);
	//用时
	DrawClock(sideLength/2, sideLength*3/2, sideLength*10/32);
	setcolor(timeColor);
	if(timeColor == WHITE) xyprintf(sideLength, sideLength, "%d", mstime/1000);//游戏时去尾，终局四舍五入
	//else if(mstime < 10*1000) xyprintf(sideLength, sideLength, "%.2f", (float)mstime/1000);//刷新周期50ms下无意义
	else if(mstime < 100*1000) xyprintf(sideLength, sideLength, "%.1f", (float)mstime/1000);//仅在100秒内显示1位小数
	else xyprintf(sideLength, sideLength, "%d", (mstime+500)/1000);
	//悬浮高亮
	if(cursorR != InvalidPosition && cursorC != InvalidPosition)
	{
		setfillcolor(EGERGBA(0xff, 0xff, 0xff, 0x10));
		if(cursorR >= 0)//高亮行
		{
			ege_fillrect(0, (lengthOfColumnNumber+cursorR)*sideLength, (lengthOfRowNumber+widthOfBoard)*sideLength, sideLength);
		}
		if(cursorC >= 0)//高亮列
		{
			ege_fillrect((lengthOfRowNumber+cursorC)*sideLength, 0, sideLength, (lengthOfColumnNumber+heightOfBoard)*sideLength);
		}
		if((cursorR >= 0 && cursorC < 0) || (cursorR < 0 && cursorC >= 0))//高亮方块
		{
			ege_fillrect((lengthOfRowNumber+cursorC)*sideLength, (lengthOfColumnNumber+cursorR)*sideLength, sideLength, sideLength);
		}
	}
}

void DrawTipLine(int r1, int c1, int r2, int c2)
{
	int r, c;
	if(r1 != InvalidPosition && c1 != InvalidPosition && (r1 == r2 || c1 == c2))//拖动操作提示线
	{
		setcolor(BLUE);
		setlinewidth(sideLength/16);
		ege_line((lengthOfRowNumber+c1)*sideLength+sideLength/2, (lengthOfColumnNumber+r1)*sideLength+sideLength/2,
			(lengthOfRowNumber+c2)*sideLength+sideLength/2, (lengthOfColumnNumber+r2)*sideLength+sideLength/2);
		setfillcolor(BLUE);
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
				ege_fillellipse((lengthOfRowNumber+c)*sideLength+sideLength*3/8,
					(lengthOfColumnNumber+r1)*sideLength+sideLength*3/8, sideLength/4, sideLength/4);
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
				ege_fillellipse((lengthOfRowNumber+c1)*sideLength+sideLength*3/8,
					(lengthOfColumnNumber+r)*sideLength+sideLength*3/8, sideLength/4, sideLength/4);
			}
		}
	}
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
		setbkmode(TRANSPARENT);
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
		if(operation == '#' && isOpen[r][c] == 0) isOpen[r][c] = 2;
		if(operation == '%' && isOpen[r][c] == 2) isOpen[r][c] = 0;
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

int IsMousePosOutside()//鼠标在窗口边界外
{
	//EGE无法区分鼠标静止和鼠标在窗口边界外，调用WindowsAPI
	HWND hwnd = getHWnd();//获取绘图窗口句柄
	RECT rect;
	POINT point;
	GetWindowRect(hwnd, &rect);//获取窗口四角坐标
	GetCursorPos(&point);//获取鼠标屏幕坐标
	return (point.x < rect.left || point.x > rect.right || point.y < rect.top || point.y > rect.bottom);
	//ScreenToClient(hwnd, &point);//转换为窗口坐标
	//窗口大小rect.right-rect.left+1, rect.bottom-rect.top+1
	//return (point.x <= 0 || point.x > rect.right-rect.left || point.y <= 0 || point.y > rect.bottom-rect.top);
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
Easy Nonogram 0.3
——新增 鼠标悬浮高亮
——新增 自定义难度
——修复 终局后的Ctrl+滚轮异常改变时间
Easy Nonogram 0.4
——新增 剩余雷数显示
——新增 按数字键或BIEGC选择难度
——新增 按R重置地图
——优化 自定义难度输入框边距
——优化 独立分析取消标记操作
——优化 设置难度鼠标在界外松开时为自定义难度
——优化 鼠标任意键均可设置难度
Easy Nonogram 0.5
——新增 拖动操作提示线节点
——优化 算法跟随Nonogram 0.6升级
——修复 高难度长按R可能卡顿
Easy Nonogram 0.6
——新增 允许拖动操作偏差一格
——优化 算法跟随Nonogram 0.7升级
——优化 更精确的计算用时
——优化 游戏结束时延时一秒防误触
Easy Nonogram 0.7
——新增 游戏结束时小于100秒的用时显示小数
——优化 过小地图维持数字占用长度至少为2
——优化 算法跟随Nonogram 0.8升级
--------------------------------*/
