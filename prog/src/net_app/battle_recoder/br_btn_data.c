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

#include "msg/msg_battle_rec.h"
#include "br_btn_data.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ボタンの座標
//=====================================
#define BR_BTN_POS_X					(42)
#define BR_BTN_POS_START_Y		(25)	//開始オフセット
#define BR_BTN_POS_OFFSET_Y		(36)	//間隔オフセット
#define BR_BTN_POS_NUMDIF_Y		(16)	//個数での位置調整オフセット
//座標取得マクロ
#define BR_BTN_DATA_GET_Y( n,m )	(BR_BTN_POS_START_Y+BR_BTN_POS_OFFSET_Y*n+BR_BTN_POS_NUMDIF_Y*(5-m))


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ボタン情報
//=====================================
struct _BR_BTN_DATA
{	
	u16	param[BR_BTN_DATA_PARAM];
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
static const sc_btn_data_browse_top[BTN_DATA_BROWSE_TOP_MAX] =
{	
	//バトルビデオ
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_TOP_MAX),
		BR_BROWSE_MENUID_TOP,
		msg_01,
		0,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_BTLVIDEO,

	},
	//バトルサブウェイ
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_TOP_MAX),
		BR_BROWSE_MENUID_TOP,
		msg_02,
		0,
		BR_BTN_TYPE_CHANGESEQ,
		0,

	},
	//ランダムマッチ
	{
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_TOP_MAX),
		BR_BROWSE_MENUID_TOP,
		msg_03,
		0,
		BR_BTN_TYPE_CHANGESEQ,
		0,

	},
	//やめる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BROWSE_TOP_MAX),
		BR_BROWSE_MENUID_TOP,
		msg_04,
		4,
		BR_BTN_TYPE_EXIT,
		0,
	},
};
//-------------------------------------
///	ブラウザモード	バトルビデオメニュー
//=====================================
#define BTN_DATA_BROWSE_BTLVIDEO_MAX	(4)
static const sc_btn_data_browse_btlviedeo[BTN_DATA_BROWSE_BTLVIDEO_MAX] =
{	
	//自分の記録
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_BTLVIDEO_MAX),
		BR_BROWSE_MENUID_BTLVIDEO,
		msg_06,
		6,
		BR_BTN_TYPE_MYRECORD,
		0,
	},
	//誰かの記録
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_BTLVIDEO_MAX),
		BR_BROWSE_MENUID_BTLVIDEO,
		msg_07,
		7,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_OTHER_RECORD,
	},
	//記録を消す
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_BTLVIDEO_MAX),
		BR_BROWSE_MENUID_BTLVIDEO,
		msg_08,
		1,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_DELETE_RECORD,
	},
	//もどる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BROWSE_BTLVIDEO_MAX),
		BR_BROWSE_MENUID_BTLVIDEO,
		msg_05,
		5,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_TOP,
	},
};

//-------------------------------------
///	ブラウザモード	誰かの記録メニュー
//=====================================
#define BTN_DATA_BROWSE_OTHER_RECORD_MAX	(4)
static const sc_btn_data_browse_other_record[BTN_DATA_BROWSE_OTHER_RECORD_MAX] =
{	
	//誰かの記録１
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_10,
		7,
		BR_BTN_TYPE_OTHERRECORD,
		0,
	},
	//誰かの記録２
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_10,
		7,
		BR_BTN_TYPE_OTHERRECORD,
		1,
	},
	//誰かの記録３
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_10,
		7,
		BR_BTN_TYPE_OTHERRECORD,
		2,
	},
	//戻る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_05,
		5,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_BTLVIDEO,
	},
};
//-------------------------------------
///	ブラウザモード	記録を消すメニュー
//=====================================
#define BTN_DATA_BROWSE_DELETE_RECORD_MAX	(3)
static const sc_btn_data_browse_delete_record[BTN_DATA_BROWSE_DELETE_RECORD_MAX] =
{	
	//自分の記録
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_DELETE_RECORD_MAX),
		BR_BROWSE_MENUID_DELETE_RECORD,
		msg_100,
		6,
		BR_BTN_TYPE_DELETE_MY,
		0,
	},
	//誰かの記録
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_DELETE_RECORD_MAX),
		BR_BROWSE_MENUID_DELETE_RECORD,
		msg_101,
		7,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_DELETE_OTHER,
	},
	//もどる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_DELETE_RECORD_MAX),
		BR_BROWSE_MENUID_DELETE_RECORD,
		msg_05,
		5,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_BTLVIDEO,
	},
};
//-------------------------------------
///	ブラウザモード	誰かの記録を消すメニュー
//=====================================
#define BTN_DATA_BROWSE_DELETE_OTHER_MAX	(4)
static const sc_btn_data_browse_delete_other[BTN_DATA_BROWSE_DELETE_OTHER_MAX] =
{	
	//誰かの記録１
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_10,
		7,
		BR_BTN_TYPE_DELETE_OTHER,
		0,
	},
	//誰かの記録２
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_10,
		7,
		BR_BTN_TYPE_DELETE_OTHER,
		1,
	},
	//誰かの記録３
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_10,
		7,
		BR_BTN_TYPE_DELETE_OTHER,
		2,
	},
	//戻る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_05,
		5,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_DELETE_RECORD,
	},
};
//-------------------------------------
///	グローバルバトルビデオモード	初期メニュー
//=====================================
#define BTN_DATA_BTLVIDEO_TOP_MAX	(3)
static const sc_btn_data_btlvideo_top[BTN_DATA_BTLVIDEO_TOP_MAX] =
{	
	//バトルビデオを見る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BTLVIDEO_TOP_MAX),
		BR_BTLVIDEO_MENUID_TOP,
		msg_700,
		2,
		BR_BTN_TYPE_SELECT,
		BR_BTLVIDEO_MENUID_LOOK,
	},
	//バトルビデオを送る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BTLVIDEO_TOP_MAX),
		BR_BTLVIDEO_MENUID_TOP,
		msg_700,
		9,
		BR_BTN_TYPE_CHANGESEQ,
		0,
	},
	//やめる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BTLVIDEO_TOP_MAX),
		BR_BTLVIDEO_MENUID_TOP,
		msg_700,
		14,
		BR_BTN_TYPE_SELECT,
		BR_BTLVIDEO_MENUID_LOOK,
	},
};
//-------------------------------------
///	グローバルバトルビデオモード	見るメニュー
//=====================================
#define BTN_DATA_BTLVIDEO_LOOK_MAX	(4)
static const sc_btn_data_btlvideo_look[BTN_DATA_BTLVIDEO_LOOK_MAX] =
{	
	//ランキングで探す
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BTLVIDEO_LOOK_MAX),
		BR_BTLVIDEO_MENUID_LOOK,
		msg_200,
		16,
		BR_BTN_TYPE_SELECT,
		BR_BTLVIDEO_MENUID_RANK,
	},
	//詳しく探す
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BTLVIDEO_LOOK_MAX),
		BR_BTLVIDEO_MENUID_LOOK,
		msg_203,
		17,
		BR_BTN_TYPE_SELECT,
		BR_BTLVIDEO_MENUID_RANK,
	},
	//ナンバーで探す
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BTLVIDEO_LOOK_MAX),
		BR_BTLVIDEO_MENUID_LOOK,
		msg_204,
		11,
		BR_BTN_TYPE_SELECT,
		BR_BTLVIDEO_MENUID_RANK,
	},
	//もどる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BTLVIDEO_LOOK_MAX),
		BR_BTLVIDEO_MENUID_LOOK,
		msg_05,
		5,
		BR_BTN_TYPE_RETURN,
		BR_BTLVIDEO_MENUID_TOP,
	},
};
//-------------------------------------
///	グローバルバトルビデオモード	ランキングで探すメニュー
//=====================================
#define BTN_DATA_BTLVIDEO_RANK_MAX	(4)
static const sc_btn_data_btlvideo_rank[BTN_DATA_BTLVIDEO_RANK_MAX] =
{	
	//最新30件
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_BTLVIDEO_RANK_MAX),
		BR_BTLVIDEO_MENUID_RANK,
		msg_202,
		10,
		BR_BTN_TYPE_CHANGESEQ,
		0,
	},
	//通信対戦ランキング
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BTLVIDEO_RANK_MAX),
		BR_BTLVIDEO_MENUID_RANK,
		msg_202,
		10,
		BR_BTN_TYPE_CHANGESEQ,
		0,
	},
	//サブウェイランキング
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BTLVIDEO_RANK_MAX),
		BR_BTLVIDEO_MENUID_RANK,
		msg_202,
		10,
		BR_BTN_TYPE_CHANGESEQ,
		0,
	},
	//もどる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(3,BTN_DATA_BTLVIDEO_RANK_MAX),
		BR_BTLVIDEO_MENUID_RANK,
		msg_05,
		5,
		BR_BTN_TYPE_RETURN,
		BR_BTLVIDEO_MENUID_LOOK,
	},
};
//-------------------------------------
///	グローバルバトルミュージカルショット	初期メニュー
//=====================================
#define BTN_DATA_MUSICAL_TOP_MAX	(3)
static const sc_btn_data_musical_top[BTN_DATA_MUSICAL_TOP_MAX] =
{	
	//写真を見る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_301,
		0,
		BR_BTN_TYPE_CHANGESEQ,
		0,
	},
	//写真を送る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_300,
		0,
		BR_BTN_TYPE_CHANGESEQ,
		0,
	},
	//やめる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_04,
		4,
		BR_BTN_TYPE_EXIT,
		0,
	},
};

//-------------------------------------
///	汎用	はい、いいえ
//=====================================
#define BTN_DATA_YESNO_MAX	(2)
static const sc_btn_data_yesno[BTN_DATA_YESNO_MAX] =
{	
	//はい
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(0,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_301,
		0,
		0,
		0
	},
	//いいえ
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_301,
		0,
		0,
		0
	},

};

//-------------------------------------
///	メニューごとのテーブル
//=====================================
static const struct 
{	
	u32					max;
	const BR_BTN_DATA *cp_data;
} scp_btn_data_tbl[BR_MENUID_MAX]	=
{	
	{	
		BTN_DATA_BROWSE_TOP_MAX,
		sc_btn_data_browse_top,
	},
	{	
		BTN_DATA_BROWSE_BTLVIDEO_MAX,
		sc_btn_data_browse_btlviedeo,
	},
	{	
		BTN_DATA_BROWSE_OTHER_RECORD_MAX,
		sc_btn_data_browse_other_record,
	},
	{	
		BTN_DATA_BROWSE_DELETE_RECORD_MAX,
		sc_btn_data_browse_delete_record,
	},
	{	
		BTN_DATA_BROWSE_DELETE_OTHER_MAX,
		sc_btn_data_browse_delete_other,
	},
	{	
		BTN_DATA_BTLVIDEO_TOP_MAX,
		sc_btn_data_btlvideo_top,
	},
	{	
		BTN_DATA_BTLVIDEO_LOOK_MAX,
		sc_btn_data_btlvideo_look,
	},
	{	
		BTN_DATA_BTLVIDEO_RANK_MAX,
		sc_btn_data_btlvideo_rank,
	},
	{	
		BTN_DATA_MUSICAL_TOP_MAX,
		sc_btn_data_musical_top,
	},
	{	
		BTN_DATA_YESNO_MAX,
		sc_btn_data_yesno,
	},
};

//=============================================================================
/**
 *					PUBLIC
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンデータを取得
 *
 *	@param	BR_MENUID menuID	所属するメニューID
 *	@param	btnID							ボタンID
 *
 *	@return	ボタンデータ
 */
//-----------------------------------------------------------------------------
const BR_BTN_DATA * BR_BTN_DATA_SYS_GetData( BR_MENUID menuID, u16 btnID )
{	
	GF_ASSERT( menuID < BR_MENUID_MAX );
	GF_ASSERT( btnID  < scp_btn_data_tbl[menuID].max );
	return scp_btn_data_tbl[menuID].cp_data[ btnID ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	メニューごとのボタン最大数を取得
 *
 *	@param	BR_MENUID menuID	メニューID
 *
 *	@return	ボタンの数
 */
//-----------------------------------------------------------------------------
u32 BR_BTN_DATA_SYS_GetDataNum( BR_MENUID menuID )
{	
	GF_ASSERT( menuID < BR_MENUID_MAX );
	return scp_btn_data_tbl[menuID].max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	それぞれのメニューを交えたメニューのボタン最大数を取得
 *
 *	@return	ボタンの数
 */
//-----------------------------------------------------------------------------
u32 BR_BTN_DATA_SYS_GetDataMax( void )
{	
	int i;
	int max;

	max	= 0;
	for( i = 0; i < BR_MENUID_MAX; i++ )
	{	
		if( max	< BR_BTN_DATA_SYS_GetDataNum( i ) )
		{	
			max	= BR_BTN_DATA_SYS_GetDataNum( i );
		}

	}
	return max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	モード別の初期メニュー取得
 *
 *	@param	BR_MODE mode モード
 *
 *	@return	初期メニュー
 */
//-----------------------------------------------------------------------------
BR_MENUID BR_BTN_DATA_SYS_GetStartMenuID( BR_MODE mode )
{	
	BR_MENUID	startID;

	switch( mode )
	{	
	case BR_MODE_BROWSE:
		startID	= BR_BROWSE_MENUID_TOP;
		break;
	case BR_MODE_GLOBAL_BV:
		startID	= BR_BTLVIDEO_MENUID_TOP;
		break;
	case BR_MODE_GLOBAL_MUSICAL:
		startID	= BR_MUSICAL_MENUID_TOP;
		break;
	default:
		GF_ASSERT(0);
	}

	return startID;
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
	GF_ASSERT( paramID < BR_BTN_DATA_PARAM_MAX );
	return cp_data->param[paramID];
}
