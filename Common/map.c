#include "all.h"

map_t makeMap(void)
{
	map_t ret;

	ret.K = newList();
	ret.V = newList();

	return ret;
}
