
//============================================================================================
/**
 * @file  btlv_finger_cursor.h
 * @brief 指カーソル
 * @author  soga
 * @date  2010.01.28
 */
//============================================================================================

#pragma once

typedef struct _BTLV_FINGER_CURSOR_WORK BTLV_FINGER_CURSOR_WORK;

extern  BTLV_FINGER_CURSOR_WORK*  BTLV_FINGER_CURSOR_Init( int pal, HEAPID heapID );
extern  void                      BTLV_FINGER_CURSOR_Exit( BTLV_FINGER_CURSOR_WORK* bfcw );
extern  void                      BTLV_FINGER_CURSOR_Main( BTLV_FINGER_CURSOR_WORK* bfcw );
extern  BOOL                      BTLV_FINGER_CURSOR_Create( BTLV_FINGER_CURSOR_WORK* bfcw,
                                                             int pos_x, int pos_y, int count, int frame, int wait );
extern  void                      BTLV_FINGER_CURSOR_Delete( BTLV_FINGER_CURSOR_WORK* bfcw );
extern  BOOL                      BTLV_FINGER_CURSOR_CheckExecute( BTLV_FINGER_CURSOR_WORK* bfcw );
