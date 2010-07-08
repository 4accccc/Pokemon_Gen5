//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_net.c
 *	@brief  バトルレコーダーネット処理
 *	@author	Toru=Nagihashi
 *	@data		2010.03.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"
#include "system/net_err.h"
#include "net/dwc_error.h"

//サウンド（エラー時強制SE停止をするため）
#include "sound/pm_sndsys.h"

//アーカイブ(エラーメッセージ用)
#include "msg/msg_battle_rec.h"
#include "msg/msg_wifi_system.h"

//外部公開
#include "br_net.h"

FS_EXTERN_OVERLAY(gds_comm);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#ifdef PM_DEBUG
#define DEBUG_BR_NET_PRINT_ON //担当者のみのプリントON
#endif //PM_DEBUG

//担当者のみのPRINTオン
#ifdef DEBUG_BR_NET_PRINT_ON
#if defined( DEBUG_ONLY_FOR_toru_nagihashi )
//#define BR_NET_Printf(...)  OS_TFPrintf(1,__VA_ARGS__)
#elif defined( DEBUG_ONLY_FOR_shimoyamada )
#define BR_NET_Printf(...)  OS_TPrintf(__VA_ARGS__)
#endif  //defined
#endif //DEBUG_BR_NET_PRINT_ON
//定義されていないときは、なにもない
#ifndef BR_NET_Printf
#define BR_NET_Printf(...)  /*  */ 
#endif //BR_NET_Printf

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	シーケンスワーク
//=====================================
typedef struct _BR_NET_SEQ_WORK BR_NET_SEQ_WORK;

//-------------------------------------
///	バトルレコーダーネット処理
//=====================================
struct _BR_NET_WORK
{ 
  
  GDS_RAP_WORK        gdsrap;         ///<GDSライブラリ、NitroDWC関連のワーク構造体
  GDS_PROFILE_PTR     p_gds_profile;  ///<GDSのプロフィール
  BR_NET_SEQ_WORK     *p_seq;         ///<状態管理

  BR_NET_REQUEST_PARAM  reqest_param;    ///<リクエストされた引数
  u32                   response_flag[BR_NET_REQUEST_MAX];  ///<レスポンスを受けたかどうかのフラグ
  BOOL                is_last_disconnect_error;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//状態関数
static void Br_Net_Seq_Nop( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_WaitReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_UploadMusicalShotReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_DownloadMusicalShotReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_UploadBattleVideoReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_DownloadBattleVideoReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_DownloadBattleSearchReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_DownloadNewRankingReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_DownloadFavoriteRankingReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_DownloadSubwayRankingReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_Net_Seq_UploadFavoriteBattleReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//レスポンス関数
static void Br_Net_Response_MusicalRegist(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info);
static void Br_Net_Response_MusicalGet(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info);
static void Br_Net_Response_BattleVideoRegist(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info);
static void Br_Net_Response_BattleVideoSearch(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info);
static void Br_Net_Response_BattleVideoDataGet(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info);
static void Br_Net_Response_BattleVideoFavorite(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info);



//-------------------------------------
///	シーケンスシステム
//=====================================
typedef void (*BR_NET_SEQ_FUNCTION)( BR_NET_SEQ_WORK *p_wk, int *p_seq, void *p_wk_adrs );

static BR_NET_SEQ_WORK *BR_NET_SEQ_Init( void *p_wk_adrs, BR_NET_SEQ_FUNCTION seq_function, HEAPID heapID );
static void BR_NET_SEQ_Exit( BR_NET_SEQ_WORK *p_wk );
static void BR_NET_SEQ_Main( BR_NET_SEQ_WORK *p_wk );
static BOOL BR_NET_SEQ_IsEnd( const BR_NET_SEQ_WORK *cp_wk );
static void BR_NET_SEQ_SetNext( BR_NET_SEQ_WORK *p_wk, BR_NET_SEQ_FUNCTION seq_function );
static void BR_NET_SEQ_End( BR_NET_SEQ_WORK *p_wk );
static void BR_NET_SEQ_SetReservSeq( BR_NET_SEQ_WORK *p_wk, int seq );
static void BR_NET_SEQ_NextReservSeq( BR_NET_SEQ_WORK *p_wk );
static BOOL BR_NET_SEQ_IsComp( const BR_NET_SEQ_WORK *cp_wk, BR_NET_SEQ_FUNCTION seq_function );




//----------------------------------------------------------------------------
/**
 *	@brief  バトルレコーダー通信  初期化
 *
 *  @param  GAMEDATA        ゲームデータ
 *  @param  DWCSvlResult    サービスロケータ
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BR_NET_WORK *BR_NET_Init( GAMEDATA *p_gamedata, DWCSvlResult *p_svl, HEAPID heapID )
{ 
  BR_NET_WORK *p_wk;
  
  GFL_OVERLAY_Load( FS_OVERLAY_ID(gds_comm) );

  p_wk = GFL_HEAP_AllocMemory( heapID, sizeof(BR_NET_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BR_NET_WORK) );

  //GDS初期化
  { 
    GDSRAP_INIT_DATA gdsrap_data;
    GFL_STD_MemClear( &gdsrap_data, sizeof(GDSRAP_INIT_DATA) );

    gdsrap_data.gamedata      = p_gamedata;
    gdsrap_data.gs_profile_id = MyStatus_GetProfileID( GAMEDATA_GetMyStatus( p_gamedata ) );
    gdsrap_data.pSvl          = p_svl;
    gdsrap_data.response_callback.callback_work                      = p_wk;
    gdsrap_data.response_callback.func_musicalshot_regist            = Br_Net_Response_MusicalRegist;
    gdsrap_data.response_callback.func_musicalshot_get               = Br_Net_Response_MusicalGet;
    gdsrap_data.response_callback.func_battle_video_regist           = Br_Net_Response_BattleVideoRegist;
    gdsrap_data.response_callback.func_battle_video_search_get       = Br_Net_Response_BattleVideoSearch;
    gdsrap_data.response_callback.func_battle_video_data_get         = Br_Net_Response_BattleVideoDataGet;
    gdsrap_data.response_callback.func_battle_video_favorite_regist  = Br_Net_Response_BattleVideoFavorite;

    GDSRAP_Init(&p_wk->gdsrap, &gdsrap_data);  //通信ライブラリ初期化
  }

  //GDSプロフィール
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );

    p_wk->p_gds_profile = GDS_Profile_AllocMemory( heapID );
    GDS_Profile_MyDataSet( p_wk->p_gds_profile, p_gamedata );
  }

  //状態管理
  { 
    p_wk->p_seq = BR_NET_SEQ_Init( p_wk, Br_Net_Seq_Nop, heapID );
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルレコーダー通信  破棄
 *
 *	@param	BR_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_NET_Exit( BR_NET_WORK *p_wk )
{ 
  BR_NET_SEQ_Exit( p_wk->p_seq );

  GDS_Profile_FreeMemory( p_wk->p_gds_profile );

  GDSRAP_Exit(&p_wk->gdsrap);
  GFL_HEAP_FreeMemory( p_wk );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(gds_comm) );

  NAGI_Printf( "BR_NET解放\n" );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルレコーダー通信  メイン処理
 *
 *	@param	BR_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_NET_Main( BR_NET_WORK *p_wk )
{ 
  BR_NET_SEQ_Main( p_wk->p_seq );
  GDSRAP_Main(&p_wk->gdsrap);
}
//=============================================================================
/**
 *    リクエスト
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  リクエスト開始
 *
 *	@param	BR_NET_WORK *p_wk                 ワーク
 *	@param	type                              リクエストの種類
 *	@param	BR_NET_REQUEST_PARAM *cp_param    引数
 */
//-----------------------------------------------------------------------------
void BR_NET_StartRequest( BR_NET_WORK *p_wk, BR_NET_REQUEST type, const BR_NET_REQUEST_PARAM *cp_param )
{ 
  GF_ASSERT( type < BR_NET_REQUEST_MAX );

  if( cp_param )
  { 
    p_wk->reqest_param  = *cp_param;
  }

  GFL_STD_MemClear( p_wk->response_flag, sizeof(u32)*BR_NET_REQUEST_MAX );

  switch( type )
  { 
  case BR_NET_REQUEST_MUSICAL_SHOT_UPLOAD:     //ミュージカルアップロード
    BR_NET_SEQ_SetNext( p_wk->p_seq, Br_Net_Seq_UploadMusicalShotReq );
    break;
  case BR_NET_REQUEST_MUSICAL_SHOT_DOWNLOAD:   //ミュージカルダウンロード
    BR_NET_SEQ_SetNext( p_wk->p_seq, Br_Net_Seq_DownloadMusicalShotReq );
    break;
  case BR_NET_REQUEST_BATTLE_VIDEO_UPLOAD:     //ビデオアップロード
    BR_NET_SEQ_SetNext( p_wk->p_seq, Br_Net_Seq_UploadBattleVideoReq );
    break;
  case BR_NET_REQUEST_BATTLE_VIDEO_DOWNLOAD:   //ビデオダウンロード
    BR_NET_SEQ_SetNext( p_wk->p_seq, Br_Net_Seq_DownloadBattleVideoReq );
    break;
  case BR_NET_REQUEST_VIDEO_SEARCH_DOWNLOAD:   //ビデオ検索ダウンロード
    BR_NET_SEQ_SetNext( p_wk->p_seq, Br_Net_Seq_DownloadBattleSearchReq );
    break;
  case BR_NET_REQUEST_NEW_RANKING_DOWNLOAD:    //最新ランキングダウンロード
    BR_NET_SEQ_SetNext( p_wk->p_seq, Br_Net_Seq_DownloadNewRankingReq );
    break;
  case BR_NET_REQUEST_FAVORITE_RANKING_DOWLOAD://人気ランキングダウンロード
    BR_NET_SEQ_SetNext( p_wk->p_seq, Br_Net_Seq_DownloadFavoriteRankingReq );
    break;
  case BR_NET_REQUEST_SUBWAY_RANKING_DOWNLOAD: //サブウェイランキングダウンロード
    BR_NET_SEQ_SetNext( p_wk->p_seq, Br_Net_Seq_DownloadSubwayRankingReq );
    break;
  case BR_NET_REQUEST_FAVORITE_VIDEO_UPLOAD:   //お気に入りアップロード
    BR_NET_SEQ_SetNext( p_wk->p_seq, Br_Net_Seq_UploadFavoriteBattleReq );
    break;
  default:
    GF_ASSERT( 0 );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リクエスト待機
 *
 *	@param	BR_NET_WORK *p_wk   ワーク
 *
 *	@return TRUE 成功 FALSE 失敗
 */
//-----------------------------------------------------------------------------
BOOL BR_NET_WaitRequest( BR_NET_WORK *p_wk )
{
  return BR_NET_SEQ_IsComp( p_wk->p_seq, Br_Net_Seq_Nop );
}
//=============================================================================
/**
 *    取得
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ミュージカルショットダウンロード取得
 *
 *	@param	BR_NET_WORK *p_wk ワーク
 *	@param	*p_data_tbl       受け取るポインタテーブル
 *	@param	tbl_max           テーブルの要素数
 *	@param  p_recv_num        データ総数
 *
 *	@retval  TRUE存在する  FALSE存在しない
 */
//-----------------------------------------------------------------------------
BOOL BR_NET_GetDownloadMusicalShot( BR_NET_WORK *p_wk, MUSICAL_SHOT_RECV **pp_data_tbl, int tbl_max, int *p_recv_num )
{ 
  BOOL response_flag  = p_wk->response_flag[ BR_NET_REQUEST_MUSICAL_SHOT_DOWNLOAD ];

  if( response_flag )
  {
    *p_recv_num = GDS_RAP_RESPONSE_MusicalShot_Download_RecvPtr_Set( &p_wk->gdsrap,
        pp_data_tbl, tbl_max );
  }

  return response_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオアップロードしたときのバトルビデオナンバー取得
 *
 *	@param	BR_NET_WORK *p_wk ワーク
 *	@param	*p_video_number   ビデオナンバー
 *
 *	@retval  TRUE存在する  FALSE存在しない
 */
//-----------------------------------------------------------------------------
BOOL BR_NET_GetUploadBattleVideoNumber( BR_NET_WORK *p_wk, u64 *p_video_number )
{ 
  BOOL response_flag  = p_wk->response_flag[ BR_NET_REQUEST_BATTLE_VIDEO_UPLOAD ];
  if( response_flag )
  {
    *p_video_number = GDS_RAP_RESPONSE_BattleVideo_Upload_DataGet( &p_wk->gdsrap );
  }

  return response_flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオダウンロード取得
 *
 *	@param	BR_NET_WORK *p_wk         ワーク
 *	@param  BATTLE_REC_RECV *p_data   ビデオ受信データへのポインタ
 *	@param  int *p_video_number       バトルビデオナンバー
 *
 *	@retval  TRUE存在する  FALSE存在しない
 */
//-----------------------------------------------------------------------------
BOOL BR_NET_GetDownloadBattleVideo( BR_NET_WORK *p_wk, BATTLE_REC_RECV **pp_data, int *p_video_number )
{ 
  BOOL response_flag  = p_wk->response_flag[ BR_NET_REQUEST_BATTLE_VIDEO_DOWNLOAD ];

  if( response_flag )
  {
    *p_video_number = GDS_RAP_RESPONSE_BattleVideoData_Download_RecvPtr_Set( &p_wk->gdsrap, 
        pp_data );
  }

  return response_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオ検索結果取得
 *
 *	@param	BR_NET_WORK *p_wk ワーク
 *	@param	*p_data_tbl      受け取るバッファのテーブル
 *	@param	tbl_max           テーブルの要素数
 *	@param	*p_recv_num       受け取った数
 *
 *	@retval  TRUE存在する  FALSE存在しない
 */
//-----------------------------------------------------------------------------
BOOL BR_NET_GetDownloadBattleSearch( BR_NET_WORK *p_wk, BATTLE_REC_OUTLINE_RECV *p_data_tbl, int tbl_max, int *p_recv_num )
{ 
  BOOL response_flag  = p_wk->response_flag[ BR_NET_REQUEST_VIDEO_SEARCH_DOWNLOAD ];

  if( response_flag )
  {
    *p_recv_num = GDS_RAP_RESPONSE_BattleVideoSearch_Download_DataCopy( 
        &p_wk->gdsrap, p_data_tbl, tbl_max );
  }

  return *p_recv_num != 0 && response_flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  最新ランキングダウンロード取得
 *
 *	@param	BR_NET_WORK *p_wk ワーク
 *	@param	*p_data_tbl      受け取るバッファのテーブル
 *	@param	tbl_max           テーブルの要素数
 *	@param	*p_recv_num       受け取った数
 *
 *	@retval  TRUE存在する  FALSE存在しない
 */
//-----------------------------------------------------------------------------
BOOL BR_NET_GetDownloadNewRanking( BR_NET_WORK *p_wk, BATTLE_REC_OUTLINE_RECV *p_data_tbl, int tbl_max, int *p_recv_num )
{ 

  BOOL response_flag  = p_wk->response_flag[ BR_NET_REQUEST_VIDEO_SEARCH_DOWNLOAD ];

  if( response_flag )
  {
    *p_recv_num = GDS_RAP_RESPONSE_BattleVideoSearch_Download_DataCopy( 
        &p_wk->gdsrap, p_data_tbl, tbl_max );
  }

  return *p_recv_num != 0 && response_flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人気ランキングダウンロード取得
 *
 *	@param	BR_NET_WORK *p_wk ワーク
 *	@param	*p_data_tbl      受け取るバッファのテーブル
 *	@param	tbl_max           テーブルの要素数
 *	@param	*p_recv_num       受け取った数
 *
 *	@retval  TRUE存在する  FALSE存在しない
 */
//-----------------------------------------------------------------------------
BOOL BR_NET_GetDownloadFavoriteRanking( BR_NET_WORK *p_wk, BATTLE_REC_OUTLINE_RECV *p_data_tbl, int tbl_max, int *p_recv_num )
{ 
  BOOL response_flag  = p_wk->response_flag[ BR_NET_REQUEST_VIDEO_SEARCH_DOWNLOAD ];

  if( response_flag )
  {
    *p_recv_num = GDS_RAP_RESPONSE_BattleVideoSearch_Download_DataCopy( 
        &p_wk->gdsrap, p_data_tbl, tbl_max );
  }

  return *p_recv_num != 0 && response_flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  サブウェイランキングダウンロード取得
 *
 *	@param	BR_NET_WORK *p_wk ワーク
 *	@param	*p_data_tbl      受け取るバッファのテーブル
 *	@param	tbl_max           テーブルの要素数
 *	@param	*p_recv_num       受け取った数

 *	@retval  TRUE存在する  FALSE存在しない
 */
//-----------------------------------------------------------------------------
BOOL BR_NET_GetDownloadSubwayRanking( BR_NET_WORK *p_wk, BATTLE_REC_OUTLINE_RECV *p_data_tbl, int tbl_max, int *p_recv_num )
{ 
  BOOL response_flag  = p_wk->response_flag[ BR_NET_REQUEST_VIDEO_SEARCH_DOWNLOAD ];

  if( response_flag )
  {
    *p_recv_num = GDS_RAP_RESPONSE_BattleVideoSearch_Download_DataCopy( 
        &p_wk->gdsrap, p_data_tbl, tbl_max );
  }
  return *p_recv_num != 0 && response_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ送受信時のエラーを取得
 *
 *	@param	BR_NET_WORK *p_wk   ワーク
 *	@param	*p_msg_no           エラーを表示するメッセージ番号
 *
 *	@return エラーの戻り先（列挙型参照）
 */
//-----------------------------------------------------------------------------
BR_NET_ERR_RETURN BR_NET_GetError( BR_NET_WORK *p_wk, int *p_msg_no )
{ 
  //poke_netからのエラー
  GDS_RAP_ERROR_INFO *p_errorinfo;

  if( GDSRAP_ErrorInfoGet(&p_wk->gdsrap, &p_errorinfo) )
  { 
    int ret = BR_NET_ERR_RETURN_ONCE;//BR_NET_ERR_RETURN_NONE;poke_netがエラーがあると言っている以上、
                                    //かならずエラーを返すようにする
	  if( p_errorinfo->type == GDS_ERROR_TYPE_LIB )
    { 
      //ライブラリのエラー
      if( p_errorinfo->result == POKE_NET_GDS_LASTERROR_NONE )
      { 
        ret = BR_NET_ERR_RETURN_ONCE;
      }
      else
      { 
        if( p_msg_no )
        { 
          *p_msg_no = msg_lib_err_000 + p_errorinfo->result;
        }

        ret = BR_NET_ERR_RETURN_ONCE;
      }
    }
    else if( p_errorinfo->type == GDS_ERROR_TYPE_STATUS )
    { 
      //ライブラリの状態エラー
      if( p_errorinfo->result == POKE_NET_GDS_STATUS_ABORTED )
      { 
        if( p_msg_no )
        { 
          *p_msg_no = msg_st_err_000 + p_errorinfo->result;
        }
        ret = BR_NET_ERR_RETURN_ONCE;
      }
      else
      { 
        ret = BR_NET_ERR_RETURN_ONCE;
      }
    }
    else if( p_errorinfo->type == GDS_ERROR_TYPE_APP )
    { 
      //アプリごとのエラー
      //
      switch( p_errorinfo->req_code )
      { 
      case POKE_NET_GDS_REQCODE_MUSICALSHOT_REGIST:
        //ミュージカル送信時
        if( p_errorinfo->result != POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_SUCCESS )
        { 
          if( p_msg_no )
          { 
            *p_msg_no = msg_err_000 + p_errorinfo->result;
          }
          ret = BR_NET_ERR_RETURN_TOPMENU;
        }
        break;

      case POKE_NET_GDS_REQCODE_MUSICALSHOT_GET:
        //ミュージカル取得時
        if( p_errorinfo->result != POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_SUCCESS )
        { 
          if( p_msg_no )
          { 
            *p_msg_no = msg_err_006 + p_errorinfo->result;
          }
          ret = BR_NET_ERR_RETURN_TOPMENU;
        }
        break;

      case POKE_NET_GDS_REQCODE_BATTLEDATA_REGIST:
        //バトルビデオ送信時
        if( p_errorinfo->result != POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_SUCCESS )
        { 
          switch( p_errorinfo->result )
          {
          case GDSRAP_SP_ERR_NHTTP_400:
            NetErr_App_ReqErrorDispForce( NHTTP_RESPONSE_400 );
            ret = BR_NET_ERR_RETURN_DISCONNECT;
            break;

          case GDSRAP_SP_ERR_NHTTP_401:
            NetErr_App_ReqErrorDispForce( NHTTP_RESPONSE_401 );
            ret = BR_NET_ERR_RETURN_DISCONNECT;
            break;

          case GDSRAP_SP_ERR_NHTTP_TIMEOUT:
            NetErr_App_ReqErrorDispForce( NHTTP_RESPONSE_TIMEOUT );
            ret = BR_NET_ERR_RETURN_DISCONNECT;
            break;

          case GDSRAP_SP_ERR_NHTTP_ETC:
            NetErr_App_ReqErrorDispForce( NHTTP_RESPONSE_ETC );
            ret = BR_NET_ERR_RETURN_DISCONNECT;
            break;

          default:
            if( p_msg_no )
            { 
              *p_msg_no = msg_err_030 + p_errorinfo->result;
            }
            ret = BR_NET_ERR_RETURN_TOPMENU;
          }
        }
        break;

      case POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH:
        //バトルビデオ検索時
        if( p_errorinfo->result != POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_SUCCESS )
        { 
          if( p_msg_no )
          { 
            *p_msg_no = msg_err_036 + p_errorinfo->result;
          }
          ret = BR_NET_ERR_RETURN_TOPMENU;
        }
        break;

      case POKE_NET_GDS_REQCODE_BATTLEDATA_GET:
        //バトルビデオ取得時
        if( p_errorinfo->result != POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_SUCCESS )
        { 
          if( p_msg_no )
          { 
            *p_msg_no = msg_err_040 + p_errorinfo->result;
          }
          ret = BR_NET_ERR_RETURN_ONCE;
        }
        break;

      case POKE_NET_GDS_REQCODE_BATTLEDATA_FAVORITE:
        //バトルビデオお気に入り送信時
        //ユーザーのローカルに影響がないのでエラー通知しない
        ret = BR_NET_ERR_RETURN_NONE;
        break;
      }
    }

    BR_NET_Printf( "BR_NET エラー発生！occ%d type%d result%d ret%d\n", p_errorinfo->occ, p_errorinfo->type, p_errorinfo->result, ret );

    //エラー消去
    GDSRAP_ErrorInfoClear( &p_wk->gdsrap );

    if( ret == BR_NET_ERR_RETURN_DISCONNECT )
    {
      p_wk->is_last_disconnect_error  = TRUE;
    }
    return ret;
  }

  return BR_NET_ERR_RETURN_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  システムエラーを取得
 *
 *	@param	BR_NET_WORK *p_wk   ワーク
 *
 *	@return エラーの戻り先（列挙型参照）
 */
//-----------------------------------------------------------------------------
BR_NET_SYSERR_RETURN BR_NET_GetSysError( BR_NET_WORK *p_wk )
{ 
  //ネット接続されていないときはエラーはない
  if( p_wk == NULL )
  { 
    return BR_NET_SYSERR_RETURN_NONE;
  }

  //DWCからのエラー
  { 
    GFL_NET_DWC_ERROR_RESULT  result;
    result  = GFL_NET_DWC_ERROR_ReqErrorDisp( TRUE, FALSE );

    switch( result )
    { 
    case GFL_NET_DWC_ERROR_RESULT_PRINT_MSG:   //メッセージを描画するだけ
      PMSND_StopSE();
      return BR_NET_SYSERR_RETURN_LIGHT;

    case GFL_NET_DWC_ERROR_RESULT_RETURN_PROC: //PROCから抜けなければならない
      PMSND_StopSE();
      p_wk->is_last_disconnect_error  = TRUE;
      return BR_NET_SYSERR_RETURN_DISCONNECT;

    case GFL_NET_DWC_ERROR_RESULT_FATAL:       //電源切断のため無限ループになる
      PMSND_StopSE();
      NetErr_App_FatalDispCall();
      break;
    }
  }

  return BR_NET_SYSERR_RETURN_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  切断エラーが起こったかどうか
 *
 *	@param	const BR_NET_WORK *cp_wk  ワーク
 *
 *	@return TRUEならば切断エラーが起こった  FALSEならば起こっていない
 */
//-----------------------------------------------------------------------------
BOOL BR_NET_IsLastDisConnectError( const BR_NET_WORK *cp_wk )
{ 
  return cp_wk->is_last_disconnect_error;
}

//=============================================================================
/**
 *  状態関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  何もしない
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_Nop( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  /*  none  */
}
//----------------------------------------------------------------------------
/**
 *	@brief  リクエスト待ち
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_WaitReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BR_NET_WORK *p_wk = p_wk_adrs;

  if( GDSRAP_MoveStatusAllCheck( &p_wk->gdsrap ) )
  { 
    BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_Nop );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ミュージカルショットアップロード
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_UploadMusicalShotReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BOOL ret;
  BR_NET_WORK *p_wk = p_wk_adrs;

  ret = GDSRAP_Tool_Send_MusicalShotUpload( &p_wk->gdsrap, p_wk->p_gds_profile, p_wk->reqest_param.cp_upload_musical_shot_data );
  GF_ASSERT( ret );

  BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_WaitReq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ミュージカルショットダウンロード
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_DownloadMusicalShotReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BOOL ret;
  BR_NET_WORK *p_wk = p_wk_adrs;

  ret = GDSRAP_Tool_Send_MusicalShotDownload( &p_wk->gdsrap, p_wk->reqest_param.download_musical_shot_search_monsno );
  GF_ASSERT( ret );

  BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_WaitReq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオアップロード
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_UploadBattleVideoReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BOOL ret;
  BR_NET_WORK *p_wk = p_wk_adrs;

  ret = GDSRAP_Tool_Send_BattleVideoUpload( &p_wk->gdsrap, p_wk->p_gds_profile );
  GF_ASSERT( ret );

  BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_WaitReq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオダウンロード
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_DownloadBattleVideoReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BOOL ret;
  BR_NET_WORK *p_wk = p_wk_adrs;

  ret = GDSRAP_Tool_Send_BattleVideo_DataDownload( &p_wk->gdsrap, p_wk->reqest_param.download_video_number );
  GF_ASSERT( ret );

  BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_WaitReq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオ検索結果ダウンロード
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_DownloadBattleSearchReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BOOL ret;
  BR_NET_WORK *p_wk = p_wk_adrs;

  ret = GDSRAP_Tool_Send_BattleVideoSearchDownload( 
      &p_wk->gdsrap, 
      p_wk->reqest_param.download_video_search_data.monsno,
      p_wk->reqest_param.download_video_search_data.battle_mode_no,
      p_wk->reqest_param.download_video_search_data.country_code,
      p_wk->reqest_param.download_video_search_data.local_code
      );
  GF_ASSERT( ret );

  BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_WaitReq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  最新ランキングダウンロード
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_DownloadNewRankingReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BOOL ret;
  BR_NET_WORK *p_wk = p_wk_adrs;

  ret = GDSRAP_Tool_Send_BattleVideoNewDownload( &p_wk->gdsrap );
  GF_ASSERT( ret );

  BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_WaitReq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人気ランキングダウンロード
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_DownloadFavoriteRankingReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BOOL ret;
  BR_NET_WORK *p_wk = p_wk_adrs;

  ret = GDSRAP_Tool_Send_BattleVideoCommDownload( &p_wk->gdsrap );
  GF_ASSERT( ret );

  BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_WaitReq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルサブウェイランキングダウンロード
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_DownloadSubwayRankingReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BOOL ret;
  BR_NET_WORK *p_wk = p_wk_adrs;

  ret = GDSRAP_Tool_Send_BattleVideoSubwayDownload( &p_wk->gdsrap );
  GF_ASSERT( ret );

  BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_WaitReq );
}
//----------------------------------------------------------------------------
/**
 *	@brief  お気に入りビデオアップロード
 *
 *	@param	BR_NET_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                内部シーケンス
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Net_Seq_UploadFavoriteBattleReq( BR_NET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  BOOL ret;
  BR_NET_WORK *p_wk = p_wk_adrs;

  ret = GDSRAP_Tool_Send_BattleVideo_FavoriteUpload( &p_wk->gdsrap, p_wk->reqest_param.upload_favorite_video_number );
  GF_ASSERT( ret );

  BR_NET_SEQ_SetNext( p_seqwk, Br_Net_Seq_WaitReq );
}

//=============================================================================
/**
 *  レスポンス関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ミュージカルアップロード時のレスポンスコールバック
 *
 *	@param	*p_wk_adrs                        ワーク
 *	@param	GDS_RAP_ERROR_INFO *p_error_info  エラー情報
 */
//-----------------------------------------------------------------------------
static void Br_Net_Response_MusicalRegist(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info)
{ 
  BR_NET_WORK *p_wk = p_wk_adrs;
  BR_NET_Printf("ミュージカルショットのアップロードレスポンス取得\n");

  p_wk->response_flag[ BR_NET_REQUEST_MUSICAL_SHOT_UPLOAD ] = TRUE;

}
//----------------------------------------------------------------------------
/**
 *	@brief  ミュージカルダウンロード時のレスポンスコールバック
 *
 *	@param	*p_wk_adrs                        ワーク
 *	@param	GDS_RAP_ERROR_INFO *p_error_info  エラー情報
 */
//-----------------------------------------------------------------------------
static void Br_Net_Response_MusicalGet(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info)
{ 
  BR_NET_WORK *p_wk = p_wk_adrs;

  BR_NET_Printf("ミュージカルショットのダウンロードレスポンス取得\n");
  p_wk->response_flag[ BR_NET_REQUEST_MUSICAL_SHOT_DOWNLOAD ] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオ登録時のレスポンスコールバック
 *
 *	@param	*p_wk_adrs                        ワーク
 *	@param	GDS_RAP_ERROR_INFO *p_error_info  エラー情報
 */
//-----------------------------------------------------------------------------
static void Br_Net_Response_BattleVideoRegist(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info)
{ 
  BR_NET_WORK *p_wk = p_wk_adrs;

  BR_NET_Printf("バトルビデオ登録時のダウンロードレスポンス取得\n");
  if(p_error_info->occ == TRUE)
  {
    //TRUEならばエラー発生しているので、ここでメニューを戻すとかアプリ終了モードへ移行とかする
    switch(p_error_info->result){
    case GDSRAP_SP_ERR_NHTTP_400:
    case GDSRAP_SP_ERR_NHTTP_401:
    case GDSRAP_SP_ERR_NHTTP_ETC:
      break;
      
  	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_SUCCESS:		//!< 登録成功
  		BR_NET_Printf("aバトルビデオ登録受信成功%d\n");
  		break;
  	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_AUTH:		//!< ユーザー認証エラー
  		BR_NET_Printf("aバトルビデオ登録受信エラー！:ユーザー認証エラー\n");
  		break;
  	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ALREADY:	//!< すでに登録されている
  		BR_NET_Printf("aバトルビデオ登録受信エラー！:既に登録されている\n");
  		break;
  	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGAL:	//!< 不正なデータ
  		BR_NET_Printf("aバトルビデオ登録受信エラー！:不正データ\n");
  		break;
    case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGAL_RANKINGTYPE:  //!< 不正なランキング種別
      BR_NET_Printf("aバトルビデオ登録受信エラー！:不正なランキング種別\n");
      break;
  	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGALPROFILE:	//!< 不正なユーザープロフィール
  		BR_NET_Printf("aバトルビデオ登録受信エラー！:不正なユーザー\n");
  		break;
  	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGALPOKEMON_VERIFY: //!< ポケモン署名でエラー
  		BR_NET_Printf("aバトルビデオ登録受信エラー！:ポケモン署名でエラー\n");
  	  break;
  	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_UNKNOWN:	//!< その他エラー
  	default:
  		BR_NET_Printf("aバトルビデオ登録受信エラー！:その他のエラー \n");
  		break;
    }
  }

  p_wk->response_flag[ BR_NET_REQUEST_BATTLE_VIDEO_UPLOAD ] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオ検索時のレスポンスコールバック
 *
 *	@param	*p_wk_adrs                        ワーク
 *	@param	GDS_RAP_ERROR_INFO *p_error_info  エラー情報
 */
//-----------------------------------------------------------------------------
static void Br_Net_Response_BattleVideoSearch(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info)
{ 
  BR_NET_WORK *p_wk = p_wk_adrs;

  BR_NET_Printf("バトルビデオ検索のダウンロードレスポンス取得\n");
  p_wk->response_flag[ BR_NET_REQUEST_VIDEO_SEARCH_DOWNLOAD ] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオダウンロード時のレスポンスコールバック
 *
 *	@param	*p_wk_adrs                        ワーク
 *	@param	GDS_RAP_ERROR_INFO *p_error_info  エラー情報
 */
//-----------------------------------------------------------------------------
static void Br_Net_Response_BattleVideoDataGet(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info)
{ 
  BR_NET_WORK *p_wk = p_wk_adrs;

  BR_NET_Printf("バトルビデオデータ取得のダウンロードレスポンス取得\n");

  p_wk->response_flag[ BR_NET_REQUEST_BATTLE_VIDEO_DOWNLOAD ] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオお気に入り登録時のレスポンスコールバック
 *
 *	@param	*p_wk_adrs                        ワーク
 *	@param	GDS_RAP_ERROR_INFO *p_error_info  エラー情報
 */
//-----------------------------------------------------------------------------
static void Br_Net_Response_BattleVideoFavorite(void *p_wk_adrs, const GDS_RAP_ERROR_INFO *p_error_info)
{ 
  BR_NET_WORK *p_wk = p_wk_adrs;

  BR_NET_Printf("バトルビデオお気に入り登録のダウンロードレスポンス取得\n");
  p_wk->response_flag[ BR_NET_REQUEST_FAVORITE_VIDEO_UPLOAD ] =  TRUE;
}

//=============================================================================
/**
 *  シーケンスシステム
 */
//=============================================================================
//-------------------------------------
///	シーケンスワーク
//=====================================
struct _BR_NET_SEQ_WORK
{
	BR_NET_SEQ_FUNCTION	seq_function;		//実行中のシーケンス関数
	int seq;											//実行中のシーケンス関数の中のシーケンス
	void *p_wk_adrs;							//実行中のシーケンス関数に渡すワーク
  int reserv_seq;               //予約シーケンス
};

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	初期化
 *
 *	@param	BR_NET_SEQ_WORK *p_wk	ワーク
 *	@param	*p_param				パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static BR_NET_SEQ_WORK * BR_NET_SEQ_Init( void *p_wk_adrs, BR_NET_SEQ_FUNCTION seq_function, HEAPID heapID )
{	
  BR_NET_SEQ_WORK *p_wk;

	//作成
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BR_NET_SEQ_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(BR_NET_SEQ_WORK) );

	//初期化
	p_wk->p_wk_adrs	= p_wk_adrs;

	//セット
	BR_NET_SEQ_SetNext( p_wk, seq_function );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	破棄
 *
 *	@param	BR_NET_SEQ_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void BR_NET_SEQ_Exit( BR_NET_SEQ_WORK *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	メイン処理
 *
 *	@param	BR_NET_SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void BR_NET_SEQ_Main( BR_NET_SEQ_WORK *p_wk )
{	
	if( p_wk->seq_function )
	{	
		p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_wk_adrs );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了取得
 *
 *	@param	const BR_NET_SEQ_WORK *cp_wk		ワーク
 *
 *	@return	TRUEならば終了	FALSEならば処理中
 */	
//-----------------------------------------------------------------------------
static BOOL BR_NET_SEQ_IsEnd( const BR_NET_SEQ_WORK *cp_wk )
{	
	return cp_wk->seq_function == NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	次のシーケンスを設定
 *
 *	@param	BR_NET_SEQ_WORK *p_wk	ワーク
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void BR_NET_SEQ_SetNext( BR_NET_SEQ_WORK *p_wk, BR_NET_SEQ_FUNCTION seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了
 *
 *	@param	BR_NET_SEQ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void BR_NET_SEQ_End( BR_NET_SEQ_WORK *p_wk )
{	
  BR_NET_SEQ_SetNext( p_wk, NULL );
}
//----------------------------------------------------------------------------
/**
 *	@brief  SEQ 次のシーケンスを予約
 *
 *	@param	BR_NET_SEQ_WORK *p_wk  ワーク
 *	@param	seq             次のシーケンス
 */
//-----------------------------------------------------------------------------
static void BR_NET_SEQ_SetReservSeq( BR_NET_SEQ_WORK *p_wk, int seq )
{ 
  p_wk->reserv_seq  = seq;
}
//----------------------------------------------------------------------------
/**
 *	@brief  SEQ 予約されたシーケンスへ飛ぶ
 *
 *	@param	BR_NET_SEQ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_NET_SEQ_NextReservSeq( BR_NET_SEQ_WORK *p_wk )
{ 
  p_wk->seq = p_wk->reserv_seq;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のシーケンスと同じかどうか
 *
 *	@param	const BR_NET_SEQ_WORK *cp_wk  ワーク
 *	@param	seq_function              シーケンス
 *
 *	@return TRUE同じ FALSE異なる
 */
//-----------------------------------------------------------------------------
static BOOL BR_NET_SEQ_IsComp( const BR_NET_SEQ_WORK *cp_wk, BR_NET_SEQ_FUNCTION seq_function )
{ 
  return cp_wk->seq_function  == seq_function;
}
