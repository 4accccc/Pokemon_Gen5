//============================================================================================
/**
 * @file    comm_wifihistory.h
 * @brief   地球儀情報登録処理
 * @author  Akito Mori
 * @date    10.06.03
 */
//============================================================================================
#ifndef __COMM_WIFIHISTORY_H__
#define __COMM_WIFIHISTORY_H__


//==============================================================================
/**
 * @brief   相手のカートリッジリージョン・国・地域コードから地球儀情報を登録する
 *
 * @param   wifiHistory   ジオネットセーブデータ構造体
 * @param   Nation        国番号
 * @param   Area          地域番号
 * @param   langCode      対象のカセットコード
 *
 * @retval  none    
 */
//==============================================================================
static inline void Comm_WifiHistoryDataSet( WIFI_HISTORY *wifiHistory, int Nation, int Area, int langCode )
{
#if (PM_LANG == LANG_JAPAN)
  // 相手のカートリッジが日本なのに
  if(langCode==LANG_JAPAN){
    // 違う国の地球儀コードを入れてきたら登録しない
    if(Nation!=WIFI_NATION_JAPAN){
      return;
    }
  }
#endif
  
  // 国・地域を登録
    OS_Printf("no=%d, Nation=%d area=%d langCode=%d\n", WIFIHISTORY_GetStat(wifiHistory, Nation, Area ), Nation, Area, langCode );
  if(WIFIHISTORY_GetStat( wifiHistory, Nation, Area )==WIFIHIST_STAT_NODATA){
    WIFIHISTORY_SetStat( wifiHistory, Nation, Area, WIFIHIST_STAT_NEW );
  }
}

//=============================================================================================
/**
 * @brief   地球儀情報をMYSTATUSの配列から登録
 *
 * @param   wifiHistory ジオネット情報構造体
 * @param   status      MYSTATUSポインタ配列
 * @param   num         MYSTATUSの数
 */
//=============================================================================================
static inline void Comm_WifiHistoryCheck( WIFI_HISTORY *wifiHistory, const MYSTATUS *status[], int num )
{
  int i;
  int Nation,Area,region;

  // 接続している人数分まわす
  for(i=0;i<num;i++){

    // 入ってなかった時対策
    if(status[i]==NULL){
      continue;
    }

    // 国・地域コードを登録（ただし日本カートリッジ専用チェックあり）
    Nation = MyStatus_GetMyNation(status[i]);
    Area   = MyStatus_GetMyArea(status[i]);
    region = MyStatus_GetRegionCode(status[i]);
    Comm_WifiHistoryDataSet( wifiHistory, Nation, Area, region );
  }
}



#endif
