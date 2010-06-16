//============================================================================
/**
 *  @file   manual_category.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL_CATEGORY
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
#include "manual_data.h"
#include "manual_common.h"
#include "manual_list.h"
#include "manual_category.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_manual.h"


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
struct _MANUAL_CATEGORY_WORK
{
  // パラメータ
  MANUAL_CATEGORY_PARAM*      param;
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
static void Manual_Category_VBlankFunc( GFL_TCB* tcb, void* wk );


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
// 初期化処理
MANUAL_CATEGORY_WORK*  MANUAL_CATEGORY_Init(
    MANUAL_CATEGORY_PARAM*  param,
    MANUAL_COMMON_WORK*     cmn_wk
)
{
  // ワーク
  MANUAL_CATEGORY_WORK*  work  = GFL_HEAP_AllocClearMemory( cmn_wk->heap_id, sizeof(MANUAL_CATEGORY_WORK) );

  // パラメータ
  work->param  = param;
  {
    // out
    work->param->result   = MANUAL_CATEGORY_RESULT_RETURN;
    work->param->cate_no  = 0;
  }
  // 共通
  work->cmn_wk = cmn_wk;

  // ここで作成
  {
    work->list_param.type = MANUAL_LIST_TYPE_CATEGORY;

    {
      u16 cate_idx;
      u16 cate_num = MANUAL_DATA_CateRefGetTotalCateNum( work->cmn_wk->data_wk );
      
      work->list_param.num  = 0;
      work->list_param.item = GFL_HEAP_AllocClearMemory( work->cmn_wk->heap_id, sizeof(MANUAL_LIST_ITEM)*cate_num );  // 最大も使わないことが多いだろうが、最大で確保しておく
      
      for( cate_idx=0; cate_idx<cate_num; cate_idx++ )
      {
        u16 open_title_num = 0;
        u16 title_num = MANUAL_DATA_CateGetTitleNum( work->cmn_wk->data_wk, cate_idx );
        u16 title_order; 
        for( title_order=0; title_order<title_num; title_order++ )
        {
          u16 title_idx = MANUAL_DATA_CateGetTitleIdx( work->cmn_wk->data_wk, cate_idx, title_order );
          u16 open_flag = MANUAL_DATA_TitleGetOpenFlag( work->cmn_wk->data_wk, title_idx );
          if( MANUAL_DATA_OpenFlagIsOpen( work->cmn_wk->data_wk, open_flag, work->cmn_wk->gamedata ) )
          {
            open_title_num++;
          }
        }
        if( open_title_num > 0 )
        {
          work->list_param.item[work->list_param.num].no     = cate_idx;
          work->list_param.item[work->list_param.num].str_id = MANUAL_DATA_CateGetCateGmmId( work->cmn_wk->data_wk, cate_idx );
          work->list_param.item[work->list_param.num].icon   = MANUAL_LIST_ICON_NONE;
          work->list_param.num++;
        }
      }
    }

    work->list_param.head_pos    = work->param->head_pos;
    work->list_param.cursor_pos  = work->param->cursor_pos;

    work->list_wk = MANUAL_LIST_Init( &work->list_param, cmn_wk );
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_Category_VBlankFunc, work, 1 );

  return work;
}

// 終了処理
void  MANUAL_CATEGORY_Exit(
    MANUAL_CATEGORY_WORK*     work
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
      work->param->result    = MANUAL_CATEGORY_RESULT_RETURN;
    }
    else if( work->list_param.result == MANUAL_LIST_RESULT_ITEM )
    {
      work->param->result   = MANUAL_CATEGORY_RESULT_TITLE;
      work->param->cate_no  = work->list_param.item[ work->param->cursor_pos ].no;
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
BOOL  MANUAL_CATEGORY_Main(
    MANUAL_CATEGORY_WORK*     work
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
static void Manual_Category_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  MANUAL_CATEGORY_WORK* work = (MANUAL_CATEGORY_WORK*)wk;
}

