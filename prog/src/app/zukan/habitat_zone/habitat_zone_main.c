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
#include "field/zonedata.h"         // for ZONEDATA_GetPlaceNameID
#include "sound/pm_sndsys.h"
#include "habitat_zone_main_def.h"  // 定数定義
#include "../zukan_common.h"

// 生息地データ
#include "fieldmap/zone_id.h" // for ZONE_ID_*
#include "../../../../resource/fldmapdata/flagwork/flag_define.h" // for SYS_FLAG_ARRIVE_*
#include "../../../../resource/habitat_list/habitat_list_data.h" // for HABITAT_LIST_DATA_MAX
#include "../../../../../resource/habitat_list/habitat_list_data.cdat" // for sc_habitat_list_data
#include "../list/zknlist_snd_def.h"  // リストと同じ音を使用

#define GET_CURPOS_X(i) ((POKEICON_CURSOR_POS_X + (i%POKEICON_LINE)*POKEICON_CURSOR_POS_W)*8)
#define GET_CURPOS_Y(i) ((POKEICON_CURSOR_POS_Y + (i/POKEICON_LINE)*POKEICON_CURSOR_POS_H)*8)
#define POKECURSOR_W (POKEICON_CURSOR_POS_W*8)
#define POKECURSOR_H (POKEICON_CURSOR_POS_H*8)

#define FCOL_WP01WN		( PRINTSYS_LSB_Make(0xF,0x1,0x2) )	// フォントカラー：両用０１白抜
#define FCOL_BP01WN		( PRINTSYS_LSB_Make(0x1,0x2,0xF) )	// フォントカラー：両用０１黒抜
#define FCOL_GP01WN		( PRINTSYS_LSB_Make(0x2,0x1,0x0) )	// フォントカラー：両用０１グレー
#define SBG_TITLE_FONT_COL PRINTSYS_LSB_Make(0xE,0xF,0)

#define MENU_BAR_SCROLL_SPEED   (3)   // メニューバーのスクロールスピード



//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
typedef void (*MainFunc)(HABITAT_ZONE_WK*);

// 定数
enum{
  // タッチバー上のボタン座標
  TOUCHBAR_ICON_Y	    = 168,
  TOUCHBAR_ICON_X_07  = 232,
  TB_SX			= 24,
  TB_SY			= 24,
  TB_PY			= TOUCHBAR_ICON_Y,
  TB_PX07		= TOUCHBAR_ICON_X_07,
  TB_PX06		= TB_PX07 - TB_SX,
  TB_PX05		= TB_PX06 - TB_SX,
  
  TB_PX04		= TB_PX06 - (TB_SX*8),
  TB_PX03		= TB_PX04 - TB_SX,

  // コンプリートイベント
  COMPLETE_EVENT_WAIT_FRAME = 30,
};

// タッチ座標テーブル
static const GFL_UI_TP_HITTBL sc_touch_hit_tbl_top_menu[] =
{
  { TB_PY, TB_PY+TB_SY-1, TB_PX05, TB_PX05+TB_SX-1 },		// Ｙ
	{ TB_PY, TB_PY+TB_SY-1, TB_PX06, TB_PX06+TB_SX-1 },		// Ｘ
	{ TB_PY, TB_PY+TB_SY-1, TB_PX07, TB_PX07+TB_SX-1 },		// 戻る
	
  { TB_PY, TB_PY+TB_SY-1,
    OBJ_POS_BUTTON_LAND_X - 12, OBJ_POS_BUTTON_LAND_X + 12 },		// 草地
  { TB_PY, TB_PY+TB_SY-1,
    OBJ_POS_BUTTON_WATERSIDE_X - 12, OBJ_POS_BUTTON_WATERSIDE_X + 12 },		// 水辺
  { TB_PY, TB_PY+TB_SY-1,
    OBJ_POS_BUTTON_FISHING_X - 12, OBJ_POS_BUTTON_FISHING_X + 12 },		// 釣り

	{ TB_PY, TB_PY+TB_SY-1, 0, 24 },		    // 上
	{ TB_PY, TB_PY+TB_SY-1, 24, 24+24 },		    // 下

	{ 60-8, 60-8 + 24, 8+4 , 32+4     },		        // 季節L
	{ 60-8, 60-8 + 24, 232-4 , 248-4  },		        // 季節R
	
  // カーソル
  { GET_CURPOS_Y(0) ,GET_CURPOS_Y(0) + POKECURSOR_H,
    GET_CURPOS_X(0),GET_CURPOS_X(0)+POKECURSOR_W},
  { GET_CURPOS_Y(1) ,GET_CURPOS_Y(1) + POKECURSOR_H,
    GET_CURPOS_X(1),GET_CURPOS_X(1)+POKECURSOR_W},
  { GET_CURPOS_Y(2) ,GET_CURPOS_Y(2) + POKECURSOR_H,
    GET_CURPOS_X(2),GET_CURPOS_X(2)+POKECURSOR_W},
  { GET_CURPOS_Y(3) ,GET_CURPOS_Y(3) + POKECURSOR_H,
    GET_CURPOS_X(3),GET_CURPOS_X(3)+POKECURSOR_W},
  { GET_CURPOS_Y(4) ,GET_CURPOS_Y(4) + POKECURSOR_H,
    GET_CURPOS_X(4),GET_CURPOS_X(4)+POKECURSOR_W},
  { GET_CURPOS_Y(5) ,GET_CURPOS_Y(5) + POKECURSOR_H,
    GET_CURPOS_X(5),GET_CURPOS_X(5)+POKECURSOR_W},
  { GET_CURPOS_Y(6) ,GET_CURPOS_Y(6) + POKECURSOR_H,
    GET_CURPOS_X(6),GET_CURPOS_X(6)+POKECURSOR_W},
  { GET_CURPOS_Y(7) ,GET_CURPOS_Y(7) + POKECURSOR_H,
    GET_CURPOS_X(7),GET_CURPOS_X(7)+POKECURSOR_W},
  { GET_CURPOS_Y(8) ,GET_CURPOS_Y(8) + POKECURSOR_H,
    GET_CURPOS_X(8),GET_CURPOS_X(8)+POKECURSOR_W},
  { GET_CURPOS_Y(9) ,GET_CURPOS_Y(9) + POKECURSOR_H,
    GET_CURPOS_X(9),GET_CURPOS_X(9)+POKECURSOR_W},
  { GET_CURPOS_Y(10) ,GET_CURPOS_Y(10) + POKECURSOR_H,
    GET_CURPOS_X(10),GET_CURPOS_X(10)+POKECURSOR_W},
  { GET_CURPOS_Y(11) ,GET_CURPOS_Y(11) + POKECURSOR_H,
    GET_CURPOS_X(11),GET_CURPOS_X(11)+POKECURSOR_W},
  { GET_CURPOS_Y(12) ,GET_CURPOS_Y(12) + POKECURSOR_H,
    GET_CURPOS_X(12),GET_CURPOS_X(12)+POKECURSOR_W},
  { GET_CURPOS_Y(13) ,GET_CURPOS_Y(13) + POKECURSOR_H,
    GET_CURPOS_X(13),GET_CURPOS_X(13)+POKECURSOR_W},
  { GET_CURPOS_Y(14) ,GET_CURPOS_Y(14) + POKECURSOR_H,
    GET_CURPOS_X(14),GET_CURPOS_X(14)+POKECURSOR_W},
  { GET_CURPOS_Y(15) ,GET_CURPOS_Y(15) + POKECURSOR_H,
    GET_CURPOS_X(15),GET_CURPOS_X(15)+POKECURSOR_W},
  { GET_CURPOS_Y(16) ,GET_CURPOS_Y(16) + POKECURSOR_H,
    GET_CURPOS_X(16),GET_CURPOS_X(16)+POKECURSOR_W},
  { GET_CURPOS_Y(17) ,GET_CURPOS_Y(17) + POKECURSOR_H,
    GET_CURPOS_X(17),GET_CURPOS_X(17)+POKECURSOR_W},
  { GET_CURPOS_Y(18) ,GET_CURPOS_Y(18) + POKECURSOR_H,
    GET_CURPOS_X(18),GET_CURPOS_X(18)+POKECURSOR_W},
  { GET_CURPOS_Y(19) ,GET_CURPOS_Y(19) + POKECURSOR_H,
    GET_CURPOS_X(19),GET_CURPOS_X(19)+POKECURSOR_W},
};

// トップキー入力ID
enum{
  TPID_BUTTON_Y,
  TPID_BUTTON_X,
  TPID_BUTTON_RETURN,
  TPID_BUTTON_GROUND_LAND,
  TPID_BUTTON_GROUND_WATERSIDE,
  TPID_BUTTON_GROUND_FISHING,
  TPID_BUTTON_UP,
  TPID_BUTTON_DOWN,
  TPID_BUTTON_SEASON_L,
  TPID_BUTTON_SEASON_R,

  TPID_LIST_CURSOR_01,
  TPID_LIST_CURSOR_02,
  TPID_LIST_CURSOR_03,
  TPID_LIST_CURSOR_04,
  TPID_LIST_CURSOR_05,
  TPID_LIST_CURSOR_06,
  TPID_LIST_CURSOR_07,
  TPID_LIST_CURSOR_08,
  TPID_LIST_CURSOR_09,
  TPID_LIST_CURSOR_10,
  TPID_LIST_CURSOR_11,
  TPID_LIST_CURSOR_12,
  TPID_LIST_CURSOR_13,
  TPID_LIST_CURSOR_14,
  TPID_LIST_CURSOR_15,
  TPID_LIST_CURSOR_16,
  TPID_LIST_CURSOR_17,
  TPID_LIST_CURSOR_18,
  TPID_LIST_CURSOR_19,
  TPID_LIST_CURSOR_20,
};

// タッチ座標テーブル
static const GFL_UI_TP_HITTBL sc_touch_hit_tbl_poke_select[] =
{
  { TB_PY, TB_PY+TB_SY-1, TB_PX07, TB_PX07+TB_SX-1 },		// 戻る
	{ TB_PY, TB_PY+TB_SY-1, 96, 212 },		                // 説明を見るボタン

  // カーソル
  { GET_CURPOS_Y(0) ,GET_CURPOS_Y(0) + POKECURSOR_H,
    GET_CURPOS_X(0),GET_CURPOS_X(0)+POKECURSOR_W},
  { GET_CURPOS_Y(1) ,GET_CURPOS_Y(1) + POKECURSOR_H,
    GET_CURPOS_X(1),GET_CURPOS_X(1)+POKECURSOR_W},
  { GET_CURPOS_Y(2) ,GET_CURPOS_Y(2) + POKECURSOR_H,
    GET_CURPOS_X(2),GET_CURPOS_X(2)+POKECURSOR_W},
  { GET_CURPOS_Y(3) ,GET_CURPOS_Y(3) + POKECURSOR_H,
    GET_CURPOS_X(3),GET_CURPOS_X(3)+POKECURSOR_W},
  { GET_CURPOS_Y(4) ,GET_CURPOS_Y(4) + POKECURSOR_H,
    GET_CURPOS_X(4),GET_CURPOS_X(4)+POKECURSOR_W},
  { GET_CURPOS_Y(5) ,GET_CURPOS_Y(5) + POKECURSOR_H,
    GET_CURPOS_X(5),GET_CURPOS_X(5)+POKECURSOR_W},
  { GET_CURPOS_Y(6) ,GET_CURPOS_Y(6) + POKECURSOR_H,
    GET_CURPOS_X(6),GET_CURPOS_X(6)+POKECURSOR_W},
  { GET_CURPOS_Y(7) ,GET_CURPOS_Y(7) + POKECURSOR_H,
    GET_CURPOS_X(7),GET_CURPOS_X(7)+POKECURSOR_W},
  { GET_CURPOS_Y(8) ,GET_CURPOS_Y(8) + POKECURSOR_H,
    GET_CURPOS_X(8),GET_CURPOS_X(8)+POKECURSOR_W},
  { GET_CURPOS_Y(9) ,GET_CURPOS_Y(9) + POKECURSOR_H,
    GET_CURPOS_X(9),GET_CURPOS_X(9)+POKECURSOR_W},
  { GET_CURPOS_Y(10) ,GET_CURPOS_Y(10) + POKECURSOR_H,
    GET_CURPOS_X(10),GET_CURPOS_X(10)+POKECURSOR_W},
  { GET_CURPOS_Y(11) ,GET_CURPOS_Y(11) + POKECURSOR_H,
    GET_CURPOS_X(11),GET_CURPOS_X(11)+POKECURSOR_W},
  { GET_CURPOS_Y(12) ,GET_CURPOS_Y(12) + POKECURSOR_H,
    GET_CURPOS_X(12),GET_CURPOS_X(12)+POKECURSOR_W},
  { GET_CURPOS_Y(13) ,GET_CURPOS_Y(13) + POKECURSOR_H,
    GET_CURPOS_X(13),GET_CURPOS_X(13)+POKECURSOR_W},
  { GET_CURPOS_Y(14) ,GET_CURPOS_Y(14) + POKECURSOR_H,
    GET_CURPOS_X(14),GET_CURPOS_X(14)+POKECURSOR_W},
  { GET_CURPOS_Y(15) ,GET_CURPOS_Y(15) + POKECURSOR_H,
    GET_CURPOS_X(15),GET_CURPOS_X(15)+POKECURSOR_W},
  { GET_CURPOS_Y(16) ,GET_CURPOS_Y(16) + POKECURSOR_H,
    GET_CURPOS_X(16),GET_CURPOS_X(16)+POKECURSOR_W},
  { GET_CURPOS_Y(17) ,GET_CURPOS_Y(17) + POKECURSOR_H,
    GET_CURPOS_X(17),GET_CURPOS_X(17)+POKECURSOR_W},
  { GET_CURPOS_Y(18) ,GET_CURPOS_Y(18) + POKECURSOR_H,
    GET_CURPOS_X(18),GET_CURPOS_X(18)+POKECURSOR_W},
  { GET_CURPOS_Y(19) ,GET_CURPOS_Y(19) + POKECURSOR_H,
    GET_CURPOS_X(19),GET_CURPOS_X(19)+POKECURSOR_W},
};

// トップキー入力ID
enum{
  TPID_PSELECT_RETURN,
  TPID_PSELECT_EXPLANATIO,

  TPID_PS_LIST_CURSOR_01,
  TPID_PS_LIST_CURSOR_02,
  TPID_PS_LIST_CURSOR_03,
  TPID_PS_LIST_CURSOR_04,
  TPID_PS_LIST_CURSOR_05,
  TPID_PS_LIST_CURSOR_06,
  TPID_PS_LIST_CURSOR_07,
  TPID_PS_LIST_CURSOR_08,
  TPID_PS_LIST_CURSOR_09,
  TPID_PS_LIST_CURSOR_10,
  TPID_PS_LIST_CURSOR_11,
  TPID_PS_LIST_CURSOR_12,
  TPID_PS_LIST_CURSOR_13,
  TPID_PS_LIST_CURSOR_14,
  TPID_PS_LIST_CURSOR_15,
  TPID_PS_LIST_CURSOR_16,
  TPID_PS_LIST_CURSOR_17,
  TPID_PS_LIST_CURSOR_18,
  TPID_PS_LIST_CURSOR_19,
  TPID_PS_LIST_CURSOR_20,
};

// コンプリートイベントシーケンス
enum{
  CMP_EVENT_ALL_CMP_CHECK,
  CMP_EVENT_FANFARE_CHECK,
  CMP_EVENT_ALL_CMP_KEY_CHECK,
  CMP_EVENT_STAMP_START,
  CMP_EVENT_STAMP_END,
  CMP_EVENT_NEXT_STATE_CHECK,
  CMP_EVENT_END,
};

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
static void MAINSEQ_AppExit( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_FadeIn( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_FadeOut( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_TopKeyInput( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_ReturnTopKeyInput( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_ChangeSeason( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_ChangeZone( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_ChangeLandType( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_PokeSelectKeyInput( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_MovePokeCursor( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_CompleteEvetn( HABITAT_ZONE_WK *p_wk );
static void MAINSEQ_CompleteTutorial( HABITAT_ZONE_WK *p_wk );

//--------------------------------------------------
// ID定義
typedef enum _MAIN_SEQ{
  MAIN_SEQ_APPINIT,
  MAIN_SEQ_FADEIN,
  MAIN_SEQ_FADEOUT,
  MAIN_SEQ_TOP_KEY_INPUT,
  MAIN_SEQ_RETURN_TOP_KEY_INPUT,
  MAIN_SEQ_CHANGE_SEASON,
  MAIN_SEQ_CHANGE_ZONE,
  MAIN_SEQ_CHANGE_LAND_TYPE,
  MAIN_SEQ_POKESELECT_KEY_INPUT,
  MAIN_SEQ_MOVE_POKE_CURSOR,
  MAIN_SEQ_COMPLETE_EVETN,
  MAIN_SEQ_APPEXIT,
  MAIN_SEQ_COMPLETE_TUTORIAL,
  MAIN_SEQ_NULL,
}MAIN_SEQ;

//--------------------------------------------------
// 関数定義（ID定義と一致している必要がある）
static const MainFunc MainSeq[] = {
  MAINSEQ_AppInit,            // アプリケーションの初期化
  MAINSEQ_FadeIn,             // フェードイン
  MAINSEQ_FadeOut,            // フェードアウト
  MAINSEQ_TopKeyInput,        // トップキー
  MAINSEQ_ReturnTopKeyInput,  // トップキー入力に戻る
  MAINSEQ_ChangeSeason,       // 季節切替
  MAINSEQ_ChangeZone,         // ゾーン切替
  MAINSEQ_ChangeLandType,     // 地形タイプ切替
  MAINSEQ_PokeSelectKeyInput, // ポケモン選択キー入力
  MAINSEQ_MovePokeCursor,     // ポケモン選択カーソルを移動させる
  MAINSEQ_CompleteEvetn,      // 達成イベント
  MAINSEQ_AppExit,            // アプリケーションの終了処理
  MAINSEQ_CompleteTutorial,   // コンプリートイベントチュートリアル
  NULL,                       // 何も処理をしない
};

// メッセージ---------------------------------------
static void DrawLandMsg( HABITAT_ZONE_WK *p_wk );
static void DrawExplanationMsg( HABITAT_ZONE_WK *p_wk ,PRINTSYS_LSB font_colro);
static void ClearExplanationMsg( HABITAT_ZONE_WK *p_wk );
static void DrawMapNameMsg( HABITAT_ZONE_WK *p_wk );
//static void DrawNoHabitatDatatMsg( HABITAT_ZONE_WK *p_wk );
//static void ClearNoHabitatDataMsg( HABITAT_ZONE_WK *p_wk );
static u32 GetLandTypeMsgID( HABITAT_GROUND_TYPE type ,HABITAT_GROUND_INFO_TYPE info_type);
static u32 GetLandTypeMsgIDSelect( HABITAT_GROUND_TYPE type ,HABITAT_GROUND_INFO_TYPE info_type ,const u32 *p_ground_msg,u32 sea_msg,u32 fishing_msg);

// データ処理系---------------------------------------
static HABITAT_ZONE_DATA* GetNowListData( HABITAT_ZONE_WK *p_wk );
static void SetCursorstatus( HABITAT_ZONE_WK *p_wk ,s32 index ,BOOL is_active );
static void MakeZoneCommonData( HABITAT_ZONE_WK *p_wk );
static void SkipLandType( HABITAT_ZONE_WK *p_wk ,BOOL is_skip_r );
static void SetRetMod( HABITAT_ZONE_WK *p_wk ,HABITAT_ZONE_RET_MODE ret_mode );
static void CreateAppData( HABITAT_ZONE_WK *p_wk );
static s32 CheckIsExistPoke( HABITAT_ZONE_WK *p_wk ,u8 index );
static HABITATLIST_STATUS CheckCompleteEvent( HABITAT_ZONE_WK *p_wk,HABITAT_GROUND_TYPE land_type );
static HABITATLIST_STATUS CheckCompleteEventNowLandType( HABITAT_ZONE_WK *p_wk );
static s32 GetSaveDataIndex( u16 zone_group_id );
static void SetShortCut( HABITAT_ZONE_WK *p_wk );
static void SetShortCutButtonCutAnmSeq( HABITAT_ZONE_WK *p_wk );
static BOOL CheckCanMoveLandType( HABITAT_ZONE_WK *p_wk );
static void CheckPokeSelectKeyInput( HABITAT_ZONE_WK *p_wk );
static void CheckTopKeyInput( HABITAT_ZONE_WK *p_wk );
static void SetLandTypeChangeSeq( HABITAT_ZONE_WK *p_wk , HABITAT_GROUND_TYPE land_type);
static void SetEventIcon( HABITAT_ZONE_WK *p_wk ,HABITAT_GROUND_TYPE ground_type ,GFL_CLWK *p_clwk );

// ポケモンアイコン系---------------------------------------
static void MakePokeIconList( HABITAT_ZONE_WK *p_wk );

// シーケンス制御 ----------------------------------------------
static void SetNextSeq( HABITAT_ZONE_WK *p_wk ,MAIN_SEQ seq_id ,SEQ_CHANGE_PARAM param );
static void SetCompleteEvetnID( HABITAT_ZONE_WK *p_wk ,HABITATLIST_STATUS id );

// 画面更新系
static void RefreshLandTypeIcon( HABITAT_ZONE_WK *p_wk );
static void RefreshLandPokeIcon( HABITAT_ZONE_WK *p_wk );
static void SetPokeSelectButton( HABITAT_ZONE_WK *p_wk );
static void SetZoneSelectButton( HABITAT_ZONE_WK *p_wk );
static void RefreshMapWin( HABITAT_ZONE_WK *p_wk );
static void SetCompIcon( HABITAT_ZONE_WK *p_wk );
static void RefreshEventIcon( HABITAT_ZONE_WK *p_wk );
static void SetExplanationButtonStatus( HABITAT_ZONE_WK *p_wk ,BOOL is_active ,BOOL is_visible );

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

  // シーケンスの状態を監視
  if( p_wk->main_status != HABITAT_ZONE_MAIN_CONTINUE )
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
        p_wk->main_scroll_cnt = FRAME_SCROLL_COUNT;
        p_wk->sub_scroll_cnt  = -FRAME_SCROLL_COUNT;
        // パレットフェードを行う
        HABITAT_ZONE_GRA_CallFade( &p_wk->gra_sys ,0,0 );
        HABITAT_ZONE_GRA_AddScrollFrame( &p_wk->gra_sys ,p_wk->main_scroll_cnt,TRUE );
        HABITAT_ZONE_GRA_AddScrollFrame( &p_wk->gra_sys ,p_wk->sub_scroll_cnt,FALSE );
        p_wk->sub_seq++;
      }
    }
    break;
  case 1:
    {
      if( HABITAT_ZONE_GRA_CheckFade( &p_wk->gra_sys ) == 0 )
      {
        // マスター輝度を戻す
        GX_SetMasterBrightness( 0 );
        GXS_SetMasterBrightness( 0 );
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
     
      // フェードが終わり次第、マップを表示する
      if( HABITAT_ZONE_GRA_CheckFade( &p_wk->gra_sys ) == 0 )
      {
        GFL_BG_SetVisible( BGS_FRAME_MAP ,VISIBLE_ON );
        GFL_BG_SetVisible( BGS_FRAME_ROOT ,VISIBLE_ON );
      }

      // スクロール待ち
      if( p_wk->main_scroll_cnt <= 0)
      {
        HABITATLIST_STATUS event;
        event = CheckCompleteEventNowLandType( p_wk );
        if( event != HABITAT_ZONE_STATUS_NONE )
        {
          // イベントIDを設定する
          SetCompleteEvetnID( p_wk ,event );
          SetNextSeq( p_wk, MAIN_SEQ_COMPLETE_EVETN ,SEQ_CHANGE_PARAM_NONE);
        }
        else
        {
          // 通常キー入力に起動
          SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
        }
      }
      else
      {
        HABITAT_ZONE_GRA_AddScrollFrame( &p_wk->gra_sys ,-MENU_BAR_SCROLL_SPEED,TRUE );
        HABITAT_ZONE_GRA_AddScrollFrame( &p_wk->gra_sys ,MENU_BAR_SCROLL_SPEED,FALSE );
        p_wk->main_scroll_cnt-=MENU_BAR_SCROLL_SPEED;
        p_wk->sub_scroll_cnt+=MENU_BAR_SCROLL_SPEED;
      }
    }
    break;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウトシーケンス
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_FadeOut( HABITAT_ZONE_WK *p_wk )
{
  switch(p_wk->sub_seq)
  {
  case 0:
    {
      // 文字描画終了を待ってフェードを呼び出す
      if( HABITAT_ZONE_GRA_CheckPrintUtilStatus( &p_wk->gra_sys ) )
      {
        GFL_CLWK *p_clwk;

        // 点滅アニメを停止
        HABITAT_ZONE_GRA_SetBlinkStop( &p_wk->gra_sys ,TRUE );

        p_wk->main_scroll_cnt = 0;
        p_wk->sub_scroll_cnt  = 0;
        // パレットフェードを行う
        HABITAT_ZONE_GRA_CallFade( &p_wk->gra_sys ,16,16 );

        // 説明を見るボタンを削除
        ClearExplanationMsg( p_wk );
        // 説明を見る
        p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,
            OBJ_WK_BUTTON_EXPLANATION );
        GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);

        // イベントアイコンも非表示に（アイコンをフェードできない）
        HABITAT_ZONE_GRA_SetEventIconEnable( &p_wk->gra_sys ,FALSE );

        // マップ面を非表示にする
        {
          u16 col = 0;
          GXS_LoadBGPltt( &col ,0 ,sizeof(col) ); // バックドロップを黒にしておく
          GFL_BG_SetVisible( BGS_FRAME_MAP ,VISIBLE_OFF );
          GFL_BG_SetVisible( BGS_FRAME_ROOT ,VISIBLE_OFF );
        }

        p_wk->sub_seq++;
      }
    }
    break;
  case 1:
    {
      if( HABITAT_ZONE_GRA_CheckFade( &p_wk->gra_sys ) == 0 )
      {
        // スクロール待ち
        if( p_wk->main_scroll_cnt >= FRAME_SCROLL_COUNT)
        {
          // 輝度を最低にしておく
          GX_SetMasterBrightness( -16 );
          GXS_SetMasterBrightness( -16 );

          // メインシーケンスの状態を終了に設定
          p_wk->main_status = HABITAT_ZONE_MAIN_EXIT;
        }
        else
        {
          HABITAT_ZONE_GRA_AddScrollFrame( &p_wk->gra_sys ,MENU_BAR_SCROLL_SPEED,TRUE );
          HABITAT_ZONE_GRA_AddScrollFrame( &p_wk->gra_sys ,-MENU_BAR_SCROLL_SPEED,FALSE );
          p_wk->main_scroll_cnt+=MENU_BAR_SCROLL_SPEED;
          p_wk->sub_scroll_cnt-=MENU_BAR_SCROLL_SPEED;
        }
      }
    }
    break;
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
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
  s32   key_status;
	key_status = GFL_UI_TP_HitTrg( sc_touch_hit_tbl_top_menu );

  switch( key_status )
  {
  case TPID_BUTTON_Y:
    {
      PMSND_PlaySE( ZKNLIST_SE_Y );
      SetShortCut( p_wk );
    }
    break;
  case TPID_BUTTON_X:
    {
      SetNextSeq( p_wk ,MAIN_SEQ_APPEXIT ,SEQ_CHANGE_PARAM_X );
      ONOUE_Printf("TP_BUTTON_X\n");
    }
    break;
  case TPID_BUTTON_RETURN:
    {
      ONOUE_Printf("TP_BUTTON_RETURN\n");
      SetNextSeq( p_wk ,MAIN_SEQ_APPEXIT ,SEQ_CHANGE_PARAM_B );
    }
    break;
  case TPID_BUTTON_GROUND_LAND:
    {
      // 地形タイプ切替(草地)
      SetLandTypeChangeSeq( p_wk , HABITAT_GROUND_LAND );
    }
    break;
  case TPID_BUTTON_GROUND_WATERSIDE:
    {
      // 地形タイプ切替(水辺)
      SetLandTypeChangeSeq( p_wk , HABITAT_GROUND_WATERSIDE );
    }
    break;
  case TPID_BUTTON_GROUND_FISHING:
    {
      // 地形タイプ切替(釣り)
      SetLandTypeChangeSeq( p_wk , HABITAT_GROUND_FISHING );
    }
    break;
  case TPID_BUTTON_UP:
    {
      if( p_wk->zone_data_cnt > 1 )
      {
        ONOUE_Printf("ゾーン切替UP\n");
        SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_ZONE ,SEQ_CHANGE_PARAM_UP );
      }
    }
    break;
  case TPID_BUTTON_DOWN:
    {
      if( p_wk->zone_data_cnt > 1 )
      {
        ONOUE_Printf("ゾーン切替DOWN\n");
        SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_ZONE ,SEQ_CHANGE_PARAM_DOWN );
      }
    }
    break;
  case TPID_BUTTON_SEASON_L:
    {
      if( p_list_data->season_enable[p_wk->land_type] == TRUE )
      {
        ONOUE_Printf("季節切替L\n");
        SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_SEASON ,SEQ_CHANGE_PARAM_L );
      }
    }
    break;
  case TPID_BUTTON_SEASON_R:
    {
      if( p_list_data->season_enable[p_wk->land_type] == TRUE )
      {
        ONOUE_Printf("季節切替R\n");
        SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_SEASON ,SEQ_CHANGE_PARAM_R );
      }
    }
    break;
  case TPID_LIST_CURSOR_01:
  case TPID_LIST_CURSOR_02:
  case TPID_LIST_CURSOR_03:
  case TPID_LIST_CURSOR_04:
  case TPID_LIST_CURSOR_05:
  case TPID_LIST_CURSOR_06:
  case TPID_LIST_CURSOR_07:
  case TPID_LIST_CURSOR_08:
  case TPID_LIST_CURSOR_09:
  case TPID_LIST_CURSOR_10:
  case TPID_LIST_CURSOR_11:
  case TPID_LIST_CURSOR_12:
  case TPID_LIST_CURSOR_13:
  case TPID_LIST_CURSOR_14:
  case TPID_LIST_CURSOR_15:
  case TPID_LIST_CURSOR_16:
  case TPID_LIST_CURSOR_17:
  case TPID_LIST_CURSOR_18:
  case TPID_LIST_CURSOR_19:
  case TPID_LIST_CURSOR_20:
    {
      // リストが存在し、ポケモンを選んだときのみ
      if( p_wk->list_size > 0 )
      {
        s32 index = key_status - TPID_LIST_CURSOR_01;
        if(  index < p_wk->list_size )
        {
          // ポケモンを選択しているか調べる
          s32 mons_no = CheckIsExistPoke( p_wk ,index );
          if( mons_no >= 0 )
          {
            p_wk->tp_select_index = index;
            SetNextSeq(p_wk,MAIN_SEQ_POKESELECT_KEY_INPUT,SEQ_CHANGE_PARAM_TOUCH );
          }
        }
      }
    }
    break;

    // タッチ入力なし
  case GFL_UI_TP_HIT_NONE:
    {
      // キー入力チェック
      CheckTopKeyInput( p_wk );
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定地形タイプに変えれるか判断した後にシーケンスを切り替える
 *
 *	@param	*p_wk 
 *	@param	land_type
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetLandTypeChangeSeq( HABITAT_ZONE_WK *p_wk , HABITAT_GROUND_TYPE land_type)
{
  if( p_wk->land_type == land_type != TRUE )
  {
    if( CheckCanMoveLandType( p_wk ) )
    {
      if( p_wk->land_appear[land_type] == TRUE )
      {
        p_wk->land_type = land_type;
        SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_LAND_TYPE ,
            SEQ_CHANGE_PARAM_SET_LAND_TYPE );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief トップ画面でのキー入力チェック
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CheckTopKeyInput( HABITAT_ZONE_WK *p_wk )
{
  s32 key_rep = GFL_UI_KEY_GetRepeat();
  s32 key_trg = GFL_UI_KEY_GetTrg();
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );

  // 四季切替
  if( key_rep & PAD_BUTTON_L )
  {
    if( p_list_data->season_enable[p_wk->land_type] == TRUE )
    {
      ONOUE_Printf("季節切替L\n");
      SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_SEASON ,SEQ_CHANGE_PARAM_L );
    }
  }
  else if( key_rep & PAD_BUTTON_R )
  {
    if( p_list_data->season_enable[p_wk->land_type] == TRUE )
    {
      ONOUE_Printf("季節切替R\n");
      SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_SEASON ,SEQ_CHANGE_PARAM_R );
    }
  }
  else if( key_rep & PAD_KEY_LEFT )
  {
    if( CheckCanMoveLandType( p_wk ) )
    {
      ONOUE_Printf("場所タイプ切替L\n");
      SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_LAND_TYPE ,SEQ_CHANGE_PARAM_L );
    }
  }
  else if( key_rep & PAD_KEY_RIGHT )
  {
    if( CheckCanMoveLandType( p_wk ) )
    {
      ONOUE_Printf("場所タイプ切替R\n");
      SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_LAND_TYPE ,SEQ_CHANGE_PARAM_R );
    }
  }
  else if( key_rep & PAD_KEY_UP )
  {
    if( p_wk->zone_data_cnt > 1 )
    {
      ONOUE_Printf("ゾーン切替UP\n");
      SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_ZONE ,SEQ_CHANGE_PARAM_UP );
    }
  }
  else if( key_rep & PAD_KEY_DOWN )
  {
    if( p_wk->zone_data_cnt > 1 )
    {
      ONOUE_Printf("ゾーン切替DOWN\n");
      SetNextSeq( p_wk ,MAIN_SEQ_CHANGE_ZONE ,SEQ_CHANGE_PARAM_DOWN );
    }
  }
  else if( key_trg & PAD_BUTTON_A )
  {
    if( p_wk->list_size > 0 )
    {
      // ポケモンを選択しているか調べる
      s32 mons_no = CheckIsExistPoke( p_wk ,0 );
      if( mons_no >= 0 )
      {
        p_wk->select_index = 0;
        SetNextSeq( p_wk ,MAIN_SEQ_POKESELECT_KEY_INPUT ,SEQ_CHANGE_PARAM_A );
        ONOUE_Printf("ポケモン選択画面へ\n");
      }
    }
  }
  else if( key_trg & PAD_BUTTON_B )
  {
    SetNextSeq( p_wk ,MAIN_SEQ_APPEXIT ,SEQ_CHANGE_PARAM_B );
    ONOUE_Printf("生息地リスト画面へ\n");
  }
  else if( key_trg & PAD_BUTTON_X )
  {
    SetNextSeq( p_wk ,MAIN_SEQ_APPEXIT ,SEQ_CHANGE_PARAM_X );
    ONOUE_Printf("フィールドへ戻る\n");
  }
  else if( key_trg & PAD_BUTTON_Y )
  {
    PMSND_PlaySE( ZKNLIST_SE_Y );
    SetShortCut( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  トップキー入力へ戻る
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_ReturnTopKeyInput( HABITAT_ZONE_WK *p_wk )
{
  GFL_CLWK *p_clwk;
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_RETURN );

  switch(p_wk->sub_seq)
  {
  case 0:
    {
      // アニメーション開始
      GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
      GFL_CLACT_WK_SetAnmSeq( p_clwk , APP_COMMON_BARICON_RETURN_ON );
      GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,TRUE );
      PMSND_PlaySE( SE_RETURN );
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      // 音待ち
      if( !PMSND_CheckPlaySE() )
      {
        // アニメーション終了待ち
        if( GFL_CLACT_WK_CheckAnmActive( p_clwk ) == FALSE )
        {
          // 現在のカーソル位置をOFFに
          SetCursorstatus( p_wk ,p_wk->select_index ,FALSE );
          // ボタン表示を切り替える
          SetZoneSelectButton( p_wk );

          // アニメーションを戻して、キー入力待ちに移行
          GFL_CLACT_WK_SetAnmSeq( p_clwk , APP_COMMON_BARICON_RETURN );
          GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,TRUE );
          GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
          SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE); 
        }
      }
    }
    break;
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
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
  s32   key_status;
  key_status = GFL_UI_TP_HitTrg( sc_touch_hit_tbl_poke_select );
  
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
        ONOUE_Printf("ポケモン選択：カーソル有効化処理実行\n");
        SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_NONE );
        PMSND_PlaySE( SE_TOP_TO_POKESELECT );
        return; 
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_TOUCH )
      {
        SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_TOUCH_INIT );
        return; 
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
  case 1:
    {
      // 音待ち
      if( !PMSND_CheckPlaySE() )
      {
        p_wk->sub_seq++;
      }
    }
    break;
  // キー入力処理
  case 2:
    {
      switch( key_status )
      {
      case TPID_PSELECT_RETURN:
        {
          // トップキー入力処理に移動
          SetNextSeq( p_wk ,MAIN_SEQ_RETURN_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_B );
          ONOUE_Printf("ポケモン選択：ゾーン詳細画面に戻る\n");
        }
        break;
      case TPID_PSELECT_EXPLANATIO:
        {
          // ポケモンを選択しているか調べる
          s32 mons_no = CheckIsExistPoke( p_wk ,p_wk->select_index );
          if( mons_no >= 0 )
          {
            SetNextSeq( p_wk ,MAIN_SEQ_APPEXIT ,SEQ_CHANGE_PARAM_POKE_SELECT );
            ONOUE_Printf("ポケモン選択：ポケモン選択[%d]\n",mons_no);
          }
        }
        break;
      case TPID_PS_LIST_CURSOR_01:
      case TPID_PS_LIST_CURSOR_02:
      case TPID_PS_LIST_CURSOR_03:
      case TPID_PS_LIST_CURSOR_04:
      case TPID_PS_LIST_CURSOR_05:
      case TPID_PS_LIST_CURSOR_06:
      case TPID_PS_LIST_CURSOR_07:
      case TPID_PS_LIST_CURSOR_08:
      case TPID_PS_LIST_CURSOR_09:
      case TPID_PS_LIST_CURSOR_10:
      case TPID_PS_LIST_CURSOR_11:
      case TPID_PS_LIST_CURSOR_12:
      case TPID_PS_LIST_CURSOR_13:
      case TPID_PS_LIST_CURSOR_14:
      case TPID_PS_LIST_CURSOR_15:
      case TPID_PS_LIST_CURSOR_16:
      case TPID_PS_LIST_CURSOR_17:
      case TPID_PS_LIST_CURSOR_18:
      case TPID_PS_LIST_CURSOR_19:
      case TPID_PS_LIST_CURSOR_20:
        {
          s32 index = key_status - TPID_PS_LIST_CURSOR_01;
          if(  index < p_wk->list_size )
          {
            p_wk->tp_select_index = index;
            SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_TOUCH );
            ONOUE_Printf("カーソル番号:%d\n",p_wk->tp_select_index +1);
          }
        }
        break;

        // タッチ入力なし
      case GFL_UI_TP_HIT_NONE:
        {
          // ポケモン選択キー入力処理
          CheckPokeSelectKeyInput( p_wk );
        }
        break;
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン選択キー入力チェック
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CheckPokeSelectKeyInput( HABITAT_ZONE_WK *p_wk )
{
  s32 key_trg = GFL_UI_KEY_GetTrg();
  s32 key_rep = GFL_UI_KEY_GetRepeat();

  if( p_wk->is_cursor_active == TRUE )
  {
    // 移動
    if( key_rep & PAD_KEY_LEFT )
    {
      SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_L );
      ONOUE_Printf("ポケモン選択：リスト左移動\n");
    }
    else if( key_rep & PAD_KEY_RIGHT )
    {
      SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_R );
      ONOUE_Printf("ポケモン選択：リスト右移動\n");
    }
    else if( key_rep & PAD_KEY_UP )
    {
      SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_UP );
      ONOUE_Printf("ポケモン選択：リスト上移動\n");
    }
    else if( key_rep & PAD_KEY_DOWN )
    {
      SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_DOWN );
      ONOUE_Printf("ポケモン選択：リスト下移動\n");
    }
    else if( key_trg & PAD_BUTTON_B )
    {
      // トップキー入力処理に移動
      SetNextSeq( p_wk ,MAIN_SEQ_RETURN_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_B );
      ONOUE_Printf("ポケモン選択：ゾーン詳細画面に戻る\n");
    }
    else if( key_trg & PAD_BUTTON_A )
    {
      // ポケモンを選択しているか調べる
      s32 mons_no = CheckIsExistPoke( p_wk ,p_wk->select_index );
      if( mons_no >= 0 )
      {
        SetNextSeq( p_wk ,MAIN_SEQ_APPEXIT ,SEQ_CHANGE_PARAM_POKE_SELECT );
      }
    }
  }
  else
  {
    if( key_trg & PAD_BUTTON_B )
    {
      SetNextSeq( p_wk ,MAIN_SEQ_RETURN_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_B );
      ONOUE_Printf("ポケモン選択：ゾーン詳細画面に戻る\n");
    }
    // カーソルがOFFの時にトリガー入力
    else if( key_trg )
    {
      PMSND_PlaySE( SE_CURSOR_MOVE );
      ONOUE_Printf("カーソルの有効化\n");
      SetNextSeq( p_wk ,MAIN_SEQ_MOVE_POKE_CURSOR ,SEQ_CHANGE_PARAM_NONE );
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
static void MAINSEQ_AppInit( HABITAT_ZONE_WK *p_wk )
{
  switch(p_wk->sub_seq)
  {
  case 0:
    {
      // フォントウィンドウのクリア
      HABITAT_ZONE_GRA_BmpFil( PRINT_UTIL_INFO_WIN , 0 ,&p_wk->gra_sys );

      // 通信アイコンを表示
      GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_ZUKAN_HABITAT_ZONE );

      // アプリケーションに必要なデータを取得する
      CreateAppData( p_wk );

      // リストデータを解析し、必要なデータを作成する
      MakeZoneCommonData( p_wk );

      // ポケモンアイコンリストを作成する
      RefreshLandPokeIcon( p_wk );

      // 地形アイコン更新
      RefreshLandTypeIcon( p_wk );

      // マップウィンドウ（上画面の更新）
      RefreshMapWin( p_wk );

      // コンプリートアイコンをセットする
      SetCompIcon( p_wk );

      // ショートカットアイコンを設定
      SetShortCutButtonCutAnmSeq( p_wk );

      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      // ポケモンアイコン転送待ち
      if( HABITAT_ZONE_GRA_CheckPokeIconTranceEnd(&p_wk->gra_sys) )
      {
        // トレイパレット転送チェック
        if( !HABITAT_ZONE_GRA_CheckTrayPalTrance( &p_wk->gra_sys ) )
        {
          // フェードシーケンスに移行
          SetNextSeq( p_wk ,MAIN_SEQ_FADEIN ,SEQ_CHANGE_PARAM_NONE );

          // メインシーケンスの状態を継続に設定
          p_wk->main_status = HABITAT_ZONE_MAIN_CONTINUE;
        }
      }
    }
    break;
  }
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
static void MAINSEQ_AppExit( HABITAT_ZONE_WK *p_wk )
{
  GFL_CLWK *p_clwk;

  // 終了ボタンにあわせて、アニメーション処理を実行する
  if( p_wk->seq_param == SEQ_CHANGE_PARAM_X )
  {
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_CANCEL );
    SetRetMod( p_wk ,HABITAT_ZONE_RET_EXIT );
  }
  else if( p_wk->seq_param == SEQ_CHANGE_PARAM_B )
  {
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_RETURN );
    SetRetMod( p_wk ,HABITAT_ZONE_RET_RETURN );
  }
  else if( p_wk->seq_param == SEQ_CHANGE_PARAM_POKE_SELECT )
  {
    p_clwk = HABITAT_ZONE_GRA_GetComClWk(&p_wk->gra_sys,OBJ_WK_BUTTON_EXPLANATION);
    SetRetMod( p_wk ,HABITAT_ZONE_RET_INFO );
    HABITATLIST_SAVE_SetDefaultGround( p_wk->p_habitat_sv ,p_wk->land_type );
    HABITATLIST_SAVE_SetDefaultSeason( p_wk->p_habitat_sv ,p_wk->season );
  }
  else
  {
    GF_ASSERT_MSG(0,"未対応の呼び出しパラメータ\n");
  }

  // 終了シーケンス
  switch(p_wk->sub_seq)
  {
  case 0:
    {
      // 閲覧中のゾーングループを設定
      HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
      HABITATLIST_SAVE_SetDefaultZoneGroupID(
          p_wk->p_habitat_sv,p_list_data->zone_group ); 

      // ボタンにあわせて、アニメーションシーケンスを設定する
      if( p_wk->seq_param == SEQ_CHANGE_PARAM_X )
      {
        GFL_CLACT_WK_SetAnmSeq( p_clwk , APP_COMMON_BARICON_EXIT_ON);
        PMSND_PlaySE( SE_EXIT );
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_B )
      {
        GFL_CLACT_WK_SetAnmSeq( p_clwk , APP_COMMON_BARICON_RETURN_ON);
        PMSND_PlaySE( SE_RETURN );
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_POKE_SELECT )
      {
        // ポケモンを選択しているか調べる
        s32 mons_no = CheckIsExistPoke( p_wk ,p_wk->select_index );
        if( mons_no >= 0 )
        {
          ZUKANSAVE_SetDefaultMons( p_wk->p_zkn_sv ,mons_no );
          ONOUE_Printf("ポケモン選択：ポケモン選択[%d]\n",mons_no);
        }
        else
        {
          GF_ASSERT(0);
        }
        GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_BUTTON_EXPLANATION_ANM );
        PMSND_PlaySE( SE_CALL_INFO );
      }
      GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
      GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,TRUE );
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      // アニメーション終了待ち
      if( GFL_CLACT_WK_CheckAnmActive( p_clwk ) == FALSE )
      {
        SetNextSeq( p_wk, MAIN_SEQ_FADEOUT ,SEQ_CHANGE_PARAM_NONE);
      }
    }
    break;
  }
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
  GFL_CLWK *p_no_anm_clwk;
  
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
  GF_ASSERT( p_list_data->season_enable[p_wk->land_type] == TRUE );

  // 移動方向にあわせて、CLWKを取得しておく
  if( p_wk->seq_param == SEQ_CHANGE_PARAM_L )
  {
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_L );
    p_no_anm_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_R );
  }
  else if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
  {
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_R );
    p_no_anm_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_L );
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
      // アニメションをとめる
      GFL_CLACT_WK_ResetAnm( p_no_anm_clwk );
      GFL_CLACT_WK_StopAnm( p_no_anm_clwk );

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

      // ポケモンアイコンリストを作成する
      RefreshLandPokeIcon( p_wk );

      // 地形アイコン更新
      RefreshLandTypeIcon( p_wk );

      // 決定音
      PMSND_PlaySE( SE_SEASON_CHANGE );

      // シーケンスを進める
      p_wk->sub_seq++;
    }
    // through break;
  case 1:
    {
      if( HABITAT_ZONE_GRA_CheckPokeIconTranceEnd(&p_wk->gra_sys) )
      {
        // トレイパレット転送チェック
        if( !HABITAT_ZONE_GRA_CheckTrayPalTrance( &p_wk->gra_sys ) )
        {
          if( GFL_CLACT_WK_CheckAnmActive( p_clwk ) == FALSE )
          {
            GFL_CLWK *p_clwk_no_anm;
            // アニメーションシーケンスを決定、アニメーションしていないほうと同じフレーム数に同期する
            if( p_wk->seq_param == SEQ_CHANGE_PARAM_L )
            {
              p_clwk_no_anm = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_R );
              GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_L );
              GFL_CLACT_WK_ResetAnm( p_clwk );
              GFL_CLACT_WK_ResetAnm( p_clwk_no_anm );
            }
            else if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
            {
              p_clwk_no_anm = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_L );
              GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_R );
              GFL_CLACT_WK_ResetAnm( p_clwk );
              GFL_CLACT_WK_ResetAnm( p_clwk_no_anm );
            }

            // アニメーションを開始
            GFL_CLACT_WK_StartAnm( p_no_anm_clwk );

            // キー入力状態に切替
            SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
          }
        }
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
      // 季節を戻す
      p_wk->season    = HABITATLIST_SAVE_GetDefaultSeason( p_wk->p_habitat_sv );

      // 地形を戻す
      p_wk->land_type = p_wk->default_land_type;

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
      GFL_CLACT_WK_StartAnm( p_clwk );
      
      // シーケンスを進める
      p_wk->sub_seq++;
    }
    // through break;
  case 1:
    {
//      // アニメーション停止
//      GFL_CLACT_WK_SetAnmSeq( p_clwk , default_anm_seq );
//      GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,TRUE );

      // リストデータを解析し、必要なデータを作成する
      MakeZoneCommonData( p_wk );

      // ポケモンアイコンリストを作成する
      RefreshLandPokeIcon( p_wk );

      // 地形アイコン更新
      RefreshLandTypeIcon( p_wk );

      // マップウィンドウ（上画面の更新）
      RefreshMapWin( p_wk );

      // コンプリートアイコンをセットする
      SetCompIcon( p_wk );

      // 決定音
      PMSND_PlaySE( SE_ZONE_CHANGE );

      // シーケンスを進める
      p_wk->sub_seq++;
    }
    // through break;
  case 2:
    {
      if( HABITAT_ZONE_GRA_CheckPokeIconTranceEnd(&p_wk->gra_sys) )
      {
        // トレイパレット転送チェック
        if( !HABITAT_ZONE_GRA_CheckTrayPalTrance( &p_wk->gra_sys ) )
        {
          // 文字描画終了を待ってキー入力状態に以降
          if( HABITAT_ZONE_GRA_CheckPrintUtilStatus( &p_wk->gra_sys ) )
          {
            HABITATLIST_STATUS event;
            event = CheckCompleteEventNowLandType( p_wk );
            if( event != HABITAT_ZONE_STATUS_NONE )
            {
              // イベントIDを設定する
              SetCompleteEvetnID( p_wk ,event );
              SetNextSeq( p_wk, MAIN_SEQ_COMPLETE_EVETN ,SEQ_CHANGE_PARAM_NONE);
            }
            else
            {
              // キー入力状態に切替(はんこアイコン表示)
              SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
            }
          }
        }
      }
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンプリートアイコンを表示
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetCompIcon( HABITAT_ZONE_WK *p_wk )
{
  GFL_CLWK *p_clwk;
  HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );
  s32 index;
  HABITATLIST_STATUS ls;
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_STAMP_BIG );

  // セーブデータの状態を取得する
  index = GetSaveDataIndex( p_zd->zone_group );
  ls = HABITATLIST_SAVE_GetZoneStatus( p_wk->p_habitat_sv,index,p_wk->land_type );

  switch( ls )
  {
  case HABITAT_ZONE_STATUS_NONE:
    {
      GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
    }
    break;
  case HABITAT_ZONE_STATUS_LOOK:
    {
      HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys ,OBJ_WK_STAMP_BIG ,OBJ_ANM_SEQ_STAMP_BIG_LOOK_STOP);
      GFL_CLACT_WK_SetBgPri( p_clwk ,1 );
    }
    break;
  case HABITAT_ZONE_STATUS_GET:
    {
      HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys ,OBJ_WK_STAMP_BIG ,OBJ_ANM_SEQ_STAMP_BIG_GET_STOP);
      GFL_CLACT_WK_SetBgPri( p_clwk ,1 );
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
          p_wk->land_type = HABITAT_GROUND_FISHING;
        }
        // 地形タイプのポケモンが存在しないときはスキップ
        if( p_wk->land_appear[p_wk->land_type] == FALSE )
        {
          ONOUE_Printf("左スキップ\n");
          SkipLandType( p_wk ,FALSE);
        }
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_R )
      {
        p_wk->land_type++;
        if( p_wk->land_type >= HABITAT_GROUND_MAX )
        {
          p_wk->land_type = HABITAT_GROUND_LAND;
        }
        // 地形タイプのポケモンが存在しないときはスキップ
        if( p_wk->land_appear[p_wk->land_type] == FALSE )
        {
          ONOUE_Printf("右スキップ\n");
          SkipLandType( p_wk ,TRUE);
        }
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_SET_LAND_TYPE )
      {
        // セットされている地形タイプに切り替える
        ONOUE_Printf("直接指定\n");
        GF_ASSERT( p_wk->land_appear[ p_wk->land_type ] == TRUE );
      }
      else 
      {
        GF_ASSERT(0);
      }

      // ポケモンアイコンリストを作成する
      RefreshLandPokeIcon( p_wk );

      // 地形アイコン更新
      RefreshLandTypeIcon( p_wk );
 
      // コンプリートアイコンをセットする
      SetCompIcon( p_wk );

      // 決定音
      PMSND_PlaySE( SE_GROUND_TYPE_CHANGE );
      
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

        GFL_CLACT_WK_SetAutoAnmFlag( p_clwk ,FALSE );
        GFL_CLACT_WK_SetAnmSeq( p_clwk , anm_seq[p_wk->land_type] );
        GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      if( HABITAT_ZONE_GRA_CheckPokeIconTranceEnd(&p_wk->gra_sys) )
      {
        // トレイパレット転送チェック
        if( !HABITAT_ZONE_GRA_CheckTrayPalTrance( &p_wk->gra_sys ) )
        {
          // 文字描画終了を待ってキー入力状態に以降
          if( HABITAT_ZONE_GRA_CheckPrintUtilStatus( &p_wk->gra_sys ) )
          {
            HABITATLIST_STATUS event;
            event = CheckCompleteEventNowLandType( p_wk );
            if( event != HABITAT_ZONE_STATUS_NONE )
            {
              // イベントIDを設定する
              SetCompleteEvetnID( p_wk ,event );
              SetNextSeq( p_wk, MAIN_SEQ_COMPLETE_EVETN ,SEQ_CHANGE_PARAM_NONE);
            }
            else
            {
              SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
            }

            // 地形位置を記憶
            p_wk->default_land_type = p_wk->land_type;
          }
          else
          {
            ONOUE_Printf("描画待ち\n");
          }
        }
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
    for( i = 0; i < HABITAT_GROUND_MAX; i++ )
    {
      p_wk->land_type++;
      if( p_wk->land_type >= HABITAT_GROUND_MAX )
      {
        p_wk->land_type = HABITAT_GROUND_LAND;
      }
      if( p_wk->land_appear[p_wk->land_type] == TRUE )
      {
        break;
      }
    }
  }
  else
  {
    for( i = 0; i < HABITAT_GROUND_MAX; i++ )
    {
      p_wk->land_type--;
      if( p_wk->land_type < 0 )
      {
        p_wk->land_type = HABITAT_GROUND_FISHING;
      }
      if( p_wk->land_appear[p_wk->land_type] == TRUE )
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
  switch( p_wk->sub_seq )
  {
  case 0:
    {
      s32 x = 0;
      s32 y = 0;
      s32 list_h = 0; 

      // 移動前のカーソル位置を保存
      s32 old_index = p_wk->select_index;

      // 現在のカーソル位置をOFFに
      if( p_wk->select_index < p_wk->list_size )
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
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_TOUCH )
      {
        ONOUE_Printf("ポケモンのみアクション\n");
        x = p_wk->tp_select_index%POKEICON_LINE;
        y = p_wk->tp_select_index/POKEICON_LINE;
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_TOUCH_INIT )
      {
        ONOUE_Printf("ポケモンのみアクション\n");
        x = p_wk->tp_select_index%POKEICON_LINE;
        y = p_wk->tp_select_index/POKEICON_LINE;
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
        else if( p_wk->seq_param == SEQ_CHANGE_PARAM_L )
        {
          // リストの範囲外の場合は一番最後を選択
          p_wk->select_index = p_wk->list_size -1;
        }
        else if( p_wk->seq_param == SEQ_CHANGE_PARAM_UP )
        {
          // 上に移動できる場合は、一つ上の枠をせんたくする
          y--;
          p_wk->select_index = x + y*POKEICON_LINE;
        }
        else if( p_wk->seq_param == SEQ_CHANGE_PARAM_DOWN )
        {
          y = 0;
          p_wk->select_index = x + y*POKEICON_LINE;
        }
        else
        {
          // リストの範囲外の場合は一番最後を選択
          p_wk->select_index = p_wk->list_size -1;
        }
      }

      // 新しいカーソル位置をONに
      if( p_wk->seq_param == SEQ_CHANGE_PARAM_TOUCH )
      {
        // ポケモンのみアクションさせる
        if( CheckIsExistPoke( p_wk ,p_wk->select_index ) >= 0 )
        {
          HABITAT_ZONE_GRA_SetPokeAnmFlag(&p_wk->gra_sys,p_wk->select_index,TRUE);
          // 動いている時のみ音を鳴らす
          if( old_index != p_wk->select_index )
          {
            PMSND_PlaySE( SE_CURSOR_MOVE );
          }
        }
        p_wk->is_cursor_active = FALSE;
        ONOUE_Printf("カーソルOFF\n");
      }
      else if( p_wk->seq_param == SEQ_CHANGE_PARAM_TOUCH_INIT )
      {
        // ポケモンのみアクションさせる
        if( CheckIsExistPoke( p_wk ,p_wk->select_index ) >= 0 )
        {
          HABITAT_ZONE_GRA_SetPokeAnmFlag(&p_wk->gra_sys,p_wk->select_index,TRUE);
          PMSND_PlaySE( SE_POKE_SELECT );
        }
        p_wk->is_cursor_active = FALSE;
        ONOUE_Printf("カーソルOFF\n");
      }
      else
      {
        SetCursorstatus( p_wk ,p_wk->select_index ,TRUE );
        ONOUE_Printf("カーソルON(選択インデックス%d)\n",p_wk->select_index);

        // カーソル有効化の時は移動音は鳴らさない
        if( p_wk->seq_param != SEQ_CHANGE_PARAM_NONE )
        {
          // 動いている時のみ音を鳴らす
          if( old_index != p_wk->select_index )
          {
            PMSND_PlaySE( SE_CURSOR_MOVE );
          }
        }
      }

      // ポケモンを選択しているか調べる
      // 説明を見るボタン書き換え
      {
        s32 mons_no = CheckIsExistPoke( p_wk ,p_wk->select_index );
        if( mons_no >= 0 )
        {
          SetExplanationButtonStatus( p_wk ,TRUE ,TRUE);
        }
        else
        {
          SetExplanationButtonStatus( p_wk ,FALSE ,TRUE);
        }
      }

      // サブシーケンスを進める
      p_wk->sub_seq++;
    }
    // through break;
  case 1:
    {
      SetNextSeq( p_wk, MAIN_SEQ_POKESELECT_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンプリートイベント
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_CompleteEvetn( HABITAT_ZONE_WK *p_wk )
{
  switch( p_wk->sub_seq )
  {
  case CMP_EVENT_ALL_CMP_CHECK:
    {
      if( p_wk->cmp_event_id == HABITAT_ZONE_STATUS_GET_ALL )
      {
        STRBUF  *str;
        STRBUF  *dest_str;
        
        s32 index;
        HABITAT_GROUND_TYPE land_type;
        HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );


        // ゾーンIDに対応した地名メッセージIDを取得
        HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
        u16 place_id = ZONEDATA_GetPlaceNameID(p_list_data->zone_group);

        ONOUE_Printf("捕獲コンプリート\n");

        // ワードセットに登録
        WORDSET_RegisterPlaceName( p_wk->p_word_set, 0, place_id );

        // ワードセット生成展開先バッファー作成
        dest_str = GFL_STR_CreateBuffer( 256, HEAPID_ZUKAN_HABITAT_ZONE );

        // メッセージの書き込み
        str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_seisoku_complete_01 );

        WORDSET_ExpandStr( p_wk->p_word_set, dest_str , str  );

        // 捕獲コンプリートフラグセット
        HABITATLIST_SAVE_SetGetCmpTutorialFlag(p_wk->p_habitat_sv);
        
        // BMPWIN作成
        HABITAT_ZONE_GRA_MakeEventBmpWinWin( &p_wk->gra_sys ,10 ,5 );

        // メッセージの書き込み
        HABITAT_ZONE_GRA_MakeFrameWinMsg( dest_str,&p_wk->gra_sys );
        
        // バッファーの消去 
        GFL_STR_DeleteBuffer( str );
        GFL_STR_DeleteBuffer( dest_str );

        // ファンファーレ再生
        PMSND_PushBGM();

        // ファンファーレ
        PMSND_PlayBGM( SEQ_ME_ZUKAN_01 );

        // 全コンプリートフラグセット
        index = GetSaveDataIndex( p_zd->zone_group );
        HABITATLIST_SAVE_SetZoneCompleteFlag( p_wk->p_habitat_sv,index );
        p_wk->sub_seq++;
      }
      else
      {
        // 全コンプリートをスキップ
        p_wk->sub_seq = CMP_EVENT_STAMP_START;
      }
    }
    break;
  case CMP_EVENT_FANFARE_CHECK:
    {
      // ファンファーレ待ち
      if( !PMSND_CheckPlayBGM() )
      {
        // 描画待ち
        if( HABITAT_ZONE_GRA_CheckPrintStreamStatus(&p_wk->gra_sys) == PRINTSTREAM_STATE_DONE )
        {
          // カーソル表示
          HABITAT_ZONE_GRA_EnableStrCursor( &p_wk->gra_sys );
          p_wk->sub_seq++;
        }
      }
    }
    break;
  case CMP_EVENT_ALL_CMP_KEY_CHECK:
    {
      // キー待ち
      if( (GFL_UI_KEY_GetTrg() & APP_PRINTSYS_COMMON_TRG_KEY) ||
          (GFL_UI_TP_GetTrg() == TRUE) )
      {
        s32 index;
        HABITAT_GROUND_TYPE land_type;
        HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );
        HABITATLIST_STATUS event;

        // BGMを戻す
        PMSND_PopBGM();

        // ウィンドウの削除
        HABITAT_ZONE_GRA_DeleteFrameWinMsg( &p_wk->gra_sys );

        // イベントが存在するかチェック
        event = CheckCompleteEventNowLandType( p_wk );

        // 全ての地形をコンプリート
        index = GetSaveDataIndex( p_zd->zone_group );
        for( land_type = 0; land_type < HABITAT_GROUND_MAX; land_type ++ )
        {
          HABITATLIST_SAVE_SetZoneStatus( p_wk->p_habitat_sv,
              index,land_type ,HABITAT_ZONE_STATUS_GET );
        }
        
        // はんこイベントを発生させるかチェック
        if( event != HABITAT_ZONE_STATUS_NONE )
        {
          p_wk->sub_seq++;
        }
        else
        {
          // イベントアイコンをリフレッシュ
          RefreshEventIcon( p_wk );
          // 通常キー入力に起動
          SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
          p_wk->sub_seq = CMP_EVENT_END;
        }
      }
    }
    break;
  case CMP_EVENT_STAMP_START:
    // はんこアニメーション再生
    {
      GFL_CLWK *p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_STAMP_BIG );
      if( (p_wk->cmp_event_id == HABITAT_ZONE_STATUS_GET) ||
          (p_wk->cmp_event_id == HABITAT_ZONE_STATUS_GET_ALL))
      {
        HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys ,OBJ_WK_STAMP_BIG ,OBJ_ANM_SEQ_STAMP_BIG_GET);
      }
      else
      {
        HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys ,OBJ_WK_STAMP_BIG ,OBJ_ANM_SEQ_STAMP_BIG_LOOK);
      }

      GFL_CLACT_WK_SetBgPri( p_clwk ,0 );
      p_wk->stump_se_frame = 0;
      p_wk->sub_seq++;
    }
    break;
  case CMP_EVENT_STAMP_END:
    // はんこアニメーション終了待ち
    {
      GFL_CLWK *p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_STAMP_BIG );
      // はんこSE
      if( p_wk->stump_se_frame == STUMP_SE_FRAME )
      {
        PMSND_PlaySE( SEQ_SE_SW_ZUKANSTUMP_01 );
      }
      p_wk->stump_se_frame++;
      // アニメーション終了待ち
      if( GFL_CLACT_WK_CheckAnmActive( p_clwk ) == FALSE )
      {
        GFL_CLACT_WK_SetBgPri( p_clwk ,1 );
        p_wk->sub_seq++;
      }
    }
    break;
  case CMP_EVENT_NEXT_STATE_CHECK:
    {
      s32 index;
      HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );

      // ゾーンのステータスを設定する(全コンプの時は事前に行っている)
      if( p_wk->cmp_event_id != HABITAT_ZONE_STATUS_GET_ALL )
      {
        index = GetSaveDataIndex( p_zd->zone_group );
        HABITATLIST_SAVE_SetZoneStatus( p_wk->p_habitat_sv,
            index,p_wk->land_type ,p_wk->cmp_event_id );
      }

      // イベントアイコンをリフレッシュ
      RefreshEventIcon( p_wk );
      
      // サブシーケンスを進めておく
      p_wk->sub_seq++;

      // 捕獲コンプリートチュートリアル判断
      if( ( p_wk->cmp_event_id == HABITAT_ZONE_STATUS_GET ) &&
          ( HABITATLIST_SAVE_GetGetCmpTutorialFlag(p_wk->p_habitat_sv) != TRUE ))
      {
        SetNextSeq( p_wk, MAIN_SEQ_COMPLETE_TUTORIAL ,SEQ_CHANGE_PARAM_NONE);
        return;
      }
      
      // 見たコンプリートチュートリアル判断
      if( ( p_wk->cmp_event_id == HABITAT_ZONE_STATUS_LOOK ) &&
          ( HABITATLIST_SAVE_GetLookCmpTutorialFlag(p_wk->p_habitat_sv) != TRUE ))
      {
        SetNextSeq( p_wk, MAIN_SEQ_COMPLETE_TUTORIAL ,SEQ_CHANGE_PARAM_NONE);
        return;
      }
      
      // 通常キー入力に起動
      SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンプリートイベントチュートリアル
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MAINSEQ_CompleteTutorial( HABITAT_ZONE_WK *p_wk )
{
  // チュートリアルイベントメイン
  switch(p_wk->sub_seq)
  {
  case 0:
    {
      STRBUF        *str;

      if( p_wk->cmp_event_id == HABITAT_ZONE_STATUS_GET )
      {
        ONOUE_Printf("捕獲コンプリートチュートリアル\n");
        // メッセージの読み込み
        str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_seisoku_explain_02 );
        // 捕獲コンプリートフラグセット
        HABITATLIST_SAVE_SetGetCmpTutorialFlag(p_wk->p_habitat_sv);
      }
      else
      {
        ONOUE_Printf("見たコンプリートチュートリアル\n");
        // メッセージの読み込み
        str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_seisoku_explain_01 );
        // メッセージの読み込み
        HABITATLIST_SAVE_SetLookCmpTutorialFlag(p_wk->p_habitat_sv);
      }
       
      // BMPWIN作成
      HABITAT_ZONE_GRA_MakeEventBmpWinWin( &p_wk->gra_sys ,8 ,9 );

      // メッセージの書き込み
      HABITAT_ZONE_GRA_MakeFrameWinMsg( str,&p_wk->gra_sys );
      GFL_STR_DeleteBuffer( str );

      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      // 描画待ち
      if( HABITAT_ZONE_GRA_CheckPrintStreamStatus(&p_wk->gra_sys) == PRINTSTREAM_STATE_DONE )
      {
        HABITAT_ZONE_GRA_EnableStrCursor( &p_wk->gra_sys );
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      s32 index;
      HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );

      // ウィンドウの削除
      HABITAT_ZONE_GRA_DeleteFrameWinMsg( &p_wk->gra_sys );

      // 通常キー入力に起動
      SetNextSeq( p_wk, MAIN_SEQ_TOP_KEY_INPUT ,SEQ_CHANGE_PARAM_NONE);

      p_wk->sub_seq++;
    }
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
  u32     msg_id;
  STRBUF  *str;
  STRBUF  *dest_str;

  // ゾーンIDに対応した地名メッセージIDを取得
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
  u16 place_id = ZONEDATA_GetPlaceNameID(p_list_data->zone_group);
 
  // 地形メッセージIDを取得
  if( p_wk->list_size > 0 )
  {
    msg_id = GetLandTypeMsgID( p_wk->land_type,p_list_data->info_ground_type);
  }
  else
  {
    // 生息していない場合用メッセージ
    msg_id = msg_seisoku_zone_09;
  }
 
  // 季節判定
  if( p_list_data->season_enable[p_wk->land_type] == TRUE )
  {
    msg_id += p_wk->season + 1; // 季節は1スタート
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
    LAND_MSG_POS_X , LAND_MSG_POS_Y ,LAND_MSG_W ,LAND_MSG_H ,FCOL_BP01WN );

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( dest_str );
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在の地形情報からメッセージIDを取得する
 *
 *	@param	type 
 *	@param	info_type
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetLandTypeMsgID( HABITAT_GROUND_TYPE type ,HABITAT_GROUND_INFO_TYPE info_type)
{
  u32 msg_id;
  const u32 msg_id_table[] =
  {
    msg_seisoku_zone_01,  // 草地
    msg_seisoku_zone_02,  // 砂地
    msg_seisoku_zone_03,  // 沼
    msg_seisoku_zone_04,  // 洞窟
    msg_seisoku_zone_08,  // 橋
    msg_seisoku_zone_07,  // 混合
    msg_seisoku_zone_09,  // なし
  };

  // メッセージを取得
  msg_id = GetLandTypeMsgIDSelect(type,info_type,
      msg_id_table,
      msg_seisoku_zone_05,
      msg_seisoku_zone_06);

  return msg_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  地形タイプを判断して、テーブルからメッセージIDを選択する
 *
 *	@param	type 
 *	@param	info_type 
 *	@param	u32 *p_ground_msg
 *	@param	sea_msg
 *	@param	fishing_msg
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetLandTypeMsgIDSelect( HABITAT_GROUND_TYPE type ,HABITAT_GROUND_INFO_TYPE info_type ,const u32 *p_ground_msg,u32 sea_msg,u32 fishing_msg)
{
  u32 msg_id = msg_seisoku_zone_01;

  // 場所ごとにメッセージを切り替える
  if( type == HABITAT_GROUND_LAND )
  {
    GF_ASSERT( info_type < HABITAT_GROUND_INFO_NONE );
    // 地形ごとにメッセージを選択
    msg_id = p_ground_msg[info_type];
  }
  else if( type == HABITAT_GROUND_WATERSIDE )
  {
    // 水辺
    msg_id = sea_msg;
   
  }
  else if( type == HABITAT_GROUND_FISHING )
  {
    // 釣り
    msg_id = fishing_msg;
  }
  else
  {
    GF_ASSERT( 0 );
  }

  return msg_id;
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
static void DrawExplanationMsg( HABITAT_ZONE_WK *p_wk ,PRINTSYS_LSB font_colro)
{
  STRBUF        *str;

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

  // ゾーンIDに対応した地名メッセージIDを取得
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );
  u16 place_id = ZONEDATA_GetPlaceNameID( p_list_data->zone_group );

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
      SBG_TITLE_FONT_COL );

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( dest_str );
}

////----------------------------------------------------------------------------
///**
// *	@brief  生息データがないメッセージ描画
// *
// *	@param	*p_wk 
// *
// *	@retval
// */
////-----------------------------------------------------------------------------
//static void DrawNoHabitatDatatMsg( HABITAT_ZONE_WK *p_wk )
//{
//  STRBUF       *str;
//
//  // メッセージの書き込み
//  str = GFL_MSG_CreateString( p_wk->p_app_msg , msg_seisoku_err_01 );
//
//  // メッセージの書き込み
//  HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_INFO_WIN ,str ,&p_wk->gra_sys ,
//      NOHABITATDATA_MSG_POS_X , NOHABITATDATA_MSG_POS_Y,
//      NOHABITATDATA_MSG_W,NOHABITATDATA_MSG_H,
//      FCOL_WP01WN );
//
//  // バッファーの消去 
//  GFL_STR_DeleteBuffer( str );
//}
//
////----------------------------------------------------------------------------
///**
// *	@brief  生息データがないメッセージクリア
// *
// *	@param	*p_wk 
// *
// *	@retval
// */
////-----------------------------------------------------------------------------
//static void ClearNoHabitatDataMsg( HABITAT_ZONE_WK *p_wk )
//{
//  // メッセージのクリア
//  HABITAT_ZONE_GRA_ClearWin( PRINT_UTIL_INFO_WIN ,&p_wk->gra_sys ,
//      NOHABITATDATA_MSG_POS_X , NOHABITATDATA_MSG_POS_Y,
//      NOHABITATDATA_MSG_W,NOHABITATDATA_MSG_H );
//}

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
  return &p_wk->p_init_param->zone_list_data[p_wk->zone_data_index];
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
    if( (p_pk[i].status == HABITAT_POKE_STATUS_NONEXISTENT) ||
        (p_pk[i].is_appear[p_wk->season][p_wk->land_type] == FALSE ) )
    {
      // 表示しないときは何もしない
    }
    else if( p_pk[i].status >= HABITAT_POKE_STATUS_LOOK )        // 見たか、捕獲のみ
    {

      u32 sex;
      u32 form;
      BOOL rare;
      BOOL is_look;

      // 閲覧中のフォルムデータ取得
      ZUKANSAVE_GetDrawData( p_wk->p_zkn_sv ,p_pk[i].monsno,&sex,&rare,&form,HEAPID_ZUKAN_HABITAT_ZONE);
      // 優先フォルムを見たことがあるかチェック
      is_look = ZUKANSAVE_CheckPokeSeeForm( p_wk->p_zkn_sv ,p_pk[i].monsno,sex,rare,p_pk[i].form[p_wk->season][p_wk->land_type]);

      if( is_look == TRUE )
      {
        form = p_pk[i].form[p_wk->season][p_wk->land_type];
      }

      // ポケモンアイコンをセット
      HABITAT_ZONE_GRA_SetPokeIcon( &p_wk->gra_sys ,p_pk[i].monsno,form,sex, index );

      // カーソルをOFF状態にする
      HABITAT_ZONE_GRA_SetCursorStatus( &p_wk->gra_sys ,
          index ,GRA_POKEICON_CURSOR_OFF );
      p_wk->monsno_list[index] = p_pk[i].monsno;

      // アイコン状態を設定
      if( p_pk[i].status == HABITAT_POKE_STATUS_LOOK )
      {
        HABITAT_ZONE_GRA_SetSmallIconStatus( &p_wk->gra_sys ,index ,
                                                GRA_SMALLICON_LOOK);
      }
      else if( p_pk[i].status == HABITAT_POKE_STATUS_GET )
      {
        HABITAT_ZONE_GRA_SetSmallIconStatus( &p_wk->gra_sys ,index ,
                                                GRA_SMALLICON_GET);
      }
      else
      {
        GF_ASSERT(0);
      }

      GF_ASSERT( index < HABITAT_ZONE_POKE_DATA_MAX );
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

    // ポケモンアイコンを非表示にする
    HABITAT_ZONE_GRA_SetSetDrawEnableIcon( &p_wk->gra_sys ,index ,FALSE );

    // 小アイコンを非表示にする
    HABITAT_ZONE_GRA_SetSmallIconStatus(&p_wk->gra_sys,index,GRA_SMALLICON_NONE);
    index++;

    // 空枠を１つにする場合はここでbreakする
    break;
  }

  // アイコンを非表示にする
  for( i = index; i < HABITAT_ZONE_POKE_LIST_MAX; i ++ )
  {
    // ポケモンアイコンを非表示にする
    HABITAT_ZONE_GRA_SetSetDrawEnableIcon( &p_wk->gra_sys ,i ,FALSE );

    // カーソルを見えない状態にする
    HABITAT_ZONE_GRA_SetCursorStatus( &p_wk->gra_sys ,
        i ,GRA_POKEICON_CURSOR_NONE );

    // 小アイコンを非表示にする
    HABITAT_ZONE_GRA_SetSmallIconStatus( &p_wk->gra_sys ,i ,GRA_SMALLICON_NONE);
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

  // 草地ボタンの点灯
  if( p_wk->land_type == HABITAT_GROUND_LAND )
  {
    HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                    GRA_LANDICON_TYPE_LAND,GRA_LANDICON_ON );
  }
  else
  {
    // 地形状況更新
    if( p_wk->land_appear[HABITAT_GROUND_LAND] == TRUE )
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
  if( p_wk->land_type == HABITAT_GROUND_WATERSIDE )
  {
    HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                    GRA_LANDICON_TYPE_WATERSIDE,GRA_LANDICON_ON );
  }
  else
  {
    // 地形状況更新
    if( p_wk->land_appear[HABITAT_GROUND_WATERSIDE] == TRUE )
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
  if( p_wk->land_type == HABITAT_GROUND_FISHING )
  {
    HABITAT_ZONE_GRA_SetLnadTypeIcon( &p_wk->gra_sys ,
                    GRA_LANDICON_TYPE_FISHING,GRA_LANDICON_ON );
  }
  else
  {
    // 地形状況更新
    if( p_wk->land_appear[HABITAT_GROUND_FISHING] == TRUE )
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

  // 地形タイプに合わせてパレットを転送
  HABITAT_ZONE_GRA_TransTrayColor( &p_wk->gra_sys,p_wk->land_type);

  // イベントアイコンをリフレッシュ
  RefreshEventIcon( p_wk );

  // 地名メッセージ書き込み
  DrawLandMsg( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベントアイコンをリフレッシュ
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RefreshEventIcon( HABITAT_ZONE_WK *p_wk )
{
  // 達成イベントアイコン表示
  GFL_CLWK *p_clwk;

  // 草地
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_EVENT_ICON_01 );
  SetEventIcon( p_wk ,HABITAT_GROUND_LAND ,p_clwk );

  // 海
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_EVENT_ICON_02 );
  SetEventIcon( p_wk ,HABITAT_GROUND_WATERSIDE ,p_clwk );

  // 釣り
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_EVENT_ICON_03 );
  SetEventIcon( p_wk ,HABITAT_GROUND_FISHING ,p_clwk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベントアイコンを設定
 *
 *	@param	*p_wk 
 *	@param	ground_type 
 *	@param	*p_clwk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetEventIcon( HABITAT_ZONE_WK *p_wk ,HABITAT_GROUND_TYPE ground_type ,GFL_CLWK *p_clwk )
{
  HABITATLIST_STATUS event;

  if( p_wk->land_appear[ground_type] == TRUE )
  {
    // 地形が存在
    event = CheckCompleteEvent( p_wk ,ground_type );
    if( event != HABITAT_ZONE_STATUS_NONE )
    {
      GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);
    }
    else
    {
      GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
    }
  }
  else
  {
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  }
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
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );

  // 地形タイプのポケモンが存在しないときはスキップ
  if( p_wk->land_appear[p_wk->land_type] == FALSE )
  {
    ONOUE_Printf("右スキップ\n");
    SkipLandType( p_wk ,TRUE);
  }

  // ポケモンアイコン作成
  MakePokeIconList( p_wk );

  // 生息地リスト
  {
    GFL_CLWK *p_clwk;
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_L );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,p_list_data->season_enable[p_wk->land_type]);
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_R );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,p_list_data->season_enable[p_wk->land_type]);
  }
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
  
  // ポケモンが存在する時のみ
  if( CheckIsExistPoke( p_wk ,index ) >= 0 )
  {
    HABITAT_ZONE_GRA_SetPokeAnmFlag( &p_wk->gra_sys ,index , is_active );
    ONOUE_Printf("ポケモンアニメーション開始,%d,%d\n",index,is_active);
  }
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
  
  // アイコンの開放状態を設定
  for( land_type = 0; land_type < HABITAT_GROUND_MAX; land_type ++ )
  {
    BOOL is_look_cmp = TRUE;
    BOOL is_get_cmp  = TRUE;
    for( i = 0; i < HABITAT_ZONE_POKE_LIST_MAX; i ++ )
    {
      // ポケモンが存在するかどうか
      if( p_pk[i].status >= HABITAT_POKE_STATUS_UNKNOWN )
      {
        for( season_id = 0; season_id < PMSEASON_TOTAL; season_id ++ )
        {
          // 出現するかどうか
          if( p_pk[i].is_appear[season_id][land_type] == TRUE )
          {
            //ONOUE_Printf("地形有り:%d\n",land_type);
            p_wk->land_appear[land_type] = TRUE;

            // 未発見の場合は、捕獲も見たもコンプリートではない
            if( p_pk[i].status == HABITAT_POKE_STATUS_UNKNOWN )
            {
              is_look_cmp  = FALSE;
              is_get_cmp   = FALSE;
            }
            // 見た状態のポケモンがいた場合、捕獲コンプリートはない
            else if( p_pk[i].status == HABITAT_POKE_STATUS_LOOK )
            {
              is_get_cmp   = FALSE;
            }
          }
        }

      }
    }

    // 優先度順にコンプリート状態を設定する
    // 捕獲コンプリート
    if( is_get_cmp == TRUE )
    {
      p_wk->land_complete_status[land_type] = HABITAT_ZONE_STATUS_GET;
      ONOUE_Printf("捕獲コンプリート\n");
    }
    // 見たコンプリート
    else if( is_look_cmp == TRUE )
    {
      p_wk->land_complete_status[land_type] = HABITAT_ZONE_STATUS_LOOK;
      ONOUE_Printf("見たコンプリート\n");
    }
    // コンプリートなし
    else
    {
      p_wk->land_complete_status[land_type] = HABITAT_ZONE_STATUS_NONE;
      ONOUE_Printf("コンプリートなし\n");
    }
  }

  if( p_wk->zone_data_cnt <= 1 )
  {
    GFL_CLWK *p_clwk;
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_UP_ARROW );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_DOWN_ARROW );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  }
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
  // イベントボタン
  HABITAT_ZONE_GRA_SetEventIconEnable( &p_wk->gra_sys ,FALSE );

  // 説明を見る
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_EXPLANATION );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);

  // 季節用左右ボタン(季節に関係なくOFF)
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_L );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_R );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);

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
  HABITAT_ZONE_DATA *p_list_data = GetNowListData( p_wk );

  // 陸地ボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_LAND );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,
      p_wk->land_appear[HABITAT_GROUND_LAND]);
  // 水辺ボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_WATERSIDE );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,
      p_wk->land_appear[HABITAT_GROUND_WATERSIDE]);
  // 釣りボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_FISHING );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,
      p_wk->land_appear[HABITAT_GROUND_FISHING]);

  if( p_wk->zone_data_cnt > 1 )
  {
    // 上矢印
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_UP_ARROW );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);
    // 下矢印
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_DOWN_ARROW );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);
  }
  else
  {
    // 上矢印
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_UP_ARROW );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
    // 下矢印
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_DOWN_ARROW );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  }
  // チェックボックス
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_CHECK_BOX );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);
  // キャンセル
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_CANCEL );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);

  // 季節用左右ボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_L );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,p_list_data->season_enable[p_wk->land_type]);
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_SEASON_R );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,p_list_data->season_enable[p_wk->land_type]);

  // 説明を見る
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,OBJ_WK_BUTTON_EXPLANATION );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);

  // イベントアイコンをリフレッシュ
  RefreshEventIcon( p_wk );

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
  ONOUE_Printf("zone_group = %d\n",p_list_data->zone_group);
  HABITAT_ZONE_GRA_SetMapBlink( &p_wk->gra_sys ,p_list_data->zone_group);

  // 主人公アイコンを表示する
  HABITAT_ZONE_GRA_SetHeroIcon( &p_wk->gra_sys );

  // マップ名を上画面に書き込む
  DrawMapNameMsg( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  戻りパラメータを設定する
 *
 *	@param	*p_wk 
 *	@param	ret_mode 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetRetMod( HABITAT_ZONE_WK *p_wk ,HABITAT_ZONE_RET_MODE ret_mode )
{
  p_wk->p_init_param->ret_mode = ret_mode;
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
static void CreateAppData( HABITAT_ZONE_WK *p_wk )
{
  s32 i;
  GAMEDATA *p_gdata = p_wk->p_init_param->p_gdata;
  ZUKAN_SAVEDATA* p_zkn_sv = GAMEDATA_GetZukanSave( p_gdata );
	SAVE_CONTROL_WORK *p_svcont = GAMEDATA_GetSaveControlWork( p_gdata );
  HABITATLIST_SAVE *p_habitat_sv = SaveData_GetHabitatListSave( p_svcont );
  u16 default_zone_group = HABITATLIST_SAVE_GetDefaultZoneGroupID( p_habitat_sv );
   
  // 初期化パラメータからデータを取得
  p_wk->p_zkn_sv        = p_zkn_sv;
  p_wk->p_habitat_sv    = p_habitat_sv; 
  p_wk->zone_data_cnt   = p_wk->p_init_param->zone_cnt;

  
  // データ位置を初期化
  p_wk->zone_data_index = 0;
  // 初期位置を検索する
  for( i = 0; i < p_wk->zone_data_cnt; i++ )
  {
    HABITAT_ZONE_DATA *p_data = 
      &p_wk->p_init_param->zone_list_data[i];
    if( default_zone_group == p_data->zone_group )
    {
      p_wk->zone_data_index = i;
    }
  }

  // default地形を設定
  p_wk->default_land_type = HABITATLIST_SAVE_GetDefaultGround( p_wk->p_habitat_sv );
  p_wk->season    = HABITATLIST_SAVE_GetDefaultSeason( p_wk->p_habitat_sv );
  p_wk->land_type = p_wk->default_land_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソル位置にポケモンが存在しているか調べる
 *
 *	@param	*p_wk 
 *	@param	index 
 *
 *	@retval 存在するときは モンスターナンバーを返し、それ以外は-1
 */
//-----------------------------------------------------------------------------
static s32 CheckIsExistPoke( HABITAT_ZONE_WK *p_wk ,u8 index )
{
  // 詳細画面に移動
  HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );
 
  if( p_wk->monsno_list[ index ] > 0 )
  {
    return p_wk->monsno_list[ index ];
  }
  return -1;
}

//----------------------------------------------------------------------------
/**
 *	@brief 達成イベント発生チェック
 *
 *	@param	*p_wk 
 *
 *	@retval 発生する場合はイベントID、しない場合はMAXを返す
 */
//-----------------------------------------------------------------------------
static HABITATLIST_STATUS CheckCompleteEventNowLandType( HABITAT_ZONE_WK *p_wk )
{
  s32 index;
  HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );

  // 全コンプリートチェック
  if( p_zd->is_zone_complete == TRUE )
  {
    index = GetSaveDataIndex( p_zd->zone_group );
    if( HABITATLIST_SAVE_GetZoneCompleteFlag( p_wk->p_habitat_sv,index ) == FALSE )
    {
      return HABITAT_ZONE_STATUS_GET_ALL;
    }
  }

  // 地形ごとのコンプリートチェック
  return CheckCompleteEvent( p_wk , p_wk->land_type );
}

//----------------------------------------------------------------------------
/**
 *	@brief  地形ごとの達成イベント発生チェック
 *
 *	@param	*p_wk
 *	@param	land_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static HABITATLIST_STATUS CheckCompleteEvent( HABITAT_ZONE_WK *p_wk,HABITAT_GROUND_TYPE land_type )
{
  s32 index;
  HABITATLIST_STATUS ls;
  HABITAT_ZONE_DATA *p_zd =  GetNowListData( p_wk );
  index = GetSaveDataIndex( p_zd->zone_group );
  ls = HABITATLIST_SAVE_GetZoneStatus( p_wk->p_habitat_sv,index,land_type );
 
  ONOUE_Printf("セーブの状態：%x\n",ls);
  ONOUE_Printf("コンプリートの状態：%x\n",p_zd->is_complete[land_type]);

  // セーブデータインデックスを取得する
  if( ls < p_zd->is_complete[land_type] )
  {
    ONOUE_Printf("地形コンプリートイベント発生\n");

    return p_zd->is_complete[land_type];
  }
  return HABITAT_ZONE_STATUS_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンプリートイベントIDを設定する
 *
 *	@param	*p_wk 
 *	@param	id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetCompleteEvetnID( HABITAT_ZONE_WK *p_wk ,HABITATLIST_STATUS id )
{
  GF_ASSERT( id != HABITAT_ZONE_STATUS_NONE );
  p_wk->cmp_event_id = id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーングループIDからセーブデータインデックスを取得する
 *
 *	@param	zone_group_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s32 GetSaveDataIndex( u16 zone_group_id )
{
  s32 i = 0;
  for( i = 0; i < HABITAT_LIST_DATA_MAX ; i++ )
  {
    if( sc_habitat_list_data[i].zone_group_id == zone_group_id )
    {
      return i;
    }
  }
  GF_ASSERT(0);
  return 0;
}

//----------------------------------------------------------------------------
/**
 *  @brief		ショートカット設定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetShortCut( HABITAT_ZONE_WK *p_wk )
{
  GAMEDATA *p_gdata = p_wk->p_init_param->p_gdata;
	if( GAMEDATA_GetShortCut( p_gdata, SHORTCUT_ID_ZUKAN_HABITAT_ZONE ) == TRUE ){
    GAMEDATA_SetShortCut( p_gdata, SHORTCUT_ID_ZUKAN_HABITAT_ZONE, FALSE );
    HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys,
        OBJ_WK_CHECK_BOX,APP_COMMON_BARICON_CHECK_OFF );
	}else{
		GAMEDATA_SetShortCut( p_gdata, SHORTCUT_ID_ZUKAN_HABITAT_ZONE, TRUE );
    HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys,
        OBJ_WK_CHECK_BOX,APP_COMMON_BARICON_CHECK_ON );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のショートカットボタンの状態を反映する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetShortCutButtonCutAnmSeq( HABITAT_ZONE_WK *p_wk )
{
  GAMEDATA *p_gdata = p_wk->p_init_param->p_gdata;
	if( GAMEDATA_GetShortCut( p_gdata, SHORTCUT_ID_ZUKAN_HABITAT_ZONE ) == TRUE ){
    HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys,
        OBJ_WK_CHECK_BOX,APP_COMMON_BARICON_CHECK_ON );
	}else{
    HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys,
        OBJ_WK_CHECK_BOX,APP_COMMON_BARICON_CHECK_OFF );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  地形移動が可能かどうか調べる
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckCanMoveLandType( HABITAT_ZONE_WK *p_wk )
{
  s32 cnt = 0;
  s32 i;
  for( i = 0; i < HABITAT_GROUND_MAX; i++ )
  {
    if( p_wk->land_appear[i] == TRUE )
    {
      cnt++;
    }
  }
  if( cnt > 1 )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  説明を見るボタンの状態を設定する
 *
 *	@param	*p_wk 
 *	@param	is_active   アクティブかどうか
 *	@param	is_visible  表示するかどうか(アクティブより優先度が高い),FALSEの時非表示
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetExplanationButtonStatus( HABITAT_ZONE_WK *p_wk ,BOOL is_active ,BOOL is_visible )
{
  GFL_CLWK *p_clwk;

  // セルアクターワークを取得する
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( &p_wk->gra_sys ,
      OBJ_WK_BUTTON_EXPLANATION );

  if( is_visible == FALSE)
  {
    // 説明を見るボタンを削除
    ClearExplanationMsg( p_wk );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  }
  else
  {
    // アニメーションシーケンスとメッセージ色を変えて表示
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE);
    if( is_active == TRUE )
    {
      DrawExplanationMsg( p_wk ,FCOL_WP01WN);
      HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys,
          OBJ_WK_BUTTON_EXPLANATION,OBJ_ANM_SEQ_BUTTON_EXPLANATION );
    }
    else
    {
      DrawExplanationMsg( p_wk ,FCOL_GP01WN);
      HABITAT_ZONE_GRA_SetAnmSeq( &p_wk->gra_sys,
          OBJ_WK_BUTTON_EXPLANATION,OBJ_ANM_SEQ_BUTTON_EXPLANATION_OFF );
    }
  }
}

