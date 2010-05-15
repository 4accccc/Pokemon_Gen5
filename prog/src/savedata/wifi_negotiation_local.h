
#pragma once

//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _WIFI_NEGOTIATION_SAVEDATA{
  MYSTATUS aMyStatus[WIFI_NEGOTIATION_DATAMAX];
  u16 date[WIFI_NEGOTIATION_DATAMAX];         //交換日時
  PMS_DATA message;   //簡易文 自己紹介文
  u16 count;   //データ循環の為のカウンタ
  u16 num;  //自分の交換ポケモン数
};

