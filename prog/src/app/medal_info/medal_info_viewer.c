//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medal_info_viewer.c
 *	@brief  メダル詳細画面モジュール
 *	@author	Masayuki Onoue
 *	@date		2011.07.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "app/mdlid_def.h"        // for MedalID
#include "system/gfl_use.h"       // GFUser_VIntr_CreateTCB
#include "arc_def.h"              // ARC_ID
#include "message.naix"           // MESSAGE_ARC_ID

#include "sound/pm_sndsys.h"      // サウンド

#include "print/gf_font.h"        // フォント
#include "font/font.naix"         // フォント
#include "print/wordset.h"        // ワードセット

#include "app_menu_common.naix"   // 共通素材
#include "app/app_menu_common.h"  // 共通素材
#include "medalinfo_gra.naix"      // グラフィックアーカイブ
#include "medalicon_gra.naix"      // グラフィックアーカイブ

#include "msg/msg_medalname.h"    // for MDL_NAME_HINT_000
#include "msg/msg_medalinfo.h"     // for MSG_MEDALBOX_～

#include "app/app_printsys_common.h"  // プリントシステム

#include "savedata/medalbox_rank_def.h" // for MEDALBOX_RANK_xxxx
#include "system/medaldata.h"

#include "system/medaldata.h"

#include "medal_info_viewer.h"      // アプリ描画モジュール
#include "medal_info_viewer.cdat"   // アプリ描画モジュールデータ

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メダル詳細画面モジュールワーク
//=====================================
struct MEDALVIEWER_WK_tag{

  // システム関連
	HEAPID heapID;
	GFL_TCB						*p_vblank_task;
	GFL_CLUNIT			  *p_clunit;
	GFL_CLWK				  *p_clwk[OBJ_CLWK_MAX];
  u32							  obj_res[OBJRESID_MAX];
  MEDALVIEWER_INITPARAM init_param;        // 初期化用パラメータ
  BOOL              is_push_medal;
  u16               main_seq;
 
  // 文字表示
  PRINT_QUE         *p_print_que;
  GFL_BMPWIN        *p_text_win;
  PRINT_UTIL        print_util;

  // メッセージ関連
	GFL_FONT			    *p_large_font;
	GFL_FONT			    *p_small_font;
	GFL_MSGDATA		    *p_app_msg;
	GFL_MSGDATA		    *p_mdlname_msg;
	GFL_MSGDATA		    *p_mdlinfo_msg;
  BOOL              is_print_util_wait;    // プリントユーティリティー終了待ちフラグ

  // スクロール
  fx32  scroll_pos;

  // 終了方法
  u32 result;

  // アプリデータ
  MEDALVIEWER_INITPARAM *p_init_param;
  MEDAL_INFO *p_now_medal_info;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

static void MEDALVIEWER_CommonDataInit( MEDALVIEWER_WK *p_wk ,MEDALVIEWER_INITPARAM *p_init_param );
static void MEDALVIEWER_CommonDataDelete( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_OBJInit( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_OBJDelete( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs );
static void MEDALVIEWER_BGInit( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_BGDelete( MEDALVIEWER_WK *p_wk );
static u32 MEDALVIEWER_BG_GetFrame( int frame_id );
static void MEDALVIEWER_GraphicInit( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_GraphicDelete( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_BMPWinInit( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_BMPWinDelete( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_PrintInit( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_PrintDelete( MEDALVIEWER_WK *p_wk );
static void DrawMedalInfo( MEDALVIEWER_WK *p_wk );
static void DrawMedalInfo_no_pushMedal( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_ScrollMain( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_InputCheck( MEDALVIEWER_WK *p_wk );
static void MEDALVIEWER_SetNextSeq( MEDALVIEWER_WK *p_wk ,u32 next_seq );
static void SetReturnButton( MEDALVIEWER_WK *p_wk );
static void SetExitButton( MEDALVIEWER_WK *p_wk );

//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  メダル詳細画面描画モジュールのセットアップ
 *
 *	@param	*p_init_param 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
MEDALVIEWER_WK* MEDALVIEWER_Setup( MEDALVIEWER_INITPARAM *p_init_param )
{
  MEDALVIEWER_WK *p_wk;
  HEAPID  heapID = p_init_param->heapID;

  // VIEWERワークを生成
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(MEDALVIEWER_WK) );
  GFL_STD_MemClear( p_wk, sizeof(MEDALVIEWER_WK) );


  // ヒープ設定
  p_wk->heapID = heapID;

  // 共通データ作成
  MEDALVIEWER_CommonDataInit( p_wk ,p_init_param );

  // グラフィック初期化
  MEDALVIEWER_GraphicInit( p_wk );

  // プリントシステムの初期化
  MEDALVIEWER_PrintInit( p_wk );

  //通信アイコン
  GFL_NET_ReloadIconTopOrBottom( TRUE , p_wk->heapID );

  // メダル情報を表示する
  if( p_wk->is_push_medal == TRUE )
  {
    DrawMedalInfo( p_wk );
  }
  else
  {
    DrawMedalInfo_no_pushMedal( p_wk );
  }

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル詳細画面描画モジュールのメイン処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT MEDALVIEWER_Main( MEDALVIEWER_WK *p_wk )
{
  GFL_PROC_RESULT  proc_ret = GFL_PROC_RES_CONTINUE;

  // メイン
  switch( p_wk->main_seq )
  {
  case SEQ_FADE_IN_START:
    {
      if( p_wk->is_print_util_wait == FALSE )
      {
        // 文字転送
        GFL_BMPWIN_TransVramCharacter( p_wk->p_text_win );
        GFL_BG_LoadScreenV_Req(MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_FONT_M));

        // ブラックアウトの開始
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
        MEDALVIEWER_SetNextSeq( p_wk , SEQ_FADE_IN_WAIT );
      }
    }
    break;
  case SEQ_FADE_IN_WAIT:
    {
      // フェードの終了待ち
      if( !GFL_FADE_CheckFade() )
      {
        MEDALVIEWER_SetNextSeq( p_wk , SEQ_MAIN );
      }
    }
    break;
  case SEQ_MAIN:
    {
      // スクロールメイン
      MEDALVIEWER_ScrollMain( p_wk );

      // キー入力メイン
      MEDALVIEWER_InputCheck( p_wk );
    }
    break;
  case SEQ_FADE_OUT_START:
    {
      // ブラックアウトの開始
      if( (GFL_CLACT_WK_CheckAnmActive(p_wk->p_clwk[OBJ_CLWK_RETURN]) == FALSE ) || 
        (GFL_CLACT_WK_CheckAnmActive(p_wk->p_clwk[OBJ_CLWK_EXIT]) == FALSE ) )
      {
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
        MEDALVIEWER_SetNextSeq( p_wk , SEQ_FADE_OUT_WAIT );
      }
    }
    break;
  case SEQ_FADE_OUT_WAIT:
    {
      // フェードの終了待ち
      if( !GFL_FADE_CheckFade() )
      {
        MEDALVIEWER_SetNextSeq( p_wk , SEQ_EXIT );
      }
    }
    break;
  case SEQ_EXIT:
    {
      proc_ret = GFL_PROC_RES_FINISH;
    }
    break;
  }

  // ユーティリティーの更新
  {
    BOOL trans_end;

    // 描画待ちフラグを初期化
    p_wk->is_print_util_wait = FALSE;

    // プリントキューを更新
    PRINTSYS_QUE_Main( p_wk->p_print_que );

    trans_end = PRINT_UTIL_Trans( &p_wk->print_util, p_wk->p_print_que );

    if( trans_end == FALSE )
    {
      p_wk->is_print_util_wait = TRUE;

    }
  }

  // セルの更新
  GFL_CLACT_SYS_Main();
  
  return proc_ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル詳細画面描画モジュールの終了処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 MEDALVIEWER_Delete( MEDALVIEWER_WK *p_wk )
{
  u32 result;

  // リザルトの保存
  result = p_wk->result;

  // 描画システム
  MEDALVIEWER_PrintDelete( p_wk );

  // グラフィックデータ
  MEDALVIEWER_GraphicDelete( p_wk );
 
  // 共通データ
  MEDALVIEWER_CommonDataDelete( p_wk );

	GFL_HEAP_FreeMemory( p_wk );

  return result;
}

//----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_InputCheck( MEDALVIEWER_WK *p_wk )
{
  const int trg = GFL_UI_KEY_GetTrg();
  int tp_hit;

  // キャンセル
  if( p_wk->p_init_param->call_mode == MEDALVIEWER_CALL_SUB )
  {
    if( trg & PAD_BUTTON_X )
    {
      SetExitButton( p_wk );
      GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
      return;
    }
  }

  // 戻る
  if( trg & PAD_BUTTON_B )
  {
    SetReturnButton( p_wk );
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    return;
  }
  
  // タッチによるあたり判定
  tp_hit = GFL_UI_TP_HitTrg( sc_ui_hittbl_trg );

  if( p_wk->p_init_param->call_mode == MEDALVIEWER_CALL_SUB )
  {
    // 両ボタンが存在する場合
    switch( tp_hit )
    {
    case UI_HIT_TABLE_TRG_R:
      SetReturnButton( p_wk );
      GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
      break;
    case UI_HIT_TABLE_TRG_L:
      SetExitButton( p_wk );
      GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      break;
    }
  }
  else
  {
    // 右側のボタンのみ判定
    switch( tp_hit )
    {
    case UI_HIT_TABLE_TRG_R:
      SetReturnButton( p_wk );
      GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
      break;
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  ×ボタン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetExitButton( MEDALVIEWER_WK *p_wk )
{
  // アニメーション
  GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[OBJ_CLWK_EXIT] , APP_COMMON_BARICON_EXIT_ON );
  GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[OBJ_CLWK_EXIT] , TRUE);
  MEDALVIEWER_SetNextSeq( p_wk , SEQ_FADE_OUT_START );
  PMSND_PlaySystemSE(SE_EXIT);

  // 終了パラメータを格納
  p_wk->result = MEDALVIEWER_RESULT_EXIT;
}

//----------------------------------------------------------------------------
/**
 *	@brief  戻るボタン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetReturnButton( MEDALVIEWER_WK *p_wk )
{
  // アニメーション
  GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[OBJ_CLWK_RETURN] , APP_COMMON_BARICON_RETURN_ON );
  GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[OBJ_CLWK_RETURN] , TRUE);

  MEDALVIEWER_SetNextSeq( p_wk , SEQ_FADE_OUT_START );
  PMSND_PlaySystemSE(SE_CANCEL);

  // 終了パラメータを格納
  p_wk->result = MEDALVIEWER_RESULT_RETURN;
}

//----------------------------------------------------------------------------
/**
 *	@brief  シーケンス切り替え
 *
 *	@param	*p_wk 
 *	@param	next_seq 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_SetNextSeq( MEDALVIEWER_WK *p_wk ,u32 next_seq )
{
  p_wk->main_seq = next_seq;
}

//----------------------------------------------------------------------------
/**
 *	@brief  背景のスクロール処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_ScrollMain( MEDALVIEWER_WK *p_wk )
{

  p_wk->scroll_pos += BACK_SCROLL_SPEED;
  if( FX_Whole(p_wk->scroll_pos) > 256 )
  {
    p_wk->scroll_pos = 0;
  }

  GFL_BG_SetScrollReq( MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BACK_M) ,GFL_BG_SCROLL_X_SET , -FX_Whole(p_wk->scroll_pos) );
  GFL_BG_SetScrollReq( MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BACK_S) ,GFL_BG_SCROLL_X_SET , -FX_Whole(p_wk->scroll_pos) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  共通データ作成
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_CommonDataInit( MEDALVIEWER_WK *p_wk ,MEDALVIEWER_INITPARAM *p_init_param )
{
  u32 target;
  HEAPID heapID = p_wk->heapID;
  
  // 初期化パラメータを保存
  p_wk->p_init_param = p_init_param;
  ONOUE_Printf("残りヒープサイズ%x\n",GFL_HEAP_GetHeapFreeSize( heapID ));
  
  // 初回ターゲットを設定
  target = p_init_param->target;
  p_wk->p_now_medal_info = &p_init_param->p_info_list[target];

  // イチオシメダルがあるかどうかの判定
  if( p_wk->p_now_medal_info->medal_id == MDLID_MAX )
  {
    p_wk->is_push_medal = FALSE;
  }else
  {
    p_wk->is_push_medal = TRUE;
  }

	// フォント
	p_wk->p_large_font	= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
	p_wk->p_small_font	= GFL_FONT_Create( ARCID_FONT, NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );


  // アプリケーション用メッセージ
  p_wk->p_app_msg =GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalinfo_dat, heapID );

	// メダル名メッセージデータ
	p_wk->p_mdlname_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalname_dat, heapID );

	// メダル説明文メッセージデータ
	p_wk->p_mdlinfo_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalexplanation_dat, heapID );

  // 描画待ちフラグを初期化
  p_wk->is_print_util_wait = TRUE;

  // スクロール位置の初期化
  p_wk->scroll_pos = 0;

}

//----------------------------------------------------------------------------
/**
 *	@brief  共通データ開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_CommonDataDelete( MEDALVIEWER_WK *p_wk )
{
  // メッセージ
  GFL_MSG_Delete( p_wk->p_app_msg );
	GFL_MSG_Delete( p_wk->p_mdlinfo_msg );
	GFL_MSG_Delete( p_wk->p_mdlname_msg );

  // フォント
	GFL_FONT_Delete( p_wk->p_small_font );
	GFL_FONT_Delete( p_wk->p_large_font );
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィック初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_GraphicInit( MEDALVIEWER_WK *p_wk )
{
  //レジスタ初期化
  G2_BlendNone();
  G2S_BlendNone();	
  
  //VRAMクリアー
  GFL_DISP_ClearVRAM( 0 );

  //VRAMバンク設定
  GFL_DISP_SetBank( &sc_vramSetTable );

  // ディスプレイON
  GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
  GFL_DISP_SetDispOn();

  //	表示
  GFL_DISP_GX_InitVisibleControl();

  //	フォント初期化
  GFL_FONTSYS_Init();

  // BGの初期化
  MEDALVIEWER_BGInit( p_wk );

  // OBJの初期化
  MEDALVIEWER_OBJInit( p_wk );

  // ビットマップウィンドウの初期化
  MEDALVIEWER_BMPWinInit( p_wk );

  // 描画用Vブランクタスクを登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB( MEDALVIEWER_Graphic_VBlankTask , p_wk , 0 );
  
}


//----------------------------------------------------------------------------
/**
 *	@brief  BGの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_BGInit( MEDALVIEWER_WK *p_wk )
{
  int i;
  HEAPID heapID = p_wk->heapID;

	//初期化
	GFL_BG_Init( heapID );

  // ブレンド設定
  {
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1 ,GX_BLEND_PLANEMASK_BG0 , 16, 5 );
  }

	//グラフィックモード設定
	{	
		static const GFL_BG_SYS_HEADER sc_bgsys_header	=
		{	
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
		};
		GFL_BG_SetBGMode( &sc_bgsys_header );
	}

	//BG面設定
	{	
		int i;
		for( i = 0; i < MEDALVIEWER_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_SetBGControl( sc_bgsetup[i].frame, &sc_bgsetup[i].bgcnt_header, sc_bgsetup[i].mode );
			GFL_BG_SetVisible( sc_bgsetup[i].frame, VISIBLE_ON );
    }
  }

	//読み込み
	{	
    u8 color_offset = 0;
		ARCHANDLE	*p_handle;
		ARCHANDLE	*p_common_handle;
		p_handle	= GFL_ARC_OpenDataHandle( ARCID_MEDALINFOGRA , heapID );
		p_common_handle	= GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON , heapID );

    // カラーオフセット取得
    color_offset = p_wk->p_now_medal_info->box_rank;
    if( MEDALBOX_RANK_BW == p_wk->p_now_medal_info->box_rank )
    {
      // ブラックバージョンのときはカラーを黒に変更
      if(GetVersion()==VERSION_B2)
      {
        color_offset += 1;
      }
    }

    // パレット読み込み
    {
      // 下画面パレット(メイン)
      GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_medalinfo_gra_medalinfo_bg_NCLR,
          PALTYPE_MAIN_BG, 0x20*color_offset , 0x20*MDLINFO_BACK_PAL_ID , 0x20 , heapID );

      // 下画面共通素材用パレット(メイン)
      GFL_ARCHDL_UTIL_TransVramPalette( p_common_handle, NARC_app_menu_common_menu_bar_NCLR,
          PALTYPE_MAIN_BG, 0x20*MDLINFO_BAR_PAL_ID, 0x20 , heapID );
 
      // フォント用パレットの読み込み
      GFL_ARCHDL_UTIL_TransVramPalette( p_common_handle, NARC_medalinfo_gra_medalinfo_bg_NCLR,
          PALTYPE_MAIN_BG, 0x20*MDLINFO_FONT_PAL_ID, 0x20 , heapID );
    }

    // キャラクターデータ
    {
      GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_medalinfo_gra_medalinfo_bg_NCGR,
          MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BACK_M), 0, 0, FALSE, heapID );
    }

    // キャラクターデータ（共通素材)
    {
      GFL_ARCHDL_UTIL_TransVramBgCharacter( p_common_handle, NARC_app_menu_common_menu_bar_NCGR,
          MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BAR_M), 0, 0, FALSE, heapID );
    }

    // スクリーン
    {
      // 背景
      GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalinfo_gra_medalinfo_bgd1_NSCR,
          MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BACK_M), 0, 0, FALSE, heapID );
      
      // 文字盤
      if( p_wk->is_push_medal == TRUE )
      {
        GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalinfo_gra_medalinfo_bgd2_NSCR,
            MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_FONTBORD_M), 0, 0, FALSE, heapID );
      }
      else
      {
        GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalinfo_gra_medalinfo_bgd3_NSCR,
            MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_FONTBORD_M), 0, 0, FALSE, heapID );
      }

      // メニューバー
      GFL_ARCHDL_UTIL_TransVramScreen( p_common_handle, NARC_app_menu_common_menu_bar_NSCR,
          MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BAR_M), 0, 0, FALSE, heapID );
    }

    // ====================================
		// 上画面(サブ)
    // ====================================
    // パレット読み込み
    {
      // 上画面パレット(サブ)
      GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_medalinfo_gra_medalinfo_bg_NCLR,
          PALTYPE_SUB_BG, 0x20*color_offset , 0x20*MDLINFO_BACK_PAL_ID , 0x20 , heapID );
    }

    // キャラクターデータ
    {
      // 背景
      GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_medalinfo_gra_medalinfo_bg_NCGR,
          MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BACK_S), 0, 0, FALSE, heapID );
    }

    // スクリーン（キャラクターは上下共通）
    {
      // 背景
      GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalinfo_gra_medalinfo_bgu1_NSCR,
          MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BACK_S), 0, 0, FALSE, heapID );
    }

		GFL_ARC_CloseDataHandle( p_handle );
		GFL_ARC_CloseDataHandle( p_common_handle );
	}

  // 共通素材面の使用パレット書き換え
  {
    GFL_BG_ChangeScreenPalette( MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BAR_M), 0, 0, 32, 24, MDLINFO_BAR_PAL_ID);
	  GFL_BG_LoadScreenReq( MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_BAR_M) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGデータからフレームIDを取得する
 *
 *	@param	frame_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 MEDALVIEWER_BG_GetFrame( int frame_id )
{
  return sc_bgsetup[ frame_id ].frame;
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックデータの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_GraphicDelete( MEDALVIEWER_WK *p_wk )
{
  
  // VBLANKTask消去
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );
 
  // ビットマップウィンドウ
  MEDALVIEWER_BMPWinDelete( p_wk );

  // OBJ
  MEDALVIEWER_OBJDelete( p_wk );

  // BGの開放
  MEDALVIEWER_BGDelete( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGデータの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_BGDelete( MEDALVIEWER_WK *p_wk )
{
  	
	//BG面破棄
	{	
		int i;
		for( i = 0; i < MEDALVIEWER_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_ClearFrame( sc_bgsetup[i].frame );
			GFL_BG_FreeBGControl( sc_bgsetup[i].frame );
		}
	}

  // ブレンド設定
  {
    G2S_SetBlendAlpha(GX_BLEND_PLANEMASK_NONE,GX_BLEND_PLANEMASK_NONE,31,0);
  }

	GFL_BG_Exit();
}

//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_BMPWinInit( MEDALVIEWER_WK *p_wk )
{
    int i;
	
  // BMPWINの初期化
  GFL_BMPWIN_Init( p_wk->heapID );
  
  // 名前用BMPWIN作成
  p_wk->p_text_win = GFL_BMPWIN_Create( MEDALVIEWER_BG_GetFrame(MEDALVIEWER_BG_FRAME_FONT_M) ,
      0   , 0   ,
      32  , 24  ,
      MDLINFO_FONT_PAL_ID , GFL_BMP_CHRAREA_GET_B );

  GF_ASSERT( p_wk->p_text_win );
  GFL_BMPWIN_MakeScreen( p_wk->p_text_win );
  
  PRINT_UTIL_Setup( &p_wk->print_util , p_wk->p_text_win  );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビットマップウィンドウの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_BMPWinDelete( MEDALVIEWER_WK *p_wk )
{
  // BMPWINの破棄
  GFL_BMPWIN_Delete( p_wk->p_text_win );
	
  // BMPWINの初期化
  GFL_BMPWIN_Exit();
}

//----------------------------------------------------------------------------
/**
 *	@brief  プリントシステム初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_PrintInit( MEDALVIEWER_WK *p_wk )
{
  // プリントキューの作成
  p_wk->p_print_que = PRINTSYS_QUE_Create( p_wk->heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  プリントシステムの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_PrintDelete( MEDALVIEWER_WK *p_wk )
{
  PRINTSYS_QUE_Clear( p_wk->p_print_que );
  PRINTSYS_QUE_Delete( p_wk->p_print_que );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル情報の書き込み
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawMedalInfo( MEDALVIEWER_WK *p_wk )
{
  STRBUF * str_buf;
  STRBUF * str;
  WORDSET *p_word_set = WORDSET_Create( p_wk->heapID ); 
  PRINTSYS_LSB font_colro = PRINTSYS_LSB_Make( FLONT_COLLAR_L , FLONT_COLLAR_S , FLONT_COLLAR_B);

  // タイトル
  {
    str = GFL_STR_CreateBuffer( 20, p_wk->heapID );
    str_buf = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_01 );
    
    // 名前を登録
	  WORDSET_RegisterWord( p_word_set, 0, p_wk->p_now_medal_info->p_trainer_name, 0, TRUE, PM_LANG );
    WORDSET_ExpandStr( p_word_set, str , str_buf  );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util ,
        p_wk->p_print_que , STR_POS_TITLE_X , STR_POS_TITLE_Y ,
        str ,p_wk->p_large_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
    GFL_STR_DeleteBuffer( str_buf );
  }

  // 獲得メダル
  {
    str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_02 );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util ,
        p_wk->p_print_que , STR_POS_TITLE_KAKUTOKU_X , STR_POS_TITLE_KAKUTOKU_Y ,
        str ,p_wk->p_small_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
  }

  // 獲得数
  {
    str = GFL_STR_CreateBuffer( 20, p_wk->heapID );
    str_buf = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_07 );

    // メダル数を登録
    WORDSET_RegisterNumber( p_word_set , 0 , p_wk->p_now_medal_info->medal_cnt , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_ZENKAKU);
    WORDSET_ExpandStr( p_word_set, str , str_buf  );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util ,
        p_wk->p_print_que , STR_POS_KAKUTOKU_X , STR_POS_KAKUTOKU_Y ,
        str ,p_wk->p_large_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
    GFL_STR_DeleteBuffer( str_buf );
  } 

  // イチオシメダル（タイトル）
  {
    str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_03 );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util ,
        p_wk->p_print_que , STR_POS_TITLE_ICHIOSI_X , STR_POS_TITLE_ICHIOSI_Y ,
        str ,p_wk->p_small_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
  }

  // メダル名
  {
    str = GFL_STR_CreateBuffer( 20, p_wk->heapID );
    str_buf = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_04 );

    // メダル名を登録
    WORDSET_RegisterMedalName( p_word_set ,0 ,p_wk->p_now_medal_info->medal_id);
    WORDSET_ExpandStr( p_word_set, str , str_buf  );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util ,
        p_wk->p_print_que , STR_POS_MEDAL_NAME_X , STR_POS_MEDAL_NAME_Y ,
        str ,p_wk->p_large_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
    GFL_STR_DeleteBuffer( str_buf );
  }
 
  // 獲得した日付(タイトル）
  {
    str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_05 );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util ,
        p_wk->p_print_que , STR_POS_TITLE_GET_DAY_X , STR_POS_TITLE_GET_DAY_Y ,
        str ,p_wk->p_small_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
  }
 
  // 獲得した日付
  {
    str = GFL_STR_CreateBuffer( 20, p_wk->heapID );
    str_buf = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_06 );

    // 日付を登録
    WORDSET_RegisterNumber( p_word_set , 0 , p_wk->p_now_medal_info->get_year , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_ZENKAKU);
    WORDSET_RegisterNumber( p_word_set , 1 , p_wk->p_now_medal_info->get_month , 2 , STR_NUM_DISP_SPACE , STR_NUM_CODE_ZENKAKU);
    WORDSET_RegisterNumber( p_word_set , 2 , p_wk->p_now_medal_info->get_day , 2 , STR_NUM_DISP_SPACE , STR_NUM_CODE_ZENKAKU);
    WORDSET_ExpandStr( p_word_set, str , str_buf  );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util ,
        p_wk->p_print_que , STR_POS_GET_DAY_X , STR_POS_GET_DAY_Y ,
        str ,p_wk->p_large_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
    GFL_STR_DeleteBuffer( str_buf );
  }

  // 説明文
  {
    str = GFL_MSG_CreateString( p_wk->p_mdlinfo_msg , p_wk->p_now_medal_info->medal_id );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util ,
        p_wk->p_print_que , STR_POS_INFO_X , STR_POS_INFO_Y ,
        str ,p_wk->p_large_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
  }

  // ワードセットの開放
  WORDSET_Delete( p_word_set );

}

//----------------------------------------------------------------------------
/**
 *	@brief  イチオシメダルがない場合の描画
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawMedalInfo_no_pushMedal( MEDALVIEWER_WK *p_wk )
{
  STRBUF * str_buf;
  STRBUF * str;
  WORDSET *p_word_set = WORDSET_Create( p_wk->heapID ); 
  PRINTSYS_LSB font_colro = PRINTSYS_LSB_Make( FLONT_COLLAR_L , FLONT_COLLAR_S , FLONT_COLLAR_B);

  // タイトル
  {
    str = GFL_STR_CreateBuffer( 20, p_wk->heapID );
    str_buf = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_01 );

    // 名前を登録
	  WORDSET_RegisterWord( p_word_set, 0, p_wk->p_now_medal_info->p_trainer_name, 0, TRUE, PM_LANG );
    WORDSET_ExpandStr( p_word_set, str , str_buf  );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util ,
        p_wk->p_print_que , STR_POS_TITLE_X_NO_PUSH , STR_POS_TITLE_Y_NO_PUSH ,
        str ,p_wk->p_large_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
    GFL_STR_DeleteBuffer( str_buf );
  }

  // 獲得メダル
  {
    str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_02 );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util , p_wk->p_print_que ,
        STR_POS_TITLE_KAKUTOKU_X_NO_PUSH , STR_POS_TITLE_KAKUTOKU_Y_NO_PUSH ,
        str ,p_wk->p_small_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
  }

  // 獲得数
  {
    str = GFL_STR_CreateBuffer( 20, p_wk->heapID );
    str_buf = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_07 );

    // メダル数を登録
    WORDSET_RegisterNumber( p_word_set , 0 , p_wk->p_now_medal_info->medal_cnt , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_ZENKAKU);
    WORDSET_ExpandStr( p_word_set, str , str_buf  );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util , p_wk->p_print_que , 
        STR_POS_KAKUTOKU_X_NO_PUSH , STR_POS_KAKUTOKU_Y_NO_PUSH ,
        str ,p_wk->p_large_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
    GFL_STR_DeleteBuffer( str_buf );
  } 

  // イチオシメダル（タイトル）
  {
    str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_03 );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util , p_wk->p_print_que , 
        STR_POS_TITLE_ICHIOSI_X_NO_PUSH , STR_POS_TITLE_ICHIOSI_Y_NO_PUSH ,
        str ,p_wk->p_small_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
  }

  // イチオシメダル(なし)
  {
    str = GFL_MSG_CreateString( p_wk->p_app_msg , MSG_MEDALINFO_08 );

    // 文字列を描画
    PRINT_UTIL_PrintColor( &p_wk->print_util , p_wk->p_print_que , 
        STR_POS_MEDAL_NAME_X_NO_PUSH , STR_POS_MEDAL_NAME_Y_NO_PUSH ,
        str ,p_wk->p_large_font , font_colro);

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
  }
 
  // ワードセットの開放
  WORDSET_Delete( p_word_set );
}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_OBJInit( MEDALVIEWER_WK *p_wk )
{
  
  HEAPID heapID = p_wk->heapID;

	//システム作成
  {
    const GFL_CLSYS_INIT sc_clsys_init = {
      0, 0,
      0, 512,
      4, 124,
      4, 124,
      0,
      32,32,32,32,
      16, 16,
    };
    GFL_CLACT_SYS_Create( &sc_clsys_init, &sc_vramSetTable , heapID );
  }

	// ユニット
	{	
		p_wk->p_clunit	= GFL_CLACT_UNIT_Create( 128, 1, heapID );
		GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );
	}

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

	//リソース読み込み
	{	
    int i;
		ARCHANDLE	*	p_handle[ARC_TYPE_MAX];
    u32* obj_res = p_wk->obj_res;
   
    // =======================================
    // リソースの読み込み
    // =======================================
    p_handle[ARC_TYPE_COMMON_BUTTON_GRA] = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON , heapID );
    p_handle[ARC_TYPE_MEDAL_ICON] = GFL_ARC_OpenDataHandle( ARCID_MEDALICON , heapID );

    // ======================================
    // メダルOBJ
    // ======================================
    obj_res[ OBJRESID_MEDAL_CELLANM ] = GFL_CLGRP_CELLANIM_Register( p_handle[ARC_TYPE_MEDAL_ICON],
        NARC_medalicon_gra_medal_icon_NCER, NARC_medalicon_gra_medal_icon_NANR , heapID );

    obj_res[ OBJRESID_MEDAL_PLTT ]    = GFL_CLGRP_PLTT_RegisterEx( p_handle[ARC_TYPE_MEDAL_ICON],
        NARC_medalicon_gra_medal_icon_NCLR , CLSYS_DRAW_MAX ,  0 , 0 , MEDALOBJ_PALETTENUM_MEDAL , heapID );
  
    obj_res[ OBJRESID_MEDAL_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_MEDAL_ICON],
        NARC_medalicon_gra_medal_icon_NCGR , FALSE, CLSYS_DRAW_MAX, heapID );

    // ======================================
    // COMMONUIOBJ
    // ======================================
    obj_res[ OBJRESID_COMMONUI_CELLANM ] = GFL_CLGRP_CELLANIM_Register( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_bar_button_128k_NCER, NARC_app_menu_common_bar_button_128k_NANR , heapID );

    obj_res[ OBJRESID_COMMONUI_PLTT ]    = GFL_CLGRP_PLTT_RegisterEx( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_bar_button_NCLR ,
        CLSYS_DRAW_MAIN ,  32 * MEDALOBJ_PALETTENUM_MEDAL, 0 ,
        MEDALOBJ_PALETTENUM_COMMON_UI , heapID );	
  
    obj_res[ OBJRESID_COMMONUI_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_bar_button_128k_NCGR , FALSE, CLSYS_DRAW_MAIN, heapID );

    // ======================================
    // アーカイブハンドルの開放
    // ======================================
    for( i = 0 ; i < ARC_TYPE_MAX ; i++ )
    {
      GFL_ARC_CloseDataHandle( p_handle[i] );
    }
  }

  // ======================================
  //CLWK作成
  // ======================================
  {
    int i;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    // ======================================
    // 下画面
    // ======================================
    // メダルCLWK作成
    {
      MEDALDATA arc_data;

      // イチオシメダルが登録されていないときはダミーデータを読み込み
      ARCHANDLE	*p_handle	= GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , heapID );
      if( p_wk->is_push_medal == TRUE )
      {
        GFL_ARC_LoadDataByHandle( p_handle , p_wk->p_now_medal_info->medal_id , &arc_data);
      }
      else
      {
        GFL_ARC_LoadDataByHandle( p_handle , 0 , &arc_data);
      }

      // 位置データの書き込み
      cldata.pos_x = OBJ_POS_MEDAL_X;
      cldata.pos_y = OBJ_POS_MEDAL_Y;

      // プライオリティー
      cldata.bgpri = MEDALVIEWER_OBJ_PRIORITY_MEDAL;
      cldata.softpri = 0;

      // ワークの作成
      p_wk->p_clwk[OBJ_CLWK_MEDAL]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_MEDAL_CGR],
          p_wk->obj_res[OBJRESID_MEDAL_PLTT],
          p_wk->obj_res[OBJRESID_MEDAL_CELLANM],
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[OBJ_CLWK_MEDAL] , arc_data.graphic_type );
      GFL_ARC_CloseDataHandle( p_handle );

      // イチオシメダルが登録されていないときは表示しない
      if( p_wk->is_push_medal == TRUE )
      {
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[OBJ_CLWK_MEDAL], TRUE );
      }
      else
      {
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[OBJ_CLWK_MEDAL], FALSE );
      }
    }

    // 戻るボタン
    {
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.bgpri   = MEDALVIEWER_OBJ_PRIORITY_UI;
      cldata.softpri = 0;

      // 位置データの書き込み
      cldata.pos_x = OBJ_POS_RETURN_X;
      cldata.pos_y = OBJ_POS_RETURN_Y;

      // ワークの作成
      p_wk->p_clwk[OBJ_CLWK_RETURN]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_COMMONUI_CGR],
          p_wk->obj_res[OBJRESID_COMMONUI_PLTT],
          p_wk->obj_res[OBJRESID_COMMONUI_CELLANM],
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[OBJ_CLWK_RETURN] , APP_COMMON_BARICON_RETURN );
    }

    // キャンセルボタン
    {
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.bgpri   = MEDALVIEWER_OBJ_PRIORITY_UI;
      cldata.softpri = 0;

      cldata.pos_x = OBJ_POS_EXIT_X;
      cldata.pos_y = OBJ_POS_EXIT_Y;

      // ワークの作成
      p_wk->p_clwk[OBJ_CLWK_EXIT]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_COMMONUI_CGR],
          p_wk->obj_res[OBJRESID_COMMONUI_PLTT],
          p_wk->obj_res[OBJRESID_COMMONUI_CELLANM],
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[OBJ_CLWK_EXIT] , APP_COMMON_BARICON_EXIT );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[OBJ_CLWK_EXIT], TRUE );
     
      // 直接呼出しのときは終了ボタンを非表示にする
      if( p_wk->p_init_param->call_mode == MEDALVIEWER_CALL_SUB )
      {
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[OBJ_CLWK_EXIT], TRUE );
      }
      else
      {
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[OBJ_CLWK_EXIT], FALSE );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJデータの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_OBJDelete( MEDALVIEWER_WK *p_wk )
{
  int i;

	//CLWK取得
	{
		for( i = 0; i < OBJ_CLWK_MAX; i++ )
		{	
			if( p_wk->p_clwk[i] )
			{	
				GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
			}
		}
	}

  //リソース破棄
  {	
    // メダルOBJ
    {
      GFL_CLGRP_CELLANIM_Release( p_wk->obj_res[OBJRESID_MEDAL_CELLANM] );
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_MEDAL_CGR] );
      GFL_CLGRP_PLTT_Release( p_wk->obj_res[OBJRESID_MEDAL_PLTT] );	
    }
    // COMMONUIOBJ
    {
      GFL_CLGRP_CELLANIM_Release( p_wk->obj_res[OBJRESID_COMMONUI_CELLANM] );
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_COMMONUI_CGR] );
      GFL_CLGRP_PLTT_Release( p_wk->obj_res[OBJRESID_COMMONUI_PLTT] );	
    }
  }


	//システム破棄
	{
    if( p_wk->p_clunit )
    {	
      GFL_CLACT_UNIT_Delete( p_wk->p_clunit );
    }
  }

	GFL_CLACT_SYS_Delete();
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックVブランクタスク
 *
 *	@param	*p_tcb
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDALVIEWER_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs )
{
  // セルの更新
  GFL_CLACT_SYS_VBlankFunc();
}


