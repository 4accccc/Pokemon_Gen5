//======================================================================
/**
 * @file  pleasure_boat.c
 * @brief 遊覧船
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "pleasure_boat.h"
#include "pleasure_boat_def.h"
#include "system/main.h"  //for HEAPID_APP_CONTROL
#include "../../../resource/fldmapdata/script/c03r0801_def.h"  //for SCRID_～
#include "script.h"     //for SCRIPT_SetEventScript

#include "pl_boat_def.h"

#include "sound/pm_sndsys.h"

#include "pl_boat_setup.h"

#define WHISTLE_MARGINE (45)

#define PL_BOAT_MODE_TIME (180*30)    //180秒

//SE
#define PL_BOAT_SE_WHISTLE  (SEQ_SE_FLD_78)   //汽笛

typedef enum {
  PL_BOAT_EVT_NONE,
  PL_BOAT_EVT_MSG,
  PL_BOAT_EVT_WHISTLE,
  PL_BOAT_EVT_END,
}PL_BOAT_EVT;

FS_EXTERN_OVERLAY(pl_boat_setup);

static PL_BOAT_EVT GetEvt(PL_BOAT_WORK_PTR work);

//--------------------------------------------------------------
/**
 * @brief	初期化関数
 * @param	void
 * @retval		PL_BOAT_WORK_PTR
 */
//--------------------------------------------------------------
PL_BOAT_WORK_PTR PL_BOAT_Init(void)
{
  PL_BOAT_WORK_PTR work;

  work = GFL_HEAP_AllocClearMemory(HEAPID_APP_CONTROL, sizeof(PL_BOAT_WORK));

  work->WhistleReq = FALSE;
  work->Time = 0;
  work->WhistleCount = 0;
  work->TrNumLeft = 0;
  work->TrNumRight = 0;
  work->TrNumSingle = 0;
  work->TrNumDouble = 0;
  work->GameEnd = FALSE;
  //@todo イベント時間テーブルをロードするかも
  ;
  GFL_OVERLAY_Load( FS_OVERLAY_ID(pl_boat_setup) );
  PL_BOAT_SETUP_EntryTrainer(work);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(pl_boat_setup) );

  return work;
}

//--------------------------------------------------------------
/**
 * @brief	動作関数
 * @param	work      PL_BOAT_WORK_PTR
 * @retval		none
*/
//--------------------------------------------------------------
void PL_BOAT_Main(PL_BOAT_WORK_PTR work)
{
  if ( work == NULL ) return;

  if ( work->GameEnd ) return;

  //汽笛のリクエストがかかっている場合はＳＥ再生
  if (work->WhistleReq)
  {
    PMSND_PlaySE(PL_BOAT_SE_WHISTLE);
    work->WhistleCount++;
    work->WhistleReq = FALSE;
  }
}

//--------------------------------------------------------------
/**
 * @brief	時間経過チェック
 * @param	 work      PL_BOAT_WORK_PTR
 * @retval GMEVENT *event    発生イベントポインタ	
*/
//--------------------------------------------------------------
GMEVENT *PL_BOAT_CheckEvt(GAMESYS_WORK *gsys, PL_BOAT_WORK_PTR work)
{
  PL_BOAT_EVT evt_type;
  GMEVENT *event;

  if ( work == NULL ) return NULL;

  if ( work->GameEnd ) return NULL;

  //時間経過
  work->Time++;

  //イベント発生時間かをチェックする
  {
    BOOL rc;  //イベント発生フラグとして使用
    evt_type = GetEvt(work);
    switch(evt_type){
    case PL_BOAT_EVT_MSG:
      //船内アナウンス用スクリプトコール
      ;
      rc = TRUE;
      break;
    case PL_BOAT_EVT_WHISTLE:
      //汽笛リクエスト
      work->WhistleReq = TRUE;
      //汽笛はイベントで鳴らすわけではないのでリターンコードはFALSEを返す
      rc = FALSE;
      break;
    case PL_BOAT_EVT_END:
      //時間満了　終了スクリプトコール
      event = SCRIPT_SetEventScript( gsys, SCRID_PRG_C03R0801_TIMEUP, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
      rc = TRUE;
      break;
    default:
      rc = FALSE;
    }
  
    if (rc) return event;
  }

  return NULL;
}

//--------------------------------------------------------------
/**
 * @brief	強制的に時間経過させる　汽笛時間をまたいた場合は汽笛時間直前までの制限をかける
 * @param   work      PL_BOAT_WORK_PTR
 * @param   inAddSec  加算時間（秒）
 * @param   inWhistle   加算結果後、汽笛がなる時間の場合、汽笛を鳴らすかどうか　0：鳴らさない　1：鳴らす
 * @retval  none
*/
//--------------------------------------------------------------
void PL_BOAT_AddTimeEvt(PL_BOAT_WORK_PTR work, const int inAddSec, const int inWhistle)
{
  int margine = work->Time%(WHISTLE_MARGINE*30);
  OS_Printf("now_time = %d::%d SEC\n",work->Time, work->Time/30);
  //時間加算
  margine += (inAddSec*30);
  if ( margine >= WHISTLE_MARGINE*30 )
  {
    if ( inWhistle )
    {
      //汽笛が鳴る直前まで時間を進行
      work->Time = (work->WhistleCount+1)*(WHISTLE_MARGINE*30)-1;
    }
    else
    {
      //汽笛が鳴った直後まで時間を進行
      work->Time = (work->WhistleCount+1)*(WHISTLE_MARGINE*30);
    }
  }
  else
  {
    work->Time += (inAddSec*30);
  }
  OS_Printf("add_after_time = %d::%d SEC\n",work->Time, work->Time/30);
}

//--------------------------------------------------------------
/**
 * @brief　トレーナー数の取得
 * @param   work      PL_BOAT_WORK_PTR
 * @param   inSearchType    pl_boat_def.h　参照
 * @retval  int トレーナー数
*/
//--------------------------------------------------------------
int PL_BOAT_GetTrNum(PL_BOAT_WORK_PTR work, const int inSearchType)
{
  int num;
  switch(inSearchType){
  case PL_TR_SEARCH_TYPE_LEFT:
    num = work->TrNumLeft;
    break;
  case PL_TR_SEARCH_TYPE_RIGHT:
    num = work->TrNumRight;
    break;
  case PL_TR_SEARCH_TYPE_SINGLE:
    num = work->TrNumSingle;
    break;
  case PL_TR_SEARCH_TYPE_DOUBLE:
    num = work->TrNumDouble;
    break;
  case PL_TR_SEARCH_TYPE_TOTAL: 
    num = work->TotalTrNum;
    break;
  case PL_TR_SEARCH_TYPE_WIN:
    {
      int i;
      num = 0;
      for(i=0;i<ROOM_NUM;i++)
      {
        if ( work->RoomParam[i].BattleEnd ) num++;
      }
    }
    break;
  default:
    GF_ASSERT(0);
    num = 0;
  }
  return num;
}

//--------------------------------------------------------------
/**
 * @brief　部屋の人物情報を取得　NPCの種類
 * @param   work      PL_BOAT_WORK_PTR
 * @apram   inRoomIdx 部屋番号
 * @retval  int       
*/
//--------------------------------------------------------------
int PL_BOAT_GetNpcType(PL_BOAT_WORK_PTR work, const int inRoomIdx)
{
  return work->RoomParam[inRoomIdx].NpcType;
}

//--------------------------------------------------------------
/**
 * @brief　部屋の人物情報を取得　トレーナーＩＤ
 * @param   work      PL_BOAT_WORK_PTR
 * @apram   inRoomIdx 部屋番号
 * @retval  int       トレーナーＩＤ
*/
//--------------------------------------------------------------
int PL_BOAT_GetTrID(PL_BOAT_WORK_PTR work, const int inRoomIdx)
{
  return work->RoomParam[inRoomIdx].TrID;
}

//--------------------------------------------------------------
/**
 * @brief　部屋の人物情報を取得　メッセージＩＤ
 * @param   work      PL_BOAT_WORK_PTR
 * @apram   inRoomIdx 部屋番号
 * @param   inMsgKind   メッセージ種類　0:通常　1：対戦後（トレーナーでないＮＰＣのときは通常）
 * @retval  int       メッセージＩＤ
*/
//--------------------------------------------------------------
int PL_BOAT_GetMsg(PL_BOAT_WORK_PTR work, const int inRoomIdx, const inMsgKind)
{
/**  
  if ( PL_BOAT_CheckDblBtl(work, inRoomIdx) )
  {
    int msg;
    if (inMsgKind == PL_BOAT_BTL_BEFORE_PAIR) msg = msg_c03r0801_babygirl2_01;
    else if ( inMsgKind == PL_BOAT_BTL_AFTER_PAIR ) msg = msg_c03r0801_babygirl2_02;
    else  msg = work->RoomParam[inRoomIdx].NpcMsg[inMsgKind];

    return msg;
  }else
  {
    return work->RoomParam[inRoomIdx].NpcMsg[inMsgKind];
  }
*/
  return work->RoomParam[inRoomIdx].NpcMsg[inMsgKind];
}

//--------------------------------------------------------------
/**
 * @brief　部屋の人物情報を操作 戦闘したかチェック
 * @param   work      PL_BOAT_WORK_PTR
 * @apram   inRoomIdx 部屋番号
 * @retval  BOOL       TRUEで戦闘した
*/
//--------------------------------------------------------------
BOOL PL_BOAT_CheckBtlFlg(PL_BOAT_WORK_PTR work, const int inRoomIdx)
{
  return work->RoomParam[inRoomIdx].BattleEnd;
}

//--------------------------------------------------------------
/**
 * @brief　部屋の人物情報を操作 戦闘したことにする
 * @param   work      PL_BOAT_WORK_PTR
 * @apram   inRoomIdx 部屋番号
 * @param   inFlg     TRUEで戦闘した
 * @retval  none
*/
//--------------------------------------------------------------
void PL_BOAT_SetBtlFlg(PL_BOAT_WORK_PTR work, const int inRoomIdx, const BOOL inFlg)
{
  work->RoomParam[inRoomIdx].BattleEnd = inFlg;
}

//--------------------------------------------------------------
/**
 * @brief	イベント取得
 * @param	 work      PL_BOAT_WORK_PTR
 * @retval PL_BOAT_EVT    発生イベントタイプ
*/
//--------------------------------------------------------------
static PL_BOAT_EVT GetEvt(PL_BOAT_WORK_PTR work)
{
  //現在時間でイベントが発生するかを調べる
  if ( work->Time >= PL_BOAT_MODE_TIME ){
    return PL_BOAT_EVT_END;
  }

  if ( work->Time % (WHISTLE_MARGINE*30) == 0 ){
    return PL_BOAT_EVT_WHISTLE;
  }
  return PL_BOAT_EVT_NONE;
}

//--------------------------------------------------------------
/**
 * @brief　ゲーム終了
 * @param   work      PL_BOAT_WORK_PTR
 * @retval  none
*/
//--------------------------------------------------------------
void PL_BOAT_EndGame(PL_BOAT_WORK_PTR work)
{
  work->GameEnd = TRUE;
}

