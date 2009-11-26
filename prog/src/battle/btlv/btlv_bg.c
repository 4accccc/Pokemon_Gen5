
//============================================================================================
/**
 * @file  btlv_bg.c
 * @brief 戦闘エフェクトBG制御
 * @author  soga
 * @date  2009.11.06
 */
//============================================================================================

#include <gflib.h>

#include "system/gfl_use.h"

#include "btlv_effect.h"
#include "btlv_bg.h"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================
struct _BTLV_BG_WORK
{
  GFL_TCBSYS*     tcb_sys;
  u32             bg_tcb_move_execute :1;
  u32                                 :31;
  HEAPID          heapID;
};

typedef struct
{
  BTLV_BG_WORK*     bbw;
  VecFx32           now_value;
  EFFTOOL_MOVE_WORK emw;
}BTLV_BG_TCB_WORK;

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  void  BTLV_BG_TCBInitialize( BTLV_BG_WORK *bbw, int type, VecFx32 *start, VecFx32 *end, int frame, int wait, int count, GFL_TCB_FUNC *func );
static  void  TCB_BTLV_BG_Move( GFL_TCB *tcb, void *work );

//============================================================================================
/**
 *  システム初期化
 *
 * @param[in] vramBank  初期化に必要なVRAMBANKマップ
 * @param[in] heapID    ヒープID
 *
 * @retval  bclw  BTLV_CLACT_WORK管理構造体へのポインタ
 */
//============================================================================================
BTLV_BG_WORK*  BTLV_BG_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID )
{
  BTLV_BG_WORK *bbw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_BG_WORK ) );

  bbw->tcb_sys = tcb_sys;
  bbw->heapID  = heapID;

  return bbw;
}

//============================================================================================
/**
 *  システム終了
 *
 * @param[in] bclw  BTLV_CLACT_WORK管理構造体へのポインタ
 */
//============================================================================================
void  BTLV_BG_Exit( BTLV_BG_WORK *bbw )
{
  GFL_HEAP_FreeMemory( bbw );
}

//============================================================================================
/**
 *  座標移動
 *
 * @param[in] bbw       BTLV_BG_WORK管理ワークへのポインタ
 * @param[in] position  攻撃側の立ち位置（スクロールの向きを決めるのに使用）
 * @param[in] type      移動タイプ
 * @param[in] scr_x     移動タイプにより意味が変化
 * @param[in] scr_y     移動タイプにより意味が変化
 *                      EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  移動先
 *                      EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame     移動フレーム数（目的地まで何フレームで到達するか）
 * @param[in] wait      移動ウエイト
 * @param[in] count     往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_BG_MovePosition( BTLV_BG_WORK *bbw, int position, int type, int scr_x, int scr_y, int frame, int wait, int count )
{
  VecFx32       start_fx32;
  VecFx32       pos_fx32;

  pos_fx32.x = scr_x << FX32_SHIFT;
  pos_fx32.y = scr_y << FX32_SHIFT;
  pos_fx32.z = 0;

  start_fx32.x = GFL_BG_GetScrollX( GFL_BG_FRAME3_M ) << FX32_SHIFT;
  start_fx32.y = GFL_BG_GetScrollY( GFL_BG_FRAME3_M ) << FX32_SHIFT;
  start_fx32.z = 0;

  //常にスクロールさせるときはポケモンの立ち位置で方向を決定する
  if( type == BTLEFF_BG_SCROLL_EVERY )
  { 
    //ポケモンの視線の方向で＋－を決定する
    if( position & 1 )
    {
      pos_fx32.x *= -1;
    }
  }

  //移動の補間は相対指定とする
  if( type == EFFTOOL_CALCTYPE_INTERPOLATION )
  { 
    pos_fx32.x += start_fx32.x;
    pos_fx32.y += start_fx32.y;
    pos_fx32.z += start_fx32.z;
  }

  BTLV_BG_TCBInitialize( bbw, type, &start_fx32, &pos_fx32, frame, wait, count, TCB_BTLV_BG_Move );
  bbw->bg_tcb_move_execute = 1;
}

//============================================================================================
/**
 *  タスクが起動中かチェック
 *
 * @param[in] bbw    BTLV_CLACT_WORK管理ワークへのポインタ
 * @param[in] index   CLWKの管理インデックス
 *
 * @retval: TRUE:起動中　FALSE:終了
 */
//============================================================================================
BOOL  BTLV_BG_CheckTCBExecute( BTLV_BG_WORK *bbw )
{
  return ( bbw->bg_tcb_move_execute != 0 );
}

//============================================================================================
/**
 *  BG操作系タスク初期化処理
 */
//============================================================================================
static  void  BTLV_BG_TCBInitialize( BTLV_BG_WORK *bbw, int type, VecFx32 *start, VecFx32 *end, int frame, int wait, int count, GFL_TCB_FUNC *func )
{
  BTLV_BG_TCB_WORK *bbtw = GFL_HEAP_AllocMemory( bbw->heapID, sizeof( BTLV_BG_TCB_WORK ) );

  bbtw->bbw               = bbw;
  bbtw->now_value.x       = start->x;
  bbtw->now_value.y       = start->y;
  bbtw->now_value.z       = start->z;
  bbtw->emw.move_type     = type;
  bbtw->emw.vec_time      = frame;
  bbtw->emw.vec_time_tmp  = frame;
  bbtw->emw.wait          = 0;
  bbtw->emw.wait_tmp      = wait;
  bbtw->emw.count         = count * 2;
  bbtw->emw.start_value.x = start->x;
  bbtw->emw.start_value.y = start->y;
  bbtw->emw.start_value.z = start->z;
  bbtw->emw.end_value.x   = end->x;
  bbtw->emw.end_value.y   = end->y;
  bbtw->emw.end_value.z   = end->z;

  switch( type ){
  case EFFTOOL_CALCTYPE_DIRECT:         //直接ポジションに移動
    break;
  case EFFTOOL_CALCTYPE_INTERPOLATION:  //移動先までを補間しながら移動
    BTLV_EFFTOOL_CalcMoveVector( &bbtw->emw.start_value, end, &bbtw->emw.vector, FX32_CONST( frame ) );
    break;
  case EFFTOOL_CALCTYPE_ROUNDTRIP_LONG: //指定した区間を往復移動
    bbtw->emw.vec_time_tmp  *= 2;
    bbtw->emw.count         *= 2;
    //fall through
  case EFFTOOL_CALCTYPE_ROUNDTRIP:      //指定した区間を往復移動
    bbtw->emw.vector.x = FX_Div( end->x, FX32_CONST( frame ) );
    bbtw->emw.vector.y = FX_Div( end->y, FX32_CONST( frame ) );
    bbtw->emw.vector.z = FX_Div( end->z, FX32_CONST( frame ) );
    break;
  }
  GFL_TCB_AddTask( bbw->tcb_sys, func, bbtw, 0 );
}

//============================================================================================
/**
 *  BG移動タスク
 */
//============================================================================================
static  void  TCB_BTLV_BG_Move( GFL_TCB *tcb, void *work )
{
  BTLV_BG_TCB_WORK *bbtw = ( BTLV_BG_TCB_WORK * )work;
  BTLV_BG_WORK *bbw = bbtw->bbw;
  int   scr_x, scr_y;
  BOOL  ret;

  if( bbtw->emw.move_type == BTLEFF_BG_SCROLL_EVERY )
  { 
    bbtw->now_value.x += bbtw->emw.end_value.x;
    bbtw->now_value.y += bbtw->emw.end_value.y;
    if( bbtw->emw.vec_time == 0 )
    { 
      ret = TRUE;
    }
    else
    { 
      bbtw->emw.vec_time--;
      ret = FALSE;
    }
  }
  else
  { 
    ret = BTLV_EFFTOOL_CalcParam( &bbtw->emw, &bbtw->now_value );
  }
  scr_x = bbtw->now_value.x >> FX32_SHIFT;
  scr_y = bbtw->now_value.y >> FX32_SHIFT;
  GFL_BG_SetScroll( GFL_BG_FRAME3_M, GFL_BG_SCROLL_X_SET, scr_x );
  GFL_BG_SetScroll( GFL_BG_FRAME3_M, GFL_BG_SCROLL_Y_SET, scr_y );
  if( ret == TRUE )
  {
    bbw->bg_tcb_move_execute = 0;
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

