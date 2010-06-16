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
#include "manual_data.h"
#include "manual_touchbar.h"
#include "manual_common.h"

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


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// メインBGの設定
static void Manual_Common_MainBgInit( MANUAL_COMMON_WORK* work );
static void Manual_Common_MainBgExit( MANUAL_COMMON_WORK* work );


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
                               NARC_message_manual_text_dat,
                               work->heap_id );
  }

  // ファイルハンドル
  {
    work->handle_system  = GFL_ARC_OpenDataHandle( ARCID_MANUAL, work->heap_id );
    work->handle_explain = GFL_ARC_OpenDataHandle( ARCID_MANUAL_IMAGE, work->heap_id );
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

  // マニュアルデータ
  {
    work->data_wk = MANUAL_DATA_Load( work->handle_system, work->heap_id ); 
  }

  // 上画面にダイレクトカラー画像を表示する
  {
    // メインBGの設定
    Manual_Common_MainBgInit( work );

    // 上画面に表示しているダイレクトカラー画像のID
    work->bg_m_dcbmp_id = MANUAL_BG_M_DCBMP_NO_IMAGE;  // 最初に表示する画像のIDと異なればどんな値でもよい

    // 上画面にダイレクトカラー画像を表示する
    MANUAL_COMMON_DrawBgMDcbmp( work, MANUAL_DATA_ImageIdGetNoImage( work->data_wk ) );
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

  // 上画面にダイレクトカラー画像を表示する
  {
    // 上画面にダイレクトカラー画像を表示する
    MANUAL_COMMON_DrawBgMDcbmp( work, MANUAL_BG_M_DCBMP_NO_IMAGE );

    // メインBGの設定
    Manual_Common_MainBgExit( work );
  }

  // マニュアルデータ
  {
    MANUAL_DATA_Unload( work->data_wk ); 
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

// 上画面にダイレクトカラー画像を表示する
void  MANUAL_COMMON_DrawBgMDcbmp(
    MANUAL_COMMON_WORK*  work,
    u16                  bg_m_dcbmp_id
)
{
  if( work->bg_m_dcbmp_id != bg_m_dcbmp_id )  // 変更する必要があるとき

  {
    u32  image_size = 256*192*2;
    u32  size;
    u16* buf;
    
    work->bg_m_dcbmp_id = bg_m_dcbmp_id;

    if( work->bg_m_dcbmp_id == MANUAL_BG_M_DCBMP_NO_IMAGE )
    {
      buf = GFL_HEAP_AllocClearMemory( work->heap_id, image_size );
      size = image_size;
    }
    else
    {
      buf = GFL_ARCHDL_UTIL_LoadEx( work->handle_explain, work->bg_m_dcbmp_id, TRUE, work->heap_id, &size );
    }

    DC_FlushRange( buf, size );
    GX_LoadBG2Bmp( buf, 0, image_size );
    GFL_HEAP_FreeMemory( buf );
  }
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
/// メインBGの設定
//=====================================
static void Manual_Common_MainBgInit( MANUAL_COMMON_WORK* work )
{
  {
    G2_SetBG2ControlDCBmp( GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000 );
    G2_SetBG2Priority( BG_FRAME_PRI_M_PIC );
    G2_BG0Mosaic( FALSE );
  }

  GFL_BG_SetVisible( BG_FRAME_M_PIC, VISIBLE_ON );
}
static void Manual_Common_MainBgExit( MANUAL_COMMON_WORK* work )
{
  // 何もしない
}

