/*
	三つの団子の山から、交互に団子を取る。
	一つの山から団子を取ります。取れなくなったら負けです。
*/

#include "C:\Factory\Common\all.h"

static int IsWinMyTurn(int a, int b, int c);

static int IsWinOSTurn(int a, int b, int c)
{
	int n;

	if (a == 0 && b == 0 && c == 0) // ? 勝ち(相手の負け)
		return 1;

	for (n = 0; n < a; n++)
		if (!IsWinMyTurn(n, b, c))
			return 0;

	for (n = 0; n < b; n++)
		if (!IsWinMyTurn(a, n, c))
			return 0;

	for (n = 0; n < c; n++)
		if (!IsWinMyTurn(a, b, n))
			return 0;

	return 1; // (自分が)負ける相手の手が無い -> 勝てる
}
static int IsWinMyTurn(int a, int b, int c)
{
	int n;

	if (a == 0 && b == 0 && c == 0) // ? 負け
		return 0;

	for (n = 0; n < a; n++)
		if (IsWinOSTurn(n, b, c))
			return 1;

	for (n = 0; n < b; n++)
		if (IsWinOSTurn(a, n, c))
			return 1;

	for (n = 0; n < c; n++)
		if (IsWinOSTurn(a, b, n))
			return 1;

	return 0; // 勝てる自分の手が無い -> 負ける
}

int main(int argc, char **argv)
{
	int tallMax = 9;
	int a;
	int b;
	int c;

	for (a = 0; a <= tallMax; a++)
	for (b = a; b <= tallMax; b++)
	for (c = b; c <= tallMax; c++)
	{
		cout(
			"団子の高さが %d %d %d のとき先攻で%s, 後攻で%s\n"
			,a
			,b
			,c
			,IsWinMyTurn(a, b, c) ? "勝てる" : "負ける"
			,IsWinOSTurn(a, b, c) ? "勝てる" : "負ける"
			);
	}
	cout("\n");
	cout("取った後以下になれば勝てる...\n");

	for (a = 0; a <= tallMax; a++)
	for (b = 0; b <= a; b++)
	for (c = 0; c <= b; c++)
	{
		if (IsWinOSTurn(a, b, c))
			cout("団子の高さ %d %d %d\n", a, b, c);
	}
}
