//======================================================================
/**
 * @file  pleasure_boat.c
 * @brief 遊覧船
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "pleasure_boat.h"
#include "system/main.h"  //for HEAPID_APP_CONTROL
#include "../../../resource/fldmapdata/script/c03r0801_def.h"  //for SCRID_～
#include "script.h"     //for SCRIPT_SetEventScript

#include "pl_boat_def.h"

#define WHISTLE_MARGINE (45)

#define PL_BOAT_MODE_TIME (10*30)    //180秒

#define NPC_TOTAL_NUM (15)
#define ROOM_NUM  (14)
#define DOUBLE_ROOM_NUM  (6)
#define LEFT_ROOM_NUM (8)
#define RIGHT_ROOM_NUM (6)

typedef struct PL_BOAT_ROOM_PRM_tag
{
  int TrID;       //トレーナーの場合はトレーナーＩＤ
  int CbjCode;    //見た目
  int NpcCode;    //メッセージとかＮＰＣタイプとかの識別用
}PL_BOAT_ROOM_PRM;

typedef struct PL_BOAT_WORK_tag
{
  BOOL WhistleReq;
  int Time;
  int WhistleCount;   //汽笛なった回数
  PL_BOAT_ROOM_PRM RoomParam[ROOM_NUM];
  u8 TrNumLeft;
  u8 TrNumRight;
  u8 TrNumSingle;
  u8 TrNumDouble;
}PL_BOAT_WORK;

typedef struct ENTRY_WORK_tag
{
  u8 RoomIdx[ROOM_NUM];
  u8 RoomEntry[ROOM_NUM];
  u8 EntryCount;
  u8 LastIdx;
}ENTRY_WORK;


typedef enum {
  PL_BOAT_EVT_NONE,
  PL_BOAT_EVT_MSG,
  PL_BOAT_EVT_WHISTLE,
  PL_BOAT_EVT_END,
}PL_BOAT_EVT;

typedef enum {
  ROOM_SIDE_LEFT,
  ROOM_SIDE_RIGHT,
}ROOM_SIDE;

typedef enum {
  ROOM_TYPE_SINGLE,
  ROOM_TYPE_DOUBLE,
}ROOM_TYPE;

//曜日ごとのトレーナー数（月～日）
static const u8 WeekTrNum[7] = {
  4,3,4,5,4,6,7
};

//部屋情報
typedef struct ROOM_ST_tag
{
  ROOM_SIDE Side;    //LEFT or RIGHT
  ROOM_TYPE Type;    //SINGLE or DOUBLE
}ROOM_ST;

static const ROOM_ST RoomSt[ROOM_NUM] = {
  {ROOM_SIDE_LEFT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_DOUBLE},

  {ROOM_SIDE_LEFT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_SINGLE},
};

//ダブル型部屋インデックス群
static const u8 DoubleRoom[DOUBLE_ROOM_NUM] = {
  0,1,2,3,4,5
};

//船内左の部屋インデックス群
static const u8 LeftRoom[LEFT_ROOM_NUM] = {
  0,1,2,3,6,7,8,9
};
//船内右の部屋インデックス群
static const u8 RightRoom[RIGHT_ROOM_NUM] = {
  4,5,10,11,12,13
};

static PL_BOAT_EVT GetEvt(PL_BOAT_WORK_PTR work);


static void InitEntryWork(ENTRY_WORK *work);
static int SearchRoomIdx(ENTRY_WORK *work, int inRoomIdx);
static int GetEntryItem(ENTRY_WORK *work, const int inIdx);
static BOOL DelEntry(ENTRY_WORK *work, const int inIdx);
static BOOL EntryRoom(ENTRY_WORK *work);

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
  
  //イベント時間テーブルをロードするかも
  ;
  return work;
}

//--------------------------------------------------------------
/**
 * @brief	終了関数
 * @param	work      PL_BOAT_WORK_PTR
 * @retval		none
*/
//--------------------------------------------------------------
void PL_BOAT_End(PL_BOAT_WORK_PTR *work)
{
  if ( *work != NULL )
  {
    GFL_HEAP_FreeMemory( *work );
    *work = NULL;
  }
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

  //汽笛のリクエストがかかっている場合はＳＥ再生
  if (work->WhistleReq)
  {
    ;
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
 * @retval  none
*/
//--------------------------------------------------------------
void PL_BOAT_AddTimeEvt(PL_BOAT_WORK_PTR work, const int inAddSec)
{
  int margine = work->Time%(WHISTLE_MARGINE*30);
  OS_Printf("now_time = %d::%d SEC\n",work->Time, work->Time/30);
  //時間加算
  margine += (inAddSec*30);
  if ( margine >= WHISTLE_MARGINE*30 )
  {
    //汽笛が鳴る直前まで時間を進行
    work->Time = (work->WhistleCount+1)*(WHISTLE_MARGINE*30)-1;
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
  default:
    GF_ASSERT(0);
    num = 0;
  }
  return num;
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
 * @brief	トレーナー抽選
 * @param	 work      PL_BOAT_WORK_PTR
 * @retval none
*/
//--------------------------------------------------------------
static void EntryTrainer(PL_BOAT_WORK *work)
{
  int tr_num;
  RTCDate date;
  u8 i;
  ENTRY_WORK entry_work;
  RTC_GetDate( &date );

  //部屋のエントリワークを初期化
  InitEntryWork(&entry_work);
  //部屋のエントリ
  {
    BOOL rc;
    rc = EntryRoom(&entry_work);
    //不測の事態のための対処策
    if (!rc)  InitEntryWork(&entry_work);   //初期化状態で続行
  }

  //曜日別にトレーナー数を決定
  switch( date.week ){
  case RTC_WEEK_MONDAY:
    tr_num = WeekTrNum[0];
    break;
  case RTC_WEEK_TUESDAY:
    tr_num = WeekTrNum[1];
    break;
  case RTC_WEEK_WEDNESDAY:
    tr_num = WeekTrNum[2];
    break;
  case RTC_WEEK_THURSDAY:
    tr_num = WeekTrNum[3];
    break;
  case RTC_WEEK_FRIDAY:
    tr_num = WeekTrNum[4];
    break;
  case RTC_WEEK_SATURDAY:
    tr_num = WeekTrNum[5];
    break;
  case RTC_WEEK_SUNDAY:
    tr_num = WeekTrNum[6];
    break;
  default:
    GF_ASSERT(0);
    tr_num = 0;
  }


  //テーブルからトレーナをエントリ
  ;
  //テーブルから非トレーナーをエントリ
  ;
  //トレーナ数が決まったら、それぞれの部屋の数を数える
  for (i=0;i<tr_num;i++)
  {
    int room_idx;
    room_idx = entry_work.RoomEntry[i];
    if (RoomSt[room_idx].Side == ROOM_SIDE_LEFT) work->TrNumLeft++;
    else work->TrNumRight++;
    if (RoomSt[room_idx].Type == ROOM_TYPE_SINGLE) work->TrNumSingle++;
    else work->TrNumDouble++;
  }
 
}

//エントリワーク初期化関数
static void InitEntryWork(ENTRY_WORK *work)
{
  int i;
  work->LastIdx = ROOM_NUM-1;
  work->EntryCount = 0;
  //部屋インデックス初期化
  for (i=0;i<ROOM_NUM;i++)
  {
    work->RoomIdx[i] = i;
    work->RoomEntry[i] = 0;
  }
}

//テーブルの中身を検索して指定インデックス場所を返す
static int SearchRoomIdx(ENTRY_WORK *work, const int inRoomIdx)
{
  int i;
  for (i=0;i<ROOM_NUM;i++){
    if (work->RoomIdx[i] == inRoomIdx) return i;
  }
  GF_ASSERT(0);
  return -1;
}

//指定インデックスの要素を取得
static int GetEntryItem(ENTRY_WORK *work, const int inIdx)
{
  //抽選最後尾インデックスを超えていたらアサート
  if (inIdx >= work->LastIdx)
  {
    GF_ASSERT(0);
    return -1;
  }
  return work->RoomIdx[inIdx];
}

//抽選対象から指定インデックスの要素をはずす
static BOOL DelEntry(ENTRY_WORK *work, const int inIdx)
{
  //抽選最後尾インデックスを超えていたらアサート
  if (inIdx >= work->LastIdx)
  {
    GF_ASSERT(0);
    return FALSE;
  }

  if ( inIdx < 0 )
  {
    GF_ASSERT(0);
    return FALSE;
  }

  //抽選対象最後尾の要素で上書きする
  work->RoomIdx[inIdx] = work->RoomIdx[work->LastIdx];
  
  //既に最後尾が0のときは例外処理
  if ( work->LastIdx == 0 )
  {
    GF_ASSERT(0);
    return FALSE;
  }

  //最後尾インデックスをデクリメント
  work->LastIdx--;

  return TRUE;
}

//部屋のエントリ順番決定
static BOOL EntryRoom(ENTRY_WORK *work)
{
  int i;
  int idx, room_idx, del_idx;
  BOOL rc;
  //ダブル型の部屋を1つ選択
  idx = GFUser_GetPublicRand(DOUBLE_ROOM_NUM);
  room_idx = DoubleRoom[idx];
  del_idx = SearchRoomIdx(work, room_idx);
  //エントリ対象から抽選したインデックッスを除く
  rc = DelEntry(work, del_idx);
  if (!rc) return FALSE;
  //エントリテーブルに格納
  work->RoomEntry[work->EntryCount] = room_idx;
  work->EntryCount++;

  //始めに選んだ部屋が船内の左右どちらにあるかで次の部屋の候補を絞る
  if ( RoomSt[idx].Side == ROOM_SIDE_LEFT ){    //左のとき
    //右の部屋から抽選する
    idx = GFUser_GetPublicRand(RIGHT_ROOM_NUM);
    room_idx = RightRoom[idx];
  }else{    //右のとき
    //左の部屋から抽選する
    idx = GFUser_GetPublicRand(LEFT_ROOM_NUM);
    room_idx = RightRoom[idx];
  }
  del_idx = SearchRoomIdx(work, room_idx);
  //エントリ対象から抽選したインデックッスを除く
  rc = DelEntry(work, del_idx);
  if (!rc) return FALSE;
  //エントリテーブルに格納
  work->RoomEntry[work->EntryCount] = room_idx;
  work->EntryCount++;

  //残りの部屋をエントリ
  for (i=0;i<ROOM_NUM-2;i++)
  {
    int denominator = (ROOM_NUM-2)-i;
    idx = GFUser_GetPublicRand(denominator);
    room_idx = GetEntryItem(work, idx);
    if (room_idx < 0) return FALSE;   //例外が起きた場合
    rc = DelEntry(work, idx);
    if (!rc) return FALSE;
    //エントリテーブルに格納
    work->RoomEntry[work->EntryCount] = room_idx;
    work->EntryCount++;
  }

  return TRUE;
}

