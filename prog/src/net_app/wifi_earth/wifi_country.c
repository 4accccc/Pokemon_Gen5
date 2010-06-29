//============================================================================================
/**
 * @file  wifi_country.c
 * @brief WI-FI地球画面で使用する文字列等のデータを外部公開するための仕組み
 * @author  taya
 * @date  2006.04.22
 */
//============================================================================================
#include <gflib.h>

#include "message.naix"
#include "msg/msg_wifi_place_msg_world.h"
#include "wifi_earth_place.naix"

#include "net_app\wifi_country.h"

#include "msg/msg_wifi_place_msg_ARG.h"
#include "msg/msg_wifi_place_msg_AUS.h"
#include "msg/msg_wifi_place_msg_BRA.h"
#include "msg/msg_wifi_place_msg_CAN.h"
#include "msg/msg_wifi_place_msg_CHN.h"
#include "msg/msg_wifi_place_msg_DEU.h"
#include "msg/msg_wifi_place_msg_ESP.h"
#include "msg/msg_wifi_place_msg_FIN.h"
#include "msg/msg_wifi_place_msg_FRA.h"
#include "msg/msg_wifi_place_msg_GBR.h"
#include "msg/msg_wifi_place_msg_IND.h"
#include "msg/msg_wifi_place_msg_ITA.h"
#include "msg/msg_wifi_place_msg_JPN.h"
#include "msg/msg_wifi_place_msg_NOR.h"
#include "msg/msg_wifi_place_msg_POL.h"
#include "msg/msg_wifi_place_msg_RUS.h"
#include "msg/msg_wifi_place_msg_SWE.h"
#include "msg/msg_wifi_place_msg_USA.h"

//データ構造体（国地域テーブルデータ）
typedef struct EARTH_AREATABLE_tag
{
  u8    nationID;
  u8    place_dataID;
  u16   msg_arcID;
  
  u8    place_index_max;    ///<地域名の最大数
  u8    padding[3];   //パディング
}EARTH_AREATABLE;

static const EARTH_AREATABLE NationFlag_to_AreaID[] = {
  {//ダミー
    country000,
    NARC_wifi_earth_place_place_pos_wrd_dat,
    NARC_message_wifi_place_msg_world_dat,
    0
  },
  { // アルゼンチン
    country009,
    NARC_wifi_earth_place_place_pos_ARG_dat,
    NARC_message_wifi_place_msg_ARG_dat,
    ARG24
  },
  { // オーストラリア
    country012,
    NARC_wifi_earth_place_place_pos_AUS_dat,
    NARC_message_wifi_place_msg_AUS_dat,
    AUS08
  },
  { // ブラジル
    country028,
    NARC_wifi_earth_place_place_pos_BRA_dat,
    NARC_message_wifi_place_msg_BRA_dat,
    BRA27
  },
  { // カナダ
    country036,
    NARC_wifi_earth_place_place_pos_CAN_dat,
    NARC_message_wifi_place_msg_CAN_dat,
    CAN13
  },
  { // 中国
    country043,
    NARC_wifi_earth_place_place_pos_CHN_dat,
    NARC_message_wifi_place_msg_CHN_dat,
    CHN33
  },
  { // ドイツ
    country079,
    NARC_wifi_earth_place_place_pos_DEU_dat,
    NARC_message_wifi_place_msg_DEU_dat,
    DEU16
  },
  { // スペイン
    country195,
    NARC_wifi_earth_place_place_pos_ESP_dat,
    NARC_message_wifi_place_msg_ESP_dat,
    ESP17
  },
  { // フィンランド
    country072,
    NARC_wifi_earth_place_place_pos_FIN_dat,
    NARC_message_wifi_place_msg_FIN_dat,
    FIN06
  },
  { // フランス
    country073,
    NARC_wifi_earth_place_place_pos_FRA_dat,
    NARC_message_wifi_place_msg_FRA_dat,
    FRA22
  },
  { // イギリス
    country218,
    NARC_wifi_earth_place_place_pos_GBR_dat,
    NARC_message_wifi_place_msg_GBR_dat,
    GBR12
  },
  { // インド
    country095,
    NARC_wifi_earth_place_place_pos_IND_dat,
    NARC_message_wifi_place_msg_IND_dat,
    IND35
  },
  { // イタリア
    country102,
    NARC_wifi_earth_place_place_pos_ITA_dat,
    NARC_message_wifi_place_msg_ITA_dat,
    ITA20
  },
  { // 日本
    country105,
    NARC_wifi_earth_place_place_pos_JPN_dat,
    NARC_message_wifi_place_msg_JPN_dat,
    JPN50
  },
  { // ノルウェー
    country155,
    NARC_wifi_earth_place_place_pos_NOR_dat,
    NARC_message_wifi_place_msg_NOR_dat,
    NOR22
  },
  { // ポーランド
    country166,
    NARC_wifi_earth_place_place_pos_POL_dat,
    NARC_message_wifi_place_msg_POL_dat,
    POL16
  },
  { // ロシア
    country174,
    NARC_wifi_earth_place_place_pos_RUS_dat,
    NARC_message_wifi_place_msg_RUS_dat,
    RUS08
  },
  { // スウェーデン
    country200,
    NARC_wifi_earth_place_place_pos_SWE_dat,
    NARC_message_wifi_place_msg_SWE_dat,
    SWE22
  },
  { // アメリカ
    country220,
    NARC_wifi_earth_place_place_pos_USA_dat,
    NARC_message_wifi_place_msg_USA_dat,
    USA51
  },
};




//------------------------------------------------------------------
/**
 * データ長を取得
 *
 * @retval  u32   
 */
//------------------------------------------------------------------
u32 WIFI_COUNTRY_GetDataLen( void )
{
  return NELEMS(NationFlag_to_AreaID);
}

//------------------------------------------------------------------
/**
 * 国コードをデータインデックスに変換
 *
 * データインデックスは、各種付随データを引っ張るために使用する。
 * データインデックスが０の時、その国のローカル地域データは存在しない。
 *
 * @param   countryCode   国コード
 *
 * @retval  u32   内部データインデックス
 */
//------------------------------------------------------------------
u32 WIFI_COUNTRY_CountryCodeToDataIndex( u32 countryCode )
{
  u32 i;

  for(i=0; i<NELEMS(NationFlag_to_AreaID); i++)
  {
    if(NationFlag_to_AreaID[i].nationID == countryCode)
    {
      return i;
    }
  }

  return 0;

}

//------------------------------------------------------------------
/**
 * 国コードから地域数を取得
 *
 * @param   countryCode   国コード
 *
 * @retval  地域数(地域が存在しない国の場合は0)
 */
//------------------------------------------------------------------
u32 WIFI_COUNTRY_CountryCodeToPlaceIndexMax( u32 countryCode )
{
  u32 i;

  for(i=0; i<NELEMS(NationFlag_to_AreaID); i++)
  {
    if(NationFlag_to_AreaID[i].nationID == countryCode)
    {
      return NationFlag_to_AreaID[i].place_index_max;
    }
  }

  return 0;

}

//------------------------------------------------------------------
/**
 * 国コードからローカル地域名文字列のメッセージデータIDを取得
 *
 * @param   countryCode   国コード
 *
 * @retval  u32   地域名文字列のメッセージデータID（ARC_MSG内）
 */
//------------------------------------------------------------------
u32 WIFI_COUNTRY_CountryCodeToPlaceMsgDataID( u32 countryCode )
{
  u32 dataIndex = WIFI_COUNTRY_CountryCodeToDataIndex( countryCode );
  return WIFI_COUNTRY_DataIndexToPlaceMsgDataID( dataIndex );
}


//------------------------------------------------------------------
/**
 * データインデックスからローカル地域名文字列のメッセージデータIDを取得
 *
 * @param   datID データインデックス
 *
 * @retval  u32   地域名文字列のメッセージデータID（ARC_MSG内）
 */
//------------------------------------------------------------------
u32 WIFI_COUNTRY_DataIndexToPlaceMsgDataID( u32 dataIndex )
{
  GF_ASSERT(dataIndex < NELEMS(NationFlag_to_AreaID));

  return NationFlag_to_AreaID[dataIndex].msg_arcID;
}

//------------------------------------------------------------------
/**
 * データインデックスから国コードを取得
 *
 * @param   dataIndex   データインデックス
 *
 * @retval  u32   国コード
 */
//------------------------------------------------------------------
u32 WIFI_COUNTRY_DataIndexToCountryCode( u32 dataIndex )
{
  GF_ASSERT(dataIndex < NELEMS(NationFlag_to_AreaID));

  return NationFlag_to_AreaID[dataIndex].nationID;
}



//------------------------------------------------------------------
/**
 * データインデックスから地域データIDを取得
 *
 * @param   dataIndex   データインデックス
 *
 * @retval  u32   地域データID
 */
//------------------------------------------------------------------
u32 WIFI_COUNTRY_DataIndexToPlaceDataID( u32 dataIndex )
{
  GF_ASSERT(dataIndex < NELEMS(NationFlag_to_AreaID));

  return NationFlag_to_AreaID[dataIndex].place_dataID;
}

//NationFlag_to_AreaIDでの日本が置いてあるテーブル
#define NATION_AREA_TABLE_JAPAN_INDEX ( 13 )

//------------------------------------------------------------------
/**
 * 国・地域関連付けテーブルから日本のIDを取得（データ更新時にズレないように）
 *
 * @retval  u32   日本の国ID
 */
//------------------------------------------------------------------
u32 WIFI_COUNTRY_GetJapanID(void)
{
  return NationFlag_to_AreaID[NATION_AREA_TABLE_JAPAN_INDEX].nationID;
}
