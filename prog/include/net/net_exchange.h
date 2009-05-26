//==============================================================================
/**
 * @file    net_exchange.h
 * @brief   Wi-Fi広場、マッチングで共通して使用する交換データのヘッダ
 * @author  matsuda
 * @date    2009.05.25(月)
 */
//==============================================================================
#pragma once

#include "savedata/save_control.h"
#include "savedata/mystatus.h"
#include "savedata/mystatus_local.h"
#include "savedata/wifihistory.h"


//==============================================================================
//  構造体定義
//==============================================================================
///Wi-Fiで共通した交換データ
typedef struct{
  DWCFriendData friendData;   //通信で渡す用のフレンドコード
  MYSTATUS mystatus;
  u8 mac[WM_SIZE_BSSID];  ///mac
  u8 my_nation;			///<自分の国
  u8 my_area;				///<自分の地域
}WIFI_EXCHANGE_WORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void NET_EXCHANGE_SetParam(SAVE_CONTROL_WORK *sv, WIFI_EXCHANGE_WORK *exc);
