//============================================================================
/**
 *  @file   zukan_toroku.c
 *  @brief  図鑑登録
 *  @author Koji Kawada
 *  @data   2009.12.03
 *  @note   ui_template.cを元に作成しました。
 *
 *  モジュール名：ZUKAN_TOROKU
 */
//============================================================================
// 必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "print/gf_font.h"
#include "font/font.naix"

#include "print/printsys.h"

#include "../info/zukan_info.h"
#include "zukan_nickname.h"

#include "zukan_toroku_graphic.h"
#include "app/zukan_toroku.h"

// アーカイブ
#include "arc_def.h"
#include "zukan_gra.naix"

// オーバーレイ
FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define ZUKAN_TOROKU_HEAP_SIZE (0x30000)  ///< ヒープサイズ

#define ZUKAN_TOROKU_FADE_IN_WAIT  (2)
#define ZUKAN_TOROKU_FADE_OUT_WAIT (2)
#define ZUKAN_TOROKU_INFO_BG_PRIORITY (0)  ///< 図鑑情報画面の最前面のプライオリティ(0か1) 

typedef enum
{
  ZUKAN_TOROKU_STATE_TOROKU,
  ZUKAN_TOROKU_STATE_NICKNAME,
}
ZUKAN_TOROKU_STATE;

typedef enum
{
  ZUKAN_TOROKU_TOROKU_STEP_FADE_IN,
  ZUKAN_TOROKU_TOROKU_STEP_WAIT_KEY,
  ZUKAN_TOROKU_TOROKU_STEP_MOVE,
  ZUKAN_TOROKU_TOROKU_STEP_FADE_OUT_BG,
  ZUKAN_TOROKU_TOROKU_STEP_FADE_IN_BG,
  ZUKAN_TOROKU_TOROKU_STEP_CENTER_STILL,
}
ZUKAN_TOROKU_TOROKU_STEP;

typedef enum
{
  ZUKAN_TOROKU_NICKNAME_STEP_FADE_IN,
  ZUKAN_TOROKU_NICKNAME_STEP_START,
  ZUKAN_TOROKU_NICKNAME_STEP_SELECT,
  ZUKAN_TOROKU_NICKNAME_STEP_FADE_OUT,
  ZUKAN_TOROKU_NICKNAME_STEP_END,
}
ZUKAN_TOROKU_NICKNAME_STEP;

#define ZUKAN_TOROKU_VBLANK_TCB_PRI (1)

//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROCワーク
//=====================================
typedef struct
{
  HEAPID heap_id;
  ZUKAN_TOROKU_GRAPHIC_WORK* graphic;
  GFL_FONT* font;
  PRINT_QUE* print_que;

  ZUKAN_TOROKU_STATE state;
  s32 step;
  s32 brightness;
  s32 brightness_wait;

  ZUKAN_INFO_WORK* zukan_info_work;
  ZUKAN_NICKNAME_WORK* zukan_nickname_work;
  
  GFL_TCB* vblank_tcb;  ///< VBlank中のTCB
  BOOL bg_visible_off;
  
  GFL_ARCUTIL_TRANSINFO bg_chara_info;  ///< BGキャラ領域
}
ZUKAN_TOROKU_WORK;

//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
static void Zukan_Toroku_CreateBG( ZUKAN_TOROKU_WORK* work );
static void Zukan_Toroku_DeleteBG( ZUKAN_TOROKU_WORK* work );
static void Zukan_Toroku_VBlankFunc( GFL_TCB* tcb, void* wk );

//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Zukan_Toroku_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Zukan_Toroku_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Zukan_Toroku_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA ZUKAN_TOROKU_ProcData =
{
  Zukan_Toroku_ProcInit,
  Zukan_Toroku_ProcMain,
  Zukan_Toroku_ProcExit,
};

//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
ZUKAN_TOROKU_PARAM* ZUKAN_TOROKU_AllocParam( HEAPID              heap_id,
                                             ZUKAN_TOROKU_LAUNCH launch,
                                             POKEMON_PARAM*      pp )
{
  ZUKAN_TOROKU_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof(ZUKAN_TOROKU_PARAM) );
  GFL_STD_MemClear( param, sizeof(ZUKAN_TOROKU_PARAM) );

  ZUKAN_TOROKU_SetParam( param,
                         launch,
                         pp );
  
  return param;
}

void ZUKAN_TOROKU_FreeParam( ZUKAN_TOROKU_PARAM* param )
{
  GFL_HEAP_FreeMemory( param );
}

//-------------------------------------
/// PROCに渡す引数の設定
//=====================================
void ZUKAN_TOROKU_SetParam( ZUKAN_TOROKU_PARAM* param,
                            ZUKAN_TOROKU_LAUNCH launch,
                            POKEMON_PARAM*      pp )
{
  param->launch = launch;
  param->pp     = pp;
}

//-------------------------------------
/// 結果を得る
//=====================================
ZUKAN_TOROKU_RESULT ZUKAN_TOROKU_GetResult( ZUKAN_TOROKU_PARAM* param )
{
  return param->result; 
}

//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
static GFL_PROC_RESULT Zukan_Toroku_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  ZUKAN_TOROKU_PARAM* param = (ZUKAN_TOROKU_PARAM*)pwk;
  ZUKAN_TOROKU_WORK* work;

  // オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_TOROKU, ZUKAN_TOROKU_HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(ZUKAN_TOROKU_WORK), HEAPID_ZUKAN_TOROKU );
    GFL_STD_MemClear( work, sizeof(ZUKAN_TOROKU_WORK) );
  }

  work->heap_id = HEAPID_ZUKAN_TOROKU;

  work->graphic = ZUKAN_TOROKU_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );

  work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );

  work->print_que = PRINTSYS_QUE_Create( work->heap_id );

  {
    if(param->launch==ZUKAN_TOROKU_LAUNCH_TOROKU)
    {
      work->state = ZUKAN_TOROKU_STATE_TOROKU;
      work->step = ZUKAN_TOROKU_TOROKU_STEP_FADE_IN;
    }
    else
    {
      work->state = ZUKAN_TOROKU_STATE_NICKNAME;
      work->step = ZUKAN_TOROKU_NICKNAME_STEP_FADE_IN;
    }
  }

  work->zukan_info_work = ZUKAN_INFO_Init( work->heap_id, param->pp,
                                           (param->launch==ZUKAN_TOROKU_LAUNCH_TOROKU)?(ZUKAN_INFO_LAUNCH_TOROKU):(ZUKAN_INFO_LAUNCH_NICKNAME),
                                           ZUKAN_INFO_DISP_M, 0,
                                           ZUKAN_TOROKU_GRAPHIC_GetClunit(work->graphic),
                                           work->font,
                                           work->print_que );

  work->zukan_nickname_work = ZUKAN_NICKNAME_Init( work->heap_id, param->pp,
                                                   ZUKAN_TOROKU_GRAPHIC_GetClunit(work->graphic), work->font, work->print_que );
 
  if( work->state == ZUKAN_TOROKU_STATE_NICKNAME )
  {
    Zukan_Toroku_CreateBG( work );
  }

  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Toroku_VBlankFunc, work, ZUKAN_TOROKU_VBLANK_TCB_PRI );
 
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, ZUKAN_TOROKU_FADE_IN_WAIT );  // 真っ白の値or真っ黒の値, 0<画面が見えている値>, 輝度が1変化するのに掛かるフレーム );

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT Zukan_Toroku_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  ZUKAN_TOROKU_PARAM* param = (ZUKAN_TOROKU_PARAM*)pwk;
  ZUKAN_TOROKU_WORK* work = (ZUKAN_TOROKU_WORK*)mywk;

  GFL_TCB_DeleteTask( work->vblank_tcb );
  
  Zukan_Toroku_DeleteBG( work );
  
  ZUKAN_NICKNAME_Exit( work->zukan_nickname_work );
  ZUKAN_INFO_Exit( work->zukan_info_work );
  
  PRINTSYS_QUE_Delete( work->print_que );

  GFL_FONT_Delete( work->font );

  ZUKAN_TOROKU_GRAPHIC_Exit( work->graphic );

  {
    HEAPID heap_id = work->heap_id;
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( heap_id );
  }
  
  // オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

  return GFL_PROC_RES_FINISH;
} 

static GFL_PROC_RESULT Zukan_Toroku_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  ZUKAN_TOROKU_PARAM* param = (ZUKAN_TOROKU_PARAM*)pwk;
  ZUKAN_TOROKU_WORK* work = (ZUKAN_TOROKU_WORK*)mywk;

  ZUKAN_NICKNAME_RESULT zukan_nickname_result;
  GFL_PROC_RESULT proc_res = GFL_PROC_RES_CONTINUE;

  ZUKAN_INFO_Main( work->zukan_info_work );
  zukan_nickname_result = ZUKAN_NICKNAME_Main( work->zukan_nickname_work );
 
  switch(work->state)
  {
  case ZUKAN_TOROKU_STATE_TOROKU:
    {
      switch(work->step)
      {
      case ZUKAN_TOROKU_TOROKU_STEP_FADE_IN:
        {
          if( !GFL_FADE_CheckFade() )
          {
            ZUKAN_INFO_Start( work->zukan_info_work );
            work->step = ZUKAN_TOROKU_TOROKU_STEP_WAIT_KEY;
          }
        }
        break;
      case ZUKAN_TOROKU_TOROKU_STEP_WAIT_KEY:
        {
          BOOL next = FALSE;
          BOOL tp_trg = GFL_UI_TP_GetTrg();
          int trg = GFL_UI_KEY_GetTrg();
          if( trg & PAD_BUTTON_A )
          {
            next = TRUE;
          }
          if( tp_trg )
          {
            next = TRUE;
          }

          if( next )
          {
            ZUKAN_INFO_Move( work->zukan_info_work );
            work->step = ZUKAN_TOROKU_TOROKU_STEP_MOVE;
          }
        }
        break;
      case ZUKAN_TOROKU_TOROKU_STEP_MOVE:
        {
          if( ZUKAN_INFO_IsMove(work->zukan_info_work) || ZUKAN_INFO_IsCenterStill(work->zukan_info_work) )  // IsCenterStillは念のため
          {
            work->brightness_wait = 0;
            work->brightness = 16;
            work->step = ZUKAN_TOROKU_TOROKU_STEP_FADE_OUT_BG;
          }
        }
        break;
      case ZUKAN_TOROKU_TOROKU_STEP_FADE_OUT_BG:
        {
          work->brightness_wait++;
          if( work->brightness_wait == 4 )
          {
            work->brightness_wait = 0;
            work->brightness--;
          }
          G2_SetBlendBrightnessExt( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3,
                                    GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3,
                                    work->brightness,
                                    0,
                                    work->brightness - 16 );
          if( work->brightness == 0 )
          {
            ZUKAN_INFO_DeleteOthers( work->zukan_info_work );
            Zukan_Toroku_CreateBG( work );
            work->step = ZUKAN_TOROKU_TOROKU_STEP_FADE_IN_BG;
          }
        }
        break;
      case ZUKAN_TOROKU_TOROKU_STEP_FADE_IN_BG:
        {
          work->brightness_wait++;
          if( work->brightness_wait == 4 )
          {
            work->brightness_wait = 0;
            work->brightness++;
          }
          G2_SetBlendBrightnessExt( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3,
                                    GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3,
                                    work->brightness,
                                    0,
                                    work->brightness - 16 );
          if( work->brightness == 16 )
          {
            work->step = ZUKAN_TOROKU_TOROKU_STEP_CENTER_STILL;
          }
        }
        break;
      case ZUKAN_TOROKU_TOROKU_STEP_CENTER_STILL:
        {
          if( ZUKAN_INFO_IsCenterStill(work->zukan_info_work) )
          {
            work->state = ZUKAN_TOROKU_STATE_NICKNAME;
            work->step = ZUKAN_TOROKU_NICKNAME_STEP_START;
          }
        }
        break;
      }
    }
    break;
  case ZUKAN_TOROKU_STATE_NICKNAME:
    {
      switch(work->step)
      {
      case ZUKAN_TOROKU_NICKNAME_STEP_FADE_IN:
        {
          if( !GFL_FADE_CheckFade() )
          {
            work->step = ZUKAN_TOROKU_NICKNAME_STEP_START;
          }
        }
        break;
      case ZUKAN_TOROKU_NICKNAME_STEP_START:
        {
          ZUKAN_NICKNAME_Start( work->zukan_nickname_work );
          work->step = ZUKAN_TOROKU_NICKNAME_STEP_SELECT;
        }
        break;
      case ZUKAN_TOROKU_NICKNAME_STEP_SELECT:
        {
          if( zukan_nickname_result == ZUKAN_NICKNAME_RES_FINISH )
          {
            GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, ZUKAN_TOROKU_FADE_OUT_WAIT );
            work->step = ZUKAN_TOROKU_NICKNAME_STEP_FADE_OUT;
          }
        }
        break;
      case ZUKAN_TOROKU_NICKNAME_STEP_FADE_OUT:
        {
          if( !GFL_FADE_CheckFade() )
          {
            ZUKAN_NICKNAME_SELECT zukan_nickname_select = ZUKAN_NICKNAME_GetSelect( work->zukan_nickname_work );
            switch( zukan_nickname_select )
            {
            case ZUKAN_NICKNAME_SELECT_NO:
              param->result = ZUKAN_TOROKU_RESULT_END;
              break;
            case ZUKAN_NICKNAME_SELECT_YES:
              param->result = ZUKAN_TOROKU_RESULT_NICKNAME;
              break;
            }
            work->step = ZUKAN_TOROKU_NICKNAME_STEP_END;
            proc_res = GFL_PROC_RES_FINISH;
          }
        }
        break;
      }
    }
    break;
  }

  PRINTSYS_QUE_Main( work->print_que );

  // 2D描画
  ZUKAN_TOROKU_GRAPHIC_2D_Draw( work->graphic );
  // 3D描画
  ZUKAN_TOROKU_GRAPHIC_3D_StartDraw( work->graphic );
  ZUKAN_TOROKU_GRAPHIC_3D_EndDraw( work->graphic );

  return proc_res;
}



static void Zukan_Toroku_CreateBG( ZUKAN_TOROKU_WORK* work )
{
  {
    //GFL_BG_SetVisible( GFL_BG_FRAME1_M, FALSE );  // ここでOFFにすると一瞬　黒が晴れて、見えなくしておいたBGやOBJが見えてしまうので、
    //GFL_BG_SetVisible( GFL_BG_FRAME2_M, FALSE );  // VBlank中にOFFにすることにした。
    work->bg_visible_off = TRUE;

    GFL_BG_ClearScreen( GFL_BG_FRAME3_M );  // GFL_BG_ClearFrameだと、これから描画しようと思っていたデータまで消されてしまうので。

    GFL_BG_SetPriority( GFL_BG_FRAME0_M, 3 );  // 最背面
    GFL_BG_SetPriority( GFL_BG_FRAME3_M, 0 );  // 最前面
  }

  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, work->heap_id );
    GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_zukan_gra_info_name_bgu_NCLR, PALTYPE_MAIN_BG,
                                      0*0x20, 1*0x20,  // その後、本数が増えたらここを修正
                                      work->heap_id );
    work->bg_chara_info = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( handle, NARC_zukan_gra_info_name_bgu_NCGR, GFL_BG_FRAME0_M,
                                                                       32*1*GFL_BG_1CHRDATASIZ, FALSE, work->heap_id );  // その後、必要なキャラ領域が増えたらここを修正
    GF_ASSERT_MSG( work->bg_chara_info != GFL_ARCUTIL_TRANSINFO_FAIL, "ZUKAN_TOROKU : BGキャラ領域が足りませんでした。\n" );
    GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_zukan_gra_info_name_bgu_NSCR, GFL_BG_FRAME0_M,
                                     GFL_ARCUTIL_TRANSINFO_GetPos( work->bg_chara_info ), 32*24*GFL_BG_1SCRDATASIZ, FALSE, work->heap_id );
    GFL_ARC_CloseDataHandle( handle );
  }

  GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
}

static void Zukan_Toroku_DeleteBG( ZUKAN_TOROKU_WORK* work )
{
  // 読み込んだリソースの破棄
  {
    GFL_BG_FreeCharacterArea( GFL_BG_FRAME0_M, GFL_ARCUTIL_TRANSINFO_GetPos( work->bg_chara_info ),
                              GFL_ARCUTIL_TRANSINFO_GetSize( work->bg_chara_info ) );
  }
}

static void Zukan_Toroku_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_TOROKU_WORK* work = (ZUKAN_TOROKU_WORK*)wk;

  if( work->bg_visible_off )
  {
    GFL_BG_SetVisible( GFL_BG_FRAME1_M, FALSE );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M, FALSE );
    work->bg_visible_off = FALSE;
  }
}
