#include <ti/sdo/ce/osal/Global.h>

static Global_ArmDspLinkConfigMemTableEntry memTable_audio_copy_x64P[] = {
    {"DDR2", 0x87800000, 0x87800000, 0x00600000, 1, 0},
    {"DSPLINKMEM", 0x87E00000, 0x87E00000, 0x00100000, 1, 0},
    {"RESET_VECTOR", 0x87F00000, 0x87F00000, 0x00001000, 1, 0},
    {"L4PER", 0x49000000, 0x49000000, 0x00100000, 0, 0},
    {"IRAM", 0x107F8000, 0x5C7F8000, 0x00008000, 1, 0},
    {"L1DSRAM", 0x10F04000, 0x5CF04000, 0x0000C000, 1, 0},
    {"DDRALGHEAP", 0x86000000, 0x86000000, 0x00001000, 0, 0},
    {"L4CORE", 0x48000000, 0x48000000, 0x01000000, 0, 0},
    {"CMEM", 0x00000000, 0x00000000, 0x00000000, 0, 0},
    {NULL, 0, 0, 0, 0, 0}
};
