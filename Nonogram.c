#include <stdio.h>
#include <stdlib.h>

int isMine[20][15];
int isMine[20][15];
int rowNumber[20][8];
int columnNumber[10][15];

// 初级6*6-27 中级10*10-64 高级15*12-90 专家20*15-148
int heightOfBoard = 6;
int widthOfBoard = 6;
int numberOfMine = 27;

void PrintBoard()
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
			if(isMine[r][c] == 1)
			{
				printf(" *");
			}
			else
			{
				printf("  ");
			}
		}
		printf("\n");
	}
}

void SummonBoard()
{
	int r, c, i, n;
	srand(0);
	// 初始化雷
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			isMine[r][c] = 0;
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
	SummonBoard();
	PrintBoard();
	return 0;
}
