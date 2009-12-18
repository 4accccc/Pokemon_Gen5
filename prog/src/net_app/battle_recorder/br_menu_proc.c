//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_menu_proc.c
 *	@brief	メニュープロセス
 *	@author	Toru=Nagihashi
 *	@data		2009.11.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID

//自分のモジュール
#include "br_btn.h"

//セーブデータ
#include "savedata/battle_rec.h"

//外部参照
#include "br_menu_proc.h"

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
///	メニューメインワーク
//=====================================
typedef struct 
{
	BR_BTN_SYS_WORK	*p_btn;

	HEAPID heapID;
} BR_MENU_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BRコアプロセス
//=====================================
static GFL_PROC_RESULT BR_MENU_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_MENU_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_MENU_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	メニュープロセスプロセス
//=====================================
const GFL_PROC_DATA BR_MENU_ProcData =
{	
	BR_MENU_PROC_Init,
	BR_MENU_PROC_Main,
	BR_MENU_PROC_Exit,
};

//=============================================================================
/**
 *					データ
 */
//=============================================================================

//=============================================================================
/**
 *					メニュープロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	メニュープロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_MENU_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_MENU_WORK				*p_wk;
	BR_MENU_PROC_PARAM	*p_param	= p_param_adrs;

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_MENU_WORK), BR_PROC_SYS_GetHeapID( p_param->p_procsys ) );
	GFL_STD_MemClear( p_wk, sizeof(BR_MENU_WORK) );	
	p_wk->heapID	= BR_PROC_SYS_GetHeapID( p_param->p_procsys );

	//グラフィック初期化


	//モジュール初期化
	{	
    int i;
    LOAD_RESULT result;
    SAVE_CONTROL_WORK *p_sv;
    GDS_PROFILE_PTR p_profile;
		BR_BTN_DATA_SETUP	setup;
    GFL_STD_MemClear( &setup, sizeof(BR_BTN_DATA_SETUP) );
    
    p_sv = GAMEDATA_GetSaveControlWork( p_param->p_gamedata );

    //設定構造体作成
    for( i = 0; i < BR_RECORD_NUM; i++ )
    {
      BattleRec_Load( p_sv, GFL_HEAP_LOWID( p_wk->heapID ), &result, i ); 
      if( result == LOAD_RESULT_OK )
      { 
        NAGI_Printf( "戦闘録画読み込み %d\n", i );
        p_profile = BattleRec_GDSProfilePtrGet();
        setup.is_valid[i] = TRUE;
        setup.p_name[i]   = GDS_Profile_CreateNameString( p_profile, GFL_HEAP_LOWID( p_wk->heapID ) );
        setup.sex[i]      = GDS_Profile_GetSex( p_profile );
      }
    }

		p_wk->p_btn	= BR_BTN_SYS_Init( p_param->menuID, p_param->p_unit, p_param->p_res, &setup, p_wk->heapID );

    //設定構造体破棄
    for( i = 0; i < BR_RECORD_NUM; i++ )
    { 
      if( setup.is_valid[i] )
      { 
        GFL_STR_DeleteBuffer( setup.p_name[i] );
      }
    }
	}

	NAGI_Printf( "MENU: Init!\n" );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニュープロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_MENU_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_MENU_WORK				*p_wk	= p_wk_adrs;
	BR_MENU_PROC_PARAM	*p_param	= p_param_adrs;

	//モジュール破棄
	BR_BTN_SYS_Exit( p_wk->p_btn );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニュープロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_MENU_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_MAIN,
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_END,
  };

	BR_MENU_WORK	*p_wk	= p_wk_adrs;
	BR_MENU_PROC_PARAM	*p_param	= p_param_adrs;

  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
    BR_FADE_StartFade( p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    *p_seq  = SEQ_FADEIN_WAIT;
    break;

  case SEQ_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_param->p_fade ) )
    { 
      *p_seq  = SEQ_MAIN;
    }
    break;
   
  case SEQ_MAIN:
    //プロセス処理
    BR_BTN_SYS_Main( p_wk->p_btn );
    if( BR_BTN_SYS_GetInput( p_wk->p_btn, &p_param->next_proc, &p_param->next_mode ) != BR_BTN_SYS_INPUT_NONE )
    { 
      *p_seq  = SEQ_FADEOUT_START;
    }
    break;

  case SEQ_FADEOUT_START:
    BR_FADE_StartFade( p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
    if( BR_FADE_IsEnd( p_param->p_fade ) )
    { 
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    //ボタンが押された処理
    switch( BR_BTN_SYS_GetInput( p_wk->p_btn, &p_param->next_proc, &p_param->next_mode ) )
    {	
      //次のプロセスへ行く
    case BR_BTN_SYS_INPUT_CHANGESEQ:
      BR_PROC_SYS_Push( p_param->p_procsys, p_param->next_proc );
      NAGI_Printf( "MENU: Next %d!\n", p_param->next_proc );
      return GFL_PROC_RES_FINISH;

    case BR_BTN_SYS_INPUT_EXIT:
      NAGI_Printf( "MENU: Exit!\n" );
      BR_PROC_SYS_Pop( p_param->p_procsys );
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

