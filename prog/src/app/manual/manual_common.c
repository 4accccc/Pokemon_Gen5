//============================================================================
/**
 *  @file   manual_common.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL_COMMON
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
#include "manual_touchbar.h"
#include "manual_common.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_manual.h"
#include "manual.naix"

// ダミー
#include "msg/msg_zkn.h"
#include "zukan_gra.naix"


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


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
// 初期化処理
MANUAL_COMMON_WORK*  MANUAL_COMMON_Init(
    GAMEDATA*  gamedata,
    HEAPID     heap_id
)
{
  // ワーク
  MANUAL_COMMON_WORK*  work  = GFL_HEAP_AllocClearMemory( heap_id, sizeof(MANUAL_COMMON_WORK) );

  // MANUALを呼んだ場所からの借り物
  {
    work->gamedata = gamedata;
  }

  // MANUALで生成したもの
  {
    work->heap_id = heap_id;
  }

  // グラフィック、フォントなど
  {
    work->graphic       = MANUAL_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    
    work->print_que_title  = PRINTSYS_QUE_Create( work->heap_id );
    work->print_que_main   = PRINTSYS_QUE_Create( work->heap_id );
    work->print_que_tb     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // メッセージ
  {
    work->msgdata_system = GFL_MSG_Create(
                               GFL_MSG_LOAD_NORMAL,
                               ARCID_MESSAGE,
                               NARC_message_manual_dat,
                               work->heap_id );
    work->msgdata_main   = GFL_MSG_Create(
                               GFL_MSG_LOAD_NORMAL,
                               ARCID_MESSAGE,
                               NARC_message_zkn_dat,
                               work->heap_id );
  }

  // ファイルハンドル
  {
    work->handle_system  = GFL_ARC_OpenDataHandle( ARCID_MANUAL, work->heap_id );
    work->handle_explain = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, work->heap_id );
  }

  // パレット
  {
    // サブBGパレット
    // テキスト
    GFL_ARC_UTIL_TransVramPaletteEx(
        ARCID_FONT,
        NARC_font_default_nclr,
        PALTYPE_SUB_BG,
        0,
        BG_PAL_POS_S_TEXT * 0x20,
        BG_PAL_NUM_S_TEXT * 0x20,
        work->heap_id );
  }

  // 呼び出した関数の中でMANUAL_COMMON_WORKを使用するものは、MANUAL_COMMON_Initの1番最後に呼び出すこと。
  // マニュアルタッチバー
  {
    work->mtb_wk = MANUAL_TOUCHBAR_Init( work );
  }

  return  work;
}

// 終了処理
void  MANUAL_COMMON_Exit(
    MANUAL_COMMON_WORK*  work
)
{
  // マニュアルタッチバー
  {
    MANUAL_TOUCHBAR_Exit( work->mtb_wk );
  }

  // ファイルハンドル
  {
    GFL_ARC_CloseDataHandle( work->handle_system );
    GFL_ARC_CloseDataHandle( work->handle_explain );
  }

  // メッセージ
  {
    GFL_MSG_Delete( work->msgdata_system );
    GFL_MSG_Delete( work->msgdata_main );
  }

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Clear( work->print_que_title );
    PRINTSYS_QUE_Delete( work->print_que_title );
    PRINTSYS_QUE_Clear( work->print_que_main );
    PRINTSYS_QUE_Delete( work->print_que_main );
    PRINTSYS_QUE_Clear( work->print_que_tb );
    PRINTSYS_QUE_Delete( work->print_que_tb );
    
    GFL_FONT_Delete( work->font );
    MANUAL_GRAPHIC_Exit( work->graphic );
  }

  // ワーク
  GFL_HEAP_FreeMemory( work );
}

// 主処理
void  MANUAL_COMMON_Main(
    MANUAL_COMMON_WORK*  work
)
{
  // print_que
  PRINTSYS_QUE_Main( work->print_que_title );
  PRINTSYS_QUE_Main( work->print_que_main );
  PRINTSYS_QUE_Main( work->print_que_tb );

  // 2D描画
  MANUAL_GRAPHIC_2D_Draw( work->graphic );

  // 3D描画
  //MANUAL_GRAPHIC_3D_StartDraw( work->graphic );
  //MANUAL_GRAPHIC_3D_EndDraw( work->graphic );
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

