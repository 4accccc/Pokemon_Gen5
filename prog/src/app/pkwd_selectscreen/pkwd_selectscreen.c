//=============================================================================
/**
 *
 *	@file		pkwd_select_screen.c
 *	@brief  ポケウッド上映選択画面
 *	@author	mori
 *	@data		2011.07.19
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"
#include "print/global_msg.h"

#include "print/wordset.h"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "pkwd_selectscreen_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//マップモデルのnsbtx
#include "fieldmap/fldmmdl_mdlres.naix"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//MCSS
#include "system/mcss.h"
#include "system/mcss_tool.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//どうぐアイコン
#include "item/item.h"
#include "item_icon.naix"

//ポケモンBG,OBJ読みこみ
#include "system/poke2dgra.h"

//アプリ共通素材
#include "app/app_menu_common.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/pkwd_selectscreen.h"

//パレット点滅処理
#include "system/blink_palanm.h"

//メッセージデータ
#include "msg/msg_pw_movietitle.h"

//wipe
#include "system/wipe.h"

#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"

#include "message.naix"
#include "pkwd_announce_gra.naix"	// アーカイブ
#include "b_scenario_gra.naix"
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース

#include "savedata/battle_rec.h"
#include "savedata/pokewood_rec.h"


//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define PKWD_SELECT_SCREEN_TOUCHBAR
#define PKWD_SELECT_SCREEN_TASKMENU

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  PKWD_SELECT_SCREEN_HEAP_SIZE = 0x60000,  ///< ヒープサイズ
};


//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_TALK_M  = GFL_BG_FRAME0_M,
	BG_FRAME_BAR_M   = GFL_BG_FRAME1_M,
	BG_FRAME_POKE_M  = GFL_BG_FRAME2_M,
	BG_FRAME_BACK_M  = GFL_BG_FRAME3_M,
	BG_FRAME_BACK_S  = GFL_BG_FRAME2_S,
	BG_FRAME_FRAME_S = GFL_BG_FRAME1_S,
  BG_FRAME_TEXT_S  = GFL_BG_FRAME0_S, 
};
//-------------------------------------
///	パレット
//=====================================
enum
{	
	//メインBG
	PLTID_BG_BACK_M				= 0,
	PLTID_BG_POKE_M				= 1,
	PLTID_BG_TASKMENU_M		= 11,
	PLTID_BG_TOUCHBAR_M		= 13,
	PLTID_BG_INFOWIN_M		= 15,
	//サブBG
	PLTID_BG_BACK_S				=	0,

	//メインOBJ
	PLTID_OBJ_TOUCHBAR_M	= 0,    // 3本使用
	PLTID_OBJ_TYPEICON_M	= 3,    // 3本使用
  PLTID_OBJ_OAM_MAPMODEL_M = 6, // 1本使用
  PLTID_OBJ_POKEICON_M = 7,     // 3本使用
  PLTID_OBJ_POKEITEM_M = 10,    // 1本使用
  PLTID_OBJ_ITEMICON_M = 11,
  PLTID_OBJ_POKE_M = 12,
  PLTID_OBJ_BALLICON_M = 13,    // 1本使用
	PLTID_OBJ_TOWNMAP_M	= 14,		
	//サブOBJ
  PLTID_OBJ_PMS_DRAW = 0,       // 5本使用
  PLTID_OBJ_SUB_MARK = 0,       // 3本分確保(0-2)

  PKWD_TITLE_STR_MAX = 26*2*2,        // タイトル文字列の文字数ＭＡＸ


  PKWD_TITLE_LIST_NUM = 4,            // 映画タイトルを一度に表示できる最大数
  LOAD_SAVEDATA_MAX   = POKEWOOD_SAVEDATA_MAX,  // =8 映画タイトルをセーブできる最大数

  PKWD_POKEMON_MAX    = 6,      // 手持ち表示する最大数
};

// リスト用OBJ定義
enum{
  LIST_OBJ_MAX  = 4,
  
  LIST_OBJ_ID0 = 0,
  LIST_OBJ_ID1,
  LIST_OBJ_ID2,
  LIST_OBJ_ID3,
};


// SE定義
enum{
  SE_PKWDSELECT_DECIDE = SEQ_SE_DECIDE1,
  SE_PKWDSELECT_MOVE   = SEQ_SE_SELECT1,

};

// BMPWIN定義
enum{
  PKWD_BMP_TITLE01=0,
  PKWD_BMP_TITLE02,
  PKWD_BMP_TITLE03,
  PKWD_BMP_TITLE04,
  PKWD_BMP_SUBTITLE_U,
  PKWD_BMP_SUBTITLE_D,
  PKWD_BMP_SUB_DATE,        // 公開日(追加)
  PKWD_BMP_MSGWIN,
  PKWD_BMP_INFORMATION,

  PKWD_SELECT_SCREEN_BMPWIN_MAX,  /// BMPWINの数
  PKWD_SELECT_SCREEN_BMPWIN_FIX_NUM = 6+1, /// 常時確保しておくBMPWINの数
                                           /// 6,7のBMPWINは使用時確保する
};

// メニュー種類定義
enum{
  PKWD_MENU_WATCH=0,
  PKWD_MENU_DELETE,
  PKWD_MENU_YESNO,
};

// SubOBJ定義
enum{
  SUB_OBJ_MARK = 0,      // 評価マーク
  //
  SUB_OBJ_MAX,
};

// シーケンス
enum{
  SUB_SEQ_GOOD     = 10, // GOOD
  SUB_SEQ_BAD      = 11, // BAD
  SUB_SEQ_STRANGE  = 12, // STRANGE
};

//=============================================================================
/**
 *							データ定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
///	BG管理ワーク
//==============================================================
typedef struct 
{
	int dummy;
} PKWD_SELECT_SCREEN_BG_WORK;

/// 録画セーブデータが取り出した情報を保持するワーク
typedef struct{
  int         slot_no;      ///< 録画データスロットNO
  LOAD_RESULT load_result;  ///< 録画データロード結果
  int         scenario_no;  ///< シナリオNO
  int         profit;       ///< 収益
  int         success;      ///< 評価
  u32         date;         ///< 日付
  u32         time;         ///< 時間
  u16         my_monsno[PKWD_POKEMON_MAX];  ///< 手持ちポケモンの種族NO
  WORDSET     *wordset;     ///< 文字列展開バッファ
  int         watched;      ///< 既に鑑賞済みか？
}PW_SVDATA;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

	PKWD_SELECT_SCREEN_BG_WORK	wk_bg;
	PKWD_SELECT_SCREEN_PARAM    *param;

  STRBUF *strbuf;

	//描画設定
	PKWD_SELECT_SCREEN_GRAPHIC_WORK	*graphic;

#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバー
	TOUCHBAR_WORK							*touchbar;
	//以下カスタムボタン使用する場合のサンプルリソース
	u32												ncg_btn;
	u32												ncl_btn;
	u32												nce_btn;
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


	//フォント
	GFL_FONT									*font;
  WORDSET                   *UpWordset;

	//プリントキュー
	PRINT_QUE									*print_que;
  PRINT_STREAM              *print_stream;
	GFL_MSGDATA								*msg;
  GFL_TCBLSYS               *pMsgTcblSys;

  // サブシーケンス管理
  int seq;
  int subseq;

  // VBLANK 処理
  GFL_TCB    *vblank_task;  ///< VblankFunc登録タスク
  int        bgLogoRequest; ///< VBlank内BG転送リクエストフラグ

#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//タスクメニュー
	APP_TASKMENU_RES					*menu_res;
	APP_TASKMENU_WORK					*menu;
#endif //PKWD_SELECT_SCREEN_TASKMENU





  // リストOBJ用定義
  u32                       ncg_pkwdObj;
  u32                       ncl_pkwdObj;
  u32                       nce_pkwdObj;
	GFL_CLWK									*clwk_pkwdObj[LIST_OBJ_MAX];
  
  // SubOBJ用定義
  u32                       ncg_SubObj;
  u32                       ncl_SubObj;
  u32                       nce_SubObj;
  GFL_CLWK                  *clwk_SubObj[SUB_OBJ_MAX];

	//プリントユーティリティ
	PRINT_UTIL								print_util[PKWD_SELECT_SCREEN_BMPWIN_MAX];

  // カーソル処理
  s16 nowpos, oldpos;
  int isTouch;
  BLINKPALANM_WORK *blinkWork[4];
  int page;

  // セーブデータ分必要な情報
  PW_SVDATA   svdata[LOAD_SAVEDATA_MAX];       ///< 外部データから取得した情報実体
  PW_SVDATA   *valid_list[LOAD_SAVEDATA_MAX];  ///< 実際に表示する有効リスト
                                               ///< 外部セーブデータなので個別に破損が起き得るため
                                               ///< ロードに成功したデータを詰めて表示する

} PKWD_SELECT_SCREEN_MAIN_WORK;

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			///< タイプ(0xffの場合は終了判定)
	u8				arcid;		///< id
	u8				offset;		///< オフセット
	u8				size;			///< サイズ(*0x20)
}PKWD_COUNT_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;	///< frame(0xffの場合は終了判定)
	u8				arcid;			///< id
	u8				offset;			///< オフセット
	u8				pad;
}PKWD_COUNT_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;	///< frame(0xffの場合は終了判定)
	u8				arcid;			///< id
	u8				offset;			///< オフセット
	u8				poff;			  ///< パレットのオフセット
}PKWD_COUNT_BGSCN_DATA;

#define PLTID_MAIN_BG_TITLE ( 0 ) // 下のテーブル読み込みのための定義

// BG定義とサブタイトル定義を取り込み
#include "../pkwd_announce/pkwd_movie_title.cdat"
#include "../pkwd_book/pokewood_category.cdat"

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT PkwdSelectScreenProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdSelectScreenProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdSelectScreenProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void PkwdSelectScreen_BG_LoadResource( PKWD_SELECT_SCREEN_BG_WORK* wk, HEAPID heapID );


#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
//-------------------------------------
///	タッチバー
//=====================================
static TOUCHBAR_WORK * PkwdSelectScreen_TOUCHBAR_InitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void PkwdSelectScreen_TOUCHBAR_ExitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU
//-------------------------------------
///	リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * PkwdSelectScreen_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID, int type );
static void PkwdSelectScreen_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void PkwdSelectScreen_TASKMENU_Main( APP_TASKMENU_WORK *menu );
#endif //PKWD_SELECT_SCREEN_TASKMENU


static void PkwdSelectScreen_LOGO_u_LoadResource( PKWD_SELECT_SCREEN_BG_WORK *wk, int scenario_no, int index );
static void PkwdSelectScreen_ListObj_loadResource( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_ListObj_UnloadResource( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_ListObj_Add( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_ListObj_Del( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_ListObj_Update( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int page );
//
// 上画面で追加
static void PkwdSelectScreen_SubObj_loadResource(   PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_SubObj_UnloadResource( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_SubObj_Add( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_SubObj_Del( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_SubObj_Visible( PKWD_SELECT_SCREEN_MAIN_WORK *wk, const s32 num, const BOOL bflg );
static void PkwdSelectScreen_SubObj_ChgSeq( PKWD_SELECT_SCREEN_MAIN_WORK *wk, const s32 num, const s32 seq );
//

static BOOL PrintTool_MainFunc( PKWD_SELECT_SCREEN_MAIN_WORK * wk );
static void  _load_pokewood_rec_data( PKWD_SELECT_SCREEN_MAIN_WORK *wk, PKWD_SELECT_SCREEN_PARAM *param );
static  int _touch_input( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static  int _key_input( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void _draw_titlelist( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void _draw_movie_subtitle( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int index );
static void SubTitle_Func( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PrintInit( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PrintExit( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void _message_print( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int msg_id );
static void _blink_palanime_exit( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void _blink_palanime_init( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static  int _get_real_pos( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int pos );
static void _touchbar_button_refresh( PKWD_SELECT_SCREEN_MAIN_WORK *wk, BOOL flag );
static void VBlankFunc( GFL_TCB *tcb, void * work );

static BOOL SubSeqFunc( PKWD_SELECT_SCREEN_MAIN_WORK *wk );

static BOOL SubSeq_FadeinStart( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_FadeinWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Start( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Main( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_DecideWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_MsgWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_WarningMsg( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Select_Watch( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Select_WatchWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Yesno_Delete1( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Yesno_Delete1Wait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Yesno_Delete2( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Yesno_Delete2Wait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_FadeOut( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_FadeOutWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_PutInformation( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_PutInformationWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );

static void PrintTool_AddBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk );
static void PrintTool_DelBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk );
static void PrintTool_ScreenTrans( PKWD_SELECT_SCREEN_MAIN_WORK * wk, int index );
static void PrintTool_PrintHP( PKWD_SELECT_SCREEN_MAIN_WORK * wk );


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA PkwdSelectScreenProcData = 
{
	PkwdSelectScreenProc_Init,
	PkwdSelectScreenProc_Main,
	PkwdSelectScreenProc_Exit,
};
//=============================================================================
/**
 *								PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					PKWD_SELECT_SCREEN_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdSelectScreenProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_SELECT_SCREEN_MAIN_WORK *wk;
	PKWD_SELECT_SCREEN_PARAM *param;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//引数取得
	param	    = pwk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PKWD_SELSCR, PKWD_SELECT_SCREEN_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(PKWD_SELECT_SCREEN_MAIN_WORK), HEAPID_PKWD_SELSCR );
  GFL_STD_MemClear( wk, sizeof(PKWD_SELECT_SCREEN_MAIN_WORK) );

  // 初期化
  wk->heapID = HEAPID_PKWD_SELSCR;
  wk->param  = param;
	
	//描画設定初期化
	wk->graphic	= PKWD_SELECT_SCREEN_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

  // 文字列描画システム初期化
  PrintInit(wk);

	//BGリソース読み込み
	PkwdSelectScreen_BG_LoadResource( &wk->wk_bg, wk->heapID );


#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= PKWD_SELECT_SCREEN_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= PkwdSelectScreen_TOUCHBAR_InitEx( wk, clunit, wk->heapID );
    TOUCHBAR_SetBGPriorityAll( wk->touchbar, 1 );
	}
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_TALK_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
#endif //PKWD_SELECT_SCREEN_TASKMENU


  // obj初期化
  PkwdSelectScreen_ListObj_loadResource( wk );
  PkwdSelectScreen_SubObj_loadResource( wk );

  // 外部セーブデータ確認・読み込み
  _load_pokewood_rec_data( wk, param );

  PrintTool_AddBmpWin( wk );			// BMPWIN作成

  // リストOBJ表示
  PkwdSelectScreen_ListObj_Add( wk );
  PkwdSelectScreen_ListObj_Update( wk, wk->page );
  // 上画面SubObj表示
  PkwdSelectScreen_SubObj_Add( wk );

  // 左右キーを表示させるかどうか設定する
  _touchbar_button_refresh( wk, TRUE );

  // 文字列描画
  _draw_titlelist( wk );
  
  // 点滅処理初期化
  _blink_palanime_init(wk);
  
  wk->nowpos = 0;

  // タッチorキー検出
  if(GFL_UI_CheckTouchOrKey()==GFL_APP_END_TOUCH){
    wk->nowpos  = -1;
    wk->isTouch = TRUE;
    // 評価マーク非表示
    PkwdSelectScreen_SubObj_Visible( wk, SUB_OBJ_MARK, FALSE );
  }else{
    _draw_movie_subtitle( wk, _get_real_pos( wk, wk->nowpos) );
    wk->isTouch = FALSE;
  }

  // ワーク初期化
  wk->seq = 0;
  
  // VBlankFunc登録
  wk->vblank_task = GFUser_VIntr_CreateTCB( VBlankFunc, wk, 1 );

  // 16dot上にずらしセンタリングする
  GFL_BG_SetScroll( GFL_BG_FRAME2_S, GFL_BG_SCROLL_Y_SET, 16 );

  GFL_NET_ReloadIconTopOrBottom( FALSE, HEAPID_PKWD_SELSCR);
  
  return GFL_PROC_RES_FINISH;
}

#define BLINK_NORMAL_COLOR    ( 0x4000 )



//----------------------------------------------------------------------------------
/// ページNoとカーソルの位置を足した実際のインデックスを返す
//----------------------------------------------------------------------------------
static int _get_real_pos( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int pos )
{
  return pos + wk->page*PKWD_TITLE_LIST_NUM;
}

//----------------------------------------------------------------------------------
/**
 * @brief	プレート点滅処理初期化
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void _blink_palanime_init( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{

  int i;
  for(i=0;i<PKWD_TITLE_LIST_NUM;i++){
    // セーブされているシナリオNOからカテゴリーを取得
    int category,index=i+wk->page*PKWD_TITLE_LIST_NUM;
    if(wk->valid_list[index]->load_result){
      category = PokewoodCategoryData[wk->valid_list[index]->scenario_no];
    }else{
      category = 4;
    }
    wk->blinkWork[i] = BLINKPALANM_Create( i*16, 16, BG_FRAME_BACK_M, HEAPID_PKWD_SELSCR );
    BLINKPALANM_SetPalBufferArc( wk->blinkWork[i], ARCID_PKWD_ANNOUNCE, 
                                NARC_pkwd_announce_gra_pkwd_title_select_d_NCLR, 
                                16*category, (5+category)*16 );
    BLINKPALANM_SetAnimeCount( wk->blinkWork[i], BLINK_NORMAL_COLOR );
    BLINKPALANM_Main( wk->blinkWork[i] );
  }


}

//----------------------------------------------------------------------------------
/**
 * @brief	プレート点滅処理解放
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void _blink_palanime_exit( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;
  for(i=0;i<4;i++){
    BLINKPALANM_Exit( wk->blinkWork[i] );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					PKWD_SELECT_SCREEN_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdSelectScreenProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	PKWD_SELECT_SCREEN_MAIN_WORK* wk = mywk;
  int i;

  // vblankFunc解放
	GFL_TCB_DeleteTask( wk->vblank_task );

  // BMP解放
  PrintTool_DelBmpWin( wk );

  // プレート点滅解放
  _blink_palanime_exit( wk );
  
  // 選択結果の表示
  OS_Printf("screen select result=%d\n", *wk->param->result );

  // obj破棄
  PkwdSelectScreen_ListObj_Del( wk );
  PkwdSelectScreen_ListObj_UnloadResource( wk );
  //
  PkwdSelectScreen_SubObj_Del( wk );
  PkwdSelectScreen_SubObj_UnloadResource( wk );

#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//TASKMENUシステム＆リソース破棄
	APP_TASKMENU_RES_Delete( wk->menu_res );	
#endif //PKWD_SELECT_SCREEN_TASKMENU


#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバー
	PkwdSelectScreen_TOUCHBAR_ExitEx( wk );
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


  GFL_STR_DeleteBuffer( wk->strbuf );

  // 文字列描画システム解放
  PrintExit( wk );

	//描画設定破棄
	PKWD_SELECT_SCREEN_GRAPHIC_Exit( wk->graphic );

	//PROC用メモリ解放
  {
    HEAPID heapID = wk->heapID;
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( heapID );
  }

	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}


//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					PKWD_SELECT_SCREEN_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdSelectScreenProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	PKWD_SELECT_SCREEN_MAIN_WORK* wk = mywk;

  // フロー制御
  if(SubSeqFunc(wk)){
    return GFL_PROC_RES_FINISH;
  }

#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバーメイン処理
	PkwdSelectScreen_TOUCHBAR_Main( wk->touchbar );
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//タスクメニューメイン処理
  if(wk->menu!=NULL){
    PkwdSelectScreen_TASKMENU_Main( wk->menu );
  }
#endif //PKWD_SELECT_SCREEN_TASKMENU

  // メッセージウインドウ用タスク
  if(wk->pMsgTcblSys){
    GFL_TCBL_Main( wk->pMsgTcblSys );
  }

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );

  PrintTool_MainFunc( wk );

  // パレット点滅メイン
  if(wk->nowpos>=0){
    BLINKPALANM_Main( wk->blinkWork[wk->nowpos] );
  }
  
	//2D描画
	PKWD_SELECT_SCREEN_GRAPHIC_2D_Draw( wk->graphic );

  

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief VBlank割り込み関数
 *
 * @param   tcb   
 * @param   work    
 */
//----------------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB *tcb, void * work )
{
  PKWD_SELECT_SCREEN_MAIN_WORK* wk = (PKWD_SELECT_SCREEN_MAIN_WORK*)work;

  //上画面タイトルロゴ転送
  if( wk->bgLogoRequest){
    int pos = _get_real_pos(wk, wk->nowpos);
    if(pos<0){
      PkwdSelectScreen_LOGO_u_LoadResource( &wk->wk_bg, -1, -1 );
    
    }else{
      OS_Printf("scenario_no=%d, pos=%d, nowpos=%d\n", wk->valid_list[pos]->scenario_no, pos, wk->nowpos);
      PkwdSelectScreen_LOGO_u_LoadResource( &wk->wk_bg, 
                                          wk->valid_list[pos]->scenario_no, 
                                          wk->nowpos );
    }
    wk->bgLogoRequest = 0;
  }
}


//=============================================================================
/**
 *								static関数
 */
//=============================================================================

  enum
  {
    RES_ARC = ARCID_PKWD_ANNOUNCE,

    RES_PAL_01S_SIZE = 0x20*15,
    RES_PAL_02S_SIZE = 0x20,
    RES_PAL_M_SIZE = 0x20*11,

    // ロゴの周りの枠
    RES_PAL_01S_ID = NARC_pkwd_announce_gra_pkwd_title_select_u_NCLR,
    RES_CHR_01S_ID = NARC_pkwd_announce_gra_pkwd_title_select_u_NCGR,
    RES_SCR_01S_ID = NARC_pkwd_announce_gra_pkwd_title_select_u01_NSCR,

    // ロゴ（デフォルト）
    RES_PAL_02S_ID = NARC_pkwd_announce_gra_pkwd_title01_logo_NCLR,
    RES_CHR_02S_ID = NARC_pkwd_announce_gra_pkwd_title01_logo_NCGR,
    RES_SCR_02S_ID = NARC_pkwd_announce_gra_pkwd_title01_NSCR,

    // 下画面
    RES_PAL_M_ID = NARC_pkwd_announce_gra_pkwd_title_select_d_NCLR,
    RES_CHR_M_ID = NARC_pkwd_announce_gra_pkwd_title_select_d_NCGR,
    RES_SCR_M_ID = NARC_pkwd_announce_gra_pkwd_title_select_d01_NSCR,

  };
//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	PKWD_SELECT_SCREEN_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_BG_LoadResource( PKWD_SELECT_SCREEN_BG_WORK* wk, HEAPID heapID )
{

	ARCHANDLE	*handle;

	handle	= GFL_ARC_OpenDataHandle( RES_ARC, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_M_ID, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, RES_PAL_M_SIZE, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_01S_ID, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_01S_SIZE, heapID );
	
  //	----- SUB -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_01S_ID, BG_FRAME_FRAME_S, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_01S_ID,	BG_FRAME_FRAME_S, 0, 0, 0, heapID );	

	//	----- MAIN -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_M_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_M_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );		



	GFL_ARC_CloseDataHandle( handle );
}


//----------------------------------------------------------------------------------
/**
 * @brief	ムービーロゴのみ転送
 *
 * @param   wk		      PKWD_SELECT_SCREEN_MAIN_WORK
 * @param   scenario_no	シナリオNO（-1のときはシナリオ指定なし）
 * @param   index       カーソル位置
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_LOGO_u_LoadResource( PKWD_SELECT_SCREEN_BG_WORK *wk, int scenario_no, int index )
{
  int pal, cgx, scr;
  ARCHANDLE	*handle;

  // ロゴBG面を輝度アップで白に
  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG2, 16 );

	handle	= GFL_ARC_OpenDataHandle( ARCID_PKWD_ANNOUNCE, HEAPID_PKWD_SELSCR );
  // タッチ待ちでカーソルがどこにも無いときは空白ロゴを転送
  if(index==-1 || scenario_no==-1){
    pal = NARC_pkwd_announce_gra_pkwd_title99_logo_NCLR;
    cgx = NARC_pkwd_announce_gra_pkwd_title99_logo_NCGR;
    scr = NARC_pkwd_announce_gra_pkwd_title99_NSCR;
    // 背景クリアの転送の時はスクリーンを先に転送してしまう
    GFL_ARCHDL_UTIL_TransVramPalette( handle, pal, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_02S_SIZE, HEAPID_PKWD_SELSCR );
    GFL_ARCHDL_UTIL_TransVramScreen(      handle, scr,	BG_FRAME_BACK_S,  0, 0, 0, HEAPID_PKWD_SELSCR );	
    GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, cgx, BG_FRAME_BACK_S,  0, 0, 0, HEAPID_PKWD_SELSCR );
  }else{
    pal = pwc_bgpal_01_tbl[scenario_logo_tbl[scenario_no]].arcid;
    cgx = pwc_bgchr_01_tbl[scenario_logo_tbl[scenario_no]].arcid;
    scr = pwc_bgscn_01_tbl[scenario_logo_tbl[scenario_no]].arcid;
    GFL_ARCHDL_UTIL_TransVramPalette( handle, pal, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_02S_SIZE, HEAPID_PKWD_SELSCR );
    GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, cgx, BG_FRAME_BACK_S,  0, 0, 0, HEAPID_PKWD_SELSCR );
    GFL_ARCHDL_UTIL_TransVramScreen(      handle, scr,	BG_FRAME_BACK_S,  0, 0, 0, HEAPID_PKWD_SELSCR );	
  }
  OS_Printf("scenario_no=%d\n", scenario_no);



	GFL_ARC_CloseDataHandle( handle );

  // ロゴＢＧ面の輝度戻す
  G2S_BlendNone();
}



#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR初期化	+ カスタムアイコン版
 *
 * @param  wk										ワーク
 *	@param	GFL_CLUNIT *clunit	CLUNIT
 *	@param	heapID							ヒープID
 *
 *	@return	TOUCHBAR_WORK
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK * PkwdSelectScreen_TOUCHBAR_InitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
{	

	//リソース読みこみ
	//サンプルとしてタウンマップの拡大縮小アイコンをカスタムボタンに登録する
	{	
		ARCHANDLE *handle;
		//ハンドル
		handle	= GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );
		//リソース読みこみ
		wk->ncg_btn	= GFL_CLGRP_CGR_Register( handle,
					NARC_townmap_gra_tmap_obj_d_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );
		wk->ncl_btn	= GFL_CLGRP_PLTT_RegisterEx( handle,
				NARC_townmap_gra_tmap_bg_d_NCLR, CLSYS_DRAW_MAIN, PLTID_OBJ_TOWNMAP_M*0x20,
				2, 2, heapID );	
		wk->nce_btn	= GFL_CLGRP_CELLANIM_Register( handle,
					NARC_townmap_gra_tmap_obj_d_NCER,
				 	NARC_townmap_gra_tmap_obj_d_NANR, heapID );
	
		GFL_ARC_CloseDataHandle( handle ) ;
	}

	//タッチバーの設定
	{	
	//アイコンの設定
	//数分作る
	TOUCHBAR_SETUP	touchbar_setup;
	TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
	{	
		{	
			TOUCHBAR_ICON_CUR_L,
			{	TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_Y },
		},
		{	
			TOUCHBAR_ICON_CUR_R,
			{	TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_Y },
		},
		{	
			TOUCHBAR_ICON_RETURN,
			{	TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
		},
	};

	//設定構造体
	//さきほどの窓情報＋リソース情報をいれる
	GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

	touchbar_setup.p_item		= touchbar_icon_tbl;				//上の窓情報
	touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
	touchbar_setup.p_unit		= clunit;										//OBJ読み込みのためのCLUNIT
	touchbar_setup.bar_frm	= BG_FRAME_BAR_M;						//BG読み込みのためのBG面
	touchbar_setup.bg_plt		= PLTID_BG_TOUCHBAR_M;			//BGﾊﾟﾚｯﾄ
	touchbar_setup.obj_plt	= PLTID_OBJ_TOUCHBAR_M;			//OBJﾊﾟﾚｯﾄ
	touchbar_setup.mapping	= APP_COMMON_MAPPING_128K;	//マッピングモード

	return TOUCHBAR_Init( &touchbar_setup, heapID );
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR破棄	カスタムボタン版
 *
 *	@param	TOUCHBAR_WORK	*touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_TOUCHBAR_ExitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{	
	//タッチバー破棄
	TOUCHBAR_Exit( wk->touchbar );

	//リソース破棄
	{	
		GFL_CLGRP_PLTT_Release( wk->ncl_btn );
		GFL_CLGRP_CGR_Release( wk->ncg_btn );
		GFL_CLGRP_CELLANIM_Release( wk->nce_btn );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBARメイン処理
 *
 *	@param	TOUCHBAR_WORK	*touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU


/// ３択用プレートの幅
#define PKWD_TASKMENU_PLATE_1_WIDTH  ( 12 )
#define PKWD_TASKMENU_PLATE_2_WIDTH  ( 14 )

typedef struct{
  const int *menu_list;
  const int menu_num;
  const int menu_w;
  const int plate[4];
}PKWD_MENU_INIT_DATA;

static int menu_list_watch[]  = { pw_select_watch, pw_select_info, pw_select_return };
static int menu_list_delete[] = { pw_select_end,   pw_select_info, pw_select_return };
static int menu_list_yesno[]  = { pw_select_yes,   pw_select_no };

const static PKWD_MENU_INIT_DATA menu_table[]={
  { // PKWD_MENU_WATCH
    menu_list_watch,  3, PKWD_TASKMENU_PLATE_1_WIDTH,
    { APP_TASKMENU_WIN_TYPE_NORMAL, APP_TASKMENU_WIN_TYPE_NORMAL,APP_TASKMENU_WIN_TYPE_RETURN },
  },
  { // PKWD_MENU_DELETE
    menu_list_delete, 3, PKWD_TASKMENU_PLATE_2_WIDTH ,    
    { APP_TASKMENU_WIN_TYPE_NORMAL, APP_TASKMENU_WIN_TYPE_NORMAL,APP_TASKMENU_WIN_TYPE_RETURN },
  },
  { // PKWD_MENU_YESNO
    menu_list_yesno,  2, APP_TASKMENU_PLATE_WIDTH_YN_WIN,
    { APP_TASKMENU_WIN_TYPE_NORMAL, APP_TASKMENU_WIN_TYPE_NORMAL,APP_TASKMENU_WIN_TYPE_NORMAL },
  },
};
//----------------------------------------------------------------------------------
/**
 * @brief	TASKMENUの初期化
 *
 * @param   menu_res		リソースデータ
 * @param   msg		      メッセージデータハンドル
 * @param   heapID		  heapID
 * @param   type		    メニューの種類
 *
 * @retval  APP_TASKMENU_WORK *	確保したメニュー管理ワーク
 */
//----------------------------------------------------------------------------------
static APP_TASKMENU_WORK * PkwdSelectScreen_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID, int type )
{	
	int i;
	APP_TASKMENU_INITWORK	init;
	APP_TASKMENU_ITEMWORK	item[3];
	APP_TASKMENU_WORK			*menu;	
  const PKWD_MENU_INIT_DATA   *menu_dat = &menu_table[type];

	//窓の設定
	for( i = 0; i < menu_dat->menu_num; i++ )
	{	
		item[i].str	      = GFL_MSG_CreateString( msg, menu_dat->menu_list[i] );	//文字列
		item[i].msgColor	= APP_TASKMENU_ITEM_MSGCOLOR; //文字色
		item[i].type			= menu_dat->plate[i];         //窓の種類
	}

	//初期化
	init.heapId		= heapID;
	init.itemNum	= menu_dat->menu_num;
	init.itemWork = item;
	init.posType	= ATPT_RIGHT_DOWN;
	init.charPosX	= 32;
	init.charPosY = 23;
	init.w				= menu_dat->menu_w;
	init.h				= APP_TASKMENU_PLATE_HEIGHT;

	menu	= APP_TASKMENU_OpenMenu( &init, menu_res );

	//文字列解放
	for( i = 0; i < menu_dat->menu_num; i++ )
	{	
		GFL_STR_DeleteBuffer( item[i].str );
	}

	return menu;

}
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの破棄
 *
 *	@param	APP_TASKMENU_WORK *menu	ワーク
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_CloseMenu( menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUのメイン処理
 *
 *	@param	APP_TASKMENU_WORK *menu	ワーク
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_UpdateMenu( menu );
}
#endif //PKWD_SELECT_SCREEN_TASKMENU








//----------------------------------------------------------------------------------
/**
 * @brief	下画面リストOBJ用リソース登録
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_ListObj_loadResource( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
	ARCHANDLE						*handle;

	//ハンドル
  handle	= GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, HEAPID_PKWD_SELSCR );

	//リソース読みこみ
	wk->ncg_pkwdObj	= GFL_CLGRP_CGR_Register( handle, NARC_b_scenario_gra_pkwd_list_obj_NCGR, 0, CLSYS_DRAW_MAIN, HEAPID_PKWD_SELSCR );
	wk->ncl_pkwdObj	= GFL_CLGRP_PLTT_RegisterEx( handle, NARC_b_scenario_gra_pkwd_list_obj_NCLR, CLSYS_DEFREND_MAIN,
	                                             5*32, 0, 1, HEAPID_PKWD_SELSCR );
//	GFL_CLGRP_PLTT_Register( handle, NARC_b_scenario_gra_pkwd_list_obj_NCLR, CLSYS_DEFREND_MAIN, 0, HEAPID_PKWD_SELSCR );
	wk->nce_pkwdObj	= GFL_CLGRP_CELLANIM_Register( handle, NARC_b_scenario_gra_pkwd_list_obj_NCER, 
	                                                       NARC_b_scenario_gra_pkwd_list_obj_NANR, HEAPID_PKWD_SELSCR );
	GFL_ARC_CloseDataHandle( handle );

}

//----------------------------------------------------------------------------------
/**
 * @brief	下画面リストOBJ用リソース解放
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_ListObj_UnloadResource( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
	//リソース破棄
	GFL_CLGRP_PLTT_Release(     wk->ncl_pkwdObj );
	GFL_CLGRP_CGR_Release(      wk->ncg_pkwdObj );
	GFL_CLGRP_CELLANIM_Release( wk->nce_pkwdObj );
  
}

static const u8 listobj_pos_tbl[][2]={
  { 14,  20 },
  { 14,  60 },
  { 14, 100 },
  { 14, 140 },
};

//----------------------------------------------------------------------------------
/**
 * @brief	リストOBJ表示登録
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_ListObj_Add( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  GFL_CLWK_DATA	cldata;
  int i;

  // 「NEW！」OBJを登録
  GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
  for(i=0;i<LIST_OBJ_MAX;i++){
    cldata.pos_x	= listobj_pos_tbl[i][0];
    cldata.pos_y	= listobj_pos_tbl[i][1];
    cldata.anmseq = 1;
    cldata.bgpri  = 2;
    wk->clwk_pkwdObj[i]	= GFL_CLACT_WK_Create( PKWD_SELECT_SCREEN_GRAPHIC_GetClunit( wk->graphic ), 
                                           wk->ncg_pkwdObj,
                                           wk->ncl_pkwdObj,
                                           wk->nce_pkwdObj,
                                           &cldata, 
                                           CLSYS_DEFREND_MAIN, HEAPID_PKWD_SELSCR );
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk_pkwdObj[i], TRUE );
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief	リストOBJ表示更新
 *
 * @param   wk		PKWD_SELECT_SCREEN_MAIN_WORK
 * @param   page	現在の表示ページ
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_ListObj_Update( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int page )
{
  int i;
  
    // セーブデータ読み込ている＆まだ上映していないなら「NEW!」表示
  for(i=0;i<LIST_OBJ_MAX;i++){
    int index = i + page*PKWD_TITLE_LIST_NUM;
    if(POKEWOOD_SAVEDATA_GetMovieShowed( wk->param->pokewood_sv, index )==0){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk_pkwdObj[i], wk->valid_list[index]->load_result );
    }else{
      GFL_CLACT_WK_SetDrawEnable( wk->clwk_pkwdObj[i], FALSE );
    }
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief	リストOBJ解放
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_ListObj_Del( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;
  for(i=0;i<LIST_OBJ_MAX;i++){
  	GFL_CLACT_WK_Remove( wk->clwk_pkwdObj[i] );
  }
}

//=====================================================================================================
//----------------------------------------------------------------------------------
/**
 * @brief	上画面SubOBJ用リソース登録
 *
 * @param   PKWD_SELECT_SCREEN_MAIN_WORK	*wk
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_SubObj_loadResource( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  ARCHANDLE						*handle;
  
  //ハンドル
  handle	= GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, HEAPID_PKWD_SELSCR );
  //リソース読みこみ
  wk->ncg_SubObj	= GFL_CLGRP_CGR_Register( handle, NARC_b_scenario_gra_pkwd_list_obj_NCGR, 0, CLSYS_DRAW_SUB, HEAPID_PKWD_SELSCR );
  // 評価マークは2番にセットされているので3本分で十分のはず
  wk->ncl_SubObj	= GFL_CLGRP_PLTT_RegisterEx( handle, NARC_b_scenario_gra_pkwd_list_obj_NCLR, CLSYS_DEFREND_SUB,
                                                 0, 0, 3, HEAPID_PKWD_SELSCR );
  wk->nce_SubObj	= GFL_CLGRP_CELLANIM_Register( handle, NARC_b_scenario_gra_pkwd_list_obj_NCER,
                                                   NARC_b_scenario_gra_pkwd_list_obj_NANR, HEAPID_PKWD_SELSCR );
  GFL_ARC_CloseDataHandle( handle );
}

//----------------------------------------------------------------------------------
/**
 * @brief	上画面SubOBJ用リソース解放
 *
 * @param   PKWD_SELECT_SCREEN_MAIN_WORK	*wk
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_SubObj_UnloadResource( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  //リソース破棄
  GFL_CLGRP_PLTT_Release(     wk->ncl_SubObj );
  GFL_CLGRP_CGR_Release(      wk->ncg_SubObj );
  GFL_CLGRP_CELLANIM_Release( wk->nce_SubObj );
}

//----------------------------------------------------------------------------------
/**
 * @brief	上画面SubOBJ表示登録
 *
 * @param   PKWD_SELECT_SCREEN_MAIN_WORK	*wk
 */
//----------------------------------------------------------------------------------
static const u8 subobj_pos_tbl[][2]={
  { 224,  144 },	// マーク
};

static void PkwdSelectScreen_SubObj_Add( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  GFL_CLWK_DATA	cldata;
  int i;
  
  // SubOBJを登録
  GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
  
  for(i=0;i<SUB_OBJ_MAX;i++){
    cldata.pos_x	= subobj_pos_tbl[i][0];
    cldata.pos_y	= subobj_pos_tbl[i][1];
    cldata.anmseq = SUB_SEQ_GOOD;
    cldata.bgpri  = 1;
    wk->clwk_SubObj[i]	= GFL_CLACT_WK_Create( PKWD_SELECT_SCREEN_GRAPHIC_GetClunit( wk->graphic ), 
                                           wk->ncg_SubObj,
                                           wk->ncl_SubObj,
                                           wk->nce_SubObj,
                                           &cldata, 
                                           CLSYS_DEFREND_SUB, HEAPID_PKWD_SELSCR );
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk_SubObj[i], TRUE );
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief	上画面SubOBJ解放
 *
 * @param   PKWD_SELECT_SCREEN_MAIN_WORK	*wk
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_SubObj_Del( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;
  
  for(i=0;i<SUB_OBJ_MAX;i++){
    GFL_CLACT_WK_Remove( wk->clwk_SubObj[i] );
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief	上画面SubOBJ表示更新
 *
 * @param   PKWD_SELECT_SCREEN_MAIN_WORK	*wk
 * @param   const s32						番号
 * @param   const BOOL						(TRUE:表示、FALSE:非表示)
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_SubObj_Visible( PKWD_SELECT_SCREEN_MAIN_WORK *wk, const s32 num, const BOOL bflg )
{
  GFL_CLACT_WK_SetDrawEnable( wk->clwk_SubObj[num], bflg );
}

//----------------------------------------------------------------------------------
/**
 * @brief	上画面SubOBJシーケンス切り替え
 *
 * @param   PKWD_SELECT_SCREEN_MAIN_WORK	*wk
 * @param   const s32						番号
 * @param   const s32						シーケンス番号
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_SubObj_ChgSeq( PKWD_SELECT_SCREEN_MAIN_WORK *wk, const s32 num, const s32 seq )
{
  // 切り替え
  GFL_CLACT_WK_SetAnmSeq( wk->clwk_SubObj[num], seq );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk_SubObj[num], TRUE );
}

//=====================================================================================================



//=============================================================================
/**
 *	PRINT_TOOL
 */
//=============================================================================
/*
		・「HP ??/??」を表示するサンプルです
		・BMPWINのサイズは 20 x 2 です
		・現在のHP = 618, 最大HP = 999 とします
		・サンプルメインが FALSE を返すと終了です
*/


// サンプルメイン
static BOOL PrintTool_MainFunc( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
  {
    int i;
    for(i=0;i<PKWD_SELECT_SCREEN_BMPWIN_MAX;i++){
      PRINT_UTIL_Trans( &wk->print_util[i], wk->print_que );
    }
  }
	return TRUE;
}

#define TITLE_WIN_W     ( 24 )  /// 下画面タイトル文字列の幅
#define TITLE_WIN_H     (  5 )  /// 
#define SUBTITLE_WIN1_W ( 26 )  /// 上画面サブタイトル文字列の幅
#define SUBTITLE_WIN1_H (  3 )  /// 
#define SUBTITLE_WIN2_W ( 26 )  /// 上画面サブタイトル文字列の幅２（使ってる？）
#define SUBTITLE_WIN2_H (  5 )  /// 
#define MSGWIN_W        ( 28 )  /// 下画面メッセージウインドウの幅
#define MSGWIN_H        (  4 )  /// 
#define INFORMATION_W   ( 28 )  /// 下画面情報ウインドウの幅
#define INFORMATION_H   ( 16 )  /// 
#define TITLE_PAL       ( 13 )  /// 上画面タイトルロゴ表示用のBGパレット位置
#define SUBTITLE_PAL    ( 14 )  /// 上画面サブタイトル文字列表示用のパレット位置
#define MSGWINFRAME_PAL ( 14 )  /// 下画面メッセージ枠用のパレット位置
#define MSGWIN_PAL      ( 15 )  /// メッセージウインドウのパレット位置
#define MSGWINFRAME_CGX ( 32 )  /// 

typedef struct {
  u16 frame;
  u16 pal;
  u8  x, y;
  u8  w, h;
}BMPWIN_DAT;


static const BMPWIN_DAT bmpdat[]={
  
  { // 下画面タイトルリスト１
    GFL_BG_FRAME2_M, TITLE_PAL,    5, 0, TITLE_WIN_W, TITLE_WIN_H,
  },
  { // 下タイトルリスト２
    GFL_BG_FRAME2_M, TITLE_PAL,    5, 5, TITLE_WIN_W, TITLE_WIN_H,
  },
  { // 下タイトルリスト３
    GFL_BG_FRAME2_M, TITLE_PAL,    5, 10, TITLE_WIN_W, TITLE_WIN_H,
  },
  { // 下タイトルリスト４
    GFL_BG_FRAME2_M, TITLE_PAL,    5, 15, TITLE_WIN_W, TITLE_WIN_H,
  },
  { // 上画面サブタイトル１
    GFL_BG_FRAME0_S, SUBTITLE_PAL, 3,  0, SUBTITLE_WIN1_W, SUBTITLE_WIN1_H,
  },
  { // 上画面サブタイトル２
    GFL_BG_FRAME0_S, SUBTITLE_PAL, 3, 16, SUBTITLE_WIN2_W, SUBTITLE_WIN2_H,
  },
  { // 上画面公開日(追加)
    GFL_BG_FRAME0_S, SUBTITLE_PAL, 2, 22, 28, 2,
  },
  { // メッセージウインドウ
    GFL_BG_FRAME0_M, MSGWIN_PAL,   2, 1,  MSGWIN_W, MSGWIN_H,
  },
  { // 情報ウインドウ
    GFL_BG_FRAME0_M, MSGWIN_PAL,   2, 2,  INFORMATION_W, INFORMATION_H,
  },
};

// BMPWIN作成
static void PrintTool_AddBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
  int i;
  
  for(i=0;i<PKWD_SELECT_SCREEN_BMPWIN_FIX_NUM;i++){
    const BMPWIN_DAT *bmp = &bmpdat[i];
    wk->print_util[i].win = GFL_BMPWIN_Create(
                              bmp->frame,     // ＢＧフレーム
                              bmp->x, bmp->y, // 表示座標
                              bmp->w, bmp->h, // 表示サイズ
                              bmp->pal,       // パレット
                              GFL_BMP_CHRAREA_GET_B );// キャラ取得方向
    OS_Printf("add bmpwin %d\n", i);
  }

  // メッセージウインドウ用のフレームリソース転送
  BmpWinFrame_PalSet( GFL_BG_FRAME0_M, MSGWINFRAME_PAL, 0, HEAPID_PKWD_SELSCR );
  BmpWinFrame_GraphicSet( GFL_BG_FRAME0_M, MSGWINFRAME_CGX, MSGWINFRAME_PAL, 0, HEAPID_PKWD_SELSCR );

}

// BMPWIN削除
static void PrintTool_DelBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
  int i;
  
  for(i=0;i<PKWD_SELECT_SCREEN_BMPWIN_FIX_NUM;i++){
    GFL_BMPWIN_Delete( wk->print_util[i].win );
  }
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( PKWD_SELECT_SCREEN_MAIN_WORK * wk, int index )
{
  GFL_BMPWIN_MakeScreen( wk->print_util[index].win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util[index].win) );
  wk->print_util[index].transReq = TRUE;
}

// ＨＰ表示
static void PrintTool_PrintHP( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
//  GFL_MSG_GetString(wk->msg, mes_mictest_title, wk->strbuf );

	// BMPWIN内の座標(32,0)を基準に右詰めで「ＨＰ」と表示
  PRINT_UTIL_Print( &wk->print_util[0], wk->print_que, 32,0, wk->strbuf, wk->font );

}


//----------------------------------------------------------------------------------
/**
 * @brief	外部セーブデータ確認・読み込み
 *
 * @param   wk    
 * @param   param 
 */
//----------------------------------------------------------------------------------
static void  _load_pokewood_rec_data( PKWD_SELECT_SCREEN_MAIN_WORK *wk, PKWD_SELECT_SCREEN_PARAM *param )
{
  int i,j,ok_count=0,ng_count=LOAD_SAVEDATA_MAX-1;

  BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
  
  // ポケウッド録画データアクセス初期化
  POKEWOOD_REC_Init( HEAPID_PKWD_SELSCR );

  for(i=0;i<LOAD_SAVEDATA_MAX;i++){
    wk->svdata[i].slot_no     = i;
    wk->svdata[i].load_result = POKEWOOD_REC_Load( param->sv, HEAPID_PKWD_SELSCR, i );
    OS_Printf("%d:load result=%d\n", i , wk->svdata[i].load_result);

    // 読み込んで各種情報を取得したい場合はココで確保
    if(wk->svdata[i].load_result==RECLOAD_RESULT_OK){
      POKEMON_PARAM *pp = GFL_HEAP_AllocMemory( HEAPID_PKWD_SELSCR, POKETOOL_GetWorkSize());

      // シナリオＮＯ取得
      wk->svdata[i].scenario_no = POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_SENARIO, NULL);
      // 興行収入取得
      wk->svdata[i].profit      = POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_PROFIT, NULL);
      // 評価取得
      wk->svdata[i].success     = POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_SUCCESS, NULL);
      // 公開日取得
      wk->svdata[i].date        = POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_DATE, NULL);
      // 公開時間取得
      wk->svdata[i].time        = POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_TIME, NULL);
      // 鑑賞済みか？
      wk->svdata[i].watched     = POKEWOOD_SAVEDATA_GetMovieShowed( param->pokewood_sv, i );
      // 手持ちポケモンNO取得
      for(j=0;j<PKWD_POKEMON_MAX;j++){
        POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_POKE0+j, pp );
        wk->svdata[i].my_monsno[j] = PP_Get( pp, ID_PARA_monsno, NULL);
      }
      
    
      // 先頭手持ちポケモン名取得
      POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_POKE0, pp );
      WORDSET_RegisterPokeMonsName( wk->svdata[i].wordset, 0, pp );

      OS_Printf("%d: senario_no=%d\n", i , wk->svdata[i].scenario_no);
      for(j=0;j<PKWD_POKEMON_MAX;j++){ OS_Printf( "PP%d=%d, ",j,wk->svdata[i].my_monsno[j] ); }
       OS_Printf( "\n");
      GFL_HEAP_FreeMemory( pp );
    }else{
      wk->svdata[i].scenario_no = -1;
    }
  }

  // 有効データだけを詰めたポインタリストを作成する
  for(i=0;i<LOAD_SAVEDATA_MAX;i++){
    if(wk->svdata[i].load_result==RECLOAD_RESULT_OK){
      wk->valid_list[ok_count] = &wk->svdata[i];
      ok_count++;
    }else{
      wk->valid_list[ng_count] = &wk->svdata[i];
      ng_count--;
    }
  }

  // ポケウッド録画データアクセス処理終了
  POKEWOOD_REC_Exit();

  BattleRec_UnloadToolModule();
}

// タッチ入力判定用定義
static const GFL_UI_TP_HITTBL tp_data[] ={
	{ 0, 40, 0, 255 },
	{ 0, 80, 0, 255 },
	{ 0,120, 0, 255 },
	{ 0,160, 0, 255 },
  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};
//----------------------------------------------------------------------------------
/**
 * @brief	タッチパネル判定
 *
 * @param   wk		PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  int		選択できる上映番号
 */
//----------------------------------------------------------------------------------
static int _touch_input( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result = GFL_UI_TP_HitTrg( tp_data );
  if( result!=GFL_UI_TP_HIT_NONE ){
    if(wk->valid_list[_get_real_pos(wk,result)]->load_result){
      wk->isTouch = TRUE;
      wk->nowpos = result;
      wk->bgLogoRequest=TRUE;
      _draw_movie_subtitle( wk, _get_real_pos( wk, wk->nowpos) );
      return result;
    }
  }

  return GFL_UI_TP_HIT_NONE;
}


static int _key_input( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(wk->isTouch==TRUE && GFL_UI_KEY_GetTrg()){
    wk->isTouch = FALSE;
    wk->nowpos = 0;
  }else{
    
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
      if(wk->nowpos!=0){
        if(wk->valid_list[_get_real_pos(wk,wk->nowpos)-1]->load_result){
          PMSND_PlaySE( SE_PKWDSELECT_MOVE );
          wk->nowpos--;
        }
      }
    }else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
      if(wk->nowpos<3){
        if(wk->valid_list[_get_real_pos(wk,wk->nowpos)+1]->load_result){
          PMSND_PlaySE( SE_PKWDSELECT_MOVE );
          wk->nowpos++;
        }
      }
    }else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
      return wk->nowpos;
    }

  }
  return GFL_UI_TP_HIT_NONE;
}


//----------------------------------------------------------------------------------
/**
 * @brief	文字列描画システム系初期化
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void PrintInit( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;
	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  // メッセージフォント用パレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT,NARC_font_default_nclr, PALTYPE_MAIN_BG, 0x20*15, 0x20, HEAPID_PKWD_SELSCR );
  

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_pw_movietitle_dat, wk->heapID );
  wk->strbuf = GFL_STR_CreateBuffer( PKWD_TITLE_STR_MAX, HEAPID_PKWD_SELSCR );

  // WORDSET(リストの数だけ作成）
  for(i=0;i<LOAD_SAVEDATA_MAX;i++){
    wk->svdata[i].wordset = WORDSET_Create( HEAPID_PKWD_SELSCR );
  }
  // 上画面用
  wk->UpWordset = WORDSET_Create( HEAPID_PKWD_SELSCR );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );
  
  // メッセージ表示用タスクシステム初期化
  wk->pMsgTcblSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 32 , 32 );


}

//----------------------------------------------------------------------------------
/**
 * @brief	文字列描画システム系解放
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void PrintExit( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;

  // メッセージ表示用タスクシステム終了
  GFL_TCBL_Exit( wk->pMsgTcblSys );

	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );

  // WORDSET解放
  for(i=0;i<LOAD_SAVEDATA_MAX;i++){
    WORDSET_Delete( wk->valid_list[i]->wordset );
  }
  WORDSET_Delete( wk->UpWordset );
}

//----------------------------------------------------------------------------------
/**
 * @brief 下画面リスト描画
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void _draw_titlelist( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;
  STRBUF *strbuf = GFL_STR_CreateBuffer( PKWD_TITLE_STR_MAX, GetHeapLowID( HEAPID_PKWD_SELSCR ));

  for(i=0;i<PKWD_TITLE_LIST_NUM;i++){
    int index = i + (wk->page*PKWD_TITLE_LIST_NUM);
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->print_util[i].win ), 0 );
    if(wk->valid_list[index]->load_result){
      OS_Printf("strID = %d\n", pw_screen_01+index);
      GFL_MSG_GetString( wk->msg, pw_screen_01+index, strbuf );
      PRINT_UTIL_PrintColor( &wk->print_util[i], wk->print_que, 0, 4, strbuf, 
                             wk->font, PRINTSYS_LSB_Make( 1, 5, 0 ) );
      GFL_MSG_GetString( wk->msg, pw_title_00+wk->valid_list[index]->scenario_no, strbuf );
      PRINT_UTIL_PrintColor( &wk->print_util[i], wk->print_que, 0, 4+16, strbuf, 
                             wk->font, PRINTSYS_LSB_Make( 1, 5, 0 ) );
    }
    PrintTool_ScreenTrans( wk, i );		// スクリーン転送
    OS_Printf("%d:=%d\n", index, wk->valid_list[index]->load_result);
  }

  GFL_STR_DeleteBuffer(strbuf);

  OS_Printf(" remain memory = %08x\n", GFL_HEAP_GetHeapFreeSize(HEAPID_PKWD_SELSCR));
}


//----------------------------------------------------------------------------------
/**
 * @brief	センタリングして文字列描画
 */
//----------------------------------------------------------------------------------
static void _centering_print( PRINT_UTIL *util, PRINT_QUE *que, STRBUF *strbuf, int y, GFL_FONT *font, PRINTSYS_LSB col)
{
  int x,str_w,bmp_w;
  str_w = PRINTSYS_GetStrWidth( strbuf, font, 0 );
  bmp_w = GFL_BMPWIN_GetSizeX( util->win )*8;
  x = bmp_w/2 - str_w/2;

  PRINT_UTIL_PrintColor( util, que, x, y, strbuf, font, col );
  
}

//----------------------------------------------------------------------------------
/**
 * @brief	上画面のサブタイトルを描画する
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 * @param   index 録画データ番号( 0-7 )
 */
//----------------------------------------------------------------------------------
static const s16 up_msg_tbl[] = {
  pw_select_monitor_000,      // ○/○/○
  pw_select_monitor_001,      // ○:○
  pw_select_monitor_002,      // こうかいび
  pw_select_monitor_002,      // こうかいび(pad)
};

static const s16 up_seq_tbl[] = {
  SUB_SEQ_BAD,                // BAD
  SUB_SEQ_GOOD,               // GOOD
  SUB_SEQ_STRANGE,            // STRANGE
  SUB_SEQ_STRANGE,            // STRANGE(pad)
};

static void _draw_movie_subtitle( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int index )
{
  STRBUF *strbuf    = GFL_STR_CreateBuffer( PKWD_TITLE_STR_MAX, GetHeapLowID( HEAPID_PKWD_SELSCR ));
  STRBUF *expandbuf = GFL_STR_CreateBuffer( PKWD_TITLE_STR_MAX, GetHeapLowID( HEAPID_PKWD_SELSCR ));
  int sex = MyStatus_GetMySex(wk->param->mystatus);
  
  // 録画データなし
  if((wk->valid_list[index]->load_result==0)||(wk->nowpos == -1)){
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->print_util[PKWD_BMP_SUBTITLE_U].win), 0 );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->print_util[PKWD_BMP_SUBTITLE_D].win), 0 );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->print_util[PKWD_BMP_SUB_DATE].win),   0 );
    // 評価マーク非表示
    PkwdSelectScreen_SubObj_Visible( wk, SUB_OBJ_MARK, FALSE );
    wk->bgLogoRequest = TRUE;
  // 録画データ存在
  }else{
    // サブタイトル上部を描画
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->print_util[PKWD_BMP_SUBTITLE_U].win), 0 );
    GFL_MSG_GetString( wk->msg, pw_title_00+wk->valid_list[index]->scenario_no, strbuf );
    _centering_print( &wk->print_util[PKWD_BMP_SUBTITLE_U], wk->print_que, 
                      strbuf, 8, wk->font, PRINTSYS_LSB_Make( 13, 12, 0 ) );
    
    // 下部を描画
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->print_util[PKWD_BMP_SUBTITLE_D].win), 0 );
    GFL_MSG_GetString( wk->msg, pkwd_subtitle_tbl[wk->valid_list[index]->scenario_no][sex], strbuf );
    WORDSET_ExpandStr( wk->valid_list[index]->wordset, expandbuf, strbuf );
    _centering_print( &wk->print_util[PKWD_BMP_SUBTITLE_D], wk->print_que,
                      expandbuf, 6, wk->font, PRINTSYS_LSB_Make( 13, 12, 0 ) );
    //
    // 公開日
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->print_util[PKWD_BMP_SUB_DATE].win), 0 );
    GFL_MSG_GetString( wk->msg, up_msg_tbl[2], strbuf );
    PRINT_UTIL_PrintColor( &wk->print_util[PKWD_BMP_SUB_DATE], wk->print_que,
                       0, 0, strbuf, wk->font, PRINTSYS_LSB_Make( 15, 14, 0 ) );
    // 日付○/○/○
    GFL_MSG_GetString( wk->msg, up_msg_tbl[0], strbuf );
    WORDSET_RegisterNumber( wk->UpWordset, 0, 2000+GFDATE_GetYear( wk->valid_list[index]->date ), 
                            4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wk->UpWordset, 1, GFDATE_GetMonth(wk->valid_list[index]->date ), 
                            2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wk->UpWordset, 2, GFDATE_GetDay(wk->valid_list[index]->date ), 
                            2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wk->UpWordset, expandbuf, strbuf );
    PRINT_UTIL_PrintColor( &wk->print_util[PKWD_BMP_SUB_DATE], wk->print_que,
                       12*8, 0, expandbuf, wk->font, PRINTSYS_LSB_Make( 15, 14, 0 ) );
    // 時刻○:○
    GFL_MSG_GetString( wk->msg, up_msg_tbl[1], strbuf );
    WORDSET_RegisterNumber( wk->UpWordset, 0, GFTIME_GetHour(  wk->valid_list[index]->time), 
                            2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wk->UpWordset, 1, GFTIME_GetMinute(wk->valid_list[index]->time), 
                            2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wk->UpWordset, expandbuf, strbuf );
    PRINT_UTIL_PrintColor( &wk->print_util[PKWD_BMP_SUB_DATE], wk->print_que,
                       23*8, 0, expandbuf, wk->font, PRINTSYS_LSB_Make( 15, 14, 0 ) );
    //
    // 評価マーク表示
    PkwdSelectScreen_SubObj_ChgSeq(  wk, SUB_OBJ_MARK, up_seq_tbl[ wk->valid_list[index]->success ]);
    if(wk->valid_list[index]->watched==TRUE){
      PkwdSelectScreen_SubObj_Visible( wk, SUB_OBJ_MARK, TRUE );
    }else{
      PkwdSelectScreen_SubObj_Visible( wk, SUB_OBJ_MARK, FALSE );
    }
    //
    wk->bgLogoRequest = TRUE;
  }
  
  GFL_STR_DeleteBuffer(expandbuf);
  GFL_STR_DeleteBuffer(strbuf);

  PrintTool_ScreenTrans( wk, PKWD_BMP_SUBTITLE_U );		// スクリーン転送
  PrintTool_ScreenTrans( wk, PKWD_BMP_SUBTITLE_D );		// スクリーン転送
  PrintTool_ScreenTrans( wk, PKWD_BMP_SUB_DATE );		// スクリーン転送
}


//----------------------------------------------------------------------------------
/**
 * @brief	カーソル移動時に他のプレートのカラーを元に戻して
 *
 * @param   wk		PKWD_SELECT_SCREEN_MAIN_WORK
 * @param   pos		現在のカーソル位置
 */
//----------------------------------------------------------------------------------
static void _set_cursor_blink( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int pos )
{
  int i;
  
  for(i=0;i<PKWD_TITLE_LIST_NUM;i++){
    if(i==pos){
      BLINKPALANM_InitAnimeCount(wk->blinkWork[pos] );
    }else{
      BLINKPALANM_SetAnimeCount(wk->blinkWork[i], BLINK_NORMAL_COLOR );
      BLINKPALANM_Main( wk->blinkWork[i] );
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief	選択箇所がが更新されている場合は上画面に反映させる
 *
 * @param   wk		PKWD_SELECT_SCREEN_MAIN_WORK
 */
//----------------------------------------------------------------------------------
static void SubTitle_Func( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(wk->oldpos!=wk->nowpos && wk->nowpos>=0){
    wk->bgLogoRequest=TRUE;
    _draw_movie_subtitle( wk, _get_real_pos( wk, wk->nowpos) );
    _set_cursor_blink( wk, wk->nowpos );
  }
  wk->oldpos = wk->nowpos;
}

// 塗りつぶしカラーマクロ
#define WINCLR_COL(col) (((col)<<4)|(col))

//----------------------------------------------------------------------------------
/**
 * @brief	メッセージウインドウ描画開始
 *
 * @param   wk      PKWD_SELECT_SCREEN_MAIN_WORK *
 * @param   msg_id	メッセージＩＤ
 */
//----------------------------------------------------------------------------------
static void _message_print( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int msg_id )
{
  const BMPWIN_DAT *bmp = &bmpdat[PKWD_BMP_MSGWIN];
  GFL_BMPWIN *bmpwin;

  // メッセージ表示用のBMPWIN確保
  wk->print_util[PKWD_BMP_MSGWIN].win = GFL_BMPWIN_Create(
                            bmp->frame,     // ＢＧフレーム
                            bmp->x, bmp->y, // 表示座標
                            bmp->w, bmp->h, // 表示サイズ
                            bmp->pal,       // パレット
                            GFL_BMP_CHRAREA_GET_B );// キャラ取得方向

  bmpwin = wk->print_util[PKWD_BMP_MSGWIN].win;

  BmpWinFrame_Write( bmpwin, WINDOW_TRANS_OFF, MSGWINFRAME_CGX, MSGWINFRAME_PAL );
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( bmpwin ), WINCLR_COL(15));

  GFL_MSG_GetString( wk->msg, msg_id, wk->strbuf );
  wk->print_stream = PRINTSYS_PrintStream(
    bmpwin, 0, 0, wk->strbuf, wk->font,
    MSGSPEED_GetWait(), wk->pMsgTcblSys, 4, HEAPID_PKWD_SELSCR, 15 );
  
  GFL_BMPWIN_MakeTransWindow( bmpwin );

}  



static const monsname_pos[][2]={
  { 8, 16, }, {12*6, 16 }, { 12*12, 16 },
  { 8, 32, }, {12*6, 32 }, { 12*12, 32 },
};
//----------------------------------------------------------------------------------
/**
 * @brief	映画の情報ウインドウを描画する
 *
 * @param   wk		PKWD_SELECT_SCREEN_MAIN_WORK
 * @param   index	指定の映画のセーブNO
 */
//----------------------------------------------------------------------------------
static void _print_movie_information( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int index )
{
  const BMPWIN_DAT *bmp = &bmpdat[PKWD_BMP_INFORMATION];
  GFL_BMPWIN *bmpwin;
  int x, length, i;

  // メッセージ表示用のBMPWIN確保
  wk->print_util[PKWD_BMP_INFORMATION].win = GFL_BMPWIN_Create(
                            bmp->frame,     // ＢＧフレーム
                            bmp->x, bmp->y, // 表示座標
                            bmp->w, bmp->h, // 表示サイズ
                            bmp->pal,       // パレット
                            GFL_BMP_CHRAREA_GET_B );// キャラ取得方向
  
  bmpwin = wk->print_util[PKWD_BMP_INFORMATION].win;
  
  BmpWinFrame_Write( bmpwin, WINDOW_TRANS_OFF, MSGWINFRAME_CGX, MSGWINFRAME_PAL );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( bmpwin ), WINCLR_COL(15));

  {
    STRBUF *cast_str, *estimate_str, *brief_str, *plot_str, *money_str, *monsname;

    // 「おもなしゅつえんキャスト」
    cast_str     = GFL_MSG_CreateString( wk->msg, pw_info_pokemons );
    PRINT_UTIL_Print( &wk->print_util[PKWD_BMP_INFORMATION], wk->print_que, 4, 0,    cast_str, wk->font );

    // 「こうぎょうせいせき」
    estimate_str = GFL_MSG_CreateString( wk->msg,pw_info_estimate );
    PRINT_UTIL_Print( &wk->print_util[PKWD_BMP_INFORMATION], wk->print_que, 4, 7*8,  estimate_str, wk->font );

    // 「あらすじ」
    brief_str    = GFL_MSG_CreateString( wk->msg,pw_info_brief );
    PRINT_UTIL_Print( &wk->print_util[PKWD_BMP_INFORMATION], wk->print_que, 4, 10*8, brief_str, wk->font );

    // あらすじの内容
    plot_str     = GFL_MSG_CreateString( wk->msg, pw_plot_000+wk->valid_list[index]->scenario_no );
    PRINT_UTIL_Print( &wk->print_util[PKWD_BMP_INFORMATION], wk->print_que, 8, 12*8, plot_str, wk->font );

    // 興行収入の実際の数値＋「おく円」
    money_str    = GFL_MSG_CreateString( wk->msg, pw_record_monitor_03 );
    WORDSET_RegisterNumber( wk->valid_list[index]->wordset, 2, wk->valid_list[index]->profit, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wk->valid_list[index]->wordset, wk->strbuf, money_str );
    length = PRINTSYS_GetStrWidth( wk->strbuf, wk->font, 0 );
    x = 8*INFORMATION_W - length-8;
    PRINT_UTIL_Print( &wk->print_util[PKWD_BMP_INFORMATION], wk->print_que, x, 7*8, wk->strbuf, wk->font );

    // ポケモン名描画
    for(i=0;i<PKWD_POKEMON_MAX;i++){
      if(wk->valid_list[index]->my_monsno[i]!=0){
        monsname = GFL_MSG_CreateString( GlobalMsg_PokeName, wk->valid_list[index]->my_monsno[i] );
        PRINT_UTIL_Print( &wk->print_util[PKWD_BMP_INFORMATION], wk->print_que, 
                          monsname_pos[i][0], monsname_pos[i][1], monsname, wk->font );
        GFL_STR_DeleteBuffer( monsname   );
      }
    }

    // 文字列解放
    GFL_STR_DeleteBuffer(money_str   );
    GFL_STR_DeleteBuffer(plot_str    );
    GFL_STR_DeleteBuffer(brief_str   );
    GFL_STR_DeleteBuffer(estimate_str);
    GFL_STR_DeleteBuffer(cast_str    );
  }
  GFL_BMPWIN_MakeTransWindow( bmpwin );
}

//===========================================================
// サブシーケンス定義
//===========================================================
enum{
  MENU_FADEIN_START=0,
  MENU_FADEIN_WAIT,        // フェードイン待ち
  MENU_START,              // 開始
  MENU_MAIN,               // メイン処理
  MENU_DECIDE_WAIT,        // 決定アニメ待ち
  MENU_MSG_WAIT,           // メッセージ表示待ち
  MENU_WARNING_MSG,        // 警告メッセージ表示開始
  MENU_SELECT_WATCH,       // 「このえいがをみますか？」開始
  MENU_SELECT_WATCH_WAIT,  // メニュー待ち
  MENU_YESNO_DELETE1,      // 削除しますか？開始
  MENU_YESNO_DELETE1_WAIT, // 削除しますか？待ち
  MENU_YESNO_DELETE2,      // 本当に削除しますか？開始
  MENU_YESNO_DELETE2_WAIT, // 本当に削除しますか？はい・いいえ待ち
  MENU_FADEOUT,            // フェードアウト開始
  MENU_FADEOUT_WAIT,       // フェードアウト待ち
  MENU_PUT_INFORMATION,     // 情報表示
  MENU_PUT_INFORMATION_WAIT // 情報表示終了待ち
};


//===========================================================
// サブシーケンス関数テーブル
//===========================================================
static BOOL (*SubSeqFuncTbl[])( PKWD_SELECT_SCREEN_MAIN_WORK *wk )={
  SubSeq_FadeinStart,        // MENU_FADEIN_START=0
  SubSeq_FadeinWait,         // MENU_FADEIN_WAIT
  SubSeq_Start,              // MENU_START,
  SubSeq_Main,               // MENU_MAIN,
  SubSeq_DecideWait,         // MENU_DECIDE_WAIT,
  SubSeq_MsgWait,            // MENU_MSG_WAIT,
  SubSeq_WarningMsg,         // MENU_WARNING_MSG,
  SubSeq_Select_Watch,      // MENU_SELECT_WATCH,     
  SubSeq_Select_WatchWait,  // MENU_SELECT_WATCH_WAIT,
  SubSeq_Yesno_Delete1,      // MENU_YESNO_DELETE1,    
  SubSeq_Yesno_Delete1Wait,  // MENU_YESNO_DELETE1_WAIT
  SubSeq_Yesno_Delete2,      // MENU_YESNO_DELETE2,    
  SubSeq_Yesno_Delete2Wait,  // MENU_YESNO_DELETE2_WAIT
  SubSeq_FadeOut,            // MENU_FADEOUT,          
  SubSeq_FadeOutWait,        // MENU_FADEOUT_WAIT
  SubSeq_PutInformation,     // MENU_PUT_INFOMATION,     // 情報表示
  SubSeq_PutInformationWait, // MENU_PUT_INFOMATION_WAIT // 情報表示終了待ち
};


//----------------------------------------------------------------------------------
/**
 * @brief	サブシーケンス呼び出しメイン関数
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeqFunc( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  return SubSeqFuncTbl[wk->seq]( wk );
}


//----------------------------------------------------------------------------------
/**
 * @brief	サブシーケンス開始。フェードイン開始
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_FadeinStart( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  wk->bgLogoRequest=TRUE;
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 0, 8, 1, HEAPID_PKWD_SELSCR);
  wk->seq = MENU_FADEIN_WAIT;
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * @brief	フェードイン待ち
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_FadeinWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(WIPE_SYS_EndCheck()){
    wk->seq = MENU_START;
  }
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief	処理スタート
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_Start( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  wk->seq = MENU_MAIN;
  return FALSE;
}

static int _check_nowpos( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;
  // ロードが成功しているカーソル位置を探す
  for(i=wk->nowpos;i>=0;i--){
    if(wk->valid_list[_get_real_pos(wk, i)]->load_result==TRUE){
      break;
    }
  }
  return i;
}


//----------------------------------------------------------------------------------
/**
 * @brief	サブシーケンスメイン
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_Main( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result;

  // タッチ・キー制御
  if((result=_touch_input(wk))==GFL_UI_TP_HIT_NONE){
    result = _key_input(wk);
  }
  SubTitle_Func(wk);

  // 決定かキャンセルか
  if(result!=GFL_UI_TP_HIT_NONE){
    PMSND_PlaySE( SE_PKWDSELECT_DECIDE ); // 決定
    BLINKPALANM_SetDecideAnime( wk->blinkWork[wk->nowpos] );
    wk->seq = MENU_DECIDE_WAIT;
  }else{
    // キャンセル
    if(TOUCHBAR_GetTrg(wk->touchbar)==TOUCHBAR_ICON_RETURN){
      wk->seq = MENU_FADEOUT;
      *wk->param->result = LOAD_SAVEDATA_MAX;
    // ページ切り替え
    }else if(TOUCHBAR_GetTrg(wk->touchbar)==TOUCHBAR_ICON_CUR_L  ||
        TOUCHBAR_GetTrg(wk->touchbar)==TOUCHBAR_ICON_CUR_R ){

      // ページ切り替え処理まとめ
      wk->page ^= 1;
//      if( wk->isTouch==TRUE){
        wk->nowpos = _check_nowpos( wk );
        _draw_titlelist( wk );
        _draw_movie_subtitle( wk, _get_real_pos( wk, wk->nowpos) );
        _blink_palanime_exit( wk );
        _blink_palanime_init( wk );
//      }
      PkwdSelectScreen_ListObj_Update( wk, wk->page );
    }
  }
  return FALSE;

}

//----------------------------------------------------------------------------------
/**
 * @brief	決定アニメ待ち
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_DecideWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(BLINKPALANM_CheckDecideAnime( wk->blinkWork[wk->nowpos] )){
    if(wk->param->mode==PKWD_SELECT_MODE_NORMAL){
      _message_print( wk, pw_select_01 ); //「このさくひんを　かんしょうしますか？」
      wk->seq    = MENU_MSG_WAIT;
      wk->subseq = MENU_SELECT_WATCH;
    }else{
      _message_print( wk, pw_select_02 ); //「このさくひんを　さくじょしますか？」
      wk->seq    = MENU_MSG_WAIT;
      wk->subseq = MENU_YESNO_DELETE1;
    }
  }
  return FALSE;

}

//----------------------------------------------------------------------------------
/**
 * @brief	メッセーじ表示終了待ち
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_MsgWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  //メッセージ描画終了待ち
  if(PRINTSYS_PrintStreamGetState( wk->print_stream )==PRINTSTREAM_STATE_DONE)
  {
    PRINTSYS_PrintStreamDelete( wk->print_stream );
    wk->seq = wk->subseq;
  }
  return FALSE;

}

//----------------------------------------------------------------------------------
/**
 * @brief	「消えますがよろしいですか？」表示終了待ち
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_WarningMsg( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
   _message_print( wk, pw_select_03 ); //「このさくひんを　さくじょしますか？」
   wk->seq    = MENU_MSG_WAIT;
   wk->subseq = MENU_YESNO_DELETE2;
  return FALSE;

}

//----------------------------------------------------------------------------------
/**
 * @brief	はい・いいえ待ち
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  int   -1:未決定 0:はい  1:いいえ
 */
//----------------------------------------------------------------------------------
static int _menuselect_wait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result = -1;
  if(APP_TASKMENU_IsFinish(wk->menu)){
    result = APP_TASKMENU_GetCursorPos( wk->menu );
    PkwdSelectScreen_TASKMENU_Exit( wk->menu );
    wk->menu = NULL;
    BmpWinFrame_Clear( wk->print_util[PKWD_BMP_MSGWIN].win, WINDOW_TRANS_OFF );
    GFL_BMPWIN_ClearTransWindow( wk->print_util[PKWD_BMP_MSGWIN].win );
    GFL_BMPWIN_Delete( wk->print_util[PKWD_BMP_MSGWIN].win );
  }
  return result;

}


//----------------------------------------------------------------------------------
/**
 * @brief	上映用の３択開始
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_Select_Watch( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  wk->menu= PkwdSelectScreen_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID, PKWD_MENU_WATCH );
  wk->seq = MENU_SELECT_WATCH_WAIT;
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * @brief	上映選択はい・いいえ待ち
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_Select_WatchWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result;
  if((result=_menuselect_wait(wk))>=0){
    // 「みる」
    if(result==0){
      wk->seq = MENU_FADEOUT;
      *wk->param->result = wk->valid_list[_get_real_pos(wk, wk->nowpos)]->slot_no;
    // 「じょうほう」
    }else if(result==1){
      wk->seq = MENU_PUT_INFORMATION;
    // もどる
    }else{
      wk->seq   = MENU_START;
      _set_cursor_blink( wk, wk->nowpos );
    }
  }
  return FALSE;
  
}

//----------------------------------------------------------------------------------
/**
 * @brief	削除選択はい・いいえ1開始
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_Yesno_Delete1( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  wk->menu= PkwdSelectScreen_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID, PKWD_MENU_DELETE );
  wk->seq = MENU_YESNO_DELETE1_WAIT;
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * @brief	削除選択はい・いいえ1待ち
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_Yesno_Delete1Wait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result;
  if((result=_menuselect_wait(wk))>=0){
    // 「しゅうりょうする」
    if(result==0){
        wk->seq = MENU_WARNING_MSG;
    // 「じょうほう」
    }else if(result==1){
      wk->seq = MENU_PUT_INFORMATION;
    // 「もどる」
    }else{
      wk->seq   = MENU_START;
      _set_cursor_blink( wk, wk->nowpos );
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * @brief	削除選択はい・いいえ2開始
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_Yesno_Delete2( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  wk->menu= PkwdSelectScreen_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID, PKWD_MENU_YESNO );
  wk->seq = MENU_YESNO_DELETE2_WAIT;
  return FALSE;
  
}

//----------------------------------------------------------------------------------
/**
 * @brief	削除選択はい・いいえ待ち２
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_Yesno_Delete2Wait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result;
  if((result=_menuselect_wait(wk))>=0){
    if(result==0){
        wk->seq = MENU_FADEOUT;
        *wk->param->result = wk->valid_list[_get_real_pos(wk, wk->nowpos)]->slot_no;
    }else{
      wk->seq   = MENU_START;
      _set_cursor_blink( wk, wk->nowpos );
    }
  }
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief	フェードアウト開始
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_FadeOut( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 0, 8, 1, HEAPID_PKWD_SELSCR);
  wk->seq = MENU_FADEOUT_WAIT;
  return FALSE;

}

//----------------------------------------------------------------------------------
/**
 * @brief	フェードアウト終了待ち
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_FadeOutWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(WIPE_SYS_EndCheck()){
    return TRUE;
  }
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief	映画情報表示
 *
 * @param   wk		  PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL		
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_PutInformation( PKWD_SELECT_SCREEN_MAIN_WORK *wk ) 
{
  _print_movie_information( wk, _get_real_pos(wk, wk->nowpos) );
    _touchbar_button_refresh( wk, FALSE ); //ページ替えボタン復帰
  wk->seq = MENU_PUT_INFORMATION_WAIT;
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * @brief	フェードアウト終了待ち
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  BOOL  TRUE:サブシーケンス終了 FALSE:継続
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_PutInformationWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(TOUCHBAR_GetTrg(wk->touchbar)==TOUCHBAR_ICON_RETURN){ 
    BmpWinFrame_Clear( wk->print_util[PKWD_BMP_INFORMATION].win, WINDOW_TRANS_OFF );
    GFL_BMPWIN_ClearTransWindow( wk->print_util[PKWD_BMP_INFORMATION].win );
    GFL_BMPWIN_Delete( wk->print_util[PKWD_BMP_INFORMATION].win );
    _touchbar_button_refresh( wk, TRUE ); //ページ替えボタン復帰
    wk->seq = MENU_START;
  }
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief	５個以上のセーブデータが存在してるなら２ページ用のボタンを表示させる
 *
 * @param   wk		
 * @param   flag	TRUE:表示ON FALSE:表示OFF
 */
//----------------------------------------------------------------------------------
static void _touchbar_button_refresh( PKWD_SELECT_SCREEN_MAIN_WORK *wk, BOOL flag )
{
  if(wk->valid_list[4]->load_result==FALSE){
    // ４個以内なら表示しない
    flag = FALSE;
  }

  TOUCHBAR_SetVisible( wk->touchbar,  TOUCHBAR_ICON_CUR_L, flag );  
  TOUCHBAR_SetVisible( wk->touchbar,  TOUCHBAR_ICON_CUR_R, flag );
}

