//==============================================================================
/**
 * @file	net_err.c
 * @brief	通信エラー画面制御
 * @author	matsuda
 * @date	2008.11.14(金)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include "net\network_define.h"
#include "system/net_err.h"
#include "system/ds_system.h"
#include "arc_def.h"
#include "net_err.naix"

#include "print\wordset.h"
#include "print\printsys.h"
#include "print\gf_font.h"
#include "font/font.naix"
#include "msg\msg_net_err.h"
#include "sound\pm_sndsys.h"
#include "system/machine_use.h"

#ifndef MULTI_BOOT_MAKE  //通常時処理
#include <dwc.h>
#include "net/dwc_rap.h"
#include "msg\msg_wifi_system.h"
#include "message.naix"
#include "sound\pm_voice.h"
#endif //MULTI_BOOT_MAKE

#ifdef PM_DEBUG

#ifdef WIFIMATCH_RATE_AUTO
#undef DEBUG_ERR_THROUGH
#define DEBUG_ERR_THROUGH  //この定義をONにすると、エラーのボタン押しをスルーするー
#endif
#endif//PM_DEBUG


//==============================================================================
//	定数定義
//==============================================================================
///通信エラー画面で使用するテンポラリヒープ
#define HEAPID_NET_TEMP			(GFL_HEAPID_APP)

///通信エラー画面を終了させるボタン
#define ERR_DISP_END_BUTTON		(PAD_BUTTON_A | PAD_BUTTON_B)

///エラー画面表示後、キーを有効にするまでのウェイト
#define ERR_DISP_KEY_OCC_WAIT		(60*2)

///メッセージ描画領域開始位置：左端X
#define MESSAGE_START_X			(1)
///メッセージ描画領域開始位置：上端Y
#define MESSAGE_START_Y			(4)
///メッセージ描画領域X長
#define MESSAGE_X_LEN			(30)
///メッセージ描画領域Y長
#define MESSAGE_Y_LEN			(16)

///メッセージのパレット展開位置
#define MESSAGE_PALNO			(0)

///背景BGグラフィック(ncg)のデータサイズ
#define BG_DATA_SIZE			(32 * 0x20)
///メッセージ描画開始キャラクタNo
#define MESSAGE_START_CHARNO	(BG_DATA_SIZE / 0x20)

//背景BG＋メッセージデータでキャラクタ領域NETERR_PUSH_CHARVRAM_SIZEをオーバーしていないかチェック
SDK_COMPILER_ASSERT(MESSAGE_X_LEN*0x20*MESSAGE_Y_LEN + BG_DATA_SIZE <= NETERR_PUSH_CHARVRAM_SIZE);


//==============================================================================
//	構造体定義
//==============================================================================
///通信エラー画面制御構造体
typedef struct{
	u8 *push_char_p;			///<キャラクタVRAM退避先
	u16 *push_scrn_p;			///<スクリーンVRAM退避先
	u16 *push_pltt_p;			///<パレットVRAM退避先

	GXDispCnt dispcnt;
	GXBg01Control bg1cnt;
	int master_brightness;		///<メイン画面マスター輝度
	int master_brightness_sub;	///<サブ画面マスター輝度
	s32 h_intr;					///<Hブランク割り込み許可
	s32 v_intr;					///<Vブランク割り込み許可
	GXVRamBG bg_bank;			    ///<BG VRAMバンク情報
	GXVRamSubBG bg_bank_sub;	///<SUB BG VRAMバンク情報
	
	NET_ERR_STATUS status;		///<エラー画面システムの状況
	s32 key_timer;				///<キー入力受付までのタイマー

  u16 g2_blend_cnt;   //特殊カラーの取得
  u16 g2s_blend_cnt;  //特殊カラーの取得

  int sub_visible;    //サブ画面の表示色の設定
  int sub_wnd;
  u16 sub_bd_color;
  u16 dummy2;

  NET_ERR_PUSHPOP_MODE  pushpop_mode;

  //以下エラー情報保存
  GFL_NETSTATE_DWCERROR   wifi_error; //WIFIのエラー
  u8                      error;      //WIFI以外のエラー
  u8                      net_type;   //ネットのタイプ
  u8                      msg_force;  //強制メッセージフラグ
  u8                      dummy;
  NET_ERR_CHECK           err_important_type;  //エラーの重度軽度判別
  u32                     wifi_msg;   //WIFIで表示するものは先に取得するため
}NET_ERR_SYSTEM;

//==============================================================================
//	ローカル変数
//==============================================================================
static NET_ERR_SYSTEM NetErrSystem = {0};

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL NetErr_DispMain(BOOL fatal_error);
static void Local_ErrDispInit(BOOL fatal_error,BOOL isExitWait);
static void Local_ErrDispExit(BOOL is_black_continue);
static BOOL Local_SystemOccCheck(void);
static void Local_ErrDispDraw(void);
static void Local_ErrMessagePrint(BOOL fatal_error,BOOL isExitWait);
static void Local_ErrUpdate(void);

//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   通信エラー画面システム初期化
 *
 * ゲーム内で一度だけ呼び出す
 */
//--------------------------------------------------------------
void NetErr_SystemInit(void)
{
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NET_ERR, NETERR_PUSH_CHARVRAM_SIZE + NETERR_PUSH_SCRNVRAM_SIZE + NETERR_PUSH_PLTTVRAM_SIZE + 0x100);  //+0x100はヘッダ分
	GFL_STD_MemClear(&NetErrSystem, sizeof(NET_ERR_SYSTEM));
}

//--------------------------------------------------------------
/**
 * @brief   通信エラー画面システム作成
 * 
 * @param	ヒープID
 * 
 * ゲームの最初に作って、以後ずっと使用するのであれば一度だけの呼び出しでOK。
 * ヒープ節約のために、各アプリなどでのみ使用する場合は、その都度
 * SystemCreate,SystemExitを呼ぶようにして使ってください
 */
//--------------------------------------------------------------
void NetErr_SystemCreate(int heap_id)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;

	GF_ASSERT(nes->push_char_p == NULL);
	GF_ASSERT(nes->push_scrn_p == NULL);
	GF_ASSERT(nes->push_pltt_p == NULL);
	
	GFL_STD_MemClear(nes, sizeof(NET_ERR_SYSTEM));

	nes->push_char_p = GFL_HEAP_AllocMemory(heap_id, NETERR_PUSH_CHARVRAM_SIZE);
	nes->push_scrn_p = GFL_HEAP_AllocMemory(heap_id, NETERR_PUSH_SCRNVRAM_SIZE);
	nes->push_pltt_p = GFL_HEAP_AllocMemory(heap_id, NETERR_PUSH_PLTTVRAM_SIZE);
}

//--------------------------------------------------------------
/**
 * @brief   通信エラー画面システム破棄
 */
//--------------------------------------------------------------
static void NetErr_SystemExit(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;

	GF_ASSERT(nes->push_char_p != NULL);
	GF_ASSERT(nes->push_scrn_p != NULL);
	GF_ASSERT(nes->push_pltt_p != NULL);
	
	GFL_HEAP_FreeMemory(nes->push_char_p);
	GFL_HEAP_FreeMemory(nes->push_scrn_p);
	GFL_HEAP_FreeMemory(nes->push_pltt_p);

	GFL_STD_MemClear(nes, sizeof(NET_ERR_SYSTEM));
	
	GFL_HEAP_DeleteHeap(HEAPID_NET_ERR);
}

//==================================================================
/**
 * エラー発生監視：メイン
 */
//==================================================================
void NetErr_Main(void)
{
  if(NetErrSystem.status == NET_ERR_STATUS_NULL && GFL_NET_IsInit() == TRUE){
    if(GFL_NET_GetErrorCode() || GFL_NET_SystemIsError() || (GFL_NET_StateGetError()!=0)){
      //GFL_NET_StateGetWifiErrorNo();  これはエラー情報の取得だけに使用する
      OS_TPrintf("NetErr エラー発生 %d %d %d\n", 
        GFL_NET_GetErrorCode(), GFL_NET_SystemIsError(), GFL_NET_StateGetError());
      GFL_NET_DeleteInitializeCallback();
      NetErr_ErrorSet();

      //WIFIのエラーならばオーバーレイが解放される前にメッセージ番号を貰う
#ifndef MULTI_BOOT_MAKE  //通常時処理
      {
        const GFLNetInitializeStruct* net_init  = GFL_NET_GetNETInitStruct();
        if( net_init->bNetType == GFL_NET_TYPE_WIFI 
            || net_init->bNetType == GFL_NET_TYPE_WIFI_LOBBY 
            || net_init->bNetType == GFL_NET_TYPE_WIFI_GTS )
        { 
          const GFL_NETSTATE_DWCERROR *cp_dwc_error  = GFL_NET_StateGetWifiError();

          OS_TPrintf("Wifiエラー番号 code%d ret%d type%d user%d\n", 
              cp_dwc_error->errorCode, cp_dwc_error->errorRet, cp_dwc_error->errorType, cp_dwc_error->errorUser );
          if( cp_dwc_error->errorUser == ERRORCODE_TIMEOUT
              || cp_dwc_error->errorUser == ERRORCODE_DISCONNECT )
          {
            //タイムアウトか相手と切断していたならばならば
            //「あいてとのせつだんがきれました」
            NetErrSystem.wifi_msg = dwc_message_0022;
          }
          else if( cp_dwc_error->errorUser == ERRORCODE_CRC
              || cp_dwc_error->errorUser == ERRORCODE_SYSTEM 
              || cp_dwc_error->errorUser == ERRORCODE_SENDQUEUE
              || cp_dwc_error->errorUser == ERRORCODE_USER_TIMEOUT
              || cp_dwc_error->errorUser == ERRORCODE_NHTTP)
          {
            //システムエラーならば
            //つうしんえらーがはっせい」
            NetErrSystem.wifi_msg = dwc_error_0015;
          }
          else
          {
            //DWCのエラーならば、タイプによる
            NetErrSystem.wifi_msg = GFL_NET_DWC_ErrorType( cp_dwc_error->errorCode,
                cp_dwc_error->errorType);
          }
        }
      }
#endif 
    }
  }
}

//==================================================================
/**
 * エラーが発生していればエラー画面を一発呼び出し
 *
 * @param   fatal_error   TRUE:リセットを促すエラー(Aで抜けられない) 
 * 
 * @retval  TRUE:エラー画面を呼び出した
 * @retval  FALSE:呼び出していない
 * 
 * エラー画面を表示した場合、ユーザーが特定の操作を行わない限り(Aを押す)
 * この関数内で無限ループします。
 */
//==================================================================
BOOL NetErr_DispCall(BOOL fatal_error)
{
  return NetErr_DispMain(fatal_error);
}

//==================================================================
/**
 * Fatalエラー専用としてエラー画面を一発呼び出し  メッセージ指定WIFI専用
 * @param   message  メッセージ番号
 * @retval  TRUE:エラー画面を呼び出した
 * @retval  FALSE:呼び出していない
 */
//==================================================================
BOOL NetErr_App_FatalDispCallWifiMessage(int message)
{
  NetErrSystem.wifi_msg = message;
  NetErrSystem.msg_force = TRUE;
  NetErrSystem.net_type = GFL_NET_TYPE_WIFI;
  return NetErr_DispCall(TRUE);
}

//==================================================================
/**
 * Fatalエラー専用としてエラー画面を一発呼び出し
 *
 * @param   none		
 *
 * @retval  TRUE:エラー画面を呼び出した
 * @retval  FALSE:呼び出していない
 */
//==================================================================
BOOL NetErr_App_FatalDispCall(void)
{
  return NetErr_DispCall(TRUE);
}

//----------------------------------------------------------------------------
/**
 *	@brief  PUSHPOP終了モードの設定
 *
 *	@param	NET_ERR_PUSHPOP_MODE pushpop_mode モード
 */
//-----------------------------------------------------------------------------
void NetErr_SetPushPopMode( NET_ERR_PUSHPOP_MODE pushpop_mode )
{
  NET_ERR_SYSTEM *nes = &NetErrSystem;
  nes->pushpop_mode = pushpop_mode;
}

//----------------------------------------------------------------------------
/**
 *	@brief  PUSHPOP終了モードのクリア
 *
 */
//-----------------------------------------------------------------------------
void NetErr_ClearPushPopMode( void )
{
  NetErr_SetPushPopMode( NET_ERR_PUSHPOP_MODE_NORMAL );
}

//----------------------------------------------------------------------------
/**
 *	@brief  PUSHPOPでNET_ERR_PUSHPOP_MODE_BLACKOUTに設定場合の
 *	        輝度もどし
 */
//-----------------------------------------------------------------------------
void NetErr_ResetPushPopBrightness( void )
{
  NET_ERR_SYSTEM *nes = &NetErrSystem;
  GX_SetMasterBrightness(nes->master_brightness);
  GXS_SetMasterBrightness(nes->master_brightness_sub);
}

//--------------------------------------------------------------
/**
 * PushPop用のNetMain
 */
//--------------------------------------------------------------
static void _PushPopErrNetMain(void)
{
  if(GFL_NET_IsInit() == TRUE){
    GFL_NET_Main();
    OS_WaitIrq(TRUE, OS_IE_V_BLANK);
  }
}

//==================================================================
/**
 * Push,Pop有のエラー画面一発呼び出し　※軽度エラー専用。通信の終了処理は行いません
 *
 * エラー画面を表示した場合、ユーザーが特定の操作を行わない限り(Aを押す)
 * この関数内で無限ループします。
 */
//==================================================================
void NetErr_DispCallPushPop(void)
{
  NET_ERR_SYSTEM *nes = &NetErrSystem;
  GXSDispCnt sub_dispcnt = GXS_GetDispCnt();
  GXWndPlane gx_outside = G2_GetWndOutsidePlane();
  GXWndPlane gxs_outside = G2S_GetWndOutsidePlane();
  GXWndPlane gx_inside = G2_GetWndOBJInsidePlane();
  GXWndPlane gxs_inside = G2S_GetWndOBJInsidePlane();
  GXWndPlane gx_wnd_inside = G2_GetWndOBJInsidePlane();
  
	if(Local_SystemOccCheck() == FALSE){
		GF_ASSERT(0); //システムが作られていない
		return;
	}

	//エラー画面描画
	Local_ErrDispInit(FALSE,FALSE);
	
	GX_VBlankIntr(TRUE);  //Wi-Fiでは繋がっているままエラー画面を出すケースがある為VBlankを生かす
	
	//通信アイコンの表示(Wi-Fiでは繋がっているままエラー画面を出すケースがある為)
	GFL_NET_WirelessIconOBJWinON();
	if(GFL_NET_WirelessIconGetVramType() == NNS_G2D_VRAM_TYPE_2DMAIN){
  	G2_SetWndOutsidePlane(GX_WND_PLANEMASK_BG1, FALSE);
  	G2_SetWndOBJInsidePlane(GX_WND_PLANEMASK_OBJ, FALSE);
  	GX_SetVisibleWnd(GX_WNDMASK_OW);
  	GX_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1);
  }
  else{
  	G2S_SetWndOutsidePlane(GX_WND_PLANEMASK_BG1, FALSE);
  	G2S_SetWndOBJInsidePlane(GX_WND_PLANEMASK_OBJ, FALSE);
    GXS_SetVisibleWnd(GX_WNDMASK_OW);
  	GXS_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1);
  }
	
//		OS_SpinWait(10000);
	
#ifndef DEBUG_ERR_THROUGH
	while((PAD_Read() & ERR_DISP_END_BUTTON) != 0){
		Local_ErrUpdate();	//ボタンを一度離すまで待つ
		_PushPopErrNetMain();
	}
	
	while((PAD_Read() & ERR_DISP_END_BUTTON) == 0){
		Local_ErrUpdate();	//エラー画面終了ボタンが押されるまで待つ
		_PushPopErrNetMain();
	}
#endif //DEBUG_ERR_THROUGH
	
	//エラー画面終了

  {
    BOOL is_black_continue  = (nes->pushpop_mode == NET_ERR_PUSHPOP_MODE_BLACKOUT);
    Local_ErrDispExit( is_black_continue );
  	
  	GFL_NET_WirelessIconOBJWinOFF();
  	if(GFL_NET_WirelessIconGetVramType() == NNS_G2D_VRAM_TYPE_2DSUB){
      //メインはLocal_ErrDispExitで復帰している為、Subだけ復帰
    	GXS_SetGraphicsMode(sub_dispcnt.bgMode);
    	GXS_SetVisiblePlane(sub_dispcnt.visiblePlane);
    	GXS_SetVisibleWnd(sub_dispcnt.visibleWnd);
    	G2S_SetWndOutsidePlane(gxs_outside.planeMask, gxs_outside.effect);
    	G2S_SetWndOBJInsidePlane(gxs_inside.planeMask, gxs_inside.effect);
  	}
  	else{
    	G2_SetWndOutsidePlane(gx_outside.planeMask, gx_outside.effect);
    	G2_SetWndOBJInsidePlane(gx_inside.planeMask, gx_inside.effect);
    }
  }
}

//==================================================================
/**
 * Push,Pop有のエラー画面一発呼び出し　※FatalError専用
 *
 * エラー画面を表示した場合、電源切断を促すため、この関数内で無限ループします。
 */
//==================================================================
void NetErr_DispCallFatal(void)
{
	if(Local_SystemOccCheck() == FALSE){
		GF_ASSERT(0); //システムが作られていない
		return;
	}

	//通信切断
//	NetErr_ExitNetSystem();

	//エラー画面描画
	Local_ErrDispInit(TRUE,FALSE);
	
//		OS_SpinWait(10000);

  //無限ループ
  GFL_UI_SoftResetEnable(GFL_UI_SOFTRESET_SVLD);  //ハードリセットが効くようにフラグを落とす
  do{
    MachineSystem_Main(); //ハードリセット用
  }while(1);
	
	//エラー画面終了
	Local_ErrDispExit(FALSE);
}


//--------------------------------------------------------------
/**
 * @brief   エラーが発生したらこの関数を使用してエラー画面を呼び出す
 *
 * @param   none		後々、エラー情報を入れてもらう
 */
//--------------------------------------------------------------
void NetErr_ErrorSet(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;

	if(Local_SystemOccCheck() == FALSE || nes->status != NET_ERR_STATUS_NULL){
		return;
	}
	nes->status = NET_ERR_STATUS_ERROR;

  if( GFL_NET_IsExit() )
  { 
    GF_ASSERT_MSG( 0, "ネットが終了しているのに、エラーが検出されました\n" );
  }
  else
  { 
    const GFLNetInitializeStruct* net_init  = GFL_NET_GetNETInitStruct();
    nes->net_type     = net_init->bNetType;
    nes->wifi_error   = *GFL_NET_StateGetWifiError();
    nes->error        = GFL_NET_StateGetError();
    
#ifndef MULTI_BOOT_MAKE  //通常時処理
    if( nes->net_type == GFL_NET_TYPE_WIFI
        || nes->net_type == GFL_NET_TYPE_WIFI_LOBBY
        || nes->net_type == GFL_NET_TYPE_WIFI_GTS )
    { 

      //WIFIならば、軽度と重度を判定
      if( nes->wifi_error.errorType == DWC_ETYPE_LIGHT
          || nes->wifi_error.errorType == DWC_ETYPE_SHOW_ERROR )
      { 
        nes->err_important_type  = NET_ERR_CHECK_LIGHT;
      }
      else
      { 
        nes->err_important_type  = NET_ERR_CHECK_HEAVY;
      }
    }
    else
    {
      nes->err_important_type  = NET_ERR_CHECK_LIGHT;
    }
#else //MB子機処理
    //常に重度のエラー
    nes->err_important_type  = NET_ERR_CHECK_HEAVY;
#endif  //MULTI_BOOT_MAKE
  }
}

//--------------------------------------------------------------
/**
 * @brief   エラーが発生したらこの関数を使用してエラー画面を呼び出す
 *
 * @param   none		後々、エラー情報を入れてもらう
 */
//--------------------------------------------------------------
#if PM_DEBUG
void NetErr_DEBUG_ErrorSet(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;

  //WiFi
#if defined( DEBUG_ONLY_FOR_ohno ) | defined( DEBUG_ONLY_FOR_matsuda ) | defined( DEBUG_ONLY_FOR_toru_nagihashi ) | defined( DEBUG_ONLY_FOR_ariizumi_nobuhiko ) | defined( DEBUG_ONLY_FOR_shimoyamada )

#ifndef MULTI_BOOT_MAKE  //通常時処理
  if( GFL_NET_IsWifiConnect() )
  {
    GFL_NET_StateSetWifiError( -30000, DWC_ETYPE_DISCONNECT, 1, 0 );
  }
#endif //MULTI_BOOT_MAKE

  NetErr_ErrorSet();
  OS_TPrintf( "ユーザーからエラー設定リクエストが発生しました\n" );
#endif


}
#endif  //PM_DEBUG

//--------------------------------------------------------------
/**
 * @brief   アプリ用：エラーが発生したか調べる
 *
 * @retval  NET_ERR_CHECK
 *
 * アプリ側はこの関数を使用してエラーが発生しているか調べ、
 * エラーが発生していた場合は、各アプリ毎のエラー用処理へ移行してください
 */
//--------------------------------------------------------------
NET_ERR_CHECK NetErr_App_CheckError(void)
{
  NET_ERR_SYSTEM *nes = &NetErrSystem;
  if(nes->status != NET_ERR_STATUS_NULL)
  {
    return nes->err_important_type;
  }
  return NET_ERR_CHECK_NONE;
}

//==================================================================
/**
 * アプリ用：NetBit指定で接続が維持できているかチェック
 *
 * @param   net_bit		      接続調査対象のNetIDをbitで指定
 *                          ※NetID 0番と3番をチェックするのであれば ( (1 << 0) | (1 << 3) )
 *
 * @retval  NET_ERR_CHECK		
 *
 * 途中離脱をOKにしていないのであれば、このチェックは要りません。
 * 途中離脱をOKにしていて、一緒に何らかのゲームをプレイしていて、
 * その相手がいなくなったら終了する必要がある場合にのみ使う必要があります。
 *
 * 2010.03.03(水)現在の所、侵入時の通信対戦のみ
 */
//==================================================================
NET_ERR_CHECK NetErr_App_CheckConnectError(u32 net_bit)
{
  NetID netID;
  
  for( netID = 0; netID < GFL_NET_MACHINE_MAX; netID++ )
  {
    if( net_bit & (1 << netID) ) //チェック対象のプレイヤーである
    {
      if(GFL_NET_IsConnectMember(netID) == FALSE)
      {
        NetErr_ErrorSet();
        return NET_ERR_CHECK_HEAVY;   //通信相手が切断している為、エラー
      }
    }
  }
  
  return NET_ERR_CHECK_NONE;
}

//==================================================================
/**
 * アプリ用：エラー画面表示リクエスト
 */
//==================================================================
void NetErr_App_ReqErrorDisp(void)
{
  //GF_ASSERT_MSG(NetErrSystem.status == NET_ERR_STATUS_ERROR, "status=%d\n", NetErrSystem.status);
  if( NetErrSystem.status == NET_ERR_STATUS_ERROR )
  { 
    NetErrSystem.status = NET_ERR_STATUS_REQ;
    OS_TPrintf("NetErr App ReqErrDisp\n");
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  強制エラー画面表示リクエスト  
 *	        WIFIではDWC経由でエラーを引き出していますが、
 *	        ライブラリのなかにはDWCのエラーにならず、ライブラリの固有のエラーを
 *	        履く場合があり、そのときに上記NetErr_App_ReqErrorDispを読んでも
 *	        抜けてきてしまうので、エラー状態でなくともリクエストできる関数を用意しました。
 *
 *	@param	msg メッセージ番号
 */
//-----------------------------------------------------------------------------
void NetErr_App_ReqErrorDispForce(int message )
{
  if( NetErrSystem.status != NET_ERR_STATUS_REQ )
  { 
    NetErrSystem.wifi_msg = message;
    NetErrSystem.msg_force = TRUE;
    NetErrSystem.net_type = GFL_NET_TYPE_WIFI;
    NetErrSystem.status = NET_ERR_STATUS_REQ;
    OS_TPrintf("NetErr App ReqErrDisp Force\n");
  }
}

//--------------------------------------------------------------
/**
 * @brief   外に退避用アドレスを渡す
 */
//--------------------------------------------------------------
void NetErr_GetTempArea( u8** charArea , u16** scrnArea , u16** plttArea )
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
  *charArea = nes->push_char_p;
  *scrnArea = nes->push_scrn_p;
  *plttArea = nes->push_pltt_p;
}

//--------------------------------------------------------------
/**
 * エラー発生ワークをクリア
 *
 * @param   none		
 */
//--------------------------------------------------------------
void NetErr_ErrWorkInit(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	nes->status = NET_ERR_STATUS_NULL;
  nes->err_important_type = NET_ERR_CHECK_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  強制切断
 *
 */
//-----------------------------------------------------------------------------
void NetErr_ExitNetSystem( void )
{ 
  if(GFL_NET_IsInit() )
  {
    GFL_NET_ResetDisconnect();  ///切断処理中でも一旦リセット
    GFL_NET_Exit(NULL);
    GFL_NET_IRCWIRELESS_ResetSystemError();  //赤外線WIRLESS切断
    do{
      GFL_NET_ShutdownMain();
      Local_ErrUpdate();
      //OS_WaitIrq(TRUE, OS_IE_V_BLANK);
//      OS_TPrintf("GFL_NET_IsExitの完了を待っています\n");
    }while(GFL_NET_IsExit() == FALSE);
  }
}

//--------------------------------------------------------------
/**
 * @brief   通信エラー画面制御メイン
 *
 * @param   fatal_error   TRUE:リセットを促すエラー(Aで抜けられない) 
 * 
 * @retval  TRUE:エラー画面を呼び出した
 * @retval  FALSE:呼び出していない
 */
//--------------------------------------------------------------
static BOOL NetErr_DispMain(BOOL fatal_error)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	
	if(Local_SystemOccCheck() == FALSE){
		return FALSE;
	}
  
  if(fatal_error == TRUE){
  	//通信切断
  	//NetErr_ExitNetSystem();
		//エラー画面描画
  	Local_ErrDispInit(fatal_error,FALSE);
    //無限ループ
    GFL_UI_SoftResetEnable(GFL_UI_SOFTRESET_SVLD);  //ハードリセットが効くようにフラグを落とす
    do{
      MachineSystem_Main(); //ハードリセット用
    }while(1);
	}
  
	if(nes->status == NET_ERR_STATUS_REQ)
	{
    BOOL isWifi = FALSE;
    if( nes->net_type == GFL_NET_TYPE_WIFI 
        || nes->net_type == GFL_NET_TYPE_WIFI_LOBBY 
        || nes->net_type == GFL_NET_TYPE_WIFI_GTS )
    {
      isWifi = TRUE;
    }
    
    if( isWifi == TRUE )
    {
  		//エラー画面描画(切断待ち表示
  		Local_ErrDispInit(fatal_error,TRUE);
  	}

    //通信ライブラリ終了待ち 
    NetErr_ExitNetSystem();

    if( isWifi == TRUE )
    {
  		//エラー画面描画(文字の書き換えのみ
    	nes->h_intr = GX_HBlankIntr(FALSE);
    	nes->v_intr = GX_VBlankIntr(FALSE);
    	Local_ErrMessagePrint(fatal_error,FALSE);
    }
    else
    {
  		//エラー画面描画
  		Local_ErrDispInit(fatal_error,FALSE);
    }
		//Local_ErrDispInit(fatal_error,FALSE);
		
//		OS_SpinWait(10000);
		
#ifdef MULTI_BOOT_MAKE  //マルチブート処理
		while( TRUE )
		{
      Local_ErrUpdate(); //常に抜けれない
    }

#endif


#ifndef DEBUG_ERR_THROUGH
		while((PAD_Read() & ERR_DISP_END_BUTTON) != 0){
			Local_ErrUpdate();	//ボタンを一度離すまで待つ
		}
		
		while((PAD_Read() & ERR_DISP_END_BUTTON) == 0){
      Local_ErrUpdate();	//エラー画面終了ボタンが押されるまで待つ
		}
#endif //DEBUG_ERR_THROUGH
		
		//エラー画面終了
		Local_ErrDispExit(FALSE);
		NetErr_ErrWorkInit();
		nes->key_timer = 0;
  	return TRUE;
	}
	
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   画面情報を退避し、エラー画面を表示
 *
 * @param   fatal_error   TRUE:リセットを促すエラー(Aで抜けられない) 
 * @param   isExitWait    TRUE:切断待ちを表示する
 * 
 */
//--------------------------------------------------------------
static void Local_ErrDispInit(BOOL fatal_error,BOOL isExitWait)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	
	if(Local_SystemOccCheck() == FALSE){
		return;
	}
	
	//書き換える画面が見えないように、まずマスター輝度を退避し、真っ暗に設定
	nes->master_brightness = GX_GetMasterBrightness();
	nes->master_brightness_sub = GXS_GetMasterBrightness();
	GX_SetMasterBrightness(-16);
	GXS_SetMasterBrightness(-16);
	
	//割り込みを止める
	if( isExitWait == FALSE )
	{
  	nes->h_intr = GX_HBlankIntr(FALSE);
  	nes->v_intr = GX_VBlankIntr(FALSE);
  }
	
	//VRAMバンク情報退避
	nes->bg_bank = GX_ResetBankForBG();
	nes->bg_bank_sub = GX_ResetBankForSubBG();
	GX_SetBankForBG(GX_VRAM_BG_128_C);
	OS_TPrintf("VRAM BANK = %d\n", nes->bg_bank);

	//dispcnt退避
	nes->dispcnt = GX_GetDispCnt();
	GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
	GX_SetVisiblePlane(GX_PLANEMASK_BG1);
	GX_SetVisibleWnd(GX_WNDMASK_NONE);

  nes->sub_wnd  = GXS_GetVisibleWnd();
  GXS_SetVisibleWnd(GX_WNDMASK_NONE);

	//BG1Control退避
	nes->bg1cnt = G2_GetBG1Control();
	G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, 
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, nes->bg1cnt.bgExtPltt);
	G2_BG1Mosaic(FALSE);
	G2_SetBG1Offset(0, 0);

	//VRAMのデータを退避(念のため上でBG1の設定をしてから行っている)
	GFL_STD_MemCopy16(G2_GetBG1CharPtr(), nes->push_char_p, NETERR_PUSH_CHARVRAM_SIZE);
	GFL_STD_MemCopy16(G2_GetBG1ScrPtr(), nes->push_scrn_p, NETERR_PUSH_SCRNVRAM_SIZE);
	GFL_STD_MemCopy16((void*)HW_PLTT, nes->push_pltt_p, NETERR_PUSH_PLTTVRAM_SIZE);

	//取得関数がないのでレジスタから取得している
  nes->g2_blend_cnt  = reg_G2_BLDCNT;
  nes->g2_blend_cnt  = reg_G2S_DB_BLDCNT;
	G2_BlendNone();
	G2S_BlendNone();

  //下画面の待避
  nes->sub_visible = GFL_DISP_GetSubVisible();
  GFL_DISP_GXS_SetVisibleControlDirect( 0 );

  //下画面はバックドロップを使うので待避
  nes->sub_bd_color  = *((u16 *)HW_DB_BG_PLTT);
  *((u16 *)HW_DB_BG_PLTT)  = 0x7eea;

	//エラー画面描画
	Local_ErrDispDraw();
	Local_ErrMessagePrint(fatal_error,isExitWait);

	//表示ON
	GX_SetMasterBrightness(0);
	GXS_SetMasterBrightness(0);
}

//--------------------------------------------------------------
/**
 * @brief   エラー画面を終了させ、元の画面に復帰させる
 */
//--------------------------------------------------------------
static void Local_ErrDispExit(BOOL is_black_continue)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	int x, y;

	if(Local_SystemOccCheck() == FALSE){
		return;
	}
	
	//書き換える画面が見えないように、真っ暗に設定
	GX_SetMasterBrightness(-16);
	GXS_SetMasterBrightness(-16);
	
  //下画面はバックドロップを使うので待避
  *((u16 *)HW_DB_BG_PLTT)  = nes->sub_bd_color;

  //下画面の描画設定
  GFL_DISP_GXS_SetVisibleControlDirect( nes->sub_visible );

  //ブレンドの設定
  reg_G2_BLDCNT     = nes->g2_blend_cnt;
  reg_G2S_DB_BLDCNT = nes->g2s_blend_cnt;

	//VRAM復帰
	GFL_STD_MemCopy16(nes->push_char_p, G2_GetBG1CharPtr(), NETERR_PUSH_CHARVRAM_SIZE);
	GFL_STD_MemCopy16(nes->push_scrn_p, G2_GetBG1ScrPtr(), NETERR_PUSH_SCRNVRAM_SIZE);
	GFL_STD_MemCopy16(nes->push_pltt_p, (void*)HW_PLTT, NETERR_PUSH_PLTTVRAM_SIZE);

	//BG1Control復帰
	G2_SetBG1Control(nes->bg1cnt.screenSize, nes->bg1cnt.colorMode,
		nes->bg1cnt.screenBase, nes->bg1cnt.charBase, nes->bg1cnt.bgExtPltt);
	G2_BG1Mosaic(nes->bg1cnt.mosaic);
	GFL_NET_BG1PosGet(&x, &y);
	G2_SetBG1Offset(x, y);
	
	//dispcnt復帰
	GX_SetGraphicsMode(nes->dispcnt.dispMode, nes->dispcnt.bgMode, nes->dispcnt.bg0_2d3d);
	GX_SetVisiblePlane(nes->dispcnt.visiblePlane);
	GX_SetVisibleWnd(nes->dispcnt.visibleWnd);

  GXS_SetVisibleWnd(nes->sub_wnd);

	//VRAMバンク情報復帰
  GX_ResetBankForSubBG();
	GX_ResetBankForBG();
	GX_SetBankForBG(nes->bg_bank);
	GX_SetBankForSubBG(nes->bg_bank_sub);

	//割り込み復帰
	GX_HBlankIntr(nes->h_intr);
	GX_VBlankIntr(nes->v_intr);
	
	//表示ON
  OS_WaitIrq(TRUE, OS_IE_V_BLANK);
  if( !is_black_continue )
  {
    GX_SetMasterBrightness(nes->master_brightness);
    GXS_SetMasterBrightness(nes->master_brightness_sub);
  }
}

//--------------------------------------------------------------
/**
 * @brief   通信エラー画面制御システムが有効か調べる
 *
 * @retval  TRUE:有効、FALSE:無効
 */
//--------------------------------------------------------------
static BOOL Local_SystemOccCheck(void)
{
	if(NetErrSystem.push_char_p != NULL){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   エラー画面描画
 */
//--------------------------------------------------------------
static void Local_ErrDispDraw(void)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	u32 data_size;
	void *arcData;
	NNSG2dCharacterData* charData;
	NNSG2dScreenData* scrnData;
	NNSG2dPaletteData* palData;
	BOOL  cmpFlag;
	NNSG2dPaletteCompressInfo*  cmpInfo;
	
	GFL_STD_MemClear32(G2_GetBG1CharPtr(), NETERR_PUSH_CHARVRAM_SIZE);

	//キャラクタ
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NCGR, 0, HEAPID_NET_TEMP);
	if(NNS_G2dGetUnpackedBGCharacterData(arcData, &charData)){
		DC_FlushRange(charData->pRawData, charData->szByte);
		//念のためDMAを使用するのは避ける
		GFL_STD_MemCopy16(charData->pRawData, G2_GetBG1CharPtr(), charData->szByte);
		OS_TPrintf("エラー画面：キャラクタサイズ＝%dbyte\n", charData->szByte);
		GF_ASSERT(MESSAGE_START_CHARNO*0x20 >= charData->szByte);	//このASSERTに引っかかる場合はBG_DATA_SIZEを大きくする必要がある。その分エラーメッセージに割けるキャラクタ領域が少なくなるので注意
	}
	GFL_HEAP_FreeMemory(arcData);
	
	//スクリーン
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NSCR, 0, HEAPID_NET_TEMP);
	if( NNS_G2dGetUnpackedScreenData( arcData, &scrnData ) ){
		DC_FlushRange(scrnData->rawData, scrnData->szByte);
		GFL_STD_MemCopy16(scrnData->rawData, G2_GetBG1ScrPtr(), scrnData->szByte);
	}
	GFL_HEAP_FreeMemory( arcData );
	
	//パレット
	arcData = GFL_ARC_UTIL_Load(ARCID_NET_ERR, NARC_net_err_net_err_NCLR, 0, HEAPID_NET_TEMP);
	cmpFlag = NNS_G2dGetUnpackedPaletteCompressInfo( arcData, &cmpInfo );
	if( NNS_G2dGetUnpackedPaletteData( arcData, &palData ) ){
		DC_FlushRange( palData->pRawData, NETERR_PUSH_PLTTVRAM_SIZE );
		GFL_STD_MemCopy16(palData->pRawData, (void*)HW_BG_PLTT, NETERR_PUSH_PLTTVRAM_SIZE);
	}
	GFL_HEAP_FreeMemory( arcData );
}


//--------------------------------------------------------------
/**
 * @brief   WIFIエラーメッセージ取得
 * @param   fatal_error   TRUE:リセットを促すエラー(Aで抜けられない) 
 * 
 */
//--------------------------------------------------------------
#ifndef MULTI_BOOT_MAKE  //通常時処理
static const u16 twlerror[]={
dwctwl_error_0001,
dwc_error_0002,
dwctwl_error_0003,
dwc_error_0004,
dwc_error_0005,
dwc_error_0006,
dwctwl_error_0007,
dwc_error_0008,
dwctwl_error_0009,
dwc_error_0010,
dwc_error_0011,
dwc_error_0012,
dwc_error_0013,
dwc_error_0014,
dwc_error_0015,
dwctwl_error_0016,
};

static const u16 nitroerror[]={
dwc_error_0001,
dwc_error_0002,
dwc_error_0003,
dwc_error_0004,
dwc_error_0005,
dwc_error_0006,
dwc_error_0007,
dwc_error_0008,
dwc_error_0009,
dwc_error_0010,
dwc_error_0011,
dwc_error_0012,
dwc_error_0013,
dwc_error_0014,
dwc_error_0015,
dwc_error_0016,
};
#endif //MULTI_BOOT_MAKE  //通常時処理


static u32 _wifierrMessage(GFL_NETSTATE_DWCERROR* pErr, int default_msg)
{
#ifndef MULTI_BOOT_MAKE  //通常時処理
  u32 msgno = default_msg;
  switch(pErr->errorUser){
  case ERRORCODE_HEAP:
    return dwc_error_0014;
  case ERRORCODE_DISCONNECT:
    return dwc_message_0022;
  case ERRORCODE_CRC:
  case ERRORCODE_SYSTEM:
  case ERRORCODE_SENDQUEUE:
  case ERRORCODE_USER_TIMEOUT:
  case ERRORCODE_NHTTP:
    return dwc_error_0015;
  case ERRORCODE_TIMEOUT:
    return dwc_message_0022;
  }
  if(11 == NetErrSystem.wifi_msg){
    return dwc_error_0015;
  }
  if( NetErrSystem.wifi_msg >= 0 )
  {
    if(DS_SYSTEM_IsRunOnTwl()){
      msgno = twlerror[NetErrSystem.wifi_msg];
    }
    else{
      msgno = nitroerror[NetErrSystem.wifi_msg];
    }
  }
  else
  {
    msgno = dwc_error_0012;
  }
  return msgno;
#else
  return 0;
#endif
}


//--------------------------------------------------------------
/**
 * @brief   エラーメッセージ表示
 *
 * @param   fatal_error   TRUE:リセットを促すエラー(Aで抜けられない) 
 * 
 */
//--------------------------------------------------------------
static void Local_ErrMessagePrint(BOOL fatal_error,BOOL isExitWait)
{
	NET_ERR_SYSTEM *nes = &NetErrSystem;
	GFL_BMP_DATA *bmpdata;
	int x, y;
	u16 *scrn_vram;
	u16 data;
	u16 *test;
	
	//スクリーンをBMP領域の形で埋め尽くし
	scrn_vram = G2_GetBG1ScrPtr();
	data = MESSAGE_START_CHARNO | (MESSAGE_PALNO << 12);
	for(y = MESSAGE_START_Y; y < MESSAGE_START_Y+MESSAGE_Y_LEN; y++){
		for(x = MESSAGE_START_X; x < MESSAGE_START_X+MESSAGE_X_LEN; x++){
			scrn_vram[y*0x20+x] = data;
			data++;
		}
	}

	//BMPキャラクタ領域を文字背景色で埋める
	GFL_STD_MemFill16((void*)((u32)G2_GetBG1CharPtr() + MESSAGE_START_CHARNO*0x20), 
	  0x7777, MESSAGE_X_LEN * MESSAGE_Y_LEN * 32);
	
	//BMP作成
	bmpdata = GFL_BMP_CreateInVRAM((void*)((u32)G2_GetBG1CharPtr() + MESSAGE_START_CHARNO*0x20), 
		MESSAGE_X_LEN, MESSAGE_Y_LEN, GFL_BMP_16_COLOR, HEAPID_NET_TEMP);
	
	//メッセージOPEN
	{
		GFL_FONT		*fontHandle;
		GFL_MSGDATA		*mm;
		STRBUF  *strbuf;
    STRBUF  *src;
    WORDSET       *wordset;
    int msgno;

    wordset = WORDSET_Create( HEAPID_NET_TEMP );


		fontHandle = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_NET_TEMP );
		

#ifndef MULTI_BOOT_MAKE  //通常時処理
    {
      if( isExitWait == TRUE )
      {
        //切断待ち
        mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
  			ARCID_MESSAGE, NARC_message_wifi_system_dat, HEAPID_NET_TEMP);
  			msgno = WIFI_DISCONNECT_WAIT;
      }
      else
      if( nes->net_type == GFL_NET_TYPE_WIFI 
          || nes->net_type == GFL_NET_TYPE_WIFI_LOBBY 
          || nes->net_type == GFL_NET_TYPE_WIFI_GTS )
      { 
        //WIFIの場合
        int type;
        
        WORDSET_RegisterNumber( wordset, 0, nes->wifi_error.errorCode, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

        mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
  			ARCID_MESSAGE, NARC_message_wifi_system_dat, HEAPID_NET_TEMP);

        if( nes->msg_force )
        {
          msgno = nes->wifi_msg;
          nes->msg_force  = FALSE;
          OS_TPrintf("強制エラーメッセージ %d \n",msgno);
        }
        else
        {
          msgno = _wifierrMessage(&nes->wifi_error, nes->wifi_msg) ;      //WIFIで表示するメッセージを取得
          OS_TPrintf("エラーメッセージ %d \n",msgno);
        }
      }
      else
      { 
        //その他の場合
        //ワイヤレスや赤外線でのエラーか
        //エラーが発生していないのにNetErr_DispCallFatalを呼び出した場合
        WORDSET_RegisterNumber( wordset, 0, nes->error, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

        mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
            ARCID_MESSAGE, NARC_message_net_err_dat, HEAPID_NET_TEMP);

        if( fatal_error == TRUE )
        {
          msgno = net_error_0002;
        }
        else
        {
          msgno = net_error_0001;
        }
      }
    }
#else
    {
      //マルチブートでは常に電源を切ってもらう
      //MB_ERROR_MSG_FILEはArc名がポケシフターと映画転送で違うのでmustiboot/src/net_err_dl.cで定義
      WORDSET_RegisterNumber( wordset, 0, nes->error, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
          ARCID_MESSAGE, MB_ERROR_MSG_FILE, HEAPID_NET_TEMP);
      msgno = 1;
      
    }
#endif



    src     = GFL_MSG_CreateString(mm, msgno);
		strbuf  = GFL_MSG_CreateString(mm, msgno);
    WORDSET_ExpandStr( wordset, strbuf, src );
		PRINTSYS_PrintColor(bmpdata, 0, 0, strbuf, fontHandle, PRINTSYS_LSB_Make(4, 0xb, 7));

		GFL_STR_DeleteBuffer(strbuf);
		GFL_STR_DeleteBuffer(src);
		GFL_MSG_Delete(mm);
		
		GFL_FONT_Delete(fontHandle);

    WORDSET_Delete( wordset );
	}
	
	GFL_BMP_Delete(bmpdata);
}


//--------------------------------------------------------------
/**
 * @brief   エラー中も更新する必要があるもの
 */
//--------------------------------------------------------------
static void Local_ErrUpdate(void)
{
  MachineSystem_Main(); //ハードリセット用

  //LCDOFF対応
  GFL_UI_Main();
  //BGMが間延びする問題対応
  PMSND_Main();
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
  PMVOICE_Main();
#endif //MULTI_BOOT_MAKE
}
