//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_ui_input.c
 *	@brief  キー入力情報のバッファリング
 *	@author	tomoya takahashi
 *	@date		2010.05.01
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "debug_ui_input.h"

#include "debug/debugwin_sys.h"

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

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

#define DEBUG_UI_INPUT_BUF_MAX  ( 60*60*20 ) // 20分分
GFL_UI_DEBUG_OVERWRITE DEBUG_UI_InputBuf[ DEBUG_UI_INPUT_BUF_MAX ] = {
  {
    0
  },
};
static u32 DEBUG_UI_InputCount = 0;
static BOOL DEBUG_UI_InputOn = 0;


//----------------------------------------------------------------------------
/**
 *	@brief  入力開始
 */
//-----------------------------------------------------------------------------
void DEBUG_UI_INPUT_Start( void )
{
  if(DEBUG_UI_InputOn==FALSE){
    DEBUG_UI_InputCount = 0;
    DEBUG_UI_InputOn = TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  入力終了
 */
//-----------------------------------------------------------------------------
void DEBUG_UI_INPUT_End( void )
{
  if(DEBUG_UI_InputOn){
    DEBUG_UI_InputOn = FALSE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  情報の格納
 */
//-----------------------------------------------------------------------------
void DEBUG_UI_INPUT_Update( void )
{
  if( DEBUG_UI_InputOn ){
    u32 tp_x, tp_y;

    // L(cont) + SELECT(trg)の場合、無視。
    if( (GFL_UI_KEY_GetTrg() & DEBUGWIN_TRG_KEY) &&
        (GFL_UI_KEY_GetCont() & DEBUGWIN_CONT_KEY) ){
      TOMOYA_Printf( "R+Y???\n" );
      return ;
    }
    if( DEBUGWIN_IsActive() ){
      TOMOYA_Printf( "???\n" );
      return ;
    }

    DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].trg = GFL_UI_KEY_GetTrg();
    DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].cont = GFL_UI_KEY_GetCont();
    DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].repeat = GFL_UI_KEY_GetRepeat();
    
    DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].tp_cont = GFL_UI_TP_GetPointCont( &tp_x, &tp_y );
    DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].tp_trg = GFL_UI_TP_GetTrg();
    DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].tp_x = tp_x;
    DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].tp_y = tp_y;

    OS_TPrintf( "{ %d, %d, %d, %d, %d, %d, %d, },// count %d\n", 
        DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].trg,
        DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].cont,
        DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].repeat,
        DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].tp_x,
        DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].tp_y,
        DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].tp_trg,
        DEBUG_UI_InputBuf[ DEBUG_UI_InputCount ].tp_cont,
        DEBUG_UI_InputCount );
    
    DEBUG_UI_InputCount ++;

    if( DEBUG_UI_InputCount >= DEBUG_UI_INPUT_BUF_MAX ){
      //終了
      OS_TPrintf("DEBUG UI Input Sampling Buff Max End\n");
      DEBUG_UI_InputOn = FALSE;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  入力バッファデータ数取得
 *	@return 
 */
//-----------------------------------------------------------------------------
u32 DEBUG_UI_INPUT_GetInputBufNum( void )
{
  return DEBUG_UI_InputCount;
}

//----------------------------------------------------------------------------
/**
 *	@brief  入力バッファを取得
 *	@return バッファ
 */ 
//-----------------------------------------------------------------------------
const GFL_UI_DEBUG_OVERWRITE* DEBUG_UI_INPUT_GetInputBuf( void )
{
  return DEBUG_UI_InputBuf;
}

