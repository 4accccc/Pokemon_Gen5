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


#define UNDATA_MAX  (UNITEDNATIONS_PEOPLE_MAX)

static void SetData(WIFI_HISTORY *wh, UNSV_WORK* work, const int inCountryCode);

//--------------------------------------------------------------
/**
 * @brief 国連関連データのセット
 * @param inCountryCode
 * @retval  none
 */
//--------------------------------------------------------------
void UN_SetData(GAMESYS_WORK *gsys, const int inCountryCode)
{
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  UNSV_WORK *work = GAMEDATA_GetUnsvWorkPtr(gdata);
  
  //国コード上限チェック
  if (inCountryCode < WIFI_COUNTRY_MAX)
  {
    //セーブデータにアクセスして、指定国コードのデータを収集
    SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
    WIFI_HISTORY *wh = SaveData_GetWifiHistory(sv);
    SetData(wh, work, inCountryCode);
  }
  else
  {
    GF_ASSERT_MSG(0,"CountryCode = %d",inCountryCode);
    UNSV_Init(work);
  }
}

//--------------------------------------------------------------
/**
 * @brief 国連関連データのセット
 *
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
          work->UnObjCode[work->ObjNum] = tr_view;  //表示OBJコード格納 @todo ここでテーブル変換 
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
    else GF_ASSERT_MSG(0,"OBJ_IDX_ERROR %d",inObjIdx);
  }
  else GF_ASSERT_MSG(0,"OBJ_IDX_ERROR %d",inObjIdx);

  return rc;
}

