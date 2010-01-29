
//============================================================================================
/**
 * @file  btlv_mcss.h
 * @brief ポケモンMCSS管理
 * @author  soga
 * @date  2008.11.14
 */
//============================================================================================

#pragma once

#include "system/mcss.h"
#include "poke_tool/poke_tool.h"
#include "battle/battle.h"

//ポケモンの立ち位置定義
// 1vs1
//    BB
//
// AA
//
// 2vs2
//    D B
//
// A C
//
// 3vs3
//     F
//    D B
//
// A C
//  E
typedef enum{
  BTLV_MCSS_POS_AA = 0,
  BTLV_MCSS_POS_BB,
  BTLV_MCSS_POS_A,
  BTLV_MCSS_POS_B,
  BTLV_MCSS_POS_C,
  BTLV_MCSS_POS_D,
  BTLV_MCSS_POS_E,
  BTLV_MCSS_POS_F,

  BTLV_MCSS_POS_MAX,

  BTLV_MCSS_POS_TR_AA = BTLV_MCSS_POS_MAX,
  BTLV_MCSS_POS_TR_BB,
  BTLV_MCSS_POS_TR_A,
  BTLV_MCSS_POS_TR_B,
  BTLV_MCSS_POS_TR_C,
  BTLV_MCSS_POS_TR_D,

  BTLV_MCSS_POS_TOTAL,    //ポケモンとトレーナーをあわせたMAX値

  BTLV_MCSS_POS_PAIR_BIT = 0x02,
  BTLV_MCSS_POS_ERROR = 0xff
}BtlvMcssPos;

typedef enum{
  BTLV_MCSS_PROJ_PERSPECTIVE = 0,
  BTLV_MCSS_PROJ_ORTHO,
  BTLV_MCSS_PROJ_MAX,
}BTLV_MCSS_PROJECTION;

#define BTLV_MCSS_MEPACHI_ON  ( MCSS_MEPACHI_ON )
#define BTLV_MCSS_MEPACHI_OFF ( MCSS_MEPACHI_OFF )
#define BTLV_MCSS_MEPACHI_FLIP  ( MCSS_MEPACHI_FLIP )
#define BTLV_MCSS_ANM_STOP_ON ( MCSS_ANM_STOP_ON )
#define BTLV_MCSS_ANM_STOP_OFF  ( MCSS_ANM_STOP_OFF )
#define BTLV_MCSS_VANISH_ON   ( MCSS_VANISH_ON )
#define BTLV_MCSS_VANISH_OFF  ( MCSS_VANISH_OFF )
#define BTLV_MCSS_VANISH_FLIP ( MCSS_VANISH_FLIP )

typedef enum{ 
  BTLV_MCSS_STATUS_FLAG_MIGAWARI = 0x00000001,    //みがわりフラグ
}BTLV_MCSS_STATUS_FLAG;

typedef struct _BTLV_MCSS BTLV_MCSS;
typedef struct _BTLV_MCSS_WORK BTLV_MCSS_WORK;

typedef struct
{ 
  BTLV_MCSS_WORK* bmw;
  BtlvMcssPos     position;
  int             axis;               //回転軸
  int             shift;              //回転シフト
	fx32	          radius_h;           //横方向半径
	fx32	          radius_v;           //縦方向半径
	int		          frame;              //1回転何フレームか
	int		          rotate_wait;        //回転ウエイト
	int		          count;              //回転数
	int		          rotate_after_wait;  //1回転した後のウエイト
  int             angle;
  int             speed;
  int             rotate_wait_count;
  int             rotate_after_wait_count;
}BTLV_MCSS_MOVE_CIRCLE_PARAM;

extern  BTLV_MCSS_WORK* BTLV_MCSS_Init( BtlRule rule, GFL_TCBSYS *tcb_sys, HEAPID heapID );
extern  void            BTLV_MCSS_Exit( BTLV_MCSS_WORK *bmw );
extern  void            BTLV_MCSS_Main( BTLV_MCSS_WORK *bmw );
extern  void            BTLV_MCSS_Draw( BTLV_MCSS_WORK *bmw );
extern  void            BTLV_MCSS_Add( BTLV_MCSS_WORK *bmw, const POKEMON_PARAM *pp, int position );
extern  void            BTLV_MCSS_AddTrainer( BTLV_MCSS_WORK *bmw, int tr_type, int position );
extern  void            BTLV_MCSS_Del( BTLV_MCSS_WORK *bmw, int position );
extern  void            BTLV_MCSS_SetPosition( BTLV_MCSS_WORK *bmw, int position, fx32 pos_x, fx32 pos_y, fx32 pos_z );
extern  void            BTLV_MCSS_SetOrthoMode( BTLV_MCSS_WORK *bmw );
extern  void            BTLV_MCSS_ResetOrthoMode( BTLV_MCSS_WORK *bmw );
extern  void            BTLV_MCSS_SetMepachiFlag( BTLV_MCSS_WORK *bmw, int position, int flag );
extern  void            BTLV_MCSS_SetAnmStopFlag( BTLV_MCSS_WORK *bmw, int position, int flag );
extern  int             BTLV_MCSS_GetVanishFlag( BTLV_MCSS_WORK *bmw, int position );
extern  void            BTLV_MCSS_SetVanishFlag( BTLV_MCSS_WORK *bmw, int position, int flag );
extern  void            BTLV_MCSS_GetPokeDefaultPos( BTLV_MCSS_WORK *bmw, VecFx32 *pos, int position );
extern  fx32            BTLV_MCSS_GetPokeDefaultScale( BTLV_MCSS_WORK *bmw, int position );
extern  fx32            BTLV_MCSS_GetPokeDefaultScaleEx( BTLV_MCSS_WORK *bmw, int position, BTLV_MCSS_PROJECTION proj );
extern  void            BTLV_MCSS_GetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale );
extern  void            BTLV_MCSS_SetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale );
extern  void            BTLV_MCSS_SetShadowVanishFlag( BTLV_MCSS_WORK *bmw, int position, u8 flag );
extern  void            BTLV_MCSS_SetAnmSpeed( BTLV_MCSS_WORK *bmw, int position, fx32 speed );
extern  void            BTLV_MCSS_MovePosition( BTLV_MCSS_WORK *bmw, int position, int move_type,
                                                VecFx32 *pos, int frame, int wait, int count );
extern  void            BTLV_MCSS_MoveScale( BTLV_MCSS_WORK *bmw, int position, int move_type,
                                             VecFx32 *scale, int frame, int wait, int count );
extern  void            BTLV_MCSS_MoveRotate( BTLV_MCSS_WORK *bmw, int position, int type,
                                              VecFx32 *rotate, int frame, int wait, int count );
extern  void            BTLV_MCSS_MoveBlink( BTLV_MCSS_WORK *bmw, int position, int type, int wait, int count );
extern  void            BTLV_MCSS_MoveAlpha( BTLV_MCSS_WORK *bmw, int position, int type,
                                             int alpha, int frame, int wait, int count );
extern  void            BTLV_MCSS_MoveCircle( BTLV_MCSS_WORK *bmw, BTLV_MCSS_MOVE_CIRCLE_PARAM* bmmcp );
extern  void            BTLV_MCSS_MoveMosaic( BTLV_MCSS_WORK *bmw, int position, int type,
                                              int mosaic, int frame, int wait, int count );
extern  BOOL            BTLV_MCSS_CheckTCBExecute( BTLV_MCSS_WORK *bmw, int position );
extern  BOOL            BTLV_MCSS_CheckExist( BTLV_MCSS_WORK *bmw, int position );
extern  void            BTLV_MCSS_SetPaletteFade( BTLV_MCSS_WORK *bmw, int position,
                                                  u8 start_evy, u8 end_evy, u8 wait, u32 rgb );
extern  u16             BTLV_MCSS_GetWeight( BTLV_MCSS_WORK *bmw, int position );
extern  u32             BTLV_MCSS_GetStatusFlag( BTLV_MCSS_WORK *bmw, int position );
extern  void            BTLV_MCSS_SetMigawari( BTLV_MCSS_WORK *bmw, int position, int sw, BOOL flag );
extern  void            BTLV_MCSS_SetMosaic( BTLV_MCSS_WORK *bmw, int position, int mosaic );
extern  void            BTLV_MCSS_CopyMAW( BTLV_MCSS_WORK *bmw, int src, int dst );

#ifdef PM_DEBUG
extern  void            BTLV_MCSS_AddDebug( BTLV_MCSS_WORK *bmw, const MCSS_ADD_DEBUG_WORK *madw, int position );
extern  void            BTLV_MCSS_SetMcss3vs3( BTLV_MCSS_WORK *bmw, int flag );
extern  void            BTLV_MCSS_SetMcssRotate( BTLV_MCSS_WORK *bmw, int flag );
extern  void            BTLV_MCSS_ClearExecute( BTLV_MCSS_WORK *bmw );
#endif

