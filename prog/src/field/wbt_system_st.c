//============================================================================================
/**
 * @file wbt_system_st.c
 * @brief WBTでどうしてもフィールド常駐に配置しないといけないモジュール
 * @author  tamada
 * @date  2011.09.09
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"
#include "wbt_system_local.h"

#include "wbt_tool.h"

#include "system/main.h"
#include "arc/message.naix"
#include "msg/msg_wbt_system.h"
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief トーナメント毎の固有データ
 */
//------------------------------------------------------------------
const WBT_TOURNAMENT_DATA WBTTournamentData[ WBTCUP_MAX - 1 ] = {
  { //WBTCUP_CHAMPION, //チャンピオントーナメント
    msg_wbtsys_title_03,
    WBTWIN_COUNT_CHAMPION,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 0, 0, 0, 0 },
  },
  { //WBTCUP_POKETYPE, //タイプエキスパートトーナメント
    msg_wbtsys_title_16,
    POKETYPE_NORMAL,   //ダミー
    WBTRECEPT_2, WBTREG_POKETYPE,
    WBTBTDATA_3,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_DOWNLOAD, //ダウンロードトーナメント
    msg_wbtsys_title_04,
    WBTWIN_COUNT_MAX,   //ダミー
    WBTRECEPT_1, WBTREG_DOWNLOAD,
    WBTBTDATA_1,        //現在WBTROM_TRID_BATTLE_MOB_STARTを指定しているため、ここのデータ
    { 2, 2, 3, 3 },
  },

  {//WBTCUP_HODOMOE,     //ホドモエトーナメント           
    msg_wbtsys_title_05,
    WBTWIN_COUNT_HODOMOE,
    WBTRECEPT_1, WBTREG_ALL,
    WBTBTDATA_1,
    { 1, 1, 2, 2 },
  },
  {//WBTCUP_ISSYU,       //イッシュリーダーズトーナメント 
    msg_wbtsys_title_06,
    WBTWIN_COUNT_ISSYU,
    WBTRECEPT_2, WBTREG_ALL,
    WBTBTDATA_2,
    { 1, 1, 2, 2 },
  },
  {//WBTCUP_KANTO,       //カントーリーダーズトーナメント 
    msg_wbtsys_title_07,
    WBTWIN_COUNT_KANTO,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_2,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_JOUTO,       //ジョウトリーダーズトーナメント 
    msg_wbtsys_title_08,
    WBTWIN_COUNT_JOUTO,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_2,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_HOUEN,       //ホウエンリーダーズトーナメント 
    msg_wbtsys_title_09,
    WBTWIN_COUNT_HOUEN,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_2,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_SINOU,       //シンオウリーダーズトーナメント 
    msg_wbtsys_title_10,
    WBTWIN_COUNT_SINOU,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_2,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_WORLD,       //ワールドトーナメント     
    msg_wbtsys_title_11,
    WBTWIN_COUNT_WORLD,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 0, 0, 0, 0 },
  },
  {//WBTCUP_HODOMOE_EVENT, //ホドモエトーナメント（イベント）           
    msg_wbtsys_title_05,
    WBTWIN_COUNT_HODOMOE,
    WBTRECEPT_1, WBTREG_ALL,
    WBTBTDATA_1,
    { 1, 0, 0, 0 },
  },
  {//WBTCUP_RENTAL,         //レンタルトーナメント
    msg_wbtsys_title_12,
    WBTWIN_COUNT_RENTAL,
    WBTRECEPT_1, WBTREG_SUBWAY,
    WBTBTDATA_1,
    { 1, 1, 2, 2 },
  },
  {//WBTCUP_RENTALMASTER,         //レンタルマスタートーナメント
    msg_wbtsys_title_13,
    WBTWIN_COUNT_RENTALMASTER,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_MIX,                //ミックストーナメント
    msg_wbtsys_title_14,
    WBTWIN_COUNT_SHUFFLE,
    WBTRECEPT_1, WBTREG_ALL,
    WBTBTDATA_1,
    { 1, 1, 2, 2 },
  },
  {//WBTCUP_MIXMASTER,         //ミックスマスタートーナメント
    msg_wbtsys_title_15,
    WBTWIN_COUNT_SHUFFLEMASTER,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 2, 2, 3, 3 },
  },

};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief トーナメント固有データを取得
 * @param[in]   wbtcup
 * @return  WBT_TOURNAMENT_DATA 各トーナメントに対応した固有データ
 */
//------------------------------------------------------------------
const WBT_TOURNAMENT_DATA * WBTTOOL_GetTournamentData( WBTCUP wbtcup )
{
  if ( wbtcup == WBTCUP_NULL || wbtcup >= WBTCUP_MAX )
  {
    GF_ASSERT_MSG( 0, "wbtcup error:%d\n", wbtcup );
    wbtcup = WBTCUP_HODOMOE;
  }
  return & WBTTournamentData[ wbtcup - 1 ];
}

//------------------------------------------------------------------
/**
 * @brief トーナメント名文字列をセット
 * @param[in]   heapID
 * @param[in]   wbtcup
 * @param[out]  strbuf
 * @note
 * この関数でダウンロードトーナメントのダウンロードデータ名は反映されない。
 * そちらが必要な場合はWBTSYS_GetTournamentNameを使用すること
 */
//------------------------------------------------------------------
void WBTTOOL_GetTournamentName( HEAPID heapID, WBTCUP wbtcup, STRBUF * strbuf )
{
  GFL_MSGDATA     *man;
  u16 msg_id;

  msg_id = WBTTOOL_GetTournamentData( wbtcup )->title_id;
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_system_dat, heapID );
  if( man )
  {
    GFL_MSG_GetString( man, msg_id, strbuf );
    GFL_MSG_Delete( man );
  }
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief WBTの観客タイプを取得する
 * @param[in] wbtsys
 * @return  u8  観客タイプの指定
 *
 * 通常のWBTモジュールOverlayがロードされる前に参照するため、
 * フィールド常駐部分に配置している
 */
//------------------------------------------------------------------
u8 WBTSYS_GetAudienceType( const WBTSYS * wbtsys )
{
  return wbtsys->audience_type;
}

