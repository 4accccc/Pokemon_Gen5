//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		boast_message.c
 *	@brief  すれ違い  自慢メッセージ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.09.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/boast_message.h"

#include "boast_message_local.h"
#include "options_local.h"      // OPTIONS_SAVEDATA

#include "system/main.h"             // for HEAPID_xxxx
#include "arc/arc_def.h"             // for ARCID_xxxx
#include "arc/script_message.naix"   // for NARC_xxxx
#include "msg/script/msg_c03r0101.h" // for msg_xxxx

//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief    構造体サイズを得る
 *	@return   サイズ
 */
//-----------------------------------------------------------------------------
int BOAST_MSG_SAVEDATA_GetWorkSize( void )
{
  return sizeof(BOAST_MSG_SAVEDATA);
}

//----------------------------------------------------------------------------
/**
 *	@brief  中身を初期化
 *
 *	@param	BOAST_MSG_SAVEDATA *p_boast_msg_sv 
 */
//-----------------------------------------------------------------------------
void BOAST_MSG_SAVEDATA_Init( BOAST_MSG_SAVEDATA *p_boast_msg_sv )
{
	GFL_STD_MemClear(p_boast_msg_sv, BOAST_MSG_SAVEDATA_GetWorkSize());

  // すれ違いデフォルトメッセージ
  {
    GFL_MSGDATA* msg;
    STRBUF* strbuf;

    msg = GFL_MSG_Create( 
        GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
        NARC_script_message_c03r0101_dat, GetHeapLowID(HEAPID_SAVE) );

    // 自慢
    strbuf = GFL_MSG_CreateString( msg, msg_c03r0101_input_c );
    GFL_STR_GetStringCode( 
        strbuf, p_boast_msg_sv->boast_message, SAVE_SURETIGAI_BOAST_LEN );
    GFL_STR_DeleteBuffer( strbuf );

    GFL_MSG_Delete( msg );
  }
}

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  すれちがい自慢メッセージデータのCONST版ポインタ取得
 *
 *	@param	const SAVE_CONTROL_WORK * cp_sv 
 *
 *	@return BOAST_MSG_SAVEDATA
 */
//-----------------------------------------------------------------------------
const BOAST_MSG_SAVEDATA * BOAST_MSG_SAVEDATA_GetSaveDataConst( const SAVE_CONTROL_WORK * cp_sv )
{
	return (const BOAST_MSG_SAVEDATA *)BOAST_MSG_SAVEDATA_GetSaveData( (SAVE_CONTROL_WORK *)cp_sv);
}

//----------------------------------------------------------------------------
/**
 *	@brief  すれちがい自慢メッセージデータのポインタ取得
 *
 *	@param	const SAVE_CONTROL_WORK * cp_sv 
 *
 *	@return BOAST_MSG_SAVEDATA
 */
//-----------------------------------------------------------------------------
BOAST_MSG_SAVEDATA * BOAST_MSG_SAVEDATA_GetSaveData( SAVE_CONTROL_WORK * p_sv )
{
  OPTIONS_SAVEDATA *opt;
  BOAST_MSG_SAVEDATA *p_msg;

  opt = SaveControl_DataPtrGet( p_sv, GMDATA_ID_OPTIONS );
  p_msg = &opt->boast_message;
  return p_msg;
}

//----------------------------------------------------------
//メッセージ取得・設定
//----------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  すれ違い通信の自慢メッセージをセット
 *
 *	@param	BOAST_MSG_SAVEDATA *p_sv
 *	@param	STRBUF *srcbuf 
 */
//-----------------------------------------------------------------------------
void BOAST_MSG_SAVEDATA_SetMsg( BOAST_MSG_SAVEDATA *p_sv, const STRBUF *srcbuf )
{
  GFL_STR_GetStringCode( srcbuf, p_sv->boast_message, SAVE_SURETIGAI_BOAST_LEN );
}
//----------------------------------------------------------------------------
/**
 *	@brief  すれ違い通信の自慢メッセージを取得
 *
 *	@param	const BOAST_MSG_SAVEDATA *cp_sv 
 *
 *	@return const STRCODE *
 */
//-----------------------------------------------------------------------------
const STRCODE * BOAST_MSG_SAVEDATA_GetMsg( const BOAST_MSG_SAVEDATA *cp_sv )
{
  return cp_sv->boast_message;
}

