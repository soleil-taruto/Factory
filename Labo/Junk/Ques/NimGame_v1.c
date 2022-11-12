/*
	�O�̒c�q�̎R����A���݂ɒc�q�����B
	��̎R����c�q�����܂��B���Ȃ��Ȃ����畉���ł��B
*/

#include "C:\Factory\Common\all.h"

static int IsWinMyTurn(int a, int b, int c);

static int IsWinOSTurn(int a, int b, int c)
{
	int n;

	if (a == 0 && b == 0 && c == 0) // ? ����(����̕���)
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

	return 1; // (������)�����鑊��̎肪���� -> ���Ă�
}
static int IsWinMyTurn(int a, int b, int c)
{
	int n;

	if (a == 0 && b == 0 && c == 0) // ? ����
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

	return 0; // ���Ă鎩���̎肪���� -> ������
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
			"�c�q�̍����� %d %d %d �̂Ƃ���U��%s, ��U��%s\n"
			,a
			,b
			,c
			,IsWinMyTurn(a, b, c) ? "���Ă�" : "������"
			,IsWinOSTurn(a, b, c) ? "���Ă�" : "������"
			);
	}
	cout("\n");
	cout("�������ȉ��ɂȂ�Ώ��Ă�...\n");

	for (a = 0; a <= tallMax; a++)
	for (b = 0; b <= a; b++)
	for (c = 0; c <= b; c++)
	{
		if (IsWinOSTurn(a, b, c))
			cout("�c�q�̍��� %d %d %d\n", a, b, c);
	}
}
