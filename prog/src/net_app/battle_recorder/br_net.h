//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_net.h
 *	@brief  バトルレコーダーネット処理
 *	@author	Toru=Nagihashi
 *	@date		2010.03.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "gamesystem/game_data.h"
#include "../gds/gds_rap.h"
#include "../gds/gds_rap_response.h"

#include "../lib/poke_net/poke_net/gds_header/gds_battle_rec_sub.h" //何故かこれincludeされてないので別個で呼ぶ
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	リクエスト
//=====================================
typedef enum
{
  BR_NET_REQUEST_MUSICAL_SHOT_UPLOAD,     //ミュージカルアップロード
  BR_NET_REQUEST_MUSICAL_SHOT_DOWNLOAD,   //ミュージカルダウンロード
  BR_NET_REQUEST_BATTLE_VIDEO_UPLOAD,     //ビデオアップロード
  BR_NET_REQUEST_BATTLE_VIDEO_DOWNLOAD,   //ビデオダウンロード
  BR_NET_REQUEST_VIDEO_SEARCH_DOWNLOAD,   //ビデオ検索ダウンロード
  BR_NET_REQUEST_NEW_RANKING_DOWNLOAD,    //最新ランキングダウンロード
  BR_NET_REQUEST_FAVORITE_RANKING_DOWLOAD,//人気ランキングダウンロード
  BR_NET_REQUEST_SUBWAY_RANKING_DOWNLOAD, //サブウェイランキングダウンロード
  BR_NET_REQUEST_FAVORITE_VIDEO_UPLOAD,   //お気に入りアップロード

  BR_NET_REQUEST_MAX
} BR_NET_REQUEST;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	バトルレコーダーネット処理
//=====================================
typedef struct _BR_NET_WORK BR_NET_WORK;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//--- system ---
extern BR_NET_WORK *BR_NET_Init( GAMEDATA *p_gamedata, DWCSvlResult *p_svl, HEAPID heapID );
extern void BR_NET_Exit( BR_NET_WORK *p_wk );
extern void BR_NET_Main( BR_NET_WORK *p_wk );


//--- down/up load ---
typedef struct 
{
  u16 monsno;
  u8 country_code;
  u8 local_code;
  BATTLEMODE_SEARCH_NO battle_mode_no;
} BR_NET_VIDEO_SEARCH_DATA;

typedef union
{ 
  //BR_NET_REQUEST_MUSICAL_SHOT_UPLOAD,     //ミュージカルアップロード
  const MUSICAL_SHOT_DATA   *cp_upload_musical_shot_data;
  //BR_NET_REQUEST_MUSICAL_SHOT_DOWNLOAD,   //ミュージカルダウンロード
  u16                       download_musical_shot_search_monsno;
  //BR_NET_REQUEST_BATTLE_VIDEO_UPLOAD,     //ビデオアップロード
  /* none */
  //BR_NET_REQUEST_BATTLE_VIDEO_DOWNLOAD,   //ビデオダウンロード
  u64                       download_video_number; 
  //BR_NET_REQUEST_VIDEO_SEARCH_DOWNLOAD,   //ビデオ検索ダウンロード
  BR_NET_VIDEO_SEARCH_DATA  download_video_search_data;
  //BR_NET_REQUEST_NEW_RANKING_DOWNLOAD,    //最新ランキングダウンロード
  /* none */
  //BR_NET_REQUEST_FAVORITE_RANKING_DOWLOAD,//人気ランキングダウンロード
  /* none */
  //BR_NET_REQUEST_SUBWAY_RANKING_DOWNLOAD, //サブウェイランキングダウンロード
  /* none */
  //BR_NET_REQUEST_FAVORITE_VIDEO_UPLOAD,   //お気に入りアップロード
  u64                       upload_favorite_video_number;
}BR_NET_REQUEST_PARAM;

extern void BR_NET_StartRequest( BR_NET_WORK *p_wk, BR_NET_REQUEST type, const BR_NET_REQUEST_PARAM *cp_param );
extern BOOL BR_NET_WaitRequest( BR_NET_WORK *p_wk );

//--- get download data ---
extern BOOL BR_NET_GetDownloadMusicalShot( BR_NET_WORK *p_wk, MUSICAL_SHOT_RECV **pp_data_tbl, int tbl_max, int *p_recv_num );
extern BOOL BR_NET_GetUploadBattleVideoNumber( BR_NET_WORK *p_wk, u64 *p_video_number );
extern BOOL BR_NET_GetDownloadBattleVideo( BR_NET_WORK *p_wk, BATTLE_REC_RECV **pp_data, int *p_video_number );
extern BOOL BR_NET_GetDownloadBattleSearch( BR_NET_WORK *p_wk, BATTLE_REC_OUTLINE_RECV *p_data_tbl, int tbl_max, int *p_recv_num );
extern BOOL BR_NET_GetDownloadNewRanking( BR_NET_WORK *p_wk, BATTLE_REC_OUTLINE_RECV *p_data_tbl, int tbl_max, int *p_recv_num );
extern BOOL BR_NET_GetDownloadFavoriteRanking( BR_NET_WORK *p_wk, BATTLE_REC_OUTLINE_RECV *p_data_tbl, int tbl_max, int *p_recv_num );
extern BOOL BR_NET_GetDownloadSubwayRanking( BR_NET_WORK *p_wk, BATTLE_REC_OUTLINE_RECV *p_data_tbl, int tbl_max, int *p_recv_num );

//----エラー
typedef enum
{ 
  BR_NET_ERR_RETURN_NONE,         //エラーが起きていない
  BR_NET_ERR_RETURN_ONCE,         //１つ前に戻る
  BR_NET_ERR_RETURN_TOPMENU,      //トップメニューへ戻る
  BR_NET_ERR_RETURN_DISCONNECT,   //切断する（このエラーはバトルビデオ送信時のサーバーレスポンスエラーでしかこない）
}BR_NET_ERR_RETURN;
extern BR_NET_ERR_RETURN BR_NET_GetError( BR_NET_WORK *p_wk, int *p_msg_no );

typedef enum
{
  BR_NET_SYSERR_RETURN_NONE,
  BR_NET_SYSERR_RETURN_LIGHT,
  BR_NET_SYSERR_RETURN_DISCONNECT,
} BR_NET_SYSERR_RETURN;

extern BR_NET_SYSERR_RETURN BR_NET_GetSysError( BR_NET_WORK *p_wk );
extern BOOL BR_NET_IsLastDisConnectError( const BR_NET_WORK *cp_wk );

