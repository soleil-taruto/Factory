/*
	�I�v�V�����ȗ� -> ������IP

	- - - -

	�t�@�C�A�E�H�[����L���ɂ��Ă���� gethosybyname() �� IP �����Ȃ��B
	�u�t�@�C���ƃv�����^�̋��L�v���O�ɂ���Ǝ擾�ł���悤�ɂȂ�B
	ping �ɉ������邩���Ȃ������֌W�H
*/

#include "C:\Factory\Common\Options\SockClient.h"

int main(int argc, char **argv)
{
	autoList_t *ips = sockLookupList(hasArgs(1) ? nextArg() : "");
	uchar ip[4];
	uint index;

	foreach (ips, ip, index)
	{
		cout("%s\n", SockIp2Line(ip));
	}
	releaseAutoList(ips);
}
