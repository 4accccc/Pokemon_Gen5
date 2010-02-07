//==============================================================================
/**
 * @file    game_beacon_accessor.c
 * @brief   ビーコン情報の値取得の為のアクセサ
 * @author  matsuda
 * @date    2010.01.19(火)
 *
 * すれ違いログ画面でしか使用しないはずなので、常駐削減の為
 * アクセサをこのファイルに集めFIELDMAPオーバーレイに配置する
 */
//==============================================================================
#include <gflib.h>
#include "gamesystem/game_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_accessor.h"
#include "gamesystem/game_beacon_types.h"



//==============================================================================
//
//  ビーコンテーブル操作
//
//==============================================================================
//==================================================================
/**
 * ビーコンスタックテーブルを更新
 *
 * @param   stack_infotbl		スタックテーブルへのポインタ
 *
 * @retval  int		スタックテーブルへ移された件数
 */
//==================================================================
int GAMEBEACON_Stack_Update(GAMEBEACON_INFO_TBL *stack_infotbl)
{
  int start_log_no = 0, update_logno;
  const GAMEBEACON_INFO *update_info;
  int new_stack_count = 0;
  RTCTime recv_time;
  u16 time;
  
  GFL_RTC_GetTime(&recv_time);
  time = (recv_time.hour << 8) | recv_time.minute;
  
  do{
    update_logno = GAMEBEACON_Get_UpdateLogNo(&start_log_no);
    if(update_logno == GAMEBEACON_SYSTEM_LOG_MAX){
      break;  //ビーコンログ無し
    }
    update_info = GAMEBEACON_Get_BeaconLog(update_logno);
    if(GAMEBEACON_InfoTbl_SetBeacon(
        stack_infotbl, update_info, time, FALSE) == GAMEBEACON_INFO_TBL_MAX){
      break;  //スタックテーブルがいっぱいになった
    }
    GAMEBEACON_Reset_UpdateFlag(update_logno);
    new_stack_count++;
  }while(1);
  
  return new_stack_count;
}

//==================================================================
/**
 * スタックテーブルからビーコン情報を取得する
 *
 * @param   stack_infotbl		スタックテーブルへのポインタ
 * @param   dest_info		    ビーコン情報代入先へのポインタ
 * @param   time            受信日時の代入先へのポインタ(上位8bit：時(0～23)、下位8bit：分(0～59))
 *
 * @retval  BOOL		TRUE:情報を取得した　　FALSE:情報取得失敗(スタックテーブルにデータが無い)
 *
 * 情報取得に成功した場合、スタックテーブル内のデータが前詰めされます
 */
//==================================================================
BOOL GAMEBEACON_Stack_GetInfo(GAMEBEACON_INFO_TBL *stack_infotbl, GAMEBEACON_INFO *dest_info, u16 *time)
{
  int log_no;
  
  if(stack_infotbl->info[0].version_bit == 0){
    return FALSE;
  }
  
  *dest_info = stack_infotbl->info[0];
  *time = stack_infotbl->time[0];
  
  //スタックテーブルを前詰め
  for(log_no = 1; log_no < GAMEBEACON_INFO_TBL_MAX; log_no++){
    stack_infotbl->info[log_no - 1] = stack_infotbl->info[log_no];
    stack_infotbl->time[log_no - 1] = stack_infotbl->time[log_no];
  }
  GFL_STD_MemClear(&stack_infotbl->info[GAMEBEACON_INFO_TBL_MAX - 1], sizeof(GAMEBEACON_INFO));
  
  return TRUE;
}

//==================================================================
/**
 * スタックテーブルに残っている情報を通信バッファへ戻す
 *
 * @param   stack_infotbl		スタックテーブルへのポインタ
 *
 * スタックに残っている情報を対象に、GAMEBEACON_Get_UpdateLogNoで新着としてひっかかる状態にします
 * ※スタックテーブルを破棄する時に使用するものです
 */
//==================================================================
void GAMEBEACON_Stack_PutBack(const GAMEBEACON_INFO_TBL *stack_infotbl)
{
  int log_no;
  
  for(log_no = 0; log_no < GAMEBEACON_INFO_TBL_MAX; log_no++){
    if(stack_infotbl->info[log_no].version_bit == 0){
      return;
    }
    GAMEBEACON_Set_SearchUpdateFlag(&stack_infotbl->info[log_no]);
  }
}

//==================================================================
/**
 * ビーコンテーブルへ新しいビーコンを登録する
 *
 * @param   infotbl		  ビーコンテーブルへのポインタ
 * @param   set_info		登録するビーコン
 * @param   time        受信日時(上位8bit：時(0～23)、下位8bit：分(0～59))
 * @param   push_out    TRUE:データがいっぱいの場合、先頭のデータをところてん式に
 *                           押し出ししてセットします。
 *                      FALSE:データいっぱいの場合、登録不可として処理を終了します
 *
 * @retval  int		登録したログ位置(登録できなかった場合はGAMEBEACON_INFO_TBL_MAX)
 * 
 * 既にテーブルに存在しているユーザーの場合は上書き。
 * 新規ユーザーの場合はテーブルの最後に追加
 */
//==================================================================
int GAMEBEACON_InfoTbl_SetBeacon(GAMEBEACON_INFO_TBL *infotbl, const GAMEBEACON_INFO *set_info, u16 time, BOOL push_out)
{
  int log_no;
  
  for(log_no = 0; log_no < GAMEBEACON_INFO_TBL_MAX; log_no++){
    if(infotbl->info[log_no].version_bit == 0 
        || infotbl->info[log_no].trainer_id == set_info->trainer_id){
      infotbl->info[log_no] = *set_info;
      infotbl->time[log_no] = time;
      return log_no;
    }
  }
  
  if(push_out == TRUE){
    //スタックテーブルを前詰め
    MI_CpuCopy32( &infotbl->info[1], &infotbl->info[0], sizeof(GAMEBEACON_INFO)*GAMEBEACON_INFO_TBL_END);
    MI_CpuCopy16( &infotbl->time[1], &infotbl->time[0], sizeof(u16)*GAMEBEACON_INFO_TBL_END);
    infotbl->info[GAMEBEACON_INFO_TBL_END] = *set_info;
    infotbl->time[GAMEBEACON_INFO_TBL_END] = time;
    return GAMEBEACON_INFO_TBL_END;
  }
  return GAMEBEACON_INFO_TBL_MAX;
}

//==================================================================
/**
 * ビーコンテーブルからログ番号のビーコン情報を取得する
 *
 * @param   infotbl		  ビーコンテーブルへのポインタ
 * @param   dest_info		ログ情報代入先
 * @param   time        受信日時の代入先(上位8bit：時(0～23)、下位8bit：分(0～59))
 * @param   log_no		  ログ番号
 *
 * @retval  BOOL		    TRUE:ログ情報取得成功　　FALSE:取得失敗(データが無い)
 */
//==================================================================
BOOL GAMEBEACON_InfoTbl_GetBeacon(GAMEBEACON_INFO_TBL *infotbl, GAMEBEACON_INFO *dest_info, u16 *time, int log_no)
{
  GF_ASSERT(log_no < GAMEBEACON_INFO_TBL_MAX);
  
  if(infotbl->info[log_no].version_bit == 0){
    return FALSE;
  }
  *dest_info = infotbl->info[log_no];
  *time = infotbl->time[log_no];
  return TRUE;
}

//==================================================================
/**
 * リング式ビーコンテーブルのTopからのオフセットindexを配列への実参照indexに変換
 *
 * @param   infotbl		  ビーコンテーブルへのポインタ
 * @param   ofs         リングトップからのindexオフセット
 *
 * @retval  int		配列の実参照index
 */
//==================================================================
int GAMEBEACON_InfoTblRing_Ofs2Idx(GAMEBEACON_INFO_TBL *infotbl, int ofs )
{
  int top = infotbl->ring_top;

  if( ofs < 0 ){
    return ((top+ofs+GAMEBEACON_INFO_TBL_MAX)%GAMEBEACON_INFO_TBL_MAX);
  }else{
    return ((top+ofs)%GAMEBEACON_INFO_TBL_MAX);
  }
}

//==================================================================
/**
 * ビーコンテーブルへ新しいビーコンを登録する(リングバッファ方式)
 *
 * @param   infotbl		  ビーコンテーブルへのポインタ
 * @param   set_info		登録するビーコン
 * @param   time        受信日時(上位8bit：時(0～23)、下位8bit：分(0～59))
 *  @param  new_f       新着かどうか(NULL可)
 * @retval  int		登録したログ位置(登録できなかった場合はGAMEBEACON_INFO_TBL_MAX)
 * 
 * 既にテーブルに存在しているユーザーの場合は上書き。
 * 新規ユーザーの場合はテーブルの最後に追加
 */
//==================================================================
int GAMEBEACON_InfoTblRing_SetBeacon(GAMEBEACON_INFO_TBL *infotbl, const GAMEBEACON_INFO *set_info, u16 time, BOOL* new_f)
{
  int i, log_no;
  
  //逆にサーチ
  if(new_f != NULL){
    *new_f = 0;
  }
  for( i = 0;i < GAMEBEACON_INFO_TBL_MAX;i++){  //既存データチェック
    log_no = GAMEBEACON_InfoTblRing_Ofs2Idx(infotbl, i);

    if( infotbl->info[log_no].version_bit == 0 ){
      break;
    }
    if( infotbl->info[log_no].trainer_id == set_info->trainer_id ){

      infotbl->info[log_no] = *set_info;
      infotbl->time[log_no] = time;
      return i;
    }
  }
  //既存リストになければ新規追加
  if(new_f != NULL){
    *new_f = TRUE;
  }
  //一番古い一つを上書き
  log_no =  GAMEBEACON_InfoTblRing_Ofs2Idx(infotbl, -1);
  infotbl->info[log_no] = *set_info;
  infotbl->time[log_no] = time;

  //リングトップを更新
  infotbl->ring_top =  log_no;
  infotbl->entry_num++;
  return 0;
}

//==================================================================
/**
 * リング式ビーコンテーブルから指定オフセット位置のビーコン情報を取得する
 *
 * @param   infotbl		  ビーコンテーブルへのポインタ
 * @param   dest_info		ログ情報代入先
 * @param   time        受信日時の代入先(上位8bit：時(0～23)、下位8bit：分(0～59))
 * @param   ofs		      リングバッファ先頭からのindexオフセット
 *
 * @retval  BOOL		    TRUE:ログ情報取得成功　　FALSE:取得失敗(データが無い)
 */
//==================================================================
BOOL GAMEBEACON_InfoTblRing_GetBeacon(GAMEBEACON_INFO_TBL *infotbl, GAMEBEACON_INFO *dest_info, u16 *time, int ofs )
{
  int log_no;
  GF_ASSERT( ofs < GAMEBEACON_INFO_TBL_MAX);
  
  log_no = GAMEBEACON_InfoTblRing_Ofs2Idx( infotbl, ofs );
  if(infotbl->info[log_no].version_bit == 0){
    return FALSE;
  }
  *dest_info = infotbl->info[log_no];
  *time = infotbl->time[log_no];
  return TRUE;
}

//==================================================================
/**
 * リング式ビーコンテーブルにエントリーしているビーコン数を取得
 *
 * @param   infotbl	  ビーコンテーブルへのポインタ
 * @retval  int		    ビーコン登録数
 */
//==================================================================
int GAMEBEACON_InfoTblRing_GetEntryNum(GAMEBEACON_INFO_TBL *infotbl )
{
  return infotbl->entry_num;
}


//==================================================================
/**
 * GAMEBEACON_INFO構造体をAllocする
 *
 * @param   heap_id		
 *
 * @retval  GAMEBEACON_INFO *		
 */
//==================================================================
GAMEBEACON_INFO * GAMEBEACON_Alloc(HEAPID heap_id)
{
  return GFL_HEAP_AllocClearMemory(heap_id, sizeof(GAMEBEACON_INFO));
}

//==================================================================
/**
 * GAMEBEACON_INFO_TBL構造体をAllocする
 *
 * @param   heap_id		
 *
 * @retval  GAMEBEACON_INFO_TBL *		
 */
//==================================================================
GAMEBEACON_INFO_TBL * GAMEBEACON_InfoTbl_Alloc(HEAPID heap_id)
{
  GAMEBEACON_INFO_TBL* tbl = GFL_HEAP_AllocClearMemory(heap_id, sizeof(GAMEBEACON_INFO_TBL));
  tbl->ring_top = 0; 
  tbl->entry_num = 0;
  return tbl;
}



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * NPCデータかどうかチェック 
 * @param   info	ビーコン情報へのポインタ
 * @retval  TRUE  NPCデータ(=詳細情報なし)
 */
//==================================================================
BOOL GAMEBEACON_Check_NPC(const GAMEBEACON_INFO *info)
{
  return FALSE; //@todo なにがしかの条件でNPC判定をする
}

//==================================================================
/**
 * プレイヤー名をSTRBUFに取得する
 * @param   info		ビーコン情報へのポインタ
 * @retval  const STRCODE *		プレイヤー名へのポインタ
 */
//==================================================================
void GAMEBEACON_Get_PlayerNameToBuf(const GAMEBEACON_INFO *info, STRBUF* strbuf)
{
  STRCODE temp_name[BUFLEN_PERSON_NAME];
  
  //文字数がFullの場合はEOMがついていないので一時バッファにコピーして終端にEOMをつける
  GFL_STD_MemCopy16(info->name, temp_name, PERSON_NAME_SIZE*2);
  temp_name[PERSON_NAME_SIZE] = GFL_STR_GetEOMCode();
  
  GFL_STR_SetStringCode( strbuf, temp_name );
}

//==================================================================
/**
 * PM_VERSIONを取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  int	  	PM_VERSION
 */
//==================================================================
int GAMEBEACON_Get_PmVersion(const GAMEBEACON_INFO *info)
{
  return info->pm_version;
}

//==================================================================
/**
 * 住んでいる国を取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  int		  国番号
 */
//==================================================================
int GAMEBEACON_Get_Nation(const GAMEBEACON_INFO *info)
{
  return info->nation;
}

//==================================================================
/**
 * 住んでいる地域を取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  int		  地域番号
 */
//==================================================================
int GAMEBEACON_Get_Area(const GAMEBEACON_INFO *info)
{
  return info->area;
}

//==================================================================
/**
 * 自己紹介簡易会話を取得
 *
 * @param   info		    ビーコン情報へのポインタ
 * @param   dest_pms		代入先
 */
//==================================================================
void GAMEBEACON_Get_IntroductionPms(const GAMEBEACON_INFO *info, PMS_DATA *dest_pms)
{
  dest_pms->sentence_type = info->details.sentence_type;
  dest_pms->sentence_id = info->details.sentence_id;
  dest_pms->word[0] = info->details.word[0];
  dest_pms->word[1] = info->details.word[1];
}

//==================================================================
/**
 * 調査隊員ランクを取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  RESEARCH_TEAM_RANK		調査隊員ランク
 */
//==================================================================
RESEARCH_TEAM_RANK GAMEBEACON_Get_ResearchTeamRank(const GAMEBEACON_INFO *info)
{
  return info->research_team_rank;
}

//==================================================================
/**
 * お気に入りの色を取得する
 *
 * @param   dest_buf		色の代入先へのポインタ
 * @param   info        ビーコン情報へのポインタ
 */
//==================================================================
void GAMEBEACON_Get_FavoriteColor(GXRgb *dest_buf, const GAMEBEACON_INFO *info)
{
  *dest_buf = OS_GetFavoriteColor(info->favorite_color_index);
}

//==================================================================
/**
 * トレーナーIDを取得する
 * @param   info  ビーコン情報へのポインタ
 * @retval  u8		トレーナーの見た目
 */
//==================================================================
u32 GAMEBEACON_Get_TrainerID(const GAMEBEACON_INFO *info)
{
  return info->trainer_id;
}


//==================================================================
/**
 * トレーナーの見た目を取得する
 * @param   info  ビーコン情報へのポインタ
 * @retval  u8		トレーナーの見た目
 *
 * ＊性別も込みで、0-15のIDを返す
 */
//==================================================================
u8 GAMEBEACON_Get_TrainerView(const GAMEBEACON_INFO *info)
{
  return info->trainer_view;
}

//==================================================================
/**
 * 性別を取得する
 * @param   info  ビーコン情報へのポインタ
 * @retval  u8		性別
 */
//==================================================================
u8 GAMEBEACON_Get_Sex(const GAMEBEACON_INFO *info)
{
  return info->sex;
}

//==================================================================
/**
 * プレイ時間を取得する
 *
 * @param   info		    ビーコン情報へのポインタ
 * @param   dest_hour		代入先：時
 * @param   dest_min		代入先：分
 */
//==================================================================
void GAMEBEACON_Get_PlayTime(const GAMEBEACON_INFO *info, u16 *dest_hour, u16 *dest_min)
{
  *dest_hour = info->play_hour;
  *dest_min = info->play_min;
}

//==================================================================
/**
 * 発動しているGパワーIDを取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  GPOWER_ID		  GパワーID
 */
//==================================================================
GPOWER_ID GAMEBEACON_Get_GPowerID(const GAMEBEACON_INFO *info)
{
  return info->g_power_id;
}

//==================================================================
/**
 * 現在地を取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16		  現在地
 */
//==================================================================
u16 GAMEBEACON_Get_ZoneID(const GAMEBEACON_INFO *info)
{
  return info->zone_id;
}

//==================================================================
/**
 * タウンマップ用のROOT_ZONE_IDを取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16		  現在地
 */
//==================================================================
u16 GAMEBEACON_Get_TownmapRootZoneID(const GAMEBEACON_INFO *info)
{
  return info->townmap_root_zone_id;
}

//==================================================================
/**
 * お礼を受けた回数を取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  u32		  お礼を受けた回数
 */
//==================================================================
u32 GAMEBEACON_Get_ThanksRecvCount(const GAMEBEACON_INFO *info)
{
  return info->thanks_recv_count;
}

//==================================================================
/**
 * すれ違い人数を取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  u32		  すれ違い人数
 */
//==================================================================
u32 GAMEBEACON_Get_SuretigaiCount(const GAMEBEACON_INFO *info)
{
  return info->suretigai_count;
}

//==================================================================
/**
 * 自己紹介メッセージを取得
 *
 * @param   info		ビーコン情報へのポインタ
 * @param   dest		メッセージ代入先
 */
//==================================================================
void GAMEBEACON_Get_SelfIntroduction(const GAMEBEACON_INFO *info, STRBUF *dest)
{
  STRCODE temp_self[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN + EOM_SIZE];
  
  //文字数がFullの場合はEOMがついていないので一時バッファにコピーして終端にEOMをつける
  GFL_STD_MemCopy16(info->self_introduction, temp_self, GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN*2);
  temp_self[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN] = GFL_STR_GetEOMCode();
  
  GFL_STR_SetStringCode( dest, temp_self );
}

//==================================================================
/**
 * お礼メッセージを取得
 *
 * @param   info		ビーコン情報へのポインタ
 * @param   dest		メッセージ代入先
 */
//==================================================================
void GAMEBEACON_Get_ThankyouMessage(const GAMEBEACON_INFO *info, STRBUF *dest)
{
  GF_ASSERT(info->action.action_no == GAMEBEACON_ACTION_THANKYOU);
  GFL_STR_SetStringCode( dest, info->action.thankyou_message );
}

//==================================================================
/**
 * 詳細情報パラメータ取得：戦闘相手のトレーナーコード
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16		  トレーナーコード
 */
//==================================================================
u16 GAMEBEACON_Get_Details_BattleTrainer(const GAMEBEACON_INFO *info)
{
  switch(info->details.details_no){
  case GAMEBEACON_DETAILS_NO_BATTLE_TRAINER:
  case GAMEBEACON_DETAILS_NO_BATTLE_JIMLEADER:
  case GAMEBEACON_DETAILS_NO_BATTLE_BIGFOUR:
  case GAMEBEACON_DETAILS_NO_BATTLE_CHAMPION:
    return info->details.battle_trainer;
  }
  GF_ASSERT(0);
  return 0;
}

//==================================================================
/**
 * 詳細情報パラメータ取得：戦闘相手のポケモンNo
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16		  ポケモンNo
 */
//==================================================================
u16 GAMEBEACON_Get_Details_BattleMonsNo(const GAMEBEACON_INFO *info)
{
  switch(info->details.details_no){
  case GAMEBEACON_DETAILS_NO_BATTLE_WILD_POKE:        ///<野生ポケモンと対戦中
  case GAMEBEACON_DETAILS_NO_BATTLE_SP_POKE:          ///<特別なポケモンと対戦中
    return info->details.battle_monsno;
  }
  GF_ASSERT(0);
  return 0;
}

//==================================================================
/**
 * 行動パラメータ取得：行動No
 *
 * @param   info		ビーコン情報へのポインタ
 *
 * @retval  GAMEBEACON_ACTION_NULL		行動No(GAMEBEACON_ACTION_xxx)
 */
//==================================================================
GAMEBEACON_ACTION GAMEBEACON_Get_Action_ActionNo(const GAMEBEACON_INFO *info)
{
  return info->action.action_no;
}

//==================================================================
/**
 * 行動パラメータ取得：ポケモン番号
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16		ポケモン番号
 */
//==================================================================
u16 GAMEBEACON_Get_Action_Monsno(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_BATTLE_WILD_POKE_START:
  case GAMEBEACON_ACTION_BATTLE_WILD_POKE_VICTORY:
  case GAMEBEACON_ACTION_BATTLE_SP_POKE_START:
  case GAMEBEACON_ACTION_BATTLE_SP_POKE_VICTORY:
  case GAMEBEACON_ACTION_POKE_GET:
  case GAMEBEACON_ACTION_SP_POKE_GET:
  case GAMEBEACON_ACTION_POKE_EVOLUTION:
    return info->action.monsno;
  }
  GF_ASSERT(0);
  return info->action.monsno;
}

//==================================================================
/**
 * 行動パラメータ取得：アイテム番号
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16		  アイテム番号
 */
//==================================================================
u16 GAMEBEACON_Get_Action_ItemNo(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_SP_ITEM_GET:
    return info->action.itemno;
  }
  GF_ASSERT(0);
  return info->action.itemno;
}

//==================================================================
/**
 * 行動パラメータ取得：配布ポケモン番号
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16		ポケモン番号
 */
//==================================================================
u16 GAMEBEACON_Get_Action_DistributionMonsno(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_DISTRIBUTION_POKE:          ///<ポケモン配布中
    return info->action.distribution.monsno;
  }
  GF_ASSERT(0);
  return info->action.distribution.monsno;
}

//==================================================================
/**
 * 行動パラメータ取得：配布アイテム番号
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16		  アイテム番号
 */
//==================================================================
u16 GAMEBEACON_Get_Action_DistributionItemNo(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_DISTRIBUTION_ITEM:
    return info->action.distribution.itemno;
  }
  GF_ASSERT(0);
  return info->action.distribution.itemno;
}

//==================================================================
/**
 * 行動パラメータ取得：ポケモンのニックネーム
 * @param   info		ビーコン情報へのポインタ
 * @retval  const STRCODE *		ニックネームへのポインタ
 */
//==================================================================
void GAMEBEACON_Get_Action_Nickname(const GAMEBEACON_INFO *info, STRBUF *dest)
{
  STRCODE temp_name[BUFLEN_POKEMON_NAME];

  //文字数がFullの場合はEOMがついていないので一時バッファにコピーして終端にEOMをつける
  GFL_STD_MemCopy16(info->name, temp_name, MONS_NAME_SIZE*2);
  temp_name[MONS_NAME_SIZE] = GFL_STR_GetEOMCode();

  GFL_STR_SetStringCode( dest, temp_name );

  switch(info->action.action_no){
  case GAMEBEACON_ACTION_POKE_EVOLUTION:
  case GAMEBEACON_ACTION_POKE_LVUP:
    return;
  }
  GF_ASSERT(0);
  return;
}

//==================================================================
/**
 * 行動パラメータ取得：トレーナー番号
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16 		トレーナー番号
 */
//==================================================================
u16 GAMEBEACON_Get_Action_TrNo(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_BATTLE_TRAINER_START:
  case GAMEBEACON_ACTION_BATTLE_TRAINER_VICTORY:
  case GAMEBEACON_ACTION_BATTLE_LEADER_START:
  case GAMEBEACON_ACTION_BATTLE_LEADER_VICTORY:
  case GAMEBEACON_ACTION_BATTLE_BIGFOUR_START:
  case GAMEBEACON_ACTION_BATTLE_BIGFOUR_VICTORY:
  case GAMEBEACON_ACTION_BATTLE_CHAMPION_START:
  case GAMEBEACON_ACTION_BATTLE_CHAMPION_VICTORY:
    return info->action.tr_no;
  }
  return info->action.tr_no;
}

//==================================================================
/**
 * 行動パラメータ取得：お礼を受けた回数
 * @param   info	ビーコン情報へのポインタ
 * @retval  u32		お礼を受けた回数
 */
//==================================================================
u32 GAMEBEACON_Get_Action_ThankyouCount(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_THANKYOU_OVER:
    return info->action.thankyou_count;
  }
  GF_ASSERT(0);
  return info->action.thankyou_count;
}

//==================================================================
/**
 * 行動パラメータ取得：経過時間
 * @param   info	ビーコン情報へのポインタ
 * @retval  u32		経過時間(時)
 */
//==================================================================
u32 GAMEBEACON_Get_Action_Hour(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_PLAYTIME:
    return info->action.hour;
  }
  GF_ASSERT(0);
  return info->action.hour;
}
