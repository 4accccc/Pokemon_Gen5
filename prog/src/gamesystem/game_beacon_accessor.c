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
#include "gamesystem/g_power.h"
#include "net_app\union\union_beacon_tool.h"

#include "app/townmap_data.h"
#include "arc/fieldmap/zone_id.h"
#include "tr_tool/trno_def.h"
#include "waza_tool/wazano_def.h"
#include "poke_tool/monsno_def.h"
#include "item/itemsym.h"
#include "savedata/th_rank_def.h"
#include "app/research_radar/question_id.h"
#include "field/research_team_def.h"

enum{
 BEACON_WSET_DEFAULT,     //デフォルト(トレーナー名)  
 BEACON_WSET_TRNAME,      //対戦相手名
 BEACON_WSET_MONSNAME,    //ポケモン種族名
 BEACON_WSET_NICKNAME,    //ポケモンニックネーム
 BEACON_WSET_POKE_W,      //ポケモン種族名とニックネーム
 BEACON_WSET_ITEM,        //アイテム名
 BEACON_WSET_PTIME,       //プレイタイム
 BEACON_WSET_THANKS,      //御礼回数
 BEACON_WSET_HAIHU_MONS,  //配布モンスター名
 BEACON_WSET_HAIHU_ITEM,  //配布アイテム名
 BEACON_WSET_WAZA,        //技名
 BEACON_WSET_VICTORY,    //サブウェイ挑戦中の連勝数1-7
 BEACON_WSET_TH_RANK,  //トライアルハウスランク
 BEACON_WSET_GPOWER,      //Gパワー名
 BEACON_WSET_FREEWORD,    //フリーワード8文字
 BEACON_WSET_HAIHU_GPOWER,    //配布Gパワー名
 BEACON_WSET_MAX,
};

///ビーコン内容wordset用テンポラリバッファ長定義
#define BUFLEN_BEACON_WORDSET_TMP (16*2+EOM_SIZE)

/*
 *  @brief  GAMEBEACON_ACTION型の並びと同一である必要があります
 */
static const u8 DATA_BeaconDataType[GAMEBEACON_ACTION_MAX][2] = {
  BEACON_WSET_DEFAULT,	  BEACON_ICON_HELLO,	    ///<GAMEBEACON_ACTION_NULL  ///<データ無し 0
  		
  BEACON_WSET_DEFAULT,	  BEACON_ICON_HELLO,	    ///<SEARCH, XXさんをサーチしました！ 1
  BEACON_WSET_MONSNAME,	  BEACON_ICON_BTL_START,	///<BATTLE_WILD_POKE_START 野生のポケモンと対戦を開始しました！2
  BEACON_WSET_MONSNAME,	  BEACON_ICON_BTL_WIN,	  ///<BATTLE_WILD_POKE_VICTORY 野生のポケモンに勝利しました！3
  BEACON_WSET_MONSNAME,	  BEACON_ICON_BTL_START,	///<BATTLE_SP_POKE_START 特別なポケモンと対戦を開始しました！ 4
  BEACON_WSET_MONSNAME,	  BEACON_ICON_BTL_WIN,	  ///<BATTLE_SP_POKE_VICTORY 特別なポケモンに勝利しました！5
  BEACON_WSET_TRNAME,	    BEACON_ICON_BTL_START,	///<BATTLE_TRAINER_START トレーナーと対戦を開始しました！6
  BEACON_WSET_TRNAME,	    BEACON_ICON_BTL_WIN,	  ///<BATTLE_TRAINER_VICTORY トレーナーに勝利しました！7
  BEACON_WSET_TRNAME,	    BEACON_ICON_BTL_START,	///<BATTLE_LEADER_START  ジムリーダーと対戦を開始しました！ 8
  BEACON_WSET_TRNAME,	    BEACON_ICON_BTL_WIN,	  ///<BATTLE_LEADER_VICTORY ジムリーダーに勝利しました！9
  BEACON_WSET_TRNAME,	    BEACON_ICON_BTL_START,	///<BATTLE_BIGFOUR_START  四天王と対戦を開始しました！10
  BEACON_WSET_TRNAME,	    BEACON_ICON_BTL_WIN,	  ///<BATTLE_BIGFOUR_VICTORY 四天王に勝利しました！11
  BEACON_WSET_TRNAME,	    BEACON_ICON_BTL_START,	///<BATTLE_CHAMPION_START チャンピオンと対戦を開始しました！12
  BEACON_WSET_TRNAME,	    BEACON_ICON_BTL_WIN,	  ///<BATTLE_CHAMPION_VICTORY チャンピオンに勝利しました！13
  BEACON_WSET_MONSNAME,	  BEACON_ICON_POKE_GET,	  ///<POKE_GET ポケモン捕獲 14
  BEACON_WSET_MONSNAME,	  BEACON_ICON_POKE_GET,	  ///<SP_POKE_GET 特別なポケモン捕獲 15
  BEACON_WSET_NICKNAME,	  BEACON_ICON_POKE_LVUP,	///<POKE_LVUP ポケモンレベルアップ 16
  BEACON_WSET_POKE_W, 	  BEACON_ICON_POKE_SHINKA,///<POKE_EVOLUTION ポケモン進化 17
  BEACON_WSET_DEFAULT,	  BEACON_ICON_GPOWER,	    ///<GPOWER Gパワー発動 18
  BEACON_WSET_ITEM,	      BEACON_ICON_ITEM,	      ///<SP_ITEM_GET 貴重品ゲット 19
  BEACON_WSET_PTIME,	    BEACON_ICON_SPECIAL,	  ///<PLAYTIME  一定のプレイ時間を越えた 20
  BEACON_WSET_DEFAULT,	  BEACON_ICON_SPECIAL,	  ///<ZUKAN_COMPLETE 図鑑完成 21
  BEACON_WSET_THANKS,	    BEACON_ICON_SPECIAL,	  ///<THANKYOU_OVER お礼を受けた回数が規定数を超えた 22
  BEACON_WSET_DEFAULT,	  BEACON_ICON_UNION,	    ///<UNION_IN ユニオンルームに入った 23
  BEACON_WSET_DEFAULT,	  BEACON_ICON_MSG,	      ///<THANKYOU ありがとう！24
  BEACON_WSET_HAIHU_MONS,	BEACON_ICON_SPECIAL,	  ///<DISTRIBUTION_POKE ポケモン配布中 25
  BEACON_WSET_HAIHU_ITEM,	BEACON_ICON_SPECIAL,	  ///<DISTRIBUTION_ITEM アイテム配布中 26
  BEACON_WSET_DEFAULT,	  BEACON_ICON_SPECIAL,	  ///<DISTRIBUTION_ETC  その他配布中 27
  BEACON_WSET_NICKNAME,	  BEACON_ICON_BTL_START,	///<CRITICAL_HIT 急所に攻撃をあてた 28
  BEACON_WSET_NICKNAME,	  BEACON_ICON_BTL_START,	///<CRITICAL_DAMAGE 急所に攻撃を受けた 29
  BEACON_WSET_DEFAULT,	  BEACON_ICON_BTL_START,  ///<ESCAPE 戦闘から逃げ出した 30
  BEACON_WSET_NICKNAME,	  BEACON_ICON_BTL_WIN,	  ///<HP_LITTLE HPが残り少ない 31
  BEACON_WSET_NICKNAME,	  BEACON_ICON_BTL_WIN,	  ///<PP_LITTLE PPが残り少ない 32
  BEACON_WSET_NICKNAME,	  BEACON_ICON_BTL_WIN,	  ///<DYING 先頭のポケモンが瀕死 33
  BEACON_WSET_NICKNAME,	  BEACON_ICON_BTL_WIN,	  ///<STATE_IS_ABNORMAL 先頭のポケモンが状態異常 34
  BEACON_WSET_ITEM,	      BEACON_ICON_ITEM,	      ///<USE_ITEM アイテムを使用 35
  BEACON_WSET_WAZA,	      BEACON_ICON_INFO,	      ///<FIELD_SKILL フィールド技を使用 36
  BEACON_WSET_DEFAULT,	  BEACON_ICON_INFO,	      ///<SODATEYA_EGG 育て屋から卵を引き取った 37
  BEACON_WSET_MONSNAME,	  BEACON_ICON_INFO,	      ///<EGG_HATCH タマゴが孵化した 38
  BEACON_WSET_DEFAULT,	  BEACON_ICON_INFO,	      ///<SHOPING 買い物中 39
  BEACON_WSET_DEFAULT,	  BEACON_ICON_BTL_START,	///<SUBWAY バトルサブウェイ挑戦中 40
  BEACON_WSET_VICTORY,	  BEACON_ICON_BTL_WIN,	  ///<SUBWAY_STRAIGHT_VICTORIES バトルサブウェイ連勝中 41
  BEACON_WSET_DEFAULT,	  BEACON_ICON_BTL_WIN,	  ///<SUBWAY_TROPHY_GET バトルサブウェイトロフィーを貰った 42
  BEACON_WSET_DEFAULT,	  BEACON_ICON_BTL_START,	///<TRIALHOUSE トライアルハウスに挑戦中 43
  BEACON_WSET_TH_RANK,	  BEACON_ICON_BTL_WIN,	  ///<TRIALHOUSE_RANK トライアルハウスでランク確定 44
  BEACON_WSET_DEFAULT,	  BEACON_ICON_INFO,	      ///<FERRIS_WHEEL 観覧車に乗った 45
  BEACON_WSET_DEFAULT,	  BEACON_ICON_INFO,	      ///<POKESHIFTER ポケシフターに入った 46
  BEACON_WSET_NICKNAME,	  BEACON_ICON_INFO,	      ///<MUSICAL ミュージカル挑戦中 47
  BEACON_WSET_GPOWER,	    BEACON_ICON_GPOWER,	    ///<OTHER_GPOWER_USE 他人のGパワーを使用 48
  BEACON_WSET_FREEWORD,	  BEACON_ICON_MSG,	      ///<FREEWORD 一言メッセージ 49
  BEACON_WSET_HAIHU_GPOWER,	  BEACON_ICON_GPOWER,     ///<DISTRIBUTION_GPOWER Gパワー配布中 50 
};

typedef BOOL (*BEACON_INFO_ERROR_CHECK_FUNC)(const GAMEBEACON_INFO* info );

static BOOL errchk_action_default(const GAMEBEACON_INFO* info );
static BOOL errchk_action_trainer_battle(const GAMEBEACON_INFO* info );
static BOOL errchk_action_poke_monsno(const GAMEBEACON_INFO* info );
static BOOL errchk_action_poke_nickname(const GAMEBEACON_INFO* info );
static BOOL errchk_action_poke_double(const GAMEBEACON_INFO* info );
static BOOL errchk_action_itemno(const GAMEBEACON_INFO* info );
static BOOL errchk_action_playtime(const GAMEBEACON_INFO* info );
static BOOL errchk_action_thanks_count(const GAMEBEACON_INFO* info );
static BOOL errchk_action_haifu_mons(const GAMEBEACON_INFO* info );
static BOOL errchk_action_haifu_item(const GAMEBEACON_INFO* info );
static BOOL errchk_action_waza(const GAMEBEACON_INFO* info );
static BOOL errchk_action_victory(const GAMEBEACON_INFO* info );
static BOOL errchk_action_th_rank(const GAMEBEACON_INFO* info );
static BOOL errchk_action_gpower(const GAMEBEACON_INFO* info );
static BOOL errchk_action_haifu_gpower(const GAMEBEACON_INFO* info );

static const BEACON_INFO_ERROR_CHECK_FUNC DATA_ErrorCheckFuncTbl[BEACON_WSET_MAX] = {
  errchk_action_default,
  errchk_action_trainer_battle,
  errchk_action_poke_monsno,
  errchk_action_poke_nickname,
  errchk_action_poke_double,
  errchk_action_itemno,
  errchk_action_playtime,
  errchk_action_thanks_count,
  errchk_action_haifu_mons,
  errchk_action_haifu_item,
  errchk_action_waza,
  errchk_action_victory,
  errchk_action_th_rank,
  errchk_action_gpower,
  errchk_action_default,
  errchk_action_haifu_gpower,
};

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
    update_info = GAMEBEACON_Get_BeaconLogDirect(update_logno);
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
  if( infotbl->entry_num < GAMEBEACON_INFO_TBL_MAX ){
    infotbl->entry_num++;
  }
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
 * @brief GAMEBEACON_INFO_TBL構造体をAllocする
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

//==================================================================
/**
 * @brief GAMEBEACON_INFO_TBL構造体を全件クリア
 */
//==================================================================
void GAMEBEACON_InfoTbl_Clear( GAMEBEACON_INFO_TBL* infoTbl )
{
  GFL_STD_MemClear( infoTbl, sizeof(GAMEBEACON_INFO_TBL));
}



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 基本データエラーチェック 
 * @param   info	ビーコン情報へのポインタ
 * @retval  TRUE  エラーを検出
 */
//==================================================================
BOOL GAMEBEACON_Check_Error(const GAMEBEACON_INFO *info)
{
  //アクションNoチェック
  if( info->action.action_no == GAMEBEACON_ACTION_NULL ||
      info->action.action_no >= GAMEBEACON_ACTION_MAX ) {
    return TRUE;
  }
  //詳細コードナンバー
  if( info->details.details_no >= GAMEBEACON_DETAILS_NO_MAX ){
    return TRUE;
  }
  //調査隊員ランク
  if( info->research_team_rank >= RESEARCH_TEAM_RANK_MAX ){
    return TRUE;
  }
  //プレイ時間
  if( info->play_hour > 999 || info->play_min > 59 ){
    return TRUE;
  }
  //GパワーIDチェック
  if( GPOWER_IsErrorID( info->g_power_id ) ) {
    return TRUE;
  }
  //ゾーンIDチェック
  if( info->zone_id >= ZONE_ID_MAX || info->townmap_root_zone_id >= ZONE_ID_MAX ){
    return TRUE;
  }
  //すれ違い人数＆御礼カウント
  if( info->suretigai_count > 99999 || info->thanks_recv_count > 99999 ){
    return TRUE;
  }
  //アクション詳細データチェック
  if( (DATA_ErrorCheckFuncTbl[GAMEBEACON_GetActionDataType(info->action.action_no )])(info) ){
    return TRUE;
  }
  return FALSE;
}


//==================================================================
/**
 * NPCデータかどうかチェック 
 * @param   info	ビーコン情報へのポインタ
 * @retval  TRUE  NPCデータ(=詳細情報なし)
 */
//==================================================================
BOOL GAMEBEACON_Check_NPC(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_DISTRIBUTION_POKE:       ///<ポケモン配布中
  case GAMEBEACON_ACTION_DISTRIBUTION_ITEM:       ///<アイテム配布中
  case GAMEBEACON_ACTION_DISTRIBUTION_ETC:        ///<その他配布中
  case GAMEBEACON_ACTION_DISTRIBUTION_GPOWER:     ///<Gパワー配布中
    return TRUE;
  }
  return FALSE;
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
  GFL_STR_SetStringCodeOrderLength( strbuf, info->name, PERSON_NAME_SIZE+EOM_SIZE );
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
 * PM_LANGを取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  int	  	PM_LANG
 */
//==================================================================
int GAMEBEACON_Get_PmLanguage(const GAMEBEACON_INFO *info)
{
  return info->language;
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
 * 仕事IDを取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  u8		  仕事ID (設定されていない場合はGAMEBEACON_DETAILS_JOB_NULL)
 */
//==================================================================
u8 GAMEBEACON_Get_Job(const GAMEBEACON_INFO *info)
{
  u32 answer;
  
  answer = QuestionnaireAnswer_ReadBit(&info->question_answer, QUESTION_ID_JOB);
  if(answer == 0){
    return GAMEBEACON_DETAILS_JOB_NULL;
  }
  return answer;
}

//==================================================================
/**
 * 趣味IDを取得
 * @param   info		ビーコン情報へのポインタ
 * @retval  u8		  趣味ID (設定されていない場合はGAMEBEACON_DETAILS_HOBBY_NULL)
 */
//==================================================================
u8 GAMEBEACON_Get_Hobby(const GAMEBEACON_INFO *info)
{
  u32 answer;
  
  answer = QuestionnaireAnswer_ReadBit(&info->question_answer, QUESTION_ID_HOBBY);
  if(answer == 0){
    return GAMEBEACON_DETAILS_HOBBY_NULL;
  }
  return answer;
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
 * @retval  u8		調査隊員ランク ( RESEARCH_TEAM_RANK_xxxx )
 */
//==================================================================
u8 GAMEBEACON_Get_ResearchTeamRank(const GAMEBEACON_INFO *info)
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

//----------------------------------------------------------------------------
/**
 * お気に入りの色インデックスを取得する
 *
 * @param   info        ビーコン情報へのポインタ
 *
 *	@return 色インデックス  0-15
 */
//-----------------------------------------------------------------------------
u8 GAMEBEACON_Get_FavoriteColorIndex(const GAMEBEACON_INFO *info)
{
  return info->favorite_color_index;
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
  if(info->sex == PM_MALE){
    return info->trainer_view;
  }
  return info->trainer_view + UNION_VIEW_INDEX_WOMAN_START;
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
  GFL_STR_ClearBuffer(dest);
  GFL_STR_SetStringCodeOrderLength( dest,
    info->self_introduction, GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN+EOM_SIZE );
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
  if( info->action.action_no != GAMEBEACON_ACTION_THANKYOU ){
    GF_ASSERT(0);
    GFL_STR_ClearBuffer(dest);
    return;
  }
  GFL_STR_SetStringCode( dest, info->action.thanks.thankyou_message );
}

//==================================================================
/**
 * 一言メッセージを取得
 *
 * @param   info		ビーコン情報へのポインタ
 * @param   dest		メッセージ代入先
 */
//==================================================================
void GAMEBEACON_Get_FreeWordMessage(const GAMEBEACON_INFO *info, STRBUF *dest)
{
  if( info->action.action_no != GAMEBEACON_ACTION_FREEWORD ){
    GF_ASSERT(0);
    GFL_STR_ClearBuffer(dest);
    return;
  }
  GFL_STR_SetStringCode( dest, info->action.freeword_message );
}

//==================================================================
/**
 * 詳細情報パラメータ取得：詳細情報No
 * @param   info		ビーコン情報へのポインタ
 * @retval  GAMEBEACON_DETAILS_NO 詳細No
 */
//==================================================================
GAMEBEACON_DETAILS_NO GAMEBEACON_Get_Details_DetailsNo(const GAMEBEACON_INFO *info)
{
  if( info->details.details_no >= GAMEBEACON_DETAILS_NO_MAX ){
    GF_ASSERT(0);
    return GAMEBEACON_DETAILS_NO_WALK;
  }
  return info->details.details_no;
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
  u32 tr_no = info->details.battle_trainer;

  switch(info->details.details_no){
  case GAMEBEACON_DETAILS_NO_BATTLE_TRAINER:
  case GAMEBEACON_DETAILS_NO_BATTLE_JYMLEADER:
  case GAMEBEACON_DETAILS_NO_BATTLE_SP_TRAINER:
    if( tr_no == TRID_NULL || tr_no >= TRID_MAX ){
      GF_ASSERT(0);
      tr_no = 1;
    }
    return tr_no;
  }
  GF_ASSERT(0);
  return 1;
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
  u32 monsno = info->details.battle_monsno;
  switch(info->details.details_no){
  case GAMEBEACON_DETAILS_NO_BATTLE_WILD_POKE:        ///<野生ポケモンと対戦中
  case GAMEBEACON_DETAILS_NO_BATTLE_SP_POKE:          ///<特別なポケモンと対戦中
    if( monsno == 0 || monsno > MONSNO_END ){
      GF_ASSERT(0);
      monsno = MONSNO_MINEZUMI; //当たり障りの無いNo
    }
    return monsno;
  }
  GF_ASSERT(0);
  return MONSNO_MINEZUMI; //当たり障りの無いNo
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
  if( info->action.action_no == GAMEBEACON_ACTION_NULL ||
      info->action.action_no >= GAMEBEACON_ACTION_MAX ) {
    GF_ASSERT(0);
    return GAMEBEACON_ACTION_SEARCH;
  }
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
  case GAMEBEACON_ACTION_EGG_HATCH:
    return info->action.monsno;
  }
  GF_ASSERT_MSG(0,"ano = %d\n",info->action.action_no);
  return MONSNO_MINEZUMI; //当たり障りの無いNo
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
  case GAMEBEACON_ACTION_USE_ITEM:
    return info->action.itemno;
  }
  GF_ASSERT(0);
  return ITEM_KIZUGUSURI; //当たり障りないNo
}

//==================================================================
/**
 * 行動パラメータ取得：技番号
 * @param   info		ビーコン情報へのポインタ
 * @retval  u16		  技番号
 */
//==================================================================
u16 GAMEBEACON_Get_Action_WazaNo(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_FIELD_SKILL:
    return info->action.wazano;
  }
  GF_ASSERT(0);
  return WAZANO_HATAKU;
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
  return MONSNO_MINEZUMI; //当たり障りの無いNo
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
  return ITEM_KIZUGUSURI; //当たり障りないNo
}

//==================================================================
/**
 * 行動パラメータ取得：配布Gパワー
 * @param   info		    ビーコン情報へのポインタ
 * @retval  GPOWER_ID		GパワーID
 */
//==================================================================
GPOWER_ID GAMEBEACON_Get_Action_DistributionGPower(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_DISTRIBUTION_GPOWER:
    return info->action.distribution.gpower_id;
  }
  GF_ASSERT(0);
  return GPOWER_ID_SOUGUU_INC_A;
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
  GFL_STR_ClearBuffer(dest);

  GFL_STR_SetStringCodeOrderLength( dest,
    info->action.normal.nickname, MONS_NAME_SIZE+EOM_SIZE );

  switch(info->action.action_no){
  case GAMEBEACON_ACTION_POKE_EVOLUTION:
  case GAMEBEACON_ACTION_POKE_LVUP:
  case GAMEBEACON_ACTION_CRITICAL_HIT:
  case GAMEBEACON_ACTION_CRITICAL_DAMAGE:
  case GAMEBEACON_ACTION_HP_LITTLE:
  case GAMEBEACON_ACTION_PP_LITTLE:
  case GAMEBEACON_ACTION_DYING:
  case GAMEBEACON_ACTION_STATE_IS_ABNORMAL:
  case GAMEBEACON_ACTION_SODATEYA_EGG:
  case GAMEBEACON_ACTION_EGG_HATCH:
  case GAMEBEACON_ACTION_MUSICAL:
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
  case GAMEBEACON_ACTION_BATTLE_SP_TRAINER_START:
  case GAMEBEACON_ACTION_BATTLE_SP_TRAINER_VICTORY:
    return info->action.tr_no;
  }
  return 1;
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
  return 1;
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
  return 1;
}

//==================================================================
/**
 * 行動パラメータ取得：サブウェイラウンド数(1～7)
 * @param   info	ビーコン情報へのポインタ
 * @retval  u32		サブウェイで勝ち進んだラウンド数
 *
 * <連勝数>とは違うので注意。7戦1セットの中の何勝目かを返します
 */
//==================================================================
u32 GAMEBEACON_Get_Action_VictoryCount(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_SUBWAY_STRAIGHT_VICTORIES:
    return info->action.victory_count;
  }
  GF_ASSERT(0);
  return 1;
}

//==================================================================
/**
 * 行動パラメータ取得：トライアルハウスランク
 * @param   info	ビーコン情報へのポインタ
 * @retval  u8		TH_RANK_xxx
 */
//==================================================================
u8 GAMEBEACON_Get_Action_TrialHouseRank(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_TRIALHOUSE_RANK:
    return info->action.trial_house_rank;
  }
  GF_ASSERT(0);
  return TH_RANK_BEGINNER;
}

//==================================================================
/**
 * 行動パラメータ取得：Gパワー
 * @param   info	ビーコン情報へのポインタ
 * @retval  u16		GパワーID
 */
//==================================================================
u16 GAMEBEACON_Get_Action_GPowerID(const GAMEBEACON_INFO *info)
{
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_OTHER_GPOWER_USE:
    return info->action.gpower_id;
  }
  GF_ASSERT(0);
  return GPOWER_ID_SOUGUU_INC_A;
}

//==================================================================
/**
 * ビーコン情報のデータタイプを取得
 *
 * @param   action          ビーコンのアクションNo
 */
//==================================================================
u8 GAMEBEACON_GetActionDataType( GAMEBEACON_ACTION action )
{
  return DATA_BeaconDataType[action][0];
}
//==================================================================
/**
 * ビーコン情報のアイコンデータタイプを取得
 *
 * @param   action          ビーコンのアクションNo
 */
//==================================================================
u8 GAMEBEACON_GetActionDataIconType( GAMEBEACON_ACTION action )
{
  return DATA_BeaconDataType[action][1];
}

//==================================================================
/**
 * ビーコン情報の内容をWORDSETする
 *
 * @param   info		        対象のビーコン情報へのポインタ
 * @param   wordset		
 * @param   temp_heap_id		内部でテンポラリとして使用するヒープID
 */
//==================================================================
void GAMEBEACON_InfoWordset(const GAMEBEACON_INFO *info, WORDSET *wordset, HEAPID temp_heap_id)
{
  u8 type;
  STRBUF *strbuf;

  WORDSET_ClearAllBuffer( wordset );
  
  strbuf = GFL_STR_CreateBuffer( BUFLEN_BEACON_WORDSET_TMP , temp_heap_id);
  if( strbuf == NULL ){
    return;
  }
  //トレーナー名展開(デフォルト)
  GAMEBEACON_Get_PlayerNameToBuf(info, strbuf);
  WORDSET_RegisterWord( wordset, 0, strbuf, 0, TRUE, PM_LANG);

  {
    GAMEBEACON_ACTION action = GAMEBEACON_Get_Action_ActionNo(info);
    type = GAMEBEACON_GetActionDataType( action );
  }

  switch( type ){
  case BEACON_WSET_TRNAME:
    {
      u16 tr_no = GAMEBEACON_Get_Action_TrNo(info);
      WORDSET_RegisterTrTypeName_byTrID( wordset, 1, tr_no );
      WORDSET_RegisterTrainerName( wordset, 2, tr_no );
    }
    break;
  case BEACON_WSET_MONSNAME:
  case BEACON_WSET_POKE_W:
    WORDSET_RegisterPokeMonsNameNo( wordset, 1,GAMEBEACON_Get_Action_Monsno(info));
    if( type == BEACON_WSET_MONSNAME ){
      break;
    }
    //ブレイクスルー
  case BEACON_WSET_NICKNAME:
    GAMEBEACON_Get_Action_Nickname( info, strbuf );
    WORDSET_RegisterWord(wordset, 2, strbuf, 0, TRUE, PM_LANG);
    break;
  case BEACON_WSET_ITEM:
    WORDSET_RegisterItemName( wordset, 1, GAMEBEACON_Get_Action_ItemNo(info) );
    break;
  case BEACON_WSET_PTIME:
    WORDSET_RegisterNumber( wordset, 1, GAMEBEACON_Get_Action_Hour(info), 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    break;
  case BEACON_WSET_THANKS:
    WORDSET_RegisterNumber( wordset, 1, GAMEBEACON_Get_Action_ThankyouCount(info), 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    break;
  case BEACON_WSET_HAIHU_MONS:
    WORDSET_RegisterPokeMonsNameNo( wordset, 1, GAMEBEACON_Get_Action_DistributionMonsno(info));
    break;
  case BEACON_WSET_HAIHU_ITEM:
    WORDSET_RegisterItemName( wordset, 1, GAMEBEACON_Get_Action_DistributionItemNo(info));
    break;
  case BEACON_WSET_WAZA:
    WORDSET_RegisterWazaName( wordset, 1, GAMEBEACON_Get_Action_WazaNo(info));
    break;
  case BEACON_WSET_VICTORY:
    WORDSET_RegisterNumber( wordset, 1, GAMEBEACON_Get_Action_VictoryCount(info), 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    break;
  case BEACON_WSET_TH_RANK:
    WORDSET_RegisterTrialHouseRank( wordset, 1, GAMEBEACON_Get_Action_TrialHouseRank(info) );
    break;
  case BEACON_WSET_GPOWER:
    WORDSET_RegisterGPowerName( wordset, 1, GAMEBEACON_Get_Action_GPowerID(info) );
    break;
  case BEACON_WSET_FREEWORD:
    GAMEBEACON_Get_FreeWordMessage( info, strbuf );
    WORDSET_RegisterWord( wordset, 1, strbuf, GAMEBEACON_Get_Sex(info), TRUE, PM_LANG);
    break;
  case BEACON_WSET_HAIHU_GPOWER:
  case BEACON_WSET_DEFAULT:
    break;
  }
  GFL_STR_DeleteBuffer(strbuf);
}


///////////////////////////////////////////////////////////////////////////////////
/*
 *  ローカル関数
 */
///////////////////////////////////////////////////////////////////////////////////

//エラーチェック デフォルトタイプ
static BOOL errchk_action_default(const GAMEBEACON_INFO* info )
{
  return FALSE;
}
//エラーチェック トレーナー対戦タイプ
static BOOL errchk_action_trainer_battle(const GAMEBEACON_INFO* info )
{
  if( info->action.tr_no == TRID_NULL || info->action.tr_no >= TRID_MAX ){
    GF_ASSERT_MSG(0,"%d\n",info->action.tr_no );
    return TRUE;
  }
  return FALSE;
}
//エラーチェック ポケモン種族名チェックタイプ
static BOOL errchk_action_poke_monsno(const GAMEBEACON_INFO* info )
{
  if( info->action.monsno == 0 || 
      info->action.monsno > MONSNO_END ){
    GF_ASSERT_MSG(0,"%d\n",info->action.monsno);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック ポケモンニックネームチェックタイプ
static BOOL errchk_action_poke_nickname(const GAMEBEACON_INFO* info )
{
  return FALSE;
}
//エラーチェック 種族名とポケモンニックネームチェックタイプ
static BOOL errchk_action_poke_double(const GAMEBEACON_INFO* info )
{
  if( info->action.monsno == 0 || 
      info->action.monsno > MONSNO_END ){
    GF_ASSERT_MSG(0,"%d\n",info->action.monsno);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック アイテムNoチェックタイプ
static BOOL errchk_action_itemno(const GAMEBEACON_INFO* info )
{
  if( info->action.itemno == ITEM_DUMMY_DATA || info->action.itemno > ITEM_DATA_MAX ){
    GF_ASSERT_MSG(0,"%d\n",info->action.itemno);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック プレイタイムタイプ
static BOOL errchk_action_playtime(const GAMEBEACON_INFO* info )
{
  if( info->play_hour > 999 | info->play_min > 59 ){
    GF_ASSERT_MSG(0,"hour %d, min %d\n",info->play_hour, info->play_min);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック 御礼回数タイプ
static BOOL errchk_action_thanks_count(const GAMEBEACON_INFO* info )
{
  if( info->action.thankyou_count > 99999 ){
    GF_ASSERT_MSG(0,"%d\n",info->action.thankyou_count);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック 配布モンスター名タイプ
static BOOL errchk_action_haifu_mons(const GAMEBEACON_INFO* info )
{
  if( info->action.distribution.monsno  == 0 ||
      info->action.distribution.monsno > MONSNO_END ){
    GF_ASSERT_MSG(0,"%d\n",info->action.distribution.monsno);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック 配布アイテムタイプ
static BOOL errchk_action_haifu_item(const GAMEBEACON_INFO* info )
{
  if( info->action.distribution.itemno  == ITEM_DUMMY_DATA ||
      info->action.distribution.itemno > ITEM_DATA_MAX ){
    GF_ASSERT_MSG(0,"%d\n",info->action.distribution.itemno);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック 技名タイプ
static BOOL errchk_action_waza(const GAMEBEACON_INFO* info )
{
  if( info->action.wazano  == WAZANO_NULL ||
      info->action.wazano >= WAZANO_MAX ){
    GF_ASSERT_MSG(0,"%d\n",info->action.wazano);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック サブウェイ連勝数タイプ 
static BOOL errchk_action_victory(const GAMEBEACON_INFO* info )
{
  if( info->action.victory_count ==0 || info->action.victory_count > 7 ){
    GF_ASSERT_MSG(0,"%d\n",info->action.victory_count);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック トライアルハウスランクタイプ 
static BOOL errchk_action_th_rank(const GAMEBEACON_INFO* info )
{
  if( info->action.trial_house_rank > TH_RANK_MASTER){
    GF_ASSERT_MSG(0,"%d\n",info->action.trial_house_rank);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック GパワーIDタイプ 
static BOOL errchk_action_gpower(const GAMEBEACON_INFO* info )
{
  if( info->action.gpower_id >= GPOWER_ID_MAX ){
    GF_ASSERT_MSG(0,"%d\n",info->action.gpower_id);
    return TRUE;
  }
  return FALSE;
}
//エラーチェック 配布GパワーIDタイプ 
static BOOL errchk_action_haifu_gpower(const GAMEBEACON_INFO* info )
{
  if( info->action.distribution.gpower_id >= GPOWER_ID_MAX ){
    GF_ASSERT_MSG(0,"%d\n",info->action.distribution.gpower_id);
    return TRUE;
  }
  return FALSE;
}

