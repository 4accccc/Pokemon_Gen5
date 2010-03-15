//============================================================================
/**
 *  @file   subway_map.c
 *  @brief  地下鉄路線図
 *  @author Koji Kawada
 *  @data   2010.03.15
 *  @note   
 *  モジュール名：SUBWAY_MAP
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "app/app_nogear_subscreen.h"

#include "subway_map_graphic.h"
#include "app/subway_map.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "script_message.naix"
#include "subway_map_gra.naix"

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x30000)               ///< ヒープサイズ

// メインBGフレーム
#define BG_FRAME_M_REAR        (GFL_BG_FRAME1_M)
#define BG_FRAME_M_FRONT       (GFL_BG_FRAME2_M)

// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_REAR    (1)
#define BG_FRAME_PRI_M_FRONT   (0)

// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_GRA             = 0,  // 全て転送  // REAR, FRONT共通パレット
};
// 位置
enum
{
  BG_PAL_POS_M_GRA            =  0,  // REAR, FRONT共通パレット
};

// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_        = 0,
};
// 位置
enum
{
  OBJ_PAL_POS_M_        = 0,
};

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
  SUBWAY_MAP_PARAM*   param;
  
  // グラフィック、フォントなど
  SUBWAY_MAP_GRAPHIC_WORK*      graphic;
  GFL_FONT*                             font;
  PRINT_QUE*                            print_que;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;
}
SUBWAY_MAP_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Subway_Map_VBlankFunc( GFL_TCB* tcb, void* wk );

// BG
static void Subway_Map_BgInit( SUBWAY_MAP_WORK* work );
static void Subway_Map_BgExit( SUBWAY_MAP_WORK* work );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Subway_Map_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Subway_Map_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Subway_Map_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    SUBWAY_MAP_ProcData =
{
  Subway_Map_ProcInit,
  Subway_Map_ProcMain,
  Subway_Map_ProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           パラメータ生成
 *
 *  @param[in]       heap_id       ヒープID
 *  @param[in]       mystatus      自分状態データ(性別を使用)
 *
 *  @retval          SUBWAY_MAP_PARAM
 */
//------------------------------------------------------------------
SUBWAY_MAP_PARAM*  SUBWAY_MAP_AllocParam(
                                HEAPID               heap_id,
                                const MYSTATUS*      mystatus
                           )
{
  SUBWAY_MAP_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof( SUBWAY_MAP_PARAM ) );
  SUBWAY_MAP_InitParam(
      param,
      mystatus );
  return param;
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータ解放
 *
 *  @param[in,out]   param      パラメータ生成で生成したもの
 *
 *  @retval          
 */
//------------------------------------------------------------------
void            SUBWAY_MAP_FreeParam(
                            SUBWAY_MAP_PARAM*  param )
{
  GFL_HEAP_FreeMemory( param );
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータを設定する
 *
 *  @param[in,out]   param         SUBWAY_MAP_PARAM
 *  @param[in]       mystatus      自分状態データ(性別を使用)
 *
 *  @retval          
 */
//------------------------------------------------------------------
void  SUBWAY_MAP_InitParam(
                  SUBWAY_MAP_PARAM*      param,
                  const MYSTATUS*                mystatus
                         )
{
  param->mystatus          = mystatus;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT Subway_Map_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  SUBWAY_MAP_WORK*     work;

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SUBWAY_MAP, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(SUBWAY_MAP_WORK), HEAPID_SUBWAY_MAP );
    GFL_STD_MemClear( work, sizeof(SUBWAY_MAP_WORK) );
    
    work->heap_id       = HEAPID_SUBWAY_MAP;
    work->param         = (SUBWAY_MAP_PARAM*)pwk;
  }

  // グラフィック、フォントなど
  {
    work->graphic       = SUBWAY_MAP_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // メインBG
  GFL_BG_SetPriority( BG_FRAME_M_REAR,   BG_FRAME_PRI_M_REAR  );
  GFL_BG_SetPriority( BG_FRAME_M_FRONT,  BG_FRAME_PRI_M_FRONT );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Subway_Map_VBlankFunc, work, 1 );

  // 生成
  Subway_Map_BgInit( work );

  // サブBG
  APP_NOGEAR_SUBSCREEN_Init();
  APP_NOGEAR_SUBSCREEN_Trans( work->heap_id, work->param->mystatus->sex );  // PM_MALE or PM_FEMALE  // include/pm_version.h

  // フェードイン(黒→見える)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Subway_Map_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  SUBWAY_MAP_WORK*     work     = (SUBWAY_MAP_WORK*)mywk;

  // サブBG
  APP_NOGEAR_SUBSCREEN_Exit();

  // 破棄
  Subway_Map_BgExit( work );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    SUBWAY_MAP_GRAPHIC_Exit( work->graphic );
  }

  // ヒープ、パラメータなど
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_SUBWAY_MAP );
  }

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT Subway_Map_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  SUBWAY_MAP_WORK*     work     = (SUBWAY_MAP_WORK*)mywk;

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
      u32 x, y;
      if( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) )
      {
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        b_end = TRUE;
      }
      else if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
        b_end = TRUE;
      }

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

  PRINTSYS_QUE_Main( work->print_que );

  // 2D描画
  SUBWAY_MAP_GRAPHIC_2D_Draw( work->graphic );

  // 3D描画
  //SUBWAY_MAP_GRAPHIC_3D_StartDraw( work->graphic );
  //SUBWAY_MAP_GRAPHIC_3D_EndDraw( work->graphic );

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
static void Subway_Map_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  SUBWAY_MAP_WORK* work = (SUBWAY_MAP_WORK*)wk;
}

//-------------------------------------
/// BG
//=====================================
static void Subway_Map_BgInit( SUBWAY_MAP_WORK* work )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_SHOUJOU, work->heap_id );

  // 共通
  GFL_ARCHDL_UTIL_TransVramPalette(
      handle,
      NARC_subway_map_gra_subwaymap_NCLR,
      PALTYPE_MAIN_BG,
      BG_PAL_POS_M_GRA * 0x20,
      BG_PAL_NUM_M_GRA * 0x20,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
      handle,
      NARC_subway_map_gra_subwaymap_NCGR,
      BG_FRAME_M_REAR,  // キャラの位置はBG_FRAME_M_FRONTも同じにしてある
			0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  // REAR
  GFL_ARCHDL_UTIL_TransVramScreen(
      handle,
      NARC_subway_map_gra_subwaymap_base_NSCR,
      BG_FRAME_M_REAR,
      0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  // FRONT
  GFL_ARCHDL_UTIL_TransVramScreen(
      handle,
      NARC_subway_map_gra_subwaymap_root_NSCR,
      BG_FRAME_M_FRONT,
      0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenReq( BG_FRAME_M_REAR );
  GFL_BG_LoadScreenReq( BG_FRAME_M_FRONT );
}
static void Subway_Map_BgExit( SUBWAY_MAP_WORK* work )
{
  // 何もしない
}

