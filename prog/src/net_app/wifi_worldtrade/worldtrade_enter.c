//============================================================================================
/**
 * @file	worldtrade_enter.c
 * @brief	世界交換入り口処理
 * @author	Akito Mori
 * @date	06.04.16
 */
//============================================================================================
#include <gflib.h>
#include <dwc.h>
//#include "libdpw/dwci_ghttp.h"
#include "ghttp/dwci_ghttp.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include <dpw_tr.h>
#include "print/wordset.h"
#include "print/printsys.h"
#include "message.naix"
#include "system/wipe.h"
//#include "system/fontproc.h"
//#include "system/fontoam.h"
//#include "system/window.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"
#include "savedata/wifilist.h"
//#include "savedata/zukanwork.h"
#include "net_app/wifi_login.h"
#include "net_app/wifi_logout.h"
#include "src\net_app\wifi_login\wifilogin_local.h"//コールバック作成のため


#include "net_app/worldtrade.h"
#include "worldtrade_local.h"

#include "msg/msg_wifi_lobby.h"
#include "msg/msg_wifi_gts.h"
#include "msg/msg_wifi_system.h"

#include "net_app/connect_anm.h"

#include "net/network_define.h"
#include "net/dwc_rap.h"

#include "worldtrade.naix"			// グラフィックアーカイブ定義
#include "wifip2pmatch.naix"			// グラフィックアーカイブ定義
#include "net/dpw_rap.h"


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
/*** 関数プロトタイプ ***/
static void InitWork( WORLDTRADE_WORK *wk );
static void FreeWork( WORLDTRADE_WORK *wk );


static int Enter_Start( WORLDTRADE_WORK *wk);
static int Enter_End( WORLDTRADE_WORK *wk);

static int Enter_MessageWait1Second( WORLDTRADE_WORK *wk );
static int Enter_Wifilogin_start( WORLDTRADE_WORK *wk );
static int Enter_Wifilogin_wait( WORLDTRADE_WORK *wk );
static int Enter_Wifilogout_start( WORLDTRADE_WORK *wk );
static int Enter_Wifilogout_wait( WORLDTRADE_WORK *wk );

//コールバック
static WIFILOGIN_CALLBACK_RESULT Enter_WifiLogInCallBack( WIFILOGIN_MESSAGE_WORK *p_msg, void *p_callback_wk );
static WIFILOGIN_CALLBACK_RESULT Enter_WifiLogOutCallBack( WIFILOGIN_MESSAGE_WORK *p_msg, void *p_callback_wk );

//コールバック内で使用する
typedef enum
{ 
  ENTER_RESULT_CONTINUE,
  ENTER_RESULT_END,
  ENTER_RESULT_ERROR,
}ENTER_RESULT;
static BOOL Enter_WifiConnectionLogin( WORLDTRADE_WORK *wk );
static ENTER_RESULT Enter_WifiConnectionLoginWait( WORLDTRADE_WORK *wk );
static BOOL Enter_DpwTrInit( WORLDTRADE_WORK *wk );
static BOOL Enter_ServerStart( WORLDTRADE_WORK *wk );

static ENTER_RESULT Enter_ServerResult( WORLDTRADE_WORK *wk );
static BOOL Enter_ProfileStart( WORLDTRADE_WORK *wk );
static ENTER_RESULT Enter_ProfileResult( WORLDTRADE_WORK *wk );
static BOOL Enter_ServerServiceError( WORLDTRADE_WORK *wk, WIFILOGIN_MESSAGE_WORK *p_msg );
static BOOL Enter_ServerServiceEnd( WORLDTRADE_WORK *wk, WIFILOGIN_MESSAGE_WORK *p_msg );

static int printCommonFunc( GFL_BMPWIN *win, STRBUF *strbuf, int x, int flag, PRINTSYS_LSB color, int font, WT_PRINT *print );

enum{
	ENTER_START=0,
	ENTER_END,

  ENTER_WIFILOGIN_PROC_START,
	ENTER_WIFILOGIN_PROC_WAIT,
  ENTER_WIFILOGOUT_PROC_START,
	ENTER_WIFILOGOUT_PROC_WAIT,
};

static int (*Functable[])( WORLDTRADE_WORK *wk ) = {
	Enter_Start,				// ENTER_START=0,
	Enter_End,             	 	// ENTER_END,

  Enter_Wifilogin_start, //ENTER_WIFILOGIN_PROC_START,
	Enter_Wifilogin_wait, //ENTER_WIFILOGIN_PROC_WAIT,
  Enter_Wifilogout_start, //ENTER_WIFILOGOUT_PROC_START,
	Enter_Wifilogout_wait, //ENTER_WIFILOGOUT_PROC_WAIT,
};

///Eメール認証エラーが発生した際のエラーメッセージコード
enum{
	EMAIL_ERROR_SEND = -5000,
	EMAIL_ERROR_SENDFAILURE = -5001,
	EMAIL_ERROR_SUCCESS = -5002,
	EMAIL_ERROR_FAILURE = -5003,
	EMAIL_ERROR_INVALIDPARAM = -5004,
	EMAIL_ERROR_SERVERSTATE = -5005,
};

///強制タイムアウトまでの時間
#define TIMEOUT_TIME			(30*60*2)	//2分

//============================================================================================
//	プロセス関数
//============================================================================================

//==============================================================================
/**
 * @brief   世界交換入り口画面初期化
 *
 * @param   wk		GTS画面ワーク
 * @param   seq		（未使用）
 *
 * @retval  int		アプリシーケンス
 */
//==============================================================================
int WorldTrade_Enter_Init(WORLDTRADE_WORK *wk, int seq)
{

  WorldTrade_ExitSystem( wk );

	// ワーク初期化
	InitWork( wk );

	// メイン画面が下に出力されるようにする
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

	
	// ワイプフェード開始
	//WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
#ifdef GTS_FADE_OSP
	OS_Printf( "******************** worldtrade_enter.c [172] MS ********************\n" );
#endif


    // 通信エラー管理のために通信ルーチンをON
//    CommStateWifiDPWStart( wk->param->savedata );
	

  //モード別起動へ変更
  switch( wk->sub_process_mode )
  { 
  case MODE_WIFILOGIN:     ///< WIFIログイン
  case MODE_WIFILOGIN_ERR:
    wk->subprocess_seq = ENTER_WIFILOGIN_PROC_START;
    break;

  case MODE_WIFILOGOUT:    ///< WIFIログアウト
    wk->subprocess_seq = ENTER_WIFILOGOUT_PROC_START;
    break;
  }
	return SEQ_FADEIN;
}

//==============================================================================
/**
 * @brief   世界交換入り口画面メイン
 *
 * @param   wk		GTS画面ワーク
 * @param   seq		（未使用）
 *
 * @retval  int		アプリシーケンス
 */
//==============================================================================
int WorldTrade_Enter_Main(WORLDTRADE_WORK *wk, int seq)
{
	int ret, temp_subprocess_seq;

	// シーケンス遷移で実行
	temp_subprocess_seq = wk->subprocess_seq;
	ret = (*Functable[wk->subprocess_seq])( wk );
	if(temp_subprocess_seq != wk->subprocess_seq){
		wk->local_seq = 0;
		wk->local_wait = 0;
	}

	return ret;
}


//==============================================================================
/**
 * @brief   世界交換入り口画面終了
 *
 * @param   wk		GTS画面ワーク
 * @param   seq		（未使用）
 *
 * @retval  int		アプリシーケンス
 */
//==============================================================================
int WorldTrade_Enter_End(WORLDTRADE_WORK *wk, int seq)
{
	FreeWork( wk );

  WorldTrade_InitSystem( wk );

	// 次のサブプロセスを設定する
	WorldTrade_SubProcessUpdate( wk );
	
	// 次のサブプロセスが入り口のままだったら世界交換終了
	if(wk->sub_process==WORLDTRADE_ENTER){
		OS_TPrintf("世界交換終了\n");
		return SEQ_OUT;
	}
	
	OS_TPrintf("WorldTrade_Enter end\n");
	// 次が設定されていたら初期化へ
	return SEQ_INIT;
}

//------------------------------------------------------------------
/**
 * 世界交換ワーク初期化
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void InitWork( WORLDTRADE_WORK *wk )
{
}


//------------------------------------------------------------------
/**
 * @brief   ワーク解放
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void FreeWork( WORLDTRADE_WORK *wk )
{
}




//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//------------------------------------------------------------------
/**
 * @brief   サブプロセスシーケンススタート処理
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int Enter_Start( WORLDTRADE_WORK *wk)
{
  PMSND_PlayBGM( WORLDTRADE_BGM );


  wk->subprocess_seq  = ENTER_END;
	wk->boxSearchFlag = 1;
  wk->OpeningFlag   = 0;

  WorldTrade_SubProcessChange( wk, WORLDTRADE_TITLE, 0 );

	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   GameSpyサーバーログイン開始
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  TRUEで終了
 */
//------------------------------------------------------------------
static BOOL Enter_WifiConnectionLogin( WORLDTRADE_WORK *wk )
{
	DWC_NASLoginAsync();
	OS_Printf("GameSpyサーバーログイン開始\n");

	return TRUE;
}
//------------------------------------------------------------------
/**
 * @brief   GameSpyサーバーログイン処理待ち
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  TRUEで終了
 */
//------------------------------------------------------------------
static ENTER_RESULT Enter_WifiConnectionLoginWait( WORLDTRADE_WORK *wk )
{
	switch(DWC_NASLoginProcess()){
	case DWC_NASLOGIN_STATE_SUCCESS:
		OS_Printf("GameSpyサーバーログイン成功\n");
    return ENTER_RESULT_END;
		break;
	case DWC_NASLOGIN_STATE_ERROR:
	case DWC_NASLOGIN_STATE_CANCELED:
	case DWC_NASLOGIN_STATE_DIRTY:
    return ENTER_RESULT_ERROR;
	}
	
	
	return ENTER_RESULT_CONTINUE;
}



//------------------------------------------------------------------
/**
 * @brief   世界交換サーバー接続ライブラリ初期化
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  TRUEで終了
 */
//------------------------------------------------------------------
static BOOL Enter_DpwTrInit( WORLDTRADE_WORK *wk )
{
	// 世界交換接続初期化
	DWCUserData		*MyUserData;		// 認証済みのDWCUSERデータしかこないはず
	s32 profileId;

	// DWCUser構造体取得
	MyUserData = WifiList_GetMyUserInfo(wk->param->wifilist);

	MyStatus_SetProfileID( wk->param->mystatus, WifiList_GetMyGSID(wk->param->wifilist) );

	// 正式なデータを取得
	profileId = MyStatus_GetProfileID( wk->param->mystatus );
	OS_Printf("Dpwサーバーログイン情報 profileId=%08x\n", profileId);

	// DPW_TR初期化
	Dpw_Tr_Init( profileId, DWC_CreateFriendKey( MyUserData ),LIBDPW_SERVER_TYPE );
	OS_TPrintf("Dpw_Tr_Init 初期化\n");

	
	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief   世界交換ライブラリ接続状況取得開始
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  TRUEで終了
 */
//------------------------------------------------------------------
static BOOL Enter_ServerStart( WORLDTRADE_WORK *wk )
{
	Dpw_Tr_GetServerStateAsync();

	OS_TPrintf("Dpw Trade サーバー状態取得開始\n");

	// サーバー状態確認待ちへ
	wk->timeout_count = 0;
	
	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief   サーバー状態確認待ち
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  TRUEで終了
 */
//------------------------------------------------------------------
static ENTER_RESULT Enter_ServerResult( WORLDTRADE_WORK *wk )
{
  Dpw_Tr_Main();

	if (Dpw_Tr_IsAsyncEnd()){
		s32 result = Dpw_Tr_GetAsyncResult();
		wk->timeout_count = 0;
		switch (result){
		case DPW_TR_STATUS_SERVER_OK:		// 正常に動作している
			OS_TPrintf(" server is up!\n");
			return ENTER_RESULT_END;
		case DPW_TR_STATUS_SERVER_STOP_SERVICE:	// サービス停止中
			OS_TPrintf(" server stop service.\n");
			WorldTrade_TimeIconDel(wk);
			wk->ConnectErrorNo = result;
      return ENTER_RESULT_ERROR;
			break;
		case DPW_TR_STATUS_SERVER_FULL:			// サーバーが満杯
		case DPW_TR_ERROR_SERVER_FULL:
			OS_TPrintf(" server full.\n");
			WorldTrade_TimeIconDel(wk);
			wk->ConnectErrorNo = result;
      return ENTER_RESULT_ERROR;
			break;

		case DPW_TR_ERROR_CANCEL :
		case DPW_TR_ERROR_FAILURE :
			// 「GTSのかくにんにしっぱいしました」→タイトルへ
			WorldTrade_TimeIconDel(wk);
			wk->ConnectErrorNo = result;
      return ENTER_RESULT_ERROR;
			break;

		case DPW_TR_ERROR_SERVER_TIMEOUT :
		case DPW_TR_ERROR_DISCONNECTED:	
			// サーバーと通信できません→終了
			OS_TPrintf(" upload error. %d \n", result);
			WorldTrade_TimeIconDel(wk);
			wk->ConnectErrorNo = result;
      return ENTER_RESULT_ERROR;
			break;
		case DPW_TR_ERROR_FATAL:			//!< 通信致命的エラー。電源の再投入が必要です
		default:
			// 即ふっとばし
			WorldTrade_TimeIconDel(wk);
      WorldTrade_DispCallFatal();
			break;

		}
		
	}
	else{
		wk->timeout_count++;
		if(wk->timeout_count == TIMEOUT_TIME){
      WorldTrade_DispCallFatal();
		}
	}
	return ENTER_RESULT_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief   世界交換ライブラリ：プロフィール(Eメール)更新開始
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static BOOL Enter_ProfileStart( WORLDTRADE_WORK *wk )
{
  DPW_RAP_CreateProfile( &wk->dc_profile, wk->param->mystatus );
	Dpw_Tr_SetProfileAsync(&wk->dc_profile, &wk->dc_profile_result);

	OS_TPrintf("Dpw Trade プロフィール(Eメール)送信\n");

	// サーバー状態確認待ちへ
	wk->timeout_count = 0;
	
	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief   Eメール更新処理返事待ち
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static ENTER_RESULT Enter_ProfileResult( WORLDTRADE_WORK *wk )
{
  Dpw_Tr_Main();

	if (Dpw_Tr_IsAsyncEnd()){
		s32 result = Dpw_Tr_GetAsyncResult();
		wk->timeout_count = 0;
		switch (result){
		case DPW_TR_STATUS_SERVER_OK:		// 正常に動作している
			OS_TPrintf(" profile is up!\n");

//			WorldTrade_TimeIconDel(wk);
			switch(wk->dc_profile_result.code){
			case DPW_PROFILE_RESULTCODE_SUCCESS:	//情報の登録に成功
				OS_TPrintf("mailAddrAuthResult = %d\n", wk->dc_profile_result.mailAddrAuthResult);
				switch(wk->dc_profile_result.mailAddrAuthResult){
				case DPW_PROFILE_AUTHRESULT_SUCCESS:	//認証成功
					// ポケモン確認サブプロセスへ移行
					WorldTrade_SubProcessChange( wk, WORLDTRADE_TITLE, 0 );
					return ENTER_RESULT_END;
					break;
				//以下のエラー処理はこのシーンでは想定していないメール認証の結果が返った場合
				// (自分の友達コードが変化したときに以前と同じメールアドレスとパスワードを送
				// 信したときにこのようになる可能性があります。通常はあり得ません。)マニュアル引用
				case DPW_PROFILE_AUTHRESULT_FAILURE:	//認証に失敗
					OS_TPrintf(" mail service error\n");
					wk->ConnectErrorNo = EMAIL_ERROR_FAILURE;
          return ENTER_RESULT_ERROR;
					break;
				case DPW_PROFILE_AUTHRESULT_SEND:	//認証メール送信した
					OS_TPrintf(" mail service error\n");
					wk->ConnectErrorNo = EMAIL_ERROR_SEND;
          return ENTER_RESULT_ERROR;
					break;
				case DPW_PROFILE_AUTHRESULT_SENDFAILURE:	//認証メールの送信に失敗
					OS_TPrintf(" mail service error\n");
					wk->ConnectErrorNo = EMAIL_ERROR_SENDFAILURE;
          return ENTER_RESULT_ERROR;
					break;
				default:	//ありえないけど一応。強制ふっとばし
          WorldTrade_DispCallFatal();
					break;
				}
				break;
			case DPW_PROFILE_RESULTCODE_ERROR_INVALIDPARAM:	//プロフィールの送信パラメータ不正
				OS_TPrintf(" server stop service.\n");
				wk->ConnectErrorNo = EMAIL_ERROR_INVALIDPARAM;
        return ENTER_RESULT_ERROR;
				break;
			case DPW_PROFILE_RESULTCODE_ERROR_SERVERSTATE:	//サーバメンテナンスor一時停止中
				OS_TPrintf(" server stop service.\n");
				wk->ConnectErrorNo = EMAIL_ERROR_SERVERSTATE;
        return ENTER_RESULT_ERROR;
				break;
			default:
				// 即ふっとばし
				OS_TPrintf("default error !\n");
				WorldTrade_TimeIconDel(wk);
        WorldTrade_DispCallFatal();
				break;
			}
			break;
		case DPW_TR_STATUS_SERVER_STOP_SERVICE:	// サービス停止中
			OS_TPrintf(" server stop service.\n");
			WorldTrade_TimeIconDel(wk);
			wk->ConnectErrorNo = result;
      return ENTER_RESULT_ERROR;
			break;
		case DPW_TR_STATUS_SERVER_FULL:			// サーバーが満杯
		case DPW_TR_ERROR_SERVER_FULL:
			OS_TPrintf(" server full.\n");
			WorldTrade_TimeIconDel(wk);
			wk->ConnectErrorNo = result;
      return ENTER_RESULT_ERROR;
			break;

		case DPW_TR_ERROR_CANCEL :
		case DPW_TR_ERROR_FAILURE :
			// 「GTSのかくにんにしっぱいしました」→タイトルへ
			WorldTrade_TimeIconDel(wk);
			wk->ConnectErrorNo = result;
      return ENTER_RESULT_ERROR;
			break;

		case DPW_TR_ERROR_SERVER_TIMEOUT :
		case DPW_TR_ERROR_DISCONNECTED:	
			// サーバーと通信できません→終了
			OS_TPrintf(" upload error. %d \n", result);
			WorldTrade_TimeIconDel(wk);
			wk->ConnectErrorNo = result;
      return ENTER_RESULT_ERROR;
			break;
		case DPW_TR_ERROR_FATAL:			//!< 通信致命的エラー。電源の再投入が必要です
		default:
			// 即ふっとばし
			WorldTrade_TimeIconDel(wk);
      WorldTrade_DispCallFatal();
			break;
		}
		
	}
	else{
		wk->timeout_count++;
		if(wk->timeout_count == TIMEOUT_TIME){
      WorldTrade_DispCallFatal();
		}
	}
  return ENTER_RESULT_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief   WIFILOGINのPROCを呼ぶ
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int Enter_Wifilogin_start( WORLDTRADE_WORK *wk )
{ 
  WIFILOGIN_PARAM *param;
  wk->sub_proc_wk = GFL_HEAP_AllocMemory( HEAPID_WORLDTRADE, sizeof(WIFILOGIN_PARAM) );
  GFL_STD_MemClear( wk->sub_proc_wk, sizeof(WIFILOGIN_PARAM) );
  param = wk->sub_proc_wk;
  param->gamedata     = GAMESYSTEM_GetGameData(wk->param->gamesys);
  param->bg           = WIFILOGIN_BG_NORMAL;
  param->display      = WIFILOGIN_DISPLAY_UP;
  param->pSvl         = &wk->svl;
  param->nsid         = WB_NET_WIFIGTS;
  param->login_after_callback = Enter_WifiLogInCallBack;
  param->p_callback_wk  = wk;

  wk->wifi_seq  = 0;
  if( wk->sub_process_mode == MODE_WIFILOGIN_ERR )
  { 
    param->mode         = WIFILOGIN_MODE_ERROR;
  }
  else
  { 
    param->mode         = WIFILOGIN_MODE_NORMAL;
  }

  GFL_PROC_LOCAL_CallProc( wk->local_proc,
		FS_OVERLAY_ID(wifi_login), 
    &WiFiLogin_ProcData, wk->sub_proc_wk );


  //もしエラーならばここでエラー画面になる
  NetErr_DispCall(FALSE); 

  wk->subprocess_seq = ENTER_WIFILOGIN_PROC_WAIT;
	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   WIFILOGINのPROC待ち
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int Enter_Wifilogin_wait( WORLDTRADE_WORK *wk )
{ 
  WIFILOGIN_PARAM *param  = wk->sub_proc_wk;
  if( wk->local_proc_status == GFL_PROC_MAIN_NULL )
  { 
    switch( param->result )
    { 
    case WIFILOGIN_RESULT_LOGIN:
      wk->subprocess_seq = ENTER_START;
      break;

    case WIFILOGIN_RESULT_CANCEL:
      //次のシーケンスがENTERだったら終了
      WorldTrade_SubProcessChange( wk, WORLDTRADE_ENTER, 0 );
      wk->subprocess_seq  = ENTER_END;
      break;
    }

    GFL_HEAP_FreeMemory(wk->sub_proc_wk);
  }

	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   WIFILOGOUTのPROCを呼ぶ
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int Enter_Wifilogout_start( WORLDTRADE_WORK *wk )
{ 
  WIFILOGOUT_PARAM *param;
  wk->sub_proc_wk = GFL_HEAP_AllocMemory( HEAPID_WORLDTRADE, sizeof(WIFILOGOUT_PARAM) );
  GFL_STD_MemClear( wk->sub_proc_wk, sizeof(WIFILOGOUT_PARAM) );
  param = wk->sub_proc_wk;
  param->gamedata     = GAMESYSTEM_GetGameData(wk->param->gamesys);
  param->bg           = WIFILOGIN_BG_NORMAL;
  param->display      = WIFILOGIN_DISPLAY_UP;
  param->logout_before_callback = Enter_WifiLogOutCallBack;
  param->p_callback_wk  = wk;

  GFL_PROC_LOCAL_CallProc( wk->local_proc,
      FS_OVERLAY_ID(wifi_login), 
      &WiFiLogout_ProcData, wk->sub_proc_wk );

  wk->subprocess_seq = ENTER_WIFILOGOUT_PROC_WAIT;

  //もしエラーならばここでエラー画面になる
  NetErr_DispCall(FALSE); 

	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   WIFILOGOUTのPROC待ち
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int Enter_Wifilogout_wait( WORLDTRADE_WORK *wk )
{ 
  if( wk->local_proc_status == GFL_PROC_MAIN_NULL )
  { 
    GFL_HEAP_FreeMemory(wk->sub_proc_wk);

    //次のシーケンスがENTERだったら終了
    WorldTrade_SubProcessChange( wk, WORLDTRADE_ENTER, 0 );
    wk->subprocess_seq  = ENTER_END;
  }

	return SEQ_MAIN;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ログイン時のコールバック
 *
 *	@param	WIFILOGIN_MESSAGE_WORK *p_msg   メッセージ描画
 *	@param	*p_callback_wk                  ユーザーワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
static WIFILOGIN_CALLBACK_RESULT Enter_WifiLogInCallBack( WIFILOGIN_MESSAGE_WORK *p_msg, void *p_callback_wk )
{ 
  enum
  { 
    SEQ_LOGIN_START,
    SEQ_LOGIN_WAIT,
    SEQ_DPW_INIT,
    SEQ_SERVER_START,
    SEQ_SERVER_RESULT,
    SEQ_PROFILE_START,
    SEQ_PROFILE_RESULT,

    SEQ_ERROR_START,
    SEQ_ERROR_WAIT,
    SEQ_ERROR_END,
  };

  WORLDTRADE_WORK *wk = p_callback_wk;

  switch( wk->wifi_seq )
  { 
  case SEQ_LOGIN_START:
    if( Enter_WifiConnectionLogin( wk ) )
    { 
      wk->wifi_seq++;
    }
    break;
  case SEQ_LOGIN_WAIT:
    switch( Enter_WifiConnectionLoginWait( wk ) )
    { 
    case ENTER_RESULT_END:
      wk->wifi_seq++;
      break;
    case ENTER_RESULT_ERROR:
      wk->wifi_seq  = SEQ_ERROR_END;
      break;
    }
    break;
  case SEQ_DPW_INIT:
    if( Enter_DpwTrInit( wk ) )
    { 
      wk->wifi_seq++;
    }
    break;
  case SEQ_SERVER_START:
    if( Enter_ServerStart( wk ) )
    { 
      wk->wifi_seq++;
    }
    break;
  case SEQ_SERVER_RESULT:
    switch( Enter_ServerResult( wk ) )
    { 
    case ENTER_RESULT_END:
      wk->wifi_seq++;
      break;
    case ENTER_RESULT_ERROR:
      wk->wifi_seq  = SEQ_ERROR_START;
      break;
    }
    break;
  case SEQ_PROFILE_START:
    if( Enter_ProfileStart( wk ) )
    { 
      wk->wifi_seq++;
    }
    break;
  case SEQ_PROFILE_RESULT:
    switch( Enter_ProfileResult( wk ) )
    { 
    case ENTER_RESULT_END:
      wk->wifi_seq  = 0;
      return WIFILOGIN_CALLBACK_RESULT_SUCCESS;
      break;
    case ENTER_RESULT_ERROR:
      wk->wifi_seq  = SEQ_ERROR_START;
      break;
    }
    break;

  case SEQ_ERROR_START:
    if( Enter_ServerServiceError( wk, p_msg ) )
    { 
      wk->wifi_seq++;
    }
    break;
  case SEQ_ERROR_WAIT:
    if( Enter_ServerServiceEnd( wk, p_msg ) )
    { 
      wk->wifi_seq++;
    }
    break;
  case SEQ_ERROR_END:
    wk->wifi_seq  = 0;
    return WIFILOGIN_CALLBACK_RESULT_FAILED;
  }

  return WIFILOGIN_CALLBACK_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ログアウト時のコールバック
 *
 *	@param	WIFILOGIN_MESSAGE_WORK *p_msg   メッセージ描画
 *	@param	*p_callback_wk                  ユーザーワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
static WIFILOGIN_CALLBACK_RESULT Enter_WifiLogOutCallBack( WIFILOGIN_MESSAGE_WORK *p_msg, void *p_callback_wk )
{
  WORLDTRADE_WORK *wk = p_callback_wk;

  OS_TPrintf( "Dpw_Trを終了しました\n" );
  Dpw_Tr_End();
  return WIFILOGIN_CALLBACK_RESULT_SUCCESS;
}


//------------------------------------------------------------------
/**
 * @brief   サブプロセスシーケンス終了処理
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int Enter_End( WORLDTRADE_WORK *wk)
{
	// 時間アイコン消去２重解放にならないようにNULLチェックしつつ
	WorldTrade_TimeIconDel( wk );

#ifdef GTS_FADE_OSP
	OS_Printf( "******************** worldtrade_enter.c [1037] MS ********************\n" );
#endif
	wk->subprocess_seq = 0;
	wk->sub_out_flg = 1;
	
	return SEQ_FADEOUT;
}

//==============================================================================
/**
 * @brief   ネットには繋がったけどサーバーエラーだった表示
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//==============================================================================
static BOOL Enter_ServerServiceError( WORLDTRADE_WORK *wk, WIFILOGIN_MESSAGE_WORK *p_msg )
{
	int msgno =0;

	switch(wk->ConnectErrorNo){
	case DPW_TR_STATUS_SERVER_STOP_SERVICE:
		msgno = msg_gtc_error_001;
		break;
	case DPW_TR_STATUS_SERVER_FULL:
	case DPW_TR_ERROR_SERVER_FULL:
		msgno = msg_gtc_error_002;
		break;
	case DPW_TR_ERROR_SERVER_TIMEOUT:
	case DPW_TR_ERROR_DISCONNECTED:
		// ＧＴＳとのせつぞくがきれました。うけつけにもどります
		msgno = msg_gtc_error_006;
		break;
	case DPW_TR_ERROR_CANCEL  :
	case DPW_TR_ERROR_FAILURE :
	case DPW_TR_ERROR_NO_DATA:
	case DPW_TR_ERROR_ILLIGAL_REQUEST :
	default:
		//　つうしんエラーが発生しました。
		msgno = msg_gtc_error_005;
		break;
	case EMAIL_ERROR_FAILURE:
		msgno = msg_gtc_email_error_004;
		break;
	case EMAIL_ERROR_SEND:
		msgno = msg_gtc_email_error_001;
		break;
	case EMAIL_ERROR_SENDFAILURE:
		msgno = msg_gtc_email_error_001;
		break;
	case EMAIL_ERROR_INVALIDPARAM:
		msgno = msg_gtc_email_error_002;
		break;
	case EMAIL_ERROR_SERVERSTATE:
		msgno = msg_gtc_email_error_003;
		break;
	}
	// エラー表示
  
  WIFILOGIN_MESSAGE_InfoMessageDispEx(p_msg, wk->MsgManager, msgno);

	OS_TPrintf("Error発生\n");
  wk->local_seq = 0;

	return TRUE;
}

//==============================================================================
/**
 * @brief   サーバーサービスの問題で終了
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//==============================================================================
static BOOL Enter_ServerServiceEnd( WORLDTRADE_WORK *wk, WIFILOGIN_MESSAGE_WORK *p_msg )
{
	switch(wk->local_seq){
	case 0:
		//Enter_MessagePrint( wk, wk->MsgManager, msg_gtc_cleanup_000, 1, 0x0f0f );
		wk->local_seq++;
		break;
	case 1:
    if( WIFILOGIN_MESSAGE_InfoMessageEndCheck(p_msg)){
      // 通信エラー管理のために通信ルーチンをOFF
      //CommStateWifiDPWEnd();
      DWC_CleanupInet();
      wk->local_seq++;
    }
		break;
	case 2:
		//Enter_MessagePrint( wk, wk->MsgManager, msg_gtc_cleanup_001, 1, 0x0f0f );
		wk->local_seq++;
		break;
	case 3:
	//	if(GF_MSG_PrintEndCheck( &wk->print )==0){

    OS_TPrintf( "Dpw_Trを終了しました\n" );
    Dpw_Tr_End();
			wk->local_seq++;
	//	}
		break;
	default:
		wk->local_wait++;
		if(wk->local_wait > WAIT_ONE_SECONDE_NUM){
      return TRUE;
		}
		break;
	}
	
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief   会話ウインドウ表示
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
void Enter_MessagePrint( WORLDTRADE_WORK *wk, GFL_MSGDATA *msgman, int msgno, int wait, u16 dat )
{
	// 文字列取得
	STRBUF *tempbuf;

	// 文字列取得
	tempbuf = GFL_MSG_CreateString(  msgman, msgno );

	// WORDSET展開
	WORDSET_ExpandStr( wk->WordSet, wk->TalkString, tempbuf );

	GFL_STR_DeleteBuffer(tempbuf);

	// 会話ウインドウ枠描画
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin),  0x0f );
	BmpWinFrame_Write( wk->MsgWin, WINDOW_TRANS_ON, WORLDTRADE_MESFRAME_CHR, WORLDTRADE_MESFRAME_PAL );

	// 文字列描画開始
	GF_STR_PrintSimple( wk->MsgWin, FONT_TALK, wk->TalkString, 0, 0, &wk->print);

	wk->wait = 0;
}






//----------------------------------------------------------------------------------
/**
 * @brief	描画位置取得
 *
 * @param   win		BMPWIN
 * @param   strbuf	文字列
 * @param   x		強制的に右にずらすドット数
 * @param   flag	1だとセンタリング、２だと右よせ
 * @param   color	カラー指定
 * @param   font	フォント指定（FONT_TALKかFONT_SYSTEM
 *
 * @retval  int		文字列描画開始位置
 */
//----------------------------------------------------------------------------------
static int printCommonFunc( GFL_BMPWIN *win, STRBUF *strbuf, int x, int flag, PRINTSYS_LSB color, int font, WT_PRINT *print )
{
	int length=0,ground;
	switch(flag){
	// センタリング
	case 1:
		length = FontProc_GetPrintStrWidth( print, font, strbuf, 0 );
		
		x          = ((GFL_BMPWIN_GetSizeX(win)*8)-length)/2;
		break;

	// 右寄せ
	case 2:
		length = FontProc_GetPrintStrWidth( print, font, strbuf, 0 );
		x          = (GFL_BMPWIN_GetSizeX(win)*8)-length;
		break;
	}
	return x;
}


//------------------------------------------------------------------
/**
 * @brief   BMPWIN内の表示位置を指定してFONT_TALKでプリント(描画のみ）
 *
 * @param   win		GFL_BMPWIN
 * @param   strbuf	
 * @param   x		X座標ずらす値
 * @param   y		Y座標ずらす値
 * @param   flag	0だと左寄せ、1だとセンタリング、2だと右寄せ
 * @param   color	文字色指定（背景色でBMPを塗りつぶします）
 *
 * @retval  none
 */
//------------------------------------------------------------------
void WorldTrade_TalkPrint( GFL_BMPWIN *win, STRBUF *strbuf, int x, int y, int flag, PRINTSYS_LSB color, WT_PRINT *print )
{
	x = printCommonFunc( win, strbuf, x, flag, color, FONT_TALK, print );

	GF_STR_PrintColor( win, FONT_TALK, strbuf, x, y, MSG_ALLPUT, color, print);
	
}

//==============================================================================
/**
 * @brief   BMPWIN内の表示位置を指定してFONT_SYSTEMでプリント(描画のみ）
 *
 * @param   win		BMPWIN
 * @param   strbuf	文字列
 * @param   x		X座標
 * @param   y		Y座標
 * @param   flag	0だと左寄せ、1だとセンタリング、2だと右寄せ
 * @param   color	カラー指定
 *
 * @retval  none
 */
//==============================================================================
void WorldTrade_SysPrint( GFL_BMPWIN *win, STRBUF *strbuf, int x, int y, int flag, PRINTSYS_LSB color, WT_PRINT *print )
{
	x = printCommonFunc( win, strbuf, x, flag, color, FONT_SYSTEM, print );

	GF_STR_PrintColor( win, FONT_SYSTEM, strbuf, x, y, MSG_ALLPUT, color, print);
}

//==============================================================================
/**
 * @brief   BMPWIN内の表示位置を指定してFONT_SYSTEMでプリント(描画のみ）
 *
 * @param   win		BMPWIN
 * @param   strbuf	文字列
 * @param   x		X座標
 * @param   y		Y座標
 * @param   flag	0だと左寄せ、1だとセンタリング、2だと右寄せ
 * @param   color	カラー指定
 *
 * @retval  none
 */
//==============================================================================
void WorldTrade_TouchPrint( GFL_BMPWIN *win, STRBUF *strbuf, int x, int y, int flag, PRINTSYS_LSB color , WT_PRINT *print )
{
	x = printCommonFunc( win, strbuf, x, flag, color, FONT_SYSTEM, print );

	GF_STR_PrintColor( win, FONT_TOUCH, strbuf, x, y, MSG_ALLPUT, color, print);
}



//==============================================================================
/**
 * @brief   GTS説明文章描画
 *
 * @param   win		BHMWPIN
 * @param   msgman	メッセージマネージャー
 * @param   no		0 - 4 「ＧＴＳ」「こうかん」「ようすをみる」「あずける」「さがす」
 *
 * @retval  none
 */
//==============================================================================
void WorldTrade_ExplainPrint( GFL_BMPWIN *win,  GFL_MSGDATA *msgman, int no, WT_PRINT *print )
{
	static const explain_tbl[]={
		msg_gts_explain_001, msg_gts_explain_002,msg_gts_explain_003,msg_gts_explain_004,msg_gts_explain_005
	};
	STRBUF *strbuf = GFL_MSG_CreateString( msgman, explain_tbl[no] );
	WorldTrade_SysPrint( win, strbuf, 0, 0, 0, PRINTSYS_LSB_Make(1,2,0), print );
	GFL_STR_DeleteBuffer(strbuf);
}

//------------------------------------------------------------------
/**
 * @brief   WIFIアイコンを表示登録する
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
void WorldTrade_WifiIconAdd( WORLDTRADE_WORK *wk )
{
    WirelessIconEasy();
}

