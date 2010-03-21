//======================================================================
/**
 * @file	  delivery_base.h
 * @brief	  配信の初期化構造体と基本情報
 * @author	k.ohno
 * @data	  10/02/28
 */
//======================================================================

#pragma once

#include "net/network_define.h"

#define DELIVERY_BEACON_ONCE_NUM  (84)  //ビーコン一回の送信サイズ
#define DELIVERY_BEACON_MAX_NUM  (17)    //DELIVERY_BEACON_MAX_NUM*DELIVERY_BEACON_ONCE_NUMバイトの送信が可能

#define DELIVERY_IRC_SEND_DATA_MAX  (7)   //日、英、フランス、イタリア、ドイツ、スペイン、韓国の７つ分
#define DELIVERY_BEACON_SEND_DATA_MAX  (7)   //日、英、フランス、イタリア、ドイツ、スペイン、韓国の７つ分ですが、ビーコン配信が遅くなるので、最高２つくらいを推奨

//IRC送信サイズ
//赤外線は内部で分割送信
#define DELIVERY_IRC_MAX_NUM  (DELIVERY_BEACON_ONCE_NUM*DELIVERY_BEACON_MAX_NUM)

typedef struct {
  int datasize;   //データ全体サイズ
  u8* pData;      //データ  受信バッファ 送信バッファ
  u32 LangCode;   //言語コード
  u32 version;    //バージョンのビットテーブル送信側（0xFFFFFFFFならばすべてのバージョンが受け取れる）
                                              //受信側　自分のバージョンのビット 1<<GET_VERSION()
} DELIVERY_DATA;

typedef struct {
  int NetDevID;   // //通信種類 network_define.h参照
  u8 ConfusionID;   //混線しないためのID 送信側で必要
  HEAPID heapID;    //HEAP

  DELIVERY_DATA   data[DELIVERY_BEACON_SEND_DATA_MAX];
  u32             dataNum;   //送信側は複数、受信側は１つだけ設定すること
} DELIVERY_BEACON_INIT;

typedef struct {
  int NetDevID;   // //通信種類  network_define.h参照
  u16 ConfusionID;   //混線しないためのID 送信側で必要
  HEAPID heapID;    //HEAPD
  DELIVERY_DATA   data[DELIVERY_IRC_SEND_DATA_MAX];
  u32             dataNum;   //送信側は複数、受信側は１つだけ設定すること
} DELIVERY_IRC_INIT;



