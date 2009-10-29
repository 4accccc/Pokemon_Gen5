

//------------------------------------------------------------------
//------------------------------------------------------------------
//NitroSDK/include/nitro/types.hよりコピペ
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
	MMLID_NOENTRY	=	0,
	MMLID_SAMPLEOBJ	=	1,

	//MAPTYPE_NOWHERE	=	0xff,
	//MAPNAME_NOTHING	=	0x33,
	WEATHER_SYS_SUNNY	=	0x11,
	PLACE_WIN_SEE	=	0x80,
	PLACE_WIN_CITY	=	0x81,

	MATRIX_ARC_MAPMATRIX = 0,
	MATRIX_ARC_NGMATRIX = 1,

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
	RSC_GRID_DEFAULT = 0,
	RSC_NOGRID_BRIDGE = 1,
	RSC_NOGRID_C03 = 2,
  RSC_GRID_PALACE = 3,
  RSC_GRID_RANDOM_MAP = 4,
  RSC_NOGRID_LEAGUE = 5,
  RSC_GRID_COMM = 6,
  RSC_NOGRID_DEFAULT = 7,
  RSC_NOGRID_C03P02 = 8,
  RSC_GRID_MUSICAL = 9,
  RSC_GRID_GYM_ELEC = 10,
  RSC_GRID_WIDE = 11,     // 3x2
  RSC_NOGRID_D09 = 12,     // 3x3
  RSC_GRID_BRIDGE_H03 = 13,     // 1x6
  RSC_HYBRID = 14,     // 

	//NO_GRID_SAMPLE = 2,
	//GRID_GS_SAMPLE = 3,
	RSC_TYPE_END
};
#include "../area_data/area_id.h"
#include "..\..\sound\wb_sound_data.sadl"
//#include "../map_matrix/map_matrix.naix"
#include "tmp/map_matrix.h"
#include "../script/script_seq.naix"      //script ID 定義ファイル
#include "../../message/script_message.naix"     //message archive ID定義ファイル
#include "..\..\message\dst\msg_place_name.h"   //地名表示用文字列ID定義ファイル
#include "header/maptype.h"
#include "../../encount/encount_data_w.naix"  //エンカウントデータID定義ファイル
#include "../eventdata/binary/eventdata.naix"      //eventdata ID 定義ファイル
#include "../../../prog/include/battle/battle_bg_def.h"  //戦闘背景ID定義

enum {
	//event_dummy = NARC_zone_event_zone_dummy_total_bin,
	//event_dummy = 0xd000,
	event_dummy = NARC_eventdata_dummy_data,

	enc_dummy = 0xffff
};

//pokemon_wb/prog/include/field/weather_no.hから暫定こぴー。
//実際にはちゃんと参照できるようにすること！！
#define		WEATHER_NO_SUNNY			(0)				// 晴れ
#define		WEATHER_NO_SNOW				(1)				// 雪		
#define		WEATHER_NO_RAIN				(2)				// 雨		
#define		WEATHER_NO_STORM			(3)				// 砂嵐		
#define		WEATHER_NO_SPARK			(4)				// 雷雨
#define		WEATHER_NO_SNOWSTORM		(5)				// 吹雪
#define		WEATHER_NO_ARARE			(6)				// 吹雪

#define		WEATHER_NO_MIRAGE			(7)				// 蜃気楼

