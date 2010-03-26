/*
 *  @file   beacon_status.c
 *  @brief  ゲーム中保持するビーコンログ管理
 *  @author Miyuki Iwasawa
 *  @date   10.01.21
 */
#include <gflib.h>
#include "system/main.h"

#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/beacon_status.h"

#include "arc/arc_def.h"
#include "msgdata.h"
#include "message.naix"
#include "msg/msg_beacon_status.h"

struct _TAG_BEACON_STATUS{
  u8  view_top_ofs; ///<ビューリストのトップのデータオフセット
  GAMEBEACON_INFO_TBL view_log; 

  STRBUF* str_fword;  //フリーワード
};

#define BUFLEN_INTRODUCTION_MSG (GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN+EOM_SIZE)

/*
 *  @brief  ビーコンステータスワーク生成
 */
BEACON_STATUS* BEACON_STATUS_Create( HEAPID heapID, HEAPID tmpHeapID )
{
  BEACON_STATUS* wk;

  wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BEACON_STATUS));
  wk->str_fword = GFL_STR_CreateBuffer( BUFLEN_INTRODUCTION_MSG ,heapID );
	
  {
    GFL_MSGDATA* msg;
    STRBUF* buf;
    STRCODE code[BUFLEN_INTRODUCTION_MSG];
    msg = GFL_MSG_Create(
		  GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_beacon_status_dat, tmpHeapID );
    //直接gmmから文字列データを取得すると2倍のstrlenでチェックされ、
    //実際に必要なバッファ長のSTRBUFには入らないため、一度STRCODE変換をかける
    buf = GFL_MSG_CreateString( msg, msg_sys_freeword_ini );
    GFL_STR_GetStringCode( buf, code, BUFLEN_INTRODUCTION_MSG );
    GFL_STR_SetStringCode( wk->str_fword, code );
    
    GFL_STR_DeleteBuffer( buf );

    GFL_MSG_Delete( msg );
  }
  return wk;
}

/*
 *  @brief  ビーコンステータスワーク破棄
 */
void BEACON_STATUS_Delete( BEACON_STATUS* wk )
{
  GFL_STR_DeleteBuffer(wk->str_fword);
  MI_CpuClear8( wk, sizeof(BEACON_STATUS));
  GFL_HEAP_FreeMemory( wk );
}

/*
 *  @brief  InfoTblを取得
 */
GAMEBEACON_INFO_TBL* BEACON_STATUS_GetInfoTbl( BEACON_STATUS* wk )
{
  return &wk->view_log;
}

/*
 *  @brief  ビューリストのトップオフセットを取得
 */
u8  BEACON_STATUS_GetViewTopOffset( BEACON_STATUS* wk )
{
  return wk->view_top_ofs;
}

/*
 *  @brief  ビューリストのトップオフセットをセット
 */
void BEACON_STATUS_SetViewTopOffset( BEACON_STATUS* wk, u8 ofs )
{
  wk->view_top_ofs = ofs;
}

/*
 *  @brief  フリーワードバッファのアドレスを取得
 */
STRBUF* BEACON_STATUS_GetFreeWordBuffer( BEACON_STATUS* wk )
{
  return wk->str_fword; 
}

