

//------------------------------------------------------------------
//------------------------------------------------------------------
//NitroSDK/include/nitro/types.h‚æ‚èƒRƒsƒy
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long u32;
typedef signed long s32;

#ifndef	TRUE
#define	TRUE	1
#endif
#ifndef	FALSE
#define	FALSE	(!TRUE)
#endif

//------------------------------------------------------------------
//------------------------------------------------------------------

enum {
	//sp_scr_dummy = NARC_scr_seq_sp_dummy_bin,
	sp_scr_dummy = 0xf000,
	//scr_dummy = NARC_scr_seq_dummy_bin,
	scr_dummy = 0xe000,
	//event_dummy = NARC_zone_event_zone_dummy_total_bin,
	event_dummy = 0xd000,
	//sp_scr_dummy = NARC_scr_seq_sp_c01_bin,
	//scr_dummy = NARC_scr_seq_c01_bin,
	//msg_dummy = NARC_msg_c01_dat,
	msg_dummy = 0xcccc,
	enc_dummy = 0xffff
};

enum {
	BG_ID_FOREST	=	3,

	MMLID_NOENTRY	=	0,
	MMLID_SAMPLEOBJ	=	1,

	MAPTYPE_NOWHERE	=	0xff,
	MAPNAME_NOTHING	=	0x33,
	WEATHER_SYS_SUNNY	=	0x11,
	PLACE_WIN_SEE	=	0x80,
	PLACE_WIN_CITY	=	0x81,

	MATRIX_ID_SINOU	=	1,
	MATRIX_ID_TEST0 =	0,
	MATRIX_ID_TEST1 =	1,
	MATRIX_ID_TEST2 =	2,
	MATRIX_ID_TEST3 =	3,
	MATRIX_ID_TEST4 =	4,
	MATRIX_ID_TEST5 =	5,
	MATRIX_ID_TEST6 =	6,
	MATRIX_ID_TEST7 =	7,
	MATRIX_ID_TEST8 =	8,
	MATRIX_ID_TEST9 =	9,
	MATRIX_ID_TEST10 =	10,

	DUMMY_DEFINE_END
};

enum {
	NO_GRID_SAMPLE = 0,
	GRID_DEFAULT = 1,
	//NO_GRID_SAMPLE = 2,
	GRID_GS_SAMPLE = 3
};
#include "../area_data/area_id.h"

