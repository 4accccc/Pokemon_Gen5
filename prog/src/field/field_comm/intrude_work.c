//==============================================================================
/**
 * @file    intrude_work.c
 * @brief   侵入システムワークに外部から触る為の関数群
 * @author  matsuda
 * @date    2009.10.10(土)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_comm.h"
#include "savedata/mystatus.h"
#include "system/net_err.h"
#include "net/network_define.h"
#include "field/intrude_comm.h"
#include "intrude_comm_command.h"
#include "net_app/union/comm_player.h"
#include "intrude_types.h"
#include "intrude_comm_command.h"
#include "intrude_main.h"
#include "intrude_work.h"
#include "intrude_mission.h"
#include "field/field_status.h"
#include "field/field_status_local.h"
#include "fieldmap/zone_id.h"
#include "field/zonedata.h"
#include "field/eventwork.h"
#include "../../../../resource/fldmapdata/flagwork/flag_define.h" //SYS_FLAG_


//==============================================================================
//  データ
//==============================================================================
#include "palace_zone_id_tbl.cdat"


//==================================================================
/**
 * 侵入システムとして常時通信を起動してもよい状態かチェックする
 *
 * @param   gsys		
 * @param   zone_id	
 *
 * @retval  BOOL		    TRUE:常時通信を起動してもよい
 * @retval  BOOL		    TRUE:起動してはいけない
 */
//==================================================================
BOOL Intrude_Check_AlwaysBoot(GAMESYS_WORK *gsys)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  
  if(GAMEDATA_GetIntrudeReverseArea(gamedata) == TRUE){
    //裏フィールドにいて、通信エラーステータスが何か残っている場合は
    //侵入としてエラー処理、切断後のイベント処理などが残っているので
    //常時通信をさせない
    if(GameCommSys_GetLastStatus(game_comm) != GAME_COMM_LAST_STATUS_NULL){
      return FALSE;
    }
    
#if 0 //ハイリンクにいるもの同士が繋がる事は無くなったので不要になった 2010.06.07(月)
    //チュートリアルが完了していなくて、裏フィールドにいる場合は「子」にならないように
    //常時通信を起動してはいけない
    if(Intrude_CheckTutorialComplete(gamedata) == FALSE){
      return FALSE;
    }
#endif
  }
  

  return TRUE;
}

//==================================================================
/**
 * 侵入通信が正常に繋がっているか調べる
 *
 * @param   game_comm		
 *
 * @retval  INTRUDE_COMM_SYS_PTR	繋がっているなら intcomm
 * @retval  INTRUDE_COMM_SYS_PTR	繋がっていないなら NULL
 *
 * 通信相手の返事待ち、親の返事待ち、などしている時に、途中でエラーが発生していないか
 * 監視する処理を共通化。
 */
//==================================================================
INTRUDE_COMM_SYS_PTR Intrude_Check_CommConnect(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(game_comm);
  
  if(NetErr_App_CheckError() || GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_INVASION 
      || GameCommSys_CheckSystemWaiting(game_comm) == TRUE || intcomm == NULL){
    return NULL;
  }
  return intcomm;
}

//==================================================================
/**
 * ワープするプレイヤー相手を設定
 *
 * @param   game_comm		
 * @param   town_tblno		
 */
//==================================================================
void Intrude_SetWarpPlayerNetID(GAME_COMM_SYS_PTR game_comm, int net_id)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL){
    return;
  }
  
  intcomm->warp_player_netid = net_id;
}

//==================================================================
/**
 * ワープ先のプレイヤー相手を取得
 *
 * @param   game_comm		
 *
 * @retval  int		街番号(PALACE_TOWN_DATA_NULLの場合は無効)
 */
//==================================================================
int Intrude_GetWarpPlayerNetID(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL){
    return GAMEDATA_GetIntrudeMyID( GameCommSys_GetGameData(game_comm) );
  }
  return intcomm->warp_player_netid;
}

//==================================================================
/**
 * ワープする街番号を設定
 *
 * @param   game_comm		
 * @param   town_tblno		
 */
//==================================================================
void Intrude_SetWarpTown(GAME_COMM_SYS_PTR game_comm, int town_tblno)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL){
    return;
  }
  
  intcomm->warp_town_tblno = town_tblno;
}

//==================================================================
/**
 * ワープ先の街番号を取得
 *
 * @param   game_comm		
 *
 * @retval  int		街番号(PALACE_TOWN_DATA_NULLの場合は無効)
 */
//==================================================================
int Intrude_GetWarpTown(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL){
    return PALACE_TOWN_DATA_NULL;
  }
  return intcomm->warp_town_tblno;
}

//==================================================================
/**
 * ミッションバトル用の相手指定のタイミングコマンドが受信できているか確認します
 *
 * @param   intcomm		
 * @param   timing_bit	    同期番号
 * @param   target_netid		同期相手のNetID
 *
 * @retval  BOOL		TRUE:同期取り完了。　FALSE:同期待ち
 */
//==================================================================
BOOL Intrude_GetMissionBattleTimingNo(INTRUDE_COMM_SYS_PTR intcomm, u8 timing_bit, u8 target_netid)
{
  u8 recv_timing_no = intcomm->mission_battle_timing_bit;
  
  if(timing_bit & recv_timing_no){
    intcomm->mission_battle_timing_bit ^= 1;
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 指定したパレスエリアの占拠情報を取得する
 *
 * @param   intcomm		
 * @param   palace_area		  
 *
 * @retval  OCCUPY_INFO *		
 */
//==================================================================
OCCUPY_INFO * Intrude_GetOccupyInfo(GAMEDATA *gamedata, int palace_area)
{
  OCCUPY_INFO *occupy;
  
  if(palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  }
  else{
    occupy = GAMEDATA_GetOccupyInfo(gamedata, palace_area);
  }
  
  return occupy;
}

//==================================================================
/**
 * 指定したNetIDのMYSTATUSを取得する
 *
 * @param   intcomm		
 * @param   net_id	  
 *
 * @retval  MYSTATUS *		
 */
//==================================================================
MYSTATUS * Intrude_GetMyStatus(GAMEDATA *gamedata, int net_id)
{
  MYSTATUS *myst;
  
  if(net_id == GAMEDATA_GetIntrudeMyID(gamedata)){
    myst = GAMEDATA_GetMyStatus(gamedata);
  }
  else{
    myst = GAMEDATA_GetMyStatusPlayer(gamedata, net_id);
  }
  
  return myst;
}

//==================================================================
/**
 * 指定パレスエリアのミッション選択候補リストのポインタを取得
 *
 * @param   intcomm		
 * @param   palace_area		パレスエリア
 *
 * @retval  MISSION_CHOICE_LIST *		ミッション選択候補リストへのポインタ
 */
//==================================================================
MISSION_CHOICE_LIST * Intrude_GetChoiceList(INTRUDE_COMM_SYS_PTR intcomm, int palace_area)
{
  return &intcomm->mission.list[palace_area];
}

//==================================================================
/**
 * 自分が今いるパレスエリアNoを取得する
 *
 * @param   intcomm		
 *
 * @retval  u8		パレスエリアNo
 */
//==================================================================
u8 Intrude_GetPalaceArea(const GAMEDATA *gamedata)
{
  return GAMEDATA_GetIntrudePalaceArea(gamedata);
}

//==================================================================
/**
 * WFBCパラメータを受信したかを取得
 *
 * @param   intcomm		
 *
 * @retval  BOOL		TRUE:受信した　FALSE:受信していない
 */
//==================================================================
BOOL Intrude_GetRecvWfbc(INTRUDE_COMM_SYS_PTR intcomm)
{
  return intcomm->wfbc_recv;
}

//==================================================================
/**
 * WFBC通信ワークを取得
 *
 * @param   intcomm		
 *
 * @retval  WFBC_COMM_DATA *		WFBC通信ワークへのポインタ
 */
//==================================================================
WFBC_COMM_DATA * Intrude_GetWfbcCommData(INTRUDE_COMM_SYS_PTR intcomm)
{
  return &intcomm->wfbc_comm_data;
}

//==================================================================
/**
 * 自分が現在いる侵入先のPM_VERSIONを取得
 *
 * @param   game_comm		
 *
 * @retval  int		自分がいるROMのPM_VERSION
 */
//==================================================================
u8 Intrude_GetRomVersion(GAME_COMM_SYS_PTR game_comm)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(game_comm);
  const MYSTATUS *myst;
  u8 palace_area;
  
  if(GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){
    return GET_VERSION();
  }
  
  palace_area = GAMEDATA_GetIntrudePalaceArea(gamedata);
  myst = Intrude_GetMyStatus(gamedata, palace_area);
  return MyStatus_GetRomCode( myst );
}

//==================================================================
/**
 * 自分が現在いる侵入先の季節を取得
 *
 * @param   game_comm		
 *
 * @retval  u8		自分がいるROMの季節ID
 */
//==================================================================
u8 Intrude_GetSeasonID(GAME_COMM_SYS_PTR game_comm)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(game_comm);
  u8 palace_area;
  
  if(GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){
    return GAMEDATA_GetSeasonID(gamedata);
  }
  
  palace_area = Intrude_GetPalaceArea(gamedata);
  if(palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    return GAMEDATA_GetSeasonID(gamedata);
  }
  return GAMEDATA_GetIntrudeSeasonID(gamedata, palace_area);
}

//==================================================================
/**
 * 隣のパレスが自分のパレスかを調べる
 *
 * @param   game_comm		
 * @param   gamedata		
 * @param   dir		      調べる隣の向き(DIR_LEFT or DIR_RIGHT)
 *
 * @retval  BOOL		    TRUE：自分のパレス　FALSE:自分ではない人のパレス
 */
//==================================================================
BOOL Intrude_CheckNextPalaceAreaMine(GAME_COMM_SYS_PTR game_comm, const GAMEDATA *gamedata, u16 dir)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  int palace_area;
  
  if(intcomm == NULL){
    return TRUE;
  }
  
  palace_area = intcomm->intrude_status_mine.palace_area;
  if(dir == DIR_LEFT){
    palace_area--;
    if(palace_area < 0){
      palace_area = intcomm->connect_map_count; //intcomm->member_num - 1;
    }
  }
  else{
    palace_area++;
    if(palace_area >= intcomm->connect_map_count + 1){ //intcomm->member_num){
      palace_area = 0;
    }
  }
  
  if(palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 次期バージョンも加味した侵入としてのROMバージョンを取得する
 *    ※使用場所には注意すること！
 *
 * @param   pm_version		
 * @param   trainer_id		
 *
 * @retval  u8		ROMバージョン
 *
 * 次期バージョンの場合はトレーナーIDから黒か白、どちらかのバージョンを返します
 */
//==================================================================
u8 Intrude_GetIntrudeRomVersion(u8 pm_version, u32 trainer_id)
{
  if(pm_version != VERSION_BLACK && pm_version != VERSION_WHITE){
    if(trainer_id & (1 << 16)){
      pm_version = VERSION_WHITE;
    }
    else{
      pm_version = VERSION_BLACK;
    }
  }
  return pm_version;
}

//==================================================================
/**
 * グレースケールに変更する必要があるマップかを調べる
 *
 * @param   game_comm		
 * @param   gamedata		
 *
 * @retval  GRAYSCALE_TYPE	グレースケールタイプ
 */
//==================================================================
GRAYSCALE_TYPE Intrude_CheckGrayScaleMap(GAME_COMM_SYS_PTR game_comm, GAMEDATA *gamedata)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  u8 invasion_netid, pm_version;
  const MYSTATUS *myst;
  
  if (FIELD_STATUS_GetMapMode( GAMEDATA_GetFieldStatus( gamedata) ) != MAPMODE_INTRUDE ) {
    return GRAYSCALE_TYPE_NULL;
  }
  
  invasion_netid = Intrude_GetPalaceArea(gamedata);
  
  if(invasion_netid == GAMEDATA_GetIntrudeMyID(gamedata)){
    return GRAYSCALE_TYPE_NULL;
  }
  
  myst = GAMEDATA_GetMyStatusPlayer(gamedata, invasion_netid);
  pm_version = Intrude_GetIntrudeRomVersion(MyStatus_GetRomCode(myst), MyStatus_GetID(myst));
  switch(pm_version){
  case VERSION_WHITE:
    return GRAYSCALE_TYPE_WHITE;
  case VERSION_BLACK:
    return GRAYSCALE_TYPE_BLACK;
  }
  return GRAYSCALE_TYPE_NEXT;
}

//==================================================================
/**
 * パレスで誰かと繋がっているかを取得する
 *
 * @param   game_comm		
 *
 * @retval  BOOL		    TRUE:パレスで誰かと接続状態
 * @retval  BOOL		    FALSE:誰とも繋がっていない
 */
//==================================================================
BOOL Intrude_CheckPalaceConnect(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL){
    return FALSE;
  }
  if(intcomm->recv_profile > 1 && GFL_NET_GetConnectNum() > 1){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * パレスに接続してもよいZONEにいるかを判定
 *
 * @param   zone_id		
 *
 * @retval  BOOL		  TRUE:接続してよい
 */
//==================================================================
BOOL Intrude_CheckZonePalaceConnect(u16 zone_id)
{
  int i;
  
  for(i = 0; i < NELEMS(PalaceZoneIDTbl); i++){
    if(PalaceZoneIDTbl[i] == zone_id){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * チュートリアルを全てこなしているかチェックする
 *
 * @param   gamedata		
 *
 * @retval  BOOL		TRUE:全てこなした　FALSE:こなしていないのが残っている
 */
//--------------------------------------------------------------
BOOL Intrude_CheckTutorialComplete(GAMEDATA *gamedata)
{
  EVENTWORK *evwork = GAMEDATA_GetEventWork(gamedata);
  static const u16 check_flag_array[] = {
    SYS_FLAG_PALACE_MISSION_START,
    SYS_FLAG_PALACE_MISSION_CLEAR,
    //SYS_FLAG_PALACE_DPOWER,
  };
  int i;
  
  for(i = 0; i < NELEMS(check_flag_array); i++){
    if(EVENTWORK_CheckEventFlag(evwork, check_flag_array[i]) == FALSE){
      return FALSE;
    }
  }
  return TRUE;
}

//==================================================================
/**
 * 誰かと接続してからミッションを始めるまでの間のタイムアウトメッセージの確認
 *
 * @param   intcomm		
 *
 * @retval  BOOL		TRUE:タイムアウト目前の為、警告メッセージを表示する必要がある
 * @retval  BOOL		FALSE:表示する必要は無い
 */
//==================================================================
BOOL Intrude_CheckStartTimeoutMsgON(INTRUDE_COMM_SYS_PTR intcomm)
{
  if(intcomm->mission_start_timeout_warning && MISSION_RecvCheck(&intcomm->mission) == FALSE){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * ライブ通信画面用：侵入状態を取得
 *
 * @param   game_comm		
 *
 * @retval  LIVE_COMM		
 */
//==================================================================
LIVE_COMM Intrude_CheckLiveCommStatus(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL){
    return LIVE_COMM_NULL;
  }
  if(intcomm->live_comm_status == 0){
    if(Intrude_GetIntrudeStatus(game_comm) == INTRUDE_CONNECT_MISSION_PARTNER){
      return LIVE_COMM_INTRUDE_PARTNER;
    }
    return LIVE_COMM_INTRUDE;
  }
  return LIVE_COMM_INTRUDE_OUT;
}

//==================================================================
/**
 * ライブ通信画面用：侵入状態をセット(LIVE_COMM_INTRUDE_OUT)
 *
 * @param   game_comm		
 */
//==================================================================
void Intrude_SetLiveCommStatus_IntrudeOut(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm != NULL){
    intcomm->live_comm_status = TRUE;
  }
}

//==================================================================
/**
 * 侵入の接続状況を取得
 *
 * @param   game_comm		
 *
 * @retval  INTRUDE_CONNECT		
 */
//==================================================================
INTRUDE_CONNECT Intrude_GetIntrudeStatus(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL){
    return INTRUDE_CONNECT_NULL;
  }
  
  if(MISSION_RecvCheck(&intcomm->mission) == TRUE){
    if(MISSION_CheckTargetIsMine(intcomm) == TRUE){
      return INTRUDE_CONNECT_MISSION_TARGET;
    }
    if(Intrude_CheckTutorialComplete( GameCommSys_GetGameData(game_comm) ) == FALSE){
      return INTRUDE_CONNECT_INTRUDE; //チュートリアルが済んでいないと協力者にはなれない
    }
    return INTRUDE_CONNECT_MISSION_PARTNER;
  }
  return INTRUDE_CONNECT_INTRUDE;
}

//==================================================================
/**
 * ミッションに参加している人数を取得
 *
 * @param   game_comm		
 *
 * @retval  int		      参加人数
 */
//==================================================================
int Intrude_GetMissionEntryNum(GAME_COMM_SYS_PTR game_comm)
{
  NetID net_id;
  int entry_num = 0;
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL){
    return 0;
  }
  if(MISSION_RecvCheck(&intcomm->mission) == FALSE){
    return 0;
  }
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(intcomm->intrude_status[net_id].mission_entry == TRUE){
      entry_num++;
    }
  }
  if(entry_num == 0){
    //通信の順番的にステータスがミッションデータ受信後にセットされるものなので0がありえる。
    //その場合は受注した人分のカウントとして1を返す
    return 1;
  }
  return entry_num;
}

//==================================================================
/**
 * ミッションのターゲットとなっているプレイヤーの名前を取得
 *
 * @param   game_comm		
 * @param   dest_buf		名前代入先
 *
 * @retval  BOOL		TRUE:名前代入成功　FALSE:代入失敗
 */
//==================================================================
BOOL Intrude_GetTargetName(GAME_COMM_SYS_PTR game_comm, STRBUF *dest_buf)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  NetID target_netid;
  MYSTATUS *target_myst;
  GAMEDATA *gamedata = GameCommSys_GetGameData(game_comm);
  
  if(intcomm == NULL || MISSION_RecvCheck(&intcomm->mission) == FALSE){
    return FALSE;
  }
  
  target_netid = MISSION_GetMissionTargetNetID(intcomm, &intcomm->mission);
  target_myst = GAMEDATA_GetMyStatusPlayer(gamedata, target_netid);
  MyStatus_CopyNameString( target_myst, dest_buf );
  return TRUE;
}

//==================================================================
/**
 * 侵入の通信を終了しようとしている状態かを調べる
 *
 * @param   game_comm		
 *
 * @retval  BOOL		TRUE:終了処理実行中
 * @retval  BOOL		FALSE:終了処理は実行していない
 */
//==================================================================
BOOL Intrude_CheckNetIsExit(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm;
  
  if(GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_INVASION){
    return FALSE; //侵入通信が起動していない
  }
  
  intcomm = GameCommSys_GetAppWork(game_comm);
  if(intcomm == NULL){
    return FALSE; //intcommがNULLはありえないが一応チェック
  }
  
  if(intcomm->comm_exit_exe == FALSE){
    return FALSE; //通信終了を実行しているタイミングではない
  }
  
  if(GFL_NET_IsExit() == TRUE){
    return FALSE;  //終了処理は既に完了している
  }
  return TRUE;  //終了処理実行中
}

//==================================================================
/**
 * 「通信プレイヤー更新不可マップにいる」と判定したフラグをリセットする
 *
 * @param   game_comm		
 *
 * 自分のZONE_IDが変更するたびに必ず呼んでください
 */
//==================================================================
void Intrude_ResetStopCommPlayerUpdateMapFlag(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm != NULL){
    intcomm->stop_comm_player_status = STOP_COMM_PLAYER_STATUS_NO_CHECK;
  }
}
