//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_list_main.c
 *  @brief  生息地リスト：リスト画面（メイン）
 *	@author	Masayuki Onoue
 *	@date		2011.08.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/main.h"            // HEAPID
#include "habitat_list.h"           
#include "habitat_list_main.h"
#include "msg/msg_seisoku.h"        // for msg_seisoku_*
#include "habitat_list_main_def.h"  // 定数定義
#include "field/zonedata.h"         // for ZONEDATA_GetPlaceNameID

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
typedef void (*MainFunc)(HABITAT_LIST_WK*);


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

// メインシーケンス---------------------------------
static void MAINSEQ_AppInit( HABITAT_LIST_WK *p_wk );
static void MAINSEQ_FadeIn( HABITAT_LIST_WK *p_wk );
static void MAINSEQ_KeyInput( HABITAT_LIST_WK *p_wk );
static void MAINSEQ_AppExit( HABITAT_LIST_WK *p_wk );
static void MAINSEQ_PageMove( HABITAT_LIST_WK *p_wk );

// 文字描画 ---------------------------------
static PRINTSYS_LSB GetMsgFontColor( void );
static void DrawTitleMsg( HABITAT_LIST_WK *p_wk );
static void DrawButtonMsg( HABITAT_LIST_WK *p_wk );

// データ処理 ---------------------------------
static void CreateAppData( HABITAT_LIST_WK *p_wk );
static void InitModeButton( HABITAT_LIST_WK *p_wk );

//--------------------------------------------------
// ID定義
typedef enum _MAIN_SEQ{
  MAIN_SEQ_APPINIT,
  MAIN_SEQ_FADEIN,
  MAIN_SEQ_KEY_INPUT,
  MAIN_SEQ_PAGE_MOVE,
  MAIN_SEQ_APPEXIT,
  MAIN_SEQ_NULL,
}MAIN_SEQ;

//--------------------------------------------------
// 関数定義（ID定義と一致している必要がある）
static const MainFunc MainSeq[] = {
  MAINSEQ_AppInit,            // アプリケーションの初期化
  MAINSEQ_FadeIn,             // フェードイン
  MAINSEQ_KeyInput,           // メインキー処理
  MAINSEQ_PageMove,           // ページ移動
  MAINSEQ_AppExit,            // アプリケーションの終了処理
  NULL,                       // 何も処理をしない
};

//--------------------------------------------------
// リスト処理管理
static const BOOL ListEnable[] = {
  FALSE,  // アプリケーションの初期化
  FALSE,  // フェードイン
  TRUE,   // メインキー処理
  FALSE,   // ページ移動
  FALSE,  // アプリケーションの終了処理
  FALSE,  // 何も処理をしない
};

//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理呼出
 *
 *	@param	*p_wk 
 *
 *	@retval TRUEでメイン継続
 */
//-----------------------------------------------------------------------------
BOOL HABITAT_LIST_Main( HABITAT_LIST_WK *p_wk )
{
  MainFunc main = MainSeq[ p_wk->main_seq ];

  // リスト更新処理
  if( ListEnable[p_wk->main_seq] == TRUE )
  {
    u32 key_state;
    key_state = HABITAT_LIST_GRA_ListMain( &p_wk->gra_sys );
    HABITAT_LIST_SetListKeyState( p_wk , key_state );
  }

  // メイン処理の実行
  if( main )
  {
    main( p_wk );
  }


#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    return FALSE;
  }
#endif

  // シーケンスの状態を監視
  if( p_wk->main_status != HABITAT_LIST_MAIN_CONTINUE )
  {
    return FALSE;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メインシーケンスの切替
 *
 *	@param	*p_wk 
 *	@param	seq_id 
 *	@param	param 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextSeq( HABITAT_LIST_WK *p_wk ,MAIN_SEQ seq_id ,SEQ_CHANGE_PARAM param )
{
  p_wk->main_seq  = seq_id;
  p_wk->seq_param = param;
  p_wk->sub_seq   = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードインシーケンス
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_FadeIn( HABITAT_LIST_WK *p_wk )
{
  switch(p_wk->sub_seq)
  {
  case 0:
    {
      // 文字描画終了を待ってフェードを呼び出す
      if( HABITAT_LIST_GRA_CheckPrintUtilStatus( &p_wk->gra_sys ) )
      {
        // オープニングフェード呼出
        GX_SetMasterBrightness( 0 );
        GXS_SetMasterBrightness( 0 );
        HABITAT_LIST_GRA_CallOpeningFade( &p_wk->gra_sys );
        p_wk->sub_seq++;
      }
    }
    break;
  case 1:
    {
      SetNextSeq( p_wk, MAIN_SEQ_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
      break;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アプリケーションの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_AppInit( HABITAT_LIST_WK *p_wk )
{
  // フォントウィンドウのクリア
  HABITAT_LIST_GRA_BmpFil( PRINT_UTIL_INFO_WIN , 0 ,&p_wk->gra_sys );
  HABITAT_LIST_GRA_BmpFil( PRINT_UTIL_TITLE_WIN , 0 ,&p_wk->gra_sys );
  
  // 通信アイコンを表示
  GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_ZUKAN_HABITAT_LIST );

  // データの取得
  CreateAppData( p_wk );

  // タイトルメッセージの書き込み
  DrawTitleMsg( p_wk );

  // ボタン描画
  InitModeButton( p_wk );

  // フェードシーケンスに移行
  SetNextSeq( p_wk ,MAIN_SEQ_FADEIN ,SEQ_CHANGE_PARAM_NONE );

  // メインシーケンスの状態を継続に設定
  p_wk->main_status = HABITAT_LIST_MAIN_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アプリケーションの終了処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
// @todo 地名表示バーとメニューバーのスクロールアウト、フェード処理を入れる
static void MAINSEQ_AppExit( HABITAT_LIST_WK *p_wk )
{
  //p_wk->main_status = HABITAT_LIST_MAIN_EXIT;
}

//----------------------------------------------------------------------------
/**
 *	@brief  キー入力
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_KeyInput( HABITAT_LIST_WK *p_wk )
{
  switch( p_wk->list_key_state )
  {
	case FRAMELIST_RET_NONE:				// 動作なし
    // キー入力
    break;
	case FRAMELIST_RET_CURSOR_ON:		// カーソル表示
		break;

	case FRAMELIST_RET_MOVE:				// カーソル移動
	case FRAMELIST_RET_SCROLL:			// スクロール通常
	case FRAMELIST_RET_RAIL:				// レールスクロール
	case FRAMELIST_RET_SLIDE:				// スライドスクロール
		//ZKNLISTOBJ_SetListPageArrowAnime( wk, FALSE );
		break;

	case FRAMELIST_RET_PAGE_UP:			// １ページ上へ
//		ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
//		wk->buttonID = ZKNLISTOBJ_IDX_TB_LEFT;
//		wk->subSeq = 0;
//		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_DOWN:		// １ページ下へ
//		ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
//		wk->buttonID = ZKNLISTOBJ_IDX_TB_RIGHT;
//		wk->subSeq = 0;
//		seq = MAINSEQ_PAGE_MOVE;
		break;

  case FRAMELIST_RET_PAGE_UP_NONE:		// ページ数が足りなくて１ページ上にスクロールできなかった
    //		return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON, MAINSEQ_MAIN );
    break;

  case FRAMELIST_RET_PAGE_DOWN_NONE:	// ページ数が足りなくて１ページ下にスクロールできなかった
//		return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON, MAINSEQ_MAIN );
    break;

	case FRAMELIST_RET_JUMP_TOP:		// リスト最上部へジャンプ
	case FRAMELIST_RET_JUMP_BOTTOM:	// リスト最下部へジャンプ
    // 更新が無かったものを全て消す
    HABITAT_LIST_GRA_RecoverDrawFlag( &p_wk->gra_sys );
    SetNextSeq( p_wk, MAIN_SEQ_PAGE_MOVE ,SEQ_CHANGE_PARAM_NONE);
		break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ページ移動
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_PageMove( HABITAT_LIST_WK *p_wk )
{
  {
    SetNextSeq( p_wk, MAIN_SEQ_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リストのキー状態を設定する
 *
 *	@param	*p_wk 
 *	@param	key_state 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_SetListKeyState( HABITAT_LIST_WK *p_wk ,u32 key_state )
{
  p_wk->list_key_state = key_state;
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイトルメッセージを書き込む
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawTitleMsg( HABITAT_LIST_WK *p_wk )
{
  STRBUF       *str;
  PRINTSYS_LSB  font_colro = GetMsgFontColor();

  // メッセージの書き込み
  str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_list_title );

  // メッセージの書き込み
  HABITAT_LIST_GRA_DrawStr( PRINT_UTIL_TITLE_WIN ,str ,&p_wk->gra_sys ,
      TITLE_MSG_POS_X , TITLE_MSG_POS_Y,
      TITLE_MSG_W,TITLE_MSG_H,
      font_colro );

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタンメッセージを描画する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawButtonMsg( HABITAT_LIST_WK *p_wk )
{
  STRBUF       *str;
  PRINTSYS_LSB  font_colro = GetMsgFontColor();

  // 生息地ボタン（常に一番下）
  {
    // メッセージの書き込み
    str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_list_title );

    // メッセージの書き込み
    HABITAT_LIST_GRA_DrawStr( PRINT_UTIL_INFO_WIN ,str ,&p_wk->gra_sys ,
        BUTTON_03_POS_X , BUTTON_03_POS_Y,
        BUTTON_03_MSG_W,BUTTON_03_MSG_H,
        font_colro );

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );
  }

  // 全国図鑑が閲覧可能かで表示を変える
  if( p_wk->is_zenkoku == TRUE )
  {
    // イッシュ
    {
      str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_list_local );

      // メッセージの書き込み
      HABITAT_LIST_GRA_DrawStr( PRINT_UTIL_INFO_WIN ,str ,&p_wk->gra_sys ,
          BUTTON_01_POS_X , BUTTON_01_POS_Y,
          BUTTON_01_MSG_W,BUTTON_01_MSG_H,
          font_colro );

      // バッファーの消去 
      GFL_STR_DeleteBuffer( str );
    }

    // 全国
    {
      str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_list_zenkoku );

      // メッセージの書き込み
      HABITAT_LIST_GRA_DrawStr( PRINT_UTIL_INFO_WIN ,str ,&p_wk->gra_sys ,
          BUTTON_02_POS_X , BUTTON_02_POS_Y,
          BUTTON_02_MSG_W,BUTTON_02_MSG_H,
          font_colro );

      // バッファーの消去 
      GFL_STR_DeleteBuffer( str );
    }

  }
  else
  {
    // イッシュ
    GFL_CLWK* p_clwk;
    str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_list_local );

    // メッセージの書き込み
    HABITAT_LIST_GRA_DrawStr( PRINT_UTIL_INFO_WIN ,str ,&p_wk->gra_sys ,
        BUTTON_02_POS_X , BUTTON_02_POS_Y,
        BUTTON_02_MSG_W,BUTTON_02_MSG_H,
        font_colro );

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );

    // 一番上のボタンを消す
    p_clwk= HABITAT_LIST_GRA_GetComClWk( &p_wk->gra_sys ,MODE_BUTTON_01_CLWKID);
    GFL_CLACT_WK_SetDrawEnable( p_clwk, FALSE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フォントカラーを取得する
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static PRINTSYS_LSB GetMsgFontColor( void )
{
  return PRINTSYS_LSB_Make(1,2,3);
}

//----------------------------------------------------------------------------
/**
 *	@brief  アプリケーションに必要なデータを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CreateAppData( HABITAT_LIST_WK *p_wk )
{
  GAMEDATA *p_gdata = p_wk->p_init_param->p_gdata;
  ZUKAN_SAVEDATA* p_zkn_sv = GAMEDATA_GetZukanSave( p_gdata );
  p_wk->is_zenkoku =  ZUKANSAVE_GetZenkokuZukanFlag( p_zkn_sv );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void InitModeButton( HABITAT_LIST_WK *p_wk )
{
  DrawButtonMsg( p_wk );
}

