//======================================================================
/*
 * @file	united_nations.c
 * @brief	国連関連
 * @author saito
 */
//======================================================================
#include "united_nations.h"
#include "gamesystem/gamesystem.h"

#include "savedata/wifihistory.h"
#include "savedata/wifihistory_local.h"
#include "savedata/un_savedata_local.h"
#include "savedata/un_savedata.h"

#include "united_nations.h"
#include "net_app/union/union_beacon_tool.h"

#include "msg/script/msg_c11r0201.h"


#define UNDATA_MAX  (UNITEDNATIONS_PEOPLE_MAX)

static void SetData(WIFI_HISTORY *wh, UNSV_WORK* work, const int inCountryCode);
static u32 GetMsg(const int inSex, const u32 inTrID, const int inNature, const BOOL inFirst);

#ifdef PM_DEBUG
int DebugUnCountryCode = 0;
#endif

//--------------------------------------------------------------
/**
 * @brief 国連関連データのセット
 * @param inCountryCode
 * @retval  none
 */
//--------------------------------------------------------------
void UN_SetData(GAMESYS_WORK *gsys, const int inCountryCode)
{
  int code;
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  UNSV_WORK *work = GAMEDATA_GetUnsvWorkPtr(gdata);
  code = inCountryCode;
#ifdef PM_DEBUG
  if (DebugUnCountryCode != 0) code = DebugUnCountryCode;
#endif //PM_DEBUG
  
  //国コード上限チェック
  if (code < WIFI_COUNTRY_MAX)
  {
    //セーブデータにアクセスして、指定国コードのデータを収集
    SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
    WIFI_HISTORY *wh = SaveData_GetWifiHistory(sv);
    SetData(wh, work, code);
  }
  else
  {
    GF_ASSERT_MSG(0,"CountryCode = %d",code);
    UNSV_Init(work);
  }
}

//--------------------------------------------------------------
/**
 * @brief 国連関連データのセット
 *
 * @param * wh              WIFI履歴ワークポインタ
 * @param * work            国連ワークポインタ
 * @param inCountryCode     国コード
 * @retval  none
 */
//--------------------------------------------------------------
static void SetData(WIFI_HISTORY *wh, UNSV_WORK* work, const int inCountryCode)
{
  int i;
  
  work->TargetCountryCode = inCountryCode;
  work->ObjNum = 0;
  for (i=0;i<UNDATA_MAX;i++)
  {
    //取得
    u8 valid;
    valid = WIFIHISTORY_GetUnInfo(wh, i, UN_INFO_VALID);
    if ( valid )
    {
      MYSTATUS *my;
      my = WIFIHISTORY_GetUnMyStatus(wh, i);
      if (my != NULL)
      {
        int country_code;
        country_code = MyStatus_GetMyNation(my);
        //国コード比較
        if (country_code == inCountryCode)    //国コード一致？
        {
          u8 tr_view;
          tr_view = MyStatus_GetTrainerView( my );
          work->UnIdx[work->ObjNum] = i;            //インデックス格納
          work->UnObjCode[work->ObjNum] = UnionView_GetObjCode(tr_view);  //表示OBJコード格納
          work->ObjNum++;                           //表示OBJ数加算
          if (work->ObjNum >= UN_ROOM_OBJ_MAX) break;
        }
      }
      else GF_ASSERT(0);
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief 指定したインデックスの国連関連データインデックスを取得する
 * @param * work            国連ワークポインタ
 * @param inOnjIdx 部屋内OBJのインデックス0～4
 * @retval  u32     国連セーブデータの他人データインデックス  失敗した場合はUN_IDX_OVER
 */
//--------------------------------------------------------------
u8 UN_GetUnIdx(UNSV_WORK* work, const u32 inObjIdx)
{
  u8 rc = UN_IDX_OVER;
  if (inObjIdx < UN_ROOM_OBJ_MAX)
  {
    if ( inObjIdx < work->ObjNum ) rc = work->UnIdx[inObjIdx];
    else GF_ASSERT_MSG(0,"OBJ_IDX_ERROR obj_idx=%d max=%d",inObjIdx, work->ObjNum);
  }
  else GF_ASSERT_MSG(0,"OBJ_IDX_ERROR %d",inObjIdx);

  return rc;
}

//--------------------------------------------------------------
/**
 * @brief OBJの話すメッセージの決定
 * @param gsys          ゲームシステムポインタ
 * @param work            国連ワークポインタ
 * @param   inObjIdx        OBJインデックス0～4
 * @param  inFirstMsg       取得したいのは初回メッセージか？
 * @retval  u32     メッセージID
 */
//--------------------------------------------------------------
u32 UN_GetRoomObjMsg(GAMESYS_WORK *gsys, UNSV_WORK* work, const int inObjIdx, const BOOL inFirstMsg)
{
  int undata_idx;
  int sex;
  u32 id;
  int nature;

  //セーブデータにアクセス
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
  WIFI_HISTORY *wh = SaveData_GetWifiHistory(sv);

  if ( inObjIdx >= 5 )
  {
    GF_ASSERT(0);
    //適当なメッセージIDを返しておく
    return msg_un_default_man_00;
  }

  undata_idx = work->UnIdx[inObjIdx];
  {
    MYSTATUS *my;
    my = WIFIHISTORY_GetUnMyStatus(wh, undata_idx);
    //性別取得
    sex = MyStatus_GetMySex(my);
    //性格取得
    nature = WIFIHISTORY_GetUnInfo(wh, undata_idx, UN_INFO_NATURE);
    //トレーナーID取得
    id = MyStatus_GetID(my);
  }
  //メッセージ決定
  return GetMsg(sex, id, nature, inFirstMsg);

}

//--------------------------------------------------------------
/**
 * @brief OBJの話すメッセージの決定
 * @param inSex         性別 0 or 1
 * @param inTrID        トレーナーID
 * @param inNature      性格0～4
 * @param inFirst       取得したいのは初回メッセージか？
 * @retval  u32     メッセージID
 */
//--------------------------------------------------------------
static u32 GetMsg(const int inSex, const u32 inTrID, const int inNature, const BOOL inFirst)
{
  u32 base;
  u32 msg;
  //性格でメッセージインデックスの基点を決める
  switch(inNature){
  case 0:
    base = msg_un_default_man_00;
    break;
  case 1:
    base = msg_un_leader_man_00;
    break;
  case 2:
    base = msg_un_wierd_man_00;
    break;
  case 3:
    base = msg_un_researcher_man_00;
    break;
  case 4:
    base = msg_un_soft_man_00;
    break;
  default:
    GF_ASSERT_MSG(0,"nature error %d",inNature);
    base = msg_un_default_man_00;
  }
  msg = base;
  //性別でオフセットを計算
  if (inSex) msg = base + 6;//女性
  //初回会話か？
  if ( !inFirst )      //2回目以降
  {
    msg += 1;
    //トレーナIDを5で割った余りを基点に足しこむ
    msg += (inTrID % 5);
  }

  return msg;
}
#ifdef PM_DEBUG
u32 DebugGetUnCouontry(GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param)
{
  return DebugUnCountryCode;
}

void DebugSetUnCouontry(GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param, u32 value)
{
  DebugUnCountryCode = value;
}
#endif
