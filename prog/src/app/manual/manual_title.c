//============================================================================
/**
 *  @file   manual_title.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL_TITLE
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_data.h"
#include "print/gf_font.h"
#include "print/printsys.h"

#include "manual_graphic.h"
#include "manual_def.h"
#include "manual_common.h"
#include "manual_list.h"
#include "manual_title.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_manual.h"
#include "manual.naix"
#include "manual_image.naix"


// ダミー
#include "msg/msg_zkn.h"


// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
struct _MANUAL_TITLE_WORK
{
  // パラメータ
  MANUAL_TITLE_PARAM*         param;
  // 共通
  MANUAL_COMMON_WORK*         cmn_wk;

  // ここで作成
  MANUAL_LIST_PARAM           list_param;
  MANUAL_LIST_WORK*           list_wk;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Manual_Title_VBlankFunc( GFL_TCB* tcb, void* wk );


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
// 初期化処理
MANUAL_TITLE_WORK*  MANUAL_TITLE_Init(
    MANUAL_TITLE_PARAM*  param,
    MANUAL_COMMON_WORK*  cmn_wk
)
{
  // ワーク
  MANUAL_TITLE_WORK*  work  = GFL_HEAP_AllocClearMemory( cmn_wk->heap_id, sizeof(MANUAL_TITLE_WORK) );

  // パラメータ
  work->param  = param;
  {
    // out
    work->param->result    = MANUAL_TITLE_RESULT_RETURN;
    work->param->serial_no = 0;
  }
  // 共通
  work->cmn_wk = cmn_wk;

  // ここで作成
  {
    if( work->param->type == MANUAL_TITLE_TYPE_CATEGORY )
    {
      u16 i;
      work->list_param.type = MANUAL_LIST_TYPE_TITLE;
      work->list_param.num  = 7 + work->param->cate_no;
      work->list_param.item = GFL_HEAP_AllocClearMemory( work->cmn_wk->heap_id, sizeof(MANUAL_LIST_ITEM)*work->list_param.num );
      for( i=0; i<work->list_param.num; i++ )
      {
        work->list_param.item[i].no     = i;
        work->list_param.item[i].str_id = i;
        work->list_param.item[i].icon   = (i%3==0)?(MANUAL_LIST_ICON_NEW):(MANUAL_LIST_ICON_NONE);
      }
      work->list_param.head_pos    = work->param->head_pos;
      work->list_param.cursor_pos  = work->param->cursor_pos;
    }
    else  // if( work->param->type == MANUAL_TITLE_TYPE_ALL )
    {
      u16 i;
      work->list_param.type = MANUAL_LIST_TYPE_TITLE;
      work->list_param.num  = 16;
      work->list_param.item = GFL_HEAP_AllocClearMemory( work->cmn_wk->heap_id, sizeof(MANUAL_LIST_ITEM)*work->list_param.num );
      for( i=0; i<work->list_param.num; i++ )
      {
        work->list_param.item[i].no     = i;
        work->list_param.item[i].str_id = i;
        work->list_param.item[i].icon   = (i%3==0)?(MANUAL_LIST_ICON_NONE):(MANUAL_LIST_ICON_NEW);
      }
      work->list_param.head_pos    = work->param->head_pos;
      work->list_param.cursor_pos  = work->param->cursor_pos;
    }

    work->list_wk = MANUAL_LIST_Init( &work->list_param, cmn_wk );
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_Title_VBlankFunc, work, 1 );

  return work;
}

// 終了処理
void  MANUAL_TITLE_Exit(
    MANUAL_TITLE_WORK*     work
)
{
  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // パラメータ
  {
    // out
    work->param->head_pos   = work->list_param.head_pos;
    work->param->cursor_pos = work->list_param.cursor_pos;
    
    if( work->list_param.result == MANUAL_LIST_RESULT_RETURN )
    {
      work->param->result    = MANUAL_TITLE_RESULT_RETURN;
    }
    else if( work->list_param.result == MANUAL_LIST_RESULT_ITEM )
    {
      work->param->result    = MANUAL_TITLE_RESULT_ITEM;
      work->param->serial_no = work->list_param.item[ work->param->cursor_pos ].no;
    }
  }

  // ここで作成
  {
    MANUAL_LIST_Exit( work->list_wk );
    GFL_HEAP_FreeMemory( work->list_param.item );
  }

  // ワーク
  GFL_HEAP_FreeMemory( work );
}

// 主処理
BOOL  MANUAL_TITLE_Main(
    MANUAL_TITLE_WORK*     work
)
{
  return MANUAL_LIST_Main( work->list_wk );
}


//------------------------------------------------------------------
/**
 *  @brief           
 *
 *  @param[in]       
 *  @param[in,out]       
 *
 *  @retval          
 */
//------------------------------------------------------------------

//-------------------------------------
/// 
//=====================================


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Manual_Title_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  MANUAL_TITLE_WORK* work = (MANUAL_TITLE_WORK*)wk;
}

