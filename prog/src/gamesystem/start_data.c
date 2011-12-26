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
//============================================================================================
//============================================================================================
//=====================================================================
//=====================================================================
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


#if 0
//2011.09.28  tamada
//swanではメールは最初からは入れておかない
//=====================================================================
//=====================================================================
static const PMS_DATA mail_pmsdata[]={
  // 「しょうぶ」してくれて　ありがとう！
  {
    PMS_TYPE_PECULIAR,  pmss_peculiar_10,
    { mail01, PMS_WORD_NULL},
  },
  // 「ドキドキ」で　とってもよかったです！
  {
    PMS_TYPE_PECULIAR,  pmss_peculiar_11,
    { mail02, PMS_WORD_NULL},
  },
  // また「しょうぶ」しようね！　「バイバイ」
  {
    PMS_TYPE_PECULIAR,  pmss_peculiar_21,
    { mail01, PMS_WORD_ONLY_DECO_BIT_WORD|PMS_DECOID_BYEBYE },
  },
};

//----------------------------------------------------------------------------------
/**
 * @brief メールボックスにベルからのメールを最初から入れておく
 *
 * @param   gamedata    
 */
//----------------------------------------------------------------------------------
static void MailBox_SetFirstMail( GAMEDATA* gamedata, HEAPID heapID )
{
  MAIL_DATA   *mail;
  GFL_MSGDATA *TrainerMsgData;
  STRBUF  *strbuf;
  STRCODE strdata[BUFLEN_PERSON_NAME];

  // トレーナー名メッセージデータオープン
  TrainerMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                                NARC_message_trname_dat, heapID );
  strbuf = GFL_MSG_CreateString( TrainerMsgData, support_01 );

  // サポート名の生文字列を取得
  GFL_STR_GetStringCode( strbuf, strdata, BUFLEN_PERSON_NAME );

  // メールデータ作成
  mail = MailData_CreateWork( heapID );

  // サポート名セット
  MailData_SetWriterName( mail, strdata );
  // 性別
  MailData_SetWriterSex( mail, PM_FEMALE );
  // メールデザインNo
  MailData_SetDesignNo( mail, 3 );
  // 国コードセット
  MailData_SetCountryCode( mail, PM_LANG );
  // カセットバージョン
  MailData_SetCasetteVersion( mail, PM_VERSION );
  // 簡易会話セット
  MailData_SetMsgByIndex( mail, (PMS_DATA*)&mail_pmsdata[0], 0 );
  MailData_SetMsgByIndex( mail, (PMS_DATA*)&mail_pmsdata[1], 1 );
  MailData_SetMsgByIndex( mail, (PMS_DATA*)&mail_pmsdata[2], 2 );
  // 簡易ワード「トレーナー」セット
  MailData_SetPmsWord( mail, mail04 );

  // メールボックスにメールをセット
  {
    MAIL_BLOCK *mail_block = GAMEDATA_GetMailBlock( gamedata );
    MAIL_AddMailFormWork( mail_block, MAILBLOCK_PASOCOM, 0, mail);
  }

  // ワーク解放
  GFL_HEAP_FreeMemory( mail );
  GFL_STR_DeleteBuffer( strbuf );
  GFL_MSG_Delete( TrainerMsgData );
}
#endif


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
void GAMEINIT_SetStartData( GAMEDATA * gamedata, HEAPID heapID )
{
  makeMyBag( gamedata, heapID );
  //MailBox_SetFirstMail( gamedata, heapID );
}


