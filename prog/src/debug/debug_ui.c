//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_ui.c
 *	@brief  UI　上書き　デバック機能
 *	@author	tomoya takahashi
 *	@date		2010.04.29
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#ifdef PM_DEBUG

#include <gflib.h>



#ifndef GFL_UI_DEBUGSYS_ENABLE

// UI DEBUGSYSが無効のばあい。
void DEBUG_UI_SetUp( DEBUG_UI_TYPE type ){}

DEBUG_UI_TYPE DEBUG_UI_GetType( void ){ return 0; }

#else 




#include "debug/debugwin_sys.h"
#include "debug/debug_ui.h"

// キーテーブル
#include "debug_ui_updown.h"
#include "debug_ui_leftright.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	上書き機能構造体
//=====================================
typedef struct {
  u16 type;
  u32 count;
  u32 data_max;
  const GFL_UI_DEBUG_OVERWRITE* cp_data;
} DEBUG_UI_WORK;

// 
static DEBUG_UI_WORK s_DEBUG_UI_WORK = {0};






//-------------------------------------
///	テーブルオーバーレイ管理
//=====================================
typedef struct {
  u16 type;
  u16 data_max;
  const GFL_UI_DEBUG_OVERWRITE* cp_data;
	FSOverlayID					overlay_id;			// オーバーレイID
} DEBUG_UI_KEY_TABLE;

// もし、プログラム内に配列でキー情報を持つ場合には、このテーブルに
// 登録することで、オーバーレイを含めて管理することが出来ます。
static const DEBUG_UI_KEY_TABLE sc_DEBUG_UI_KEY_TABLE[] = {
  // 自動　UP　DOWN
  {
    DEBUG_UI_AUTO_UPDOWN,
    DEBUG_UI_UPDOWN_DATA_MAX,
    c_DEBUG_UI_UPDOWN,
		FS_OVERLAY_ID(debug_ui_updown)
  },

  // 自動　LEFT　RIGHT
  {
    DEBUG_UI_AUTO_LEFTRIGHT,
    DEBUG_UI_LEFTRIGHT_DATA_MAX,
    c_DEBUG_UI_LEFTRIGHT,
		FS_OVERLAY_ID(debug_ui_leftright)
  },
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static BOOL DebugUI_OverWriteCallback( GFL_UI_DEBUG_OVERWRITE* p_data );
static const DEBUG_UI_KEY_TABLE* DebugUI_GetKeyTable( DEBUG_UI_TYPE type );

//----------------------------------------------------------------------------
/**
 *	@brief  セットアップ
 *
 *	@param	type  タイプ
 */
//-----------------------------------------------------------------------------
void DEBUG_UI_SetUp( DEBUG_UI_TYPE type )
{
  const DEBUG_UI_KEY_TABLE* cp_keytable;
  if( type == s_DEBUG_UI_WORK.type ){
    return ;
  }

  // デバッカーか？
  if( !(OS_GetConsoleType() & (OS_CONSOLE_ISDEBUGGER|OS_CONSOLE_TWLDEBUGGER)) ){
    return ;
  }

  // 過去のものを破棄
  cp_keytable = DebugUI_GetKeyTable( s_DEBUG_UI_WORK.type );
  if( cp_keytable ){
    if( cp_keytable->overlay_id != GFL_OVERLAY_BLANK_ID ){
  		GFL_OVERLAY_Unload( cp_keytable->overlay_id );
    }
  }
 
  // 初期化
  s_DEBUG_UI_WORK.type  = type;
  s_DEBUG_UI_WORK.count = 0;
  s_DEBUG_UI_WORK.data_max = 0;
  s_DEBUG_UI_WORK.cp_data = NULL;

  // テーブル設定
  cp_keytable = DebugUI_GetKeyTable( type );
  if( cp_keytable ){
    s_DEBUG_UI_WORK.data_max = cp_keytable->data_max;
    s_DEBUG_UI_WORK.cp_data  = cp_keytable->cp_data;

    if( cp_keytable->overlay_id != GFL_OVERLAY_BLANK_ID ){
  		GFL_OVERLAY_Load( cp_keytable->overlay_id );
    }
  }


  if( (type != DEBUG_UI_NONE) && (s_DEBUG_UI_WORK.cp_data != NULL) ){
    GFL_UI_DEBUG_OVERWRITE_SetCallBack( DebugUI_OverWriteCallback );
  }else{
    GFL_UI_DEBUG_OVERWRITE_SetCallBack( NULL );
  }

}

DEBUG_UI_TYPE DEBUG_UI_GetType( void )
{ 
  return s_DEBUG_UI_WORK.type; 
}




//-----------------------------------------------------------------------------
/**
 *    private関数
 */
//-----------------------------------------------------------------------------
static BOOL DebugUI_OverWriteCallback( GFL_UI_DEBUG_OVERWRITE* p_data )
{

  if( DEBUGWIN_IsActive() ){
    return FALSE;
  }

  *p_data = s_DEBUG_UI_WORK.cp_data[ s_DEBUG_UI_WORK.count ];
    
  // L+SELECTは保持
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
    if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_L) ){
      p_data->trg |= (PAD_BUTTON_SELECT);
      p_data->cont |= (PAD_BUTTON_L);
    }
  }

  s_DEBUG_UI_WORK.count = (s_DEBUG_UI_WORK.count + 1) % s_DEBUG_UI_WORK.data_max;
  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  typeのテーブルを取得
 *
 *	@param	type  タイプ
 *
 *	@return テーブル情報
 */
//-----------------------------------------------------------------------------
static const DEBUG_UI_KEY_TABLE* DebugUI_GetKeyTable( DEBUG_UI_TYPE type )
{
  int i;


  for( i=0; i<NELEMS(sc_DEBUG_UI_KEY_TABLE); i++ ){
    if( sc_DEBUG_UI_KEY_TABLE[i].type == type ){
      return &sc_DEBUG_UI_KEY_TABLE[i];
    }
  }
  return NULL;
}


#endif // GFL_UI_DEBUGSYS_ENABLE

#endif // PM_DEBUG
