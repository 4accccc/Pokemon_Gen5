

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
  RSC_GRID_FOURKINGS = 15,     // 四天王部屋用
  RSC_GRID_NOSCROLL = 16,     // グリッド　スクロールなし動作
  RSC_GRID_CABIN = 17,     // グリッド　客室用　1x5
  RSC_GRID_H04 = 18,     // グリッド　H04 スクロールなし動作
  RSC_GRID_DEF_TAILDRAW = 19,
  RSC_GRID_LEAGIN02 = 20, //グリッド　リーグフロント２専用

	//NO_GRID_SAMPLE = 2,
	//GRID_GS_SAMPLE = 3,
	RSC_TYPE_END
};
#include "zone_id.h"
#include "../area_data/area_id.h"
#include "..\..\sound\wb_sound_data.sadl"
//#include "../map_matrix/map_matrix.naix"
#include "tmp/map_matrix.h"
#include "../script/script_seq.naix"      //script ID 定義ファイル
#include "../../message/script_message.naix"     //message archive ID定義ファイル
#include "..\..\message\dst\msg_place_name.h"   //地名表示用文字列ID定義ファイル
#include "../../encount/encount_data_w.naix"  //エンカウントデータID定義ファイル
#include "../eventdata/binary/eventdata.naix"      //eventdata ID 定義ファイル
#include "../../../prog/include/battle/battle_bg_def.h"  //戦闘背景ID定義
#include "../camera_scroll/camera_scroll.naix"//カメラエリアデータ
#include "../../../prog/src/field/map_change_type.h" //マップ切り替えタイプ定義ファイル
#include "../mm_list/mmlist_def.h" //動作モデル登録定義ファイル
#include "../../../prog/include/field/weather_no.h" // 天気

enum {
	enc_dummy = 0xffff,

	camera_area_dummy = 0xffff,
};

