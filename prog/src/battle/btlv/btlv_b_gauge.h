
//============================================================================================
/**
 * @file	btlv_b_gauge.h
 * @brief	戦闘ボールゲージ
 * @author	soga
 * @date	2009.07.23
 */
//============================================================================================

#pragma once
#include "battle/btl_common.h"

typedef struct _BTLV_BALL_GAUGE_WORK BTLV_BALL_GAUGE_WORK;

enum
{ 
  BTLV_BALL_GAUGE_STATUS_NONE = 0, ///<ポケモンがいない
  BTLV_BALL_GAUGE_STATUS_ALIVE,    ///<ポケモンがいる（いきてる）
  BTLV_BALL_GAUGE_STATUS_DEAD,     ///<ポケモンがいる（気絶）
  BTLV_BALL_GAUGE_STATUS_NG,       ///<ポケモンがいる（ステータス異常）
  BTLV_BALL_GAUGE_STATUS_ARROW,    ///<矢印表示用
  BTLV_BALL_GAUGE_STATUS_MAX,
};

typedef enum
{ 
  BTLV_BALL_GAUGE_ANM_NORMAL_E = 0,
  BTLV_BALL_GAUGE_ANM_NG_E,
  BTLV_BALL_GAUGE_ANM_DEAD_E,
  BTLV_BALL_GAUGE_ANM_NORMAL_M,
  BTLV_BALL_GAUGE_ANM_NG_M,
  BTLV_BALL_GAUGE_ANM_DEAD_M,
  BTLV_BALL_GAUGE_ANM_NORMAL,
  BTLV_BALL_GAUGE_ANM_NG,
  BTLV_BALL_GAUGE_ANM_DEAD,
  BTLV_BALL_GAUGE_ANM_NONE,
  BTLV_BALL_GAUGE_ANM_ARROW_E,
  BTLV_BALL_GAUGE_ANM_ARROW_M,
}BTLV_BALL_GAUGE_ANM_NO;

typedef enum
{
  BTLV_BALL_GAUGE_TYPE_MINE = 0,
  BTLV_BALL_GAUGE_TYPE_ENEMY,
  BTLV_BALL_GAUGE_TYPE_MAX,
}BTLV_BALL_GAUGE_TYPE;

typedef struct
{ 
  BTLV_BALL_GAUGE_TYPE  type;
  int                   status[ TEMOTI_POKEMAX ];
}BTLV_BALL_GAUGE_PARAM;


extern  BTLV_BALL_GAUGE_WORK* BTLV_BALL_GAUGE_Create( const BTLV_BALL_GAUGE_PARAM* bbgp, HEAPID heapID );
extern  void                  BTLV_BALL_GAUGE_Delete( BTLV_BALL_GAUGE_WORK* bbgw );
extern  BOOL                  BTLV_BALL_GAUGE_CheckExecute( BTLV_BALL_GAUGE_WORK* bbgw );
