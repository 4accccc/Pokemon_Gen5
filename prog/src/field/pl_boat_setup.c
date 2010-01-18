//======================================================================
/**
 * @file  pl_boat_setup.c
 * @brief 遊覧船
 * @author  Saito
 */
//======================================================================

#include <gflib.h>
#include "pleasure_boat.h"
#include "pleasure_boat_def.h"
#include "pl_boat_def.h"
#include "pl_boat_setup.h"

#include "arc/fieldmap/fldmmdl_objcode.h"  //for OBJCODE
#include "msg/script/msg_c03r0801.h"
#include "tr_tool/trno_def.h"

#include "system/gfl_use.h"


#define DOUBLE_ROOM_NUM  (6)
#define LEFT_ROOM_NUM (8)
#define RIGHT_ROOM_NUM (7)

#define TRAINER_MAX     (10)
#define SHORT_TALKER_MAX     (15)
#define LONG_TALKER_MAX     (5)

#define ENTRY_TR_MAX    (7)
#define ENTRY_LONG_TALKER_MAX (2)
#define ENTRY_SHORT_TALKER_MAX (10)

typedef struct ENTRY_WORK_tag
{
  u8 RoomIdx[ROOM_NUM];
  u8 RoomEntry[ROOM_NUM];
  u8 EntryCount;
  u8 LastIdx;
  u8 Trainer[ENTRY_TR_MAX];
  u8 LongTalker[ENTRY_LONG_TALKER_MAX];
  u8 ShortTalker[ENTRY_SHORT_TALKER_MAX];
//  BOOL DblBtl[DBL_BTL_ROOM_NUM];
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
  ROOM_SIDE_LEFT,
  ROOM_SIDE_RIGHT,
}ROOM_SIDE;

typedef enum {
  ROOM_TYPE_SINGLE,
  ROOM_TYPE_DOUBLE,
}ROOM_TYPE;

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

//曜日ごとのトレーナー数（月～日）
static const u8 WeekTrNum[7] = {
  4,3,4,5,4,6,7
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

#include "../../../resource/pl_boat/pl_boat_data.cdat"

static void InitEntryWork(ENTRY_WORK *work);
static int SearchRoomIdx(ENTRY_WORK *work, int inRoomIdx);
static int GetEntryItem(ENTRY_WORK *work, const int inIdx);
static BOOL DelEntry(ENTRY_WORK *work, const int inIdx);
static BOOL EntryRoom(ENTRY_WORK *work);
static void CreateTrTbl(const int inTblLen, const int inNum, u8 *workTbl, u8 *outTbl);

//--------------------------------------------------------------
/**
 * @brief	トレーナー抽選
 * @param	 work      PL_BOAT_WORK_PTR
 * @retval none
*/
//--------------------------------------------------------------
void PL_BOAT_SETUP_EntryTrainer(PL_BOAT_WORK *work)
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

  work->TotalTrNum = tr_num;

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

  //テーブルからトレーナをエントリ
  CreateTrTbl(
      TRAINER_MAX, tr_num, tr_tbl, entry_work.Trainer
      );
/**  
  //ダブルバトルを抽選する場合、トレーナーテーブルの先頭のものを上書き
  if(1){//@todo
    entry_work.Trainer[0] = TRAINER_MAX-1;
  }
*/  
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
/**    
    //ダブルバトル部屋セット
    for (i=0;i<DBL_BTL_ROOM_NUM;i++)
    {
      work->DblBtl[i] = entry_work.DblBtl[i];
    }
*/    
    //部屋にトレーナーを格納
    for (i=0;i<tr_num;i++)
    {
      u8 room_idx = entry_work.RoomEntry[count++];
      u8 entry_id = entry_work.Trainer[i];
      work->RoomParam[room_idx].EntryID = entry_id;
      work->RoomParam[room_idx].TrID = TrData[entry_id].TrID;
      work->RoomParam[room_idx].ObjCode = TrData[entry_id].ObjCode;
      work->RoomParam[room_idx].NpcType = NPC_TYPE_TRAINER;
      work->RoomParam[room_idx].NpcMsg[0] = TrData[entry_id].Msg[0];
      work->RoomParam[room_idx].NpcMsg[1] = TrData[entry_id].Msg[1];
    }
    //部屋に非トレーナー（長話）を格納
    for(i=0;i<ENTRY_LONG_TALKER_MAX;i++)
    {
      u8 room_idx = entry_work.RoomEntry[count++];
      u8 entry_id = entry_work.LongTalker[i];
      work->RoomParam[room_idx].EntryID = i;
      work->RoomParam[room_idx].TrID = 0;
      work->RoomParam[room_idx].ObjCode = LongTalkerData[entry_id].ObjCode;
      work->RoomParam[room_idx].NpcType = NPC_TYPE_LONG_TALKER;
      work->RoomParam[room_idx].NpcMsg[0] = LongTalkerData[entry_id].Msg[0];
      work->RoomParam[room_idx].NpcMsg[1] = LongTalkerData[entry_id].Msg[1];
    }
    //部屋に非トレーナー（通常）を格納
    for(i=0;i<ROOM_NUM-(tr_num+ENTRY_LONG_TALKER_MAX);i++)
    {
      u8 room_idx = entry_work.RoomEntry[count++];
      u8 entry_id = entry_work.ShortTalker[i];
      work->RoomParam[room_idx].EntryID = i;
      work->RoomParam[room_idx].TrID = 0;
      work->RoomParam[room_idx].ObjCode = ShortTalkerData[entry_id].ObjCode;
      work->RoomParam[room_idx].NpcType = NPC_TYPE_SHORT_TALKER;
      work->RoomParam[room_idx].NpcMsg[0] = ShortTalkerData[entry_id].Msg[0];
      work->RoomParam[room_idx].NpcMsg[1] = ShortTalkerData[entry_id].Msg[1];
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
/**  
  //ダブルバトル部屋クリア
  for (i=0;i<DBL_BTL_ROOM_NUM;i++)
  {
    work->DblBtl[i] = FALSE;
  }
*/
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
/**
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
*/
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


