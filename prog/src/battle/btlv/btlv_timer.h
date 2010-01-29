
//============================================================================================
/**
 * @file  btlv_timer.h
 * @brief 戦闘タイマー
 * @author  soga
 * @date  2009.10.27
 */
//============================================================================================

#pragma once

typedef enum
{ 
  BTLV_TIMER_TYPE_GAME_TIME = 0,
  BTLV_TIMER_TYPE_COMMAND_TIME,
  BTLV_TIMER_TYPE_MAX,
}BTLV_TIMER_TYPE;

typedef struct _BTLV_TIMER_WORK BTLV_TIMER_WORK;
typedef struct _BTLV_TIMER_CLWK BTLV_TIMER_CLWK;

extern  BTLV_TIMER_WORK*  BTLV_TIMER_Init( HEAPID heapID );
extern  void              BTLV_TIMER_Exit( BTLV_TIMER_WORK *btw );
extern  void              BTLV_TIMER_Create( BTLV_TIMER_WORK* btw, int game_time, int command_time );
extern  void              BTLV_TIMER_Delete( BTLV_TIMER_WORK* btw );
extern  void              BTLV_TIMER_SetDrawEnable( BTLV_TIMER_WORK* btw, BTLV_TIMER_TYPE type, BOOL enable, BOOL init );
extern  BOOL              BTLV_TIMER_IsZero( BTLV_TIMER_WORK* btw, BTLV_TIMER_TYPE type );

