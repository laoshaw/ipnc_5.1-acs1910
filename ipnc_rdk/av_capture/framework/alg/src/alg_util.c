
#include <alg_priv.h>
#include <osa_cmem.h>

int checkMinMax(int value, int min, int max)
{
	value = (value<min) ? min:value;
	value = (value>max) ? max:value;

	return value;
}
