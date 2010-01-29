
//============================================================================================
/**
 * @file  btlv_finger_cursor.c
 * @brief 指カーソル
 * @author  soga
 * @date  2010.01.28
 */
//============================================================================================
#include <gflib.h>

#include "btlv_finger_cursor.h"

#include "system/palanm.h"
#include "btlv_effect.h"

#include "arc_def.h"
#include "battle/battgra_wb.naix"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================

enum
{ 
  BTLV_FINGER_CURSOR_SRC_PALNO      = 0,
  BTLV_FINGER_CURSOR_PAL_TRANS_SIZE = 1,

  BTLV_FINGER_CURSOR_MAX = 1,
};

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

typedef struct
{ 
  int         seq_no;
  int         count;
  int         frame;
  int         wait;
  BOOL        anm_end_flag;
}BTLV_FINGER_CURSOR_PARAM;

struct _BTLV_FINGER_CURSOR_WORK
{
  GFL_CLUNIT* clunit;
  GFL_CLWK*   clwk;
  GFL_TCB*    tcb;
  u32         charID;
  u32         plttID;
  u32         cellID;

  BTLV_FINGER_CURSOR_PARAM  bfcp;

  HEAPID      heapID;
};

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  void  BTLV_FINGER_CURSOR_CallBackFunc( u32 param, fx32 currentFrame );
static  void  TCB_BTLV_FINGER_CURSOR_WatchAnmEndFlag( GFL_TCB* tcb, void* work );

//============================================================================================
/**
 *  @brief  システム初期化
 *
 *  @param  pal     指カーソルパレットの読み込み先
 *  @param  heapID  ヒープID
 *
 *  @retval システム管理構造体のポインタ
 */
//============================================================================================
BTLV_FINGER_CURSOR_WORK*  BTLV_FINGER_CURSOR_Init( int pal, HEAPID heapID )
{ 
  BTLV_FINGER_CURSOR_WORK* bfcw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_FINGER_CURSOR_WORK ) );
  ARCHANDLE*  handle  = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, heapID );

  bfcw->heapID  = heapID;

  //リソース読み込み
  bfcw->charID = GFL_CLGRP_CGR_Register( handle, NARC_battgra_wb_finger_cursor_NCGR, FALSE, CLSYS_DRAW_SUB, heapID );
  bfcw->cellID = GFL_CLGRP_CELLANIM_Register( handle, NARC_battgra_wb_finger_cursor_NCER,
                                              NARC_battgra_wb_finger_cursor_NANR, heapID );
  bfcw->plttID = GFL_CLGRP_PLTT_RegisterEx( handle, NARC_battgra_wb_finger_cursor_NCLR, CLSYS_DRAW_SUB, pal * 0x20,
                                            BTLV_FINGER_CURSOR_SRC_PALNO,
                                            BTLV_FINGER_CURSOR_PAL_TRANS_SIZE,
                                            heapID );
  PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_SUB_OBJ,
                           GFL_CLGRP_PLTT_GetAddr( bfcw->plttID, CLSYS_DRAW_SUB ) / 2, 0x20 );

  //CLACT初期化
  bfcw->clunit = GFL_CLACT_UNIT_Create( BTLV_FINGER_CURSOR_MAX, 0, heapID );

  GFL_ARC_CloseDataHandle( handle );

  //アニメフラグ監視TCBセット
  GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), TCB_BTLV_FINGER_CURSOR_WatchAnmEndFlag, bfcw, 0 );

  return bfcw;
}

//============================================================================================
/**
 *  @brief  システム終了
 *
 *  @retval システム管理構造体のポインタ
 */
//============================================================================================
void  BTLV_FINGER_CURSOR_Exit( BTLV_FINGER_CURSOR_WORK* bfcw )
{ 
  BTLV_FINGER_CURSOR_Delete( bfcw );

  GFL_CLGRP_CGR_Release( bfcw->charID );
  GFL_CLGRP_CELLANIM_Release( bfcw->cellID );
  GFL_CLGRP_PLTT_Release( bfcw->plttID );

  GFL_CLACT_UNIT_Delete( bfcw->clunit );
  GFL_HEAP_FreeMemory( bfcw );
}

//============================================================================================
/**
 *  @brief  システムメインループ
 *
 *  @param[in]  bfcw システム管理構造体のポインタ
 */
//============================================================================================
void  BTLV_FINGER_CURSOR_Main( BTLV_FINGER_CURSOR_WORK* bfcw )
{ 

}

//============================================================================================
/**
 *  @brief  指カーソル生成
 *
 *  @param[in]  bfcw    システム管理構造体のポインタ
 *  @param[in]  pos_x   指カーソルX座標
 *  @param[in]  pos_y   指カーソルY座標
 *  @param[in]  count   押すアニメに移行するまでのループ回数
 *  @param[in]  frame   押すアニメ中、押した動作に移行するアニメーションフレーム
 *  @param[in]  wait    押すアニメのあと消えるまでのウエイト
 *
 * @retval  TRUE:Create成功 FALSE:Create失敗（まだアニメが終わっていない）
 */
//============================================================================================
BOOL  BTLV_FINGER_CURSOR_Create( BTLV_FINGER_CURSOR_WORK* bfcw, int pos_x, int pos_y, int count, int frame, int wait )
{ 
  GFL_CLWK_AFFINEDATA cursor = {  
    { 
      0, 0,                   //x, y
      0, 0, 0,                //アニメ番号、優先順位、BGプライオリティ
    },
	  0, 0,		                  // アフィンｘ座標 アフィンｙ座標
	  FX32_ONE, FX32_ONE,	      // 拡大ｘ値 拡大ｙ値
	  0,                        // 回転角度(0～0xffff 0xffffが360度)
	  CLSYS_AFFINETYPE_DOUBLE,  // 上書きアフィンタイプ（CLSYS_AFFINETYPE）
  };

  if( bfcw->bfcp.anm_end_flag )
  { 
    return FALSE;
  }

  if( bfcw->clwk )
  { 
    BTLV_FINGER_CURSOR_Delete( bfcw );
    bfcw->clwk = NULL;
  }

  cursor.clwkdata.pos_x = pos_x;
  cursor.clwkdata.pos_y = pos_y;

  bfcw->clwk = GFL_CLACT_WK_CreateAffine( bfcw->clunit, bfcw->charID, bfcw->plttID, bfcw->cellID,
                                          &cursor, CLSYS_DEFREND_SUB, bfcw->heapID );

  GFL_CLACT_WK_SetAnmSeq( bfcw->clwk, 0 );
  GFL_CLACT_WK_SetAnmFrame( bfcw->clwk, 0 );
  GFL_CLACT_WK_SetAutoAnmFlag( bfcw->clwk, TRUE );
  GFL_CLACT_WK_SetDrawEnable( bfcw->clwk, TRUE );

  if( count )
  { 
    bfcw->bfcp.count  = count;
  }
  else
  { 
    bfcw->bfcp.count  = 1;  //最低でも1回はループするようにする
  }

  if( wait )
  { 
    bfcw->bfcp.wait  = wait;
  }
  else
  { 
    bfcw->bfcp.wait  = 1;  //最低でも1は待つ
  }

  bfcw->bfcp.frame  = frame;

  bfcw->bfcp.seq_no = 0;
  bfcw->bfcp.anm_end_flag = FALSE;

  { 
	  GFL_CLWK_ANM_CALLBACK	dat;
	  dat.param = ( u32 )bfcw;
	  dat.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM;
	  dat.frame_idx     = 0;
	  dat.p_func        = BTLV_FINGER_CURSOR_CallBackFunc;
  
	  GFL_CLACT_WK_StartAnmCallBack( bfcw->clwk, &dat );
  }

  return TRUE;
}

//============================================================================================
/**
 *  @brief  指カーソル削除
 *
 *  @param[in]  bfcw システム管理構造体のポインタ
 */
//============================================================================================
void  BTLV_FINGER_CURSOR_Delete( BTLV_FINGER_CURSOR_WORK* bfcw )
{ 
  if( bfcw->clwk )
  { 
    GFL_CLACT_WK_Remove( bfcw->clwk );
    bfcw->clwk = NULL;
  }
}

//============================================================================================
/**
 *  @brief  指カーソルアニメチェック
 *
 *  @param[in]  bfcw システム管理構造体のポインタ
 */
//============================================================================================
BOOL  BTLV_FINGER_CURSOR_CheckExecute( BTLV_FINGER_CURSOR_WORK* bfcw )
{ 
  return bfcw->bfcp.anm_end_flag;
}

//============================================================================================
/**
 *  @brief  指カーソルコールバック
 *
 *  @param[in]  bfcw システム管理構造体のポインタ
 */
//============================================================================================
static  void  BTLV_FINGER_CURSOR_CallBackFunc( u32 param, fx32 currentFrame )
{ 
	BTLV_FINGER_CURSOR_WORK* bfcw = ( BTLV_FINGER_CURSOR_WORK* )param;

  switch( bfcw->bfcp.seq_no ){ 
  case 0:
    if( --bfcw->bfcp.count == 0 )
    { 
	    GFL_CLACT_WK_SetAnmSeq( bfcw->clwk, 1 );
      bfcw->bfcp.seq_no++;
      { 
	      GFL_CLWK_ANM_CALLBACK	dat;
	      dat.param = param;
	      dat.callback_type = CLWK_ANM_CALLBACK_TYPE_SPEC_FRM;
	      dat.frame_idx     = bfcw->bfcp.frame;
	      dat.p_func        = BTLV_FINGER_CURSOR_CallBackFunc;
  
	      GFL_CLACT_WK_StartAnmCallBack( bfcw->clwk, &dat );
      }
    }
    break;
  case 1:
    bfcw->bfcp.anm_end_flag = TRUE;
    bfcw->bfcp.seq_no++;
    { 
      GFL_CLWK_ANM_CALLBACK	dat;
      dat.param = param;
      dat.callback_type = CLWK_ANM_CALLBACK_TYPE_EVER_FRM;
      dat.frame_idx     = 0;
      dat.p_func        = BTLV_FINGER_CURSOR_CallBackFunc;
  
      GFL_CLACT_WK_StartAnmCallBack( bfcw->clwk, &dat );
    }
    break;
  case 2:
    GFL_CLACT_WK_SetDrawEnable( bfcw->clwk, FALSE );
    bfcw->bfcp.anm_end_flag = FALSE;
    bfcw->bfcp.seq_no++;
    break;
  case 3:
  default:
    break;
  }
}

//============================================================================================
/**
 *  @brief  anm_end_flag監視TCB
 */
//============================================================================================
static  void  TCB_BTLV_FINGER_CURSOR_WatchAnmEndFlag( GFL_TCB* tcb, void* work )
{ 
  BTLV_FINGER_CURSOR_WORK* bfcw = ( BTLV_FINGER_CURSOR_WORK* )work;

  if( bfcw->bfcp.anm_end_flag )
  { 
    if( --bfcw->bfcp.wait == 0 )
    { 
      GFL_CLACT_WK_SetDrawEnable( bfcw->clwk, FALSE );
      bfcw->bfcp.anm_end_flag = FALSE;
    }
  }
}

