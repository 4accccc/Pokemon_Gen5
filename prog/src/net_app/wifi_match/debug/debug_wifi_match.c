//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_wifi_match.c
 *	@brief  サーバーアクセスデバッグ
 *	@author	Toru=Nagihashi
 *	@data		2009.12.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//-------------------------------------
///	インクルード
//=====================================
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"
#include "system/gfl_use.h"
#include "debug/debug_str_conv.h"
#include "gamesystem/game_data.h"

//アーカイブ
#include "font/font.naix"
#include "arc_def.h"
#include "message.naix"

//自分のモジュール
#include "../wifibattlematch_graphic.h"
#include "../wifibattlematch_util.h"
#include "../wifibattlematch_net.h"
#include "../atlas_syachi2ds_v1.h"

//外部公開
#include "net_app/debug_wifi_match.h"

//デバッグ
//#include <wchar.h>          //wcslen

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(dpw_common);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define DEBUG_BMPWIN_MAX  (20)
#define DEBUG_DATA_MAX  (20)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	BGフレーム
//=====================================
enum
{
	//メイン画面
	BG_FRAME_M_FONT =	GFL_BG_FRAME0_M, 

	//サブ画面
	BG_FRAME_S_NUM =	GFL_BG_FRAME0_S, 
	BG_FRAME_S_FONT	=	GFL_BG_FRAME1_S,
} ;

//-------------------------------------
///	パレット
//=====================================
enum
{
	//メイン画面BG
	PLT_BG_M	  =	0,
	PLT_FONT_M	= 15,

	//サブ画面BG
	PLT_BG_S    = 0,
	PLT_FONT_S	= 15,

	//メイン画面OBJ
	
	//サブ画面OBJ
} ;

//-------------------------------------
///	入力窓
//=====================================
typedef struct 
{
  u8  x;
  u8  y;
  u8  w;
  u8  h;
}INPUTWIN_PARAM;

//-------------------------------------
///	数字入力
//=====================================
typedef struct 
{
  GFL_BMPWIN  *p_bmpwin;
  GFL_FONT    *p_font;
  s32         min;
  s32         max;
  s32         now;
  u8          keta;
  u8          active;
  u16         dummy;
  STRBUF      *p_strbuf;
} NUMINPUT_WORK;

//-------------------------------------
///	ATLASデバッグ
//=====================================
typedef struct 
{
  u32         seq;
  u32         pre_seq;
  HEAPID      heapID;
  GFL_BMPWIN  *p_bmpwin[DEBUG_BMPWIN_MAX];
  u32         recv_data[DEBUG_DATA_MAX];
  u32         send_data[2][DEBUG_DATA_MAX];
  BOOL        is_auth;

  WIFIBATTLEMATCH_NET_WORK  *p_net;
  WIFIBATTLEMATCH_GDB_RND_SCORE_DATA  score;
  WIFIBATTLEMATCH_SC_DEBUG_DATA       report;
  WBM_TEXT_WORK             *p_text;
  GFL_FONT                  *p_font;
  PRINT_QUE                 *p_que;
  NUMINPUT_WORK             numinput;
  u32                       select;
  STRBUF                    *p_strbuf;
  BOOL                      is_you;
  SAVE_CONTROL_WORK         *p_save;
} DEBUG_ATLAS_WORK;

//-------------------------------------
///	SAKEデバッグ
//=====================================
typedef struct 
{
  u32         seq;
  HEAPID      heapID;
  GFL_BMPWIN  *p_bmpwin[DEBUG_BMPWIN_MAX];
  u32         data[DEBUG_DATA_MAX];
  WIFIBATTLEMATCH_NET_WORK  *p_net;
  WIFIBATTLEMATCH_GDB_RND_SCORE_DATA  score;
  WBM_TEXT_WORK             *p_text;
  GFL_FONT                  *p_font;
  PRINT_QUE                 *p_que;
  NUMINPUT_WORK             numinput;
  u32                       select;
  STRBUF                    *p_strbuf;
} DEBUG_SAKE_WORK;

//-------------------------------------
///	ワーク
//=====================================
typedef struct 
{
  //グラフィック設定
	WIFIBATTLEMATCH_GRAPHIC_WORK	*p_graphic;

  //SAKE
  DEBUG_SAKE_WORK               sake;

  //ATLAS
  DEBUG_ATLAS_WORK              atlas;

} WIFIBATTLEMATCH_DEBUG_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_DEBUG_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_DEBUG_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_DEBUG_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	ウィンドウデータ
//=====================================
static void INPUTWIN_GetRange( u32 key, INPUTWIN_PARAM *p_wk );

//-------------------------------------
///	SAKE
//=====================================
static void SAKE_Init( DEBUG_SAKE_WORK *p_wk,WIFI_LIST *p_wifilist, HEAPID heapID );
static void SAKE_Exit( DEBUG_SAKE_WORK *p_wk );
static BOOL SAKE_Main( DEBUG_SAKE_WORK *p_wk );
static void Sake_CreateDisplay( DEBUG_SAKE_WORK *p_wk, HEAPID heapID );
static void Sake_DeleteDisplay( DEBUG_SAKE_WORK *p_wk );
static void Sake_UpdateDisplay( DEBUG_SAKE_WORK *p_wk );

//-------------------------------------
///	ATLAS
//=====================================
static void ATLAS_Init( DEBUG_ATLAS_WORK *p_wk, WIFI_LIST *p_wifilist, SAVE_CONTROL_WORK* p_save, HEAPID heapID );
static void ATLAS_Exit( DEBUG_ATLAS_WORK *p_wk );
static BOOL ATLAS_Main( DEBUG_ATLAS_WORK *p_wk );
static void Atlas_CreateRecvDisplay( DEBUG_ATLAS_WORK *p_wk, HEAPID heapID );
static void Atlas_DeleteRecvDisplay( DEBUG_ATLAS_WORK *p_wk );
static void Atlas_UpdateRecvDisplay( DEBUG_ATLAS_WORK *p_wk );
static void Atlas_CreateReportDisplay( DEBUG_ATLAS_WORK *p_wk, HEAPID heapID );
static void Atlas_DeleteReportDisplay( DEBUG_ATLAS_WORK *p_wk );
static void Atlas_UpdateReportDisplay( DEBUG_ATLAS_WORK *p_wk );

//-------------------------------------
///	NUMINPUT
//=====================================
static void NUMINPUT_Init( NUMINPUT_WORK *p_wk, u8 frm, u8 x, u8 y , u8 w, u8 h, u8 plt, GFL_FONT *p_font, HEAPID heapID );
static void NUMINPUT_Exit( NUMINPUT_WORK *p_wk );
static void NUMINPUT_Start( NUMINPUT_WORK *p_wk, u8 keta, s32 min, s32 max, s32 now );
static BOOL NUMINPUT_Main( NUMINPUT_WORK *p_wk );
static u16  NUMINPUT_GetNum( const NUMINPUT_WORK *cp_wk );
static u32 NumInput_AddKeta( u32 now, u32 keta, s8 dir );
static void NumInput_Print( NUMINPUT_WORK *p_wk );

//-------------------------------------
///	etc
//=====================================
static void PRINT_GetStrWithNumber( STRBUF *p_dst, const char *cp_src, ... );
static BOOL BMPWINIsCollisionRxP( const GFL_BMPWIN *cp_bmpwin, u32 x, u32 y );
static u8 GetFig( u32 now, u8 keta );


//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//-------------------------------------
///	プロセスデータ
//=====================================
const GFL_PROC_DATA	DEBUG_WifiBattleMatch_ProcData =
{ 
  WIFIBATTLEMATCH_DEBUG_PROC_Init,
  WIFIBATTLEMATCH_DEBUG_PROC_Main,
  WIFIBATTLEMATCH_DEBUG_PROC_Exit,
};

//=============================================================================
/**
 *          データ
 */
//=============================================================================
//-------------------------------------
///	キー名
//=====================================
static const char *sc_key_name[] =
{ 
  "しょうはい %d",
  "シングルかち %d",
  "シングルまけ %d",
  "シングルレート %d",
  "ダブルかち %d",
  "ダブルまけ %d",
  "ダブルレート %d",
  "トリプルかち %d",
  "トリプルまけ %d",
  "トリプルレート %d",
  "ローテかち %d",
  "ローテまけ %d",
  "ローテレート %d",
  "シュータかち %d",
  "シュータまけ %d",
  "シュータレート %d",
  "ふせいカウンタ %d",
  "ただしいレポート %d",
};
//-------------------------------------
///	スタット名
//=====================================
static const char *sc_stat_name[]  =
{ 
  "シングルかち %d",
  "シングルまけ %d",
  "シングルレート %d",
  "ダブルかち %d",
  "ダブルまけ %d",
  "ダブルレート %d",
  "トリプルかち %d",
  "トリプルまけ %d",
  "トリプルレート %d",
  "ローテかち %d",
  "ローテまけ %d",
  "ローテレート %d",
  "シュータかち %d",
  "シュータまけ %d",
  "シュータレート %d",
  "せつだんカウンタ %d",
  "ふせいカウンタ %d",
};

//=============================================================================
/**
 *          プロセス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグプロセス  初期化
 *
 *	@param	GFL_PROC *p_proc  プロセス
 *	@param	*p_seq            シーケンス
 *	@param	*p_param_adrs     引数
 *	@param	*p_wk_adrs        ワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_DEBUG_PROC_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  DEBUG_WIFIBATTLEMATCH_PARAM *p_param  = p_param_adrs;
  WIFIBATTLEMATCH_DEBUG_WORK  *p_wk;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIMATCH_DEBUG, 0x30000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_DEBUG_WORK), HEAPID_WIFIMATCH_DEBUG );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_DEBUG_WORK) );	

  //グラフィック設定
	p_wk->p_graphic	= WIFIBATTLEMATCH_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_WIFIMATCH_DEBUG );


	//読みこみ
	{	
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_MAIN_BG, PLT_FONT_M*0x20, 0, HEAPID_WIFIMATCH_DEBUG );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_SUB_BG, PLT_FONT_S*0x20, 0, HEAPID_WIFIMATCH_DEBUG );
	}

  { 
    WIFI_LIST *p_wifilist = GAMEDATA_GetWiFiList( p_param->p_gamedata );

    if( p_param->mode == DEBUG_WIFIBATTLEMATCH_MODE_ATLAS )
    { 
      ATLAS_Init( &p_wk->atlas, p_wifilist, p_param->p_save, HEAPID_WIFIMATCH_DEBUG );
    }
    else if( p_param->mode == DEBUG_WIFIBATTLEMATCH_MODE_SAKE )
    { 
      SAKE_Init( &p_wk->sake, p_wifilist, HEAPID_WIFIMATCH_DEBUG );
    }
  }


  //デバッグなのでフェードは簡便してもらいます
  GX_SetMasterBrightness( 0 );
	GXS_SetMasterBrightness( 0 );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグプロセス  破棄
 *
 *	@param	GFL_PROC *p_proc  プロセス
 *	@param	*p_seq            シーケンス
 *	@param	*p_param_adrs     引数
 *	@param	*p_wk_adrs        ワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_DEBUG_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  DEBUG_WIFIBATTLEMATCH_PARAM *p_param  = p_param_adrs;
  WIFIBATTLEMATCH_DEBUG_WORK  *p_wk = p_wk_adrs;

  //デバッグなのでフェードは簡便してもらいます
  GX_SetMasterBrightness( 16 );
	GXS_SetMasterBrightness( 16 );

  if( p_param->mode == DEBUG_WIFIBATTLEMATCH_MODE_ATLAS )
  { 
    ATLAS_Exit( &p_wk->atlas );
  }
  else if( p_param->mode == DEBUG_WIFIBATTLEMATCH_MODE_SAKE )
  { 
    SAKE_Exit( &p_wk->sake );
  }


  //グラフィック破棄
	WIFIBATTLEMATCH_GRAPHIC_Exit( p_wk->p_graphic );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIMATCH_DEBUG );
	
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグプロセス  メイン処理
 *
 *	@param	GFL_PROC *p_proc  プロセス
 *	@param	*p_seq            シーケンス
 *	@param	*p_param_adrs     引数
 *	@param	*p_wk_adrs        ワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_DEBUG_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  DEBUG_WIFIBATTLEMATCH_PARAM *p_param  = p_param_adrs;
  WIFIBATTLEMATCH_DEBUG_WORK  *p_wk = p_wk_adrs;
  BOOL ret;

  if( p_param->mode == DEBUG_WIFIBATTLEMATCH_MODE_ATLAS )
  { 
    ret = ATLAS_Main( &p_wk->atlas );
  }
  else if( p_param->mode == DEBUG_WIFIBATTLEMATCH_MODE_SAKE )
  { 
    ret = SAKE_Main( &p_wk->sake );
  }

  if( ret )
  { 
    return GFL_PROC_RES_FINISH;
  }

	return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINのサイズ取得
 *
 *	@param	u32 key キー
 *	@param	*p_wk   取得ワーク
 */
//-----------------------------------------------------------------------------
static void INPUTWIN_GetRange( u32 key, INPUTWIN_PARAM *p_wk )
{ 
  const u8 w  = 15;
  const u8 h  = 2;
  const u8 padding_w  = 1;
  const u8 padding_h  = 0;

  u16 x;

  x = key * (w + padding_w);

  p_wk->x =  (x) % 32;
  p_wk->y =  ((x) / 32) * (h + padding_h);

  p_wk->w = w;
  p_wk->h = h;

}

//=============================================================================
/**
 *    SAKE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  サケデバッグ  初期化
 *
 *	@param	DEBUG_SAKE_WORK *p_wk ワーク
 *	@param	*p_user_data  ユーザーデータ
 *	@param	heapID ヒープ
 */
//-----------------------------------------------------------------------------
static void SAKE_Init( DEBUG_SAKE_WORK *p_wk,WIFI_LIST *p_wifilist, HEAPID heapID )
{ 
  GFL_STD_MemClear( p_wk, sizeof(DEBUG_SAKE_WORK) );
  p_wk->heapID    = heapID;
	p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 255, heapID );
  p_wk->p_que     = PRINTSYS_QUE_Create( heapID );
  p_wk->p_text  = WBM_TEXT_Init( BG_FRAME_M_FONT, PLT_FONT_M, 0, 0, p_wk->p_que, p_wk->p_font, heapID );
  Sake_CreateDisplay( p_wk, heapID );
  { 
    DWCUserData *p_user_data  = WifiList_GetMyUserInfo( p_wifilist );
    p_wk->p_net = WIFIBATTLEMATCH_NET_Init( p_user_data, p_wifilist, heapID );
  }

  NUMINPUT_Init( &p_wk->numinput, BG_FRAME_S_NUM, 10, 10, 6, 2, PLT_FONT_S, p_wk->p_font, heapID );
  GFL_BG_SetVisible( BG_FRAME_S_NUM, FALSE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  サケデバッグ  破棄
 *
 *	@param	DEBUG_SAKE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SAKE_Exit( DEBUG_SAKE_WORK *p_wk )
{ 
  NUMINPUT_Exit( &p_wk->numinput );

  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
  Sake_DeleteDisplay( p_wk );
  WBM_TEXT_Main( p_wk->p_text );
	GFL_FONT_Delete( p_wk->p_font );
  GFL_STD_MemClear( p_wk, sizeof(DEBUG_SAKE_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  サケデバッグ  メイン処理
 *
 *	@param	DEBUG_SAKE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static BOOL SAKE_Main( DEBUG_SAKE_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,

    SEQ_START_NUMINPUT,
    SEQ_WAIT_NUMINPUT,
  
    SEQ_START_RECVDATA,
    SEQ_WAIT_RECVDATA,

    SEQ_START_SENDDATA,
    SEQ_WAIT_SENDDATA,

    SEQ_WAIT_DISCONNECT,
    SEQ_EXIT,
  };
  switch( p_wk->seq )
  { 
  case SEQ_INIT:
    { 
      const u16 str[] = L"Aで受信、Bで送信　タッチで値変更";
      WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
    }
    p_wk->seq = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    {
      int i;
      u32 x, y;
      if( GFL_UI_TP_GetPointTrg( &x, &y) )
      { 
        for( i = 0; i < DEBUG_BMPWIN_MAX; i++ )
        {
          if( p_wk->p_bmpwin[i] )
          { 
            if( BMPWINIsCollisionRxP( p_wk->p_bmpwin[i], x, y ) )
            {
              p_wk->select  = i;
              p_wk->seq     = SEQ_START_NUMINPUT;
              break;
            }
          }
        }
      }
    }

    //送信
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    { 
      p_wk->seq = SEQ_START_SENDDATA;
    }
    //受信
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    { 
      p_wk->seq = SEQ_START_RECVDATA;
    }
    break;

  case SEQ_START_NUMINPUT:
    GFL_BG_SetVisible( BG_FRAME_S_NUM, TRUE );
    { 
      NUMINPUT_Start( &p_wk->numinput, 4, 0, 9999, p_wk->data[ p_wk->select ] );
    }
    p_wk->seq = SEQ_WAIT_NUMINPUT;
    break;

  case SEQ_WAIT_NUMINPUT:
    if( NUMINPUT_Main( &p_wk->numinput ) )
    { 
      p_wk->data[ p_wk->select ] = NUMINPUT_GetNum( &p_wk->numinput );
      Sake_UpdateDisplay( p_wk );
      GFL_BG_SetVisible( BG_FRAME_S_NUM, FALSE );
      p_wk->seq = SEQ_MAIN;
    }
    break;

  case SEQ_START_RECVDATA:
    { 
      const u16 str[] = L"SAKEからデータを取得中";
      WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
    }
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->score );
    p_wk->seq = SEQ_WAIT_RECVDATA;
    break;

  case SEQ_WAIT_RECVDATA:
    { 
      DWCGdbError result;
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &result ) )
      { 
        { 
          const u16 str[] = L"データ取得完了";
          WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
        }

        p_wk->data[0] = p_wk->score.single_win;
        p_wk->data[1] = p_wk->score.single_lose;
        p_wk->data[2] = p_wk->score.single_rate;
        p_wk->data[3] = p_wk->score.double_win;
        p_wk->data[4] = p_wk->score.double_lose;
        p_wk->data[5] = p_wk->score.double_rate;
        p_wk->data[6] = p_wk->score.rot_win;
        p_wk->data[7] = p_wk->score.rot_lose;
        p_wk->data[8] = p_wk->score.rot_rate;
        p_wk->data[9] = p_wk->score.triple_win;
        p_wk->data[10] = p_wk->score.triple_lose;
        p_wk->data[11] = p_wk->score.triple_rate;
        p_wk->data[12] = p_wk->score.shooter_win;
        p_wk->data[13] = p_wk->score.shooter_lose;
        p_wk->data[14] = p_wk->score.shooter_rate;
        p_wk->data[15] = p_wk->score.disconnect;
        p_wk->data[16] = p_wk->score.cheat;

        p_wk->seq = SEQ_MAIN;
      }
    }
    break;

  case SEQ_START_SENDDATA:
    { 
        p_wk->score.single_win  =p_wk->data[0];
        p_wk->score.single_lose =p_wk->data[1];
        p_wk->score.single_rate =p_wk->data[2];
        p_wk->score.double_win  =p_wk->data[3];
        p_wk->score.double_lose =p_wk->data[4];
        p_wk->score.double_rate =p_wk->data[5];
        p_wk->score.rot_win     =p_wk->data[6];
        p_wk->score.rot_lose    =p_wk->data[7];
        p_wk->score.rot_rate    =p_wk->data[8];
        p_wk->score.triple_win  =p_wk->data[9];
        p_wk->score.triple_lose =p_wk->data[10];
        p_wk->score.triple_rate =p_wk->data[11];
        p_wk->score.shooter_win =p_wk->data[12];
        p_wk->score.shooter_lose  =p_wk->data[13];
        p_wk->score.shooter_rate  =p_wk->data[14];
        p_wk->score.disconnect  =p_wk->data[15];
        p_wk->score.cheat       =p_wk->data[16];
    }
    { 
      const u16 str[] = L"データ送信中";
      WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
    }
    WIFIBATTLEMATCH_GDB_StartWrite( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->score );
    p_wk->seq = SEQ_WAIT_SENDDATA;
    break;

  case SEQ_WAIT_SENDDATA:
    { 
      DWCGdbError result;
      if( WIFIBATTLEMATCH_GDB_ProcessWrite( p_wk->p_net, &result )  )
      { 
        { 
          const u16 str[] = L"データ送信完了";
          WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
        }
        p_wk->seq = SEQ_MAIN;
      }
    }
    break;

  case SEQ_WAIT_DISCONNECT:
    if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, FALSE ) )
    { 
      p_wk->seq = SEQ_EXIT;
    }
    break;

  case SEQ_EXIT:
    return TRUE;
  }

  WIFIBATTLEMATCH_NET_Main( p_wk->p_net );
  WBM_TEXT_Main( p_wk->p_text );
  PRINTSYS_QUE_Main( p_wk->p_que );

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  SAKE用画面作成
 *
 *	@param	DEBUG_SAKE_WORK *p_wk ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void Sake_CreateDisplay( DEBUG_SAKE_WORK *p_wk, HEAPID heapID )
{ 
  int i;
  INPUTWIN_PARAM  param;

  for( i = 0; i < NELEMS(sc_stat_name); i++ )
  { 

    INPUTWIN_GetRange( i, &param );

    PRINT_GetStrWithNumber( p_wk->p_strbuf, sc_stat_name[i], p_wk->data[i] );

    p_wk->p_bmpwin[i]  = GFL_BMPWIN_Create( BG_FRAME_S_FONT, 
        param.x, param.y, param.w, param.h, 
        PLT_FONT_S, GFL_BMP_CHRAREA_GET_B );


    PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0, 0, p_wk->p_strbuf, p_wk->p_font );

    GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  SAKE用画面破棄
 *
 *	@param	DEBUG_SAKE_WORK *p_wk ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void Sake_DeleteDisplay( DEBUG_SAKE_WORK *p_wk )
{ 
  int i;
  for( i = 0; i < NELEMS(sc_stat_name); i++ )
  {
    if( p_wk->p_bmpwin[i] )
    { 
      GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
      p_wk->p_bmpwin[i] = NULL;
    }
  }

  GFL_BG_ClearScreen( BG_FRAME_S_FONT );
}
//----------------------------------------------------------------------------
/**
 *	@brief  SAKE用画面アップデート
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Sake_UpdateDisplay( DEBUG_SAKE_WORK *p_wk )
{ 
  int i;
  INPUTWIN_PARAM  param;

  for( i = 0; i < NELEMS(sc_stat_name); i++ )
  { 

    INPUTWIN_GetRange( i, &param );

    PRINT_GetStrWithNumber( p_wk->p_strbuf, sc_stat_name[i], p_wk->data[i] );

    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0 );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0, 0, p_wk->p_strbuf, p_wk->p_font );

    GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );
  }
}

//=============================================================================
/**
 *    ATLAS
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  アトラスデバッグ  初期化
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk    ワーク
 *	@param	heapID                    ヒープID
 */
//-----------------------------------------------------------------------------
static void ATLAS_Init( DEBUG_ATLAS_WORK *p_wk, WIFI_LIST *p_wifilist, SAVE_CONTROL_WORK* p_save, HEAPID heapID )
{ 
  GFL_STD_MemClear( p_wk, sizeof(DEBUG_ATLAS_WORK) );
  p_wk->heapID    = heapID;
  p_wk->p_save    = p_save;
	p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 255, heapID );
  p_wk->p_que     = PRINTSYS_QUE_Create( heapID );
  p_wk->p_text  = WBM_TEXT_Init( BG_FRAME_M_FONT, PLT_FONT_M, 0, 0, p_wk->p_que, p_wk->p_font, heapID );
  Atlas_CreateReportDisplay( p_wk, heapID );

  { 
    DWCUserData *p_user_data  = WifiList_GetMyUserInfo( p_wifilist );
    p_wk->p_net = WIFIBATTLEMATCH_NET_Init( p_user_data, p_wifilist, heapID );
  }

  NUMINPUT_Init( &p_wk->numinput, BG_FRAME_S_NUM, 10, 10, 6, 2, PLT_FONT_S, p_wk->p_font, heapID );
  GFL_BG_SetVisible( BG_FRAME_S_NUM, FALSE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アドラスデバッグ  破棄
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void ATLAS_Exit( DEBUG_ATLAS_WORK *p_wk )
{ 
  NUMINPUT_Exit( &p_wk->numinput );

  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
  Atlas_DeleteRecvDisplay( p_wk );
  WBM_TEXT_Main( p_wk->p_text );
  PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );
  GFL_STD_MemClear( p_wk, sizeof(DEBUG_ATLAS_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アトラスデバッグ  メイン処理
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static BOOL ATLAS_Main( DEBUG_ATLAS_WORK *p_wk )
{ 
  enum
  { 
    SEQ_START_MATCH,
    SEQ_WAIT_MATCH,

    SEQ_START_INIT,
    SEQ_WAIT_INIT,

    SEQ_SENDMAIN,
    SEQ_RECVMAIN,

    SEQ_START_NUMINPUT,
    SEQ_WAIT_NUMINPUT,
  
    SEQ_START_RECVDATA,
    SEQ_WAIT_RECVDATA,

    SEQ_START_SENDDATA,
    SEQ_WAIT_SENDDATA,

    SEQ_WAIT_DISCONNECT,
    SEQ_EXIT,
  };

  switch( p_wk->seq )
  { 
  case SEQ_START_MATCH:
    { 
      const u16 str[] = L"デバッグ相手とマッチングして下さい。";
      WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
    }
    WIFIBATTLEMATCH_NET_StartMatchMakeDebug( p_wk->p_net );
    p_wk->seq = SEQ_WAIT_MATCH;
    break;

  case SEQ_WAIT_MATCH:
    if( WIFIBATTLEMATCH_NET_WaitMatchMake( p_wk->p_net ) )
    { 
      p_wk->seq = SEQ_START_INIT;
    }
    break;

  case SEQ_START_INIT:
    { 
      const u16 str[] = L"初期化を行っています。";
      WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
    }
    WIFIBATTLEMATCH_NET_StartInitialize( p_wk->p_net );
    p_wk->seq = SEQ_WAIT_INIT;
    break;

  case SEQ_WAIT_INIT:
    { 
      DWCGdbError error;
      if( WIFIBATTLEMATCH_NET_WaitInitialize( p_wk->p_net, p_wk->p_save, &error )  )
      { 
        { 
          const u16 str[] = L"送信自分。Aで送信、Xでモード、タッチで値";
          WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
        }
        p_wk->seq = SEQ_SENDMAIN;
      }
    }
    break;

  case SEQ_SENDMAIN:
    {
      int i;
      u32 x, y;
      if( GFL_UI_TP_GetPointTrg( &x, &y) )
      { 
        for( i = 0; i < DEBUG_BMPWIN_MAX; i++ )
        {
          if( p_wk->p_bmpwin[i] )
          { 
            if( BMPWINIsCollisionRxP( p_wk->p_bmpwin[i], x, y ) )
            {
              if( i == 17 )
              { 
                p_wk->is_auth ^= 1;
                Atlas_UpdateReportDisplay( p_wk );
              }
              else
              { 
                p_wk->select  = i;
                p_wk->pre_seq = SEQ_SENDMAIN;
                p_wk->seq     = SEQ_START_NUMINPUT;
              }
              break;
            }
          }
        }
      }
    }

    //送信
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    { 
      p_wk->seq = SEQ_START_SENDDATA;
    }

    //情報切り替え
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    { 
      p_wk->is_you  ^=  1;
      { 
        if( !p_wk->is_you )
        { 
          const u16 str[] = L"送信自分。Aで送信、Xでモード、タッチで値";
          WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
        }
        else
        { 
          const u16 str[] = L"送信相手。Aで送信、Xでモード、タッチで値";
          WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
        }
        Atlas_UpdateReportDisplay( p_wk );
      }
    }

    //切り替え
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    { 
      { 
        const u16 str[] = L"受信モード。Aで受信、Xでモード切替";
        WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
      }
      Atlas_DeleteReportDisplay( p_wk );
      Atlas_CreateRecvDisplay( p_wk, p_wk->heapID );
      p_wk->seq = SEQ_RECVMAIN;
    }
    break;

  case SEQ_RECVMAIN:
    //受信
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    { 
      p_wk->seq = SEQ_START_RECVDATA;
    }

    //切り替え
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    { 
      { 
        const u16 str[] = L"送信モード。Aで送信、Xでモード切替、タッチで値変更";
        WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
      }
      Atlas_DeleteRecvDisplay( p_wk );
      Atlas_CreateReportDisplay( p_wk, p_wk->heapID );
      p_wk->seq = SEQ_SENDMAIN;
    }
    break;

  case SEQ_START_NUMINPUT:
    GFL_BG_SetVisible( BG_FRAME_S_NUM, TRUE );
    { 
      u32 data;
      u32 max;
      if( p_wk->pre_seq == SEQ_RECVMAIN )
      { 
        data  = p_wk->recv_data[ p_wk->select ];
      }
      else if( p_wk->pre_seq == SEQ_SENDMAIN )
      { 
        data  = p_wk->send_data[p_wk->is_you][ p_wk->select ];
      }
      NUMINPUT_Start( &p_wk->numinput, 4, 0, 1, data );
    }
    p_wk->seq = SEQ_WAIT_NUMINPUT;
    break;

  case SEQ_WAIT_NUMINPUT:
    if( NUMINPUT_Main( &p_wk->numinput ) )
    { 
      if( p_wk->pre_seq == SEQ_RECVMAIN )
      { 
        p_wk->recv_data[ p_wk->select ] = NUMINPUT_GetNum( &p_wk->numinput );
        Atlas_UpdateRecvDisplay( p_wk );
      }
      else if( p_wk->pre_seq == SEQ_SENDMAIN )
      { 
        p_wk->send_data[p_wk->is_you][ p_wk->select ] = NUMINPUT_GetNum( &p_wk->numinput );
        Atlas_UpdateReportDisplay( p_wk );
      }
      GFL_BG_SetVisible( BG_FRAME_S_NUM, FALSE );
      p_wk->seq = p_wk->pre_seq;
    }
    break;

  case SEQ_START_RECVDATA:
    { 
      const u16 str[] = L"SAKEからデータを取得中";
      WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
    }
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->score );
    p_wk->seq = SEQ_WAIT_RECVDATA;
    break;

  case SEQ_WAIT_RECVDATA:
    { 
      DWCGdbError result;
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &result ) )
      { 
        { 
          const u16 str[] = L"データ取得完了";
          WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
        }

        p_wk->recv_data[0] = p_wk->score.single_win;
        p_wk->recv_data[1] = p_wk->score.single_lose;
        p_wk->recv_data[2] = p_wk->score.single_rate;
        p_wk->recv_data[3] = p_wk->score.double_win;
        p_wk->recv_data[4] = p_wk->score.double_lose;
        p_wk->recv_data[5] = p_wk->score.double_rate;
        p_wk->recv_data[6] = p_wk->score.rot_win;
        p_wk->recv_data[7] = p_wk->score.rot_lose;
        p_wk->recv_data[8] = p_wk->score.rot_rate;
        p_wk->recv_data[9] = p_wk->score.triple_win;
        p_wk->recv_data[10] = p_wk->score.triple_lose;
        p_wk->recv_data[11] = p_wk->score.triple_rate;
        p_wk->recv_data[12] = p_wk->score.shooter_win;
        p_wk->recv_data[13] = p_wk->score.shooter_lose;
        p_wk->recv_data[14] = p_wk->score.shooter_rate;
        p_wk->recv_data[15] = p_wk->score.disconnect;
        p_wk->recv_data[16] = p_wk->score.cheat;

        Atlas_UpdateRecvDisplay( p_wk );

        p_wk->seq = SEQ_RECVMAIN;
      }
    }
    break;

  case SEQ_START_SENDDATA:
    { 
      p_wk->report.my_result      = p_wk->send_data[0][0];
      p_wk->report.my_single_win  = p_wk->send_data[0][1];
      p_wk->report.my_single_lose = p_wk->send_data[0][2];
      p_wk->report.my_single_rate = p_wk->send_data[0][3];
      p_wk->report.my_double_win  = p_wk->send_data[0][4];
      p_wk->report.my_double_lose = p_wk->send_data[0][5];
      p_wk->report.my_double_rate = p_wk->send_data[0][6];
      p_wk->report.my_triple_win  = p_wk->send_data[0][7];
      p_wk->report.my_triple_lose = p_wk->send_data[0][8];
      p_wk->report.my_triple_rate = p_wk->send_data[0][9];
      p_wk->report.my_rot_win     = p_wk->send_data[0][10];
      p_wk->report.my_rot_lose    = p_wk->send_data[0][11];
      p_wk->report.my_rot_rate    = p_wk->send_data[0][12];
      p_wk->report.my_shooter_win  = p_wk->send_data[0][13];
      p_wk->report.my_shooter_lose = p_wk->send_data[0][14];
      p_wk->report.my_shooter_rate = p_wk->send_data[0][15];
      p_wk->report.my_cheat       = p_wk->send_data[0][16];
      p_wk->report.you_result      = p_wk->send_data[1][0];
      p_wk->report.you_single_win  = p_wk->send_data[1][1];
      p_wk->report.you_single_lose = p_wk->send_data[1][2];
      p_wk->report.you_single_rate = p_wk->send_data[1][3];
      p_wk->report.you_double_win  = p_wk->send_data[1][4];
      p_wk->report.you_double_lose = p_wk->send_data[1][5];
      p_wk->report.you_double_rate = p_wk->send_data[1][6];
      p_wk->report.you_triple_win  = p_wk->send_data[1][7];
      p_wk->report.you_triple_lose = p_wk->send_data[1][8];
      p_wk->report.you_triple_rate = p_wk->send_data[1][9];
      p_wk->report.you_rot_win     = p_wk->send_data[1][10];
      p_wk->report.you_rot_lose    = p_wk->send_data[1][11];
      p_wk->report.you_rot_rate    = p_wk->send_data[1][12];
      p_wk->report.you_shooter_win  = p_wk->send_data[1][13];
      p_wk->report.you_shooter_lose = p_wk->send_data[1][14];
      p_wk->report.you_shooter_rate = p_wk->send_data[1][15];
      p_wk->report.you_cheat       = p_wk->send_data[1][16];
    }
    { 
      const u16 str[] = L"レポート送信中";
      WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
    }
    WIFIBATTLEMATCH_SC_StartDebug( p_wk->p_net, &p_wk->report, p_wk->is_auth );
    p_wk->seq = SEQ_WAIT_SENDDATA;
    break;

  case SEQ_WAIT_SENDDATA:
    { 
      DWCScResult result;
      if( WIFIBATTLEMATCH_SC_Process( p_wk->p_net, &result )  )
      { 
        { 
          const u16 str[] = L"レポート送信完了";
          WBM_TEXT_PrintDebug( p_wk->p_text, str, NELEMS(str), p_wk->p_font );
        }
        p_wk->seq = SEQ_SENDMAIN;
      }
    }
    break;

  case SEQ_WAIT_DISCONNECT:
    if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, FALSE ) )
    { 
      p_wk->seq = SEQ_EXIT;
    }
    break;

  case SEQ_EXIT:
    return TRUE;
  }

  WIFIBATTLEMATCH_NET_Main( p_wk->p_net );
  WBM_TEXT_Main( p_wk->p_text );
  PRINTSYS_QUE_Main( p_wk->p_que );

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アトラス用画面表示作成
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk  ワーク
 *	@param	heapID                  ヒープID
 */
//-----------------------------------------------------------------------------
static void Atlas_CreateRecvDisplay( DEBUG_ATLAS_WORK *p_wk, HEAPID heapID )
{ 
  int i;
  INPUTWIN_PARAM  param;

  for( i = 0; i < NELEMS(sc_stat_name); i++ )
  { 

    INPUTWIN_GetRange( i, &param );

    PRINT_GetStrWithNumber( p_wk->p_strbuf, sc_stat_name[i], p_wk->recv_data[i] );

    p_wk->p_bmpwin[i]  = GFL_BMPWIN_Create( BG_FRAME_S_FONT, 
        param.x, param.y, param.w, param.h, 
        PLT_FONT_S, GFL_BMP_CHRAREA_GET_B );


    PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0, 0, p_wk->p_strbuf, p_wk->p_font );

    GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アトラス用画面表示破棄
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void Atlas_DeleteRecvDisplay( DEBUG_ATLAS_WORK *p_wk )
{ 
  int i;
  for( i = 0; i < NELEMS(sc_stat_name); i++ )
  {
    if( p_wk->p_bmpwin[i] )
    { 
      GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
      p_wk->p_bmpwin[i] = NULL;
    }
  }

  GFL_BG_ClearScreen( BG_FRAME_S_FONT );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アトラス用画面アップデート
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk  ワーク
 *	@param  
 *
 */
//-----------------------------------------------------------------------------
static void Atlas_UpdateRecvDisplay( DEBUG_ATLAS_WORK *p_wk )
{ 
  int i;

  for( i = 0; i < NELEMS(sc_stat_name); i++ )
  { 
    
    PRINT_GetStrWithNumber( p_wk->p_strbuf, sc_stat_name[i], p_wk->recv_data[i] );

    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0 );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0, 0, p_wk->p_strbuf, p_wk->p_font );

    GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アトラス用レポート画面作成
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk  ワーク
 *	@param	heapID                  ヒープID
 */
//-----------------------------------------------------------------------------
static void Atlas_CreateReportDisplay( DEBUG_ATLAS_WORK *p_wk, HEAPID heapID )
{ 
  int i;
  INPUTWIN_PARAM  param;

  for( i = 0; i < NELEMS(sc_key_name)-1; i++ )
  { 
    INPUTWIN_GetRange( i, &param );

    PRINT_GetStrWithNumber( p_wk->p_strbuf, sc_key_name[i], p_wk->send_data[p_wk->is_you][i] );

    p_wk->p_bmpwin[i]  = GFL_BMPWIN_Create( BG_FRAME_S_FONT, 
        param.x, param.y, param.w, param.h, 
        PLT_FONT_S, GFL_BMP_CHRAREA_GET_B );


    PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0, 0, p_wk->p_strbuf, p_wk->p_font );

    GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );
  }
  
  INPUTWIN_GetRange( i, &param );
  PRINT_GetStrWithNumber( p_wk->p_strbuf, sc_key_name[i], p_wk->is_auth );
  p_wk->p_bmpwin[i]  = GFL_BMPWIN_Create( BG_FRAME_S_FONT, 
        param.x, param.y, param.w, param.h, 
        PLT_FONT_S, GFL_BMP_CHRAREA_GET_B );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0, 0, p_wk->p_strbuf, p_wk->p_font );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

}
//----------------------------------------------------------------------------
/**
 *	@brief  アトラス用レポート画面作成
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk  ワーク
 *	@param	heapID                  ヒープID
 */
//-----------------------------------------------------------------------------
static void Atlas_DeleteReportDisplay( DEBUG_ATLAS_WORK *p_wk )
{ 
  int i;
  for( i = 0; i < NELEMS(sc_key_name); i++ )
  {
    if( p_wk->p_bmpwin[i] )
    { 
      GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
      p_wk->p_bmpwin[i] = NULL;
    }
  }
  GFL_BG_ClearScreen( BG_FRAME_S_FONT );
}
//----------------------------------------------------------------------------
/**
 *	@brief  アトラス用レポート画面作成
 *
 *	@param	DEBUG_ATLAS_WORK *p_wk  ワーク
 *	@param	heapID                  ヒープID
 */
//-----------------------------------------------------------------------------
static void Atlas_UpdateReportDisplay( DEBUG_ATLAS_WORK *p_wk )
{ 
  int i;

  for( i = 0; i < NELEMS(sc_key_name) - 1; i++ )
  { 
    PRINT_GetStrWithNumber( p_wk->p_strbuf, sc_key_name[i], p_wk->send_data[p_wk->is_you][i] );

    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0 );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0, 0, p_wk->p_strbuf, p_wk->p_font );

    GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin[i] );
  }

  PRINT_GetStrWithNumber( p_wk->p_strbuf, sc_key_name[i], p_wk->is_auth );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0 );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0, 0, p_wk->p_strbuf, p_wk->p_font );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

}

//=============================================================================
/**
 *  NUMINPUT
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  数値入力  初期化
 *
 *	@param	NUMINPUT_WORK *p_wk ワーク  
 *	@param	frm                 作成フレーム
 *	@param	x                   X
 *	@param	y                   Y
 *	@param	w                   幅
 *	@param	h                   高さ
 *	@param	plt                 パレット
 *	@param	*p_font             フォント
 *	@param	heapID              ヒープID
 */
//-----------------------------------------------------------------------------
static void NUMINPUT_Init( NUMINPUT_WORK *p_wk, u8 frm, u8 x, u8 y , u8 w, u8 h, u8 plt, GFL_FONT *p_font, HEAPID heapID )
{ 
  GFL_STD_MemClear( p_wk, sizeof(NUMINPUT_WORK) );
  p_wk->p_font  = p_font;
  p_wk->p_bmpwin  = GFL_BMPWIN_Create( frm, x, y, w, h, plt, GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 255, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  数値入力  破棄
 *
 *	@param	NUMINPUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void NUMINPUT_Exit( NUMINPUT_WORK *p_wk )
{ 
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );
  GFL_STD_MemClear( p_wk, sizeof(NUMINPUT_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  数値入力  開始
 *
 *	@param	NUMINPUT_WORK *p_wk ワーク
 *	@param	keta                桁
 *	@param	min                 最小
 *	@param	max                 最大
 *	@param  now                 初期値
 */
//-----------------------------------------------------------------------------
static void NUMINPUT_Start( NUMINPUT_WORK *p_wk, u8 keta, s32 min, s32 max, s32 now )
{ 
  p_wk->keta  = keta;
  p_wk->min   = min;
  p_wk->max   = max;
  p_wk->now   = now;

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 15 );
  NumInput_Print( p_wk );
  GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *	@brief  数値入力  終了
 *
 *	@param	NUMINPUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static BOOL NUMINPUT_Main( NUMINPUT_WORK *p_wk )
{ 
  BOOL  is_update = FALSE;

  //左右で変更する数字を切り替える
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
  { 
    p_wk->active++;
    p_wk->active  %= p_wk->keta;

    is_update = TRUE;
  }
  else if( GFL_UI_KEY_GetTrg( ) & PAD_KEY_RIGHT )
  { 
    if( p_wk->active == 0 )
    { 
      p_wk->active  = p_wk->keta  - 1 ;
    }
    else
    { 
      p_wk->active--;
    }
    is_update = TRUE;
  }

  //上下で数字を切り替える
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
  { 
    p_wk->now = NumInput_AddKeta( p_wk->now, p_wk->active, 1 );
    p_wk->now = MATH_CLAMP( p_wk->now, p_wk->min, p_wk->max );
    is_update = TRUE;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  { 
    p_wk->now = NumInput_AddKeta( p_wk->now, p_wk->active, -1 );
    p_wk->now = MATH_CLAMP( p_wk->now, p_wk->min, p_wk->max );
    is_update = TRUE;
  }

  //決定
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  { 
    return TRUE;
  }

  //描画
  if( is_update )
  { 
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 15 );
    NumInput_Print( p_wk );
    GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin );
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  入力した数値取得
 *
 *	@param	const NUMINPUT_WORK *cp_wk ワーク
 *
 *	@return 数値
 */
//-----------------------------------------------------------------------------
static u16  NUMINPUT_GetNum( const NUMINPUT_WORK *cp_wk )
{ 
  return cp_wk->now;
}

//----------------------------------------------------------------------------
/**
 *	@brief  数値入力  表示
 *
 *	@param	NUMINPUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void NumInput_Print( NUMINPUT_WORK *p_wk )
{ 

  int i;
  PRINTSYS_LSB color;
  u8 fig;

  //描画
  for( i = 0; i < p_wk->keta; i++ )
  { 
    fig =  GetFig( p_wk->now, p_wk->keta - i - 1 );

    PRINT_GetStrWithNumber( p_wk->p_strbuf, "%d", fig );

    if( p_wk->active  == p_wk->keta - i - 1 )
    { 
      color = PRINTSYS_LSB_Make( 3,4,15 );
    }
    else
    { 
      color = PRINTSYS_LSB_Make( 1,2,15 );
    }
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), i * 12, 0, p_wk->p_strbuf, p_wk->p_font, color );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  任意の桁に１加減算する
 *
 *	@param	u32 now 現在の値
 *	@param	keta    桁
 *	@param	dir     +ならば加算 -ならば減算
 *
 *	@return 修正後の数値
 */
//-----------------------------------------------------------------------------
static u32 NumInput_AddKeta( u32 now, u32 keta, s8 dir )
{
  u32 add   = 1;
  while( keta-- )
  { 
    add *= 10;
  }
    
  now +=  dir*add;
  return now;
}

//----------------------------------------------------------------------------
/**
 *	@brief  文字に数値をいれる
 *
 *	@param	STRBUF *p_dst   受け取り
 *	@param	u16 *cp_src     文字列
 */
//-----------------------------------------------------------------------------
static void PRINT_GetStrWithNumber( STRBUF *p_dst, const char *cp_src, ... )
{ 
  char str[128];
  STRCODE str_code[128];
  u16 strlen;
  va_list vlist;

  //数値を入れる
  va_start(vlist, cp_src);
  STD_TVSNPrintf( str, 128, cp_src, vlist );
  va_end(vlist);

  //STRBUF用に変換
  DEB_STR_CONV_SjisToStrcode( str, str_code, 128 );

  strlen  = GFL_STD_StrLen(str)*2;
  str_code[strlen] = GFL_STR_GetEOMCode();

  //STRBUFに転送
  GFL_STR_SetStringCode( p_dst, str_code);
}

//----------------------------------------------------------------------------
/**
 *	@brief  BMPWIN当たり判定
 *
 *	@param	const GFL_BMPWIN *cp_bmpwin
 *	@param	x 点X
 *	@param	y 点Y
 *
 *	@return TRUEで衝突  FALSEで衝突していない
 */
//-----------------------------------------------------------------------------
static BOOL BMPWINIsCollisionRxP( const GFL_BMPWIN *cp_bmpwin, u32 x, u32 y )
{
	s16	left, right, top, bottom;

	left	  = ( GFL_BMPWIN_GetPosX( cp_bmpwin ) )*8;
	right	  = ( GFL_BMPWIN_GetPosX( cp_bmpwin ) + GFL_BMPWIN_GetSizeX( cp_bmpwin ) )*8;
	top		  = ( GFL_BMPWIN_GetPosY( cp_bmpwin ) )*8;
	bottom  = ( GFL_BMPWIN_GetPosY( cp_bmpwin ) + GFL_BMPWIN_GetSizeY( cp_bmpwin ) )*8;

	return ((s32)( x - left) <= (s32)(right - left))
		&	((s32)( y - top) <= (s32)(bottom - top));
}
//----------------------------------------------------------------------------
/**
 *	@brief  桁番目の数値を取得
 *
 *	@param	u32 now 数字
 *	@param	keta    桁
 *
 *	@return keta番目の数値
 */
//-----------------------------------------------------------------------------
static u8 GetFig( u32 now, u8 keta )
{ 
  static const u32 sc_keta_tbl[]  =
  { 
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
  };

  GF_ASSERT( keta < NELEMS(sc_keta_tbl) );
  return (now % sc_keta_tbl[ keta + 1 ]) / sc_keta_tbl[ keta ] ;
}

