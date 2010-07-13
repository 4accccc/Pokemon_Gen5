//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_btn_data.c
 *	@brief	ボタン情報
 *	@author	Toru=Nagihashi
 *	@data		2009.11.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//アーカイブ
#include "msg/msg_battle_rec.h"

//自分のモジュール
#include "br_core.h"
#include "br_bvrank_proc.h"
#include "br_bvdelete_proc.h"
#include "br_record_proc.h"

//外部公開
#include "br_btn_data.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ボタンの座標
//=====================================
#define BR_BTN_POS_START_Y		(25)	//開始オフセット
#define BR_BTN_POS_OFFSET_Y		(36)	//間隔オフセット
#define BR_BTN_POS_NUMDIF_Y		(16)	//個数での位置調整オフセット
//座標取得マクロ
#define BR_BTN_DATA_GET_Y( n,m )	(BR_BTN_POS_START_Y+BR_BTN_POS_OFFSET_Y*n+BR_BTN_POS_NUMDIF_Y*(5-m))

#define BR_BTN_LINK_END				(0xFFFFFFFF)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ボタン繋がり情報
//=====================================
typedef struct 
{
	BR_MENUID	menuID;
	u32				btnID;
} BR_BTN_LINK;

//-------------------------------------
///	ボタン情報
//=====================================
struct _BR_BTN_DATA
{	
	u16			param[BR_BTN_DATA_PARAM_MAX];
};

//-------------------------------------
///	ボタン情報
//=====================================
struct _BR_BTN_DATA_SYS
{
	BR_BTN_DATA_SETUP   setup;
	BR_BTN_DATA					buff[0];
};

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	ブラウザモード	初期メニュー
//=====================================
#define BTN_DATA_BROWSE_TOP_MAX	(4)
static const BR_BTN_DATA sc_btn_data_browse_top[BTN_DATA_BROWSE_TOP_MAX] =
{	
	//バトルビデオ
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_TOP_MAX),
		BR_BROWSE_MENUID_TOP,
		msg_01,
		7,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_BTLVIDEO,
		0,
		0,
		0,
		0,
		0,
	},
	//バトルサブウェイ
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_TOP_MAX),
		BR_BROWSE_MENUID_TOP,
		msg_02,
		3,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BTLSUBWAY,
		0,
		0,
		0,
		0,
    0,

	},
	//ランダムマッチ
	{
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_TOP_MAX),
		BR_BROWSE_MENUID_TOP,
		msg_03,
		9,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_RNDMATCH,
		0,
		0,
		0,
		0,
    0,

	},
	//やめる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BROWSE_TOP_MAX),
		BR_BROWSE_MENUID_TOP,
		msg_04,
		0,
		BR_BTN_TYPE_EXIT,
		0,
		0,
		0,
		0,
		0,
    0,
	},
};
static const BR_BTN_LINK sc_btn_link_browse_top[] = 
{	
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	}
};

//-------------------------------------
///	ブラウザモード	バトルビデオメニュー
//=====================================
#define BTN_DATA_BROWSE_BTLVIDEO_MAX	(4)
static const BR_BTN_DATA sc_btn_data_browse_btlvideo[BTN_DATA_BROWSE_BTLVIDEO_MAX] =
{	
	//自分の記録
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_BTLVIDEO_MAX),
		BR_BROWSE_MENUID_BTLVIDEO,
		msg_06,
		10,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_RECORD,
		BR_RECODE_PROC_MY,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
    BR_BTN_UNIQUE_MYRECORD,
	},
	//誰かの記録
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_BTLVIDEO_MAX),
		BR_BROWSE_MENUID_BTLVIDEO,
		msg_07,
		11,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_OTHER_RECORD,
		0,
		0,
		0,
		0,
    0,
	},
	//記録を消す
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_BTLVIDEO_MAX),
		BR_BROWSE_MENUID_BTLVIDEO,
		msg_08,
		12,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_DELETE_RECORD,
		0,
		0,
		0,
		0,
    0,
	},
	//もどる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BROWSE_BTLVIDEO_MAX),
		BR_BROWSE_MENUID_BTLVIDEO,
		msg_05,
		1,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_TOP,
		0,
		0,
		0,
		0,
    0,
	},
};
static const BR_BTN_LINK sc_btn_link_browse_btlvideo[] = 
{	
	{	
		BR_BROWSE_MENUID_TOP,
		0,
	},
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	},
};

//-------------------------------------
///	ブラウザモード	誰かの記録メニュー
//=====================================
#define BTN_DATA_BROWSE_OTHER_RECORD_MAX	(4)
static const BR_BTN_DATA sc_btn_data_browse_other_record[BTN_DATA_BROWSE_OTHER_RECORD_MAX] =
{	
	//誰かの記録１
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_10,
		11,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_RECORD,
		BR_RECODE_PROC_OTHER_00,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
    BR_BTN_UNIQUE_OTHERRECORD1,
	},
	//誰かの記録２
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_10,
		11,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_RECORD,
		BR_RECODE_PROC_OTHER_01,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
    BR_BTN_UNIQUE_OTHERRECORD2,
	},
	//誰かの記録３
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_10,
		11,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_RECORD,
		BR_RECODE_PROC_OTHER_02,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
    BR_BTN_UNIQUE_OTHERRECORD3,
	},
	//戻る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_05,
		1,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_BTLVIDEO,
		0,
		0,
		0,
		0,
    0,
	},
};
static const BR_BTN_LINK sc_btn_link_browse_other_record[] = 
{	
	{	
		BR_BROWSE_MENUID_TOP,
		0,
	},
	{	
		BR_BROWSE_MENUID_BTLVIDEO,
		1,
	},
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	},
};
//-------------------------------------
///	ブラウザモード	記録を消すメニュー
//=====================================
#define BTN_DATA_BROWSE_DELETE_RECORD_MAX	(3)
static const BR_BTN_DATA sc_btn_data_browse_delete_record[BTN_DATA_BROWSE_DELETE_RECORD_MAX] =
{	
	//自分の記録
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_DELETE_RECORD_MAX),
		BR_BROWSE_MENUID_DELETE_RECORD,
		msg_100,
		10,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_DELETE,
		BR_BVDELETE_MODE_MY,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
    BR_BTN_UNIQUE_MYRECORD,
	},
	//誰かの記録
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_DELETE_RECORD_MAX),
		BR_BROWSE_MENUID_DELETE_RECORD,
		msg_101,
		11,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_DELETE_OTHER,
		0,
		0,
		0,
		0,
    0,
	},
	//もどる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_DELETE_RECORD_MAX),
		BR_BROWSE_MENUID_DELETE_RECORD,
		msg_05,
		1,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_BTLVIDEO,
		0,
		0,
		0,
		0,
    0,
	},
};
static const BR_BTN_LINK sc_btn_link_browse_delete_record[] = 
{	
	{	
		BR_BROWSE_MENUID_TOP,
		0,
	},
	{	
		BR_BROWSE_MENUID_BTLVIDEO,
		2,
	},
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	},
};
//-------------------------------------
///	ブラウザモード	誰かの記録を消すメニュー
//=====================================
#define BTN_DATA_BROWSE_DELETE_OTHER_MAX	(4)
static const BR_BTN_DATA sc_btn_data_browse_delete_other[BTN_DATA_BROWSE_DELETE_OTHER_MAX] =
{	
	//誰かの記録１
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_10,
		11,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_DELETE,
		BR_BVDELETE_MODE_OTHER1,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
    BR_BTN_UNIQUE_OTHERRECORD1,
	},
	//誰かの記録２
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_10,
		11,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_DELETE,
		BR_BVDELETE_MODE_OTHER2,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
    BR_BTN_UNIQUE_OTHERRECORD2,
	},
	//誰かの記録３
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_10,
		11,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_DELETE,
		BR_BVDELETE_MODE_OTHER3,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
    BR_BTN_UNIQUE_OTHERRECORD3,
	},
	//戻る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_05,
		1,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_DELETE_RECORD,
		0,
		0,
		0,
		0,
    0,
	},
};
static const BR_BTN_LINK sc_btn_link_browse_delete_other[] = 
{	
	{	
		BR_BROWSE_MENUID_TOP,
		0,
	},
	{	
		BR_BROWSE_MENUID_BTLVIDEO,
		2,
	},
	{	
		BR_BROWSE_MENUID_DELETE_RECORD,
		1,
	},
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	},
};


//-------------------------------------
///	グローバルバトルビデオモード	初期メニュー
//=====================================
#define BTN_DATA_BTLVIDEO_TOP_MAX	(3)
static const BR_BTN_DATA sc_btn_data_btlvideo_top[BTN_DATA_BTLVIDEO_TOP_MAX] =
{	
	//バトルビデオを見る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BTLVIDEO_TOP_MAX),
		BR_BTLVIDEO_MENUID_TOP,
		msg_700,
		7,
		BR_BTN_TYPE_SELECT,
		BR_BTLVIDEO_MENUID_LOOK,
		0,
		0,
		0,
		0,
    0,
	},
	//バトルビデオを送る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BTLVIDEO_TOP_MAX),
		BR_BTLVIDEO_MENUID_TOP,
		msg_701,
		8,
		BR_BTN_TYPE_SELECT_MSG,
		BR_MENUID_BVSEND_YESNO, //BR_PROCID_BV_SEND,
		msg_info_032,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_726,
    BR_BTN_UNIQUE_MYRECORD,
	},
	//やめる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BTLVIDEO_TOP_MAX),
		BR_BTLVIDEO_MENUID_TOP,
		msg_04,
		0,
		BR_BTN_TYPE_SELECT_MSG,
		BR_BTLVIDEO_MENUID_EXIT_YESNO,
		msg_10001,
		0,
		0,
		0,
    0,
	},
};
static const BR_BTN_LINK sc_btn_link_btlvideo_top[] = 
{	
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	}
};

//-------------------------------------
///	バトルビデオを送る？	はい、いいえ
//=====================================
#define BTN_DATA_YESNO_MAX	(2)
static const BR_BTN_DATA sc_btn_data_bvsend_yesno[BTN_DATA_YESNO_MAX] =
{	
	//はい
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_YESNO_MAX),
		BR_MENUID_BVSEND_YESNO,
		msg_1002,
		2,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_SEND,
		0,
		0,
		0,
		0,
    0,
	},
	//いいえ
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_YESNO_MAX),
		BR_MENUID_BVSEND_YESNO,
		msg_1003,
		1,
		BR_BTN_TYPE_RETURN,
		BR_BTLVIDEO_MENUID_TOP,
		0,
		0,
		0,
		0,
    0,
	},

};

static const BR_BTN_LINK sc_btn_link_bvsend_yesno[] = 
{	
  { 
    BR_BTLVIDEO_MENUID_TOP,
    1,
  },
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	}
};

//-------------------------------------
///	バトルビデオをやめる？	はい、いいえ
//=====================================
#define BTN_DATA_YESNO_MAX	(2)
static const BR_BTN_DATA sc_btn_data_bvexit_yesno[BTN_DATA_YESNO_MAX] =
{	
	//はい
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_YESNO_MAX),
		BR_BTLVIDEO_MENUID_EXIT_YESNO,
		msg_1002,
		2,
		BR_BTN_TYPE_EXIT,
		0,
		0,
		0,
		0,
		0,
    0,
	},
	//いいえ
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_YESNO_MAX),
		BR_BTLVIDEO_MENUID_EXIT_YESNO,
		msg_1003,
		1,
		BR_BTN_TYPE_RETURN,
		BR_BTLVIDEO_MENUID_TOP,
		0,
		0,
		0,
		0,
    0,
	},

};

static const BR_BTN_LINK sc_btn_link_bvexit_yesno[] = 
{	
  { 
    BR_BTLVIDEO_MENUID_TOP,
    2,
  },
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	}
};


//-------------------------------------
///	グローバルバトルビデオモード	見るメニュー
//=====================================
#define BTN_DATA_BTLVIDEO_LOOK_MAX	(4)
static const BR_BTN_DATA sc_btn_data_btlvideo_look[BTN_DATA_BTLVIDEO_LOOK_MAX] =
{	
	//ランキングで探す
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BTLVIDEO_LOOK_MAX),
		BR_BTLVIDEO_MENUID_LOOK,
		msg_200,
		13,
		BR_BTN_TYPE_SELECT,
		BR_BTLVIDEO_MENUID_RANK,
		0,
		0,
		0,
		0,
    0,
	},
	//詳しく探す
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BTLVIDEO_LOOK_MAX),
		BR_BTLVIDEO_MENUID_LOOK,
		msg_203,
		5,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_SEARCH,
		0,
		0,
		0,
		0,
    0,
	},
	//ナンバーで探す
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BTLVIDEO_LOOK_MAX),
		BR_BTLVIDEO_MENUID_LOOK,
		msg_204,
		6,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_CODEIN,
		0,
		0,
		0,
		0,
    0,
	},
	//もどる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BTLVIDEO_LOOK_MAX),
		BR_BTLVIDEO_MENUID_LOOK,
		msg_05,
		1,
		BR_BTN_TYPE_RETURN,
		BR_BTLVIDEO_MENUID_TOP,
		0,
		0,
		0,
		0,
    0,
	},
};
static const BR_BTN_LINK sc_btn_link_btlvideo_look[] = 
{	
	{	
		BR_BTLVIDEO_MENUID_TOP,
		0,
	},
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	}
};
//-------------------------------------
///	グローバルバトルビデオモード	ランキングで探すメニュー
//=====================================
#define BTN_DATA_BTLVIDEO_RANK_MAX	(4)
static const BR_BTN_DATA sc_btn_data_btlvideo_rank[BTN_DATA_BTLVIDEO_RANK_MAX] =
{	
	//最新30件
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BTLVIDEO_RANK_MAX),
		BR_BTLVIDEO_MENUID_RANK,
		msg_202,
		4,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_RANK,
		BR_BVRANK_MODE_NEW,
		0,
		0,
		0,
    0,
	},
	//通信対戦ランキング
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BTLVIDEO_RANK_MAX),
		BR_BTLVIDEO_MENUID_RANK,
		msg_208,
		14,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_RANK,
		BR_BVRANK_MODE_FAVORITE,
		0,
		0,
		0,
    0,
	},
	//サブウェイランキング
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BTLVIDEO_RANK_MAX),
		BR_BTLVIDEO_MENUID_RANK,
		msg_209,
		3,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_RANK,
		BR_BVRANK_MODE_SUBWAY,
		0,
		0,
		0,
    0,
	},
	//もどる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BTLVIDEO_RANK_MAX),
		BR_BTLVIDEO_MENUID_RANK,
		msg_05,
		1,
		BR_BTN_TYPE_RETURN,
		BR_BTLVIDEO_MENUID_LOOK,
		0,
		0,
		0,
		0,
    0,
	},
};
static const BR_BTN_LINK sc_btn_link_btlvideo_rank[] = 
{	
	{	
		BR_BTLVIDEO_MENUID_TOP,
		0,
	},
	{	
		BR_BTLVIDEO_MENUID_LOOK,
		0,
	},
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	},
};

//=============================================================================
/**
 *  ミュージカルショット  
 *    ミュージカルショットのみOBJが違うためアニメシーケンス番号が違う
 */
//=============================================================================
//-------------------------------------
///	グローバルバトルミュージカルショット	初期メニュー
//=====================================
#define BTN_DATA_MUSICAL_TOP_MAX	(3)
static const BR_BTN_DATA sc_btn_data_musical_top[BTN_DATA_MUSICAL_TOP_MAX] =
{	
	//写真を見る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_301,
		0,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_MUSICAL_LOOK,
		0,
		0,
		0,
		0,
    0,
	},
	//写真を送る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_300,
		1,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_MUSICAL_SEND,
		0,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_021,
    BR_BTN_UNIQUE_MUSICALSHOT,
	},
	//やめる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_04,
		2,
		BR_BTN_TYPE_SELECT_MSG,
		BR_MENUID_MUSICALEXIT_YESNO,
		msg_10001,
		0,
		0,
		0,
    0,
	},
};
static const BR_BTN_LINK sc_btn_link_musical_top[] = 
{	
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	}
};

//-------------------------------------
///	ミュージカルショットをやめる？	はい、いいえ
//=====================================
static const BR_BTN_DATA sc_btn_data_musicalexit_yesno[BTN_DATA_YESNO_MAX] =
{	
	//はい
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_YESNO_MAX),
		BR_MENUID_MUSICALEXIT_YESNO,
		msg_1002,
		3,
		BR_BTN_TYPE_EXIT,
		0,
		0,
		0,
		0,
		0,
    0,
	},
	//いいえ
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_YESNO_MAX),
		BR_MENUID_MUSICALEXIT_YESNO,
		msg_1003,
		4,
		BR_BTN_TYPE_RETURN,
		BR_MUSICAL_MENUID_TOP,
		0,
		0,
		0,
		0,
    0,
	},

};

static const BR_BTN_LINK sc_btn_link_musicalexit_yesno[] = 
{	
  { 
    BR_MUSICAL_MENUID_TOP,
    2,
  },
	{	
		BR_BTN_LINK_END,
		BR_BTN_LINK_END,
	}
};



//-------------------------------------
///	汎用	はい、いいえ
//=====================================
static const BR_BTN_DATA sc_btn_data_yesno[BTN_DATA_YESNO_MAX] =
{	
	//はい
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_301,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
    0,
	},
	//いいえ
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_301,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
    0,
	},

};

//-------------------------------------
///	メニューごとのテーブル
//=====================================
static const struct 
{	
	u16								max;			//ボタンの数
	const BR_BTN_DATA *cp_data;	//ボタン情報
	const BR_BTN_LINK	*cp_link;	//ボタン繋がり
} sc_btn_data_tbl[BR_MENUID_MAX]	=
{	
  //以下ブラウザ
	{	
		BTN_DATA_BROWSE_TOP_MAX,
		sc_btn_data_browse_top,
		sc_btn_link_browse_top,
	},
	{	
		BTN_DATA_BROWSE_BTLVIDEO_MAX,
		sc_btn_data_browse_btlvideo,
		sc_btn_link_browse_btlvideo,
	},
	{	
		BTN_DATA_BROWSE_OTHER_RECORD_MAX,
		sc_btn_data_browse_other_record,
		sc_btn_link_browse_other_record,
	},
	{	
		BTN_DATA_BROWSE_DELETE_RECORD_MAX,
		sc_btn_data_browse_delete_record,
		sc_btn_link_browse_delete_record,
	},
	{	
		BTN_DATA_BROWSE_DELETE_OTHER_MAX,
		sc_btn_data_browse_delete_other,
		sc_btn_link_browse_delete_other,
	},
  //以下バトルビデオ
	{	
		BTN_DATA_BTLVIDEO_TOP_MAX,
		sc_btn_data_btlvideo_top,
		sc_btn_link_btlvideo_top,
	},
	{	
		BTN_DATA_BTLVIDEO_LOOK_MAX,
		sc_btn_data_btlvideo_look,
		sc_btn_link_btlvideo_look,
	},
	{	
		BTN_DATA_BTLVIDEO_RANK_MAX,
		sc_btn_data_btlvideo_rank,
		sc_btn_link_btlvideo_rank,
	},
  { 
    BTN_DATA_YESNO_MAX,
    sc_btn_data_bvsend_yesno,
    sc_btn_link_bvsend_yesno,
  },
  { 
    BTN_DATA_YESNO_MAX,
    sc_btn_data_bvexit_yesno,
    sc_btn_link_bvexit_yesno,
  },
  //以下ミュージカル
	{	
		BTN_DATA_MUSICAL_TOP_MAX,
		sc_btn_data_musical_top,
		sc_btn_link_musical_top,
	},
  { 
    BTN_DATA_YESNO_MAX,
    sc_btn_data_musicalexit_yesno,
    sc_btn_link_musicalexit_yesno,
  },
};
//=============================================================================
/**
 *          PRIVATE
 */
//=============================================================================
static void BR_BTN_DATA_SYS_ReLoadSaveData( BR_BTN_DATA_SYS *p_wk );
//=============================================================================
/**
 *					PUBLIC
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンデータバッファ作成
 *
 *  @param	バトルレコーダーセーブデータ
 *	@param	HEAPID heapID	ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
BR_BTN_DATA_SYS * BR_BTN_DATA_SYS_Init( const BR_BTN_DATA_SETUP *cp_setup, HEAPID heapID )
{	
	BR_BTN_DATA_SYS	*p_wk;
	u32 size;

	//ヘッダつきバッファのサイズ計算
	{	
		u32 buff_size;
		int i;

		//バッファ数を数える
		buff_size	= 0;
		for( i = 0; i < BR_MENUID_MAX; i++ )
		{
			buff_size	+= sc_btn_data_tbl[i].max;
		}
		size	= sizeof(BR_BTN_DATA_SYS) + sizeof(BR_BTN_DATA) * buff_size;
	}

	//ヘッダつきバッファ作成
	p_wk	= GFL_HEAP_AllocMemory( heapID, size );
	GFL_STD_MemClear( p_wk, size );
  p_wk->setup = *cp_setup;

	//バッファに情報設定
  BR_BTN_DATA_SYS_ReLoadSaveData( p_wk );

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンデータバッファ破棄
 *
 *	@param	BR_BTN_DATA_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_BTN_DATA_SYS_Exit( BR_BTN_DATA_SYS *p_wk )
{	
	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンにセーブデータを再読み込み
 *	        （内部ではボタンに押せるかどうかを設定させている）
 *
 *	@param	BR_BTN_DATA_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_BTN_DATA_SYS_ReLoadSaveData( BR_BTN_DATA_SYS *p_wk )
{ 
  const BR_SAVE_INFO  *cp_saveinfo  = p_wk->setup.cp_saveinfo;

	//バッファに情報設定
	{	
		int i,j;
		u32 max;
		u32 cnt;
		const BR_BTN_DATA *cp_data;

		cnt	= 0;
		for( i = 0; i < BR_MENUID_MAX; i++ )
		{	
			max			= sc_btn_data_tbl[i].max;
			cp_data	= sc_btn_data_tbl[i].cp_data;
			for( j = 0; j < max; j++ )
			{	
				//初期データを設定
				p_wk->buff[cnt]	= cp_data[j];


				//外部からの情報をいれる
				switch( p_wk->buff[cnt].param[ BR_BTN_DATA_PARAM_UNIQUE ] )
				{	
				//自分の記録がなかったら押せない
				case BR_BTN_UNIQUE_MYRECORD:
					p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_VALID]	= cp_saveinfo->is_valid[ 0 ];
					break;

				//誰かの記録がなかったら押せない
				//押せるならば表示が「あき」→「×××のきろく」に切り替る
				case BR_BTN_UNIQUE_OTHERRECORD1:
					/* fallthrough */
				case BR_BTN_UNIQUE_OTHERRECORD2:
					/* fallthrough */
				case BR_BTN_UNIQUE_OTHERRECORD3:
					{	
            u32 id  = p_wk->buff[cnt].param[ BR_BTN_DATA_PARAM_UNIQUE ] - BR_BTN_UNIQUE_OTHERRECORD1;

						GF_ASSERT_HEAVY( id < 4 );
						p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_VALID]	= cp_saveinfo->is_valid[ 1+id ];
						if( p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_VALID] )
						{	
							p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_MSGID]	= msg_25;
						}
					}
          break;

        case BR_BTN_UNIQUE_MUSICALSHOT:
          { 
            p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_VALID]	= cp_saveinfo->is_musical_valid;
          }
          break;

				default:	//その他はどんなときでも押せる
					p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_VALID]	= TRUE;	
				}
				cnt++;


			}
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンデータを取得
 *
 *  @param	p_wk							ワーク
 *	@param	BR_MENUID menuID	所属するメニューID
 *	@param	btnID							ボタンID
 *
 *	@return	ボタンデータ
 */
//-----------------------------------------------------------------------------
const BR_BTN_DATA * BR_BTN_DATA_SYS_GetData( const BR_BTN_DATA_SYS *cp_wk, BR_MENUID menuID, u16 btnID )
{	
	int i;
	u32 offset;

	GF_ASSERT_HEAVY( menuID < BR_MENUID_MAX );
	GF_ASSERT_HEAVY( btnID  < sc_btn_data_tbl[menuID].max );

	//オフセット計算
	offset	= 0;
	for( i = 0; i < menuID; i++ )
	{	
		offset	+= sc_btn_data_tbl[i].max;
	}

	return &(cp_wk->buff[offset + btnID]);
}

//----------------------------------------------------------------------------
/**
 *	@brief	メニューごとのボタン最大数を取得
 *
 *  @param	p_wk							ワーク
 *	@param	BR_MENUID menuID	メニューID
 *
 *	@return	ボタンの数
 */
//-----------------------------------------------------------------------------
u32 BR_BTN_DATA_SYS_GetDataNum( const BR_BTN_DATA_SYS *cp_wk, BR_MENUID menuID )
{	
	GF_ASSERT_HEAVY( menuID < BR_MENUID_MAX );
	return sc_btn_data_tbl[menuID].max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	それぞれのメニューを交えたメニューのボタン最大数を取得
 *  @param	p_wk							ワーク
 *	@return	ボタンの数
 */
//-----------------------------------------------------------------------------
u32 BR_BTN_DATA_SYS_GetDataMax( const BR_BTN_DATA_SYS *cp_wk )
{	
	int i;
	int max;

	max	= 0;
	for( i = 0; i < BR_MENUID_MAX; i++ )
	{	
		if( max	< BR_BTN_DATA_SYS_GetDataNum( cp_wk, i ) )
		{	
			max	= BR_BTN_DATA_SYS_GetDataNum( cp_wk, i );
		}

	}
	return max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	リンクを取得
 *
 *	@param	const BR_BTN_DATA_SYS *cp_wk	ワーク
 *	@param	menuID		メニューID
 *	@param	idx				どのくらい前のリンクか	0で1つ前　1で2つ前
 *
 *	@return	リンク
 */
//-----------------------------------------------------------------------------
void BR_BTN_DATA_SYS_GetLink( const BR_BTN_DATA_SYS *cp_wk, BR_MENUID menuID, u32 idx, BR_MENUID *p_preID, u32 *p_btnID )
{	
	u32 max;
	max	= BR_BTN_DATA_SYS_GetLinkMax( cp_wk, menuID );

	GF_ASSERT_HEAVY( idx < max );

	if( p_preID )
	{	
		*p_preID	= sc_btn_data_tbl[ menuID ].cp_link[idx].menuID;
	}

	if( p_btnID )
	{	
		*p_btnID	= sc_btn_data_tbl[ menuID ].cp_link[idx].btnID;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	リンク数を取得
 *
 *	@param	const BR_BTN_DATA_SYS *cp_wk	ワーク
 *	@param	menuID												メニューID
 *
 *	@return	リンク数
 */
//-----------------------------------------------------------------------------
u32 BR_BTN_DATA_SYS_GetLinkMax( const BR_BTN_DATA_SYS *cp_wk, BR_MENUID menuID )
{	
	int i;
	GF_ASSERT_HEAVY( menuID < BR_MENUID_MAX );

	for( i = 0; sc_btn_data_tbl[ menuID ].cp_link[i].menuID != BR_BTN_LINK_END; i++ )
	{
		/* nothing */
	}

	return i;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの公開情報を取得
 *
 *	@param	const BR_BTN_DATA *cp_data	データ
 *	@param	paramID											公開情報インデックス
 *
 *	@return	パラメータ
 */
//-----------------------------------------------------------------------------
u16 BR_BTN_DATA_GetParam( const BR_BTN_DATA *cp_data, BR_BTN_DATA_PARAM paramID )
{	
	GF_ASSERT_HEAVY( paramID < BR_BTN_DATA_PARAM_MAX );
	return cp_data->param[paramID];
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンに表示する文字列を返す
 *
 *	@param	const BR_BTN_DATA *cp_data  ワーク
 *	@param	*p_msg                      メッセージデータ
 *	@param	heapID                      ヒープID
 *
 *	@return ボタンに表示する文字列
 */
//-----------------------------------------------------------------------------
STRBUF *BR_BTN_DATA_CreateString( const BR_BTN_DATA_SYS *cp_sys, const BR_BTN_DATA *cp_data, GFL_MSGDATA *p_msg, HEAPID heapID )
{ 
  STRBUF  *p_strbuf;
  BR_BTN_UNIQUE uni = BR_BTN_DATA_GetParam( cp_data, BR_BTN_DATA_PARAM_UNIQUE );
  u32 id;
  BOOL is_target  = FALSE;

  if( (BR_BTN_UNIQUE_MYRECORD <= uni && uni <= BR_BTN_UNIQUE_OTHERRECORD3 ) )
  { 
    id  = uni - BR_BTN_UNIQUE_MYRECORD;
    is_target = TRUE;
  }

  if( is_target )
  {	
    if( cp_sys->setup.cp_saveinfo->is_valid[ id ] )
    { 
      WORDSET *p_word;
      STRBUF  *p_src;

      //単語作成を一時的に使用
      p_word  = WORDSET_Create( GFL_HEAP_LOWID( heapID ) );

      WORDSET_RegisterWord( p_word, 0, 
          cp_sys->setup.cp_saveinfo->p_name[ id ],
          cp_sys->setup.cp_saveinfo->sex[ id ], TRUE, PM_LANG );

      p_src	= GFL_MSG_CreateString( p_msg, BR_BTN_DATA_GetParam( cp_data, BR_BTN_DATA_PARAM_MSGID) );
      p_strbuf	= GFL_MSG_CreateString( p_msg, BR_BTN_DATA_GetParam( cp_data, BR_BTN_DATA_PARAM_MSGID) );
      WORDSET_ExpandStr( p_word, p_strbuf, p_src );
      WORDSET_Delete( p_word );
      GFL_STR_DeleteBuffer( p_src );
    }
    else
    { 
      p_strbuf	= GFL_MSG_CreateString( p_msg, BR_BTN_DATA_GetParam( cp_data, BR_BTN_DATA_PARAM_MSGID) );
    }
  }
  else
  { 
    p_strbuf	= GFL_MSG_CreateString( p_msg, BR_BTN_DATA_GetParam( cp_data, BR_BTN_DATA_PARAM_MSGID) );
  }


  return p_strbuf;
}
