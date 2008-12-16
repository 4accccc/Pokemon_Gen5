//==============================================================================
/**
 * @file	save.c
 * @brief	WBセーブ
 * @author	matsuda
 * @date	2008.08.27(水)
 */
//==============================================================================
#include <gflib.h>
#include <backup_system.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/contest_savedata.h"
#include "savedata/test_savedata.h"
#include "savedata/box_savedata.h"
#include "savedata/myitem_savedata.h"
#include "poke_tool/pokeparty.h"
#include "playerdata_local.h"
#include "savedata/wifihistory.h"


//==============================================================================
//	定数定義
//==============================================================================
#define MAGIC_NUMBER	(0x31053527)
#define	SECTOR_SIZE		(SAVE_SECTOR_SIZE)
#define SECTOR_MAX		(SAVE_PAGE_MAX)

///ノーマル領域で使用するセーブサイズ
#define SAVESIZE_NORMAL		(SECTOR_SIZE * SECTOR_MAX)
///EXTRA1領域で使用するセーブサイズ
#define SAVESIZE_EXTRA1		(SECTOR_SIZE)
///EXTRA2領域で使用するセーブサイズ
#define SAVESIZE_EXTRA2		(SECTOR_SIZE)

///ミラーリング領域で使用するセーブ開始アドレス
#define MIRROR_SAVE_ADDRESS		(0x20000)


static const GFL_SAVEDATA_TABLE SaveDataTbl_Normal[] = {
	{	//コンテスト
		GMDATA_ID_CONTEST,
//		SVBLK_ID_NORMAL,
		(FUNC_GET_SIZE)CONDATA_GetWorkSize,
		(FUNC_INIT_WORK)CONDATA_Init,
	},
	{	//松田デバッグ機能用セーブ
		GMDATA_ID_MATSU_DEBUG,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_0,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_0,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_1,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_1,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_2,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_2,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_3,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_3,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_4,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_4,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_5,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_5,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_6,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_6,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_7,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_7,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_8,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_8,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_9,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_9,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_a,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_a,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_b,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_b,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_c,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_c,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_d,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_d,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},
	{	//テスト
		GMDATA_ID_MATSU_DEBUG_e,
		(FUNC_GET_SIZE)DebugMatsuSave_GetWorkSize_e,
		(FUNC_INIT_WORK)DebugMatsuSave_Init,
	},

	{	//BOXデータ(中身は仮です Ari081107
		GMDATA_ID_BOXDATA,
		(FUNC_GET_SIZE)BOXDAT_GetTotalSize,
		(FUNC_INIT_WORK)BOXDAT_Init,
	},
	{	//手持ちアイテム
		GMDATA_ID_MYITEM,
		(FUNC_GET_SIZE)MYITEM_GetWorkSize,
		(FUNC_INIT_WORK)MYITEM_Init,
	},
	{	//手持ちポケモン
		GMDATA_ID_MYPOKE,
		(FUNC_GET_SIZE)PokeParty_GetWorkSize,
		(FUNC_INIT_WORK)PokeParty_InitWork,
	},
	{	//プレイヤーデータ
		GMDATA_ID_PLAYER_DATA,
		(FUNC_GET_SIZE)PLAYERDATA_GetWorkSize,
		(FUNC_INIT_WORK)PLAYERDATA_Init,
	},
	{	//WiFi通信履歴データ
		GMDATA_ID_WIFIHISTORY,
		(FUNC_GET_SIZE)WIFIHISTORY_GetWorkSize,
		(FUNC_INIT_WORK)WIFIHISTORY_Init,
	},
};


//--------------------------------------------------------------
/**
 * @brief   セーブパラメータテーブル
 */
//--------------------------------------------------------------
const GFL_SVLD_PARAM SaveParam_Normal = {
	SaveDataTbl_Normal,
	NELEMS(SaveDataTbl_Normal),
	0,								//バックアップ領域先頭アドレス
	MIRROR_SAVE_ADDRESS,			//ミラーリング領域先頭アドレス
	SAVESIZE_NORMAL,				//使用するバックアップ領域の大きさ
	MAGIC_NUMBER,
	GFL_BACKUP_NORMAL_FLASH,
};

