//==============================================================================
/**
 * @file    union_chat.c
 * @brief   チャット処理
 * @author  matsuda
 * @date    2009.08.26(水)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "net_app/union/union_main.h"
#include "print\str_tool.h"
#include "union_local.h"
#include "union_chat.h"


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static BOOL UnionChat_CheckSameLog(UNION_CHAT_LOG *log, const u8 *mac_address, u16 rand);



//==================================================================
/**
 * チャット登録
 *
 * @param   unisub		
 * @param   bpc		          発言したプレイヤー(自分の発言の場合はNULL指定)
 * @param   mine_pmsdata		自分の発言(自分ではない人の発言の場合はNULL指定)
 * @param   mine_chat_type	自分の発言のチャットタイプ(自分ではない人の発言の場合は無視)
 */
//==================================================================
void UnionChat_AddChat(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *bpc, const PMS_DATA *mine_pmsdata, UNION_CHAT_TYPE mine_chat_type)
{
  UNION_CHAT_LOG *log = &unisys->chat_log;
  UNION_CHAT_DATA *dest;
  STRCODE *name;
  
  GF_ASSERT(bpc != NULL || mine_pmsdata != NULL);
  
  //自分の発言は同一チェックを通さない。他人の発言は同一チェックをする
  if(bpc != NULL && UnionChat_CheckSameLog(log, bpc->mac_address, bpc->beacon.pms_rand) == FALSE){
    return;
  }
  
  //ログ管理データにデータ代入
  dest = UnionChat_GetSpaceBuffer(log);
  if(bpc == NULL){
    MYSTATUS *myst = unisys->uniparent->mystatus;
    MyStatus_CopyNameStrCode(myst, dest->name, PERSON_NAME_SIZE + EOM_SIZE);
    GF_ASSERT(mine_pmsdata != NULL || (mine_pmsdata==NULL && mine_chat_type != UNION_CHAT_TYPE_NORMAL));
    dest->pmsdata = *mine_pmsdata;
    dest->chat_type = mine_chat_type;
    dest->rand = 0;
    OS_GetMacAddress(dest->mac_address);
    dest->sex = MyStatus_GetMySex(myst);
  }
  else{
    STRTOOL_Copy(bpc->beacon.name, dest->name, PERSON_NAME_SIZE + EOM_SIZE);
    dest->pmsdata = bpc->beacon.pmsdata;
    dest->chat_type = bpc->beacon.chat_type;
    dest->rand = bpc->beacon.pms_rand;
    GFL_STD_MemCopy(bpc->mac_address, dest->mac_address, 6);
    dest->sex = bpc->beacon.sex;
  }
  
  //現在見ている表示位置が終端であるなら、表示位置も追加されたチャット分に合わせてずらしていく
  if(log->chat_log_count == log->chat_view_no){
    log->chat_view_no++;
  }
  log->chat_log_count++;
  //表示位置がログ保存数から溢れた場所を見ているなら、その分表示位置を進める
  if(log->chat_log_count - log->chat_view_no >= UNION_CHAT_LOG_MAX){
    log->chat_view_no = log->chat_log_count - UNION_CHAT_LOG_MAX + 1;
  }
}

//--------------------------------------------------------------
/**
 * チャットログ管理データから空きバッファを取得する
 *
 * @param   log		チャットログ管理データへのポインタ
 *
 * @retval  UNION_CHAT_DATA *		空きバッファへのポインタ
 */
//--------------------------------------------------------------
UNION_CHAT_DATA * UnionChat_GetSpaceBuffer(UNION_CHAT_LOG *log)
{
  UNION_CHAT_DATA *buffer;
  
  buffer = &log->chat[log->end_no];
  
  log->end_no++;
  if(log->end_no >= UNION_CHAT_LOG_MAX){
    log->end_no = 0;
  }
  if(log->end_no == log->start_no){
    log->start_no++;
    if(log->start_no >= UNION_CHAT_LOG_MAX){
      log->start_no = 0;
    }
  }
  
  return buffer;
}

//--------------------------------------------------------------
/**
 * チャットログ管理データから指定ログ番号のデータを読み取る
 *
 * @param   log		    チャットログ管理データへのポインタ
 * @param   log_no		ログ番号
 *
 * @retval  UNION_CHAT_DATA *		チャットデータへのポインタ(データが無い場合はNULL)
 */
//--------------------------------------------------------------
UNION_CHAT_DATA * UnionChat_GetReadBuffer(UNION_CHAT_LOG *log, u32 log_no)
{
  u32 buffer_pos;
  static const u8 invalid_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  
  buffer_pos = (log->start_no + log_no) % UNION_CHAT_LOG_MAX;
  if(GFL_STD_MemComp(log->chat[buffer_pos].mac_address, invalid_mac, 6) == 0){
    return NULL;
  }
  return &log->chat[buffer_pos];
}

//--------------------------------------------------------------
/**
 * チャットデータが同じ人物が最後に発言したチャットデータと同じものではないか調べる
 *
 * @param   log		        ログ管理データへのポインタ
 * @param   mac_address		発言者のMacAddress
 * @param   rand  	      発言者のランダムコード
 *
 * @retval  BOOL		TRUE:同じ発言ではない。　FALSE:同じ発言である
 */
//--------------------------------------------------------------
static BOOL UnionChat_CheckSameLog(UNION_CHAT_LOG *log, const u8 *mac_address, u16 rand)
{
  int log_no;
  UNION_CHAT_DATA *check_chat;
  
  for(log_no = UNION_CHAT_LOG_MAX - 1; log_no > -1; log_no--){
    check_chat = UnionChat_GetReadBuffer(log, log_no);
    if(check_chat != NULL){
      if(GFL_STD_MemComp(check_chat->mac_address, mac_address, 6) == 0){
        if(check_chat->rand != rand){
          return TRUE;  //同じ人物の直前のログが違うrandコードの為、新規の発言
        }
        return FALSE;
      }
    }
  }
  return TRUE;  //この人物の発言がログに存在しない
}

