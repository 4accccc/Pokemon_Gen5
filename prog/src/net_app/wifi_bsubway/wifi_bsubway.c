//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wifi_bsubway.c
 *	@brief  Wi-Fiバトルサブウェイ
 *	@author	tomoya takahashi
 *	@date		2010.02.17
 *
 *	モジュール名：WIFI_BSUBWAY
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include <dpw_bt.h>

#include "net/network_define.h"
#include "net/dwc_rap.h"
#include "net/dwc_error.h"
#include "net/dpw_rap.h"
#include "net/nhttp_rap.h"
#include "net/nhttp_rap_evilcheck.h"

#include "system/net_err.h"
#include "system/main.h"

#include "savedata/wifilist.h"
#include "savedata/wifihistory.h"
#include "savedata/mystatus.h"
#include "savedata/system_data.h"

#include "app/codein.h"

#include "msg/msg_wifi_bsubway.h"

#include "net_app/wifi_bsubway.h"
#include "net_app/wifi_login.h"
#include "net_app/wifi_logout.h"

#include "wifi_bsubway_graphic.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	メインシーケンス
//=====================================
enum {

  BSUBWAY_SEQ_LOGIN,        // Login処理
  BSUBWAY_SEQ_LOGIN_WAIT,   // Login処理
  
  BSUBWAY_SEQ_FADEIN,       // フェードイン
  BSUBWAY_SEQ_FADEINWAIT,   // フェードインウエイト

  BSUBWAY_SEQ_INIT,       // 初期化
  BSUBWAY_SEQ_INIT_WAIT,  // 初期化ウエイト
  BSUBWAY_SEQ_PERSON_SETUP,// Email認証
  BSUBWAY_SEQ_PERSON_SETUP_WAIT,// Email認証
  BSUBWAY_SEQ_MAIN,       // メイン処理
  BSUBWAY_SEQ_END,        // 終了　＆　フェードアウト
  BSUBWAY_SEQ_END_WAIT,   // 終了ウエイト

  BSUBWAY_SEQ_LOGOUT,     // Logout処理

  BSUBWAY_SEQ_ALL_END,   // 終了

  BSUBWAY_SEQ_ERROR,      // エラー処理
  BSUBWAY_SEQ_ERROR_WAIT, // エラーウエイト
} ;


//-------------------------------------
///	サブシーケンス
//=====================================
enum{
  // 記録のアップロード
  SCORE_UPLOAD_SEQ_PERSON_SETUP = 0,
  SCORE_UPLOAD_SEQ_PERSON_SETUP_WAIT,
  SCORE_UPLOAD_SEQ_PERSON_POKE_SETUP,
  SCORE_UPLOAD_SEQ_PERSON_POKE_SETUP_WAIT,
  SCORE_UPLOAD_SEQ_PERSON_UPDATE,
  SCORE_UPLOAD_SEQ_PERSON_UPDATE_WAIT,
  SCORE_UPLOAD_SEQ_PERSON_END,

  // ゲーム情報のダウンロード
  SCORE_UPLOAD_SEQ_GAMEDATA_CODEIN_RANK = 0,
  SCORE_UPLOAD_SEQ_GAMEDATA_CODEIN_RANK_WAIT,
  SCORE_UPLOAD_SEQ_GAMEDATA_DOWNLOAD_RANK_WAIT,
  SCORE_UPLOAD_SEQ_GAMEDATA_CODEIN_ROOM_NO,
  SCORE_UPLOAD_SEQ_GAMEDATA_CODEIN_ROOM_NO_WAIT,
  SCORE_UPLOAD_SEQ_GAMEDATA_DOWNLOAD_ROOM_WAIT,
  SCORE_UPLOAD_SEQ_GAMEDATA_DOWNLOAD_END,

  // 歴代情報のダウンロード
  SCORE_UPLOAD_SEQ_SUCCESSDATA_CODEIN_RANK = 0,
  SCORE_UPLOAD_SEQ_SUCCESSDATA_CODEIN_RANK_WAIT,
  SCORE_UPLOAD_SEQ_SUCCESSDATA_DOWNLOAD_RANK_WAIT,
  SCORE_UPLOAD_SEQ_SUCCESSDATA_CODEIN_ROOM_NO,
  SCORE_UPLOAD_SEQ_SUCCESSDATA_CODEIN_ROOM_NO_WAIT,
  SCORE_UPLOAD_SEQ_SUCCESSDATA_DOWNLOAD_ROOM_WAIT,
  SCORE_UPLOAD_SEQ_SUCCESSDATA_DOWNLOAD_END,
};


//-------------------------------------
///	NHTTPのエラー分岐
//=====================================
typedef enum{
  BSUBWAY_NHTTP_ERROR_NONE,
  BSUBWAY_NHTTP_ERROR_POKE_ERROR,   // ポケモンが不正
  BSUBWAY_NHTTP_ERROR_DISCONNECTED, // 正常終了しなかった

  BSUBWAY_NHTTP_ERROR_MAX,
} BSUBWAY_NHTTP_ERROR;


///強制タイムアウトまでの時間
#define BSUBWAY_TIMEOUT_TIME			(30*60*2)	//2分

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	PersonalData
//=====================================
typedef struct 
{
  BOOL setup;
	Dpw_Common_Profile			  dc_profile;					// 自分の情報登録用構造体
	Dpw_Common_ProfileResult	dc_profile_result;	// 自分の情報登録レスポンス用構造体
	Dpw_Bt_Player	bt_player;							// 勝ち抜いた自分のデータ
  const MYSTATUS* cp_mystatus;


  // ポケモン認証　署名
  NHTTP_RAP_WORK* p_nhttp;  // ポケモン認証ワーク
  u16 poke_num;
  u8 sign[NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN];

} WIFI_BSUBWAY_PERSONAL;


//-------------------------------------
///	RoomData
//=====================================
typedef struct 
{
  BOOL setup;
  BOOL in_roomdata;
  
  u16 rank;
  u16 room_no;
  u16 room_num;

	Dpw_Bt_Room 	bt_roomdata;						// ダウンロード部屋情報


} WIFI_BSUBWAY_ROOM;

//-------------------------------------
///	dpw_bt Error
//=====================================
typedef struct {
  DpwBtError          error_code;
  DpwBtServerStatus   server_status;

  u16 timeout_flag;
  s16 timeout;

  BSUBWAY_NHTTP_ERROR nhttp_error;
} WIFI_BSUBWAY_ERROR;



//-------------------------------------
///	バトルサブウェイ　WiFi部分ワーク
//=====================================
typedef struct 
{
  // パラメータ
  WIFI_BSUBWAY_PARAM* p_param;

  // Login Logout
  WIFILOGIN_PARAM   login;
  WIFILOGOUT_PARAM  logout;

  // 認証用ワーク
  DWCSvlResult svl_result;

  // DpwMainをまわすフラグ
  BOOL dpw_main_do;

  // サブシーケンス
  s32 seq;

  // 部屋情報
  WIFI_BSUBWAY_ROOM roomdata;

  // 人物情報
  WIFI_BSUBWAY_PERSONAL personaldata;

  // DPW BT エラー状態
  WIFI_BSUBWAY_ERROR  bt_error;

  // 数値入力情報
  CODEIN_PARAM* p_codein;

  // 各種情報
  GAMEDATA*     p_gamedata;
	SYSTEMDATA*   p_systemdata;		// システムセーブデータ（DPWライブラリ用ともだちコードを保存）
	WIFI_LIST*    p_wifilist;			// ともだち手帳
	MYSTATUS*     p_mystatus;			// トレーナー情報
  
} WIFI_BSUBWAY;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	process
//=====================================
static GFL_PROC_RESULT WiFiBsubway_ProcInit( GFL_PROC * p_proc, int * p_seq, void * p_param, void * p_work );
static GFL_PROC_RESULT WiFiBsubway_ProcMain( GFL_PROC * p_proc, int * p_seq, void * p_param, void * p_work );
static GFL_PROC_RESULT WiFiBsubway_ProcEnd( GFL_PROC * p_proc, int * p_seq, void * p_param, void * p_work );


//-------------------------------------
///	RoomData 
//=====================================
// 初期化
static void ROOM_DATA_Init( WIFI_BSUBWAY_ROOM* p_wk );
// ルーム数の取得処理
static void ROOM_DATA_SetUp( WIFI_BSUBWAY_ROOM* p_wk, WIFI_BSUBWAY_ERROR* p_error, u16 rank );
static BOOL ROOM_DATA_SetUpWait( WIFI_BSUBWAY_ROOM* p_wk, WIFI_BSUBWAY_ERROR* p_error );

// ルーム情報の取得処理
static void ROOM_DATA_LoadRoomData( WIFI_BSUBWAY_ROOM* p_wk, WIFI_BSUBWAY_ERROR* p_error, u16 room_no );
static BOOL ROOM_DATA_LoadRoomDataWait( WIFI_BSUBWAY_ROOM* p_wk, WIFI_BSUBWAY_ERROR* p_error );

// 情報の取得
static u16 ROOM_DATA_GetRank( const WIFI_BSUBWAY_ROOM* cp_wk );
static u16 ROOM_DATA_GetRoomNo( const WIFI_BSUBWAY_ROOM* cp_wk );
static u16 ROOM_DATA_GetRoomNum( const WIFI_BSUBWAY_ROOM* cp_wk );
static const Dpw_Bt_Room* ROOM_DATA_GetRoomData( const WIFI_BSUBWAY_ROOM* cp_wk );

//-------------------------------------
///	PersonalData
//=====================================
static void PERSONAL_DATA_Init( WIFI_BSUBWAY_PERSONAL* p_wk );
static void PERSONAL_DATA_Exit( WIFI_BSUBWAY_PERSONAL* p_wk );
// 人情報認証
static void PERSONAL_DATA_SetUpProfile( WIFI_BSUBWAY_PERSONAL* p_wk, GAMEDATA* p_gamedata, WIFI_BSUBWAY_ERROR* p_error );
static BOOL PERSONAL_DATA_SetUpProfileWait( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error );

// 任天堂認証
static void PERSONAL_DATA_SetUpNhttp( WIFI_BSUBWAY_PERSONAL* p_wk, DWCSvlResult* p_svl_result, WIFI_BSUBWAY_ERROR* p_error, HEAPID heapID );
static BOOL PERSONAL_DATA_SetUpNhttpWait( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error );

// ポケモン認証
static void PERSONAL_DATA_SetUpNhttpPokemon( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error, HEAPID heapID );
static BOOL PERSONAL_DATA_SetUpNhttpPokemonWait( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error );

// 人情報アップデート
static void PERSONAL_DATA_UploadPersonalData( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error  );
static BOOL PERSONAL_DATA_UploadPersonalDataWait( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error  );

//-------------------------------------
///	ErrorData
//=====================================
static void ERROR_DATA_Init( WIFI_BSUBWAY_ERROR* p_wk );
static void ERROR_DATA_GetAsyncStart( WIFI_BSUBWAY_ERROR* p_wk );
static BOOL ERROR_DATA_GetAsyncResult( WIFI_BSUBWAY_ERROR* p_wk, s32* p_result );
static BOOL ERROR_DATA_IsError( const WIFI_BSUBWAY_ERROR* cp_wk );
static s32 ERROR_DATA_GetPrintMessageID( const WIFI_BSUBWAY_ERROR* cp_wk );
static void ERROR_DATA_SetNhttpError( WIFI_BSUBWAY_ERROR* p_wk, BSUBWAY_NHTTP_ERROR error );
static void ERROR_DATA_StartOnlyTimeOut( WIFI_BSUBWAY_ERROR* p_wk );
static void ERROR_DATA_OnlyTimeOutCount( WIFI_BSUBWAY_ERROR* p_wk );


//-------------------------------------
///	CodeIn
//=====================================
static void CODEIN_StartProc( WIFI_BSUBWAY* p_wk, HEAPID heapID );
static void CODEIN_EndProc( WIFI_BSUBWAY* p_wk );
static void CODEIN_GetString( const WIFI_BSUBWAY* cp_wk, STRBUF* p_str );

//-------------------------------------
///	Login
//=====================================
static void LOGIN_StartProc( WIFI_BSUBWAY* p_wk );
static void LOGIN_EndProc( WIFI_BSUBWAY* p_wk );
static WIFILOGIN_RESULT LOGIN_GetResult( const WIFI_BSUBWAY* cp_wk );

//-------------------------------------
///	Logout
//=====================================
static void LOGOUT_StartProc( WIFI_BSUBWAY* p_wk );
static void LOGOUT_EndProc( WIFI_BSUBWAY* p_wk );

//-------------------------------------
///	メイン処理処理
//=====================================
static GFL_PROC_RESULT WiFiBsubway_Main( WIFI_BSUBWAY* p_wk, HEAPID heapID );
static GFL_PROC_RESULT WiFiBsubway_Main_ScoreUpload( WIFI_BSUBWAY* p_wk, HEAPID heapID );
static GFL_PROC_RESULT WiFiBsubway_Main_GamedataDownload( WIFI_BSUBWAY* p_wk, HEAPID heapID );
static GFL_PROC_RESULT WiFiBsubway_Main_SuccessdataDownload( WIFI_BSUBWAY* p_wk, HEAPID heapID );


//-------------------------------------
///	接続・切断・エラー
//=====================================
static void WiFiBsubway_Connect( WIFI_BSUBWAY* p_wk );
static BOOL WiFiBsubway_ConnectWait( WIFI_BSUBWAY* p_wk );
static void WiFiBsubway_Disconnect( WIFI_BSUBWAY* p_wk );
static BOOL WiFiBsubway_DisconnectWait( WIFI_BSUBWAY* p_wk );
static BOOL WiFiBsubway_Error( WIFI_BSUBWAY* p_wk );
static BOOL WiFiBsubway_ErrorWait( WIFI_BSUBWAY* p_wk );

//-----------------------------------------------------------------------------
/**
 *      process情報
 */
//-----------------------------------------------------------------------------
const GFL_PROC_DATA WIFI_BSUBWAY_Proc = {
  WiFiBsubway_ProcInit,
  WiFiBsubway_ProcMain,
  WiFiBsubway_ProcEnd,
};









//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  バトルサブウェイ  process 初期化
 *
 *	@param	p_proc      processワーク
 *	@param	p_seq       シーケンス
 *	@param	p_param     パラメータ
 *	@param	p_work      ワーク
 *
 *	@retval GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
 *	@retval GFL_PROC_RES_FINISH				    ///<動作終了
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiBsubway_ProcInit( GFL_PROC * p_proc, int * p_seq, void * p_param, void * p_work )
{
  WIFI_BSUBWAY* p_wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFI_BSUBWAY, 0x18000 );

  p_wk = GFL_PROC_AllocWork( p_proc, sizeof(WIFI_BSUBWAY), HEAPID_WIFI_BSUBWAY );

  // Wi-FiLogin済みかチェック
  GF_ASSERT( GFL_NET_IsInit() );


  // ワーク初期化
  p_wk->p_param = p_param;
  p_wk->p_param->result = WIFI_BSUBWAY_RESULT_NG; // 戻り値をNGで初期化

  // 各種情報の取得
  {
    GAMEDATA* p_gamedata = GAMESYSTEM_GetGameData( p_wk->p_param->p_gamesystem );
    SAVE_CONTROL_WORK* p_savedata = GAMEDATA_GetSaveControlWork( p_gamedata );
    
    p_wk->p_gamedata    = p_gamedata;
    p_wk->p_systemdata  = SaveData_GetSystemData( p_savedata );
    p_wk->p_wifilist    = GAMEDATA_GetWiFiList( p_gamedata );
    p_wk->p_mystatus    = GAMEDATA_GetMyStatus( p_gamedata );
  }

  // システムワーク初期化
  ROOM_DATA_Init( &p_wk->roomdata );
  PERSONAL_DATA_Init( &p_wk->personaldata );
  ERROR_DATA_Init( &p_wk->bt_error );
  
  // DEBUG出力ON
  GFL_NET_DebugPrintOn();

  return GFL_PROC_RES_FINISH;  
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトルサブウェイ  process メイン
 *
 *	@param	p_proc      processワーク
 *	@param	p_seq       シーケンス
 *	@param	p_param     パラメータ
 *	@param	p_work      ワーク
 *
 *	@retval GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
 *	@retval GFL_PROC_RES_FINISH				    ///<動作終了
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiBsubway_ProcMain( GFL_PROC * p_proc, int * p_seq, void * p_param, void * p_work )
{
  WIFI_BSUBWAY* p_wk = p_work;

  // 通信エラーチェック
  if( ((*p_seq) <= BSUBWAY_SEQ_MAIN) && (GFL_FADE_CheckFade() == FALSE) && (p_wk->dpw_main_do) ){
    if( ERROR_DATA_IsError( &p_wk->bt_error ) || (NetErr_App_CheckError() != NET_ERR_CHECK_NONE) )
    {
      (*p_seq) = BSUBWAY_SEQ_ERROR;
    }
  }
  
  // メイン処理
  switch( (*p_seq) ){
  // Login処理
  case BSUBWAY_SEQ_LOGIN:        
    LOGIN_StartProc( p_wk );
    (*p_seq)++;
    break;

  case BSUBWAY_SEQ_LOGIN_WAIT:
    {
      WIFILOGIN_RESULT result;
      result = LOGIN_GetResult( p_wk );
      if( result == WIFILOGIN_RESULT_LOGIN ){
        (*p_seq)++;
      }else{
        (*p_seq) = BSUBWAY_SEQ_ALL_END;
      }
    }
    break;

  case BSUBWAY_SEQ_FADEIN:       // フェードイン
    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, 1);
    (*p_seq) ++;
    break;

  case BSUBWAY_SEQ_FADEINWAIT:   // フェードインウエイト
    if( GFL_FADE_CheckFade() == FALSE ){
      (*p_seq) ++;
    }
    break;

  // 初期化
  case BSUBWAY_SEQ_INIT:       
    WiFiBsubway_Connect( p_wk );
    (*p_seq) ++;
    break;
 
  // 初期化ウエイト
  case BSUBWAY_SEQ_INIT_WAIT:  
    if( WiFiBsubway_ConnectWait( p_wk ) )
    {
      (*p_seq) ++;
    }
    break;

  //Email認証
  case BSUBWAY_SEQ_PERSON_SETUP:
    PERSONAL_DATA_SetUpProfile( &p_wk->personaldata, p_wk->p_gamedata, &p_wk->bt_error );
    (*p_seq) ++;
    break;

  //Email認証ウエイト
  case BSUBWAY_SEQ_PERSON_SETUP_WAIT:
    if( PERSONAL_DATA_SetUpProfileWait( &p_wk->personaldata, &p_wk->bt_error ) ){
      (*p_seq) ++;
    }
    break;

  // メイン処理
  case BSUBWAY_SEQ_MAIN:
    if( WiFiBsubway_Main( p_wk, HEAPID_WIFI_BSUBWAY ) == GFL_PROC_RES_FINISH ){
      p_wk->p_param->result = WIFI_BSUBWAY_RESULT_OK; //処理成功！
      (*p_seq) ++;
    }
    break;

  // 終了
  case BSUBWAY_SEQ_END:        
    WiFiBsubway_Disconnect( p_wk );
    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, 1);
    (*p_seq) ++;
    break;

  // 終了ウエイト
  case BSUBWAY_SEQ_END_WAIT:   
    if( WiFiBsubway_DisconnectWait( p_wk ) && (GFL_FADE_CheckFade() == FALSE) )
    {
      (*p_seq) ++;
    }
    break;

  // Logout処理
  case BSUBWAY_SEQ_LOGOUT:     
    LOGOUT_StartProc( p_wk );
    (*p_seq)++;
    break;

  // 終了
  case BSUBWAY_SEQ_ALL_END:   
    return GFL_PROC_RES_FINISH;

  // エラー処理
  case BSUBWAY_SEQ_ERROR:      
    if( WiFiBsubway_Error( p_wk ) ){
      (*p_seq) = BSUBWAY_SEQ_ERROR_WAIT;
    }else{
      (*p_seq) = BSUBWAY_SEQ_END;
    }
    break;

  // エラーウエイト
  case BSUBWAY_SEQ_ERROR_WAIT: 
    if( WiFiBsubway_Error( p_wk ) ){
      (*p_seq) = BSUBWAY_SEQ_END;
    }
    break;
    
  default:
    GF_ASSERT( 0 );
    break;
  }

  // Main dpw_main_doがTRUE
  // のときのみ動作
  if( p_wk->dpw_main_do ){
    Dpw_Bt_Main();
  }


  return GFL_PROC_RES_CONTINUE;  
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトルサブウェイ  process 破棄
 *
 *	@param	p_proc      processワーク
 *	@param	p_seq       シーケンス
 *	@param	p_param     パラメータ
 *	@param	p_work      ワーク
 *
 *	@retval GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
 *	@retval GFL_PROC_RES_FINISH				    ///<動作終了
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiBsubway_ProcEnd( GFL_PROC * p_proc, int * p_seq, void * p_param, void * p_work )
{
  WIFI_BSUBWAY* p_wk = p_work;
  
  // 数値入力ワーク破棄
  CODEIN_EndProc( p_wk );

  LOGIN_EndProc( p_wk );
  LOGOUT_EndProc( p_wk );

  // システムワーク破棄
  PERSONAL_DATA_Exit( &p_wk->personaldata );
  
  // procワーク破棄
  GFL_PROC_FreeWork( p_proc );

  // HEAPID破棄
  GFL_HEAP_DeleteHeap( HEAPID_WIFI_BSUBWAY );

  return GFL_PROC_RES_FINISH;
}


//-------------------------------------
///	RoomData 
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief　RoomData初期化
 */
//-----------------------------------------------------------------------------
static void ROOM_DATA_Init( WIFI_BSUBWAY_ROOM* p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(WIFI_BSUBWAY_ROOM) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  RoomData  Rankの部屋情報をセットアップ
 *
 *	@param	p_wk    ワーク
 *	@param	rank    ランク
 */
//-----------------------------------------------------------------------------
static void ROOM_DATA_SetUp( WIFI_BSUBWAY_ROOM* p_wk, WIFI_BSUBWAY_ERROR* p_error, u16 rank )
{
  GF_ASSERT( p_wk->setup == FALSE );
  GF_ASSERT( p_wk->in_roomdata == FALSE );

  p_wk->rank = rank;

  // ルーム数を取得
  Dpw_Bt_GetRoomNumAsync( rank );
  ERROR_DATA_GetAsyncStart( p_error );
}

//----------------------------------------------------------------------------
/**
 *	@brief  RoomData Rankの部屋情報セットアップ待ち
 *
 *	@param	p_wk  ワーク
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL ROOM_DATA_SetUpWait( WIFI_BSUBWAY_ROOM* p_wk, WIFI_BSUBWAY_ERROR* p_error )
{
  s32 result;
  GF_ASSERT( p_wk->setup == FALSE );
  GF_ASSERT( p_wk->in_roomdata == FALSE );

  // 受信完了待ち
	if( ERROR_DATA_GetAsyncResult( p_error, &result ) ){
    
    // エラー終了でなければ完了
    if( ERROR_DATA_IsError( p_error ) ){
      // 完了
      p_wk->room_num  = result;
      p_wk->setup     = TRUE;
      return TRUE;
    }
  }

  return FALSE;
}


// ルーム情報の取得処理
//----------------------------------------------------------------------------
/**
 *	@brief  RoomData 部屋情報の取得処理 
 *
 *	@param	p_wk      ワーク
 *	@param	room_no   部屋ナンバー
 */
//-----------------------------------------------------------------------------
static void ROOM_DATA_LoadRoomData( WIFI_BSUBWAY_ROOM* p_wk, WIFI_BSUBWAY_ERROR* p_error, u16 room_no )
{
  GF_ASSERT( p_wk->setup );
  GF_ASSERT( p_wk->in_roomdata == FALSE );

  p_wk->room_no = room_no;
  GF_ASSERT( p_wk->room_no < p_wk->room_num );
	Dpw_Bt_DownloadRoomAsync( p_wk->rank, p_wk->room_no, &p_wk->bt_roomdata );
  ERROR_DATA_GetAsyncStart( p_error );
}

//----------------------------------------------------------------------------
/**
 *	@brief  RoomData 部屋情報　取得ウエイト
 *
 *	@param	p_wk  ワーク
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL ROOM_DATA_LoadRoomDataWait( WIFI_BSUBWAY_ROOM* p_wk, WIFI_BSUBWAY_ERROR* p_error )
{
  s32 result;

  GF_ASSERT( p_wk->setup == FALSE );
  GF_ASSERT( p_wk->in_roomdata == FALSE );

  // 受信完了待ち
	if( ERROR_DATA_GetAsyncResult( p_error, &result ) ){
    
    // エラー終了でなければ完了
    if( ERROR_DATA_IsError( p_error ) ){
      // 完了
      p_wk->in_roomdata = TRUE;
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  RoomData  rankの取得
 */
//-----------------------------------------------------------------------------
static u16 ROOM_DATA_GetRank( const WIFI_BSUBWAY_ROOM* cp_wk )
{
  GF_ASSERT( cp_wk );
  return cp_wk->rank;
}
//----------------------------------------------------------------------------
/**
 *	@brief  RoomData roomNoの取得
 */
//-----------------------------------------------------------------------------
static u16 ROOM_DATA_GetRoomNo( const WIFI_BSUBWAY_ROOM* cp_wk )
{
  GF_ASSERT( cp_wk );
  return cp_wk->room_no;
}

//----------------------------------------------------------------------------
/**
 *	@brief  RoomData roomNumの取得
 */
//-----------------------------------------------------------------------------
static u16 ROOM_DATA_GetRoomNum( const WIFI_BSUBWAY_ROOM* cp_wk )
{
  GF_ASSERT( cp_wk );
  GF_ASSERT( cp_wk->setup );
  return cp_wk->room_num;
}

//----------------------------------------------------------------------------
/**
 *	@brief  RoomData 部屋情報の取得
 */
//-----------------------------------------------------------------------------
static const Dpw_Bt_Room* ROOM_DATA_GetRoomData( const WIFI_BSUBWAY_ROOM* cp_wk )
{
  GF_ASSERT( cp_wk );
  GF_ASSERT( cp_wk->setup );
  GF_ASSERT( cp_wk->in_roomdata );
  return &cp_wk->bt_roomdata;
}




//-------------------------------------
///	PersonalData
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  PersonalData  初期化
 */
//-----------------------------------------------------------------------------
static void PERSONAL_DATA_Init( WIFI_BSUBWAY_PERSONAL* p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(WIFI_BSUBWAY_PERSONAL) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  PersonalData  破棄
 */
//-----------------------------------------------------------------------------
static void PERSONAL_DATA_Exit( WIFI_BSUBWAY_PERSONAL* p_wk )
{
  if(p_wk->p_nhttp){
    NHTTP_RAP_PokemonEvilCheckDelete( p_wk->p_nhttp );
    NHTTP_RAP_End( p_wk->p_nhttp );
    p_wk->p_nhttp = NULL;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  PersonalData  プロフィールセットアップ
 *
 *	@param	p_wk        ワーク
 *	@param	p_gamedata  ゲームData
 *	@param	p_error     エラーワーク
 */
//-----------------------------------------------------------------------------
static void PERSONAL_DATA_SetUpProfile( WIFI_BSUBWAY_PERSONAL* p_wk, GAMEDATA* p_gamedata, WIFI_BSUBWAY_ERROR* p_error )
{
  SAVE_CONTROL_WORK* p_savedata = GAMEDATA_GetSaveControlWork( p_gamedata );
  MYSTATUS* p_mystatus          = GAMEDATA_GetMyStatus( p_gamedata );
  WIFI_HISTORY* p_history       = SaveData_GetWifiHistory( p_savedata );
  WIFI_LIST* p_wifilist         = GAMEDATA_GetWiFiList( p_gamedata );

  GF_ASSERT( p_wk->setup == FALSE );
  
  // DC_PROFILE作成
  DPW_RAP_CreateProfile( &p_wk->dc_profile, p_mystatus );
  
  // 認証
	Dpw_Bt_SetProfileAsync( &p_wk->dc_profile, &p_wk->dc_profile_result );
  ERROR_DATA_GetAsyncStart( p_error );

  
  // ユーザーのゲーム情報を生成
  // @TODO
  // p_wk->bt_player

  // Mystatus保存
  p_wk->cp_mystatus = p_mystatus;
}

//----------------------------------------------------------------------------
/**
 *	@brief  PersonalData  プロフィールセットアップ完了待ち
 *
 *	@param	p_wk      ワーク
 *	@param	p_error   エラーワーク
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL PERSONAL_DATA_SetUpProfileWait( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error )
{
  s32 result;

  GF_ASSERT( p_wk->setup == FALSE );

  // 受信完了待ち
	if( ERROR_DATA_GetAsyncResult( p_error, &result ) ){
    
    // エラー終了でなければ完了
    if( ERROR_DATA_IsError( p_error ) ){
      // 完了
      p_wk->setup = TRUE;
      return TRUE;
    }
  }
  return FALSE;
}


// 任天堂認証
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン認証のための人物認証
 *
 *	@param	p_wk      ワーク
 *	@param  p_svl_result
 *	@param  p_error
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void PERSONAL_DATA_SetUpNhttp( WIFI_BSUBWAY_PERSONAL* p_wk, DWCSvlResult* p_svl_result, WIFI_BSUBWAY_ERROR* p_error, HEAPID heapID )
{
  BOOL result;
  GF_ASSERT( p_wk->setup ); 

  p_wk->p_nhttp = NHTTP_RAP_Init( heapID,MyStatus_GetProfileID( p_wk->cp_mystatus ), p_svl_result );

  result = NHTTP_RAP_SvlGetTokenStart( p_wk->p_nhttp );
  GF_ASSERT( result );

  // タイムアウトチェック開始
  ERROR_DATA_StartOnlyTimeOut( p_error );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン認証のための人物認証　完了待ち
 *
 *	@param	p_wk  ワーク
 *	@param  p_error
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL PERSONAL_DATA_SetUpNhttpWait( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error )
{
  GF_ASSERT( p_wk->p_nhttp );

  if( NHTTP_RAP_SvlGetTokenMain( p_wk->p_nhttp ) )
  { 
    return TRUE;
  }

  // タイムアウトカウント
  ERROR_DATA_OnlyTimeOutCount( p_error );
  return FALSE;
}

// ポケモン認証
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン認証　
 *
 *	@param	p_wk      ワーク
 *	@param  p_error
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void PERSONAL_DATA_SetUpNhttpPokemon( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error, HEAPID heapID )
{
  BOOL result;
  //POKEMON_PASO_PARAM  *pp = PPPPointerGet( (POKEMON_PARAM*)wk->UploadPokemonData.postData.data );
 
  
  GF_ASSERT( p_wk->p_nhttp );

  NHTTP_RAP_PokemonEvilCheckCreate( p_wk->p_nhttp, heapID, POKETOOL_GetPPPWorkSize(), NHTTP_POKECHK_SUBWAY);

  // @TODO ポケモンを設定する
  //NHTTP_RAP_PokemonEvilCheckAdd( p_wk->p_nhttp, pp, POKETOOL_GetPPPWorkSize() );

  // 設定ポケモン数
  p_wk->poke_num = 1;

  result = NHTTP_RAP_PokemonEvilCheckConectionCreate( p_wk->p_nhttp );
  GF_ASSERT( result );

  result = NHTTP_RAP_StartConnect( p_wk->p_nhttp );  
  GF_ASSERT( result );

  // タイムアウトチェック開始
  ERROR_DATA_StartOnlyTimeOut( p_error );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン認証　完了待ち
 *
 *	@param	p_wk      ワーク
 *	@param  p_error
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL PERSONAL_DATA_SetUpNhttpPokemonWait( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error )
{
  NHTTPError error;

  GF_ASSERT( p_wk->p_nhttp );
  
  error = NHTTP_RAP_Process( p_wk->p_nhttp );
  if( NHTTP_ERROR_BUSY != error )
  { 
    // タイムアウトカウント
    ERROR_DATA_OnlyTimeOutCount( p_error );
  }
  else
  {
    void *p_data;
    int i;
    int poke_result;
    p_data  = NHTTP_RAP_GetRecvBuffer( p_wk->p_nhttp );


    //送られてきた状態は正常か
    if( NHTTP_RAP_EVILCHECK_GetStatusCode( p_data ) == 0 )
    {
      // 正常

      // 署名を取得
      { 
        const s8 *cp_sign  = NHTTP_RAP_EVILCHECK_GetSign( p_data, 1 );
        GFL_STD_MemCopy( cp_sign, p_wk->sign, NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN );
      }

      for( i=0; i<p_wk->poke_num; i++ ){
        poke_result  = NHTTP_RAP_EVILCHECK_GetPokeResult( p_data, i );
        if( poke_result != NHTTP_RAP_EVILCHECK_RESULT_OK ){
          ERROR_DATA_SetNhttpError( p_error, BSUBWAY_NHTTP_ERROR_POKE_ERROR );
          break;
        }
      }
    }
    else
    {
      // 認証確認失敗
      ERROR_DATA_SetNhttpError( p_error, BSUBWAY_NHTTP_ERROR_DISCONNECTED );
    }

    NHTTP_RAP_PokemonEvilCheckDelete( p_wk->p_nhttp );
    NHTTP_RAP_End( p_wk->p_nhttp );
    p_wk->p_nhttp = NULL;


    return TRUE;
  }

  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  Personal Data アップデート
 */
//-----------------------------------------------------------------------------
static void PERSONAL_DATA_UploadPersonalData( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error  )
{
  GF_ASSERT( p_wk->setup );

  {
    s32 rank, room_no;
    s32 win;
    rank = 0;
    room_no = 0;
    win = 0;
    // @TODO ちゃんとプレイ情報をアップすること
  	Dpw_Bt_UploadPlayerAsync(  rank,  room_no,  win, &p_wk->bt_player, p_wk->sign, NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN );
    ERROR_DATA_GetAsyncStart( p_error );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  Personal Data アップデート完了待ち
 *
 *	@param	p_wk      ワーク
 *	@param	p_error   エラーワーク
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL PERSONAL_DATA_UploadPersonalDataWait( WIFI_BSUBWAY_PERSONAL* p_wk, WIFI_BSUBWAY_ERROR* p_error  )
{
  s32 result;

  // 受信完了待ち
	if( ERROR_DATA_GetAsyncResult( p_error, &result ) ){
    
    // エラー終了でなければ完了
    if( ERROR_DATA_IsError( p_error ) ){
      return TRUE;
    }
  }
  return FALSE;
}







//-------------------------------------
///	ErrorData
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  ErrorData初期化
 */
//-----------------------------------------------------------------------------
static void ERROR_DATA_Init( WIFI_BSUBWAY_ERROR* p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(WIFI_BSUBWAY_ERROR) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  Erro検知機能つき　情報受信処理開始
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void ERROR_DATA_GetAsyncStart( WIFI_BSUBWAY_ERROR* p_wk )
{
  p_wk->timeout_flag  = FALSE;
  p_wk->timeout = 0;
}


//----------------------------------------------------------------------------
/**
 *	@brief  Error検知機能つき　情報受信処理
 *
 *	@param  p_wk        ワーク
 *	@param  p_result    結果格納先
 *
 *	@retval TRUE    受信完了
 *	@retval FALSE   受信中
 */
//-----------------------------------------------------------------------------
static BOOL ERROR_DATA_GetAsyncResult( WIFI_BSUBWAY_ERROR* p_wk, s32* p_result )
{
  s32 result = 0;
  BOOL complete = FALSE;
  
	if( !Dpw_Tr_IsAsyncEnd() ){
    // タイムアウトチェック
    p_wk->timeout ++;
    if( p_wk->timeout == BSUBWAY_TIMEOUT_TIME ){
      // 過去策↓
			// CommFatalErrorFunc_NoNumber();	//強制ふっとばし
      // @TODO ふっとばし　SERVERタイムアウトと同様の処理を行う。
      // キャンセルコール
      Dpw_Bt_CancelAsync();

      // タイムアウトへ
      p_wk->timeout_flag = TRUE;
    }
  }else{
		result    = Dpw_Tr_GetAsyncResult();
    complete  = TRUE;
		switch (result){
		case DPW_TR_STATUS_SERVER_OK:		// 正常に動作している
			TOMOYA_Printf(" server ok\n");
      *p_result = result;
			break;
		case DPW_TR_STATUS_SERVER_STOP_SERVICE:	// サービス停止中
		case DPW_TR_STATUS_SERVER_FULL:			// サーバーが満杯
		case DPW_TR_ERROR_SERVER_FULL:
		case DPW_TR_ERROR_CANCEL:
		case DPW_TR_ERROR_FAILURE:
		case DPW_TR_ERROR_SERVER_TIMEOUT:
		case DPW_TR_ERROR_DISCONNECTED:	
			TOMOYA_Printf(" server ng\n");
      p_wk->error_code = result;
      break;

		case DPW_TR_ERROR_FATAL:			//!< 通信致命的エラー。電源の再投入が必要です
		default:
      NetErr_ErrorSet();
			break;
		}
  }

  return complete;
}

//----------------------------------------------------------------------------
/**
 *	@brief  Dpw Btのエラー状態チェック
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval TRUE    エラー状態
 *	@retval FALSE   エラー状態ではない
 */
//-----------------------------------------------------------------------------
static BOOL ERROR_DATA_IsError( const WIFI_BSUBWAY_ERROR* cp_wk )
{
  // BTエラー
  if( cp_wk->error_code < 0 ){
    return TRUE;
  }
  // SERVERエラー
  if( cp_wk->server_status != DPW_BT_STATUS_SERVER_OK ){
    return TRUE;
  }
  // nhttpエラー
  if( cp_wk->nhttp_error != BSUBWAY_NHTTP_ERROR_NONE ){
    return TRUE;
  }
  // タイムアウト
  if( cp_wk->timeout_flag ){
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ErrorData Dpw BTのエラーメッセージ取得
 *
 *	@param	cp_wk   ワーク
 *    
 *	@return エラーメッセージID
 */
//-----------------------------------------------------------------------------
static s32 ERROR_DATA_GetPrintMessageID( const WIFI_BSUBWAY_ERROR* cp_wk )
{
	int msgno =0;

  // BtError
  if( cp_wk->error_code < 0 ){
    switch( cp_wk->error_code ){
    case DPW_BT_ERROR_SERVER_TIMEOUT:
    case DPW_BT_ERROR_DISCONNECTED:
      msgno = msg_wifi_bt_error_003;
      break;
    case DPW_BT_ERROR_SERVER_FULL:
      msgno = msg_wifi_bt_error_002;
      break;
    case DPW_BT_ERROR_FAILURE:
    case DPW_BT_ERROR_CANCEL:
      msgno = msg_wifi_bt_error_004;
      break;
    case DPW_BT_ERROR_ILLIGAL_REQUEST:
      msgno = msg_wifi_bt_error_005;
      break;

    default:
      GF_ASSERT(0);
      break;
    }
  // SERVERError
  }else if( cp_wk->server_status != DPW_BT_STATUS_SERVER_OK ){
    switch( cp_wk->server_status ){
    case DPW_BT_STATUS_SERVER_STOP_SERVICE:
      msgno = msg_wifi_bt_error_001;
      break;
    case DPW_BT_STATUS_SERVER_FULL:
      msgno = msg_wifi_bt_error_002;
      break;

    default:
      GF_ASSERT(0);
      break;
    }
  // nhttp エラー
  }else if( cp_wk->nhttp_error != BSUBWAY_NHTTP_ERROR_NONE ){
    switch( cp_wk->nhttp_error ){
    case BSUBWAY_NHTTP_ERROR_POKE_ERROR:
      msgno = msg_wifi_bt_error_005;
      break;
    case BSUBWAY_NHTTP_ERROR_DISCONNECTED:
      msgno = msg_wifi_bt_error_003;
      break;
    default:
      GF_ASSERT(0);
      break;
    }
  }
  else if( cp_wk->timeout_flag ){
    // タイムアウトエラー
    msgno = msg_wifi_bt_error_003;
  }
  else{
    GF_ASSERT(0);
  }
  

  return msgno;
}

//----------------------------------------------------------------------------
/**
 *	@brief  NhttpError  設定
 *
 *	@param	p_wk    ワーク
 *	@param	error   エラータイプ
 */
//-----------------------------------------------------------------------------
static void ERROR_DATA_SetNhttpError( WIFI_BSUBWAY_ERROR* p_wk, BSUBWAY_NHTTP_ERROR error )
{
  p_wk->nhttp_error = error;
}


//----------------------------------------------------------------------------
/**
 *	@brief  タイムアウトを開始
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void ERROR_DATA_StartOnlyTimeOut( WIFI_BSUBWAY_ERROR* p_wk )
{
  p_wk->timeout_flag  = FALSE;
  p_wk->timeout = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイムアウトカウント
 *
 *	@param	p_wk  ワーク
 *
 *	タイムアウトになったら自動でエラー処理へ行きます。
 */
//-----------------------------------------------------------------------------
static void ERROR_DATA_OnlyTimeOutCount( WIFI_BSUBWAY_ERROR* p_wk )
{
  // タイムアウトチェック
  p_wk->timeout ++;
  if( p_wk->timeout == BSUBWAY_TIMEOUT_TIME ){
    p_wk->timeout_flag = TRUE;
  }
}


//-------------------------------------
///	CodeIn
//=====================================
//----------------------------------------------------------------------------
/**
 *	@briefCodeIn開始
 *
 *	@param	p_wk      ワーク
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void CODEIN_StartProc( WIFI_BSUBWAY* p_wk, HEAPID heapID )
{
  static int s_CODEIN_BLOCK[CODE_BLOCK_MAX] = {
    2,
    0,
    0,
  };

  p_wk->p_codein = CodeInput_ParamCreate( heapID, 2, s_CODEIN_BLOCK );
  
  // プロックコール
  GFL_PROC_SysCallProc( FS_OVERLAY_ID(codein), &CodeInput_ProcData, p_wk->p_codein );
}

//----------------------------------------------------------------------------
/**
 *	@brief  CodeIn終了
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void CODEIN_EndProc( WIFI_BSUBWAY* p_wk )
{
  if( p_wk->p_codein )
  {
    CodeInput_ParamDelete( p_wk->p_codein );
    p_wk->p_codein = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  CodeIn　入力文字列取得
 *
 *	@param	cp_wk   ワーク
 *	@param  p_str 　格納バッファ
 */
//-----------------------------------------------------------------------------
static void CODEIN_GetString( const WIFI_BSUBWAY* cp_wk, STRBUF* p_str )
{
  GF_ASSERT( cp_wk->p_codein );

  GFL_STR_CopyBuffer( p_str, cp_wk->p_codein->strbuf );
}




//-------------------------------------
///	Login
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  Login処理 開始
 *
 *	@param	p_wk      ワーク  
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void LOGIN_StartProc( WIFI_BSUBWAY* p_wk )
{
  p_wk->login.gamedata  = p_wk->p_gamedata;
  p_wk->login.bg        = WIFILOGIN_BG_NORMAL;
  p_wk->login.display   = WIFILOGIN_DISPLAY_UP;
  p_wk->login.nsid      = WB_NET_BSUBWAY;
  p_wk->login.pSvl      = &p_wk->svl_result;


  // プロックコール
  GFL_PROC_SysCallProc( FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, &p_wk->login );
}

//----------------------------------------------------------------------------
/**
 *	@brief  Login処理 後始末
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void LOGIN_EndProc( WIFI_BSUBWAY* p_wk )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  Login処理 結果
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval WIFILOGIN_RESULT_LOGIN, //ログインした
 *	@retval WIFILOGIN_RESULT_CANCEL,//キャンセルした
 */
//-----------------------------------------------------------------------------
static WIFILOGIN_RESULT LOGIN_GetResult( const WIFI_BSUBWAY* cp_wk )
{
  return cp_wk->login.result;
}



//-------------------------------------
///	Logout
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  Logout処理　開始
 *
 *	@param	p_wk      ワーク
 *	@param	heapID    ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void LOGOUT_StartProc( WIFI_BSUBWAY* p_wk )
{
  p_wk->logout.gamedata  = p_wk->p_gamedata;
  p_wk->logout.bg        = WIFILOGIN_BG_NORMAL;
  p_wk->logout.display   = WIFILOGIN_DISPLAY_UP;

  // プロックコール
  GFL_PROC_SysCallProc( FS_OVERLAY_ID(wifi_login), &WiFiLogout_ProcData, &p_wk->logout );
}

//----------------------------------------------------------------------------
/**
 *	@brief  Logout処理  終了
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void LOGOUT_EndProc( WIFI_BSUBWAY* p_wk )
{
}




//-------------------------------------
///	メイン処理処理
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  バトルサブウェイ　メイン処理
 *
 *	@param	p_wk  ワーク
 *
 *	@retval GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
 *	@retval GFL_PROC_RES_FINISH				    ///<動作終了
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiBsubway_Main( WIFI_BSUBWAY* p_wk, HEAPID heapID )
{
  GFL_PROC_RESULT (*pFunc[ WIFI_BSUBWAY_MODE_MAX ])( WIFI_BSUBWAY* p_wk, HEAPID heapID ) = 
  {
    WiFiBsubway_Main_ScoreUpload,
    WiFiBsubway_Main_GamedataDownload,
    WiFiBsubway_Main_SuccessdataDownload,
  };
  GF_ASSERT( p_wk->p_param->mode < WIFI_BSUBWAY_MODE_MAX );
  
  return pFunc[ p_wk->p_param->mode ]( p_wk, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  前回情報のアップロード
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiBsubway_Main_ScoreUpload( WIFI_BSUBWAY* p_wk, HEAPID heapID )
{
  switch( p_wk->seq )
  {
  case SCORE_UPLOAD_SEQ_PERSON_SETUP:
    PERSONAL_DATA_SetUpNhttp( &p_wk->personaldata, &p_wk->svl_result, &p_wk->bt_error, heapID );
    p_wk->seq++;
    break;
  case SCORE_UPLOAD_SEQ_PERSON_SETUP_WAIT:
    if( PERSONAL_DATA_SetUpNhttpWait( &p_wk->personaldata, &p_wk->bt_error ) ){
      p_wk->seq++;
    }
    break;
  case SCORE_UPLOAD_SEQ_PERSON_POKE_SETUP:
    PERSONAL_DATA_SetUpNhttpPokemon( &p_wk->personaldata, &p_wk->bt_error, heapID );
    p_wk->seq++;
    break;
  case SCORE_UPLOAD_SEQ_PERSON_POKE_SETUP_WAIT:
    if( PERSONAL_DATA_SetUpNhttpPokemonWait( &p_wk->personaldata, &p_wk->bt_error ) ){
      p_wk->seq++;
    }
    break;
  case SCORE_UPLOAD_SEQ_PERSON_UPDATE:
    PERSONAL_DATA_UploadPersonalData( &p_wk->personaldata, &p_wk->bt_error );
    p_wk->seq++;
    break;
  case SCORE_UPLOAD_SEQ_PERSON_UPDATE_WAIT:
    if( PERSONAL_DATA_UploadPersonalDataWait( &p_wk->personaldata, &p_wk->bt_error ) ){
      p_wk->seq++;
    }
    break;
  case SCORE_UPLOAD_SEQ_PERSON_END:
    return GFL_PROC_RES_FINISH;

  default:
    GF_ASSERT(0);
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム情報のダウンロード
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiBsubway_Main_GamedataDownload( WIFI_BSUBWAY* p_wk, HEAPID heapID )
{
  switch( p_wk->seq )
  {
  case SCORE_UPLOAD_SEQ_GAMEDATA_CODEIN_RANK:
    CODEIN_StartProc( p_wk, heapID );
    p_wk->seq ++;
    break;

  case SCORE_UPLOAD_SEQ_GAMEDATA_CODEIN_RANK_WAIT:
    // 文字列を取得
    CODEIN_EndProc( p_wk );

    // 入れる。
    ROOM_DATA_SetUp( &p_wk->roomdata, &p_wk->bt_error, 0 );
    p_wk->seq ++;
    break;

  case SCORE_UPLOAD_SEQ_GAMEDATA_DOWNLOAD_RANK_WAIT:
    if( ROOM_DATA_SetUpWait( &p_wk->roomdata, &p_wk->bt_error ) ){
      p_wk->seq ++;
    }
    break;

  case SCORE_UPLOAD_SEQ_GAMEDATA_CODEIN_ROOM_NO:
    CODEIN_StartProc( p_wk, heapID );
    p_wk->seq ++;
    break;

  case SCORE_UPLOAD_SEQ_GAMEDATA_CODEIN_ROOM_NO_WAIT:
    // 文字列を取得
    CODEIN_EndProc( p_wk );

    // 入れる。
    ROOM_DATA_LoadRoomData( &p_wk->roomdata, &p_wk->bt_error, 0 );
    p_wk->seq ++;
    break;

  case SCORE_UPLOAD_SEQ_GAMEDATA_DOWNLOAD_ROOM_WAIT:
    if( ROOM_DATA_LoadRoomDataWait( &p_wk->roomdata, &p_wk->bt_error ) ){
      p_wk->seq ++;
    }
    break;

  case SCORE_UPLOAD_SEQ_GAMEDATA_DOWNLOAD_END:
    return GFL_PROC_RES_FINISH;

  default:
    GF_ASSERT(0);
    break;
  }
  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  歴代情報のダウンロード
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiBsubway_Main_SuccessdataDownload( WIFI_BSUBWAY* p_wk, HEAPID heapID )
{
  switch( p_wk->seq )
  {
  case SCORE_UPLOAD_SEQ_SUCCESSDATA_CODEIN_RANK:
    CODEIN_StartProc( p_wk, heapID );
    p_wk->seq ++;
    break;

  case SCORE_UPLOAD_SEQ_SUCCESSDATA_CODEIN_RANK_WAIT:
    // 文字列を取得
    CODEIN_EndProc( p_wk );

    // 入れる。
    ROOM_DATA_SetUp( &p_wk->roomdata, &p_wk->bt_error, 0 );
    p_wk->seq ++;
    break;

  case SCORE_UPLOAD_SEQ_SUCCESSDATA_DOWNLOAD_RANK_WAIT:
    if( ROOM_DATA_SetUpWait( &p_wk->roomdata, &p_wk->bt_error ) ){
      p_wk->seq ++;
    }
    break;

  case SCORE_UPLOAD_SEQ_SUCCESSDATA_CODEIN_ROOM_NO:
    CODEIN_StartProc( p_wk, heapID );
    p_wk->seq ++;
    break;

  case SCORE_UPLOAD_SEQ_SUCCESSDATA_CODEIN_ROOM_NO_WAIT:
    // 文字列を取得
    CODEIN_EndProc( p_wk );

    // 入れる。
    ROOM_DATA_LoadRoomData( &p_wk->roomdata, &p_wk->bt_error, 0 );
    p_wk->seq ++;
    break;

  case SCORE_UPLOAD_SEQ_SUCCESSDATA_DOWNLOAD_ROOM_WAIT:
    if( ROOM_DATA_LoadRoomDataWait( &p_wk->roomdata, &p_wk->bt_error ) ){
      p_wk->seq ++;
    }
    break;

  case SCORE_UPLOAD_SEQ_SUCCESSDATA_DOWNLOAD_END:
    return GFL_PROC_RES_FINISH;

  default:
    GF_ASSERT(0);
    break;
  }
  return GFL_PROC_RES_CONTINUE;
}



  
//-------------------------------------
///	接続・切断・エラー
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  接続開始
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void WiFiBsubway_Connect( WIFI_BSUBWAY* p_wk )
{
	// 世界交換接続初期化
	DWCUserData* p_userdata;		// 認証済みのDWCUSERデータしかこないはず
	s32 profileId;

	// DWCUser構造体取得
	p_userdata = WifiList_GetMyUserInfo( p_wk->p_wifilist );

	MyStatus_SetProfileID( p_wk->p_mystatus, WifiList_GetMyGSID( p_wk->p_wifilist ) );

	// 正式なデータを取得
	profileId = MyStatus_GetProfileID( p_wk->p_mystatus );
	TOMOYA_Printf("Dpwサーバーログイン情報 profileId=%08x\n", profileId);

	// DPW_BT初期化
	Dpw_Bt_Init( profileId, DWC_CreateFriendKey( p_userdata ),LIBDPW_SERVER_TYPE );

	TOMOYA_Printf("Dpw Bsubway 初期化\n");

	Dpw_Bt_GetServerStateAsync();
  ERROR_DATA_GetAsyncStart( &p_wk->bt_error );

	TOMOYA_Printf("Dpw Bsubway サーバー状態取得開始\n");

  p_wk->dpw_main_do = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  接続ウエイト
 *
 *	@param	p_wk  ワーク
 *
 *	@retval TRUE    接続完了
 *	@retval FALSE   接続途中
 */
//-----------------------------------------------------------------------------
static BOOL WiFiBsubway_ConnectWait( WIFI_BSUBWAY* p_wk )
{
  s32 result;
  return ERROR_DATA_GetAsyncResult( &p_wk->bt_error, &result );
}

//----------------------------------------------------------------------------
/**
 *	@brief  切断開始
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void WiFiBsubway_Disconnect( WIFI_BSUBWAY* p_wk )
{
  if( Dpw_Bt_IsAsyncEnd() == FALSE ){
    // キャンセルコール
    Dpw_Bt_CancelAsync();
    ERROR_DATA_GetAsyncStart( &p_wk->bt_error );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  切断ウエイト
 *  
 *	@param	p_wk  ワーク
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL WiFiBsubway_DisconnectWait( WIFI_BSUBWAY* p_wk )
{
  s32 result;
  if( ERROR_DATA_GetAsyncResult( &p_wk->bt_error, &result ) ){
    // 終了
    Dpw_Bt_End();
    p_wk->dpw_main_do = FALSE;
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エラー処理
 *
 *	@param	p_wk  ワーク
 *
 *	@retval BOOL  ErrorWaitの有無
 */
//-----------------------------------------------------------------------------
static BOOL WiFiBsubway_Error( WIFI_BSUBWAY* p_wk )
{
  // Netエラーチェック
  if( GFL_NET_DWC_ERROR_ReqErrorDisp() )
  {
    // 終了へ
    return FALSE;
  }
  // Btエラーチェック
  if( ERROR_DATA_IsError( &p_wk->bt_error ) )
  {
    //　エラー表示処理
    // エラー表示終了を待つ
    return TRUE;
  }

  // Not Error
  GF_ASSERT(0);
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エラー処理　ウエイト
 *
 *	@param	p_wk  ワーク
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL WiFiBsubway_ErrorWait( WIFI_BSUBWAY* p_wk )
{
  // 表示完了
  return TRUE;
}

