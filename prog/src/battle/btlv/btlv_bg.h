
//============================================================================================
/**
 * @file  btlv_bg.h
 * @brief 戦闘エフェクトBG制御
 * @author  soga
 * @date  2009.11.06
 */
//============================================================================================

#pragma once

typedef struct _BTLV_BG_WORK BTLV_BG_WORK;

extern  BTLV_BG_WORK* BTLV_BG_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID );
extern  void          BTLV_BG_Exit( BTLV_BG_WORK *bbw );

extern  void          BTLV_BG_MovePosition( BTLV_BG_WORK *bbw, int position, int type, int scr_x, int scr_y, int frame, int wait, int count );
extern  BOOL          BTLV_BG_CheckTCBExecute( BTLV_BG_WORK *bbw );
