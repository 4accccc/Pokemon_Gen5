//============================================================================================
/**
 * @file	start_data.c
 * @brief	ゲーム開始時のデータ追加処理
 * @author	tamada GAME FREAK Inc.
 * @date	09.10.24
 *
 */
//============================================================================================

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"

#include "gamesystem/start_data.h"


#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_memo.h"
#include "item/item.h"  //デバッグアイテム生成用
#include "waza_tool/wazano_def.h"  //デバッグポケ生成用
#include "savedata/zukan_savedata.h"		// 図鑑捕獲フラグ設定

#include "savedata/intrude_save_field.h"
#include "savedata/intrude_save.h"

#include "arc_def.h"  //ARCID_MESSAGE
#include "system/gfl_use.h"   //GFUser_GetPublicRand

#include "app/research_radar/question_id.h"
#include "savedata/questionnaire_save.h"

#include "savedata/mail_util.h"   // MAIL_BLOCK
#include "arc/message.naix"       // メッセージデータ参照用
#include "msg/msg_trname.h"       // 文字列名support_01
#include "msg/msg_pmss_peculiar.h"

#include "savedata/dummy3ds_savedata.h"
//============================================================================================
//============================================================================================
//----------------------------------------------------------------------------------
/**
 * @brief   ゲーム開始用に手持ちどうぐを加える
 * @param   gamedata  GAMEDATAへのポインタ		
 * @param   heapID    利用するヒープIDの指定
 */
//----------------------------------------------------------------------------------
static void makeMyBag(GAMEDATA * gamedata, HEAPID heapID)
{
	u32	i;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );
  MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );

	MYITEM_Init( myitem );
  if ( MyStatus_GetMySex( mystat ) == PM_MALE )
  {
		MYITEM_AddItem( myitem, ITEM_RAIBUKYASUTAA, 1, heapID );
	}
  else
  {
		MYITEM_AddItem( myitem, ITEM_RAIBUKYASUTAA_1, 1, heapID );
  }
  MYITEM_AddItem( myitem, ITEM_TOMODATITETYOU, 1, heapID );
}

//----------------------------------------------------------------------------------
//  CYGNUS対応用のswan側内部データを初期化
//----------------------------------------------------------------------------------
static void makeCygnusInnerData(GAMEDATA * gamedata, HEAPID heapID )
{
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  DUMMY3DS_SAVEDATA * d3ds = SaveData_GetDummy3ds( sv );
  u32 onetime_key = GFUser_GetPublicRand( GFL_STD_RAND_MAX );
  DUMMY3DS_SAVEDATA_SetData( d3ds, DUMMY3DS_SAVEDATA_PARAM_ONETIME_KEY, onetime_key );
  DUMMY3DS_SAVEDATA_SetData( d3ds, DUMMY3DS_SAVEDATA_PARAM_IS_PLAYED, FALSE );
  DUMMY3DS_SAVEDATA_SetData( d3ds, DUMMY3DS_SAVEDATA_PARAM_FLAG, 0 );
}


//============================================================================================
//============================================================================================
//----------------------------------------------------------------------------------
//  「さいしょから」ではじめたときのデータ初期化処理
//----------------------------------------------------------------------------------
void GAMEINIT_SetNormalStartData( GAMEDATA * gamedata, HEAPID heapID )
{
  makeMyBag( gamedata, heapID );
}

//============================================================================================
//============================================================================================
//----------------------------------------------------------------------------------
//  「さいしょから」「デバッグ開始」共通のデータ初期化処理
//----------------------------------------------------------------------------------
void GAMEINIT_SetCommonStartData( GAMEDATA * gamedata, HEAPID heapID )
{
  makeCygnusInnerData( gamedata, heapID );
}



