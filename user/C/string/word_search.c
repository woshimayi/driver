/*
 * @*************************************:
 * @FilePath: /user/C/string/word_search.c
 * @version:
 * @Author: dof
 * @Date: 2022-07-02 13:28:13
 * @LastEditors: dof
 * @LastEditTime: 2022-07-02 13:34:06
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
	false,
	true
}bool；

int directions[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

bool check(char **board, int boardSize, int boardColSize, int **visited, int i, int j, char *s, int sSize, int k)
{
	if (board[i][j] != s[k])
	{
		return false;
	}
	else if (k == sSize - 1)
	{
		return true;
	}
	visited[i][j] = true;
	bool result = false;
	for (int sel = 0; sel < 4; sel++)
	{
		int newi = i + directions[sel][0], newj = j + directions[sel][1];
		if (newi >= 0 && newi < boardSize && newj >= 0 && newj < boardColSize)
		{
			if (!visited[newi][newj])
			{
				bool flag = check(board, boardSize, boardColSize, visited, newi, newj, s, sSize, k + 1);
				if (flag)
				{
					result = true;
					break;
				}
			}
		}
	}
	visited[i][j] = false;
	return result;
}

bool exist(char **board, int boardSize, int *boardColSize, char *word)
{
	int **visited = malloc(sizeof(int *) * boardSize);
	for (int i = 0; i < boardSize; i++)
	{
		visited[i] = malloc(sizeof(int) * boardColSize[0]);
		memset(visited[i], 0, sizeof(int) * boardColSize[0]);
	}
	
	int wordSize = strlen(word);
	for (int i = 0; i < boardSize; i++)
	{
		for (int j = 0; j < boardColSize[0]; j++)
		{
			bool flag = check(board, boardSize, boardColSize[0], visited, i, j, word, wordSize, 0);
			if (flag)
			{
				return true;
			}
		}
	}
	return false;
}
