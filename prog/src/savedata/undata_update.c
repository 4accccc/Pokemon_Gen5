//==============================================================================
/**
 * @file    undata_update.c
 * @brief   国連データ押し出し処理
 * @author  saito
 * @date    2010.01.02.03
 */
//==============================================================================

#include "savedata/wifihistory_local.h"
#include "savedata/undata_update.h"

#define SAME_COUNTRY_DATA_MAX  (5)    //同じ国のデータは5件まで

static u32 GetDataNumByCountryCode(const UNITEDNATIONS_SAVE *un_data, const u32 inCountryCode);
static u32 GetDataIdxByCountryCode(const UNITEDNATIONS_SAVE *un_data, const u32 inCountryCode);
static u32 SearchSameData(const UNITEDNATIONS_SAVE *un_data, const MYSTATUS * inMyStatus);
static void  DelData(UNITEDNATIONS_SAVE *un_data, const u32 inDelIdx);
static void  AddData(WIFI_HISTORY * wh, UNITEDNATIONS_SAVE *add_data, const BOOL inTalkFlg);
static void  AddDataCore(UNITEDNATIONS_SAVE *un_data, UNITEDNATIONS_SAVE *add_data, const BOOL inTalkFlg);

//----------------------------------------------------------
/**
 * @brief	データ更新
 * @param	  wh        wifi履歴データポインタ
 * @param   add_data  追加するデータ
 *
 * @return	none
 */
//----------------------------------------------------------
void UNDATAUP_Update(WIFI_HISTORY * wh, UNITEDNATIONS_SAVE *add_data)
{
  UNITEDNATIONS_SAVE *un_data;
  u32 same_idx;

  un_data = WIFIHISTORY_GetUNDataPtr(wh);

  //同一人物検索
  same_idx = SearchSameData(un_data, &add_data->aMyStatus);
  if ( same_idx != UNITEDNATIONS_PEOPLE_MAX )
  {       //見つかった
    BOOL talk;
    //探し出したデータの会話状態をチェック
    if ( un_data[same_idx].bTalk ) talk = TRUE;   //会話している
    else talk = FALSE;                            //会話していない
    //削除
    DelData(un_data, same_idx);
    //リストの最後尾へ追加  自分の持っていたデータに存在していたので会話状態は引き継ぐ
    AddData(wh, add_data, talk);
  }
  else
  {       //見つからなかった
    u32 same_cont_num;
    u32 country_code;
    //追加データの国コードを取得
    country_code = MyStatus_GetMyNation(&add_data->aMyStatus);
    //追加データと同じ国コードを持つデータの数を取得
    same_cont_num = GetDataNumByCountryCode(un_data, country_code);
    if (same_cont_num < SAME_COUNTRY_DATA_MAX)
    {     //定数未満
      u32 total_num;
      //有効データ数取得
      total_num = UNDATAUP_GetDataNum(un_data);
      //有効データ件数がUNITEDNATIONS_PEOPLE_MAXか？
      if ( total_num >= UNITEDNATIONS_PEOPLE_MAX )
      {
        //リストの先頭のデータ(一番古いデータ)を削除
        DelData(un_data, 0);
      }
      //リストの最後尾へ追加　自分の持っていたデータには存在していなかったので会話フラグはFALSE確定
      AddData(wh, add_data, FALSE);
    }
    else //定数
    {
      u32 del_idx;
      //同じ国データの中で一番古いデータを消す
      del_idx = GetDataIdxByCountryCode(un_data, country_code);
      DelData(un_data, del_idx);
      //リストの最後尾へ追加  自分の持っていたデータには存在していなかったので会話フラグはFALSE確定
      AddData(wh, add_data, FALSE);
    }
  }
}

//----------------------------------------------------------
/**
 * @brief	有効データ数の取得
 * @param	  un_data   国連データの先頭ポインタ
 *
 * @return	u32   有効データ数
 */
//----------------------------------------------------------
u32 UNDATAUP_GetDataNum(const UNITEDNATIONS_SAVE *un_data)
{
  u32 i;
  for (i=0;i<UNITEDNATIONS_PEOPLE_MAX;i++)
  {
    if ( !un_data[i].valid ) break;
  }
  return i;
}

//----------------------------------------------------------
/**
 * @brief	指定国コードを持つデータの数を返す
 * @param	  un_data   国連データの先頭ポインタ
 * @param   inCountryCode   検索国コード
 *
 * @return	u32   件数(最大はSAME_COUNTRY_DATA_MAX)
 */
//----------------------------------------------------------
static u32 GetDataNumByCountryCode(const UNITEDNATIONS_SAVE *un_data, const u32 inCountryCode)
{
  u32 i;
  u32 num;
  num = 0;
  for (i=0;i<UNITEDNATIONS_PEOPLE_MAX;i++)
  {
    u32 code;
    if ( !un_data[i].valid ) break;   //有効データの末尾まで到達
    //データの国コードを取得
    code = MyStatus_GetMyNation(&un_data[i].aMyStatus);
    if ( code == inCountryCode ) num++;
  }

  if (num > SAME_COUNTRY_DATA_MAX)
  {
    GF_ASSERT_MSG(0,"num over %d",num);
    num = SAME_COUNTRY_DATA_MAX;
  }
  return num;
}

//----------------------------------------------------------
/**
 * @brief	指定国コードを持つ一番初めに見つかった(最も古い)データのインデックスを返す
 * @param	  un_data   国連データの先頭ポインタ
 * @param   inCountryCode   検索国コード
 *
 * @return	u32   一番初めに見つかったインデックス　未発見の場合はUNITEDNATIONS_PEOPLE_MAX
 */
//----------------------------------------------------------
static u32 GetDataIdxByCountryCode(const UNITEDNATIONS_SAVE *un_data, const u32 inCountryCode)
{
  u32 i;
  for (i=0;i<UNITEDNATIONS_PEOPLE_MAX;i++)
  {
    u32 code;
    if ( !un_data[i].valid ) break;   //有効データの末尾まで到達
    //データの国コードを取得
    code = MyStatus_GetMyNation(&un_data[i].aMyStatus);
    if ( code == inCountryCode ) return i;    //合致データ発見
  }
  return UNITEDNATIONS_PEOPLE_MAX;
}

//----------------------------------------------------------
/**
 * @brief	同一人物の検索する
 * @param	  un_data   国連データの先頭ポインタ
 * @param   inMyStatus    対象のマイステータス
 *
 * @return	u32   一番初めに見つかったインデックス　未発見の場合はUNITEDNATIONS_PEOPLE_MAX
 */
//----------------------------------------------------------
static u32 SearchSameData(const UNITEDNATIONS_SAVE *un_data, const MYSTATUS * inMyStatus)
{
  u32 i;
  u32 id;
  u32 country_code;
  id = MyStatus_GetID(inMyStatus);
  country_code = MyStatus_GetMyNation(inMyStatus);

  for (i=0;i<UNITEDNATIONS_PEOPLE_MAX;i++)
  {
    u32 tr_id;
    if ( !un_data[i].valid ) break;   //有効データの末尾まで到達
    //ID取得
    tr_id = MyStatus_GetID(&un_data[i].aMyStatus);
    if ( tr_id == id )
    {
      u32 code;
      //国コードを取得
      code = MyStatus_GetMyNation(&un_data[i].aMyStatus);
      if ( code == country_code ) return i;    //合致データ発見
    }
  }
  return UNITEDNATIONS_PEOPLE_MAX;
}

//----------------------------------------------------------
/**
 * @brief	指定位置のデータを削除してデータをつめる
 * @param	  un_data   国連データの先頭ポインタ
 * @param   inDelIdx     指定位置  0～UNITEDNATIONS_PEOPLE_MAX-1
 *
 * @return	none
 */
//----------------------------------------------------------
static void  DelData(UNITEDNATIONS_SAVE *un_data, const u32 inDelIdx)
{
  u32 i;

  if ( inDelIdx >= UNITEDNATIONS_PEOPLE_MAX )
  {
    GF_ASSERT_MSG(0,"idx %d",inDelIdx);
    return;
  }
  //有効データをつめる
  for(i=inDelIdx;i<(UNITEDNATIONS_PEOPLE_MAX-1);i++)
  {
    un_data[i] = un_data[i+1];
    if ( !un_data[i+1].valid ) break;
  }
  i++;
  //以降は無効データのはずなので無効状態にする
  for (;i<UNITEDNATIONS_PEOPLE_MAX;i++)
  {
    un_data[i].valid = 0;
  }
}

//----------------------------------------------------------
/**
 * @brief	指定位置のデータを削除してデータをつめる
 * @param	  wh          wifi履歴データポインタ
 * @param   add_data    追加する国連データ
 * @param   inTalkFlg   追加するデータの人を会話したか（自分が持っていたデータの更新の可能性があるので）
 *
 * @return	none
 */
//----------------------------------------------------------
static void  AddData(WIFI_HISTORY * wh, UNITEDNATIONS_SAVE *add_data, const BOOL inTalkFlg)
{
  UNITEDNATIONS_SAVE *un_data = WIFIHISTORY_GetUNDataPtr(wh);
  AddDataCore(un_data, add_data, inTalkFlg);
  //国ビット立てる
  {
    u32 code;
    //国コードを取得
    code = MyStatus_GetMyNation(&add_data->aMyStatus);
    WIFIHISTORY_SetCountryBit(wh, code);
  }
}


//----------------------------------------------------------
/**
 * @brief	指定位置のデータを削除してデータをつめる
 * @param	  un_data     国連データの先頭ポインタ
 * @param   add_data    追加する国連データ
 * @param   inTalkFlg   追加するデータの人を会話したか（自分が持っていたデータの更新の可能性があるので）
 *
 * @return	none
 */
//----------------------------------------------------------
static void  AddDataCore(UNITEDNATIONS_SAVE *un_data, UNITEDNATIONS_SAVE *add_data, const BOOL inTalkFlg)
{
  u32 last_idx;

  //有効データ件数を取得して末尾のインデックスを決定する
  last_idx = UNDATAUP_GetDataNum(un_data);

  if (last_idx >= UNITEDNATIONS_PEOPLE_MAX)
  {
    GF_ASSERT_MSG(0,"index over %d",last_idx);
    return;   //追加せずに戻る
  }

  //追加するデータ内で、初期化しなければならないメンバをここで初期化する
  if (inTalkFlg) add_data->bTalk = 1;     //話したか？ >>  話した
  else add_data->bTalk = 0;               //話したか？ >>  まだ話していない
  add_data->valid = 1;                    //有効データ？ >> 有効データ

  un_data[last_idx] = *add_data;
}
