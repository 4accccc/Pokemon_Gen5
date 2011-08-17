//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_zone_main.c
 *  @brief   生息地リスト：ゾーン詳細（メイン）
 *	@author	Masayuki Onoue
 *	@date		2011.08.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/main.h"            // HEAPID
#include "habitat_zone.h"           
#include "habitat_zone_main.h"
#include "msg/msg_seisoku.h"        // for msg_seisoku_*
#include "habitat_zone_main_def.h"  // 定数定義
#include "field/zonedata.h"         // for ZONEDATA_GetPlaceNameID

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
typedef void (*MainFunc)(HABITAT_ZONE_WK*);


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
static void MAINSEQ_AppInit( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_FadeIn( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_TopKeyInput( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_ChangeSeason( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_ChangeZone( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_ChangeLandType( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_PokeSelectKeyInput( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_MovePokeCursor( HABITAT_ZONE_WK *p_wk );

//--------------------------------------------------
// ID定義
typedef enum _MAIN_SEQ{
  MAIN_SEQ_APPINIT,
  MAIN_SEQ_FADEIN,
  MAIN_SEQ_TOP_KEY_INPUT,
  MAIN_SEQ_CHANGE_SEASON,
  MAIN_SEQ_CHANGE_ZONE,
  MAIN_SEQ_CHANGE_LAND_TYPE,
  MAIN_SEQ_POKESELECT_KEY_INPUT,
  MAIN_SEQ_MOVE_POKE_CURSOR,
  MAIN_SEQ_NULL,
}MAIN_SEQ;

//--------------------------------------------------
// 関数定義（ID定義と一致している必要がある）
static const MainFunc MainSeq[] = {
  MAINSEQ_AppInit,            // アプリケーションの初期化
  MAINSEQ_FadeIn,             // フェードイン
  MAINSEQ_TopKeyInput,        // トップキー
  MAINSEQ_ChangeSeason,       // 季節切替
  MAINSEQ_ChangeZone,         // ゾーン切替
  MAINSEQ_ChangeLandType,     // 地形タイプ切替
  MAINSEQ_PokeSelectKeyInput, // ポケモン選択キー入力
  MAINSEQ_MovePokeCursor,     // ポケモン選択カーソルを移動させる
  NULL,                       // 何も処理をしない
};

// メッセージ---------------------------------------
static PRINTSYS_LSB GetMedalFontColor( void );
static void DrawLandMsg( HABITAT_ZONE_WK *p_wk );
static void DrawSeasonMsg( HABITAT_ZONE_WK *p_wk );
static void DrawExplanationMsg( HABITAT_ZONE_WK *p_wk );
static void ClearExplanationMsg( HABITAT_ZONE_WK *p_wk );
static void DrawMapNameMsg( HABITAT_ZONE_WK *p_wk );

// データ処理系---------------------------------------
static HABITAT_ZONE_DATA* GetNowListData( HABITAT_ZONE_WK *p_wk );
static void SetCursorstatus( HABITAT_ZONE_WK *p_wk ,s32 index ,BOOL is_active );
static void MakeZoneCommonData( HABITAT_ZONE_WK *p_wk );
static void SkipLandType( HABITAT_ZONE_WK *p_wk ,BOOL is_skip_r );

// ポケモンアイコン系---------------------------------------
static void MakePokeIconList( HABITAT_ZONE_WK *p_wk );

// シーケンス制御 ----------------------------------------------
static void SetNextSeq( HABITAT_ZONE_WK *p_wk ,MAIN_SEQ seq_id ,SEQ_CHANGE_PARAM param );

// 画面更新系
static void RefreshLandTypeIcon( HABITAT_ZONE_WK *p_wk );
static void RefreshLandPokeIcon( HABITAT_ZONE_WK *p_wk );
static void SetPokeSelectButton( HABITAT_ZONE_WK *p_wk );
static void SetZoneSelectButton( HABITAT_ZONE_WK *p_wk );
static void RefreshMapWin( HABITAT_ZONE_WK *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理呼出
 *
 *	@param	*p_wk 
 *
 *	@retval TRUEでメイン継続
 */
//-----------------------------------------------------------------------------
BOOL HABITAT_ZONE_Main( HABITAT_ZONE_WK *p_wk )
{
  MainFunc main = MainSeq[ p_wk->main_seq ];

  if( main )
  {
    main( p_wk );
  }

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
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
static void SetNextSeq( HABITAT_ZONE_WK *p_wk ,MAIN_SEQ seq_id ,SEQ_CHANGE_PARAM param )
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
static void MAINSEQ_FadeIn( HABITAT_ZONE_WK *p_wk )
{
  switch(p_wk->sub_seq)
  {
  case 0:
    {
      // 文字描画終了を待ってフェードを呼び出す
      if( HABITAT_ZONE_GRA_CheckPrintUtilStatus( &p_wk->gra_sys ) )
      {

        // オープニングフェード呼出
        GX_SetMasterBrightness( 0 );
        GXS_SetMasterBrightness( 0 );
        HABITAT_ZONE_GRA_CallOpeningFade( &p_wk->gra_sys );
        p_wk->sub_seq++;
      }
    }
    break;
  case 1:
    {
      SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
      break;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  トップキー入力処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_TopKeyInput( HABITAT_ZONE_WK *p_wk )
{
  s32 key_trg = GFL_UI_KEY_GetTrg();

  // 四季切替
  if( key_trg & PAD_BUTTON_L )
  {
    ONOUE_Printf("季節切替L\n");
    SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_SEASON ,SEQ_CHANGE_PARAM_L );
  }
  else if( key_trg & PAD_BUTTON_R )
  {
    ONOUE_Printf("季節切替R\n");
    SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_SEASON ,SEQ_CHANGE_PARAM_R );
  }
  else if( key_trg & PAD_KEY_LEFT )
  {
    ONOUE_Printf("場所タイプ切替L\n");
    SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_LAND_TYPE ,SEQ_CHANGE_PARAM_L );
  }
  else if( key_trg & PAD_KEY_RIGHT )
  {
    ONOUE_Printf("場所タイプ切替R\n");
    SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_LAND_TYPE ,SEQ_CHANGE_PARAM_R );
  }
  else if( key_trg & PAD_KEY_UP )
  {
    ONOUE_Printf("ゾーン切替UP\n");
    SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_ZONE ,SEQ_CHANGE_PARAM_UP );
  }
  else if( key_trg & PAD_KEY_DOWN )
  {
    ONOUE_Printf("ゾーン切替DOWN\n");
    SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_ZONE ,SEQ_CHANGE_PARAM_DOWN );
  }
  else if( key_trg & PAD_BUTTON_A )
  {
    SetNextSeq( p_wk ,MAIN_SEQ_POKESELECT_KEY_INPUT ,SEQ_CHANGE_PARAM_A );
    ONOUE_Printf("ポケモン選択画面へ\n");
  }
  else if( key_trg & PAD_BUTTON_B )
  {
    // @todo 生息地リストへ飛ばす
    ONOUE_Printf("生息地リスト画面へ\n");
  }
  else if( key_trg & PAD_BUTTON_X )
  {
    ONOUE_Printf("フィールドへ戻る\n");
  }
  else if( key_trg & PAD_BUTTON_Y )
  {
    ONOUE_Printf("ショートカット登録ON/OFF\n");
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン選択キー入力
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_PokeSelectKeyInput( HABITAT_ZONE_WK *p_wk )
{
  switch(p_wk->sub_seq)
  {
  // 初回選択状態反映、メニューの切替
  case 0:
    {
      // ポケモン選択時用ボタン表示に切替
      SetPokeSelectButton( p_wk );

      // ボタンの有効状態書き換え
      if( p_wk->seq_param == SEQ_CHANGE_PARAM_A )
      {
        // カーソルはアクティブ状態に移行
        ONOUE_Printf("ポケモン選択：カーソル有効化処理実行\n");
        SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_NONE );
        return; 
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_TOUCH )
      {
        // @todo 処理を確認(カーソルは選択されないまま見るが出る？)
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_NONE )
      {
        // カーソル移動から戻ってきたときは何もしない
      }
      else
      {
        GF_ASSERT(0);
      }
      p_wk->sub_seq++;
    }
    break;

  // キー入力処理
  case 1:
    {
      s32 key_trg = GFL_UI_KEY_GetTrg();

      // 四季切替
      if( key_trg & PAD_KEY_LEFT )
      {
        ONOUE_Printf("ポケモン選択：リスト左移動\n");
        SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_L );
      }
      else if( key_trg & PAD_KEY_RIGHT )
      {
        ONOUE_Printf("ポケモン選択：リスト右移動\n");
        SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_R );
      }
      else if( key_trg & PAD_KEY_UP )
      {
        ONOUE_Printf("ポケモン選択：リスト上移動\n");
        SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_UP );
      }
      else if( key_trg & PAD_KEY_DOWN )
      {
        ONOUE_Printf("ポケモン選択：リスト下移動\n");
        SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_DOWN );
      }
      else if( key_trg & PAD_BUTTON_B )
      {
        ONOUE_Printf("ポケモン選択：ゾーン詳細画面に戻る\n");
        // 現在のカーソル位置をOFFに
        SetCursorstatus( p_wk ,p_wk->select_index ,FALSE );
        // ボタン表示を切り替える
        SetZoneSelectButton( p_wk );
        // トップキー入力処理に移動
        SetNextSeq( p_wk ,MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_R );
      }
    }
    break;
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
static void MAINSEQ_AppInit( HABITAT_ZONE_WK *p_wk )
{
  // フォントウィンドウのクリア
  HABITAT_ZONE_GRA_BmpFil( PRINT_UTIL_INFO_WIN , 0 ,&p_wk->gra_sys );
  
  // 通信アイコンを表示
  GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_ZUKAN_HABITAT_ZONE );
  
  // 初期化パラメータからデータを取得
  p_wk->zone_data_cnt   = p_wk->p_init_param->zone_cnt;
  p_wk->season          = p_wk->p_init_param->start_season;
  p_wk->land_type       = p_wk->p_init_param->start_land_type;
  p_wk->zone_data_index = p_wk->p_init_param->start_zone_index;

  // リストデータを解析し、必要なデータを作成する
  MakeZoneCommonData( p_wk );

  // 四季メッセージ書き込み
  DrawSeasonMsg( p_wk );
 
  // 地形アイコン更新
  RefreshLandTypeIcon( p_wk );

  // ポケモンアイコンリストを作成する
  RefreshLandPokeIcon( p_wk );

  // マップウィンドウ（上画面の更新）
  RefreshMapWin( p_wk );

  // フェードシーケンスに移行
  SetNextSeq( p_wk ,MAIN_SEQ_FADEIN ,SEQ_CHANGE_PARAM_NONE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  下画面メダルタイトル部分のフォントカラーを取得する
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static PRINTSYS_LSB GetMedalFontColor( void )
{
  return PRINTSYS_LSB_Make(1,2,3);
}

//----------------------------------------------------------------------------
/**
 *	@brief  季節を切り替える
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_ChangeSeason( HABITAT_ZONE_WK *p_wk )
{
  GFL_CLWK *p_clwk;

  // 移動方向にあわせて、CLWKを取得しておく
  if( p_wk->seq_param == SEQ_CHANGE_PARAM_L )
  {
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_L );
  }
  else if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
  {
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_R );
  }
  else
  {
    GF_ASSERT_MSG(0,"左右以外の切替パラメータを受け付けません。");
  }

  switch(p_wk->sub_seq)
  {
  // 左右ボタンアニメーション再生
  case 0:
    {
      // アニメーションシーケンスを決定、季節の切替
      if( p_wk->seq_param == SEQ_CHANGE_PARAM_L )
      {
        GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_L_ANM);
        p_wk->season--;
        if( p_wk->season < 0 )
        {
          p_wk->season = PMSEASON_WINTER;
        }
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
      {
        GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_R_ANM);
        p_wk->season++;
        if( p_wk->season >= PMSEASON_TOTAL )
        {
          p_wk->season = PMSEASON_SPRING;
        }
      }
      // アニメーションを初期化して、再生
      GFL_CLACT_WK_SetAnmIndex( p_clwk , 0);
      GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,TRUE );

      // 四季メッセージ書き込み
      DrawSeasonMsg( p_wk );
 
      // 地形アイコン更新
      RefreshLandTypeIcon( p_wk );

      // ポケモンアイコンリストを作成する
      RefreshLandPokeIcon( p_wk );

      // シーケンスを進める
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      // アニメーション終了待ち
      if( GFL_CLACT_WK_CheckAnmActive( p_clwk ) == FALSE )
      {
        // アニメーションシーケンスを決定
        if( p_wk->seq_param == SEQ_CHANGE_PARAM_L )
        {
          GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_L );
        }
        else if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
        {
          GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_R );
        }
        // アニメーションの状態をリセット
        GFL_CLACT_WK_SetAnmIndex( p_clwk , 0);
        GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,FALSE );
      
        // キー入力状態に切替
        SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
      }
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンを切り替える
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_ChangeZone( HABITAT_ZONE_WK *p_wk )
{
  GFL_CLWK *p_clwk;
  s32      default_anm_seq;
  s32      anm_seq;
  // アニメーションシーケンスを決定、季節の切替
  if( p_wk->seq_param == SEQ_CHANGE_PARAM_UP )
  {
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_UP_ARROW );
    anm_seq = APP_COMMON_BARICON_CURSOR_UP_ON;
    default_anm_seq = APP_COMMON_BARICON_CURSOR_UP;
  }
  else if( p_wk->seq_param == SEQ_CHANGE_PARAM_DOWN )
  {
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_DOWN_ARROW );
    anm_seq = APP_COMMON_BARICON_CURSOR_DOWN_ON;
    default_anm_seq = APP_COMMON_BARICON_CURSOR_DOWN;
  }
  else
  {
    GF_ASSERT_MSG(0,"上下以外の切替パラメータを受付ません\n");
  }

  switch(p_wk->sub_seq)
  {
  // メッセージ書き換え
  case 0:
    {
      // アニメーションシーケンスを決定、季節の切替
      if( p_wk->seq_param == SEQ_CHANGE_PARAM_UP )
      {
        p_wk->zone_data_index--;
        if( p_wk->zone_data_index < 0 )
        {
          p_wk->zone_data_index = p_wk->zone_data_cnt - 1;
        }
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_DOWN )
      {
        p_wk->zone_data_index++;
        if( p_wk->zone_data_index >= p_wk->zone_data_cnt )
        {
          p_wk->zone_data_index = 0;
        }
      }

      // アニメーション開始
      GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
      GFL_CLACT_WK_SetAnmSeq( p_clwk , anm_seq );
      GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,TRUE );

      // シーケンスを進める
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      // アニメーション終了待ち
      if( GFL_CLACT_WK_CheckAnmActive( p_clwk ) == FALSE )
      {
        // アニメーション停止
        GFL_CLACT_WK_SetAnmSeq( p_clwk , default_anm_seq );
        GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,TRUE );

        // リストデータを解析し、必要なデータを作成する
        MakeZoneCommonData( p_wk );

        // 地形アイコン更新
        RefreshLandTypeIcon( p_wk );

        // ポケモンアイコンリストを作成する
        RefreshLandPokeIcon( p_wk );

        // マップウィンドウ（上画面の更新）
        RefreshMapWin( p_wk );
        
        // シーケンスを進める
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      // 文字描画終了を待ってキー入力状態に以降
      if( HABITAT_ZONE_GRA_CheckPrintUtilStatus( &p_wk->gra_sys ) )
      {
        // キー入力状態に切替
        SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
      }
    }
      break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  地形タイプ切替
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_ChangeLandType( HABITAT_ZONE_WK *p_wk )
{
  s32 i;
  GFL_CLWK *p_clwk;
  // タイプごとのアニメーションシーケンス
  s32 anm_seq[] =
  {
    OBJ_ANM_SEQ_BUTTON_LAND_ANM,
    OBJ_ANM_SEQ_BUTTON_WATERSIDE_ANM,
    OBJ_ANM_SEQ_BUTTON_FISHING_ANM,
  };

 
  switch(p_wk->sub_seq)
  {
  case 0:
    {
      // アニメーションシーケンスを決定、季節の切替
      if( p_wk->seq_param == SEQ_CHANGE_PARAM_L )
      {
        p_wk->land_type--;
        if( p_wk->land_type < 0 )
        {
          p_wk->land_type = HABITAT_ZONE_APPEAR_FISHING;
        }
        // 地形タイプのポケモンが存在しないときはスキップ
        if( p_wk->land_appear[p_wk->season][p_wk->land_type] == FALSE )
        {
          ONOUE_Printf("左スキップ\n");
          SkipLandType( p_wk ,FALSE);
        }
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
      {
        p_wk->land_type++;
        if( p_wk->land_type >= HABITAT_ZONE_APPEAR_MAX )
        {
          p_wk->land_type = HABITAT_ZONE_APPEAR_LAND;
        }
        // 地形タイプのポケモンが存在しないときはスキップ
        if( p_wk->land_appear[p_wk->season][p_wk->land_type] == FALSE )
        {
          ONOUE_Printf("右スキップ\n");
          SkipLandType( p_wk ,TRUE);
        }
      }
 
      // 地形アイコン更新
      RefreshLandTypeIcon( p_wk );

      // アニメーション開始
      p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys
                                  ,OBJ_WK_BUTTON_LAND + p_wk->land_type);
      GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
      GFL_CLACT_WK_SetAnmSeq( p_clwk , anm_seq[p_wk->land_type] );
      GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,TRUE );

      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      // アニメーション終了待ち
      p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys
                                  ,OBJ_WK_BUTTON_LAND + p_wk->land_type);
      if( GFL_CLACT_WK_CheckAnmActive( p_clwk ) == FALSE )
      {

        // ポケモンアイコンリストを作成する
        RefreshLandPokeIcon( p_wk );

        GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,FALSE );
        GFL_CLACT_WK_SetAnmSeq( p_clwk , anm_seq[p_wk->land_type] );
        GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      // 文字描画終了を待ってキー入力状態に以降
      if( HABITAT_ZONE_GRA_CheckPrintUtilStatus( &p_wk->gra_sys ) )
      {
        // キー入力状態に切替
        SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
      }
      else
      {
        ONOUE_Printf("描画待ち\n");
      }
    }
    break;
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  存在しない地形タイプをスキップする
 *
 *	@param	*p_wk 
 *	@param	is_skip_r TRUEの時は右方向にスキップする
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SkipLandType( HABITAT_ZONE_WK *p_wk ,BOOL is_skip_r )
{
  s32 i;

  if( is_skip_r == TRUE )
  {
    for( i = 0; i < HABITAT_ZONE_APPEAR_MAX; i++ )
    {
      p_wk->land_type++;
      if( p_wk->land_type >= HABITAT_ZONE_APPEAR_MAX )
      {
        p_wk->land_type = HABITAT_ZONE_APPEAR_LAND;
      }
      if( p_wk->land_appear[p_wk->season][HABITAT_ZONE_APPEAR_LAND] == TRUE )
      {
        break;
      }
    }
  }
  else
  {
    for( i = 0; i < HABITAT_ZONE_APPEAR_MAX; i++ )
    {
      p_wk->land_type--;
      if( p_wk->land_type < 0 )
      {
        p_wk->land_type = HABITAT_ZONE_APPEAR_FISHING;
      }
      if( p_wk->land_appear[p_wk->season][HABITAT_ZONE_APPEAR_LAND] == TRUE )
      {
        break;
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン選択カーソルを移動させる
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_MovePokeCursor( HABITAT_ZONE_WK *p_wk )
{
  // @todo 見た目を確認して、カーソルが動かないときは何もしないか決める
  switch( p_wk->sub_seq )
  {
  case 0:
    {
      s32 x = 0;
      s32 y = 0;
      s32 list_h = 0; 

      // 現在のカーソル位置をOFFに
      if( p_wk->is_cursor_active == TRUE )
      {
        SetCursorstatus( p_wk ,p_wk->select_index ,FALSE );
      }

      // 現在のリスト位置を計算
      if( p_wk->select_index != 0 )
      {
        x = p_wk->select_index%POKEICON_LINE;
        y = p_wk->select_index/POKEICON_LINE;
      }
      ONOUE_Printf("1:x = %d,y=%d\n",x,y);

      // リストの高さを計算
      list_h = (p_wk->list_size-1)/POKEICON_LINE;

      // カーソル位置を変更して終了
      if( p_wk->seq_param == SEQ_CHANGE_PARAM_L )
      {
        x --;
        // 右端に移動チェック
        if( x < 0 )
        {
          x = POKEICON_LINE - 1;
        }
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
      {
        x++;
        // 右端に移動チェック
        if( x > POKEICON_LINE - 1 )
        {
          x = 0;
        }
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_UP )
      {
        y--;
        // 下の行に回りこみ
        if( y < 0 )
        {
          y = list_h;
        }
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_DOWN )
      {
        y++;
        // 上の行に回りこみ
        if( y > list_h )
        {
          y = 0;
        }
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_NONE )
      {
        ONOUE_Printf("カーソルの有効化\n");
      }

      // x,y位置をリスト位置に変換
      ONOUE_Printf("2:x = %d,y=%d\n",x,y);
      p_wk->select_index = x + y*POKEICON_LINE;

      // リスト範囲外チェック
      if( p_wk->select_index >= p_wk->list_size )
      {
        if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
        {
          // 右移動で範囲外に出た時だけ、リストの左端へカーソルを移動させる
          x = 0;
          p_wk->select_index = x + y*POKEICON_LINE;
        }
        else
        {
          if( y  <= 0 )
          {
            // リストの範囲外の場合は一番最後を選択
            p_wk->select_index = p_wk->list_size -1;
          }
          else
          {
            // 上に移動できる場合は、一つ上の枠をせんたくする
            y--;
            p_wk->select_index = x + y*POKEICON_LINE;
          }
        }
      }

      // 新しいカーソル位置をONに
      SetCursorstatus( p_wk ,p_wk->select_index ,TRUE );
      
      // サブシーケンスを進める
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      // @todo SE待ちを行う
      SetNextSeq( p_wk, MAIN_SEQ_POKESELECT_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
    }
    break;
  }
}

//=============================================================================
// メッセージ書き込み系
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  地形メッセージ描画
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawLandMsg( HABITAT_ZONE_WK *p_wk )
{
  u32     msg_id = msg_seisoku_zone_01;
  STRBUF  *str;
  STRBUF  *dest_str;
  PRINTSYS_LSB  font_colro = GetMedalFontColor();

  // ゾーンIDに対応した地名メッセージIDを取得
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
  u16 place_id = ZONEDATA_GetPlaceNameID(p_list_data->zone_id);
  
  // 場所ごとにメッセージを切り替える
  if( p_wk->land_type == HABITAT_ZONE_APPEAR_LAND )
  {
    // @todo 場所ごとに、草むら、砂地、沼、洞窟を切り替える処理を作る
    msg_id = msg_seisoku_zone_01;
//    msg_id = msg_seisoku_zone_02;
//    msg_id = msg_seisoku_zone_03;
//    msg_id = msg_seisoku_zone_04;
  }
  else if( p_wk->land_type == HABITAT_ZONE_APPEAR_WATERSIDE )
  {
    msg_id = msg_seisoku_zone_05;
  }
  else if( p_wk->land_type == HABITAT_ZONE_APPEAR_FISHING )
  {
    msg_id = msg_seisoku_zone_06;
  }
  else
  {
    GF_ASSERT( 0 );
  }

  // ワードセットに登録
  WORDSET_RegisterPlaceName( p_wk->p_word_set, 0, place_id );
  
  // ワードセット生成展開先バッファー作成
  dest_str = GFL_STR_CreateBuffer( 256, HEAPID_ZUKAN_HABITAT_ZONE );

  // メッセージの書き込み
  str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_id );
            
  WORDSET_ExpandStr( p_wk->p_word_set, dest_str , str  );
 
  // メッセージの書き込み
  HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_INFO_WIN ,dest_str ,&p_wk->gra_sys ,
    LAND_MSG_POS_X , LAND_MSG_POS_Y ,LAND_MSG_W ,LAND_MSG_H ,font_colro );

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( dest_str );
}

//----------------------------------------------------------------------------
/**
 *	@brief  四季メッセージ書き込み
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawSeasonMsg( HABITAT_ZONE_WK *p_wk )
{
  s32           index = p_wk->season;
  STRBUF        *str;
  PRINTSYS_LSB  font_colro = GetMedalFontColor();

  // 季節
  u32 season_msg_id[] =
  {
    msg_seisoku_season_01,
    msg_seisoku_season_02,
    msg_seisoku_season_03,
    msg_seisoku_season_04,
  };

  // メッセージの書き込み
  str = GFL_MSG_CreateString( p_wk->p_app_msg , season_msg_id[ index ] );

  // メッセージの書き込み
  HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_INFO_WIN ,str ,&p_wk->gra_sys ,
      SEASON_MSG_POS_X , SEASON_MSG_POS_Y,SEASON_MSG_W,SEASON_MSG_H ,
      font_colro );

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );
}

//----------------------------------------------------------------------------
/**
 *	@brief  説明を見る用ボタンメッセージ
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawExplanationMsg( HABITAT_ZONE_WK *p_wk )
{
  STRBUF        *str;
  PRINTSYS_LSB  font_colro = GetMedalFontColor();

  // メッセージの書き込み
  str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_explanation_button_01 );

  // メッセージの書き込み
  HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_INFO_WIN ,str ,&p_wk->gra_sys ,
      EXPLANATION_MSG_POS_X , EXPLANATION_MSG_POS_Y,
      EXPLANATION_MSG_W,EXPLANATION_MSG_H ,
      font_colro );

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );
}

//----------------------------------------------------------------------------
/**
 *	@brief  説明を見るの文章をクリアする
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ClearExplanationMsg( HABITAT_ZONE_WK *p_wk )
{
  // メッセージの書き込み
  HABITAT_ZONE_GRA_ClearWin( PRINT_UTIL_INFO_WIN ,&p_wk->gra_sys ,
      EXPLANATION_MSG_POS_X , EXPLANATION_MSG_POS_Y,
      EXPLANATION_MSG_W,EXPLANATION_MSG_H );
}

//----------------------------------------------------------------------------
/**
 *	@brief  上画面マップ名を表示
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawMapNameMsg( HABITAT_ZONE_WK *p_wk )
{
  STRBUF  *str;
  STRBUF  *dest_str;
  PRINTSYS_LSB  font_colro = GetMedalFontColor();

  // ゾーンIDに対応した地名メッセージIDを取得
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
  u16 place_id = ZONEDATA_GetPlaceNameID(p_list_data->zone_id);
  
  // ワードセットに登録
  WORDSET_RegisterPlaceName( p_wk->p_word_set, 0, place_id );
  
  // ワードセット生成展開先バッファー作成
  dest_str = GFL_STR_CreateBuffer( 256, HEAPID_ZUKAN_HABITAT_ZONE );

  // メッセージの書き込み
  str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_map_name );
            
  WORDSET_ExpandStr( p_wk->p_word_set, dest_str , str  );
 
  // メッセージの書き込み
  HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_MAP_NAME ,dest_str ,&p_wk->gra_sys ,
      MAPNAME_MSG_POS_X , MAPNAME_MSG_POS_Y,
      MAPNAME_MSG_W,MAPNAME_MSG_H ,
      font_colro );

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( dest_str );
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在表示するべきリストデータを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static HABITAT_ZONE_DATA* GetNowListData( HABITAT_ZONE_WK *p_wk )
{
  return &p_wk->p_init_param->p_zone_list_data[p_wk->zone_data_index];
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンリストを作成する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MakePokeIconList( HABITAT_ZONE_WK *p_wk )
{
  s32 i;
  u8  index = 0;
  u8  nonexistent_cnt = 0;
  HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );
  HABITAT_ZONE_POKE_DATA *p_pk;
  
  GF_ASSERT( p_wk->zone_data_cnt > p_wk->zone_data_index );

  // モンスターナンバー配列を初期化
	GFL_STD_MemClear( p_wk->monsno_list , sizeof(p_wk->monsno_list) );
    
  // ポケモンデータのポインタを取得
  p_pk = &p_zd->poke_data[0];

  for( i = 0; i < HABITAT_ZONE_POKE_LIST_MAX; i ++ )
  {
    // 表示判断
    if( (p_pk[i].status == HABITAT_ZONE_STATUS_NONEXISTENT) ||
        (p_pk[i].is_appear[p_wk->season][p_wk->land_type] == FALSE ) )
    {
      // 表示しないときは何もしない
      ONOUE_Printf("%2d:存在しない\n",i);
    }
    else if( p_pk[i].status >= HABITAT_ZONE_STATUS_LOOK )        // 見たか、捕獲のみ
    {
      // ポケモンアイコンをセット
      HABITAT_ZONE_GRA_SetPokeIcon( &p_wk->gra_sys ,p_pk[i].monsno, index );
      // カーソルをOFF状態にする
      HABITAT_ZONE_GRA_SetCursorStatus( &p_wk->gra_sys ,
          index ,GRA_POKEICON_CURSOR_OFF );
      p_wk->monsno_list[index] = p_pk[i].monsno;
      index++;
      ONOUE_Printf("%2d:ポケモンアイコン読み込み\n",i);
    }
    else
    {
      // 未発見の数を数えておく
      nonexistent_cnt ++;
      ONOUE_Printf("%2d:未発見\n",i);
    }
  }

  // 未発見のものカーソルをOFF表示
  for( i = 0; i < nonexistent_cnt; i++ )
  {
    // カーソルをOFF状態にする
    HABITAT_ZONE_GRA_SetCursorStatus( &p_wk->gra_sys ,
                          index ,GRA_POKEICON_CURSOR_OFF );
    index++;
  }

  // リストのサイズを保存
  p_wk->list_size = index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在の状態に合わせて、地形アイコンを更新する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RefreshLandTypeIcon( HABITAT_ZONE_WK *p_wk )
{
  // 地形タイプのポケモンが存在しないときはスキップ
  if( p_wk->land_appear[p_wk->season][p_wk->land_type] == FALSE )
  {
    ONOUE_Printf("左スキップ\n");
    SkipLandType( p_wk ,FALSE);
  }

  // 草地
  if( p_wk->land_type == HABITAT_ZONE_APPEAR_LAND )
  {
    HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                    GRA_LANDICON_TYPE_LAND,GRA_LANDICON_ON );
  }
  else
  {
    if( p_wk->land_appear[p_wk->season][HABITAT_ZONE_APPEAR_LAND] == TRUE )
    {
      HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                        GRA_LANDICON_TYPE_LAND,GRA_LANDICON_OFF );
    }
    else
    {
      HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                        GRA_LANDICON_TYPE_LAND,GRA_LANDICON_NONE );
    }
  }

  // 波乗り
  if( p_wk->land_type == HABITAT_ZONE_APPEAR_WATERSIDE )
  {
    HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                    GRA_LANDICON_TYPE_WATERSIDE,GRA_LANDICON_ON );
  }
  else
  {
    if( p_wk->land_appear[p_wk->season][HABITAT_ZONE_APPEAR_WATERSIDE] == TRUE )
    {
      HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                        GRA_LANDICON_TYPE_WATERSIDE,GRA_LANDICON_OFF );
    }
    else
    {
      HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                        GRA_LANDICON_TYPE_WATERSIDE,GRA_LANDICON_NONE );
    }
  }

  // 釣り
  if( p_wk->land_type == HABITAT_ZONE_APPEAR_FISHING )
  {
    HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                    GRA_LANDICON_TYPE_FISHING,GRA_LANDICON_ON );
  }
  else
  {
    if( p_wk->land_appear[p_wk->season][HABITAT_ZONE_APPEAR_FISHING] == TRUE )
    {
      HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                        GRA_LANDICON_TYPE_FISHING,GRA_LANDICON_OFF );
    }
    else
    {
      HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                        GRA_LANDICON_TYPE_FISHING,GRA_LANDICON_NONE );
    }
  }

  // 地名メッセージ書き込み
  DrawLandMsg( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンリストを再描画する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RefreshLandPokeIcon( HABITAT_ZONE_WK *p_wk )
{
  s32 i;

  for( i = 0; i < HABITAT_ZONE_POKE_LIST_MAX; i ++ )
  {
    // 全てのポケモンアイコンを削除する
    HABITAT_ZONE_GRA_DeletePokeIcon( &p_wk->gra_sys ,i );

    // 全てのカーソルを見えない状態にする
    HABITAT_ZONE_GRA_SetCursorStatus( &p_wk->gra_sys ,
        i ,GRA_POKEICON_CURSOR_NONE );
  }

  // ポケモンアイコン作成
  MakePokeIconList( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルのアクティブ状態を設定する
 *
 *	@param	*p_wk 
 *	@param	index 
 *	@param	is_active 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetCursorstatus( HABITAT_ZONE_WK *p_wk ,s32 index ,BOOL is_active )
{
  GF_ASSERT( index >= 0 );

  if( is_active == TRUE )
  {
    HABITAT_ZONE_GRA_SetCursorStatus( &p_wk->gra_sys ,
                p_wk->select_index ,GRA_POKEICON_CURSOR_ON);
  }
  else
  {
    HABITAT_ZONE_GRA_SetCursorStatus( &p_wk->gra_sys ,
                p_wk->select_index ,GRA_POKEICON_CURSOR_OFF);
  }
  HABITAT_ZONE_GRA_SetPokeAnmFlag( &p_wk->gra_sys ,index , is_active );
  p_wk->is_cursor_active = is_active;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のリストデータから、ゾーン固有データを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MakeZoneCommonData( HABITAT_ZONE_WK *p_wk )
{
  s32 i;
  s32 land_type;
  s32 season_id;
  HABITAT_ZONE_POKE_DATA *p_pk;
  HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );

  // ポケモンデータのポインタを取得
  p_pk = &p_zd->poke_data[0];
	
  // 出現状態配列を初期化
  GFL_STD_MemClear( p_wk->land_appear , sizeof(p_wk->land_appear) );

  // 季節ごとの地形アイコン表示状態を取得す
  for( season_id = 0; season_id < PMSEASON_TOTAL; season_id ++ )
  {
    for( land_type = 0; land_type < HABITAT_ZONE_APPEAR_MAX; land_type ++ )
    {
      for( i = 0; i < HABITAT_ZONE_POKE_LIST_MAX; i ++ )
      {
        // ポケモンが存在するかどうか
        if( p_pk[i].status >= HABITAT_ZONE_STATUS_UNKNOWN )
        {
          // 季節と地形チェック
          if( p_pk[i].is_appear[season_id][land_type] == TRUE )
          {
            p_wk->land_appear[season_id][land_type] = TRUE;
            break;
          }
        }
      }
    }
  }

  // @todo 釣りポケモン開放済みかどうか
  // @todo 波乗り開放済みかどうか

}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン選択用ボタン表示
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetPokeSelectButton( HABITAT_ZONE_WK *p_wk )
{
  GFL_CLWK *p_clwk;
  // 陸地ボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_LAND );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  // 水辺ボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_WATERSIDE );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  // 釣りボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_FISHING );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  // 上矢印
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_UP_ARROW );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  // 下矢印
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_DOWN_ARROW );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  // チェックボックス
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_CHECK_BOX );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  // キャンセル
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_CANCEL );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);

  // 説明を見る
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_EXPLANATION );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);

  // 説明を見るの文字列書き込み
  DrawExplanationMsg( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーン詳細画面ボタン描画
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetZoneSelectButton( HABITAT_ZONE_WK *p_wk )
{
  GFL_CLWK *p_clwk;
  // 陸地ボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_LAND );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,
      p_wk->land_appear[p_wk->season][HABITAT_ZONE_APPEAR_LAND]);
  // 水辺ボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_WATERSIDE );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,
      p_wk->land_appear[p_wk->season][HABITAT_ZONE_APPEAR_WATERSIDE]);
  // 釣りボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_FISHING );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,
      p_wk->land_appear[p_wk->season][HABITAT_ZONE_APPEAR_FISHING]);
  // 上矢印
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_UP_ARROW );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);
  // 下矢印
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_DOWN_ARROW );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);
  // チェックボックス
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_CHECK_BOX );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);
  // キャンセル
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_CANCEL );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);

  // 説明を見る
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_EXPLANATION );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);

  // 説明を見るの文章を削除する
  ClearExplanationMsg( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップ情報を更新する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RefreshMapWin( HABITAT_ZONE_WK *p_wk )
{
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );

  // マップ上の点滅アニメーションを表示
  ONOUE_Printf("zone_id = %x\n",p_list_data->zone_id);
  HABITAT_ZONE_GRA_SetMapBlink( &p_wk->gra_sys ,p_list_data->zone_id);

  // マップ名を上画面に書き込む
  DrawMapNameMsg( p_wk );
}

