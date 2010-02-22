//=============================================================================
/**
 * @file	  app_printsys_common.c
 * @brief	  PRINTSYS_PrintStreamGetStateの処理を共通化するプログラム
 * @author  k.ohno
 * @date	  10/02/22
 */
//=============================================================================

#include <gflib.h>
#include "app/app_printsys_common.h"


//----------------------------------------------------------------------------
/**
 *	@brief	プリントシステム制御初期化
 *	@param	トリガーを制御すりワーク u8
 */
//-----------------------------------------------------------------------------

void APP_PRINTSYS_COMMON_PrintStreamInit(u8* trgWork)
{
 *trgWork = FALSE; 
}


//----------------------------------------------------------------------------
/**
 *	@brief	プリントシステム実行関数
 *  @param  PRINT_STREAM* ストリームハンドル
 *	@param	初期化で渡したトリガーを制御すりワーク u8
 */
//-----------------------------------------------------------------------------

BOOL APP_PRINTSYS_COMMON_PrintStreamFunc(PRINT_STREAM* handle,u8* trgWork)
{
  if(handle){
    int state = PRINTSYS_PrintStreamGetState( handle );
    switch(state){
    case PRINTSTREAM_STATE_DONE:
		  return TRUE;// 終わっている
      break;
    case PRINTSTREAM_STATE_PAUSE:
      if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL)){
  		  PRINTSYS_PrintStreamReleasePause( handle );
   		}
      break;
    case PRINTSTREAM_STATE_RUNNING:
      // メッセージスキップ
			if(*trgWork==FALSE){               //キーを離して無いと有効にならない
         if( GFL_UI_KEY_GetCont() == 0 ){
            *trgWork=TRUE;
         }
      }
      else{
        if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) )
        {
          PRINTSYS_PrintStreamShortWait( handle, 0 );
        }
      }
      break;
    default:
      break;
    }
    return FALSE;  //まだ終わってない
  }
  return TRUE;// 終わっている
}


