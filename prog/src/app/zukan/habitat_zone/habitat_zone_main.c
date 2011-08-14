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

//--------------------------------------------------
// ID定義
typedef enum _MAIN_SEQ{
  MAIN_SEQ_APPINIT,
  MAIN_SEQ_FADEIN,
  MAIN_SEQ_TOP_KEY_INPUT,
  MAIN_SEQ_CHANGE_SEASON,
  MAIN_SEQ_NULL,
}MAIN_SEQ;

//--------------------------------------------------
// 関数定義（ID定義と一致している必要がある）
static const MainFunc MainSeq[] = {
  MAINSEQ_AppInit,      // アプリケーションの初期化
  MAINSEQ_FadeIn,       // フェードイン
  MAINSEQ_TopKeyInput,  // トップキー
  MAINSEQ_ChangeSeason, // 季節切替
  NULL,                 // 何も処理をしない
};

// メッセージ---------------------------------------
static PRINTSYS_LSB GetMedalFontColor( void );
static void DrawLandMsg( HABITAT_ZONE_WK *p_wk );
static void DrawSeasonMsg( HABITAT_ZONE_WK *p_wk );

// データ処理系---------------------------------------
static HABITAT_ZONE_DATA* GetNowListData( HABITAT_ZONE_WK *p_wk );

// ポケモンアイコン系---------------------------------------
static void MakePokeIconList( HABITAT_ZONE_WK *p_wk );
static void GetPokeIconPos( u8 index ,GFL_CLACTPOS *p_pos );

// シーケンス制御 ----------------------------------------------
static void SetNextSeq( HABITAT_ZONE_WK *p_wk ,MAIN_SEQ seq_id ,SEQ_CHANGE_PARAM param );


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
  }
  else if( key_trg & PAD_KEY_RIGHT )
  {
    ONOUE_Printf("場所タイプ切替R\n");
  }
  else if( key_trg & PAD_KEY_UP )
  {
    ONOUE_Printf("ゾーン切替UP\n");
  }
  else if( key_trg & PAD_KEY_DOWN )
  {
    ONOUE_Printf("ゾーン切替DOWN\n");
  }
  else if( key_trg & PAD_BUTTON_A )
  {
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

  // ポケモンアイコンリストを作成する
  MakePokeIconList( p_wk );

  // 地形メッセージ書き込み
  DrawLandMsg( p_wk );

  // 四季メッセージ書き込み
  DrawSeasonMsg( p_wk );

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
      // アニメーションシーケンスを決定
      if( p_wk->seq_param == SEQ_CHANGE_PARAM_L )
      {
        GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_L_ANM);
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
      {
        GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_R_ANM);
      }
      // アニメーションを初期化して、再生
      GFL_CLACT_WK_SetAnmIndex( p_clwk , 0);
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

//=============================================================================
// メッセージ書き込み系
//=============================================================================
static void DrawLandMsg( HABITAT_ZONE_WK *p_wk )
{
  // @todo ゾーンごとのインデックスに変換する必要がある
//  u32   index = p_wk->land_type;
  u32     index = 0;
  STRBUF  *str;
  STRBUF  *dest_str;
  PRINTSYS_LSB  font_colro = GetMedalFontColor();

  // 地形
  u32 land_msg_id[] =
  {
    msg_seisoku_zone_01,
    msg_seisoku_zone_02,
    msg_seisoku_zone_03,
    msg_seisoku_zone_04,
    msg_seisoku_zone_05,
    msg_seisoku_zone_06,
  };

  // ゾーンIDに対応した地名メッセージIDを取得
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
  u16 place_id = ZONEDATA_GetPlaceNameID(p_list_data->zone_id);
  
  // ワードセットに登録
  WORDSET_RegisterPlaceName( p_wk->p_word_set, 0, place_id );
  
  // ワードセット生成展開先バッファー作成
  dest_str = GFL_STR_CreateBuffer( 200, HEAPID_ZUKAN_HABITAT_ZONE );

  // メッセージの書き込み
  str = GFL_MSG_CreateString( p_wk->p_app_msg , land_msg_id[ index ] );
            
  WORDSET_ExpandStr( p_wk->p_word_set, dest_str , str  );

  // メッセージの書き込み
  HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_INFO_WIN ,dest_str ,&p_wk->gra_sys ,
                                  LAND_MSG_POS_X , LAND_MSG_POS_Y ,font_colro );

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
  u32           index = p_wk->season;
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
                                  SEASON_MSG_POS_X , SEASON_MSG_POS_Y ,font_colro );

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );
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
  HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );
  HABITAT_ZONE_POKE_DATA *p_pk;
  
  GF_ASSERT( p_wk->zone_data_cnt > p_wk->zone_data_index );

  // 表示中のゾーンのポケモンリストを作成
  p_pk = &p_zd->poke_data[p_wk->zone_data_index];

  for( i = 0; i < HABITAT_ZONE_POKE_LIST_MAX; i ++ )
  {
    // 表示判断
    if( (p_pk[i].is_appear_season[p_wk->season]) &&     // 季節
        (p_pk[i].is_appear_land[p_wk->land_type]) &&    // 場所
        (p_pk[i].status >= HABITAT_ZONE_STATUS_LOOK) )  // 見たか、捕獲のみ
    {
      // リソース読み込み
      GFL_CLWK *p_clwk = HABITAT_ZONE_GRA_LoadPokeIcon( 
          &p_wk->gra_sys ,p_pk[i].monsno, index );
      // 場所設定
      {
        GFL_CLACTPOS pos;
        GetPokeIconPos( index ,&pos );
        GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DEFREND_MAIN );
      }
      index++;
      ONOUE_Printf("ポケモンアイコン読み込み%d\n",index);
    }
  }

  // リストのサイズを保存
  p_wk->list_size = index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンの座標を取得する
 *
 *	@param	index 
 *	@param	*p_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetPokeIconPos( u8 index ,GFL_CLACTPOS *p_pos )
{
  p_pos->x = POKEICON_START_X + (index%POKEICON_LINE)*POKEICON_MARGIN_W;
  p_pos->y = POKEICON_START_Y + (index/POKEICON_LINE)*POKEICON_MARGIN_Y;
}


