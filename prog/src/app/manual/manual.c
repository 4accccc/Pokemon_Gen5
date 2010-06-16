//============================================================================
/**
 *  @file   manual.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL
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
#include "manual_top.h"
#include "manual_category.h"
#include "manual_title.h"
#include "manual_explain.h"
#include "app/manual.h"

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
#define HEAP_SIZE              (0x50000)               ///< ヒープサイズ

// ProcMainのシーケンス
enum
{
  SEQ_START          = 0,
  SEQ_FADE_IN,
  SEQ_MAIN,
  SEQ_FADE_OUT,
  SEQ_END,
};

// フェード
#define FADE_IN_WAIT  (0)
#define FADE_OUT_WAIT (0)

// 表示画面
typedef enum
{
  DISP_TYPE_TOP,
  DISP_TYPE_CATEGORY,
  DISP_TYPE_TITLE,
  DISP_TYPE_EXPLAIN,
}
DISP_TYPE;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // ヒープ、パラメータなど
  HEAPID                      heap_id;
  MANUAL_PARAM*               param;
 
  // 共通
  MANUAL_COMMON_WORK*         cmn_wk;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // 表示画面
  DISP_TYPE                   disp_type;

  // トップ画面
  MANUAL_TOP_PARAM            top_param;
  MANUAL_TOP_WORK*            top_wk;
  // カテゴリ選択画面
  MANUAL_CATEGORY_PARAM       category_param;
  MANUAL_CATEGORY_WORK*       category_wk;
  // タイトル選択画面
  MANUAL_TITLE_PARAM          title_param;
  MANUAL_TITLE_WORK*          title_wk;
  // 説明画面
  MANUAL_EXPLAIN_PARAM        explain_param;
  MANUAL_EXPLAIN_WORK*        explain_wk;
}
MANUAL_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Manual_VBlankFunc( GFL_TCB* tcb, void* wk );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Manual_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Manual_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Manual_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA  MANUAL_ProcData =
{
  Manual_ProcInit,
  Manual_ProcMain,
  Manual_ProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
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


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT Manual_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MANUAL_WORK*     work;

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MANUAL, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(MANUAL_WORK), HEAPID_MANUAL );
    GFL_STD_MemClear( work, sizeof(MANUAL_WORK) );
    
    work->heap_id       = HEAPID_MANUAL;
    work->param         = (MANUAL_PARAM*)pwk;

    work->param->result = MANUAL_RESULT_RETURN;
  }

  // 共通
  work->cmn_wk = MANUAL_COMMON_Init( work->param->gamedata, work->heap_id );

  // メインBG
  //GFL_BG_SetPriority( BG_FRAME_M_PIC,     BG_FRAME_PRI_M_PIC );
  // サブBG
  GFL_BG_SetPriority( BG_FRAME_S_REAR,    BG_FRAME_PRI_S_REAR );
  GFL_BG_SetPriority( BG_FRAME_S_MAIN,    BG_FRAME_PRI_S_MAIN );
  GFL_BG_SetPriority( BG_FRAME_S_TB_BAR,  BG_FRAME_PRI_S_TB_BAR );
  GFL_BG_SetPriority( BG_FRAME_S_TB_TEXT, BG_FRAME_PRI_S_TB_TEXT );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_VBlankFunc, work, 1 );
  
  // フェードイン(黒→見える)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );

  // 通信アイコン
  GFL_NET_ReloadIconTopOrBottom( FALSE, work->heap_id );

  // 各画面のパラメータ初期化
  {
    // トップ画面
    // in
    work->top_param.cursor_pos = 0;

    // カテゴリ選択画面
    // in
    work->category_param.head_pos   = 0;
    work->category_param.cursor_pos = 0;

    // タイトル選択画面
    // in
    work->title_param.type        = MANUAL_TITLE_TYPE_CATEGORY;
    work->title_param.cate_no     = 0;
    work->title_param.head_pos    = 0;
    work->title_param.cursor_pos  = 0;

    // 説明画面
    // in
    work->explain_param.page_num       = 1;
    work->explain_param.title_str_id   = 0;
    work->explain_param.page[0].image  = MANUAL_BG_M_DCBMP_NO_IMAGE;
    work->explain_param.page[0].str_id = 0;
  }
  
  // 表示画面
  work->disp_type = DISP_TYPE_TOP;
  
  // トップ画面
  work->top_wk = MANUAL_TOP_Init( &work->top_param, work->cmn_wk );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Manual_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MANUAL_WORK* work = (MANUAL_WORK*)mywk;

  switch( work->disp_type )
  {
  case DISP_TYPE_TOP:
    {
      // トップ画面
      MANUAL_TOP_Exit( work->top_wk );
    }
    break;
  case DISP_TYPE_CATEGORY:
    {
      // カテゴリ選択画面
      MANUAL_CATEGORY_Exit( work->category_wk );
    }
    break;
  case DISP_TYPE_TITLE:
    {
      // タイトル選択画面
      MANUAL_TITLE_Exit( work->title_wk );
    }
    break;
  case DISP_TYPE_EXPLAIN:
    {
      // 説明画面
      MANUAL_EXPLAIN_Exit( work->explain_wk );
    }
    break;
  }

  // 通信アイコン
  // 終了するときは通信アイコンに対して何もしない

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // 共通
  MANUAL_COMMON_Exit( work->cmn_wk );

  // ヒープ、パラメータなど
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_MANUAL );
  }

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT Manual_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MANUAL_WORK* work = (MANUAL_WORK*)mywk;

  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_FADE_IN;
    }
    break;
  case SEQ_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      BOOL b_end = FALSE;
      BOOL b_ret = FALSE;

      switch( work->disp_type )
      {
      case DISP_TYPE_TOP:
        {
          // トップ画面
          b_ret = MANUAL_TOP_Main( work->top_wk );
          if( b_ret )
          {
            if(    work->top_param.result == MANUAL_TOP_RESULT_CLOSE
                || work->top_param.result == MANUAL_TOP_RESULT_RETURN )
            {
              if( work->top_param.result == MANUAL_TOP_RESULT_CLOSE )        work->param->result = MANUAL_RESULT_CLOSE;
              else if( work->top_param.result == MANUAL_TOP_RESULT_RETURN )  work->param->result = MANUAL_RESULT_RETURN;
              b_end = TRUE;
            }
            else
            {
              // トップ画面
              MANUAL_TOP_Exit( work->top_wk );

              if( work->top_param.result == MANUAL_TOP_RESULT_CATEGORY )
              {
                // カテゴリ選択画面
                work->disp_type = DISP_TYPE_CATEGORY;
                work->category_param.head_pos   = 0;
                work->category_param.cursor_pos = 0;
                work->category_wk = MANUAL_CATEGORY_Init( &work->category_param, work->cmn_wk );
              }
              else if( work->top_param.result == MANUAL_TOP_RESULT_ALL )
              {
                // タイトル選択画面
                work->disp_type = DISP_TYPE_TITLE;
                work->title_param.type        = MANUAL_TITLE_TYPE_ALL;
                work->title_param.cate_no     = 0;
                work->title_param.head_pos    = 0;
                work->title_param.cursor_pos  = 0;
                work->title_wk = MANUAL_TITLE_Init( &work->title_param, work->cmn_wk );
              }
            }
          }
        }
        break;
      case DISP_TYPE_CATEGORY:
        {
          // タイトル選択画面
          b_ret = MANUAL_CATEGORY_Main( work->category_wk );
          if( b_ret )
          {
            // タイトル選択画面
            MANUAL_CATEGORY_Exit( work->category_wk );

            if( work->category_param.result == MANUAL_CATEGORY_RESULT_RETURN )
            {
              // トップ画面
              work->disp_type = DISP_TYPE_TOP;
              // カーソル位置はそのままで
              work->top_wk = MANUAL_TOP_Init( &work->top_param, work->cmn_wk );
            }
            else
            {
              // タイトル選択画面
              work->disp_type = DISP_TYPE_TITLE;
              work->title_param.type        = MANUAL_TITLE_TYPE_CATEGORY;
              work->title_param.cate_no     = work->category_param.cate_no;
              work->title_param.head_pos    = 0;
              work->title_param.cursor_pos  = 0;
              work->title_wk = MANUAL_TITLE_Init( &work->title_param, work->cmn_wk );
            }
          }
        }
        break;
      case DISP_TYPE_TITLE:
        {
          // タイトル選択画面
          b_ret = MANUAL_TITLE_Main( work->title_wk );
          if( b_ret )
          {
            // タイトル選択画面
            MANUAL_TITLE_Exit( work->title_wk );

            if( work->title_param.result == MANUAL_TITLE_RESULT_RETURN )
            {
              if( work->title_param.type == MANUAL_TITLE_TYPE_CATEGORY )
              {
                // カテゴリ選択画面
                work->disp_type = DISP_TYPE_CATEGORY;
                // カーソル位置はそのままで
                work->category_wk = MANUAL_CATEGORY_Init( &work->category_param, work->cmn_wk );
              }
              else if( work->title_param.type == MANUAL_TITLE_TYPE_ALL )
              {
                // トップ画面
                work->disp_type = DISP_TYPE_TOP;
                // カーソル位置はそのままで
                work->top_wk = MANUAL_TOP_Init( &work->top_param, work->cmn_wk );
              }
            }
            else
            {
              // 説明画面
              work->disp_type = DISP_TYPE_EXPLAIN;
              {
                u16 title_idx = work->title_param.serial_no;  // resultがMANUAL_TITLE_RESULT_ITEMのときのみ使用する  // serial_noと言っているが実際はtitle_idxである。
                u16 page_num = MANUAL_DATA_TitleGetPageNum( work->cmn_wk->data_wk, title_idx );

                // 既読にする
                {
                  u16 read_flag = MANUAL_DATA_TitleGetReadFlag( work->cmn_wk->data_wk, title_idx );
                  if( !MANUAL_DATA_ReadFlagIsRead( work->cmn_wk->data_wk, read_flag, work->cmn_wk->gamedata ) )
                  {
                    MANUAL_DATA_ReadFlagSetRead( work->cmn_wk->data_wk, read_flag, work->cmn_wk->gamedata );
                  }
                }

                work->explain_param.title_str_id   = MANUAL_DATA_TitleGetTitleGmmId( work->cmn_wk->data_wk, title_idx );
                work->explain_param.page_num       = MANUAL_DATA_TitleGetPageNum( work->cmn_wk->data_wk, title_idx );

                {
                  u16 page_order;
                  for( page_order=0; page_order<page_num; page_order++ )
                  {
                    u16 image_id = MANUAL_DATA_TitleGetPageImageId( work->cmn_wk->data_wk, title_idx, page_order );
                    if( !MANUAL_DATA_ImageIdIsValid( work->cmn_wk->data_wk, image_id ) )
                    {
                      image_id = MANUAL_BG_M_DCBMP_NO_IMAGE;
                    }
                    work->explain_param.page[page_order].image  = image_id;
                    
                    work->explain_param.page[page_order].str_id = MANUAL_DATA_TitleGetPageGmmId( work->cmn_wk->data_wk, title_idx, page_order );
                  }
                }
              }
              work->explain_wk = MANUAL_EXPLAIN_Init( &work->explain_param, work->cmn_wk );
            }
          }
        }
        break;
      case DISP_TYPE_EXPLAIN:
        {
          // 説明画面
          b_ret = MANUAL_EXPLAIN_Main( work->explain_wk );
          if( b_ret )
          {
            // 説明画面
            MANUAL_EXPLAIN_Exit( work->explain_wk );

            if( work->explain_param.result == MANUAL_EXPLAIN_RESULT_RETURN )
            {
              // タイトル選択画面
              work->disp_type = DISP_TYPE_TITLE;
              // カテゴリか全表示かはそのままで
              // カーソル位置はそのままで
              work->title_wk = MANUAL_TITLE_Init( &work->title_param, work->cmn_wk );
            }
          }
        }
        break;
      }
     
      // 終了
      if( b_end )
      {
        *seq = SEQ_FADE_OUT;
        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        *seq = SEQ_END;
      }
    }
    break;
  case SEQ_END:
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  // 共通
  MANUAL_COMMON_Main( work->cmn_wk );

  return GFL_PROC_RES_CONTINUE;
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Manual_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  MANUAL_WORK* work = (MANUAL_WORK*)wk;
}

