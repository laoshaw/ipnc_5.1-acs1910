#include <avserver.h>

char TILogo_Y_80_32[] = {
0xfb,0xfd,0xfe,0xf6,0xf9,0xfb,0xfb,0xfe,0xfd,0xfe,0xf8,0xf8,0xfd,0xe8,0xfc,0xfd,
0xfa,0xf5,0xf6,0xfe,0xfd,0xf9,0xf9,0xf7,0xfa,0xfb,0xf7,0xf6,0xf8,0xfb,0xf9,0xf9,
0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,
0xfc,0xfd,0xfd,0xfd,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,
0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfd,0xfb,0xfb,0xfb,0xfb,0xf9,0xfd,0xfe,
0xeb,0xf3,0xf1,0xf0,0xfd,0xfd,0xee,0xdb,0xe5,0xef,0xf4,0xf4,0xeb,0xfe,0xfe,0xe2,
0xfd,0xfa,0xf2,0xf3,0xf3,0xf0,0xf3,0xf3,0xf5,0xf5,0xf5,0xf4,0xf5,0xf5,0xf5,0xf4,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf7,0xf5,0xf5,0xf7,0xf6,0xf5,0xf9,0xfe,
0xf9,0xfc,0xf4,0xed,0xf6,0xef,0xb9,0x7c,0x7c,0x6e,0x98,0xf4,0xea,0xe7,0xf2,0xfa,
0xeb,0xf1,0xef,0xed,0xf4,0xf6,0xf0,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,
0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,
0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,
0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf1,0xf3,0xf0,0xf1,0xf3,0xf3,0xf2,0xf7,0xfd,
0xf3,0xf4,0xf0,0xeb,0xf6,0xf3,0xb0,0x58,0x5e,0x5f,0x77,0xe3,0xef,0xd7,0xf5,0xf0,
0xef,0xf9,0xfb,0xf0,0xf5,0xf9,0xef,0xf5,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf2,0xf3,
0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,
0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,
0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf2,0xf0,0xf0,0xf3,0xf4,0xf4,0xf8,0xfe,
0xf1,0xf2,0xf1,0xef,0xfa,0xfa,0xbd,0x5c,0x5b,0x63,0x75,0xe3,0xd4,0x52,0xb5,0xf8,
0xec,0xef,0xfc,0xef,0xed,0xf3,0xeb,0xf7,0xf2,0xf3,0xf3,0xf3,0xf3,0xf3,0xf2,0xf2,
0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,
0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,
0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf0,0xf1,0xf4,0xf5,0xf5,0xf9,0xfe,
0xf9,0xf7,0xf7,0xf2,0xf4,0xf4,0xb8,0x54,0x65,0x6e,0x56,0xa4,0xcc,0x9e,0xd6,0x8a,
0x9a,0xa3,0xe0,0xf3,0xf0,0xf7,0xec,0xf4,0xf1,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf1,0xef,0xf0,0xf3,0xf3,0xf3,0xf8,0xfe,
0xf5,0xec,0xf0,0xf2,0xf4,0xf4,0xbf,0x5c,0x62,0x52,0x63,0xac,0x9f,0x94,0xbe,0x62,
0x53,0x5f,0xc8,0xfb,0xf5,0xf9,0xef,0xf3,0xf1,0xf1,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,
0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,
0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf4,0xf4,0xf4,0xf5,
0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf5,0xf2,0xf0,0xf1,0xf3,0xf4,0xf3,0xf8,0xfe,
0xfd,0xf1,0xf6,0xf8,0xf6,0xf3,0xbf,0x59,0x5f,0x72,0xc1,0xd1,0x65,0x93,0xdc,0xaa,
0x67,0x63,0xcf,0xfa,0xe9,0xef,0xed,0xf4,0xf3,0xf3,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,
0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,
0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf1,0xf1,0xf1,0xf2,
0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf5,0xf3,0xf3,0xf5,0xf5,0xf5,0xf9,0xfe,
0xfb,0xee,0xbc,0xa3,0x96,0xaf,0x88,0x58,0x55,0x8f,0xec,0xda,0x59,0x9f,0xed,0xa6,
0x5d,0x66,0xa4,0xf5,0xef,0xea,0xf6,0xf3,0xe6,0xe3,0xf4,0xef,0xe5,0xf5,0xe8,0xf3,
0xf6,0xe5,0xfe,0xf2,0xf6,0xf6,0xf5,0xf3,0xeb,0xf0,0xf2,0xf4,0xec,0xf6,0xf6,0xea,
0xf9,0xea,0xf8,0xf6,0xef,0xfd,0xf8,0xed,0xfb,0xed,0xf4,0xf6,0xf1,0xf2,0xf1,0xf1,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf2,0xf3,0xf4,0xf2,0xf2,0xf8,0xfe,
0xf1,0xef,0xa5,0x54,0x71,0x5a,0x4d,0x69,0x6c,0x5c,0xa7,0xad,0x5c,0xba,0x93,0x76,
0x56,0x5b,0x6a,0xd2,0xf9,0xfe,0xdb,0x67,0x24,0x3b,0x36,0x14,0x2a,0x29,0x40,0x9f,
0xfb,0xf5,0xea,0xf3,0xe6,0xf5,0xeb,0xfc,0xfa,0xf6,0xed,0xf8,0xf3,0xe3,0xf3,0xf7,
0xf0,0xfc,0xf1,0xee,0xf8,0xea,0xe1,0xf7,0xe6,0xee,0xf9,0xf2,0xea,0xf4,0xf7,0xf2,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf2,0xf3,0xf4,0xf2,0xf2,0xf8,0xfe,
0xf8,0xf5,0xe6,0x90,0x5e,0x55,0x71,0x5f,0x62,0x59,0xa0,0xa0,0x77,0xbd,0x4e,0x5f,
0x67,0x63,0x65,0xe0,0xf9,0xe8,0xe4,0xa4,0xc0,0xb8,0x54,0x15,0xa5,0xb9,0x97,0xc3,
0xfd,0xf7,0xfb,0xf8,0xed,0xfe,0xee,0xef,0xe6,0xfb,0xe7,0xf6,0xf6,0xfe,0xfe,0xec,
0xee,0xfb,0xf1,0xf1,0xfe,0xfe,0xfe,0xfe,0xfe,0xf6,0xf1,0xf4,0xf5,0xf2,0xf3,0xf4,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf2,0xf2,0xf3,0xf2,0xf3,0xf4,0xf2,0xf2,0xf8,0xfe,
0xf7,0xea,0xfa,0xf2,0x6c,0x63,0x78,0x4d,0x50,0x60,0xb9,0x8f,0x82,0xa1,0x71,0x5e,
0x6b,0x50,0x5d,0xdc,0xf9,0xe8,0xf7,0xf8,0xfa,0xfb,0x6f,0x00,0xcf,0xf8,0x99,0x5a,
0x55,0x6a,0x4c,0x91,0xd2,0x5a,0x5e,0xb3,0xfd,0x61,0x74,0xd0,0xfb,0xd9,0x65,0x99,
0xfb,0xe9,0xfe,0xde,0x7f,0x5d,0x62,0x70,0xe8,0xf3,0xf1,0xf6,0xf8,0xf0,0xf2,0xf2,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf2,0xf2,0xf3,0xf2,0xf3,0xf4,0xf2,0xf2,0xf8,0xfe,
0xf7,0xec,0xed,0xfa,0xb2,0x64,0x52,0x8e,0x7f,0x52,0xc6,0xd0,0xdb,0xd8,0xa2,0x4f,
0x5c,0x6b,0x9f,0xe1,0xef,0xfd,0xf4,0xea,0xeb,0xfd,0x76,0x01,0xc4,0xf9,0x8d,0x05,
0x70,0x9c,0x89,0x81,0xe5,0x68,0x01,0x8d,0xbc,0x19,0xa7,0xf1,0xfe,0x8e,0x01,0x2d,
0xed,0xee,0xef,0x6c,0x44,0xad,0xa4,0x69,0xb7,0xef,0xf7,0xf3,0xf2,0xf0,0xf9,0xf2,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf2,0xf2,0xf2,0xf2,0xf2,0xf3,0xf4,0xf2,0xf2,0xf8,0xfe,
0xf7,0xf8,0xef,0xe9,0xf4,0xa3,0x99,0xda,0xcf,0x59,0xaa,0xef,0xf1,0xe8,0x86,0x5d,
0xa4,0xcb,0xf9,0xfd,0xe3,0xfb,0xed,0xf7,0xf3,0xf6,0x6a,0x01,0xc7,0xfe,0x9a,0x01,
0xea,0xfe,0xfe,0xdd,0xfe,0xfd,0x4e,0x1e,0x36,0x93,0xfe,0xf8,0xe9,0x4f,0x81,0x01,
0x9b,0xfe,0xe7,0x1f,0x31,0xec,0xfe,0xec,0xed,0xfe,0xe7,0xf3,0xfe,0xe9,0xf3,0xfb,
0xf3,0xf3,0xf3,0xf3,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf3,0xf4,0xf2,0xf2,0xf8,0xfe,
0xf0,0xf0,0xfb,0xee,0xf9,0xe7,0xf1,0xf2,0xf3,0x92,0x60,0x95,0x89,0x88,0x64,0xae,
0xf4,0xf4,0xf4,0xf7,0xe7,0xf3,0xec,0xf9,0xed,0xfe,0x72,0x08,0xc0,0xf5,0x99,0x14,
0x3b,0x60,0x2c,0xc3,0xf5,0xf3,0xf3,0x18,0x4a,0xf8,0xed,0xf3,0xb4,0x56,0xfe,0x36,
0x39,0xe7,0xfe,0x7c,0x0f,0x01,0x2a,0x8e,0xef,0xfe,0xe3,0xf4,0xfe,0xe8,0xf0,0xf7,
0xf3,0xf3,0xf3,0xf3,0xf2,0xf2,0xf2,0xf2,0xf2,0xf2,0xf3,0xf4,0xf2,0xf2,0xf8,0xfe,
0xfa,0xf5,0xec,0xfb,0xe9,0xf8,0xf2,0xee,0xe8,0xe8,0x59,0x5e,0x62,0x54,0x90,0xf2,
0xea,0xf5,0xe3,0xf7,0xf0,0xf1,0xf2,0xee,0xef,0xfe,0x5e,0x02,0xc6,0xfe,0x95,0x03,
0x8d,0xa7,0x96,0xbd,0xfa,0xf6,0xe1,0x39,0x15,0xc1,0xfc,0xfe,0x55,0x53,0x9b,0x2d,
0x06,0xb3,0xfe,0xf5,0xae,0x5c,0x33,0x01,0x69,0xeb,0xfe,0xef,0xee,0xf3,0xfe,0xe5,
0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf2,0xf3,0xf4,0xf2,0xf2,0xf8,0xfe,
0xf8,0xfc,0xed,0xf0,0xf2,0xee,0xf1,0xf3,0xf7,0xe4,0xa6,0x5d,0x63,0x5a,0xca,0xf0,
0xf2,0xed,0xf4,0xed,0xf7,0xf6,0xf0,0xee,0xf5,0xfb,0x6a,0x04,0xc6,0xfd,0xa8,0x08,
0xdb,0xff,0xf2,0xfc,0xf1,0xff,0x4d,0x56,0x1f,0x37,0xf8,0xdc,0x38,0x62,0x61,0x68,
0x07,0x66,0xd8,0xdc,0xf8,0xfd,0xfa,0x3f,0x31,0xe1,0xf2,0xf8,0xf3,0xee,0xfe,0xed,
0xf6,0xf0,0xf3,0xf6,0xf3,0xec,0xf5,0xef,0xee,0xf9,0xfb,0xef,0xf5,0xfb,0xf2,0xf9,
0xfd,0xe6,0xf3,0xfe,0xf9,0xf0,0xf9,0xf6,0xe7,0xf9,0xdf,0x76,0x65,0x60,0xbc,0xf3,
0xf6,0xf4,0xf8,0xf5,0xf0,0xf2,0xf6,0xee,0xde,0xfe,0x58,0x0b,0xbd,0xf4,0x79,0x04,
0x68,0xa4,0x86,0x65,0xc7,0x7b,0x16,0xec,0x9a,0x06,0x77,0x68,0x57,0xfe,0xf6,0xf6,
0x29,0x1c,0xb7,0x2b,0x6a,0xab,0x8c,0x22,0x7c,0xfe,0xf7,0xe9,0xf6,0xe7,0xed,0xfe,
0xf9,0xe3,0xfd,0xf0,0xf1,0xfe,0xe7,0xfb,0xfc,0xe7,0xf0,0xf4,0xeb,0xf1,0xfa,0xff,
0xfe,0xea,0xc3,0x8a,0xa6,0xe7,0xf0,0xe4,0xf7,0xe9,0xfb,0xe1,0xa4,0x8b,0xd2,0xf4,
0xe1,0xf8,0xf3,0xf6,0xef,0xf0,0xef,0xf3,0xfd,0xd0,0x84,0x4d,0x9e,0xfe,0x86,0x6d,
0x60,0x4e,0x5b,0x73,0xa5,0x6c,0x95,0xeb,0xca,0x58,0x62,0x66,0x84,0xfe,0xfe,0xec,
0x87,0x55,0x93,0xa5,0x6d,0x63,0x43,0x91,0xe6,0xe8,0xef,0xf2,0xf3,0xfe,0xfe,0xda,
0xf9,0xf4,0xfb,0xe1,0xe3,0xfe,0xe8,0xf7,0xe6,0xfe,0xe4,0xfb,0xff,0xe8,0xff,0xf8,
0xee,0xfe,0x98,0x03,0x73,0xfe,0xfa,0xfe,0xfe,0xf2,0xee,0xf2,0xfc,0xf3,0xf0,0xf4,
0xfe,0xfe,0xea,0xf6,0xef,0xfe,0xfa,0xf9,0xf2,0xfe,0xe9,0xfe,0xfd,0xfa,0xf8,0xf7,
0xfe,0xfa,0xf5,0xfe,0xf9,0xf3,0xfb,0xee,0xed,0xfe,0xfd,0xfe,0xf9,0xe4,0xf1,0xf2,
0xfc,0xff,0xf9,0xf7,0xfc,0xe5,0xff,0xfb,0xfe,0xfb,0xfb,0xf5,0xf6,0xe6,0xed,0xfe,
0xf4,0xfe,0xee,0xfe,0xfe,0xf7,0xf0,0xf8,0xf1,0xf9,0xfc,0xe9,0xff,0xff,0xe1,0xff,
0xf1,0xfd,0xac,0x01,0x82,0xfe,0xbe,0xbe,0xe9,0xf1,0xe8,0xc9,0xca,0xfa,0xf2,0xd5,
0xa1,0xc0,0xd7,0xf4,0xc2,0xd0,0xba,0xd2,0xbe,0xe1,0xd6,0xb8,0xc9,0xb7,0xc8,0xe9,
0xe8,0xee,0xb8,0xbf,0xe3,0xff,0xd5,0xd8,0xd3,0xc3,0xc3,0xf0,0xff,0xea,0xec,0xf1,
0xbf,0xca,0xde,0xc4,0xc6,0xc6,0xc0,0xb9,0xe4,0xd3,0xc4,0xe2,0xf7,0xf1,0xe1,0xbd,
0xd8,0xcd,0xb3,0xc0,0xb7,0xc4,0xcc,0xea,0xfe,0xd0,0xac,0xb8,0xd2,0xe6,0xf6,0xfc,
0xfb,0xf8,0xbf,0x07,0x7a,0xf6,0x6a,0x02,0x8b,0xfd,0xfa,0x48,0x50,0xe7,0x7f,0x22,
0x54,0x2d,0x63,0x90,0x26,0x43,0x1a,0x10,0x35,0x3a,0xa8,0x08,0x22,0x5f,0x31,0x26,
0xb5,0xc2,0x04,0x54,0xeb,0xf8,0xdd,0x2f,0xd6,0x51,0x18,0x96,0xf6,0xf7,0xfd,0x9a,
0x09,0x89,0xf2,0x3a,0x0c,0x37,0x4d,0x25,0x85,0xc4,0x1f,0x1b,0xc9,0xfe,0xaf,0x26,
0xa2,0x41,0x44,0x32,0x12,0x46,0x3b,0xa4,0x95,0x25,0x4e,0x32,0x48,0xe6,0xff,0xfe,
0xf6,0xe9,0xb9,0x01,0x7a,0xfe,0x6b,0x01,0x08,0x99,0xee,0x7d,0xa2,0xcc,0x0e,0xb7,
0xfa,0xfe,0xb5,0xac,0xe5,0xdd,0x0b,0x6b,0xfe,0xc6,0xba,0x10,0x82,0xfe,0xe8,0x1c,
0x2b,0xfa,0x20,0x72,0xff,0xd6,0xff,0x6e,0xef,0x65,0x01,0x55,0xe7,0xfe,0xd5,0x31,
0x00,0xba,0xf5,0x73,0x12,0xe7,0xff,0xef,0xc5,0xe7,0x21,0x0a,0x43,0xd6,0xf9,0x33,
0xb5,0xd4,0xf7,0x4e,0x17,0xeb,0xcd,0xa7,0x0c,0x75,0xff,0xff,0xbc,0xce,0xfd,0xf9,
0xf9,0xf4,0xbf,0x01,0x8c,0xf5,0x6a,0x63,0x2d,0x09,0xbf,0x86,0x71,0xe3,0x03,0x26,
0x79,0xaa,0xe5,0xfe,0xf4,0xd9,0x10,0x64,0xfe,0xf2,0xd6,0x01,0x75,0xf1,0xc7,0x1b,
0x66,0xe2,0x1c,0x6a,0xf8,0xff,0xe8,0x6c,0xdf,0x77,0x1e,0x01,0xa9,0xff,0x7a,0x0e,
0x27,0xa2,0xff,0x59,0x0c,0xbb,0xb4,0xa7,0xf7,0xe1,0x3e,0x5f,0x01,0x4d,0xd9,0x3e,
0xdd,0xf5,0xf7,0x59,0x18,0xf1,0xfe,0xe0,0x10,0x1e,0x56,0xa3,0xf3,0xff,0xed,0xff,
0xfa,0xff,0xa9,0x01,0x72,0xff,0x68,0x87,0xcb,0x15,0x22,0x42,0x82,0xfe,0x89,0x22,
0x01,0x04,0x32,0xcc,0xff,0xca,0x13,0x60,0xff,0xee,0xd4,0x0e,0x23,0x4d,0x01,0x74,
0xff,0xec,0x19,0x69,0xfb,0xed,0xf1,0x65,0xf0,0x4e,0x7f,0x29,0x43,0xce,0x3d,0x7c,
0x0f,0xb2,0xfb,0x63,0x00,0x4d,0x4b,0x37,0xea,0xe1,0x33,0xf3,0x74,0x01,0x53,0x3a,
0xd6,0xe7,0xfd,0x59,0x18,0xf0,0xf7,0xf4,0xa4,0x29,0x04,0x01,0x2e,0xc6,0xf3,0xff,
0xee,0xf5,0xb7,0x01,0x92,0xff,0x60,0x8f,0xfe,0xbb,0x1d,0x01,0x87,0xfa,0xf6,0xf2,
0xc0,0x7b,0x01,0x74,0xff,0xe6,0x07,0x66,0xff,0xf0,0xc9,0x0e,0x79,0xd2,0x52,0x28,
0xce,0xeb,0x0d,0x78,0xff,0xea,0xff,0x6c,0xd7,0x85,0x9c,0x7b,0x04,0x21,0x6e,0xbc,
0x06,0xba,0xff,0x5e,0x11,0xdf,0xf4,0xec,0xeb,0xe8,0x31,0xe0,0xe8,0x77,0x01,0x2b,
0xea,0xf3,0xec,0x4b,0x16,0xe7,0xf5,0xe1,0xff,0xec,0xbd,0x82,0x00,0x5e,0xff,0xf3,
0xee,0xfd,0xbe,0x01,0x74,0xfe,0x5f,0x82,0xfa,0xf1,0xad,0x03,0x94,0xac,0xa2,0xeb,
0xfe,0xbc,0x1a,0x6f,0xef,0xdd,0x0a,0x6b,0xfa,0xfe,0xce,0x01,0x85,0xfe,0xb1,0x05,
0x48,0xf4,0x40,0x33,0xe3,0xf7,0xc6,0x2e,0xed,0x6e,0x94,0xef,0x20,0x10,0xe2,0xb0,
0x01,0xab,0xff,0x5c,0x18,0xdc,0xf7,0xf4,0xcc,0xe1,0x43,0xd9,0xff,0xee,0x43,0x11,
0xcc,0xfe,0xfc,0x59,0x2b,0xf3,0xff,0xb6,0x7f,0xe5,0xff,0xed,0x0d,0x52,0xff,0xf8,
0xf6,0xf0,0x77,0x0a,0x61,0xfd,0x5f,0x55,0xfd,0xef,0xf9,0x8c,0x96,0xca,0x20,0x48,
0x57,0x36,0x47,0xe5,0xfe,0xae,0x09,0x53,0xe8,0xf5,0xaf,0x01,0x51,0xe9,0xf0,0x8e,
0x0a,0x95,0xb2,0x2c,0x44,0x62,0x25,0x99,0xe8,0x40,0x76,0xf9,0x77,0x7c,0xfc,0x78,
0x17,0x83,0xdf,0x43,0x11,0x3c,0x46,0x31,0x50,0xad,0x15,0xbf,0xe8,0xf0,0xff,0x5d,
0xd6,0xfe,0xd2,0x39,0x0e,0xbb,0xfb,0xd7,0x42,0x3f,0x40,0x4e,0x4d,0xc9,0xff,0xff,
0xf5,0xfd,0xca,0xcc,0xbc,0xea,0xb4,0xd5,0xf5,0xec,0xf4,0xfa,0xe0,0xfd,0xd3,0xcf,
0xc4,0xda,0xfb,0xf8,0xe9,0xf0,0xc2,0xb3,0xe6,0xeb,0xe2,0xcc,0xd1,0xf0,0xfb,0xf4,
0xcf,0xde,0xdd,0xeb,0xbd,0x9c,0xdc,0xfa,0xf2,0xb9,0xcb,0xf5,0xe7,0xe6,0xf6,0xce,
0xae,0xc9,0xf0,0xbe,0xcc,0xbb,0xd0,0xb8,0xd0,0xdb,0xc0,0xdc,0xf4,0xff,0xe9,0xdf,
0xec,0xf3,0xee,0xd1,0xb5,0xe0,0xf5,0xf5,0xe3,0xc9,0xd3,0xc4,0xe6,0xff,0xeb,0xff,
0xfe,0xf4,0xe9,0xff,0xfe,0xf7,0xfe,0xf2,0xf3,0xf2,0xfe,0xee,0xf4,0xe0,0xff,0xff,
0xfe,0xf1,0xfa,0xef,0xfd,0xf5,0xf8,0xfe,0xfe,0xfb,0xed,0xfe,0xee,0xf2,0xf5,0xfb,
0xff,0xea,0xff,0xff,0xee,0xff,0xff,0xe7,0xf2,0xff,0xfe,0xf0,0xff,0xf6,0xfb,0xfc,
0xff,0xfe,0xf9,0xff,0xff,0xfa,0xff,0xfd,0xff,0xff,0xf7,0xfa,0xff,0xed,0xf6,0xff,
0xff,0xf0,0xf5,0xff,0xff,0xfd,0xf5,0xfb,0xff,0xfc,0xff,0xf5,0xff,0xf8,0xff,0xfc,
0xf5,0xfe,0xff,0xfe,0xf6,0xee,0xfe,0xf4,0xfe,0xfe,0xf8,0xfe,0xfd,0xfd,0xf7,0xfa,
0xff,0xfe,0xfe,0xfa,0xfe,0xfb,0xf5,0xfe,0xf5,0xfb,0xfe,0xfe,0xfe,0xfe,0xfe,0xf4,
0xff,0xff,0xf2,0xf5,0xfc,0xf8,0xfa,0xf8,0xff,0xfa,0xff,0xfb,0xff,0xfb,0xff,0xf5,
0xf2,0xfe,0xf3,0xfe,0xf6,0xf3,0xfb,0xfe,0xff,0xf1,0xff,0xfd,0xf5,0xfd,0xff,0xfc,
0xff,0xf9,0xff,0xf9,0xfd,0xff,0xfc,0xf7,0xf1,0xfa,0xf9,0xff,0xff,0xf6,0xff,0xf4,
0xfe,0xfe,0xf4,0xfe,0xfc,0xfe,0xfb,0xf5,0xf9,0xff,0xfe,0xfe,0xfd,0xf1,0xfc,0xf8,
0xfe,0xf2,0xf9,0xfe,0xf5,0xfe,0xfe,0xfc,0xfe,0xf8,0xfe,0xfe,0xfd,0xee,0xfe,0xfe,
0xf5,0xfa,0xff,0xff,0xfc,0xf9,0xff,0xfc,0xff,0xf2,0xff,0xf8,0xff,0xf9,0xfa,0xff,
0xff,0xf5,0xfe,0xf6,0xff,0xfd,0xff,0xfc,0xff,0xf8,0xf8,0xfe,0xff,0xff,0xfb,0xff,
0xff,0xf5,0xff,0xfc,0xfc,0xfd,0xf6,0xff,0xfe,0xff,0xf8,0xfd,0xff,0xff,0xf5,0xff
};
 // 2560 bytes

char TILogo_UV_80_32[] = {
0x85,0x7d,0x83,0x7b,0x81,0x80,0x7e,0x8a,0x7f,0x8b,0x81,0x82,0x82,0x7d,0x83,0x7d,
0x82,0x7b,0x86,0x7b,0x85,0x83,0x82,0x88,0x7c,0x81,0x7c,0x73,0x81,0x76,0x85,0x85,
0x84,0x7f,0x84,0x7d,0x84,0x7d,0x84,0x7d,0x84,0x7d,0x84,0x7d,0x84,0x7d,0x84,0x7d,
0x82,0x7d,0x82,0x7d,0x83,0x7e,0x83,0x7e,0x84,0x7d,0x85,0x7d,0x85,0x7d,0x85,0x7d,
0x84,0x7d,0x85,0x7d,0x86,0x7e,0x86,0x7e,0x84,0x7e,0x84,0x7d,0x82,0x7d,0x80,0x7e,
0x86,0x82,0x86,0x74,0x7f,0x81,0x78,0xa9,0x76,0xb8,0x78,0x9d,0x7a,0x8a,0x7d,0x88,
0x80,0x86,0x84,0x7c,0x84,0x7a,0x86,0x81,0x84,0x81,0x82,0x79,0x86,0x7a,0x87,0x84,
0x85,0x7e,0x85,0x7d,0x85,0x7d,0x85,0x7d,0x85,0x7d,0x85,0x7d,0x85,0x7d,0x85,0x7d,
0x83,0x7e,0x84,0x7d,0x84,0x7d,0x85,0x7d,0x85,0x7d,0x86,0x7e,0x86,0x7e,0x86,0x7e,
0x84,0x7d,0x85,0x7d,0x86,0x7e,0x86,0x7e,0x86,0x7e,0x84,0x7d,0x82,0x7d,0x80,0x7e,
0x83,0x83,0x84,0x77,0x7b,0x83,0x71,0xab,0x6c,0xc1,0x6d,0xb3,0x72,0x9f,0x76,0x98,
0x77,0x9d,0x7f,0x86,0x85,0x76,0x88,0x7c,0x87,0x83,0x87,0x7f,0x87,0x7b,0x87,0x7f,
0x86,0x7e,0x86,0x7e,0x86,0x7e,0x86,0x7e,0x86,0x7e,0x86,0x7e,0x86,0x7e,0x86,0x7e,
0x85,0x7d,0x85,0x7d,0x86,0x7e,0x86,0x7e,0x86,0x7d,0x86,0x7d,0x86,0x7d,0x86,0x7d,
0x84,0x7d,0x85,0x7d,0x86,0x7e,0x86,0x7d,0x86,0x7d,0x84,0x7d,0x83,0x7e,0x81,0x7e,
0x83,0x82,0x80,0x85,0x77,0x87,0x6a,0xa2,0x67,0xaf,0x6e,0xa6,0x71,0xaa,0x78,0xa1,
0x6e,0xb7,0x78,0x91,0x84,0x7a,0x88,0x7c,0x86,0x86,0x86,0x83,0x84,0x7b,0x85,0x79,
0x86,0x7d,0x86,0x7d,0x86,0x7d,0x86,0x7d,0x86,0x7d,0x86,0x7d,0x86,0x7d,0x86,0x7d,
0x86,0x7e,0x86,0x7e,0x86,0x7d,0x86,0x7d,0x87,0x7d,0x87,0x7d,0x87,0x7e,0x87,0x7e,
0x85,0x7d,0x86,0x7e,0x86,0x7d,0x87,0x7d,0x86,0x7e,0x85,0x7d,0x83,0x7e,0x81,0x7e,
0x7e,0x86,0x77,0x94,0x6f,0xaa,0x6a,0xb8,0x69,0xb4,0x6d,0xaa,0x75,0xaa,0x77,0xb6,
0x6c,0xc6,0x73,0xa2,0x7f,0x81,0x87,0x80,0x86,0x88,0x81,0x84,0x7f,0x7a,0x80,0x76,
0x83,0x7d,0x85,0x7d,0x85,0x7d,0x83,0x7e,0x83,0x7e,0x85,0x7d,0x85,0x7d,0x85,0x7e,
0x84,0x7e,0x86,0x7e,0x85,0x7e,0x85,0x7e,0x87,0x7d,0x85,0x7e,0x87,0x7e,0x86,0x7e,
0x86,0x7d,0x86,0x7d,0x86,0x7d,0x86,0x7d,0x86,0x7d,0x85,0x7d,0x84,0x7d,0x81,0x7e,
0x7b,0x87,0x7b,0x86,0x74,0xaf,0x70,0xcd,0x70,0xc5,0x72,0xa7,0x75,0xa4,0x77,0xbb,
0x70,0xbc,0x77,0x9b,0x81,0x81,0x84,0x80,0x82,0x83,0x7f,0x82,0x7d,0x7a,0x7e,0x7a,
0x82,0x7d,0x83,0x7e,0x83,0x7e,0x83,0x7e,0x82,0x7e,0x83,0x7e,0x82,0x7e,0x84,0x7d,
0x83,0x7e,0x82,0x7e,0x85,0x7d,0x86,0x7e,0x86,0x7d,0x85,0x7e,0x85,0x7e,0x87,0x7d,
0x86,0x7d,0x86,0x7d,0x86,0x7d,0x87,0x7e,0x87,0x7d,0x86,0x7d,0x84,0x7d,0x81,0x7e,
0x81,0x80,0x7f,0x81,0x7e,0x91,0x7d,0xa2,0x79,0xa8,0x76,0x97,0x78,0x8d,0x71,0xb0,
0x7b,0xa2,0x82,0x82,0x84,0x79,0x86,0x7c,0x83,0x82,0x7f,0x7f,0x80,0x7d,0x81,0x80,
0x81,0x7e,0x81,0x7e,0x80,0x7e,0x82,0x7d,0x82,0x7d,0x81,0x7e,0x82,0x7d,0x81,0x7e,
0x81,0x7e,0x83,0x7e,0x84,0x7d,0x82,0x7e,0x82,0x7e,0x86,0x7e,0x83,0x7e,0x84,0x7e,
0x86,0x7d,0x86,0x7d,0x87,0x7e,0x87,0x7e,0x87,0x7e,0x86,0x7d,0x85,0x7d,0x82,0x7e,
0x84,0x77,0x84,0x77,0x84,0x7b,0x81,0x83,0x7b,0x8e,0x75,0x9a,0x6f,0xa2,0x73,0x98,
0x82,0x8a,0x85,0x7a,0x88,0x74,0x87,0x79,0x82,0x7e,0x82,0x7d,0x81,0x7e,0x84,0x83,
0x82,0x7e,0x82,0x7d,0x82,0x7d,0x82,0x7d,0x82,0x7d,0x81,0x7e,0x82,0x7d,0x82,0x7d,
0x82,0x7d,0x81,0x7e,0x83,0x7e,0x82,0x7e,0x84,0x7d,0x82,0x7e,0x85,0x7d,0x82,0x7e,
0x86,0x7d,0x86,0x7d,0x86,0x7d,0x86,0x7d,0x87,0x7d,0x86,0x7d,0x84,0x7d,0x82,0x7e,
0x84,0x7b,0x83,0x7c,0x83,0x7f,0x81,0x83,0x80,0x86,0x7c,0x92,0x7a,0x9a,0x7d,0x93,
0x83,0x81,0x84,0x7d,0x85,0x7d,0x85,0x7d,0x82,0x7d,0x85,0x7d,0x85,0x7d,0x85,0x7d,
0x84,0x7f,0x84,0x7f,0x84,0x7f,0x84,0x7f,0x84,0x7f,0x84,0x7f,0x82,0x7f,0x83,0x7f,
0x82,0x7f,0x82,0x7f,0x82,0x7f,0x84,0x7f,0x82,0x7f,0x84,0x7f,0x85,0x7f,0x82,0x7f,
0x84,0x7f,0x83,0x7f,0x83,0x7f,0x83,0x7f,0x83,0x7f,0x84,0x7f,0x84,0x7f,0x81,0x7f,
0x83,0x80,0x85,0x7f,0x82,0x7f,0x81,0x7f,0x80,0x81,0x7f,0x84,0x7f,0x85,0x7e,0x88,
0x80,0x7f,0x85,0x7d,0x82,0x7e,0x83,0x7e,0x82,0x7e,0x82,0x7e,0x82,0x7e,0x84,0x7d,
0x81,0x7f,0x82,0x7f,0x83,0x7f,0x83,0x7f,0x82,0x7f,0x80,0x7f,0x83,0x7f,0x83,0x7f,
0x80,0x7f,0x82,0x7f,0x82,0x7f,0x81,0x7f,0x81,0x7f,0x83,0x7f,0x85,0x7f,0x82,0x7f,
0x83,0x7f,0x83,0x7f,0x82,0x7f,0x84,0x7f,0x83,0x7f,0x82,0x7f,0x80,0x7f,0x81,0x7f,
0x83,0x82,0x84,0x80,0x83,0x7e,0x82,0x7d,0x82,0x7b,0x82,0x79,0x82,0x78,0x82,0x7a,
0x83,0x7d,0x84,0x7d,0x84,0x7d,0x84,0x7d,0x84,0x7d,0x84,0x7d,0x84,0x7d,0x84,0x7d,
0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x81,0x7f,
0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x84,0x7f,0x82,0x7f,0x85,0x7f,
0x86,0x7f,0x86,0x7f,0x85,0x7f,0x84,0x7f,0x84,0x7f,0x82,0x7f,0x82,0x7f,0x80,0x7f,
0x82,0x80,0x82,0x7f,0x84,0x7e,0x84,0x7c,0x84,0x7b,0x84,0x7a,0x85,0x78,0x84,0x7b,
0x83,0x7e,0x82,0x7f,0x84,0x7f,0x84,0x7f,0x82,0x7f,0x83,0x7f,0x84,0x7f,0x83,0x7f,
0x82,0x7f,0x82,0x7f,0x81,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x81,0x7f,0x82,0x7f,
0x81,0x7f,0x80,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x83,0x7f,0x84,0x7f,
0x85,0x7f,0x84,0x7f,0x84,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x81,0x7f,0x81,0x7f,
0x82,0x7d,0x82,0x7e,0x81,0x7f,0x83,0x7f,0x82,0x80,0x84,0x80,0x83,0x80,0x84,0x80,
0x83,0x7f,0x82,0x7f,0x81,0x7f,0x82,0x7f,0x81,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,
0x82,0x7f,0x82,0x7f,0x81,0x7f,0x81,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x81,0x7f,
0x7f,0x7f,0x80,0x7f,0x81,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,
0x84,0x7f,0x84,0x7f,0x84,0x7f,0x82,0x7f,0x81,0x7f,0x82,0x7f,0x82,0x7f,0x81,0x7f,
0x82,0x7b,0x82,0x7c,0x81,0x7f,0x82,0x82,0x82,0x83,0x83,0x84,0x82,0x85,0x82,0x84,
0x82,0x80,0x82,0x80,0x81,0x80,0x82,0x80,0x82,0x80,0x82,0x80,0x82,0x80,0x82,0x80,
0x81,0x7f,0x81,0x7f,0x81,0x7f,0x81,0x7f,0x81,0x7f,0x81,0x7f,0x81,0x7f,0x81,0x7f,
0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x81,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x81,0x7f,
0x82,0x7f,0x84,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x82,0x7f,0x81,0x7f,0x80,0x7f,
0x81,0x7d,0x81,0x7e,0x81,0x80,0x80,0x81,0x80,0x82,0x80,0x80,0x80,0x80,0x80,0x7f,
0x81,0x80,0x82,0x80,0x81,0x80,0x81,0x80,0x81,0x80,0x81,0x80,0x82,0x80,0x82,0x80,
0x80,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,
0x7f,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x82,0x7f,0x81,0x7f,0x81,0x7f,
0x82,0x7f,0x81,0x7f,0x80,0x7f,0x82,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,
0x80,0x7e,0x81,0x80,0x80,0x80,0x81,0x82,0x80,0x80,0x7f,0x7e,0x7f,0x7b,0x7f,0x7a,
0x81,0x7f,0x81,0x80,0x81,0x80,0x81,0x80,0x81,0x80,0x80,0x80,0x81,0x80,0x80,0x80,
0x81,0x7f,0x80,0x7f,0x81,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,
0x7f,0x7f,0x7f,0x7f,0x80,0x7f,0x80,0x7f,0x80,0x7f,0x81,0x7f,0x80,0x7f,0x81,0x7f,
0x81,0x7f,0x80,0x7f,0x81,0x7f,0x81,0x7f,0x80,0x7f,0x81,0x7f,0x80,0x7f,0x80,0x7f
};
// 1280 bytes
