#include "C:\Factory\Common\all.h"
#include "C:\Factory\Satellite\libs\Flowertact\Fortewave.h"

#define FORTEWAVE_IDENT "{319a5123-066b-44fe-b754-5cb16cc9c79d}" // shared_uuid@g

int main(int argc, char **argv)
{
	autoBlock_t gab;
	Frtwv_t *i = Frtwv_Create(FORTEWAVE_IDENT);

	if (argIs("/C"))
	{
		Frtwv_Clear(i);
	}

	if (argIs("START"))
	{
		Frtwv_SendOL(i, gndBlockLineVar("START", gab), 0);
	}
	else if (argIs("CLEAR"))
	{
		Frtwv_SendOL(i, gndBlockLineVar("CLEAR", gab), 0);
	}
	else if (argIs("END"))
	{
		Frtwv_SendOL(i, gndBlockLineVar("END", gab), 0);
	}
	else if (argIs("EXIT"))
	{
		Frtwv_SendOL(i, gndBlockLineVar("EXIT", gab), 0);
	}
	else
	{
		error_m("不明なコマンド");
	}
	Frtwv_Release(i);
}
