//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_cgear.c
 *	@brief
 *	@author	 
 *	@date		2010.04.12
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"
#include "system/bmp_winframe.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_cgear.h"

#include "msgdata.h"
#include "msg/msg_cg_power.h"

#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

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
#define BMPWIN_POS_X  (1)
#define BMPWIN_POS_Y  (1)
#define BMPWIN_POS_SIZX  (30)
#define BMPWIN_POS_SIZY  (10)
#define BMPWIN_PAL  (11)

#define FRAME_PAL  (13)

#define USE_FRAME ( GFL_BG_FRAME1_M )

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//--------------------------------------------------------------
/**
 * @brief   ＣＧｅａｒ警告画面を出す
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallCGearCommentPut( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  HEAPID heapID = FIELDMAP_GetHeapID( fparam->fieldMap );
  GFL_MSGDATA*    p_msgdata;
  STRBUF*         p_str;
  GFL_BMPWIN*     p_bmp;
  FLDMSGBG *      p_fmb = FIELDMAP_GetFldMsgBG( fparam->fieldMap );
	u16 on_flag	= VMGetU16( core );
  
  if( on_flag ){

    FLDMSGBG_TransResource( USE_FRAME, heapID );

    // ON
    p_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_cg_power_dat, heapID );


    p_bmp = GFL_BMPWIN_Create( USE_FRAME, BMPWIN_POS_X, BMPWIN_POS_Y, 
        BMPWIN_POS_SIZX, BMPWIN_POS_SIZY, BMPWIN_PAL , GFL_BMP_CHRAREA_GET_B );

    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_bmp ), 15 );
    GFL_BMPWIN_MakeScreen( p_bmp );
    BmpWinFrame_Write( p_bmp, WINDOW_TRANS_ON, 1, FRAME_PAL );

    
    p_str = GFL_STR_CreateBuffer( 180, heapID );
    GFL_MSG_GetString( p_msgdata, cg_power_04, p_str );

    // 書き込む
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_bmp ), 0, 0, p_str, FLDMSGBG_GetFontHandle( p_fmb ) );
    
    // 強制転送
    GFL_BMPWIN_TransVramCharacter( p_bmp );

    GFL_BG_LoadScreenV_Req( USE_FRAME );

    // すべて破棄
    GFL_STR_DeleteBuffer( p_str );
    GFL_MSG_Delete( p_msgdata );
    GFL_BMPWIN_Delete( p_bmp );
  }else{

    // OFF
    GFL_BG_FillScreen( USE_FRAME, 0, BMPWIN_POS_X-1, BMPWIN_POS_Y-1,
        BMPWIN_POS_SIZX+2, BMPWIN_POS_SIZY+2, GFL_BG_SCRWRT_PALIN );     
    GFL_BG_LoadScreenV_Req( USE_FRAME );
  }
  

  
  return VMCMD_RESULT_CONTINUE;
}



