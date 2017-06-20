#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <asm/types.h>

#include <av_server_ctrl.h>
#include <system_control.h>
#include <sys_env_type.h>

const unsigned int enc_framerate_day_1_dm368[10][6][4] =
{
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{60000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{60000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{20000,15000,8000},
		{20000,15000,8000},
		{10000},
		{10000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
};

const unsigned int enc_framerate_day_2_dm368[10][6][4] =
{
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
		{0},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
};
const unsigned int enc_framerate_day_3_dm368[10][6][4] =
{
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
	},
	{
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
};

const unsigned int enc_framerate_night_1_dm368[10][6][4] =
{
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{60000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{60000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{20000,15000,5000},
		{20000,15000,5000},
		{10000},
		{10000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
};

const unsigned int enc_framerate_night_2_dm368[10][6][4] =
{
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
		{0},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
};
const unsigned int enc_framerate_night_3_dm368[10][6][4] =
{
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0},
		{0,0,0},
	},
	{
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
};

const unsigned int enc_framerate_day_1_dm365[10][6][4] =
{
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{45000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{45000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{15000,8000},
		{15000,8000},
		{8000},
		{8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
};

const unsigned int enc_framerate_day_2_dm365[10][6][4] =
{
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0},
		{0,0},
		{0},
		{0},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{30000,24000,15000,8000},
		{24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
		{24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
};

const unsigned int enc_framerate_day_3_dm365[10][6][4] =
{
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0},
		{0,0},
		{0},
		{0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0,0},
		{0,0,0},
	},
	{
		{30000,24000,15000,8000},
	},
	{
		{30000,24000,15000,8000},
	},
};

const unsigned int enc_framerate_night_1_dm365[10][6][4] =
{
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{24000,15000,5000},
		{45000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{24000,15000,5000},
		{45000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{15000,5000},
		{15000,5000},
		{8000},
		{8000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{24000,15000,5000},
		{24000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{24000,15000,5000},
		{24000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{24000,15000,5000},
		{24000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{24000,15000,5000},
		{24000,15000,5000},
	},
	{
		{30000,15000,5000},
		{24000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
};

const unsigned int enc_framerate_night_2_dm365[10][6][4] =
{
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0},
		{0,0},
		{0},
		{0},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{24000,15000,5000},
		{24000,15000,5000},
	},
	{
		{30000,15000,5000},
		{30000,15000,5000},
		{30000,15000,5000},
		{24000,15000,5000},
		{24000,15000,5000},
	},
	{
		{30000,15000,5000},
		{24000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
};
const unsigned int enc_framerate_night_3_dm365[10][6][4] =
{
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0},
		{0,0},
		{0},
		{0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
	},
	{
		{0,0,0},
		{0,0,0},
	},
	{
		{30000,15000,5000},
	},
	{
		{30000,15000,5000},
	},
};
