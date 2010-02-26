//======================================================================
/**
 * @file  united_nations_st.c
 * @brief   国連常駐部
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "arc/fieldmap/fldmmdl_objcode.h"  //for OBJCODE
#include "savedata/un_savedata.h"
#include "savedata/un_savedata_local.h"
#include "un_roominfo_def.h"
#include "united_nations.h"

#include "gamesystem/game_event.h" //for GMEVENT_RESULT

#include "app/un_select.h"  //for overlay and UN_SELECT_PARAM 

typedef struct UN_COUNT_SEL_WORK_tag
{
  UN_SELECT_PARAM Param;
  u16 *OutFloor;
  u16 *CountryCode;
  u16 *Ret;
}UN_COUNT_SEL_WORK;

static GMEVENT_RESULT CallAppEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * @brief　部屋の人物情報を取得　ＯＢＪコード
 * @note    ゾーンチェンジ時にスクリプトからコールされる
 * @param   work      ワークポインタ
 * @apram   inObjIdx インデックス番号 0～UN_ROOM_OBJ_MAX
 * @retval  u32       ＯＢＪコード
*/
//--------------------------------------------------------------
u32 UN_GetRoomObjCode(UNSV_WORK* work, const u32 inObjIdx)
{
  if (inObjIdx >= work->ObjNum)
  {
    if (inObjIdx >= UN_ROOM_OBJ_MAX)
    {
      GF_ASSERT_MSG(0,"idx error idx=%d",inObjIdx);
      return BOY2;
    }
    //定員数以内であれば、表示はしないはずだが、定数を返しておく
    return BOY2;
  }
  
  return work->UnObjCode[inObjIdx];
}

//--------------------------------------------------------------
/**
 * @brief　部屋情報取得
 * @note  　ゾーンチェンジ時にスクリプトからコールで部屋のお客の数を取得するので常駐に置く
 * @note    取得できる情報は、客数、部屋の国コード、フロア
 * @param   work      ワークポインタ
 * @param   inType    取得情報タイプ  un_roominfo_def.h 参照
 * @retval  u32       取得結果
*/
//--------------------------------------------------------------
u32 UN_GetRoomInfo(UNSV_WORK* work, const u32 inType)
{
  u32 val;
  switch(inType){
  case UN_ROOMINFO_TYPE_OBJNUM:
    val = work->ObjNum;
    break;
  case UN_ROOMINFO_TYPE_COUNTRY:
    val = work->TargetCountryCode;
    break;
  case UN_ROOMINFO_TYPE_FLOOR:
    val = work->Floor;
    break;
  default:
    GF_ASSERT_MSG(0,"info code error %d",inType);
    val = 0;
  }
  return val;
}

//--------------------------------------------------------------
/**
 * @brief　国選択アプリ呼び出し
 * @param   gsys      ゲームシステムポインタ
 * @param   inFloor     現在フロア　エントランスにいるときは1以下を指定
 * @param   outFloor    フロア格納バッファ
 * @param   outCountry  国コード格納バッファ
 * @param   ret_wk      結果格納
 * @retval  GMEVENT   イベントポインタ
*/
//--------------------------------------------------------------
GMEVENT * UN_CreateAppEvt(GAMESYS_WORK *gsys, u16 inFloor, u16 *outFloor, u16 *outCountry, u16 *ret_wk)
{
  UN_COUNT_SEL_WORK *evt_work;
  GMEVENT *event;
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, CallAppEvt, sizeof(UN_COUNT_SEL_WORK) );
  
  evt_work = GMEVENT_GetEventWork(event);
  evt_work->Param.InFloor = inFloor;

  return event;
}

//--------------------------------------------------------------
/**
 * @brief   国選択アプリコールイベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
*/
//--------------------------------------------------------------
static GMEVENT_RESULT CallAppEvt( GMEVENT* event, int* seq, void* work )
{
  UN_COUNT_SEL_WORK *evt_work;
  evt_work = GMEVENT_GetEventWork(event);

  switch(*seq){
  case 0:
    //デモプロック
    GMEVENT_CallProc( event, FS_OVERLAY_ID(un_select), &UNSelectProcData, &evt_work->Param );
    (*seq)++;
    break;
  case 1:
    //デモからデータ取得
    if ( evt_work->Param.Decide == TRUE )
    {
      //決定した
      *(evt_work->Ret) = TRUE;
      *(evt_work->OutFloor) = evt_work->Param.OutFloor;
      *(evt_work->CountryCode) = evt_work->Param.CountryCode;
    }
    else
    {
      //キャンセルした
      *(evt_work->Ret) = FALSE;
      *(evt_work->OutFloor) = 0;
      *(evt_work->CountryCode) = 0;
    }

    //イベント終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}


