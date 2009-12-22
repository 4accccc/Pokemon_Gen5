
//============================================================================================
/**
 * @file  btlv_b_gauge.c
 * @brief 戦闘ボールゲージ
 * @author  soga
 * @date  2009.07.23
 */
//============================================================================================

#include <gflib.h>

#include "btlv_effect.h"
#include "btlv_b_gauge.h"

#include "sound/pm_sndsys.h"

#include "arc_def.h"
#include "battle/battgra_wb.naix"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================

#define BTLV_BALL_GAUGE_CLWK_MAX  ( 7 )

enum{ 
  BTLV_BALL_GAUGE_BALL_1 = 0,
  BTLV_BALL_GAUGE_BALL_2,
  BTLV_BALL_GAUGE_BALL_3,
  BTLV_BALL_GAUGE_BALL_4,
  BTLV_BALL_GAUGE_BALL_5,
  BTLV_BALL_GAUGE_BALL_6,
  BTLV_BALL_GAUGE_ARROW,
};

enum
{ 
  BTLV_BALL_GAUGE_POS_BALL_X_E = 96,
  BTLV_BALL_GAUGE_POS_BALL_Y_E = 40,
  BTLV_BALL_GAUGE_POS_ARROW_X_E = 56,
  BTLV_BALL_GAUGE_POS_ARROW_Y_E = BTLV_BALL_GAUGE_POS_BALL_Y_E + 8,

  BTLV_BALL_GAUGE_POS_BALL_X_M = 160,
  BTLV_BALL_GAUGE_POS_BALL_Y_M = 112,
  BTLV_BALL_GAUGE_POS_ARROW_X_M = 184,
  BTLV_BALL_GAUGE_POS_ARROW_Y_M = BTLV_BALL_GAUGE_POS_BALL_Y_M + 8,

  BTLV_BALL_GAUGE_BALL_OFFSET = 16,
  BTLV_BALL_GAUGE_STOP_OFFSET = 2,

  BTLV_BALL_GAUGE_MOVE_OFFSET = 128,
  BTLV_BALL_GAUGE_MOVE_SPEED = 12,
  BTLV_BALL_GAUGE_ARROW_MOVE_SPEED = 16,

  BTLV_BALL_GAUGE_MOVE_WAIT = 6,
};

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================
typedef struct
{ 
  GFL_CLWK*       clwk;

  int             move_pos;
  int             stop_pos;
  int             speed;
  int             stop_anm_no;
  int             wait;

  GFL_CLACTPOS    now_pos;
}BTLV_BALL_GAUGE_CLWK;

struct _BTLV_BALL_GAUGE_WORK
{
  GFL_CLUNIT*           clunit;

  u32                   charID;
  u32                   cellID;
  u32                   plttID;

  BTLV_BALL_GAUGE_CLWK  bbgcl[ BTLV_BALL_GAUGE_CLWK_MAX ];

  int                   seq_no;

  int                   tcb_execute;

  int                   player_no;

  HEAPID                heapID;
};

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  void  TCB_BTLV_BALL_GAUGE_Anime( GFL_TCB* tcb, void* work );

//============================================================================================
/**
 *  @brief  システム初期化
 *
 *  @param[in] heapID    ヒープID
 *
 *  @retval  bbgw  BTLV_BALL_GAUGE_WORK管理構造体へのポインタ
 */
//============================================================================================
BTLV_BALL_GAUGE_WORK* BTLV_BALL_GAUGE_Create( const BTLV_BALL_GAUGE_PARAM* bbgp, HEAPID heapID )
{
  BTLV_BALL_GAUGE_WORK* bbgw    = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_BALL_GAUGE_WORK ) );
  ARCHANDLE*            handle  = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, heapID );

  bbgw->heapID = heapID;
  bbgw->clunit = GFL_CLACT_UNIT_Create( BTLV_BALL_GAUGE_CLWK_MAX, 0, bbgw->heapID );

  //リソース読み込み
  bbgw->charID = GFL_CLGRP_CGR_Register( handle, NARC_battgra_wb_gauge_m_ball_NCGR, FALSE, CLSYS_DRAW_MAIN, bbgw->heapID );
  bbgw->cellID = GFL_CLGRP_CELLANIM_Register( handle, NARC_battgra_wb_gauge_m_ball_NCER, NARC_battgra_wb_gauge_m_ball_NANR,
                                              bbgw->heapID );
  bbgw->plttID = GFL_CLGRP_PLTT_Register( handle, NARC_battgra_wb_gauge_m_ball_NCLR, CLSYS_DRAW_MAIN, 0x40, bbgw->heapID );
  PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_MAIN_OBJ,
                           GFL_CLGRP_PLTT_GetAddr( bbgw->plttID, CLSYS_DRAW_MAIN ) / 2, 0x20 * 1 );

  //CLWK
  { 
    int i;
    GFL_CLWK_DATA ball_gauge = {
      0, 0,     //x, y
      0, 0, 0,  //アニメ番号、優先順位、BGプライオリティ
    };
    static  const GFL_CLACTPOS  pos[][ 2 ] = {
      //自分側
      { 
        //ボール
        { BTLV_BALL_GAUGE_POS_BALL_X_M, BTLV_BALL_GAUGE_POS_BALL_Y_M },
        //矢印
        { BTLV_BALL_GAUGE_POS_ARROW_X_M, BTLV_BALL_GAUGE_POS_ARROW_Y_M },
      },
      //相手側
      { 
        //ボール
        { BTLV_BALL_GAUGE_POS_BALL_X_E, BTLV_BALL_GAUGE_POS_BALL_Y_E },
        //矢印
        { BTLV_BALL_GAUGE_POS_ARROW_X_E, BTLV_BALL_GAUGE_POS_ARROW_Y_E },
      },
    };
    static  const int anm_no_table[][ BTLV_BALL_GAUGE_STATUS_MAX ] = {  
      //自分側
      { 
        BTLV_BALL_GAUGE_ANM_NONE,
        BTLV_BALL_GAUGE_ANM_NORMAL_M,
        BTLV_BALL_GAUGE_ANM_DEAD_M,
        BTLV_BALL_GAUGE_ANM_NG_M,
        BTLV_BALL_GAUGE_ANM_ARROW_M,
      },
      //相手側
      { 
        BTLV_BALL_GAUGE_ANM_NONE,
        BTLV_BALL_GAUGE_ANM_NORMAL_E,
        BTLV_BALL_GAUGE_ANM_DEAD_E,
        BTLV_BALL_GAUGE_ANM_NG_E,
        BTLV_BALL_GAUGE_ANM_ARROW_E,
      },
      //停止アニメ
      { 
        BTLV_BALL_GAUGE_ANM_NONE,
        BTLV_BALL_GAUGE_ANM_NORMAL,
        BTLV_BALL_GAUGE_ANM_DEAD,
        BTLV_BALL_GAUGE_ANM_NG,
        BTLV_BALL_GAUGE_ANM_ARROW_E,
      },
    };
    int stop_pos = pos[ bbgp->type ][ 0 ].x;

    ball_gauge.pos_x = pos[ bbgp->type ][ 0 ].x + ( ( bbgp->type == BTLV_BALL_GAUGE_TYPE_MINE ) ?
                                                      BTLV_BALL_GAUGE_MOVE_OFFSET :
                                                     -BTLV_BALL_GAUGE_MOVE_OFFSET
                                                  );
    ball_gauge.pos_y = pos[ bbgp->type ][ 0 ].y;
    for( i = 0 ; i < BTLV_BALL_GAUGE_CLWK_MAX - 1 ; i++ ) 
    { 
      bbgw->bbgcl[ i ].clwk = GFL_CLACT_WK_Create( bbgw->clunit,
                                                   bbgw->charID, bbgw->plttID, bbgw->cellID,
                                                   &ball_gauge, CLSYS_DEFREND_MAIN, bbgw->heapID );
      GFL_CLACT_WK_GetPos( bbgw->bbgcl[ i ].clwk, &bbgw->bbgcl[ i ].now_pos, CLSYS_DEFREND_MAIN );
      GFL_CLACT_WK_SetAutoAnmFlag( bbgw->bbgcl[ i ].clwk, TRUE );
      GFL_CLACT_WK_SetAnmSeq( bbgw->bbgcl[ i ].clwk, anm_no_table[ bbgp->type ][ bbgp->status[ i ] ] );
      bbgw->bbgcl[ i ].stop_anm_no = anm_no_table[ 2 ][ bbgp->status[ i ] ];
      bbgw->bbgcl[ i ].wait = BTLV_BALL_GAUGE_MOVE_WAIT * ( i + 1 );
      bbgw->bbgcl[ i ].stop_pos = stop_pos;
      bbgw->bbgcl[ i ].move_pos = stop_pos +
                                  ( ( bbgp->type == BTLV_BALL_GAUGE_TYPE_MINE ) ?
                                  -BTLV_BALL_GAUGE_STOP_OFFSET :
                                  BTLV_BALL_GAUGE_STOP_OFFSET )
                                  * ( i + 1 );
      bbgw->bbgcl[ i ].speed = ( bbgp->type == BTLV_BALL_GAUGE_TYPE_MINE ) ?
                                -BTLV_BALL_GAUGE_MOVE_SPEED :
                                 BTLV_BALL_GAUGE_MOVE_SPEED;
      stop_pos += ( bbgp->type == BTLV_BALL_GAUGE_TYPE_MINE ) ?
                  BTLV_BALL_GAUGE_BALL_OFFSET :
                 -BTLV_BALL_GAUGE_BALL_OFFSET;
    }

    ball_gauge.pos_x = pos[ bbgp->type ][ 1 ].x + ( ( bbgp->type == BTLV_BALL_GAUGE_TYPE_MINE ) ?
                                                      BTLV_BALL_GAUGE_MOVE_OFFSET :
                                                     -BTLV_BALL_GAUGE_MOVE_OFFSET
                                                  );
    ball_gauge.pos_y = pos[ bbgp->type ][ 1 ].y;
    bbgw->bbgcl[ i ].clwk = GFL_CLACT_WK_Create( bbgw->clunit,
                                           bbgw->charID, bbgw->plttID, bbgw->cellID,
                                           &ball_gauge, CLSYS_DEFREND_MAIN, bbgw->heapID );
    GFL_CLACT_WK_GetPos( bbgw->bbgcl[ i ].clwk, &bbgw->bbgcl[ i ].now_pos, CLSYS_DEFREND_MAIN );
    GFL_CLACT_WK_SetAutoAnmFlag( bbgw->bbgcl[ i ].clwk, TRUE );
    GFL_CLACT_WK_SetAnmSeq( bbgw->bbgcl[ i ].clwk, anm_no_table[ bbgp->type ][ BTLV_BALL_GAUGE_STATUS_ARROW ] );
    bbgw->bbgcl[ i ].stop_anm_no = anm_no_table[ bbgp->type ][ BTLV_BALL_GAUGE_STATUS_ARROW ];
    bbgw->bbgcl[ i ].wait = 0;
    bbgw->bbgcl[ i ].stop_pos = bbgw->bbgcl[ i ].move_pos = stop_pos = pos[ bbgp->type ][ 1 ].x;
    bbgw->bbgcl[ i ].speed = ( bbgp->type == BTLV_BALL_GAUGE_TYPE_MINE ) ?
                                -BTLV_BALL_GAUGE_ARROW_MOVE_SPEED :
                                 BTLV_BALL_GAUGE_ARROW_MOVE_SPEED;
  }

  GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), TCB_BTLV_BALL_GAUGE_Anime, bbgw, 0 );
  bbgw->tcb_execute = 1;

  GFL_ARC_CloseDataHandle( handle );

  PMSND_PlaySE( SEQ_SE_TB_START );

  return bbgw;
}

//============================================================================================
/**
 *  @brief  システム終了
 *
 *  @param[in]  bbgw  BTLV_BALL_GAUGE_WORK管理構造体へのポインタ
 */
//============================================================================================
void  BTLV_BALL_GAUGE_Delete( BTLV_BALL_GAUGE_WORK* bbgw )
{ 
  int i;

  for( i = 0 ; i < BTLV_BALL_GAUGE_CLWK_MAX ; i++ )
  { 
    GFL_CLACT_WK_Remove( bbgw->bbgcl[ i ].clwk );
  }

  GFL_CLGRP_CGR_Release( bbgw->charID );
  GFL_CLGRP_PLTT_Release( bbgw->plttID );
  GFL_CLGRP_CELLANIM_Release( bbgw->cellID );
  GFL_CLACT_UNIT_Delete( bbgw->clunit );
  GFL_HEAP_FreeMemory( bbgw );
}

//============================================================================================
/**
 *  @brief  ボールゲージエフェクトが起動中かチェック
 *
 *  @param[in]  bbgw  BTLV_BALL_GAUGE_WORK管理構造体へのポインタ
 *
 *  @retval FALSE:終了　TRUE:起動中
 */
//============================================================================================
BOOL  BTLV_BALL_GAUGE_CheckExecute( BTLV_BALL_GAUGE_WORK* bbgw )
{ 
  return ( bbgw->tcb_execute != 0 );
}

//============================================================================================
/**
 *  @brief  ボールゲージアニメ
 *
 *  @param[in]  bbgw  BTLV_BALL_GAUGE_WORK管理構造体へのポインタ
 *
 *  @retval FALSE:終了　TRUE:起動中
 */
//============================================================================================
static  void  TCB_BTLV_BALL_GAUGE_Anime( GFL_TCB* tcb, void* work )
{ 
  BTLV_BALL_GAUGE_WORK* bbgw = ( BTLV_BALL_GAUGE_WORK* )work;
  BOOL flag = FALSE;
  int i;

  switch( bbgw->seq_no ){ 
  case 0:
    for( i = 0 ; i < BTLV_BALL_GAUGE_CLWK_MAX ; i++ )
    { 
      if( bbgw->bbgcl[ i ].wait )
      { 
        flag = TRUE;
        bbgw->bbgcl[ i ].wait--;
        continue;
      }
      if( bbgw->bbgcl[ i ].speed )
      { 
        flag = TRUE;
        bbgw->bbgcl[ i ].now_pos.x += bbgw->bbgcl[ i ].speed;
        if( bbgw->bbgcl[ i ].speed > 0 )
        { 
          if( bbgw->bbgcl[ i ].now_pos.x >= bbgw->bbgcl[ i ].move_pos )
          { 
            bbgw->bbgcl[ i ].now_pos.x = bbgw->bbgcl[ i ].move_pos;
            bbgw->bbgcl[ i ].speed = 0;
          }
        }
        else
        { 
          if( bbgw->bbgcl[ i ].now_pos.x <= bbgw->bbgcl[ i ].move_pos )
          { 
            bbgw->bbgcl[ i ].now_pos.x = bbgw->bbgcl[ i ].move_pos;
            bbgw->bbgcl[ i ].speed = 0;
          }
        }
        GFL_CLACT_WK_SetPos( bbgw->bbgcl[ i ].clwk, &bbgw->bbgcl[ i ].now_pos, CLSYS_DEFREND_MAIN );
        if( bbgw->bbgcl[ i ].speed == 0 )
        { 
          int se_no;
          GFL_CLACT_WK_SetAnmSeq( bbgw->bbgcl[ i ].clwk, bbgw->bbgcl[ i ].stop_anm_no );
          switch( bbgw->bbgcl[ i ].stop_anm_no ){ 
          case BTLV_BALL_GAUGE_ANM_NONE:
            se_no = SEQ_SE_TB_KARA;
            break;
          case BTLV_BALL_GAUGE_ANM_ARROW_M:
          case BTLV_BALL_GAUGE_ANM_ARROW_E:
            se_no = 0;
            break;
          default:
            se_no = SEQ_SE_TB_KON;
            break;
          }
          if( se_no )
          { 
            PMSND_PlaySE_byPlayerID( se_no, bbgw->player_no );
            bbgw->player_no ^= 1;
          }
        }
      }
    }
    if( flag == FALSE )
    { 
      bbgw->seq_no++;
    }
    break;
  case 1:
    for( i = 0 ; i < BTLV_BALL_GAUGE_CLWK_MAX ; i++ )
    { 
      if( bbgw->bbgcl[ i ].now_pos.x < bbgw->bbgcl[ i ].stop_pos )
      { 
        bbgw->bbgcl[ i ].now_pos.x += 2;
        if( bbgw->bbgcl[ i ].now_pos.x >= bbgw->bbgcl[ i ].stop_pos )
        { 
          bbgw->bbgcl[ i ].now_pos.x = bbgw->bbgcl[ i ].stop_pos;
        }
        flag = TRUE;
      }
      else if( bbgw->bbgcl[ i ].now_pos.x > bbgw->bbgcl[ i ].stop_pos )
      { 
        bbgw->bbgcl[ i ].now_pos.x -= 2;
        if( bbgw->bbgcl[ i ].now_pos.x <= bbgw->bbgcl[ i ].stop_pos )
        { 
          bbgw->bbgcl[ i ].now_pos.x = bbgw->bbgcl[ i ].stop_pos;
        }
        flag = TRUE;
      }
      GFL_CLACT_WK_SetPos( bbgw->bbgcl[ i ].clwk, &bbgw->bbgcl[ i ].now_pos, CLSYS_DEFREND_MAIN );
    }
    if( flag == FALSE )
    { 
      bbgw->tcb_execute = 0;
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

