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

#include "arc/fieldmap/fldmmdl_objcode.h"  //for OBJCODE
#include "msg/script/msg_c03r0801.h"
#include "tr_tool/trno_def.h"
#include "sound/pm_sndsys.h"

#define WHISTLE_MARGINE (45)

#define PL_BOAT_MODE_TIME (180*30)    //180秒

#define DOUBLE_ROOM_NUM  (6)
#define LEFT_ROOM_NUM (8)
#define RIGHT_ROOM_NUM (7)


#define TRAINER_MAX     (10)
#define SHORT_TALKER_MAX     (15)
#define LONG_TALKER_MAX     (5)

#define ENTRY_TR_MAX    (7)
#define ENTRY_LONG_TALKER_MAX (2)
#define ENTRY_SHORT_TALKER_MAX (10)

//SE
#define PL_BOAT_SE_WHISTLE  (SEQ_SE_FLD_78)   //汽笛

typedef struct ENTRY_WORK_tag
{
  u8 RoomIdx[ROOM_NUM];
  u8 RoomEntry[ROOM_NUM];
  u8 EntryCount;
  u8 LastIdx;
  u8 Trainer[ENTRY_TR_MAX];
  u8 LongTalker[ENTRY_LONG_TALKER_MAX];
  u8 ShortTalker[ENTRY_SHORT_TALKER_MAX];
  BOOL DblBtl[DBL_BTL_ROOM_NUM];
}ENTRY_WORK;

typedef struct TR_DATA_tag
{
  int ObjCode;
  int TrID;
  int Msg[2];
}TR_DATA;

typedef struct NONE_TR_DATA_tag
{
  int ObjCode;
  int Msg[2];
}NONE_TR_DATA;

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

#ifdef PM_DEBUG
static const TR_DATA DbgTrData[TRAINER_MAX] = {
  {BABYGIRL1,TRID_BUSINESS1_05,{msg_c03r0801_businessman_04,msg_c03r0801_businessman_05}},
  {BOY2,TRID_ELITEM_05,{msg_c03r0801_trainerm_03,msg_c03r0801_trainerm_04}},
  {BOY3,TRID_ELITEW_05,{msg_c03r0801_trainerw_03,msg_c03r0801_trainerw_04}},
  {GIRL2,TRID_TANPAN_11,{msg_c03r0801_boy2_03,msg_c03r0801_boy2_04}},
  {GIRL3,TRID_MINI_05,{msg_c03r0801_girl2_04,msg_c03r0801_girl2_05}},
  {GIRL4,TRID_PRINCE_04,{msg_c03r0801_boy4_03,msg_c03r0801_boy4_04}},
  {MIDDLEMAN2,TRID_PRINCESS_04,{msg_c03r0801_girl4_03,msg_c03r0801_girl4_04}},
  {MAN2,TRID_GENTLE_01,{msg_c03r0801_gentleman_04,msg_c03r0801_gentleman_05}},
  {WOMAN1,TRID_MADAM_01,{msg_c03r0801_lady_04,msg_c03r0801_lady_05}},
  {WOMAN3,TRID_FUTAGO_03,{msg_c03r0801_babygirl1_01,msg_c03r0801_babygirl1_02}},    //ダブルバトルトレーナー
};

static const NONE_TR_DATA DbgLongTalkerData[LONG_TALKER_MAX] = {
  {BREEDERM,{msg_c03r0801_businessman_02,msg_c03r0801_businessman_03}},
  {BREEDERW,{msg_c03r0801_girl2_02,msg_c03r0801_girl2_03}},
  {RANGERM,{msg_c03r0801_gentleman_02,msg_c03r0801_gentleman_03}},
  {RANGERW,{msg_c03r0801_lady_02,msg_c03r0801_lady_03}},
  {WAITER,{msg_c03r0801_man3_02,msg_c03r0801_man3_03}},
};

static const NONE_TR_DATA DbgShortTalkerData[SHORT_TALKER_MAX] = {
  {BREEDERM,{msg_c03r0801_businessman_01,msg_c03r0801_businessman_01}},
  {BREEDERW,{msg_c03r0801_trainerm_01,msg_c03r0801_trainerm_01}},
  {RANGERM,{msg_c03r0801_trainerm_02,msg_c03r0801_trainerm_02}},
  {RANGERW,{msg_c03r0801_trainerw_01,msg_c03r0801_trainerw_01}},
  {WAITER,{msg_c03r0801_trainerw_02,msg_c03r0801_trainerw_02}},
  {WAITRESS,{msg_c03r0801_boy2_01,msg_c03r0801_boy2_01}},
  {BACKPACKERM,{msg_c03r0801_boy2_02,msg_c03r0801_boy2_02}},
  {BACKPACKERW,{msg_c03r0801_girl2_01,msg_c03r0801_girl2_01}},
  {DOCTOR,{msg_c03r0801_boy4_01,msg_c03r0801_boy4_01}},
  {NURSE,{msg_c03r0801_boy4_02,msg_c03r0801_boy4_02}},

  {NURSE,{msg_c03r0801_girl4_01,msg_c03r0801_girl4_01}},
  {NURSE,{msg_c03r0801_girl4_02,msg_c03r0801_girl4_02}},
  {NURSE,{msg_c03r0801_gentleman_01,msg_c03r0801_gentleman_01}},
  {NURSE,{msg_c03r0801_lady_01,msg_c03r0801_lady_01}},
  {NURSE,{msg_c03r0801_man3_01,msg_c03r0801_man3_01}},
};


#endif

//部屋情報
typedef struct ROOM_ST_tag
{
  ROOM_SIDE Side;    //LEFT or RIGHT
  ROOM_TYPE Type;    //SINGLE or DOUBLE
}ROOM_ST;

static const ROOM_ST RoomSt[ROOM_NUM] = 
{
  {ROOM_SIDE_LEFT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_SINGLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_SINGLE},

  {ROOM_SIDE_LEFT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_LEFT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_DOUBLE},
  {ROOM_SIDE_RIGHT, ROOM_TYPE_DOUBLE},
};

//ダブル型部屋インデックス群
static const u8 DoubleRoom[DOUBLE_ROOM_NUM] = {
  9,10,11,12,13,14
};

//船内左の部屋インデックス群
static const u8 LeftRoom[LEFT_ROOM_NUM] = {
  0,1,2,3,9,10,11,12
};
//船内右の部屋インデックス群
static const u8 RightRoom[RIGHT_ROOM_NUM] = {
  4,5,6,7,8,13,14
};

static PL_BOAT_EVT GetEvt(PL_BOAT_WORK_PTR work);

static void EntryTrainer(PL_BOAT_WORK *work);
static void InitEntryWork(ENTRY_WORK *work);
static int SearchRoomIdx(ENTRY_WORK *work, int inRoomIdx);
static int GetEntryItem(ENTRY_WORK *work, const int inIdx);
static BOOL DelEntry(ENTRY_WORK *work, const int inIdx);
static BOOL EntryRoom(ENTRY_WORK *work);
static void CreateTrTbl(const int inTblLen, const int inNum, u8 *workTbl, u8 *outTbl);

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
  
  //@todo イベント時間テーブルをロードするかも
  ;
  EntryTrainer(work);
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
 * @param   inMsgKind   メッセージ種類　0:通常　1：対戦後（トレーナーでないＮＰＣのときは通常）3:戦闘前ダブル相方　4：戦闘後ダブル相方
 * @retval  int       メッセージＩＤ
*/
//--------------------------------------------------------------
int PL_BOAT_GetMsg(PL_BOAT_WORK_PTR work, const int inRoomIdx, const inMsgKind)
{
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

  u8 tr_tbl[TRAINER_MAX];
  u8 long_tbl[LONG_TALKER_MAX];
  u8 short_tbl[SHORT_TALKER_MAX];

  GFL_RTC_GetDate( &date );

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

  OS_Printf("TrainerNum = %d\n",tr_num);

  //部屋のエントリワークを初期化
  InitEntryWork(&entry_work);
  //部屋のエントリ
  {
    BOOL rc;
    rc = EntryRoom(&entry_work);
    //不測の事態のための対処策
    if (!rc)  InitEntryWork(&entry_work);   //初期化状態で続行
  }

  //テーブルからトレーナをエントリ(ダブルバトルトレーナーを除く)
  CreateTrTbl(
      TRAINER_MAX-1, tr_num, tr_tbl, entry_work.Trainer
      );
  //ダブルバトルを抽選する場合、トレーナーテーブルの先頭のものを上書き
  if(1){//@todo
    entry_work.Trainer[0] = TRAINER_MAX-1;
  }
   //テーブルから非トレーナー（長話）をエントリ
  CreateTrTbl(
      LONG_TALKER_MAX, ENTRY_LONG_TALKER_MAX, long_tbl, entry_work.LongTalker
      );
  //テーブルから非トレーナー（通常）をエントリ
  CreateTrTbl(
      SHORT_TALKER_MAX, ROOM_NUM-(tr_num+ENTRY_LONG_TALKER_MAX), short_tbl, entry_work.ShortTalker
      );
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

  {
    int count = 0;
    //ダブルバトル部屋セット
    for (i=0;i<DBL_BTL_ROOM_NUM;i++)
    {
      work->DblBtl[i] = entry_work.DblBtl[i];
    }
    //部屋にトレーナーを格納
    for (i=0;i<tr_num;i++)
    {
      u8 room_idx = entry_work.RoomEntry[count++];
      u8 entry_id = entry_work.Trainer[i];
      work->RoomParam[room_idx].EntryID = entry_id;
      work->RoomParam[room_idx].TrID = DbgTrData[entry_id].TrID;
      work->RoomParam[room_idx].ObjCode = DbgTrData[entry_id].ObjCode;
      work->RoomParam[room_idx].NpcType = NPC_TYPE_TRAINER;
      work->RoomParam[room_idx].NpcMsg[0] = DbgTrData[entry_id].Msg[0];
      work->RoomParam[room_idx].NpcMsg[1] = DbgTrData[entry_id].Msg[1];
    }
    //部屋に非トレーナー（長話）を格納
    for(i=0;i<ENTRY_LONG_TALKER_MAX;i++)
    {
      u8 room_idx = entry_work.RoomEntry[count++];
      u8 entry_id = entry_work.LongTalker[i];
      work->RoomParam[room_idx].EntryID = i;
      work->RoomParam[room_idx].TrID = 0;
      work->RoomParam[room_idx].ObjCode = DbgLongTalkerData[entry_id].ObjCode;
      work->RoomParam[room_idx].NpcType = NPC_TYPE_LONG_TALKER;
      work->RoomParam[room_idx].NpcMsg[0] = DbgLongTalkerData[entry_id].Msg[0];
      work->RoomParam[room_idx].NpcMsg[1] = DbgLongTalkerData[entry_id].Msg[1];
    }
    //部屋に非トレーナー（通常）を格納
    for(i=0;i<ROOM_NUM-(tr_num+ENTRY_LONG_TALKER_MAX);i++)
    {
      u8 room_idx = entry_work.RoomEntry[count++];
      u8 entry_id = entry_work.ShortTalker[i];
      work->RoomParam[room_idx].EntryID = i;
      work->RoomParam[room_idx].TrID = 0;
      work->RoomParam[room_idx].ObjCode = DbgShortTalkerData[entry_id].ObjCode;
      work->RoomParam[room_idx].NpcType = NPC_TYPE_SHORT_TALKER;
      work->RoomParam[room_idx].NpcMsg[0] = DbgShortTalkerData[entry_id].Msg[0];
      work->RoomParam[room_idx].NpcMsg[1] = DbgShortTalkerData[entry_id].Msg[1];
    }
  }

  //ダンプ
  for (i=0;i<ROOM_NUM;i++)
  {
    //u8 room_idx = entry_work.RoomEntry[i];
    u8 room_idx = i;
    OS_Printf("room %d\n",room_idx);
    OS_Printf("OBJCODE %d\n",work->RoomParam[room_idx].ObjCode);
    OS_Printf("NPC TYPE %d\n",work->RoomParam[room_idx].NpcType);
    OS_Printf("ENTRY_ID %d\n",work->RoomParam[room_idx].EntryID);
    OS_Printf("MSG %d,%d\n",
        work->RoomParam[room_idx].NpcMsg[0],work->RoomParam[room_idx].NpcMsg[1]);
  }
 
}

//ランダムテーブル作成
static void CreateTrTbl(const int inTblLen, const int inNum, u8 *workTbl, u8 *outTbl)
{
  int num;
  u8 i;

  for (i=0;i<inTblLen;i++)
  {
    workTbl[i] = i;
  }

  num = inNum;
  for (i=0;i<inNum;i++)
  {
    u8 idx;
    idx = GFUser_GetPublicRand(num);
    outTbl[i] = workTbl[idx];
    //使用したデータを末尾データで上書き
    workTbl[idx] = workTbl[num-1];
    num--;
  }
}


//エントリワーク初期化関数
static void InitEntryWork(ENTRY_WORK *work)
{
  int i;
  work->LastIdx = ROOM_NUM-1;
  work->EntryCount = 0;

  //ダブルバトル部屋クリア
  for (i=0;i<DBL_BTL_ROOM_NUM;i++)
  {
    work->DblBtl[i] = FALSE;
  }

  //部屋インデックス初期化
  for (i=0;i<ROOM_NUM;i++)
  {
    work->RoomIdx[i] = i;
    work->RoomEntry[i] = 0;
  }
  //トレーナーテーブル初期化
  for(i=0;i<ENTRY_TR_MAX;i++)
  {
    work->Trainer[i] = i;
  }
  //非トレーナー(長話)テーブル初期化
  for(i=0;i<ENTRY_LONG_TALKER_MAX;i++)
  {
    work->LongTalker[i] = i;
  }
  //非トレーナーテーブル初期化
  for(i=0;i<ENTRY_SHORT_TALKER_MAX;i++)
  {
    work->ShortTalker[i] = i;
  }
}

//テーブルの中身を検索して指定インデックス場所を返す
static int SearchRoomIdx(ENTRY_WORK *work, const int inRoomIdx)
{
  int i;
  for (i=0;i<ROOM_NUM;i++){
    if (work->RoomIdx[i] == inRoomIdx) return i;
  }
  GF_ASSERT_MSG(0,"roomidx=%d",inRoomIdx);
  return -1;
}

//指定インデックスの要素を取得
static int GetEntryItem(ENTRY_WORK *work, const int inIdx)
{
  //抽選最後尾インデックスを超えていたらアサート
  if (inIdx > work->LastIdx)
  {
    GF_ASSERT_MSG(0,"idx=%d  last=%d\n",inIdx, work->LastIdx);
    return -1;
  }
  return work->RoomIdx[inIdx];
}

//抽選対象から指定インデックスの要素をはずす
static BOOL DelEntry(ENTRY_WORK *work, const int inIdx)
{
  //抽選最後尾インデックスを超えていたらアサート
  if (inIdx > work->LastIdx)
  {
    GF_ASSERT_MSG(0,"idx=%d  last=%d\n",inIdx, work->LastIdx);
    return FALSE;
  }

  if ( inIdx < 0 )
  {
    GF_ASSERT(0);
    return FALSE;
  }

  //抽選対象最後尾の要素で上書きする
  work->RoomIdx[inIdx] = work->RoomIdx[work->LastIdx];
  
  //最後尾インデックスをデクリメント
  if ( work->LastIdx > 0 ) work->LastIdx--;

  return TRUE;
}

//部屋のエントリ順番決定
static BOOL EntryRoom(ENTRY_WORK *work)
{
  int i;
  int idx, room_idx, del_idx;
  int entry_room_num;
  BOOL rc;
  BOOL left,right;
  left = FALSE;
  right = FALSE;

  entry_room_num = ROOM_NUM;

  //ダブルバトルできる部屋を選択
  if(1)    //ダブルバトル抽選する @todo
  {
    if ( GFUser_GetPublicRand(2) ){
      room_idx = 2; //三号室
      left = TRUE;
      work->DblBtl[0] = TRUE;
    }else{
      room_idx = 7; //八号室
      right = TRUE;
      work->DblBtl[1] = TRUE;
    }
    del_idx = SearchRoomIdx(work, room_idx);
    //エントリ対象から抽選したインデックッスを除く
    rc = DelEntry(work, del_idx);
    if (!rc) return FALSE;
    //エントリテーブルに格納
    work->RoomEntry[work->EntryCount] = room_idx;
    work->EntryCount++;

    entry_room_num--;   //抽選総数デクリメント
  }

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

  entry_room_num--;   //抽選総数デクリメント

  if (RoomSt[idx].Side == ROOM_SIDE_LEFT) left = TRUE;
  else right = TRUE;

  //ここまでの抽選で、両サイドの部屋が抽選されていない場合は、抽選されていない部屋から選ぶ
  if ( (!right) || (!left) )
  {
    if ( right == FALSE )    //右がまだ抽選されてない
    {
      //右の部屋から抽選する
      idx = GFUser_GetPublicRand(RIGHT_ROOM_NUM);
      room_idx = RightRoom[idx];
    }else    //左がまだ抽選されてない
    {
      //左の部屋から抽選する
      idx = GFUser_GetPublicRand(LEFT_ROOM_NUM);
      room_idx = LeftRoom[idx];
    }
    del_idx = SearchRoomIdx(work, room_idx);
    //エントリ対象から抽選したインデックッスを除く
    rc = DelEntry(work, del_idx);
    if (!rc) return FALSE;
    //エントリテーブルに格納
    work->RoomEntry[work->EntryCount] = room_idx;
    work->EntryCount++;

    entry_room_num--;   //抽選総数デクリメント
  }

  //残りの部屋をエントリ
  for (i=0;i<entry_room_num;i++)
  {
    int denominator = entry_room_num-i;
    idx = GFUser_GetPublicRand(denominator);
    room_idx = GetEntryItem(work, idx);
    OS_Printf("idx = %d  room = %d\n",idx, room_idx);
    if (room_idx < 0) return FALSE;   //例外が起きた場合
    rc = DelEntry(work, idx);
    if (!rc) return FALSE;
    //エントリテーブルに格納
    work->RoomEntry[work->EntryCount] = room_idx;
    work->EntryCount++;
  }

  return TRUE;
}

