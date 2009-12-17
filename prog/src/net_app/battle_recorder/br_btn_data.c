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

#include "msg/msg_battle_rec.h"
#include "br_btn_data.h"
#include "br_core.h"
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
	const BR_BTLREC_SET *cp_rec;
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
		0,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_BTLVIDEO,
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
		0,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BTLSUBWAY,
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
		0,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_RNDMATCH,
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
		4,
		BR_BTN_TYPE_EXIT,
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
		6,
		BR_BTN_TYPE_MYRECORD,
		BR_PROCID_RECORD,
		0,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
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
		1,
		BR_BTN_TYPE_SELECT,
		BR_BROWSE_MENUID_DELETE_RECORD,
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
		5,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_TOP,
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
		7,
		BR_BTN_TYPE_OTHERRECORD,
		BR_PROCID_RECORD,
		1,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
	},
	//誰かの記録２
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_10,
		7,
		BR_BTN_TYPE_OTHERRECORD,
		BR_PROCID_RECORD,
		2,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
	},
	//誰かの記録３
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_OTHER_RECORD_MAX),
		BR_BROWSE_MENUID_OTHER_RECORD,
		msg_10,
		7,
		BR_BTN_TYPE_OTHERRECORD,
		BR_PROCID_RECORD,
		3,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
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
		6,
		BR_BTN_TYPE_DELETE_MY,
		0,
		0,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
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
		5,
		BR_BTN_TYPE_RETURN,
		BR_BROWSE_MENUID_BTLVIDEO,
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
		7,
		BR_BTN_TYPE_DELETE_OTHER,
		0,
		0,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
	},
	//誰かの記録２
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_10,
		7,
		BR_BTN_TYPE_DELETE_OTHER,
		0,
		1,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
	},
	//誰かの記録３
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BROWSE_DELETE_OTHER_MAX),
		BR_BROWSE_MENUID_DELETE_OTHER,
		msg_10,
		7,
		BR_BTN_TYPE_DELETE_OTHER,
		0,
		2,
		0,
		BR_BTN_NONEPROC_NOPUSH,
		msg_info_000,
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
		2,
		BR_BTN_TYPE_SELECT,
		BR_BTLVIDEO_MENUID_LOOK,
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
		9,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_SEND,
		0,
		0,
		0,
		0,
	},
	//やめる
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(2,BTN_DATA_BTLVIDEO_TOP_MAX),
		BR_BTLVIDEO_MENUID_TOP,
		msg_04,
		4,
		BR_BTN_TYPE_EXIT,
		0,
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
		16,
		BR_BTN_TYPE_SELECT,
		BR_BTLVIDEO_MENUID_RANK,
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
		17,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_SEARCH,
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
		11,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_CODEIN,
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
		5,
		BR_BTN_TYPE_RETURN,
		BR_BTLVIDEO_MENUID_TOP,
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
		10,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_RANK,
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
		msg_202,
		10,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_BV_RANK,
		1,
		0,
		0,
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
		BR_PROCID_BV_RANK,
		2,
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
		5,
		BR_BTN_TYPE_RETURN,
		BR_BTLVIDEO_MENUID_LOOK,
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
	},
	//写真を送る
	{	
		BR_BTN_POS_X,
		BR_BTN_DATA_GET_Y(1,BTN_DATA_MUSICAL_TOP_MAX),
		BR_MUSICAL_MENUID_TOP,
		msg_300,
		0,
		BR_BTN_TYPE_CHANGESEQ,
		BR_PROCID_MUSICAL_SEND,
		0,
		0,
		0,
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
///	汎用	はい、いいえ
//=====================================
#define BTN_DATA_YESNO_MAX	(2)
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
		BTN_DATA_MUSICAL_TOP_MAX,
		sc_btn_data_musical_top,
		sc_btn_link_musical_top,
	},
	{	
		BTN_DATA_YESNO_MAX,
		sc_btn_data_yesno,
		NULL,
	},
};

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
BR_BTN_DATA_SYS * BR_BTN_DATA_SYS_Init( const BR_BTLREC_SET *cp_rec, HEAPID heapID )
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
	p_wk->cp_rec	= cp_rec;

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
				switch( p_wk->buff[cnt].param[ BR_BTN_DATA_PARAM_TYPE ] )
				{	
				//自分の記録がなかったら押せない
				case BR_BTN_TYPE_MYRECORD:
					/* fallthrough */
				case BR_BTN_TYPE_DELETE_MY:
					p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_VALID]	= cp_rec->my.is_valid;
					break;

				//誰かの記録がなかったら押せない
				//押せるならば表示が「あき」→「×××のきろく」に切り替る
				case BR_BTN_TYPE_OTHERRECORD:
					/* fallthrough */
				case BR_BTN_TYPE_DELETE_OTHER:
					{	
						u32 id	= p_wk->buff[cnt].param[ BR_BTN_DATA_PARAM_DATA2 ];
						GF_ASSERT( id < 4 );
						p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_VALID]	= cp_rec->other[ id-1 ].is_valid;
						if( cp_rec->other[ id-1 ].is_valid )
						{	
							p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_MSGID]	= msg_09;
						}
					}
					break;

					//@todo ランダムマッチやバトルサブウェイはもしかしたら押せない状況がありえるかも

				default:	//その他はどんなときでも押せる
					p_wk->buff[cnt].param[BR_BTN_DATA_PARAM_VALID]	= TRUE;	
				}
				cnt++;


			}
		}
	}

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

	GF_ASSERT( menuID < BR_MENUID_MAX );
	GF_ASSERT( btnID  < sc_btn_data_tbl[menuID].max );

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
	GF_ASSERT( menuID < BR_MENUID_MAX );
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

	GF_ASSERT( idx < max );

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
	GF_ASSERT( menuID < BR_MENUID_MAX );

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
	GF_ASSERT( paramID < BR_BTN_DATA_PARAM_MAX );
	return cp_data->param[paramID];
}
