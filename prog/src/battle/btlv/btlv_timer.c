
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
  BTLV_TIMER_SEPARATES_ANM,
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

  BTLV_TIMER_ALERT,

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
  BTLV_TIMER_SEPARATES,
  BTLV_TIMER_SECOND_10,
  BTLV_TIMER_SECOND_01,
  BTLV_TIMER_CLWK_MAX,

  BTLV_TIMER_1SEC_FRAME = 60,

  BTLV_TIMER_MAX = 99 * 60 + 59,
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

  int             second[ BTLV_TIMER_TYPE_MAX ];
  OSTick          tick[ BTLV_TIMER_TYPE_MAX ];

  int             alert[ BTLV_TIMER_TYPE_MAX ];
  
  GFL_TCB*        draw;
  GFL_TCB*        count_down;

  HEAPID          heapID;
#ifdef PM_DEBUG
  BOOL            timer_stop_flag;
  BOOL            timer_edit_flag;
  BOOL            timer_edit_wait;
  BTLV_TIMER_TYPE timer_edit_pos;
#endif
};

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  void  BTLV_TIMER_Draw( BTLV_TIMER_WORK* btw, BTLV_TIMER_TYPE type );
static  void  TCB_BTLV_TIMER_Draw( GFL_TCB *tcb, void *work );

#ifdef PM_DEBUG
static  void  edit_timer( BTLV_TIMER_WORK* btw );
#endif

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
  handle = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, GFL_HEAP_LOWID( btw->heapID ) );

  btw->clunit[ BTLV_TIMER_TYPE_GAME_TIME ] = GFL_CLACT_UNIT_Create( BTLV_TIMER_CLWK_MAX, 0, btw->heapID );
  btw->clunit[ BTLV_TIMER_TYPE_COMMAND_TIME ] = GFL_CLACT_UNIT_Create( BTLV_TIMER_CLWK_MAX, 0, btw->heapID );
  btw->charID = GFL_CLGRP_CGR_Register( handle, NARC_battgra_wb_battle_time_num_NCGR, FALSE, CLSYS_DRAW_MAIN, btw->heapID );
  btw->cellID = GFL_CLGRP_CELLANIM_Register( handle, NARC_battgra_wb_battle_time_num_NCER,
                                             NARC_battgra_wb_battle_time_num_NANR, btw->heapID );
  btw->plttID = GFL_CLGRP_PLTT_Register( handle, NARC_battgra_wb_gauge_NCLR, CLSYS_DRAW_MAIN,
                                         BTLV_OBJ_PLTT_TIMER, btw->heapID );
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
 *  @param[in]  game_time     初期タイマー値（秒）
 *  @param[in]  command_time  初期タイマー値（秒）
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
    { BTLV_TIMER_GAME_TIME,    BTLV_TIMER_ZERO, BTLV_TIMER_ZERO, BTLV_TIMER_SEPARATES_ANM, BTLV_TIMER_ZERO, BTLV_TIMER_ZERO },
    { BTLV_TIMER_COMMAND_TIME, BTLV_TIMER_ZERO, BTLV_TIMER_ZERO, BTLV_TIMER_SEPARATES_ANM, BTLV_TIMER_ZERO, BTLV_TIMER_ZERO },
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

  if( game_time > BTLV_TIMER_MAX )
  { 
    game_time = BTLV_TIMER_MAX;
  }
  if( command_time > BTLV_TIMER_MAX )
  { 
    command_time = BTLV_TIMER_MAX;
  }

  btw->second[ BTLV_TIMER_TYPE_GAME_TIME ]         = game_time;
  btw->second[ BTLV_TIMER_TYPE_COMMAND_TIME ]      = command_time;
  btw->tick[ BTLV_TIMER_TYPE_GAME_TIME ] = btw->tick[ BTLV_TIMER_TYPE_COMMAND_TIME ] = OS_GetTick();

//  BTLV_TIMER_Draw( btw, BTLV_TIMER_TYPE_GAME_TIME );
//  BTLV_TIMER_Draw( btw, BTLV_TIMER_TYPE_COMMAND_TIME );
  BTLV_TIMER_SetDrawEnable( btw, BTLV_TIMER_TYPE_GAME_TIME, FALSE, FALSE );
  BTLV_TIMER_SetDrawEnable( btw, BTLV_TIMER_TYPE_COMMAND_TIME, FALSE, FALSE );

  btw->draw       = GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), TCB_BTLV_TIMER_Draw, btw, 0 );
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
    btw->tick[ type ] = OS_GetTick();
    btw->alert[ type ] = 0;
    BTLV_TIMER_Draw( btw, type );
  }

  GFL_CLACT_UNIT_SetDrawEnable( btw->clunit[ type ], enable );
}

//============================================================================================
/**
 *  @brief  タイマーゼロチェック
 *
 *  @param[in]  btw     システム管理構造体のポインタ
 *  @param[in]  type    タイマータイプ
 *
 *  @retval FALSE:0じゃない TRUE:0です
 */
//============================================================================================
BOOL  BTLV_TIMER_IsZero( BTLV_TIMER_WORK* btw, BTLV_TIMER_TYPE type )
{ 
  int timer = btw->second[ type ] - OS_TicksToSeconds( OS_GetTick() - btw->tick[ type ] );

#ifdef PM_DEBUG
  //タイマー編集中は0になるタイミングが存在してしまうので、
  //編集中は0であることを返さないようにする
  if( btw->timer_edit_flag )
  { 
    return FALSE;
  }
#endif

  return ( timer <= 0 );
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
  int alert[ BTLV_TIMER_TYPE_MAX ] = { 300, 15 };
  int label_anm = ( type == BTLV_TIMER_TYPE_GAME_TIME ) ? BTLV_TIMER_GAME_TIME : BTLV_TIMER_COMMAND_TIME;
  int timer = btw->second[ type ] - OS_TicksToSeconds( OS_GetTick() - btw->tick[ type ] );
  int minute;
  int second;
  int minute_10;
  int minute_01;
  int second_10;
  int second_01;

  if( timer < 0 )
  { 
    timer = 0;
  }

  minute = timer / 60;
  second = timer % 60; 
  minute_10 = minute / 10;
  minute_01 = minute % 10;
  second_10 = second / 10;
  second_01 = second % 10;

#ifdef PM_DEBUG
  if( ( timer <= alert[ type ] ) && ( btw->timer_edit_flag == FALSE ) )
#else
  if( timer == alert[ type ] )
#endif
  { 
    btw->alert[ type ] = BTLV_TIMER_ALERT;
  }

  GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_LABEL ], label_anm + btw->alert[ type ] );

  if( minute_10 == 0 )
  { 
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_10 ], FALSE );
  }
  else
  { 
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_10 ], TRUE );
    GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_10 ], BTLV_TIMER_ZERO + minute_10 + btw->alert[ type ] );
  }

  if( ( minute_10 == 0 ) && ( minute_01 == 0 ) )
  { 
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_01 ], FALSE );
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_SEPARATES ], FALSE );
  }
  else
  { 
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_01 ], TRUE );
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_SEPARATES ], TRUE );
    GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_MINUTE_01 ], BTLV_TIMER_ZERO + minute_01 + btw->alert[ type ] );
    GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_SEPARATES ], BTLV_TIMER_SEPARATES_ANM + btw->alert[ type ] );
  }

  if( ( minute_10 == 0 ) && ( minute_01 == 0 ) && ( second_10 == 0 ) )
  { 
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_SECOND_10 ], FALSE );
  }
  else
  { 
    GFL_CLACT_WK_SetDrawEnable( btw->btcl[ type ].clwk[ BTLV_TIMER_SECOND_10 ], TRUE );
    GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_SECOND_10 ], BTLV_TIMER_ZERO + second_10 + btw->alert[ type ] );
  }

  GFL_CLACT_WK_SetAnmSeq( btw->btcl[ type ].clwk[ BTLV_TIMER_SECOND_01 ], BTLV_TIMER_ZERO + second_01 + btw->alert[ type ] );
}

//============================================================================================
/**
 *  @brief  タイマー描画タスク
 */
//============================================================================================
static  void  TCB_BTLV_TIMER_Draw( GFL_TCB *tcb, void *work )
{ 
  BTLV_TIMER_WORK *btw = ( BTLV_TIMER_WORK* )work;

#ifdef PM_DEBUG
  if( ( btw->timer_stop_flag ) || ( btw->timer_edit_flag ) )
  { 
    btw->tick[ BTLV_TIMER_TYPE_GAME_TIME ]    = OS_GetTick();
    btw->tick[ BTLV_TIMER_TYPE_COMMAND_TIME ] = OS_GetTick();
  }
  edit_timer( btw );
#endif

  BTLV_TIMER_Draw( btw, BTLV_TIMER_TYPE_GAME_TIME );
  BTLV_TIMER_Draw( btw, BTLV_TIMER_TYPE_COMMAND_TIME );
}

#ifdef PM_DEBUG
//============================================================================================
/**
 *  @brief  タイマーストップオン／オフ
 */
//============================================================================================
BOOL  BTLV_TIMER_SwitchTimerStopFlag( BTLV_TIMER_WORK* btw )
{ 
  BOOL  ret = ( btw->draw != NULL );
  if( ret )
  { 
    btw->timer_stop_flag ^= 1;
    if( btw->timer_stop_flag )
    { 
      btw->second[ BTLV_TIMER_TYPE_GAME_TIME ] -=
        OS_TicksToSeconds( OS_GetTick() - btw->tick[ BTLV_TIMER_TYPE_GAME_TIME ] );
      btw->second[ BTLV_TIMER_TYPE_COMMAND_TIME ] -=
        OS_TicksToSeconds( OS_GetTick() - btw->tick[ BTLV_TIMER_TYPE_COMMAND_TIME ] );
    }
  }
  return ret;
}

//============================================================================================
/**
 *  @brief  タイマーエディットオン／オフ
 */
//============================================================================================
BOOL  BTLV_TIMER_SwitchTimerEditFlag( BTLV_TIMER_WORK* btw )
{ 
  BOOL  ret = ( btw->draw != NULL );
  if( ret )
  { 
    btw->timer_edit_flag ^= 1;
    if( btw->timer_edit_flag )
    { 
      btw->timer_edit_pos   = 0;
      btw->timer_edit_wait  = TRUE;
      btw->alert[ BTLV_TIMER_TYPE_GAME_TIME ]    = BTLV_TIMER_ALERT;
      btw->alert[ BTLV_TIMER_TYPE_COMMAND_TIME ] = 0;
      btw->second[ BTLV_TIMER_TYPE_GAME_TIME ] -=
        OS_TicksToSeconds( OS_GetTick() - btw->tick[ BTLV_TIMER_TYPE_GAME_TIME ] );
      btw->second[ BTLV_TIMER_TYPE_COMMAND_TIME ] -=
        OS_TicksToSeconds( OS_GetTick() - btw->tick[ BTLV_TIMER_TYPE_COMMAND_TIME ] );
    }
    else
    { 
      btw->alert[ BTLV_TIMER_TYPE_GAME_TIME ]    = 0;
      btw->alert[ BTLV_TIMER_TYPE_COMMAND_TIME ] = 0;
    }
  }
  return ret;
}

//============================================================================================
/**
 *  @brief  タイマーエディット
 */
//============================================================================================
static  void  edit_timer( BTLV_TIMER_WORK* btw )
{ 
  if( btw->timer_edit_flag == FALSE ) return;

  { 
    int cont  = GFL_UI_KEY_GetCont();
    int trg   = GFL_UI_KEY_GetTrg();
    int rep   = GFL_UI_KEY_GetRepeat();

    if( ( btw->timer_edit_wait ) && ( cont & PAD_BUTTON_L ) ) return;

    btw->timer_edit_wait = FALSE;

    if( ( trg & PAD_KEY_UP ) || ( trg & PAD_KEY_DOWN ) )
    { 
      btw->alert[ btw->timer_edit_pos ] = 0;
      btw->timer_edit_pos ^= 1;
      btw->alert[ btw->timer_edit_pos ] = BTLV_TIMER_ALERT;
    }
    if( ( rep & PAD_BUTTON_A ) && ( btw->second[ btw->timer_edit_pos ] < BTLV_TIMER_MAX ) )
    { 
      btw->second[ btw->timer_edit_pos ]++;
    }
    if( ( rep & PAD_BUTTON_B ) && ( btw->second[ btw->timer_edit_pos ] ) )
    { 
      btw->second[ btw->timer_edit_pos ]--;
    }
    if( rep & PAD_BUTTON_X )
    { 
      if( ( btw->second[ btw->timer_edit_pos ] + 10 ) < BTLV_TIMER_MAX )
      { 
        btw->second[ btw->timer_edit_pos ] += 10;
      }
      else
      { 
        btw->second[ btw->timer_edit_pos ] = BTLV_TIMER_MAX;
      }
    }
    if( rep & PAD_BUTTON_Y )
    { 
      if( ( btw->second[ btw->timer_edit_pos ] - 10 ) > 0 )
      { 
        btw->second[ btw->timer_edit_pos ] -= 10;
      }
      else
      { 
        btw->second[ btw->timer_edit_pos ] = 0;
      }
    }
    if( rep & PAD_BUTTON_R )
    { 
      if( ( btw->second[ btw->timer_edit_pos ] + 60 ) < BTLV_TIMER_MAX )
      { 
        btw->second[ btw->timer_edit_pos ] += 60;
      }
      else
      { 
        btw->second[ btw->timer_edit_pos ] = BTLV_TIMER_MAX;
      }
    }
    if( rep & PAD_BUTTON_L )
    { 
      if( ( btw->second[ btw->timer_edit_pos ] - 60 ) > 0 )
      { 
        btw->second[ btw->timer_edit_pos ] -= 60;
      }
      else
      { 
        btw->second[ btw->timer_edit_pos ] = 0;
      }
    }
  }
}

#endif
