//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wbt_download_proc.c
 *	@brief  WBTダウンロードプロセス
 *	@author	Toru=Nagihashi
 *	@data		2011.05.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"
#include "system/bmp_winframe.h"

//ネット
#include "net/network_define.h"
#include "net/net_common_download.h"
#include "net_app/wifi_login.h"
#include "net_app/wifi_logout.h"
#include "system/net_err.h"

//アーカイブ
#include "arc_def.h"
#include "net_app/connect_anm.h"
#include "font/font.naix"
#include "message.naix"
#include "wifi_login.naix"
#include "msg/msg_wbt_download.h"

//自分のモジュール
#include "wbt_download_net_graphic.h"
#include "wbt_download_net_util.h"

//外部
#include "wbt_download_net.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
FS_EXTERN_OVERLAY(common_download);
FS_EXTERN_OVERLAY(dpw_common);

// 描画フレーム
enum
{
  WBT_DOWNLOAD_NET_BG_BACK_S      = GFL_BG_FRAME0_S,
  WBT_DOWNLOAD_NET_BG_BACK_M      = GFL_BG_FRAME0_M,
  WBT_DOWNLOAD_NET_BG_BACK_BASE_M = GFL_BG_FRAME3_M,
  WBT_DOWNLOAD_NET_BG_WND_M       = GFL_BG_FRAME1_M,
  WBT_DOWNLOAD_NET_BG_YESNO_M     = GFL_BG_FRAME2_M,
};

// BGパレット
enum
{
  WBT_DOWNLOAD_NET_PLT_BG_MSG_M  = 12,
  WBT_DOWNLOAD_NET_PLT_BG_WND_M  = 13,
};

#if PM_LANG == LANG_JAPAN
#define WBT_DOWNLOAD_ATTRIBUTE_01 "WBT_J"
#else
#error
#define WBT_DOWNLOAD_ATTRIBUTE_01 "WBT_?"
#endif

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	BG
//=====================================
typedef struct
{
  CONNECT_BG_PALANM bg_palanm;
  u32 frame_bgchar;
  u32 frame_bgchar_sys;
} WB_DOWNLOAD_NET_BG;

//-------------------------------------
///	ワーク
//=====================================
typedef struct
{
  WBT_DOWNLOAD_NET_PARAM        *p_param;   //PROC引数
  WBTDLNET_SEQ_WORK             *p_seq;     //シーケンス
  WBT_DONLOAD_NET_GRAPHIC_WORK  *p_graphic; //描画設定
  COMMON_DOWNLOAD               *p_common_download; //通信
  WB_DOWNLOAD_NET_BG            bg;       //背景
  GFL_FONT                      *p_font;  //フォント
  PRINT_QUE                     *p_que;   //プリントキュー
  GFL_MSGDATA                   *p_msg;   //メッセージ
  WORDSET                       *p_word;  //単語登録
  WBTDLNET_TEXT_WORK            *p_text;  //テキスト面
} WB_DOWNLOAD_NET_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// プロセス
//=====================================
static GFL_PROC_RESULT Wbt_Download_Net_Proc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Wbt_Download_Net_Proc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Wbt_Download_Net_Proc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
/// SEQ
//=====================================
static void Wbt_Download_Net_Seq_Main( WBTDLNET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Wbt_Download_Net_Seq_Wifi( WBTDLNET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	BG初期化
//=====================================
static void Wbt_Download_Bg_Init( WB_DOWNLOAD_NET_BG *p_wk, HEAPID heapID );
static void Wbt_Download_Bg_Exit( WB_DOWNLOAD_NET_BG *p_wk );
static void Wbt_Download_Bg_Main( WB_DOWNLOAD_NET_BG *p_wk );
static void Wbt_Download_Bg_Load( WB_DOWNLOAD_NET_BG *p_wk, HEAPID heapID );
static void Wbt_Download_Bg_UnLoad( WB_DOWNLOAD_NET_BG *p_wk );

typedef enum
{
  WBT_DOWNLOAD_BG_DATA_ID_BGCHARA_TEXT,
  WBT_DOWNLOAD_BG_DATA_ID_BGCHARA_YESNO,
}WBT_DOWNLOAD_BG_DATA_ID;
static u32 Wbt_Download_Bg_GetData( const WB_DOWNLOAD_NET_BG *cp_wk, WBT_DOWNLOAD_BG_DATA_ID id );

//-------------------------------------
/// ETC
//=====================================
static u32 Wbt_Download_Net_GetDownloadIndexCallback( const DWCNdFileInfo *cp_fileInfo_array, u32 array_len );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	プロセスデータ
//=====================================
const GFL_PROC_DATA WBT_DOWNLOAD_NET_ProcData =
{
  Wbt_Download_Net_Proc_Init,
  Wbt_Download_Net_Proc_Main,
  Wbt_Download_Net_Proc_Exit,
};
//=============================================================================
/**
 *					プロセスデータ
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  コアプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Wbt_Download_Net_Proc_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WB_DOWNLOAD_NET_WORK *p_wk;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(common_download) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WBT_DOWNLOAD_NET, 0x50000 );

  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(WB_DOWNLOAD_NET_WORK), HEAPID_WBT_DOWNLOAD_NET );
  GFL_STD_MemClear( p_wk, sizeof(WB_DOWNLOAD_NET_WORK) );

  p_wk->p_param  = p_param_adrs;

  //共通モジュールの初期化
  p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WBT_DOWNLOAD_NET );
  p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_WBT_DOWNLOAD_NET );
  p_wk->p_msg		  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_download_dat, HEAPID_WBT_DOWNLOAD_NET );
  p_wk->p_word    = WORDSET_Create( HEAPID_WBT_DOWNLOAD_NET );


  //グラフィック初期化
  p_wk->p_graphic = WBT_DONLOAD_NET_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_WBT_DOWNLOAD_NET );

  //背景初期化
  Wbt_Download_Bg_Init( &p_wk->bg, HEAPID_WBT_DOWNLOAD_NET );

  //通信
  {
    COMMON_DOWNLOAD_INIT_PARAM  init_param;
    GFL_STD_MemClear( &init_param, sizeof(COMMON_DOWNLOAD_INIT_PARAM) );
    init_param.net_type     = COMMON_DOWNLOAD_NET_TYPE_WIFI_ND;
    init_param.p_gamedata   = p_wk->p_param->p_gamedata;
    init_param.buffer_size  = WBT_DOWNLOAD_MAX_SIZE;
    init_param.heapID       = HEAPID_WBT_DOWNLOAD_NET;
    p_wk->p_common_download = COMMON_DOWNLOAD_Init( &init_param );
  }

  GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_WBT_DOWNLOAD_NET );

  //テキスト面
  p_wk->p_text = WBTDLNET_TEXT_Init( WBT_DOWNLOAD_NET_BG_WND_M, 1, 19, 30, 4, WBT_DOWNLOAD_NET_PLT_BG_MSG_M, p_wk->p_font, HEAPID_WBT_DOWNLOAD_NET );
  WBTDLNET_TEXT_SetColor( p_wk->p_text, PRINTSYS_LSB_Make( 1, 2, 0xF ) );
  {
    u32 chara = Wbt_Download_Bg_GetData( &p_wk->bg, WBT_DOWNLOAD_BG_DATA_ID_BGCHARA_TEXT );
    WBTDLNET_TEXT_WriteWindowFrame( p_wk->p_text, chara, WBT_DOWNLOAD_NET_PLT_BG_WND_M );
  }
  WBTDLNET_TEXT_Print( p_wk->p_text, p_wk->p_msg, wbt_dl_ev_msg_01, WBTDLNET_TEXT_TYPE_WAIT );

  //シーケンス
  p_wk->p_seq = WBTDLNET_SEQ_Init( 5, p_wk, Wbt_Download_Net_Seq_Main, HEAPID_WBT_DOWNLOAD_NET );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  コアプロセス破棄
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Wbt_Download_Net_Proc_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WB_DOWNLOAD_NET_WORK *p_wk = p_wk_adrs;

  //シーケンス
  WBTDLNET_SEQ_Exit( p_wk->p_seq );

  //テキスト面破棄
  WBTDLNET_TEXT_Exit( p_wk->p_text );

  //通信破棄
  COMMON_DOWNLOAD_Exit( p_wk->p_common_download );

  //背景破棄
  Wbt_Download_Bg_Exit( &p_wk->bg );

  //グラフィック破棄
  WBT_DONLOAD_NET_GRAPHIC_Exit( p_wk->p_graphic );

  //共通モジュール破棄
  WORDSET_Delete( p_wk->p_word );
	GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );

  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_WBT_DOWNLOAD_NET );


  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(common_download) );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  コアプロセスメイン
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Wbt_Download_Net_Proc_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WB_DOWNLOAD_NET_WORK *p_wk = p_wk_adrs;

  //エラー処理
  switch( COMMON_DOWNLOAD_HandleError(p_wk->p_common_download) )
  {
  case COMMON_DOWNLOAD_ERROR_TYPE_NONE:        //エラーが起こっていません。
    /*  なにもしない  */
    break;
  case COMMON_DOWNLOAD_ERROR_TYPE_RETURN:      //再度通信をすればつながります。
    /*   エラー時のシーケンス遷移など　 */
    COMMON_DOWNLOAD_ClearError( p_wk->p_common_download );
    return GFL_PROC_RES_CONTINUE;
  case COMMON_DOWNLOAD_ERROR_TYPE_DISCONNECT:  //アプリ終了
    p_wk->p_param->result = WBT_DOWNLOAD_NET_RESULT_ERROR;
    return GFL_PROC_RES_FINISH;
  default:
    GF_ASSERT(0);
  }

  //通信前処理
  COMMON_DOWNLOAD_MainTop( p_wk->p_common_download );

  //シーケンス
  WBTDLNET_SEQ_Main( p_wk->p_seq );

  //描画
  WBT_DONLOAD_NET_GRAPHIC_2D_Draw( p_wk->p_graphic );

  //背景アニメ
  Wbt_Download_Bg_Main( &p_wk->bg );

  //キューメイン
  PRINTSYS_QUE_Main( p_wk->p_que );

  //テキスト面描画
  WBTDLNET_TEXT_Draw( p_wk->p_text );

  //通信後処理
  COMMON_DOWNLOAD_MainTail( p_wk->p_common_download );

  //終了チェック
  if( WBTDLNET_SEQ_IsEnd( p_wk->p_seq ) )
  {
    return GFL_PROC_RES_FINISH;
  }
  else
  {
    return GFL_PROC_RES_CONTINUE;
  }
}

//=============================================================================
/**
 *					SEQ
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  メインシーケンス
 *
 *	@param	WBTDLNET_SEQ_WORK *p_wk　シーケンスワーク
 *	@param	*p_seq                シーケンス
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Wbt_Download_Net_Seq_Main( WBTDLNET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_FADEIN,
    SEQ_MAIN,
    SEQ_FADEOUT,
    SEQ_END,
  };

  WB_DOWNLOAD_NET_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_FADEIN:
    WBTDLNET_SEQ_Call( p_seqwk, WBTDLNET_SEQ_FadeInBlack );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    WBTDLNET_SEQ_Call( p_seqwk, Wbt_Download_Net_Seq_Wifi );
    (*p_seq)++;
    break;
  case SEQ_FADEOUT:
    WBTDLNET_SEQ_Call( p_seqwk, WBTDLNET_SEQ_FadeOutBlack );
    (*p_seq)++;
    break;
  case SEQ_END:
    WBTDLNET_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  Wi-Fiダウンロードシーケンス
 *
 *	@param	WBTDLNET_SEQ_WORK *p_wk　シーケンスワーク
 *	@param	*p_seq                シーケンス
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Wbt_Download_Net_Seq_Wifi( WBTDLNET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_START_SEARCH_DOWNLOAD,
    SEQ_WAIT_SEARCH_DOWNLOAD,
    SEQ_CHECK_DATA,
    SEQ_MSG_WAIT,
    SEQ_END,
  };

  WB_DOWNLOAD_NET_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_START_SEARCH_DOWNLOAD:
    {
      COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM param;
      GFL_STD_MemClear( &param, sizeof(COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM) );
      param.download_search_num   = 1;
      param.cp_attribute_01   = WBT_DOWNLOAD_ATTRIBUTE_01;
      param.cp_attribute_02   = "";
      param.cp_attribute_03   = "";
      param.GetDownloadIndexCallback = Wbt_Download_Net_GetDownloadIndexCallback;
      param.file_size = 0x10; //TODO filesize

      OS_TPrintf( "Wi-Fiサーチリクエスト\n" );
      if( COMMON_DOWNLOAD_RequestState( p_wk->p_common_download, COMMON_DOWNLOAD_REQ_WIFI_ND_START_SEARCH_DOWNLOAD, &param ) )
      {
        OS_TPrintf( "Wi-Fiサーチリクエストが通りました\n" );
        (*p_seq)++;
      }
    }
    break;
  case SEQ_WAIT_SEARCH_DOWNLOAD:
    if( COMMON_DOWNLOAD_GetState(p_wk->p_common_download ) == COMMON_DOWNLOAD_STATE_WAIT )
    {
      const u8 *cp_data_adrs = COMMON_DOWNLOAD_GetDownloadData( p_wk->p_common_download );
      COMMON_DOWNLOAD_RESULT result = COMMON_DOWNLOAD_GetDownloadResult( p_wk->p_common_download );

      if( result == COMMON_DOWNLOAD_RESULT_SUCCESS )
      {
        GFL_STD_MemCopy( cp_data_adrs, p_wk->p_param->p_buffer, WBT_DOWNLOAD_MAX_SIZE );
      }

      OS_TPrintf( "Wi-Fiサーチ終了\n" );
      OS_TPrintf( "download result%d\n", result );

#ifdef PM_DEBUG
#if 0
      if( result == COMMON_DOWNLOAD_RESULT_SUCCESS )
      {
        int i;
        for( i = 0; i < WBT_DOWNLOAD_MAX_SIZE; i++ )
        {
          OS_TPrintf( "%x ", cp_data_adrs[i] );
        }
        OS_TPrintf( "\n" );
      }
#endif
#endif

      (*p_seq)++;
    }
    else
    {
#if 0
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      {
        if( COMMON_DOWNLOAD_RequestState( p_wk->p_common_download, COMMON_DOWNLOAD_REQ_WIFI_ND_CANCEL_DOWNLOAD, NULL ) )
        {
          OS_TPrintf( "Wi-Fiキャンセルリクエスト\n" );
        }
      }
#endif
    }
    break;

  case SEQ_CHECK_DATA:
    {
      BOOL is_success = FALSE;
      switch( COMMON_DOWNLOAD_GetDownloadResult( p_wk->p_common_download ) )
      {
      case COMMON_DOWNLOAD_RESULT_SUCCESS:   //受信成功
        //データチェック
        if( 0 ) //TODO
        {
          //CRCエラー
          WBTDLNET_TEXT_Print( p_wk->p_text, p_wk->p_msg, wbt_dl_error_01, WBTDLNET_TEXT_TYPE_STREAM );
        }
        else if( 0 )  //TODO
        {
          //カセットバージョンが違う
          WBTDLNET_TEXT_Print( p_wk->p_text, p_wk->p_msg, wbt_dl_error_04, WBTDLNET_TEXT_TYPE_STREAM );
        }
        else if( 0 )  //TODO
        {
          //言語コードが違う
          WBTDLNET_TEXT_Print( p_wk->p_text, p_wk->p_msg, wbt_dl_error_05, WBTDLNET_TEXT_TYPE_STREAM );
        }
        else
        {
          //正常
          is_success = TRUE;
          WBTDLNET_TEXT_Print( p_wk->p_text, p_wk->p_msg, wbt_dl_ev_msg_06, WBTDLNET_TEXT_TYPE_STREAM );
        }
        break;
      case COMMON_DOWNLOAD_RESULT_TIMEOUT:   //タイムアウト(２分)  WiFiでしか返ってこない
        WBTDLNET_TEXT_Print( p_wk->p_text, p_wk->p_msg, wbt_dl_error_03, WBTDLNET_TEXT_TYPE_STREAM );
        break;
      default:
        GF_ASSERT(0); //こない
      case COMMON_DOWNLOAD_RESULT_NONE:      //なにも受信していない
        GF_ASSERT(0); //受信終了しているはず
      case COMMON_DOWNLOAD_RESULT_CANCEL:    //キャンセルした
        GF_ASSERT(0); //キャンセルはない
      case COMMON_DOWNLOAD_RESULT_FAILED:    //通信がつながらなかった  WiFiでしか返ってこない
      case COMMON_DOWNLOAD_RESULT_EMPTY:     //受信するものがない  WiFiでしか返ってこない
        WBTDLNET_TEXT_Print( p_wk->p_text, p_wk->p_msg, wbt_dl_error_02, WBTDLNET_TEXT_TYPE_STREAM );
        break;
      }

      if( is_success )
      {
        p_wk->p_param->result = WBT_DOWNLOAD_NET_RESULT_SUCCESS;
      }
      else
      {
        p_wk->p_param->result = WBT_DOWNLOAD_NET_RESULT_DATA_ILLIGAL;
      }
    }
    (*p_seq)++;
    break;

  case SEQ_MSG_WAIT:
    if( WBTDLNET_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    WBTDLNET_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  BG初期化
 *
 *	@param	WB_DOWNLOAD_NET_BG *p_wk  ワーク
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void Wbt_Download_Bg_Init( WB_DOWNLOAD_NET_BG *p_wk, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(WB_DOWNLOAD_NET_BG) );
  Wbt_Download_Bg_Load( p_wk, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BG破棄
 *
 *	@param	WB_DOWNLOAD_NET_BG *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Wbt_Download_Bg_Exit( WB_DOWNLOAD_NET_BG *p_wk )
{
  Wbt_Download_Bg_UnLoad( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BGメイン
 *
 *	@param	WB_DOWNLOAD_NET_BG *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Wbt_Download_Bg_Main( WB_DOWNLOAD_NET_BG *p_wk )
{
  ConnectBGPalAnm_Main( &p_wk->bg_palanm );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BG読み込み
 *
 *	@param	WB_DOWNLOAD_NET_BG *p_wk  ワーク
 *	@param	heapID                    ヒープID
 */
//-----------------------------------------------------------------------------
static void Wbt_Download_Bg_Load( WB_DOWNLOAD_NET_BG *p_wk, HEAPID heapID )
{

  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFI_LOGIN, heapID );
  u32 char_ofs;
  
  GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifi_login_connect_win_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  heapID);
  // サブ画面BG0キャラ転送
  char_ofs = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_wifi_login_connect_win_NCGR,
                                                                WBT_DOWNLOAD_NET_BG_BACK_S, 0, 0, heapID);

  // サブ画面BG0スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_wifi_login_connect_win2_d_NSCR,
                                            WBT_DOWNLOAD_NET_BG_BACK_S, 0,
                                            GFL_ARCUTIL_TRANSINFO_GetPos(char_ofs), 0, 0,
                                            heapID);


  GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifi_login_connect_win_NCLR,
                                    PALTYPE_MAIN_BG, 0, 0,  heapID);
  // メイン画面BG0キャラ転送
  char_ofs = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_wifi_login_connect_win_NCGR,
                                                                WBT_DOWNLOAD_NET_BG_BACK_M, 0, 0, heapID);

  // メイン画面BG0スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_wifi_login_connect_win1_u_NSCR,
                                            WBT_DOWNLOAD_NET_BG_BACK_M, 0,
                                            GFL_ARCUTIL_TRANSINFO_GetPos(char_ofs), 0, 0,
                                            heapID);

  // メイン画面BG3キャラ転送
  char_ofs = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_wifi_login_connect_win_NCGR,
                                                                WBT_DOWNLOAD_NET_BG_BACK_BASE_M, 0, 0, heapID);

  // メイン画面BG3スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_wifi_login_connect_win2_u_NSCR,
                                            WBT_DOWNLOAD_NET_BG_BACK_BASE_M, 0,
                                            GFL_ARCUTIL_TRANSINFO_GetPos(char_ofs), 0, 0,
                                            heapID);


  //パレットアニメシステム作成
  ConnectBGPalAnm_InitBg( &p_wk->bg_palanm, p_handle, NARC_wifi_login_connect_ani_NCLR, heapID, WBT_DOWNLOAD_NET_BG_BACK_M, WBT_DOWNLOAD_NET_BG_BACK_BASE_M );

  GFL_ARC_CloseDataHandle(p_handle);



  // フレーム生成
  GFL_BG_AllocCharacterArea( WBT_DOWNLOAD_NET_BG_WND_M, GFL_BG_1CHRDATASIZ, GFL_BG_CHRAREA_GET_F ); //0charをあけとく
  GFL_BG_AllocCharacterArea( WBT_DOWNLOAD_NET_BG_YESNO_M, GFL_BG_1CHRDATASIZ, GFL_BG_CHRAREA_GET_F ); //0charをあけとく
  p_wk->frame_bgchar = BmpWinFrame_GraphicSetAreaMan( WBT_DOWNLOAD_NET_BG_WND_M, WBT_DOWNLOAD_NET_PLT_BG_WND_M, MENU_TYPE_SYSTEM, heapID);
  p_wk->frame_bgchar_sys = BmpWinFrame_GraphicSetAreaMan( WBT_DOWNLOAD_NET_BG_YESNO_M, WBT_DOWNLOAD_NET_PLT_BG_WND_M, MENU_TYPE_SYSTEM, heapID);

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                              0x20*WBT_DOWNLOAD_NET_PLT_BG_MSG_M, 0x20, heapID);
}
//----------------------------------------------------------------------------
/**
 *	@brief  BG破棄
 *
 *	@param	WB_DOWNLOAD_NET_BG *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Wbt_Download_Bg_UnLoad( WB_DOWNLOAD_NET_BG *p_wk )
{
  GFL_BG_FreeCharacterArea( WBT_DOWNLOAD_NET_BG_WND_M,
      GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_bgchar),
      GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->frame_bgchar));

  GFL_BG_FreeCharacterArea( WBT_DOWNLOAD_NET_BG_YESNO_M,
      GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_bgchar_sys),
      GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->frame_bgchar_sys));

  ConnectBGPalAnm_End( &p_wk->bg_palanm );
  G2_BlendNone();
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ取得
 *
 *	@param	const WB_DOWNLOAD_NET_BG *cp_wk ワーク
 *	@param	id  取得するデータ
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
static u32 Wbt_Download_Bg_GetData( const WB_DOWNLOAD_NET_BG *cp_wk, WBT_DOWNLOAD_BG_DATA_ID id )
{
  switch( id )
  {
  case WBT_DOWNLOAD_BG_DATA_ID_BGCHARA_TEXT:
    return GFL_ARCUTIL_TRANSINFO_GetPos(cp_wk->frame_bgchar);
  case WBT_DOWNLOAD_BG_DATA_ID_BGCHARA_YESNO:
    return GFL_ARCUTIL_TRANSINFO_GetPos(cp_wk->frame_bgchar_sys);
  default:
    GF_ASSERT(0);
    return 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ファイルテーブルからダウンロードするインデックスを返す
 *
 *	@param	const DWCNdFileInfo *cp_fileInfo_array  ファイルデータテーブル
 *	@param	array_len テーブル数
 *
 *	@return ダウンロードするインデックス
 */
//-----------------------------------------------------------------------------
static u32 Wbt_Download_Net_GetDownloadIndexCallback( const DWCNdFileInfo *cp_fileInfo_array, u32 array_len )
{
  DWCNdFileInfo   empty = {0};

  int i;
  for( i = 0; i < array_len; i++ )
  {
    //中身が空じゃないものの先頭を返す
    if( GFL_STD_MemComp( &empty, &cp_fileInfo_array[i], sizeof(DWCNdFileInfo) ) != 0 )
    {
      return i;
    }
  }

  GF_ASSERT(0);
  return 0;
}
