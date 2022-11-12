#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\mt19937.h"
#include "LCom.h"

int main(int argc, char **argv)
{
	autoList_t *keys = newList();
	autoList_t *values = newList();
	uint id;
	char *prekey;

	id = toValue(nextArg()); // 1 å≈íË
	prekey = nextArg();

	mt19937_init32(toValue(nextArg()));

	while (waitKey(0) != 0x1b)
	{
		if (mt19937_rnd(2))
		{
			char *key = xcout("%u", mt19937_rnd(100));
			char *value = xcout("%u", mt19937_rnd32());
			char *command;
			char *result;

			command = xcout("%s.%s=%s", prekey, key, value);
			cout("SET: %s\n", command);
			result = LComSendLine(command, id);
			cout("RET: %s\n", result);

			errorCase(strcmp(result, "Settled"));

			// Ç†Ç¡ÇΩÇÁçÌèú
			{
				uint keyidx = findLine(keys, key);

				if (keyidx < getCount(keys))
				{
					memFree((void *)fastDesertElement(keys, keyidx));
					memFree((void *)fastDesertElement(values, keyidx));
				}
			}
			addElement(keys, (uint)key);
			addElement(values, (uint)value);

			memFree(command);
			memFree(result);
		}
		else if (getCount(keys))
		{
			uint keyidx = mt19937_rnd(getCount(keys));
			char *key;
			char *value;
			char *command;
			char *result;

			key = getLine(keys, keyidx);
			value = getLine(values, keyidx);

			command = xcout("%s.%s", prekey, key);
			cout("GET: %s\n", command);
			result = LComSendLine(command, id);
			cout("RET: %s (%s)\n", result, value);

			errorCase(strcmp(result, value));

			memFree(command);
			memFree(result);
		}
	}
}
