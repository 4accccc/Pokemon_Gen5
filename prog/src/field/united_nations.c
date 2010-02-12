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
#include "fieldmap/fldmmdl_objcode.h"

#include "system/main.h"   //for HEAPID

#define UNDATA_MAX  (UNITEDNATIONS_PEOPLE_MAX)

#define UN_SETNUM (12)

#define UN_NATURE_MSG_MARGINE (6)   //各性格ごとに6メッセージ
#define UN_NATURE_MSG_MARGINE_TOTAL (UN_NATURE_MSG_MARGINE*2)   //各性格ごとに6メッセージｘ性別

static void SetData(WIFI_HISTORY *wh, UNSV_WORK* work, const int inCountryCode, const u32 inFloor);
static u32 GetMsg(const int inSex, const u32 inTrID,
                  const u32 inNature, const BOOL inFirst, const BOOL inPlace);
static void SetWords(WORDSET *wordset, GAMEDATA *gdata, const u32 inDataIdx);

#ifdef PM_DEBUG
int DebugUnCountryCode = 0;
#endif

//--------------------------------------------------------------
/**
 * @brief 国連関連データのセット
 * @param inCountryCode   国コード
 * @param inFloor         フロア
 * @retval  none
 */
//--------------------------------------------------------------
void UN_SetData(GAMESYS_WORK *gsys, const int inCountryCode, const u32 inFloor)
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
    SetData(wh, work, code, inFloor);
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
 * @param inFloor           フロア
 * @retval  none
 */
//--------------------------------------------------------------
static void SetData(WIFI_HISTORY *wh, UNSV_WORK* work, const int inCountryCode, const u32 inFloor)
{
  int i;
  
  work->TargetCountryCode = inCountryCode;
  work->Floor = inFloor;
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
          if (tr_view > UNION_VIEW_INDEX_MAX)
          {
            GF_ASSERT_MSG(0,"tr_view = %d",tr_view);
            if ( MyStatus_GetMySex(my) ) tr_view = GIRL2;
            else tr_view = BOY2;
          }
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
u32 UN_GetRoomObjMsg(WORDSET *wordset, GAMESYS_WORK *gsys, UNSV_WORK* work, const int inObjIdx, const BOOL inFirstMsg)
{
  int undata_idx;
  int sex;
  u32 id;
  int nature;
  BOOL place_flg = FALSE;

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
  //ワードセット関連処理
  SetWords(wordset, gdata, undata_idx);
  {
    MYSTATUS *my;
    my = WIFIHISTORY_GetUnMyStatus(wh, undata_idx);
    //性別取得
    sex = MyStatus_GetMySex(my);
    //性格取得
    nature = WIFIHISTORY_GetUnInfo(wh, undata_idx, UN_INFO_NATURE);
    //トレーナーID取得
    id = MyStatus_GetID(my);

    //地域あるか？
    {
      u32 nation;
      u32 place;
      UNSV_WORK *unsv;
      unsv = GAMEDATA_GetUnsvWorkPtr(gdata);
      nation = unsv->TargetCountryCode;
      place = WIFI_COUNTRY_CountryCodeToPlaceIndexMax( nation );
      if(place > 0) place_flg = TRUE;
    }
  }
  //メッセージ決定
  return GetMsg(sex, id, nature, inFirstMsg, place_flg);

}

//--------------------------------------------------------------
/**
 * @brief OBJの話すメッセージの決定
 * @param inSex         性別 0 or 1
 * @param inTrID        トレーナーID
 * @param inNature      性格0～4
 * @param inFirst       取得したいのは初回メッセージか？
 * @param inPlace       地域あるか
 * @retval  u32     メッセージID
 */
//--------------------------------------------------------------
static u32 GetMsg(const int inSex, const u32 inTrID,
                  const u32 inNature, const BOOL inFirst, const BOOL inPlace)
{
  u32 base;
  u32 msg;
  //性格でメッセージインデックスの基点を決める

  if (inPlace) base = msg_un_default_man_00;    //地域あり
  else base = msg_un_default_man_06;       //地域無し
  
  if ( inNature < 5 ) msg = base + (inNature*UN_NATURE_MSG_MARGINE_TOTAL);
  else
  {
    GF_ASSERT_MSG(0,"nature error %d",inNature);
    msg = base;
  }
  
  //性別でオフセットを計算
  if (inSex) msg += UN_NATURE_MSG_MARGINE;//女性
  //初回会話か？
  if ( !inFirst )      //2回目以降
  {
    msg += 1;
    //トレーナIDを5で割った余りを基点に足しこむ
    msg += (inTrID % 5);
  }

  return msg;
}

//--------------------------------------------------------------
/**
 * @brief メッセージタグ展開
 * @param wordset         ワードセットポインタ
 * @param gdata           ゲームデータポインタ
 * @param inDataIdx       国連データインデックス0～19
 * @retval  none
 */
//--------------------------------------------------------------
static void SetWords(WORDSET *wordset, GAMEDATA *gdata, const u32 inDataIdx)
{
  HEAPID  heapID;
  u32 monsno;
  WIFI_HISTORY *wh;
  UNSV_WORK *unsv;
  u32 nation;
  MYSTATUS *npc_my;

  {
    SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
    wh = SaveData_GetWifiHistory(sv);
    unsv = GAMEDATA_GetUnsvWorkPtr(gdata);
    nation = unsv->TargetCountryCode;
  }

  npc_my = WIFIHISTORY_GetUnMyStatus(wh, inDataIdx);

  heapID = HEAPID_FIELDMAP;

  //0番に国名セット
  WORDSET_RegisterCountryName( wordset, 0, nation );
  //1番に地域名セット
  {
    //指定した国の地域数を取得
    u32 count = WIFI_COUNTRY_CountryCodeToPlaceIndexMax( nation );
    if (count != 0)
    {
      u32 place;
      place = MyStatus_GetMyArea(npc_my);
      WORDSET_RegisterLocalPlaceName( wordset, 1, nation, place );
    }
  }
  //3番にプレーヤー名をセット
  {
    MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gdata );
    WORDSET_RegisterPlayerName( wordset, 3, mystatus );
  }
  //5番にNPCがプレーヤーからもらったポケモンをセット
  {
    monsno = WIFIHISTORY_GetUnInfo(wh, inDataIdx, UN_INFO_RECV_POKE);
    WORDSET_RegisterPokeMonsNameNo( wordset, 5, monsno );
  }
  //6番にプレーヤーの趣味をセット  @todo
  ;
  //7番にNPCトレーナー名をセット
  {
    WORDSET_RegisterPlayerName( wordset, 3, npc_my );
  }
  //8番にNPC交換国数をセット
  {
    int keta = 3;
    int number;
    number = WIFIHISTORY_GetUnInfo(wh, inDataIdx, UN_INFO_COUNTRY_NUM);
    WORDSET_RegisterNumber( wordset, 8, number, keta, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  }
  //9番にNPCがプレーヤーにあげたポケモンをセット
  {
    monsno = WIFIHISTORY_GetUnInfo(wh, inDataIdx, UN_INFO_SEND_POKE);
    WORDSET_RegisterPokeMonsNameNo( wordset, 9, monsno );
  }
  //10番にNPCの趣味をセット @todo
  ;
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
