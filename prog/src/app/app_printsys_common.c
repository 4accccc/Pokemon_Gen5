//=============================================================================
/**
 * @file	  app_printsys_common.c
 * @brief	  PRINTSYS_PrintStreamGetStateの処理を共通化するプログラム
 * @author  k.ohno
 * @date	  10/02/22
 */
//=============================================================================

#include <gflib.h>
#include "sound/pm_sndsys.h"
#include "app/app_printsys_common.h"


//-------------------------------------
///	プロトタイプ
//=====================================
static BOOL App_PrintSys_IsTrg( APP_PRINTSYS_COMMON_TYPE type );
static BOOL App_PrintSys_IsCont( APP_PRINTSYS_COMMON_TYPE type );
static BOOL App_PrintSys_IsRelease( APP_PRINTSYS_COMMON_TYPE type );

//----------------------------------------------------------------------------
/**
 *	@brief	プリントシステム制御初期化
 *	@param	APP_PRINTSYS_COMMON_WORK  ワーク
 *	@param  type                      入力タイプ
 */
//-----------------------------------------------------------------------------
void APP_PRINTSYS_COMMON_PrintStreamInit(APP_PRINTSYS_COMMON_WORK *wk, APP_PRINTSYS_COMMON_TYPE type)
{
  GFL_STD_MemClear( wk, sizeof(APP_PRINTSYS_COMMON_WORK) );
  wk->type  = type;
}

//----------------------------------------------------------------------------
/**
 *	@brief	プリントシステム実行関数
 *  @param  PRINT_STREAM* ストリームハンドル
 *	@param  handle  プリントストリーム
 *
 *	@retval TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL APP_PRINTSYS_COMMON_PrintStreamFunc( APP_PRINTSYS_COMMON_WORK *wk, PRINT_STREAM* handle )
{
  if(handle){
    int state = PRINTSYS_PrintStreamGetState( handle );
    BOOL is_input = FALSE;

    switch(state){
    case PRINTSTREAM_STATE_DONE:
		  return TRUE;// 終わっている
      break;

    case PRINTSTREAM_STATE_PAUSE:
      //入力
      if( App_PrintSys_IsTrg( wk->type ) )
      { 
        PMSND_PlaySE( APP_PRINTSYS_COMMON_SE_TRG );
  		  PRINTSYS_PrintStreamReleasePause( handle );
   		}
      break;

    case PRINTSTREAM_STATE_RUNNING:
      // メッセージスキップ
			if(wk->trg == FALSE){               //キーを離して無いと有効にならない
        if( App_PrintSys_IsRelease( wk->type ) )
        {
          wk->trg=TRUE;
        }
      }
      else{
        if( App_PrintSys_IsCont( wk->type ) )
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

//----------------------------------------------------------------------------
/**
 *	@brief  トリガ状態を検知
 *
 *	@param	APP_PRINTSYS_COMMON_TYPE type 入力タイプ
 *
 *	@return TRUEトリガしている FALSEで何か押している
 */
//-----------------------------------------------------------------------------
static BOOL App_PrintSys_IsTrg( APP_PRINTSYS_COMMON_TYPE type )
{
  BOOL is_trg = FALSE;

  //キーの場合
  if( type & APP_PRINTSYS_COMMON_TYPE_KEY ){ 
    if(GFL_UI_KEY_GetTrg() & APP_PRINTSYS_COMMON_TRG_KEY){
      is_trg  = TRUE;
    }
  }
  //タッチの場合
  if( type & APP_PRINTSYS_COMMON_TYPE_TOUCH ){ 
    if( GFL_UI_TP_GetTrg() ){ 
      is_trg  = TRUE;
    }
  }

  return is_trg;
}

//----------------------------------------------------------------------------
/**
 *	@brief  押し続け状態を検知
 *
 *	@param	APP_PRINTSYS_COMMON_TYPE type 入力タイプ
 *
 *	@return TRUEで押し続けている FALSEで何か押している
 */
//-----------------------------------------------------------------------------
static BOOL App_PrintSys_IsCont( APP_PRINTSYS_COMMON_TYPE type )
{   
  BOOL is_cont = FALSE;
  if( type & APP_PRINTSYS_COMMON_TYPE_KEY ){ 
    if( GFL_UI_KEY_GetCont() & APP_PRINTSYS_COMMON_TRG_KEY ){ 
      is_cont  = TRUE;
    }
  }

  if( type & APP_PRINTSYS_COMMON_TYPE_TOUCH ){ 
    if( GFL_UI_TP_GetCont() ){ 

      is_cont  = TRUE;
    }
  }

  return is_cont;
}

//----------------------------------------------------------------------------
/**
 *	@brief  離し状態を検知
 *
 *	@param	APP_PRINTSYS_COMMON_TYPE type 入力タイプ
 *
 *	@return TRUEで離している FALSEで何か押している
 */
//-----------------------------------------------------------------------------
static BOOL App_PrintSys_IsRelease( APP_PRINTSYS_COMMON_TYPE type )
{ 
  BOOL is_release = FALSE;
  if( type & APP_PRINTSYS_COMMON_TYPE_KEY ){ 
    if( GFL_UI_KEY_GetCont() == 0 ){ 
      is_release  = TRUE;
    }
  }

  if( type & APP_PRINTSYS_COMMON_TYPE_TOUCH ){ 
    if( GFL_UI_TP_GetCont() == 0 ){ 

      is_release  = TRUE;
    }
  }

  return is_release;
}
