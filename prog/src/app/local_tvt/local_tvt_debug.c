//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_debug.c
 *	@brief  ローカルライブキャスターデバッグ機能
 *	@author	Toru=Nagihashi
 *	@data		2011.01.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG

//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "debug/debugwin_sys.h"
#include "savedata/person_state.h"

//自分のモジュール
#include "local_tvt_local_def.h"
#include "local_tvt_graphic.h"
#include "local_tvt_chara.h"
#include "local_tvt_util.h"

//外部公開
#include "local_tvt_debug.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define DEBUGWIN_GROUP_LTVT (10)
#define DEBUGWIN_GROUP_GRAPHIC (11)
#define DEBUGWIN_GROUP_CALLMODE (12)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
static LOCAL_TVT_MEMBER     s_member;
static LOCAL_TVT_TIME_ZONE  s_timezone;
static LOCAL_TVT_CHARA_PARAM  s_chara[LTM_4_MEMBER];
static int  s_chara_idx = 0;
static BOOL s_is_chara_load_req  = FALSE;
static LOCAL_TVT_DEBUG_PARAM s_param  = {0};
static u8 s_debug_flag[LOCAL_TVT_DEBUG_FLAG_MAX]  = {0};
static u8 s_debug_state[LOCAL_TVT_PERSON_MAX]     = {0};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static BOOL DebugWin_Util_ChangeValue( int *p_param, int min, int max );

//グラフィック系
static void Ltvt_Debug_Update_Member( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_Member( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_TimeZone( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_TimeZone( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_CharaIndex( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_CharaIndex( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_Back( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_Back( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_Weather( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_Weather( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_Character( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_Character( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );

//呼び出しモード系
static void Ltvt_Debug_Update_DoctorState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_DoctorState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_RivalState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_RivalState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_ChelenState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_ChelenState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_BellState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_BellState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_MotherState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_MotherState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_SecretState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_SecretState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Update_CheckZukanFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Ltvt_Debug_Draw_CheckZukanFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );

//=============================================================================
/**
 *    データ
 */
//=============================================================================
//-------------------------------------
///	共通ラベル
//=====================================
static const char * scp_yesno_label[] =
{
  "NO",
  "YES",
};

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグシステム初期化
 *
 *  @param  引数
 *	@param	HEAPID heapID ヒープID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_DEBUG_Init( const LOCAL_TVT_DEBUG_PARAM *cp_param, HEAPID heapID )
{
  s_param = *cp_param;

  DEBUGWIN_InitProc( GFL_BG_FRAME0_M, cp_param->p_font );
  //DEBUGWIN_ChangeLetterColor( 0,31,0 );
  
  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUP_LTVT, "ライブキャスター", heapID );

  //グラフィック系デバッグ
  {
    LOCAL_TVT_CHARA_SYS_GetLoadParam( cp_param->p_chara_sys, s_chara, &s_member );
    s_member  = s_member == 0 ?LTM_2_MEMBER:LTM_4_MEMBER;
    s_timezone  = LOCAL_TVT_CHARA_SYS_GetTimeZone( cp_param->p_chara_sys );

    DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_GRAPHIC, "グラフィック", DEBUGWIN_GROUP_LTVT, heapID );
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_Member, 
                                Ltvt_Debug_Draw_Member, 
                                cp_param->p_chara_sys, 
                                DEBUGWIN_GROUP_GRAPHIC,
                                heapID ); 
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_TimeZone, 
                                Ltvt_Debug_Draw_TimeZone, 
                                cp_param->p_chara_sys, 
                                DEBUGWIN_GROUP_GRAPHIC,
                                heapID );
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_CharaIndex, 
                                Ltvt_Debug_Draw_CharaIndex, 
                                cp_param->p_chara_sys, 
                                DEBUGWIN_GROUP_GRAPHIC,
                                heapID );

    //各キャラ
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_Character, 
                                Ltvt_Debug_Draw_Character, 
                                cp_param->p_chara_sys, 
                                DEBUGWIN_GROUP_GRAPHIC,
                                heapID );
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_Back, 
                                Ltvt_Debug_Draw_Back, 
                                cp_param->p_chara_sys, 
                                DEBUGWIN_GROUP_GRAPHIC,
                                heapID );

    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_Weather, 
                                Ltvt_Debug_Draw_Weather, 
                                cp_param->p_chara_sys, 
                                DEBUGWIN_GROUP_GRAPHIC,
                                heapID );
  }

  //呼び出しモード
  {
    int i;
    const PERSON_STATE  *cp_sv  = PERSON_STATE_GetWorkConst( GAMEDATA_GetSaveControlWorkConst(cp_param->p_gamedata) );
    for( i = 0; i < LOCAL_TVT_PERSON_MAX; i++ )
    {
      s_debug_state[i]  = PERSON_STATE_GetState( cp_sv, i );
    }
    DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_CALLMODE, "よびだしモード", DEBUGWIN_GROUP_LTVT, heapID );
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_DoctorState, 
                                Ltvt_Debug_Draw_DoctorState, 
                                NULL, 
                                DEBUGWIN_GROUP_CALLMODE,
                                heapID ); 
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_RivalState, 
                                Ltvt_Debug_Draw_RivalState, 
                                NULL, 
                                DEBUGWIN_GROUP_CALLMODE,
                                heapID ); 
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_ChelenState, 
                                Ltvt_Debug_Draw_ChelenState, 
                                NULL, 
                                DEBUGWIN_GROUP_CALLMODE,
                                heapID ); 
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_BellState, 
                                Ltvt_Debug_Draw_BellState, 
                                NULL, 
                                DEBUGWIN_GROUP_CALLMODE,
                                heapID ); 
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_MotherState, 
                                Ltvt_Debug_Draw_MotherState, 
                                NULL, 
                                DEBUGWIN_GROUP_CALLMODE,
                                heapID ); 
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_SecretState, 
                                Ltvt_Debug_Draw_SecretState, 
                                NULL, 
                                DEBUGWIN_GROUP_CALLMODE,
                                heapID ); 
    DEBUGWIN_AddItemToGroupEx( Ltvt_Debug_Update_CheckZukanFlag, 
                                Ltvt_Debug_Draw_CheckZukanFlag, 
                                NULL, 
                                DEBUGWIN_GROUP_CALLMODE,
                                heapID ); 
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグシステム破棄
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_DEBUG_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_CALLMODE );
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_GRAPHIC );
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_LTVT );
  DEBUGWIN_ExitProc();
}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメイン
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_DEBUG_Main( void )
{
  if( s_is_chara_load_req )
  {
    LOCAL_TVT_CHARA_SYS_LoadChara( s_param.p_chara_sys, s_chara, s_member );
    s_is_chara_load_req = FALSE;

    LOCAL_TVT_CHARA_SYS_SetVisible( s_param.p_chara_sys, TRUE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグフラグ取得
 *
 *	@param	LOCAL_TVT_DEBUG_FLAG flag   フラグ
 *
 *	@return デバッグフラグ
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_DEBUG_IsFlagOn( LOCAL_TVT_DEBUG_FLAG flag )
{
  return s_debug_flag[ flag ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー  汎用値更新関数
 *
 *	@param	*p_param  更新する値
 *	@param	min 最小値
 *	@param	max 最大値
 *	@retval TRUE更新  FALSEしていない
 */
//-----------------------------------------------------------------------------
static BOOL DebugWin_Util_ChangeValue( int *p_param, int min, int max )
{ 
  int value = 0;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  { 
    value = -1;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    value = +1;
  }

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A )
  { 
    value *= 10;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  { 
    value *= 100;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y )
  { 
    value *= 1000;
  }
  
  (*p_param)  +=  value;
  (*p_param)  = MATH_CLAMP( (*p_param), min, max );


  if( value != 0 )
  { 
    return TRUE;
  }

  return FALSE;
}
//=============================================================================
/**
 *    グラフィック計
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  人数  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_Member( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  LOCAL_TVT_CHARA_SYS *p_chara_sys  = p_wk_adrs;
  int buff  = (s_member == LTM_2_MEMBER? 0: 1);
  if( DebugWin_Util_ChangeValue( &buff, 0, 1 ) )
  {
    s_member  = buff == 0 ?LTM_2_MEMBER:LTM_4_MEMBER;
    s_is_chara_load_req = TRUE;

    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人数  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_Member( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DEBUGWIN_ITEM_SetNameV( p_item , "ひょうじにんずう[%d]", s_member );
}
//----------------------------------------------------------------------------
/**
 *	@brief  時間帯  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_TimeZone( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  LOCAL_TVT_CHARA_SYS *p_chara_sys  = p_wk_adrs;
  int timezone  = s_timezone;
  if( DebugWin_Util_ChangeValue( &timezone, 0, LTTZ_MAX-1 ) )
  {
    s_timezone  = timezone;

    LOCAL_TVT_CHARA_SYS_SetTimeZone( p_chara_sys, s_timezone );
    s_is_chara_load_req = TRUE;

    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  時間帯  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_TimeZone( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char * scp_label[LTTZ_MAX] =
  {
    "ひる",
    "ゆう",
    "よる",
  };
  DEBUGWIN_ITEM_SetNameV( p_item , "じかん[%s]", scp_label[ s_timezone ] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  操作キャラIDX  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_CharaIndex( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  if( DebugWin_Util_ChangeValue( &s_chara_idx, 0, s_member ) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  操作キャラIDX  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_CharaIndex( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DEBUGWIN_ITEM_SetNameV( p_item , "そうさキャラ[%d]", s_chara_idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief  背景  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_Back( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  LOCAL_TVT_CHARA_SYS *p_chara_sys  = p_wk_adrs;
  int bg  = s_chara[ s_chara_idx ].bgType;
  if( DebugWin_Util_ChangeValue( &bg, 0, LOCAL_TVT_BACK_MAX-1 ) )
  {
    s_chara[ s_chara_idx ].bgType  = bg;
    s_is_chara_load_req = TRUE;

    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  背景  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_Back( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char * scp_label[] =
  {
    "へや",
    "どうくつ",
    "まち",
    "へいや",
    "やま",
    "うみ",
    "さばく",
  };

  int bg  = s_chara[ s_chara_idx ].bgType;
  DEBUGWIN_ITEM_SetNameV( p_item , "%dの はいけい[%s]", s_chara_idx, scp_label[ bg ] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  天気  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_Weather( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  LOCAL_TVT_CHARA_SYS *p_chara_sys  = p_wk_adrs;
  int weather  = s_chara[ s_chara_idx ].weatherType;
  if( DebugWin_Util_ChangeValue( &weather, 0, LOCAL_TVT_WEATHER_MAX ) )
  {
    s_chara[ s_chara_idx ].weatherType  = weather;
    s_is_chara_load_req = TRUE;

    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  天気　描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_Weather( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char * scp_label[] =
  {
    "あめ",
    "すな",
    "ゆき",
    "はれ",
  };

  int weather  = s_chara[ s_chara_idx ].weatherType;
  DEBUGWIN_ITEM_SetNameV( p_item , "%dの てんき[%s]", s_chara_idx, scp_label[ weather ] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  キャラ  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_Character( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  LOCAL_TVT_CHARA_SYS *p_chara_sys  = p_wk_adrs;
  int chara  = s_chara[ s_chara_idx ].charaType;
  if( DebugWin_Util_ChangeValue( &chara, 0, LTCT_PLAYER_F-1 ) )
  {
    s_chara[ s_chara_idx ].charaType  = chara;
    s_is_chara_load_req = TRUE;

    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  キャラ  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_Character( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char * scp_label[] =
  {
    "プレイヤー",
    "ライバル",
    "チェレン",
    "ベル",
    "はかせ",
    "ママ",
  };

  int chara  = s_chara[ s_chara_idx ].charaType;
  DEBUGWIN_ITEM_SetNameV( p_item , "%dの キャラ[%s]", s_chara_idx, scp_label[ chara ] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_DoctorState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_DOCTOR ];
  if( DebugWin_Util_ChangeValue( &state, 0, DOCTOR_STATE_MAX-1 ) )
  {
    s_debug_state[ LOCAL_TVT_PERSON_DOCTOR ]  = state;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_DoctorState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_DOCTOR ];
  DEBUGWIN_ITEM_SetNameV( p_item , "はかせ じょうたい[%d]", state );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_RivalState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_RIVAL ];
  if( DebugWin_Util_ChangeValue( &state, 0, RIVAL_STATE_MAX-1 ) )
  {
    s_debug_state[ LOCAL_TVT_PERSON_RIVAL ]  = state;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_RivalState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_RIVAL ];
  DEBUGWIN_ITEM_SetNameV( p_item , "ライバル じょうたい[%d]", state );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_ChelenState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_CHELEN ];
  if( DebugWin_Util_ChangeValue( &state, 0, CHELEN_STATE_MAX-1 ) )
  {
    s_debug_state[ LOCAL_TVT_PERSON_CHELEN ]  = state;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_ChelenState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_CHELEN ];
  DEBUGWIN_ITEM_SetNameV( p_item , "チェレン じょうたい[%d]", state );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_BellState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_BELL ];
  if( DebugWin_Util_ChangeValue( &state, 0, BELL_STATE_MAX-1 ) )
  {
    s_debug_state[ LOCAL_TVT_PERSON_BELL ]  = state;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_BellState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_BELL ];
  DEBUGWIN_ITEM_SetNameV( p_item , "ベル じょうたい[%d]", state );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_MotherState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_MOTHER ];
  if( DebugWin_Util_ChangeValue( &state, 0, MOTHER_STATE_MAX-1 ) )
  {
    s_debug_state[ LOCAL_TVT_PERSON_MOTHER ]  = state;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_MotherState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_MOTHER ];
  DEBUGWIN_ITEM_SetNameV( p_item , "ママ じょうたい[%d]", state );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_SecretState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_SECRET ];
  if( DebugWin_Util_ChangeValue( &state, 0, SECRET_STATE_MAX-1 ) )
  {
    s_debug_state[ LOCAL_TVT_PERSON_SECRET ]  = state;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_SecretState( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int state  = s_debug_state[ LOCAL_TVT_PERSON_SECRET ];
  DEBUGWIN_ITEM_SetNameV( p_item , "かくし じょうたい[%d]", state );
}
//----------------------------------------------------------------------------
/**
 *	@brief  図鑑チェックフラグ  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Update_CheckZukanFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int flag  = s_debug_flag[ LOCAL_TVT_DEBUG_FLAG_CHECK_ZUKAN_ON ];
  if( DebugWin_Util_ChangeValue( &flag, 0, 1 ) )
  {
    s_debug_flag[ LOCAL_TVT_DEBUG_FLAG_CHECK_ZUKAN_ON ]  = flag;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  図鑑チェックフラグ  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Ltvt_Debug_Draw_CheckZukanFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  int flag  = s_debug_flag[ LOCAL_TVT_DEBUG_FLAG_CHECK_ZUKAN_ON ];
  DEBUGWIN_ITEM_SetNameV( p_item , "ずかんチェック[%s]", scp_yesno_label[ flag ] );
}

#endif //PM_DEBUG
