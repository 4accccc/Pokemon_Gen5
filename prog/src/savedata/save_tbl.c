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
#include "savedata/wifilist.h"
#include "savedata/c_gear_data.h"
#include "savedata/trainercard_data.h"
#include "savedata/mystery_data.h"
#include "savedata/situation.h"
#include "perapvoice_local.h"
#include "savedata/system_data.h"
#include "savedata/record.h"
#include "system/pms_word.h"
#include "field/fldmmdl.h"
#include "savedata/wifi_hiroba_save.h"
#include "savedata/musical_save.h"
#include "savedata/randommap_save.h"
#include "savedata/irc_compatible_savedata.h"
#include "savedata/sp_ribbon_save.h"
#include "savedata/worldtrade_data.h"
#include "field/eventwork.h"

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
	{	//状況データ
		GMDATA_ID_SITUATION,
		(FUNC_GET_SIZE)Situation_GetWorkSize,
		(FUNC_INIT_WORK)Situation_Init,
	},
	{	//WiFi通信履歴データ
		GMDATA_ID_WIFIHISTORY,
		(FUNC_GET_SIZE)WIFIHISTORY_GetWorkSize,
		(FUNC_INIT_WORK)WIFIHISTORY_Init,
	},
	{	//WiFi友達コードデータ
		GMDATA_ID_WIFILIST,
		(FUNC_GET_SIZE)WifiList_GetWorkSize,
		(FUNC_INIT_WORK)WifiList_Init,
	},
	{	// C-GEAR
		GMDATA_ID_CGEAR,
		(FUNC_GET_SIZE)CGEAR_SV_GetWorkSize,
		(FUNC_INIT_WORK)CGEAR_SV_Init,
	},
	{	//トレーナーカード(サイン
		GMDATA_ID_TRCARD,
		(FUNC_GET_SIZE)TRCSave_GetSaveDataSize,
		(FUNC_INIT_WORK)TRCSave_InitSaveData,
	},
	{	//不思議な贈り物
		GMDATA_ID_MYSTERYDATA,
		(FUNC_GET_SIZE)MYSTERYDATA_GetWorkSize,
		(FUNC_INIT_WORK)MYSTERYDATA_Init,
	},
	{	//ぺラップボイス
		GMDATA_ID_PERAPVOICE,
		(FUNC_GET_SIZE)PERAPVOICE_GetWorkSize,
		(FUNC_INIT_WORK)PERAPVOICE_Init,
	},
	{	//システムデータ
		GMDATA_ID_SYSTEM_DATA,
		(FUNC_GET_SIZE)SYSTEMDATA_GetWorkSize,
		(FUNC_INIT_WORK)SYSTEMDATA_Init,
	},
	{	//レコード(スコア
		GMDATA_ID_RECORD,
		(FUNC_GET_SIZE)RECORD_GetWorkSize,
		(FUNC_INIT_WORK)RECORD_Init,
	},
	{	//簡易文
		GMDATA_ID_PMS,
		(FUNC_GET_SIZE)PMSW_GetSaveDataSize,
		(FUNC_INIT_WORK)PMSW_InitSaveData,
	},
	{
		GMDATA_ID_MMDL,
		(FUNC_GET_SIZE)MMDL_SAVEDATA_GetWorkSize,
		(FUNC_INIT_WORK)MMDL_SAVEDATA_Init,
	},
	{	//Wi－Fiひろば
		GMDATA_ID_WFHIROBA,
		(FUNC_GET_SIZE)SAVEDATA_WFLBY_GetWorkSize,
		(FUNC_INIT_WORK)SAVEDATA_WFLBY_InitWork,
	},
	{	//ミュージカル
		GMDATA_ID_MUSICAL,
		(FUNC_GET_SIZE)MUSICAL_SAVE_GetWorkSize,
		(FUNC_INIT_WORK)MUSICAL_SAVE_InitWork,
	},
  { //ランダム生成マップ
    GMDATA_ID_RANDOMMAP,  
    (FUNC_GET_SIZE)RANDOMMAP_SAVE_GetWorkSize,
    (FUNC_INIT_WORK)RANDOMMAP_SAVE_InitWork,
  },
	{	//相性チェック
		GMDATA_ID_IRCCOMPATIBLE,
    (FUNC_GET_SIZE)IRC_COMPATIBLE_SV_GetWorkSize,
    (FUNC_INIT_WORK)IRC_COMPATIBLE_SV_Init,
	},
  { //配布リボン
    GMDATA_ID_SP_RIBBON,
    (FUNC_GET_SIZE)SP_RIBBON_SAVE_GetWorkSize,
    (FUNC_INIT_WORK)SP_RIBBON_SAVE_InitWork,
  },
  { //イベントワーク
    GMDATA_ID_EVENT_WORK,
    (FUNC_GET_SIZE)EVENTWORK_GetWorkSize,
    (FUNC_INIT_WORK)EVENTWORK_InitWork,
  },
	{	//GTS
    GMDATA_ID_WORLDTRADEDATA,
    (FUNC_GET_SIZE)WorldTradeData_GetWorkSize,
    (FUNC_INIT_WORK)WorldTradeData_Init,
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

