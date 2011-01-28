//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_util.c
 *	@brief  ローカルライブキャスター汎用モジュール
 *	@author	Toru=Nagihashi
 *	@data		2011.01.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/ds_system.h"
#include "system/rtc_tool.h"
#include "system/bmp_winframe.h"
#include "app/app_menu_common.h"
#include "sound/pm_sndsys.h"
#include "pokeicon/pokeicon.h"

//アーカイブ
#include "arc_def.h"
#include "comm_tvt.naix"
#include "local_tvt_graphic.naix"
#include "msg/msg_pokelist.h"

//自分のモジュール
#include "local_tvt_snd.h"
#include "local_tvt_graphic.h"

//外部公開
#include "local_tvt_util.h"

//=============================================================================
/**
 *					private
*/
//=============================================================================
//-------------------------------------
///	バーアイコン１つ読み込み用
//=====================================
enum
{
  RESID_OBJ_PLT,
  RESID_OBJ_NCG,
  RESID_OBJ_ANM,
  RESID_OBJ_MAX,
};

typedef struct
{
  GFL_CLWK  *p_clwk_return;
  u32 res[RESID_OBJ_MAX];
}BARICON;

static void BARICON_Init( BARICON *p_wk, GFL_CLUNIT *p_clunit, HEAPID heapID );
static void BARICON_Exit( BARICON *p_wk );
static GFL_CLWK * BARICON_GetClwk( const BARICON *cp_wk );

//=============================================================================
/**
 *					タイムゾーン
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  タイムゾーンを取得
 *
 *	@param	const GAMEDATA *cp_gamedata ゲームデータ
 *
 *	@return タイムゾーン
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_TIME_ZONE LOCAL_TVT_UTIL_GetTimeZone( const GAMEDATA *cp_gamedata )
{
  RTCTime time;
  const u8 season = GAMEDATA_GetSeasonID( cp_gamedata );
  const int morning = PM_RTC_GetTimeZoneChangeHour( season , TIMEZONE_MORNING );
  const int evening = PM_RTC_GetTimeZoneChangeHour( season , TIMEZONE_EVENING );
  const int night   = PM_RTC_GetTimeZoneChangeHour( season , TIMEZONE_NIGHT );

  GFL_RTC_GetTime( &time );

  if( time.hour >= morning && time.hour < evening )
  {
    return LTTZ_NOON;
  }
  else if( time.hour >= evening && time.hour < night )
  {
    return LTTZ_EVENING;
  }
  else
  {
    return LTTZ_NIGHT;
  }
}
//=============================================================================
/**
 *					背景素材
*/
//=============================================================================
//-------------------------------------
///	BG管理ワーク
//=====================================
struct _LOCAL_TVT_BG
{
  GFL_TCB *vBlankTcb;
} ;

//-------------------------------------
///	private
//=====================================
static void LOCAL_TVT_BG_VBlankFunc(GFL_TCB *tcb, void *wk_adrs );

//----------------------------------------------------------------------------
/**
 *	@brief  背景素材読込み
 *
 *	@param	const GAMEDATA *p_gamedata   ゲームデータ
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_BG * LOCAL_TVT_BG_Load( GAMEDATA *p_gamedata, HEAPID heapID )
{
 LOCAL_TVT_BG *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_BG ) ); 

  {
    MYSTATUS *myStatus = GAMEDATA_GetMyStatus( p_gamedata );
    ARCHANDLE *ctvtArcHandle = GFL_ARC_OpenDataHandle( ARCID_COMM_TVT_GRA, heapID );

    if( MyStatus_GetMySex( myStatus ) == PM_MALE )
    {
      GFL_ARCHDL_UTIL_TransVramPalette( ctvtArcHandle , NARC_comm_tvt_tv_t_tuuwa_bg2_NCLR , 
          PALTYPE_SUB_BG , 0*32 , 32*3 , heapID );
    }
    else
    {
      GFL_ARCHDL_UTIL_TransVramPalette( ctvtArcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NCLR , 
          PALTYPE_SUB_BG , 0*32 , 32*3 , heapID );
    }
    GFL_ARCHDL_UTIL_TransVramBgCharacter( ctvtArcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NCGR ,
        LTVT_FRAME_SUB_BG , 0 , 0, FALSE , heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( ctvtArcHandle , NARC_comm_tvt_tv_t_common_bg_NSCR , 
        LTVT_FRAME_SUB_BG ,  0 , 0, FALSE , heapID );
    GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_BG );

    GFL_ARCHDL_UTIL_TransVramPalette( ctvtArcHandle , NARC_comm_tvt_noise_bg_NCLR , 
        PALTYPE_MAIN_BG , LTVT_PLT_MAIN_NOIZE*32 , 32*1 , heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( ctvtArcHandle , NARC_comm_tvt_noise_bg_NCGR ,
        LTVT_FRAME_NOIZE_BG , 0 , 0, FALSE , heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( ctvtArcHandle , NARC_comm_tvt_noise_bg_NSCR , 
        LTVT_FRAME_NOIZE_BG ,  0 , 0, FALSE , heapID );
    GFL_BG_ChangeScreenPalette( LTVT_FRAME_NOIZE_BG , 0,0,32,32,LTVT_PLT_MAIN_NOIZE );
    GFL_BG_LoadScreenReq( LTVT_FRAME_NOIZE_BG );

    GFL_ARC_CloseDataHandle( ctvtArcHandle );
  }

  p_wk->vBlankTcb = GFUser_VIntr_CreateTCB( LOCAL_TVT_BG_VBlankFunc, p_wk, 0 );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	LOCAL_TVT_BG *p_bg BGワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_BG_UnLoad( LOCAL_TVT_BG *p_wk )
{
  GFL_TCB_DeleteTask( p_wk->vBlankTcb );

  GFL_HEAP_FreeMemory( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  BG操作用  VBLANK処理
 *
 *	@param	*tcb
 *	@param	*wk_adrs 
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_BG_VBlankFunc(GFL_TCB *tcb, void *wk_adrs )
{
  //上画面ノイズ
  GFL_BG_SetScroll( LTVT_FRAME_NOIZE_BG , GFL_BG_SCROLL_X_SET , GFUser_GetPublicRand0(256) );
  GFL_BG_SetScroll( LTVT_FRAME_NOIZE_BG , GFL_BG_SCROLL_Y_SET , GFUser_GetPublicRand0(256) );
}
//=============================================================================
/**
 *  下画面バー
 */
//=============================================================================


//-------------------------------------
///	ワーク
//=====================================
struct _LOCAL_TVT_TOUCHBAR
{
  BARICON bar_icon;
  BOOL  is_active;
  BOOL  is_trg;
  GFL_ARCUTIL_TRANSINFO bg;
};

//----------------------------------------------------------------------------
/**
 *	@brief  下画面バー作成  （バー用BGは下部だけ読み込むため、上部は別の用途に使える）
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_TOUCHBAR * LOCAL_TVT_TOUCHBAR_Init( GFL_CLUNIT *p_clunit, HEAPID heapID )
{
  LOCAL_TVT_TOUCHBAR *p_wk  = GFL_HEAP_AllocClearMemory(heapID,sizeof(LOCAL_TVT_TOUCHBAR));

  BARICON_Init( &p_wk->bar_icon, p_clunit, heapID );
  //リソース読込み
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId() , heapID );

    //BG
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, APP_COMMON_GetBarPltArcIdx() , 
        PALTYPE_SUB_BG , LTVT_PLT_BG_SUB_BAR*32 , 32 , heapID );

    p_wk->bg  = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, 
        APP_COMMON_GetBarCharArcIdx(),
        LTVT_FRAME_SUB_BAR , 0, FALSE , heapID );

    //いったんメモリにとり、下部のバーだけ転送（上部を書き換えないため）
    {
      void* p_scr_buff;
      int i;
      NNSG2dScreenData  *p_scr_data;
      p_scr_buff  = GFL_ARCHDL_UTIL_LoadScreen( p_handle, APP_COMMON_GetBarScrnArcIdx(), FALSE, &p_scr_data, heapID );
      for( i = 0; i < 32*24; i++ )
      {
        u16 *p_data = (u16*)p_scr_data->rawData;
        p_data[i]+=GFL_ARCUTIL_TRANSINFO_GetPos( p_wk->bg );
      }
      GFL_BG_WriteScreenExpand( LTVT_FRAME_SUB_BAR, 0,21,32,3, p_scr_data->rawData, 0,21, 32,24 );
      GFL_HEAP_FreeMemory( p_scr_buff );
    }
  

    GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_BAR , 0 , 21 , 32 , 3 , LTVT_PLT_BG_SUB_BAR );
    GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_BAR );
    GFL_ARC_CloseDataHandle( p_handle );
  }

  LOCAL_TVT_TOUCHBAR_SetActive( p_wk, TRUE);

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  下画面バー破棄
 *
 *	@param	LOCAL_TVT_TOUCHBAR *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_TOUCHBAR_Exit( LOCAL_TVT_TOUCHBAR *p_wk )
{

  GFL_BG_FreeCharacterArea(LTVT_FRAME_SUB_BAR,
                           GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->bg),
                           GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->bg));

  BARICON_Exit( &p_wk->bar_icon );

  //BGクリア
  GFL_BG_ClearFrame( LTVT_FRAME_SUB_BAR );
  GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_BAR );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  下画面バーメイン処理
 *
 *	@param	LOCAL_TVT_TOUCHBAR *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_TOUCHBAR_Main( LOCAL_TVT_TOUCHBAR *p_wk )
{
  if( p_wk->is_active == FALSE )
  {
    return;
  }

  {
    //戻るボタン
    static const GFL_UI_TP_HITTBL hitTbl[2] = 
    {
      {
        168 , 192 ,
        224 , 224+24 ,
      },
      {GFL_UI_TP_HIT_END,0,0,0}
    };
    const int ret = GFL_UI_TP_HitTrg( hitTbl );

    BOOL is_decide = FALSE;

    if( ret == 0 )
    {
      GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      is_decide = TRUE;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
      is_decide = TRUE;
    }

    if( is_decide )
    {
      GFL_CLACT_WK_SetAnmSeq( BARICON_GetClwk( &p_wk->bar_icon ), APP_COMMON_BARICON_RETURN_ON );
      PMSND_PlaySE( LOCAL_TVT_SE_CANCEL );
      p_wk->is_trg  = TRUE;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  表示設定
 *
 *	@param	LOCAL_TVT_TOUCHBAR *p_wk  ワーク
 *	@param	is_visible                TRUEで表示、FALSEで非表示
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_TOUCHBAR_SetVisible( LOCAL_TVT_TOUCHBAR *p_wk, BOOL is_visible )
{
  GFL_CLACT_WK_SetDrawEnable( BARICON_GetClwk( &p_wk->bar_icon ), is_visible );
  GFL_BG_SetVisible( LTVT_FRAME_SUB_BAR, is_visible );
}
//----------------------------------------------------------------------------
/**
 *	@brief  タッチしたかどうか
 *
 *	@param	const LOCAL_TVT_TOUCHBAR *cp_wk ワーク 
 *
 *	@return TRUEでタッチ  FALSEでしていない
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_TOUCHBAR_IsTrg( const LOCAL_TVT_TOUCHBAR *cp_wk )
{
  return cp_wk->is_trg;
}
//----------------------------------------------------------------------------
/**
 *	@brief  下画面バー  タッチ演出まち
 *
 *	@param	const LOCAL_TVT_TOUCHBAR *cp_wk   ワーク
 *
 *	@return TRUEで終了  FALSEで演出中
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_TOUCHBAR_IsEnd( const LOCAL_TVT_TOUCHBAR *cp_wk )
{
  return GFL_CLACT_WK_CheckAnmActive( BARICON_GetClwk( &cp_wk->bar_icon ) ) == FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  下画面バー  アクティブ設定
 *
 *	@param	LOCAL_TVT_TOUCHBAR *p_wk  ワーク
 *	@param	is_active TRUEアクティブ  FALSEノンアクティブ（反応なし）
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_TOUCHBAR_SetActive( LOCAL_TVT_TOUCHBAR *p_wk, BOOL is_active)
{
  p_wk->is_active = is_active;
  if( p_wk->is_active )
  {
    GFL_CLACT_WK_SetAnmSeq( BARICON_GetClwk( &p_wk->bar_icon ),APP_COMMON_BARICON_RETURN );
  }
  else
  {
    GFL_CLACT_WK_SetAnmSeq( BARICON_GetClwk( &p_wk->bar_icon ),APP_COMMON_BARICON_RETURN_OFF );
  }
}
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  シーケンス管理
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
/// シーケンスワーク
//=====================================
struct _LOCAL_TVT_SEQ
{
  LOCAL_TVT_SEQ_FUNCTION  seq_function;   //実行中のシーケンス関数
  int seq;                      //実行中のシーケンス関数の中のシーケンス
  void *p_wk_adrs;              //実行中のシーケンス関数に渡すワーク
  int reserv_seq;               //予約シーケンス
};

//-------------------------------------
/// パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 初期化
 *
 *  @param  LOCAL_TVT_SEQ *p_wk  ワーク
 *  @param  *p_param        パラメータ
 *  @param  seq_function    シーケンス
 *
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_SEQ * LOCAL_TVT_SEQ_Init( void *p_wk_adrs, LOCAL_TVT_SEQ_FUNCTION seq_function, HEAPID heapID )
{ 
  LOCAL_TVT_SEQ *p_wk;

  //作成
  p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_SEQ) );

  //初期化
  p_wk->p_wk_adrs = p_wk_adrs;

  //セット
  LOCAL_TVT_SEQ_SetNextCore( p_wk, seq_function );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 破棄
 *
 *  @param  LOCAL_TVT_SEQ *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_SEQ_Exit( LOCAL_TVT_SEQ *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ メイン処理
 *
 *  @param  LOCAL_TVT_SEQ *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_SEQ_Main( LOCAL_TVT_SEQ *p_wk )
{ 
  if( p_wk->seq_function )
  { 
    p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_wk_adrs );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 終了取得
 *
 *  @param  const LOCAL_TVT_SEQ *cp_wk   ワーク
 *
 *  @return TRUEならば終了  FALSEならば処理中
 */ 
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_SEQ_IsEnd( const LOCAL_TVT_SEQ *cp_wk )
{ 
  return cp_wk->seq_function == NULL;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 次のシーケンスを設定
 *
 *  @param  LOCAL_TVT_SEQ *p_wk  ワーク
 *  @param  seq_function    シーケンス
 *
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_SEQ_SetNextCore( LOCAL_TVT_SEQ *p_wk, LOCAL_TVT_SEQ_FUNCTION seq_function )
{ 
  p_wk->seq_function  = seq_function;
  p_wk->seq = 0;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 終了
 *
 *  @param  LOCAL_TVT_SEQ *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_SEQ_End( LOCAL_TVT_SEQ *p_wk )
{ 
  LOCAL_TVT_SEQ_SetNextCore( p_wk, NULL );
}

//=============================================================================
/**
 *					リスト
*/
//=============================================================================
//-------------------------------------
///	リストワーク
//=====================================
struct _LOCAL_TVT_LIST
{
  LOCAL_TVT_LIST_PARAM  param;
  GFL_BMPWIN            *p_selectwin[LOCAL_TVT_LIST_WINDOW_MAX];
  s32                   cursor_pos;
  u32                   seq;
  BOOL                  is_trg;
  u16                   plt_normal[0x10];
  u16                   plt_decide[0x10];
  u16                   plt_buff[0x10];
  u16                   anm_cnt;
  u16                   anm_frame;

  BARICON               bar_icon;
  LIST_UI               *p_ui;
};

static LIST_UI_RESULT Local_Tvt_List_IsTouch( void *p_wk_adrs );
static LIST_UI_RESULT Local_Tvt_List_IsKey( void *p_wk_adrs );
static void Local_Tvt_List_Change( void *p_wk_adrs, int touch_or_key );

//----------------------------------------------------------------------------
/**
 *	@brief  リスト作成
 *
 *	@param	const LOCAL_TVT_LIST_PARAM *cp_param  引数
 *	@param	heapID  ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_LIST * LOCAL_TVT_LIST_Init( const LOCAL_TVT_LIST_PARAM *cp_param, HEAPID heapID )
{
  LOCAL_TVT_LIST *p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_LIST ));
  p_wk->param       = *cp_param;
  p_wk->cursor_pos  = cp_param->cursor_pos;


  //リソース読み込み
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_LOCAL_TVT_GRAPHIC, heapID );
    //フレーム用キャラ
    {
      GFL_BG_SetClearCharacter( LTVT_FRAME_SUB_MSG, 32, 0, heapID );
      GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_local_tvt_graphic_frame_NCGR,
          LTVT_FRAME_SUB_MSG, 1, 0, FALSE, heapID );

    }

    //フレーム用パレットをメモリ上にも読み込み
    {
      void *p_plt_buff;
      const u16 *cp_plt_adrs;
      NNSG2dPaletteData *p_plt_data;

      p_plt_buff  = GFL_ARCHDL_UTIL_LoadPalette( p_handle, NARC_local_tvt_graphic_flame_NCLR, &p_plt_data, heapID );
      cp_plt_adrs = p_plt_data->pRawData;
      GFL_STD_MemCopy( (u8*)cp_plt_adrs + 0 * 0x20, p_wk->plt_normal, sizeof(u16) * 0x10 );
      GFL_STD_MemCopy( (u8*)cp_plt_adrs + 1 * 0x20, p_wk->plt_decide, sizeof(u16) * 0x10 );
      GFL_BG_LoadPalette( LTVT_FRAME_SUB_MSG, p_wk->plt_normal, sizeof(u16)*0x10, LTVT_PLT_BG_SUB_LIST * 32 );
      GFL_BG_LoadPalette( LTVT_FRAME_SUB_MSG, p_wk->plt_decide, sizeof(u16)*0x10, LTVT_PLT_BG_SUB_LIST_ON * 32 );

      GFL_HEAP_FreeMemory( p_plt_buff );
    }

    GFL_ARC_CloseDataHandle( p_handle );
  }

  //アイコン読み込み
  BARICON_Init( &p_wk->bar_icon, p_wk->param.p_clunit, heapID );
  {
    GFL_CLACTPOS  pos;
    pos.x = 200-16;
    pos.y = 24+(p_wk->param.max-1)*40 - 4;
    GFL_CLACT_WK_SetPos( BARICON_GetClwk( &p_wk->bar_icon ), &pos, CLSYS_DRAW_SUB );
    GFL_CLACT_WK_SetAnmSeq( BARICON_GetClwk( &p_wk->bar_icon ),APP_COMMON_BARICON_RETURN );
  }


  //BMPWIN作成
  {
    int i;
    int x, y, w, h;
    for( i = 0; i < p_wk->param.max; i++ )
    {
      x = 5;
      y = 3 + 5 * i;
      w = 21;
      h = 2;
      p_wk->p_selectwin[i]  = GFL_BMPWIN_Create( LTVT_FRAME_SUB_MSG, 
          x, y, w, h, LTVT_PLT_BG_SUB_LIST, GFL_BG_CHRAREA_GET_B );

      //文字
      LOCAL_TVT_MESSAGE_SetPrintColor( p_wk->param.p_message, PRINTSYS_MACRO_LSB(0xf,0xE,3) );
      LOCAL_TVT_MESSAGE_SetPrintPos( p_wk->param.p_message, 0, 0, LOCAL_TVT_MESSAGE_POS_CENTER );
      LOCAL_TVT_MESSAGE_PrintMsg( p_wk->param.p_message, p_wk->p_selectwin[i], p_wk->param.cp_msgdata, p_wk->param.msgID[i] );
      GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_selectwin[i] );
      BmpWinFrame_Write( p_wk->p_selectwin[i], WINDOW_TRANS_ON_V, 1, LTVT_PLT_BG_SUB_LIST );
    }
  }

  //UI
  {
    LIST_UI_PARAM param;
    GFL_STD_MemClear( &param, sizeof(LIST_UI_PARAM) );
    param.p_wk_adrs   = p_wk;
    param.UpdateTouch = Local_Tvt_List_IsTouch;
    param.UpdateKey   = Local_Tvt_List_IsKey;
    param.Change      = Local_Tvt_List_Change;

    p_wk->p_ui  = LIST_UI_Init( &param, heapID );
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト破棄
 *
 *	@param	LOCAL_TVT_LIST *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_LIST_Exit( LOCAL_TVT_LIST *p_wk )
{
  //UI
  LIST_UI_Exit( p_wk->p_ui );
    
  //BMPWIN破棄
  {
    int i;
    for( i = 0; i < p_wk->param.max; i++ )
    {
      GFL_BMPWIN_Delete( p_wk->p_selectwin[i] );
    }
    GFL_BG_ClearScreen( LTVT_FRAME_SUB_MSG );
    GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_MSG );
  }

  //アイコン破棄
  BARICON_Exit( &p_wk->bar_icon );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リストメイン処理
 *
 *	@param	LOCAL_TVT_LIST *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_LIST_Main( LOCAL_TVT_LIST *p_wk )
{
  enum
  {
    SEQ_MAIN,
    SEQ_ANM,
    SEQ_END,
  };

  switch( p_wk->seq )
  {
  case SEQ_MAIN:
    {
      //UIメイン処理
      LIST_UI_Main( p_wk->p_ui );

      //決定のとき
      if( LIST_UI_IsDecide( p_wk->p_ui ) )
      {
        p_wk->anm_cnt     = 0;
        p_wk->anm_frame   = 0;
        p_wk->seq         = SEQ_ANM;

        //パレットの色を戻す
        GFL_BG_LoadPalette( LTVT_FRAME_SUB_MSG, p_wk->plt_decide, sizeof(u16)*0x10, LTVT_PLT_BG_SUB_LIST_ON * 32 );

        //いったん全部選択解除
        {
          int i;
          for( i = 0; i < p_wk->param.max; i++ )
          {
            GFL_BMPWIN  *p_bmpwin = p_wk->p_selectwin[ i ];
            u8  x = GFL_BMPWIN_GetPosX(p_bmpwin)-1;
            u8  y = GFL_BMPWIN_GetPosY(p_bmpwin)-1;
            u8  w = GFL_BMPWIN_GetSizeX(p_bmpwin)+2;
            u8  h = GFL_BMPWIN_GetSizeY(p_bmpwin)+2;
            GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_MSG,
                x, y, w, h, LTVT_PLT_BG_SUB_LIST );
          }
          break;
        }
      }

      if ( p_wk->anm_frame + 0x400 >= 0x10000 )
      {
        p_wk->anm_frame = p_wk->anm_frame + 0x400 - 0x10000;
      }
      else
      {
      p_wk->anm_frame += 0x400;
      }
      LOCAL_TVT_UTIL_MainPltAnmLine( NNS_GFD_DST_2D_BG_PLTT_SUB, p_wk->plt_buff, p_wk->anm_frame, LTVT_PLT_BG_SUB_LIST_ON, p_wk->plt_normal, p_wk->plt_decide );
    }
    break;

  case SEQ_ANM:
    {
      //選択しているものだけアニメーション
      GFL_BMPWIN  *p_bmpwin = p_wk->p_selectwin[ p_wk->cursor_pos ];
      u8  x = GFL_BMPWIN_GetPosX(p_bmpwin)-1;
      u8  y = GFL_BMPWIN_GetPosY(p_bmpwin)-1;
      u8  w = GFL_BMPWIN_GetSizeX(p_bmpwin)+2;
      u8  h = GFL_BMPWIN_GetSizeY(p_bmpwin)+2;
      u8  plt;
 
      if( (p_wk->anm_cnt/4)%2 )
      {
        plt = LTVT_PLT_BG_SUB_LIST_ON;
      }
      else
      {
        plt = LTVT_PLT_BG_SUB_LIST;
      }
      GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_MSG,
          x, y, w, h, plt );
      GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_MSG );
    }

    if( p_wk->anm_cnt++ > 16 )
    {
      p_wk->seq = SEQ_END;
    }
    break;

  case SEQ_END:
    p_wk->is_trg  = TRUE;
    break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト終了検知
 *
 *	@param	const LOCAL_TVT_LIST *cp_wk   ワーク
 *
 *	@return TRUE選択した  FALSEまだ
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_LIST_IsEnd( const LOCAL_TVT_LIST *cp_wk )
{
  return cp_wk->is_trg;
}
//----------------------------------------------------------------------------
/**
 *	@brief  選んだ物を取得
 *
 *	@param	const LOCAL_TVT_LIST *cp_wk   ワーク
 *
 *	@return 選んだインデックス
 */
//-----------------------------------------------------------------------------
s32 LOCAL_TVT_LIST_GetSelect( const LOCAL_TVT_LIST *cp_wk )
{
  return cp_wk->cursor_pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief  タッチしたかどうか
 *
 *	@param	const LOCAL_TVT_LIST *cp_wk ワーク
 *	@param	*p_index  インデックス
 *
 *	@return 結果
 */
//-----------------------------------------------------------------------------
static LIST_UI_RESULT Local_Tvt_List_IsTouch( void *p_wk_adrs )
{
  LOCAL_TVT_LIST *p_wk  = p_wk_adrs;
  //戻るボタン
  static const GFL_UI_TP_HITTBL hitTbl[4+1] = 
  {
    {
      2*8 , 2*8+4*8 ,
      4*8 , 4*8+23*8,
    },
    {
      7*8, 7*8+4*8 ,
      4*8, 4*8+23*8,
    },
    {
      12*8, 12*8+4*8 ,
      4*8, 4*8+23*8,
    },
    {
      17*8, 17*8+4*8 ,
      4*8, 4*8+23*8,
    },
    {GFL_UI_TP_HIT_END,0,0,0}
  };
  const int ret = GFL_UI_TP_HitTrg( hitTbl );

  if( ret != GFL_UI_TP_HIT_NONE && ret < p_wk->param.max )
  {
    PMSND_PlaySE( LOCAL_TVT_SE_DECIDE );
    p_wk->cursor_pos  = ret;
    return LIST_UI_RESULT_DECIDE;
  }

  return LIST_UI_RESULT_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  キーしたかどうか
 *
 *	@param	void *p_wk_adrs ワーク
 *
 *	@return 結果
 */
//-----------------------------------------------------------------------------
static LIST_UI_RESULT Local_Tvt_List_IsKey( void *p_wk_adrs )
{
  LOCAL_TVT_LIST *p_wk  = p_wk_adrs;
  BOOL is_key_move  = FALSE;
  BOOL is_decide    = FALSE;

  //上下移動
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
  {
    p_wk->cursor_pos--;
    if( p_wk->cursor_pos < 0 )
    {
      p_wk->cursor_pos  = p_wk->param.max-1;
    }
    PMSND_PlaySE( LOCAL_TVT_SE_SELECT );
    is_key_move = TRUE;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
  {
    p_wk->cursor_pos++;
    p_wk->cursor_pos  %= p_wk->param.max;

    PMSND_PlaySE( LOCAL_TVT_SE_SELECT );
    is_key_move = TRUE;
  }

  //決定
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {

    PMSND_PlaySE( LOCAL_TVT_SE_DECIDE );
    is_decide = TRUE;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
  {
    PMSND_PlaySE( LOCAL_TVT_SE_CANCEL );
    is_decide = TRUE;
    p_wk->cursor_pos  = p_wk->param.max-1;
  }

  //カーソル移動
  if( is_key_move )
  {
    int i;
    u8 plt;
    //選択を変更
    for( i = 0; i < p_wk->param.max; i++ )
    {
      if( i == p_wk->cursor_pos )
      {
        plt = LTVT_PLT_BG_SUB_LIST_ON;
      }
      else
      {
        plt = LTVT_PLT_BG_SUB_LIST;
      }

      {
        GFL_BMPWIN  *p_bmpwin = p_wk->p_selectwin[ i ];
        u8  x = GFL_BMPWIN_GetPosX(p_bmpwin)-1;
        u8  y = GFL_BMPWIN_GetPosY(p_bmpwin)-1;
        u8  w = GFL_BMPWIN_GetSizeX(p_bmpwin)+2;
        u8  h = GFL_BMPWIN_GetSizeY(p_bmpwin)+2;
        GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_MSG,
            x, y, w, h, plt );
      }
    }

    //アニメを最初からにする
    p_wk->anm_frame = 0;

    GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_MSG );
  }

  //終了
  if( is_decide )
  {
    return LIST_UI_RESULT_DECIDE;
  }
  else if( is_key_move )
  {
    return LIST_UI_RESULT_MOVE;
  }
  else
  {
    return LIST_UI_RESULT_NONE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  タッチとキーの切り替え関数
 *
 *	@param	void *p_wk_adrs ワーク
 *	@param	touch_or_key    切り替え後のGFL_APP_END_TOUCH　or GFL_APP_END_KEY
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_List_Change( void *p_wk_adrs, int touch_or_key )
{
  LOCAL_TVT_LIST *p_wk  = p_wk_adrs;

  if( touch_or_key == GFL_APP_END_TOUCH )
  {
    //タッチモードになった

    int i;
    //選択をすべて解除
    for( i = 0; i < p_wk->param.max; i++ )
    {
      GFL_BMPWIN  *p_bmpwin = p_wk->p_selectwin[ i ];
      u8  x = GFL_BMPWIN_GetPosX(p_bmpwin)-1;
      u8  y = GFL_BMPWIN_GetPosY(p_bmpwin)-1;
      u8  w = GFL_BMPWIN_GetSizeX(p_bmpwin)+2;
      u8  h = GFL_BMPWIN_GetSizeY(p_bmpwin)+2;
      GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_MSG,
          x, y, w, h, LTVT_PLT_BG_SUB_LIST );
    }
    GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_MSG );
  }
  else
  {
    //キーモードになった
    
    //カーソル位置を選択表示
    GFL_BMPWIN  *p_bmpwin = p_wk->p_selectwin[ p_wk->cursor_pos ];
    u8  x = GFL_BMPWIN_GetPosX(p_bmpwin)-1;
    u8  y = GFL_BMPWIN_GetPosY(p_bmpwin)-1;
    u8  w = GFL_BMPWIN_GetSizeX(p_bmpwin)+2;
    u8  h = GFL_BMPWIN_GetSizeY(p_bmpwin)+2;
    GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_MSG,
        x, y, w, h, LTVT_PLT_BG_SUB_LIST_ON );
    GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_MSG );
  }
}

//=============================================================================
/**
 *					ポケモンリスト
*/
//=============================================================================
static const GFL_UI_TP_HITTBL sc_pokelist_hit_tbl[]  =
{
  {
    1*8,6*8,
    0*8,15*8
  },
  {
    3*8,8*8,
    16*8,31*8
  },
  {
    7*8,12*8,
    0*8,15*8
  },
  {
    9*8,14*8,
    16*8,31*8
  },
  {
    13*8,18*8,
    0*8,15*8
  },
  {
    15*8,20*8,
    16*8,31*8
  },
};

#define POKELIST_X( x ) (sc_pokelist_hit_tbl[x].rect.left/8)
#define POKELIST_Y( x ) (sc_pokelist_hit_tbl[x].rect.top/8)
#define POKELIST_W( x ) (sc_pokelist_hit_tbl[x].rect.right/8-sc_pokelist_hit_tbl[x].rect.left/8+1)
#define POKELIST_H( x ) (sc_pokelist_hit_tbl[x].rect.bottom/8-sc_pokelist_hit_tbl[x].rect.top/8+1)

enum
{
  POKELIST_ROW_MAX  = 3,
  POKELIST_COL_MAX  = 2,
};

enum
{
  POKELIST_RESID_CEL,
  POKELIST_RESID_PLT,
  POKELIST_RESID_NCG_TOP,
  POKELIST_RESID_NCG_END  = POKELIST_RESID_NCG_TOP + 6,
  POKELIST_RESID_NCG_MAX
};

static const int sc_pokelist_pos[POKELIST_ROW_MAX][POKELIST_COL_MAX]  = 
{
  {0,1},
  {2,3},
  {4,5},
};

//-------------------------------------
///	ポケモンリストワーク
//=====================================
struct _LOCAL_TVT_POKELIST
{
  LOCAL_TVT_POKELIST_PARAM  param;
  LOCAL_TVT_POKELIST_RESULT result;
  LOCAL_TVT_TOUCHBAR  *p_touchbar;
  GFL_ARCUTIL_TRANSINFO plate_bg;
  u16   plt_normal[0x10];
  u16   plt_decide[0x10];
  u16   plt_nopush[0x10];
  u16   plt_buff[0x10];
  u32   seq;
  s32   cursor_pos;
  u16   anm_frame;
  u16   anm_cnt;
  BOOL  is_active;
  LIST_UI *p_ui;
  GFL_BMPWIN  *p_bmpwin[6];
  GFL_CLWK    *p_clwk_poke[6];
  u16   res[POKELIST_RESID_NCG_MAX];
  GFL_TCB    *p_set_visible_tcb;
  BOOL is_visible_req;
};
static void Local_Tvt_Pokelist_ClearList( LOCAL_TVT_POKELIST *p_wk, BOOL is_cursor );

static void Local_Tvt_Pokelist_GetRowColIndex( u32 cursor_pos, s16 *p_row, s16 *p_col );
static void Local_Tvt_Pokelist_GetRowColIndexMax( u32 cursor_pos, u32 cursor_max, s16 *p_row, s16 *p_col );
static LIST_UI_RESULT Local_Tvt_Pokelist_TouchUpdate( void *p_wk_adrs );
static LIST_UI_RESULT Local_Tvt_Pokelist_KeyUpdate( void *p_wk_adrs );
static void Local_Tvt_Pokelist_Change( void *p_wk_adrs, int touch_or_key );
static void Local_Tvt_Pokelist_SetVisibleVBlank( GFL_TCB *p_tcb, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンリスト作成
 *
 *	@param	const LOCAL_TVT_POKELIST_PARAM *cp_param  引数
 *	@param	heapID  ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_POKELIST * LOCAL_TVT_POKELIST_Init( const LOCAL_TVT_POKELIST_PARAM *cp_param, HEAPID heapID )
{
  LOCAL_TVT_POKELIST *p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_POKELIST));
  p_wk->param   = *cp_param;
  p_wk->cursor_pos  = 0;
  p_wk->is_active = TRUE;

  GFL_BG_SetVisible( LTVT_FRAME_SUB_BAR, FALSE );
  GFL_BG_SetVisible( LTVT_FRAME_SUB_MISC, FALSE );
  GFL_BG_SetVisible( LTVT_FRAME_SUB_MSG, FALSE );

  //リソース読み込み
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_LOCAL_TVT_GRAPHIC, heapID );

    //ポケモンリストはバーと同じBGに読み込むため、AreaManを使う

    {
      void *p_plt_buff;
      const u16 *cp_plt_adrs;
      NNSG2dPaletteData *p_plt_data;

      p_plt_buff  = GFL_ARCHDL_UTIL_LoadPalette( p_handle, NARC_local_tvt_graphic_pokelist_NCLR, &p_plt_data, heapID );
      cp_plt_adrs = p_plt_data->pRawData;
      GFL_STD_MemCopy( (u8*)cp_plt_adrs + 0 * 0x20, p_wk->plt_normal, sizeof(u16) * 0x10 );
      GFL_STD_MemCopy( (u8*)cp_plt_adrs + 1 * 0x20, p_wk->plt_decide, sizeof(u16) * 0x10 );
      GFL_STD_MemCopy( (u8*)cp_plt_adrs + 2 * 0x20, p_wk->plt_nopush, sizeof(u16) * 0x10 );

      GFL_BG_LoadPalette( LTVT_FRAME_SUB_BAR, p_wk->plt_normal, sizeof(u16)*0x10, LTVT_PLT_BG_SUB_LIST * 32 );
      GFL_BG_LoadPalette( LTVT_FRAME_SUB_BAR, p_wk->plt_decide, sizeof(u16)*0x10, LTVT_PLT_BG_SUB_LIST_ON * 32 );
      GFL_BG_LoadPalette( LTVT_FRAME_SUB_BAR, p_wk->plt_nopush, sizeof(u16)*0x10, LTVT_PLT_BG_SUB_LIST_OFF * 32 );

      GFL_HEAP_FreeMemory( p_plt_buff );
    }

    p_wk->plate_bg  = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle,
        NARC_local_tvt_graphic_pokelist_NCGR, LTVT_FRAME_SUB_BAR, 0, FALSE, heapID );

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs( p_handle,
        NARC_local_tvt_graphic_pokelist_NSCR, LTVT_FRAME_SUB_BAR, 0, 
        GFL_ARCUTIL_TRANSINFO_GetPos( p_wk->plate_bg ),
        0, FALSE, heapID );

    //初期リスト設定
    Local_Tvt_Pokelist_ClearList( p_wk, FALSE );

    //背景
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_local_tvt_graphic_back_pokelist_NCLR, 
        PALTYPE_SUB_BG , LTVT_PLT_BG_SUB_BACK*32 , 32, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_local_tvt_graphic_back_pokelist_NCGR,
        LTVT_FRAME_SUB_MISC , 0 , 0, FALSE , heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_local_tvt_graphic_back_pokelist_NSCR, 
        LTVT_FRAME_SUB_MISC,  0 , 0, FALSE , heapID );
    GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_MISC, 0 , 0, 32 , 24, LTVT_PLT_BG_SUB_BACK );
    GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_MISC );

    GFL_ARC_CloseDataHandle( p_handle );
  }
  
  //タッチバー作成
  p_wk->p_touchbar  = LOCAL_TVT_TOUCHBAR_Init( p_wk->param.p_clunit, heapID );
  LOCAL_TVT_TOUCHBAR_SetVisible( p_wk->p_touchbar, FALSE );

  //UI
  {
    LIST_UI_PARAM param;
    GFL_STD_MemClear( &param, sizeof(LIST_UI_PARAM));
    param.p_wk_adrs = p_wk;
    param.UpdateTouch = Local_Tvt_Pokelist_TouchUpdate;
    param.UpdateKey   = Local_Tvt_Pokelist_KeyUpdate;
    param.Change      = Local_Tvt_Pokelist_Change;
    p_wk->p_ui        = LIST_UI_Init( &param, heapID );
  }

  //ポケモンアイコン
  {
    int i;
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_POKEICON, heapID );

    p_wk->res[POKELIST_RESID_PLT] = GFL_CLGRP_PLTT_RegisterComp( p_handle , 
        POKEICON_GetPalArcIndex() , CLSYS_DRAW_SUB , 
        LTVT_PLT_OBJ_SUB_POKEMON*32 , heapID  );

    p_wk->res[POKELIST_RESID_CEL] = GFL_CLGRP_CELLANIM_Register( p_handle , 
        POKEICON_GetCellSubArcIndex() , 
        POKEICON_GetAnmSubArcIndex(), heapID  );

    for( i = 0; i < PokeParty_GetPokeCount(p_wk->param.p_party); i++ )
    {
      GFL_CLWK_DATA clwk_data;
      POKEMON_PARAM *p_pp  = PokeParty_GetMemberPointer( p_wk->param.p_party, i );
      u32 cgr = POKEICON_GetCgxArcIndex( PP_GetPPPPointerConst(p_pp) );
      u8  plt = POKEICON_GetPalNumGetByPPP(PP_GetPPPPointerConst(p_pp));

      GFL_STD_MemClear( &clwk_data , sizeof(GFL_CLWK_DATA));
      clwk_data.anmseq  = POKEICON_ANM_HPMAX;
      clwk_data.pos_x   = sc_pokelist_hit_tbl[i].rect.left + 24;
      clwk_data.pos_y   = sc_pokelist_hit_tbl[i].rect.top + 16;

      p_wk->res[POKELIST_RESID_NCG_TOP+i] = GFL_CLGRP_CGR_Register( p_handle , 
          cgr, FALSE, CLSYS_DRAW_SUB , heapID  );

      p_wk->p_clwk_poke[i]  = GFL_CLACT_WK_Create( p_wk->param.p_clunit,
          p_wk->res[POKELIST_RESID_NCG_TOP+i],
          p_wk->res[POKELIST_RESID_PLT],
          p_wk->res[POKELIST_RESID_CEL],
          &clwk_data, CLSYS_DRAW_SUB, 
          heapID
          );
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk_poke[i], plt, CLWK_PLTTOFFS_MODE_PLTT_TOP );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_poke[i], FALSE );
    }

    GFL_ARC_CloseDataHandle( p_handle );
  }

  //BMPWIN
  {
    int i;

    for( i = 0; i < PokeParty_GetPokeCount(p_wk->param.p_party); i++ )
    {
      u8  x = POKELIST_X(i)+1;
      u8  y = POKELIST_Y(i)+1;
      u8  w = POKELIST_W(i)-1;
      u8  h = POKELIST_H(i)-1;
      POKEMON_PARAM *p_pp  = PokeParty_GetMemberPointer( p_wk->param.p_party, i );

      p_wk->p_bmpwin[i] = GFL_BMPWIN_Create( LTVT_FRAME_SUB_MSG, 
          x, y, w, h, LTVT_PLT_BG_SUB_FONT, GFL_BG_CHRAREA_GET_B );
      GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_bmpwin[i] );

      //ステータス書き込み
      LOCAL_TVT_MESSAGE_PrintPokeStatus( p_wk->param.p_message, p_wk->p_bmpwin[i], p_pp );
    }
    GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_MSG );
  }

  p_wk->p_set_visible_tcb  = GFUser_VIntr_CreateTCB(Local_Tvt_Pokelist_SetVisibleVBlank, p_wk, 0);
  p_wk->is_visible_req = TRUE;
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンリスト破棄
 *
 *	@param	LOCAL_TVT_POKELIST *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_POKELIST_Exit( LOCAL_TVT_POKELIST *p_wk )
{
  GFL_TCB_DeleteTask( p_wk->p_set_visible_tcb );
  GXS_SetVisibleWnd( GX_WNDMASK_NONE );
  G2S_BlendNone();

  //BMPWIN
  {
    int i;
    for( i = 0; i < 6; i++ )
    {
      if( p_wk->p_bmpwin[i] )
      {
        GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
      }
    }
  }

  //ポケモンアイコン消去
  {
    int i;
    for( i = 0; i < 6; i++ )
    {
      if( p_wk->p_clwk_poke[i] )
      {
        GFL_CLACT_WK_Remove( p_wk->p_clwk_poke[i] );
        GFL_CLGRP_CGR_Release( p_wk->res[POKELIST_RESID_NCG_TOP+i] );
      }
    }
    GFL_CLGRP_PLTT_Release( p_wk->res[POKELIST_RESID_PLT] );
    GFL_CLGRP_CELLANIM_Release( p_wk->res[POKELIST_RESID_CEL] );
  }


  //UI
  LIST_UI_Exit( p_wk->p_ui );

  //タッチバー終了
  LOCAL_TVT_TOUCHBAR_Exit( p_wk->p_touchbar );

  GFL_BG_FreeCharacterArea(LTVT_FRAME_SUB_BAR,
                           GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->plate_bg),
                           GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->plate_bg));

  //BGクリア
  GFL_BG_ClearFrame( LTVT_FRAME_SUB_MISC );
  GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_MISC );
  GFL_BG_ClearFrame( LTVT_FRAME_SUB_BAR );
  GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_BAR );
  GFL_BG_ClearFrame( LTVT_FRAME_SUB_MSG );
  GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_MSG );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンリストメイン処理
 *
 *	@param	LOCAL_TVT_POKELIST *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_POKELIST_Main( LOCAL_TVT_POKELIST *p_wk )
{
  enum
  {
    SEQ_MAIN,
    SEQ_DECIDE_ANM,
    SEQ_CANCEL_ANM,
    SEQ_END,
  };

  if( p_wk->is_active == FALSE )
  {
    return;
  }

  switch( p_wk->seq )
  {
  case SEQ_MAIN:
    LIST_UI_Main( p_wk->p_ui );
    LOCAL_TVT_TOUCHBAR_Main( p_wk->p_touchbar );

    if( LIST_UI_IsDecide( p_wk->p_ui ) )
    {
      p_wk->anm_cnt     = 0;
      p_wk->anm_frame   = 0;
      p_wk->seq = SEQ_DECIDE_ANM;

      //パレットの色を戻す
      GFL_BG_LoadPalette( LTVT_FRAME_SUB_BAR, p_wk->plt_decide, sizeof(u16)*0x10, LTVT_PLT_BG_SUB_LIST_ON * 32 );

      //いったん全部選択解除
      Local_Tvt_Pokelist_ClearList( p_wk, FALSE );
      break;
    }
    else if( LOCAL_TVT_TOUCHBAR_IsTrg( p_wk->p_touchbar ) )
    {
      p_wk->seq = SEQ_CANCEL_ANM;
    }

    if ( p_wk->anm_frame + 0x400 >= 0x10000 )
    {
      p_wk->anm_frame = p_wk->anm_frame + 0x400 - 0x10000;
    }
    else
    {
      p_wk->anm_frame += 0x400;
    }
    {
      const POKEMON_PARAM *cp_pp  = PokeParty_GetMemberPointer( p_wk->param.p_party, p_wk->cursor_pos );
      BOOL is_egg = PP_Get( cp_pp, ID_PARA_tamago_flag, NULL );
      u16 * p_plt = is_egg ? p_wk->plt_nopush: p_wk->plt_normal;
      LOCAL_TVT_UTIL_MainPltAnmLine( NNS_GFD_DST_2D_BG_PLTT_SUB, p_wk->plt_buff, p_wk->anm_frame, LTVT_PLT_BG_SUB_LIST_ON, p_plt, p_wk->plt_decide );
    }
    break;

  case SEQ_DECIDE_ANM:
    {
      u8  x = POKELIST_X(p_wk->cursor_pos);
      u8  y = POKELIST_Y(p_wk->cursor_pos);
      u8  w = POKELIST_W(p_wk->cursor_pos);
      u8  h = POKELIST_H(p_wk->cursor_pos);
      u8  plt;

      if( (p_wk->anm_cnt/4)%2 )
      {
        plt = LTVT_PLT_BG_SUB_LIST_ON;
      }
      else
      {
        plt = LTVT_PLT_BG_SUB_LIST;
      }
      GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_BAR,
          x, y, w, h, plt );
      GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_BAR );

      if( p_wk->anm_cnt++ > 16 )
      {
        p_wk->result  = LOCAL_TVT_POKELIST_RESULT_DECIDE;
        p_wk->seq = SEQ_END;
      }
    }
    break;

  case SEQ_CANCEL_ANM:
    if( LOCAL_TVT_TOUCHBAR_IsEnd( p_wk->p_touchbar ) )
    {
      p_wk->result  = LOCAL_TVT_POKELIST_RESULT_CANCEL;
      p_wk->seq = SEQ_END;
    }
    break;

  case SEQ_END:
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンリスト  アクティブ設定
 *
 *	@param	LOCAL_TVT_POKELIST *p_wk  ワーク
 *	@param	is_active   TRUEでアクティブ  FALSEでノンアクティブ
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_POKELIST_SetActive( LOCAL_TVT_POKELIST *p_wk, BOOL is_active )
{
  p_wk->is_active = is_active;
  if( p_wk->is_active )
  {
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 ); 
    G2S_SetWnd0Position( 0, 192-24, 255, 192 );
    G2S_SetWnd1Position( 1, 192-24, 256, 192 );
    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, GX_BLEND_PLANEMASK_BG2, 13, 3 );
  }
  else
  {
    G2S_SetWnd0Position( 256-16, 0, 256, 16 );
    G2S_SetWnd1Position( 256-16, 0, 256, 16 );
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG0 |GX_BLEND_PLANEMASK_OBJ , 8 ); 
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンリスト  再スタート
 *
 *	@param	LOCAL_TVT_POKELIST *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_POKELIST_ReStart( LOCAL_TVT_POKELIST *p_wk )
{
  p_wk->seq = 0;
  p_wk->result  = LOCAL_TVT_POKELIST_RESULT_NONE;

  LIST_UI_ReStart( p_wk->p_ui );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンリスト  終了検知
 *
 *	@param	const LOCAL_TVT_POKELIST *cp_wk   ワーク
 *
 *	@return LOCAL_TVT_POKELIST_RESULT_NONE,   まだ
 *          LOCAL_TVT_POKELIST_RESULT_DECIDE, 決定
 *          LOCAL_TVT_POKELIST_RESULT_CANCEL, キャンセル
 *
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_POKELIST_RESULT LOCAL_TVT_POKELIST_GetResult( const LOCAL_TVT_POKELIST *cp_wk )
{
  return cp_wk->result;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンリスト  選んだものを取得
 *
 *	@param	const LOCAL_TVT_POKELIST *cp_wk   ワーク  
 *
 *	@return 選んだポケモンパラメータ
 */
//-----------------------------------------------------------------------------
const POKEMON_PARAM * LOCAL_TVT_POKELIST_GetSelect( const LOCAL_TVT_POKELIST *cp_wk )
{
  return PokeParty_GetMemberPointer( cp_wk->param.p_party, cp_wk->cursor_pos );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リストをクリア
 *
 *	@param	LOCAL_TVT_POKELIST *p_wk ワーク
 *	@param  BOOL  is_cursor TRUEでカーソルの箇所は決定色で塗る
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_Pokelist_ClearList( LOCAL_TVT_POKELIST *p_wk, BOOL is_cursor )
{
  int i;
  for( i = 0; i < PokeParty_GetPokeCountMax( p_wk->param.p_party ); i++ )
  {

    u8  x = POKELIST_X(i);
    u8  y = POKELIST_Y(i);
    u8  w = POKELIST_W(i);
    u8  h = POKELIST_H(i);

    const POKEMON_PARAM *cp_pp  = NULL;
    
    if( i < PokeParty_GetPokeCount( p_wk->param.p_party ) )
    {
      cp_pp = PokeParty_GetMemberPointer( p_wk->param.p_party, i );
    }

    if( cp_pp == NULL || PP_Get( cp_pp, ID_PARA_poke_exist, NULL ) == FALSE )
    {
      GFL_BG_FillScreen( LTVT_FRAME_SUB_BAR, 
          0, x, y, w, h, GFL_BG_SCRWRT_PALNL );
    }
    else if( PP_Get( cp_pp, ID_PARA_tamago_flag, NULL ) )
    {
      GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_BAR , x , y, w, h, LTVT_PLT_BG_SUB_LIST_OFF );
    }
    else
    {
      GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_BAR , x , y, w, h, LTVT_PLT_BG_SUB_LIST );
    }

    if( is_cursor )
    {
      if( i == p_wk->cursor_pos )
      {
        GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_BAR , x , y, w, h, LTVT_PLT_BG_SUB_LIST_ON);
      }
    }
  }

  GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_BAR );
}
//----------------------------------------------------------------------------
/**
 *	@brief  たて横のインデックスを取得
 *
 *	@param	u32 cursor_pos  カーソル位置
 *	@param	*p_row          たてインデックス
 *	@param	*p_col          よこインデックス
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_Pokelist_GetRowColIndex( u32 cursor_pos, s16 *p_row, s16 *p_col )
{
  for( *p_row = 0; *p_row < POKELIST_ROW_MAX; (*p_row)++ )
  {
    for( *p_col = 0; *p_col < POKELIST_COL_MAX; (*p_col)++ )
    {
      if( cursor_pos-- == 0 )
      {
        return;
      }
    }
  }
  GF_ASSERT(0);
}
//----------------------------------------------------------------------------
/**
 *	@brief  たて横のインデックス最大値を取得
 *
 *	@param	u32 cursor_pos  カーソル位置
 *	@param	u32 cursor_max  カーソル最大
 *	@param	*p_row          たてインデックス最大
 *	@param	*p_col          よこインデックス最大
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_Pokelist_GetRowColIndexMax( u32 cursor_pos, u32 cursor_max, s16 *p_row, s16 *p_col )
{
  s16 now_row;
  s16 now_col;

  //現在位置を取得
  Local_Tvt_Pokelist_GetRowColIndex( cursor_pos, &now_row, &now_col );

  //上中下
  for( *p_row = 0; *p_row < POKELIST_ROW_MAX; (*p_row)++ )
  {
    if( sc_pokelist_pos[*p_row][now_col] >= (cursor_max) )
    {
      break;
    }
  }

  //左右
  for( *p_col = 0; *p_col < POKELIST_COL_MAX; (*p_col)++ )
  {
    if( sc_pokelist_pos[now_row][*p_col] >= (cursor_max) )
    {
      break;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  タッチ操作
 *
 *	@param	void *p_wk_adrs   ワークアドレス
 *
 *	@return 更新
 */
//-----------------------------------------------------------------------------
static LIST_UI_RESULT Local_Tvt_Pokelist_TouchUpdate( void *p_wk_adrs )
{
  LOCAL_TVT_POKELIST  *p_wk = p_wk_adrs;
  int ret = GFL_UI_TP_HitTrg( sc_pokelist_hit_tbl );

  if( ret != GFL_UI_TP_HIT_NONE && ret < PokeParty_GetPokeCount( p_wk->param.p_party ) )
  {
    const POKEMON_PARAM *cp_pp  = PokeParty_GetMemberPointer( p_wk->param.p_party, ret );

    //一旦全部選択解除
    Local_Tvt_Pokelist_ClearList( p_wk, FALSE );
    if( PP_Get( cp_pp, ID_PARA_tamago_flag, NULL ) )
    {
      //卵は押せない
      PMSND_PlaySE( LOCAL_TVT_SE_ERROR );
      p_wk->cursor_pos  = ret;
      //アニメを最初からにする
      return LIST_UI_RESULT_NONE;
    }
    else
    {
      PMSND_PlaySE( LOCAL_TVT_SE_DECIDE );
      p_wk->cursor_pos  = ret;
      return LIST_UI_RESULT_DECIDE;
    }
  }
  return LIST_UI_RESULT_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  キー操作
 *
 *	@param	void *p_wk_adrs   ワークアドレス
 *
 *	@return 更新
 */
//-----------------------------------------------------------------------------
static LIST_UI_RESULT Local_Tvt_Pokelist_KeyUpdate( void *p_wk_adrs )
{
  LOCAL_TVT_POKELIST  *p_wk = p_wk_adrs;
  BOOL is_key_move  = FALSE;
  BOOL is_decide    = FALSE;

  s16 now_row;
  s16 now_col;
  s16 max_row;
  s16 max_col;

  //現在の上下左右インデックス取得
  Local_Tvt_Pokelist_GetRowColIndex( p_wk->cursor_pos, &now_row, &now_col );
  Local_Tvt_Pokelist_GetRowColIndexMax( p_wk->cursor_pos, PokeParty_GetPokeCount(p_wk->param.p_party), &max_row, &max_col );

  NAGI_Printf( "row %d/%d col %d/%d\n", now_row, max_row, now_col, max_col );

  //上下左右移動
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
  {
    now_row--;
    if( now_row < 0 )
    {
      now_row = max_row-1;
    }
    PMSND_PlaySE( LOCAL_TVT_SE_SELECT );
    is_key_move = TRUE;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
  {
    now_row++;
    now_row  %= max_row;

    PMSND_PlaySE( LOCAL_TVT_SE_SELECT );
    is_key_move = TRUE;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    now_col--;
    if( now_col < 0 )
    {
      now_col = max_col-1;
    }

    PMSND_PlaySE( LOCAL_TVT_SE_SELECT );
    is_key_move = TRUE;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    now_col++;
    now_col  %= max_col;

    PMSND_PlaySE( LOCAL_TVT_SE_SELECT );
    is_key_move = TRUE;
  }

  //決定
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    is_decide =TRUE;
  }

  //動作
  if( is_key_move )
  {
    p_wk->cursor_pos  = sc_pokelist_pos[now_row][now_col];

    //選択を変更
    Local_Tvt_Pokelist_ClearList( p_wk, TRUE );

    //アニメを最初からにする
    p_wk->anm_frame = 0;

    GFL_BG_LoadScreenV_Req( LTVT_FRAME_SUB_BAR );
  }
  
  //終了
  if( is_decide )
  {
    const POKEMON_PARAM *cp_pp  = PokeParty_GetMemberPointer( p_wk->param.p_party, p_wk->cursor_pos );
    if( PP_Get( cp_pp, ID_PARA_tamago_flag, NULL ) )
    {
      PMSND_PlaySE( LOCAL_TVT_SE_ERROR );
      //アニメを最初からにする
      p_wk->anm_frame = 0;
      return LIST_UI_RESULT_NONE;
    }
    else
    {
      PMSND_PlaySE( LOCAL_TVT_SE_DECIDE );
      return LIST_UI_RESULT_DECIDE;
    }
  }
  else if( is_key_move )
  {
    return LIST_UI_RESULT_MOVE;
  }
  else
  {
    return LIST_UI_RESULT_NONE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  タッチ、キー切り替え時の処理
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	touch_or_key    切り替え後のGFL_APP_END_TOUCH　or GFL_APP_END_KEY
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_Pokelist_Change( void *p_wk_adrs, int touch_or_key )
{
  LOCAL_TVT_POKELIST  *p_wk = p_wk_adrs;

  if( touch_or_key == GFL_APP_END_TOUCH )
  {
    //タッチモードになった
    Local_Tvt_Pokelist_ClearList( p_wk, FALSE );
  }
  else
  {
    //キーモードになった
    Local_Tvt_Pokelist_ClearList( p_wk, TRUE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  VBlankにて表示を設定する関数
 *
 *	@param	GFL_TCB *p_tcb  TCB
 *	@param	*p_wk_adrs      ワーク
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_Pokelist_SetVisibleVBlank( GFL_TCB *p_tcb, void *p_wk_adrs )
{
  LOCAL_TVT_POKELIST  *p_wk = p_wk_adrs;
  if( p_wk->is_visible_req )
  {
    int i;
    for( i = 0; i < 6; i++ )
    {
      if( p_wk->p_clwk_poke[i] )
      {
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_poke[i], TRUE );
      }
    }

    p_wk->is_visible_req  = FALSE;

    LOCAL_TVT_TOUCHBAR_SetVisible( p_wk->p_touchbar, TRUE );
    GFL_BG_SetVisible( LTVT_FRAME_SUB_BAR, TRUE );
    GFL_BG_SetVisible( LTVT_FRAME_SUB_MISC, TRUE );
    GFL_BG_SetVisible( LTVT_FRAME_SUB_MSG, TRUE );

    //ポケモンリストのプレートにアルファをかける
    //ただしバーと共通のBG面なのでバーにはかけないようにするためウィンドウマスク
    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, GX_BLEND_PLANEMASK_BG2, 13, 3 );
    GXS_SetVisibleWnd( GX_WNDMASK_W0|GX_WNDMASK_W1 );
    G2S_SetWnd0Position( 0, 192-24, 255, 192 );
    G2S_SetWnd1Position( 1, 192-24, 256, 192 );
    G2S_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG1|
        GX_WND_PLANEMASK_BG2|
        GX_WND_PLANEMASK_OBJ, FALSE );
    G2S_SetWnd1InsidePlane( GX_WND_PLANEMASK_BG1|
        GX_WND_PLANEMASK_BG2|
        GX_WND_PLANEMASK_OBJ, FALSE );
    G2S_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0|
        GX_WND_PLANEMASK_BG1|
        GX_WND_PLANEMASK_BG2|
        GX_WND_PLANEMASK_BG3|
        GX_WND_PLANEMASK_OBJ,
        TRUE );
  }
}

//=============================================================================
/**
 *					ETC
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  パレットフェード１行すべて塗り替える版
 *
 *  @param  NNS_GFD_DST_TYPE type 転送先
 *  @param  *p_buff               バッファ0x10個分
 *  @param  cnt                   カウント
 *  @param  plt_num               パレット行
 *  @param  GXRgb start[]         開始色0x10個分
 *  @param  end[]                 終了色0x10個分
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_UTIL_MainPltAnmLine( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 cnt, u8 plt_num, GXRgb start[], GXRgb end[] )
{ 
  int i;
 //0～１に変換
  for( i = 0; i < 0x10; i++ )
  { 
    const fx16 cos = (FX_CosIdx(cnt)+FX16_ONE)/2;
    const u8 start_r  = (start[i] & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    const u8 start_g  = (start[i] & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    const u8 start_b  = (start[i] & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;
    const u8 end_r  = (end[i] & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    const u8 end_g  = (end[i] & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    const u8 end_b  = (end[i] & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;

    const u8 r = start_r + (((end_r-start_r)*cos)>>FX16_SHIFT);
    const u8 g = start_g + (((end_g-start_g)*cos)>>FX16_SHIFT);
    const u8 b = start_b + (((end_b-start_b)*cos)>>FX16_SHIFT);

    p_buff[i] = GX_RGB(r, g, b);
  }

  { 
    BOOL ret;
    ret = NNS_GfdRegisterNewVramTransferTask( type,
        plt_num * 32,
        p_buff, 32 );
    GF_ASSERT(ret);
  }

}
//=============================================================================
/**
 *    バーアイコン
 *
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  バーアイコン初期化
 *
 *	@param	BARICON *p_wk ワーク
 *	@param	*p_clunit CLUNIT
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void BARICON_Init( BARICON *p_wk, GFL_CLUNIT *p_clunit, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(BARICON));
  //リソース読込み
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId() , heapID );

    //OBJ
    p_wk->res[RESID_OBJ_PLT] = GFL_CLGRP_PLTT_RegisterEx( p_handle , 
        APP_COMMON_GetBarIconPltArcIdx() , CLSYS_DRAW_SUB , 
        LTVT_PLT_OBJ_SUB_BARICON*32 , 0 , APP_COMMON_BARICON_PLT_NUM , heapID  );
    p_wk->res[RESID_OBJ_NCG] = GFL_CLGRP_CGR_Register( p_handle , 
        APP_COMMON_GetBarIconCharArcIdx() , FALSE , CLSYS_DRAW_SUB , heapID  );
    p_wk->res[RESID_OBJ_ANM] = GFL_CLGRP_CELLANIM_Register( p_handle , 
        APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_128K) , 
        APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_128K), heapID  );
  /*
    //BG
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, APP_COMMON_GetBarPltArcIdx() , 
        PALTYPE_SUB_BG , LTVT_PLT_BG_SUB_BAR*32 , 32 , heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, APP_COMMON_GetBarCharArcIdx() ,
        LTVT_FRAME_SUB_BAR , 0 , 0, FALSE , heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, APP_COMMON_GetBarScrnArcIdx() , 
        LTVT_FRAME_SUB_BAR ,  0 , 0, FALSE , heapID );
    GFL_BG_ChangeScreenPalette( LTVT_FRAME_SUB_BAR , 0 , 21 , 32 , 3 , LTVT_PLT_BG_SUB_BAR );
    GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_BAR );
*/

    GFL_ARC_CloseDataHandle( p_handle );
  }

  //CLWK作成
  {
    GFL_CLWK_DATA clwk_data;
    GFL_STD_MemClear( &clwk_data , sizeof(GFL_CLWK_DATA));
    clwk_data.pos_x = 224;
    clwk_data.pos_y = 192-24;

    p_wk->p_clwk_return = GFL_CLACT_WK_Create( p_clunit,
              p_wk->res[RESID_OBJ_NCG],
              p_wk->res[RESID_OBJ_PLT],
              p_wk->res[RESID_OBJ_ANM],
              &clwk_data ,CLSYS_DRAW_SUB , heapID );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_return , TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk_return , TRUE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーアイコン破棄
 *
 *	@param	BARICON *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BARICON_Exit( BARICON *p_wk )
{
  //CLWK破棄
  GFL_CLACT_WK_Remove( p_wk->p_clwk_return );

  //リソース破棄
  GFL_CLGRP_PLTT_Release( p_wk->res[RESID_OBJ_PLT] );
  GFL_CLGRP_CGR_Release( p_wk->res[RESID_OBJ_NCG] );
  GFL_CLGRP_CELLANIM_Release( p_wk->res[RESID_OBJ_ANM] );

  GFL_STD_MemClear( p_wk, sizeof(BARICON));
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーアイコン取得
 *
 *	@param	BARICON *p_wk   ワーク
 *
 *	@return バーアイコン
 */
//-----------------------------------------------------------------------------
static GFL_CLWK * BARICON_GetClwk( const BARICON *cp_wk )
{
  return cp_wk->p_clwk_return;
}

//=============================================================================
/**
 *	リストをカーソルとタッチで操作するシステム
 */
//=============================================================================
struct _LIST_UI
{
  u16   select_max;
  u16   dummy;
  int   touch_or_key;
  BOOL  is_decide;

  LIST_UI_PARAM param;
};

static LIST_UI_RESULT List_Ui_UpdateTouch( LIST_UI *p_wk );
static LIST_UI_RESULT List_Ui_UpdateKey( LIST_UI *p_wk );
static void List_Ui_Change( LIST_UI *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  UI操作  初期化
 *
 *	@param	LIST_UI_PARAM *cp_param 引数
 *	@param	heapID                  ヒープID
 *
 *	@retval   ワーク
 */
//-----------------------------------------------------------------------------
LIST_UI * LIST_UI_Init( const LIST_UI_PARAM *cp_param, HEAPID heapID )
{
  LIST_UI *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LIST_UI ) );
  p_wk->param         = *cp_param;
  p_wk->touch_or_key  = GFL_UI_CheckTouchOrKey();
  //切り替えを最初の初期化としても使う
  List_Ui_Change( p_wk );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  UI操作  破棄
 *
 *	@param	LIST_UI *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIST_UI_Exit( LIST_UI *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  UI操作  メイン処理
 *
 *	@param	LIST_UI *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIST_UI_Main( LIST_UI *p_wk )
{
  if( p_wk->touch_or_key == GFL_APP_END_TOUCH )
  {
    if( GFL_UI_KEY_GetTrg() != 0 )
    {
      PMSND_PlaySE( LOCAL_TVT_SE_SELECT );
      //タッチモードでキー入力したらキーモードへいく
      p_wk->touch_or_key = GFL_APP_END_KEY;
      List_Ui_Change( p_wk );
    }
    else 
    {
      if( LIST_UI_RESULT_DECIDE == List_Ui_UpdateTouch( p_wk ) )
      {
        //タッチモードでタッチしたら終了
        GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
        p_wk->is_decide = TRUE;
      }
    }
  }
  else
  {
    switch( List_Ui_UpdateTouch( p_wk ) )
    {
    case LIST_UI_RESULT_DECIDE:
      //キーモードでタッチ決定したら終了
      GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      p_wk->is_decide = TRUE;
      break;
    case LIST_UI_RESULT_MOVE:
      //キーモードでタッチ入力したらタッチモードへいく
      p_wk->touch_or_key = GFL_APP_END_TOUCH;
      List_Ui_Change( p_wk );
      break;
    case LIST_UI_RESULT_NONE:
      //何もないときにキーを動かす
      if( LIST_UI_RESULT_DECIDE == List_Ui_UpdateKey( p_wk ) )
      {
        //キーモードでキー入力したら終了
        GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
        p_wk->is_decide = TRUE;
      }
      break;

    default:
      GF_ASSERT( 0 );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  UI操作  決定取得
 *
 *	@param	const LIST_UI *cp_wk  ワーク
 *
 *	@return TRUE決定  FALSE決定していない
 */
//-----------------------------------------------------------------------------
BOOL LIST_UI_IsDecide( const LIST_UI *cp_wk )
{
  return cp_wk->is_decide;
}

//----------------------------------------------------------------------------
/**
 *	@brief  UI操作  再初期化
 *
 *	@param	LIST_UI *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIST_UI_ReStart( LIST_UI *p_wk )
{
  p_wk->is_decide = FALSE;
  p_wk->touch_or_key  = GFL_UI_CheckTouchOrKey();
  //切り替えを最初の初期化としても使う
  List_Ui_Change( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  タッチ入力仮想関数呼び出し
 *
 *	@param	LIST_UI *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static LIST_UI_RESULT List_Ui_UpdateTouch( LIST_UI *p_wk )
{
  if( p_wk->param.UpdateTouch != NULL )
  {
    return p_wk->param.UpdateTouch( p_wk->param.p_wk_adrs );
  }
  return LIST_UI_RESULT_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  キー入力仮想関数呼び出し
 *
 *	@param	LIST_UI *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static LIST_UI_RESULT List_Ui_UpdateKey( LIST_UI *p_wk )
{
  if( p_wk->param.UpdateKey != NULL )
  {
    return p_wk->param.UpdateKey( p_wk->param.p_wk_adrs );
  }
  return LIST_UI_RESULT_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  切り替え仮想関数呼び出し
 *
 *	@param	LIST_UI *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void List_Ui_Change( LIST_UI *p_wk )
{
  if( p_wk->param.Change != NULL )
  {
    p_wk->param.Change( p_wk->param.p_wk_adrs, p_wk->touch_or_key );
  }
}
