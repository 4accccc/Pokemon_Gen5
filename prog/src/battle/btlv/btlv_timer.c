
//============================================================================================
/**
 * @file  btlv_timer.c
 * @brief 戦闘タイマー
 * @author  soga
 * @date  2009.10.27
 */
//============================================================================================

#include <gflib.h>

#include "btlv_effect.h"
#include "btlv_timer.h"

#include "arc_def.h"
#include "battle/battgra_wb.naix"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================
enum
{ 
  BTLV_TIMER_GAME_TIME = 0,
  BTLV_TIMER_COMMAND_TIME,
  BTLV_TIMER_SEPARATES,
  BTLV_TIMER_ZERO,
  BTLV_TIMER_ONE,
  BTLV_TIMER_TWO,
  BTLV_TIMER_THREE,
  BTLV_TIMER_FOUR,
  BTLV_TIMER_FIVE,
  BTLV_TIMER_SIX,
  BTLV_TIMER_SEVEN,
  BTLV_TIMER_EIGHT,
  BTLV_TIMER_NINE,

  BTLV_TIMER_GAME_TIME_Y    = 128,
  BTLV_TIMER_COMMAND_TIME_Y = 136,

  BTLV_TIMER_LABEL_X      = 0,
  BTLV_TIMER_MINUTE_10_X  = BTLV_TIMER_LABEL_X + 64,
  BTLV_TIMER_MINUTE_01_X  = BTLV_TIMER_MINUTE_10_X + 4,
  BTLV_TIMER_SEPARATES_X  = BTLV_TIMER_MINUTE_01_X + 4,
  BTLV_TIMER_SECOND_10_X  = BTLV_TIMER_SEPARATES_X + 6,
  BTLV_TIMER_SECOND_01_X  = BTLV_TIMER_SECOND_10_X + 4,

  BTLV_TIMER_LABEL = 0,
  BTLV_TIMER_MINUTE_10,
  BTLV_TIMER_MINUTE_01,
  BTLV_TIMER_SEPATATES,
  BTLV_TIMER_SECOND_10,
  BTLV_TIMER_SECOND_01,
  BTLV_TIMER_CLWK_MAX,

  BTLV_TIMER_1SEC_FRAME = 60,
};

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

struct  _BTLV_TIMER_CLWK
{ 
  GFL_CLWK*     clwk[ BTLV_TIMER_CLWK_MAX ];
  GFL_CLACTPOS  pos;
};

struct  _BTLV_TIMER_WORK
{ 
  GFL_CLUNIT*     clunit[ BTLV_TIMER_TYPE_MAX ];
  BTLV_TIMER_CLWK btcl[ BTLV_TIMER_TYPE_MAX ];

  u32             charID;
  u32             cellID;
  u32             plttID;

  int             minute[ BTLV_TIMER_TYPE_MAX ];
  int             minute_temp[ BTLV_TIMER_TYPE_MAX ];
  int             second[ BTLV_TIMER_TYPE_MAX ];

  int             frame[ BTLV_TIMER_TYPE_MAX ];

  GFL_TCB*        draw;
  GFL_TCB*        count_down;

  HEAPID          heapID;
};

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  void  BTLV_TIMER_Draw( BTLV_TIMER_WORK* btw, BTLV_TIMER_TYPE type );
static  void  TCB_BTLV_TIMER_Draw( GFL_TCB *tcb, void *work );
static  void	TCB_BTLV_TIMER_CountDown( GFL_TCB *tcb, void *work );

//============================================================================================
/**
 *  @brief  システム初期化
 *
 *  @param[in]  heapID  ヒープID
 */
//============================================================================================
BTLV_TIMER_WORK*  BTLV_TIMER_Init( HEAPID heapID )
{ 
  BTLV_TIMER_WORK *btw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_TIMER_WORK ) );
  ARCHANDLE*      handle;

  btw->heapID = heapID;
  handle = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, btw->heapID );

  btw->clunit[ BTLV_TIMER_TYPE_GAME_TIME ] = GFL_CLACT_UNIT_Create( BTLV_TIMER_CLWK_MAX, 0, btw->heapID );
  btw->clunit[ BTLV_TIMER_TYPE_COMMAND_TIME ] = GFL_CLACT_UNIT_Create( BTLV_TIMER_CLWK_MAX, 0, btw->heapID );
  btw->charID = GFL_CLGRP_CGR_Register( handle, NARC_battgra_wb_battle_time_num_NCGR, FALSE, CLSYS_DRAW_MAIN, btw->heapID );
  btw->cellID = GFL_CLGRP_CELLANIM_Register( handle, NARC_battgra_wb_battle_time_num_NCER,
                                             NARC_battgra_wb_battle_time_num_NANR, btw->heapID );
  btw->plttID = GFL_CLGRP_PLTT_Register( handle, NARC_battgra_wb_gauge_NCLR, CLSYS_DRAW_MAIN, 0x20, btw->heapID );
  PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_MAIN_OBJ,
                           GFL_CLGRP_PLTT_GetAddr( btw->plttID, CLSYS_DRAW_MAIN ) / 2, 0x20 * 1 );

  GFL_ARC_CloseDataHandle( handle );

  return btw;
}

//============================================================================================
/**
 *  @brief  システム終了
 *
 *  @param[in]  btw  システム管理構造体のポインタ
 */
//============================================================================================
void  BTLV_TIMER_Exit( BTLV_TIMER_WORK* btw )
{ 
  BTLV_TIMER_Delete( btw );

  GFL_CLGRP_CGR_Release( btw->charID );
  GFL_CLGRP_CELLANIM_Release( btw->cellID );
  GFL_CLGRP_PLTT_Release( btw->plttID );
  GFL_CLACT_UNIT_Delete( btw->clunit[ BTLV_TIMER_TYPE_GAME_TIME ] );
  GFL_CLACT_UNIT_Delete( btw->clunit[ BTLV_TIMER_TYPE_COMMAND_TIME ] );
  GFL_HEAP_FreeMemory( btw );
}

//============================================================================================
/**
 *  @brief  タイマー生成
 *
 *  @param[in]  btw           システム管理構造体のポインタ
 *  @param[in]  game_time     初期タイマー値（分）
 *  @param[in]  command_time  初期タイマー値（分）
 */
//============================================================================================
void  BTLV_TIMER_Create( BTLV_TIMER_WORK* btw, int game_time, int command_time )
{ 
  GFL_CLWK_DATA timer = {
    0, 0,     //x, y
    0, 0, 0,  //アニメ番号、優先順位、BGプライオリティ
  };
  int  timer_pos_x[] = { 
    BTLV_TIMER_LABEL_X,
    BTLV_TIMER_MINUTE_10_X,
    BTLV_TIMER_MINUTE_01_X,
    BTLV_TIMER_SEPARATES_X,
    BTLV_TIMER_SECOND_10_X,
    BTLV_TIMER_SECOND_01_X,
  };
  int  timer_pos_y[] = { 
    BTLV_TIMER_GAME_TIME_Y,
    BTLV_TIMER_COMMAND_TIME_Y,
  };
  int timer_anm[ 2 ][ BTLV_TIMER_CLWK_MAX ] = { 
    { BTLV_TIMER_GAME_TIME,    BTLV_TIMER_ZERO, BTLV_TIMER_ZERO, BTLV_TIMER_SEPARATES, BTLV_TIMER_ZERO, BTLV_TIMER_ZERO },
    { BTLV_TIMER_COMMAND_TIME, BTLV_TIMER_ZERO, BTLV_TIMER_ZERO, BTLV_TIMER_SEPARATES, BTLV_TIMER_ZERO, BTLV_TIMER_ZERO },
  };
  int i,j;

  for( i = 0 ; i < BTLV_TIMER_TYPE_MAX ; i++ )
  { 
    for( j = 0 ; j < BTLV_TIMER_CLWK_MAX ; j++ )
    { 
      timer.pos_x = timer_pos_x[ j ];
      timer.pos_y = timer_pos_y[ i ];
      timer.anmseq = timer_anm[ i ][ j ];
      btw->btcl[ i ].clwk[ j ] = GFL_CLACT_WK_Create( btw->clunit[ i ], btw->charID, btw->plttID, btw->cellID,
                                                      &timer, CLSYS_DEFREND_MAIN, btw->heapID );
      GFL_CLACT_WK_SetAutoAnmFlag( btw->btcl[ i ].clwk[ j ], TRUE );
    }
  }
  btw->minute[ BTLV_TIMER_TYPE_GAME_TIME ]         = game_time;
  btw->minute_temp[ BTLV_TIMER_TYPE_GAME_TIME ]    = game_time;
  btw->minute[ BTLV_TIMER_TYPE_COMMAND_TIME ]      = command_time;
  btw->minute_temp[ BTLV_TIMER_TYPE_COMMAND_TIME ] = command_time;

  BTLV_TIMER_Draw( btw, BTLV_TIMER_TYPE_GAME_TIME );
  BTLV_TIMER_Draw( btw, BTLV_TIMER_TYPE_COMMAND_TIME );

  btw->draw       = GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), TCB_BTLV_TIMER_Draw, btw, 0 );
	btw->count_down = GFUser_VIntr_CreateTCB( TCB_BTLV_TIMER_CountDown, btw, 0 );
}

//============================================================================================
/**
 *  @brief  タイマー削除
 *
 *  @param[in]  btw  システム管理構造体のポインタ
 */
//============================================================================================
void  BTLV_TIMER_Delete( BTLV_TIMER_WORK* btw )
{ 
  int i,j;

  for( i = 0 ; i < BTLV_TIMER_TYPE_MAX ; i++ )
  { 
    for( j = 0 ; j < BTLV_TIMER_CLWK_MAX ; j++ )
    { 
      if( btw->btcl[ i ].clwk[ j ] ) 
      { 
        GFL_CLACT_WK_Remove( btw->btcl[ i ].clwk[ j ] );
        btw->btcl[ i ].clwk[ j ] = NULL; 
      }
    }
  }
  if( btw->draw )
  { 
    GFL_TCB_DeleteTask( btw->draw );
    btw->draw = NULL;
  }
  if( btw->count_down )
  { 
    GFL_TCB_DeleteTask( btw->count_down );
    btw->count_down = NULL;
  }
}

//============================================================================================
/**
 *  @brief  タイマー描画許可
 *
 *  @param[in]  btw     システム管理構造体のポインタ
 *  @param[in]  type    タイマータイプ
 *  @param[in]  enable  TRUE:描画　FALSE:非描画
 *  @param[in]  init    TRUE:カウンタを初期化して描画　FALSE:初期化せず描画（enableがTRUEのときにしか意味がありません）
 *
 */
//============================================================================================
void  BTLV_TIMER_SetDrawEnable( BTLV_TIMER_WORK* btw, BTLV_TIMER_TYPE type, BOOL enable, BOOL init )
{ 
  if( ( enable == TRUE ) && ( init == TRUE ))
  { 
    btw->minute[ type ] = btw->minute_temp[ type ];
    btw->second[ type ] = 0;
    btw->frame[ type ] = 0;
    BTLV_TIMER_Draw( btw, type );
  }

  GFL_CLACT_UNIT_SetDrawEnable( btw->clunit[ type ], enable );
}

//============================================================================================
/**
 *  @brief  タイマー描画
 *
 *  @param[in]  btw   システム管理構造体のポインタ
 *  @param[in]  type  タイマータイプ
 */
//============================================================================================
static  void  BTLV_TIMER_Draw( BTLV_TIMER_WORK* btw, BTLV_TIMER_TYPE type )
{ 
  int minute_10 = btw->minute[ type ] / 10;
  int minute_01 = btw->minute[ type ] % 10;
  int second_10 = btw->second[ type ] / 10;
  int second_01 = btw->second[ type ] % 10;

  if( minute_10 == 0 )
  { 
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_10 ], FALSE );
  }
  else
  { 
    GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_10 ], BTLV_TIMER_ZERO + minute_10 );
  }

  if( ( minute_10 == 0 ) && ( minute_01 == 0 ) )
  { 
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_01 ], FALSE );
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_SEPATATES ], FALSE );
  }
  else
  { 
    GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_01 ], BTLV_TIMER_ZERO + minute_01 );
  }

  if( ( minute_10 == 0 ) && ( minute_01 == 0 ) && ( second_10 == 0 ) )
  { 
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_SECOND_10 ], FALSE );
  }
  else
  { 
    GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_SECOND_10 ], BTLV_TIMER_ZERO + second_10 );
  }

  GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_SECOND_01 ], BTLV_TIMER_ZERO + second_01 );
}

//============================================================================================
/**
 *  @brief  タイマー描画タスク
 */
//============================================================================================
static  void  TCB_BTLV_TIMER_Draw( GFL_TCB *tcb, void *work )
{ 
  BTLV_TIMER_WORK *btw = ( BTLV_TIMER_WORK* )work;

  BTLV_TIMER_Draw( btw, BTLV_TIMER_TYPE_GAME_TIME );
  BTLV_TIMER_Draw( btw, BTLV_TIMER_TYPE_COMMAND_TIME );
}

//============================================================================================
/**
 *  @brief  タイマーカウントダウンタスク
 */
//============================================================================================
static  void	TCB_BTLV_TIMER_CountDown( GFL_TCB *tcb, void *work )
{ 
  BTLV_TIMER_WORK *btw = ( BTLV_TIMER_WORK* )work;
  int i;

  for( i = 0 ; i < BTLV_TIMER_TYPE_MAX ; i++ )
  { 
    btw->frame[ i ]++;
    if( btw->frame[ i ] >= BTLV_TIMER_1SEC_FRAME )
    { 
      btw->frame[ i ] = 0;
      
      if( btw->second[ i ] == 0 )
      { 
        if( btw->minute[ i ] )
        { 
          btw->second[ i ] = 59;
          btw->minute[ i ]--;
        }
      }
      else
      { 
        btw->second[ i ]--;
      }
    }
  }
}

