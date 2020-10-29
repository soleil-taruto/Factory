#include "all.h"

#define HEADER "{fa2b2611-7ff1-4e17-a692-5d99b415aacc}" // shared_uuid:2
#define HEADER_SIZE 38

static char *EmbedConfig =
	HEADER
	"E" // EMBCFG_FACTORY_DIR_DISABLED
	"X" // EMBCFG_DUMMY_01
	"X" // EMBCFG_DUMMY_02
	"X" // EMBCFG_DUMMY_03
	;

int isFactoryDirDisabled(void)
{
	return EmbedConfig[HEADER_SIZE + EMBCFG_FACTORY_DIR_DISABLED] == 'D';
}
int isFactoryDirEnabled(void)
{
	return !isFactoryDirDisabled();
}
