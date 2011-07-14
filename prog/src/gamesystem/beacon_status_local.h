/*
 *  @file   beacon_status_local.h
 *  @brief  ゲーム中保持するビーコンログ管理 ローカルヘッダ
 *  @author Miyuki Iwasawa
 *  @date   10.01.21
 */

#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/beacon_status.h"

struct _TAG_BEACON_STATUS{
  GAMEBEACON_INFO_TBL view_log; 
  
  u16 ret_fword_input;  ///<フリーワード入力があったかの結果を受けるワーク(u16の必要がある)
  u8  view_top_ofs;     ///<ビューリストのトップのデータオフセット
 
  u8  sp_gpower_req_ct; ///<スペシャルGパワーのリクエスト数
  u8  sp_gpower_req[SP_GPOWER_REQ_MAX]; ///<スペシャルGパワーリクエストスタック

  GPOWER_ID beacon_gpower_id; ///<ビーコンで届いたGパワーの退避場所
  STRBUF* str_beacon_gpower_owner; ///<ビーコンで届いたGパワーの送り主トレーナー名
  
  STRBUF* str_fword;  //フリーワード
#ifdef PM_DEBUG
  u8  debug_stack_check_throw:1;
#endif
};


