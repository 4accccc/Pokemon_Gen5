//============================================================================
/**
 *  @file   zukan_detail_headbar.c
 *  @brief  図鑑詳細画面共通のタイトルバー
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_HEADBAR
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/bmp_winframe.h"
#include "print/gf_font.h"
#include "print/printsys.h"

#include "zukan_detail_def.h"
#include "zukan_detail_headbar.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_zkn.h"

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// スクロール
#define SCROLL_Y_DISAPPEAR   (24)
#define SCROLL_Y_APPEAR      (0)
#define SCROLL_VALUE         (1)  // 移動量
#define SCROLL_WAIT          (0)  // 0=毎フレーム移動; 1=移動した後に1フレーム待つ; 2=移動した後に2フレーム待つ;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
struct _ZUKAN_DETAIL_HEADBAR_WORK
{
  // 他のところから借用するもの
  HEAPID                      heap_id;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // ここで作成するもの
  // 状態
  ZUKAN_DETAIL_HEADBAR_STATE  state;
  ZUKAN_DETAIL_HEADBAR_TYPE   type;
  u8                          scroll_wait_count;

  // タイトルバー
  GFL_BMPWIN*                 dummy_bmpwin;
  GFL_BMPWIN*                 head_bmpwin;
  GFL_MSGDATA*                head_msgdata;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Headbar_VBlankFunc( GFL_TCB* tcb, void* wk );

// タイトルバー
static void Zukan_Detail_Headbar_CreateBase( ZUKAN_DETAIL_HEADBAR_WORK* work );
static void Zukan_Detail_Headbar_DeleteBase( ZUKAN_DETAIL_HEADBAR_WORK* work );
static void Zukan_Detail_Headbar_Create( ZUKAN_DETAIL_HEADBAR_WORK* work );
static void Zukan_Detail_Headbar_Delete( ZUKAN_DETAIL_HEADBAR_WORK* work );


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
ZUKAN_DETAIL_HEADBAR_WORK* ZUKAN_DETAIL_HEADBAR_Init(
                               HEAPID      heap_id,
                               GFL_FONT*   font,
                               PRINT_QUE*  print_que )
{
  ZUKAN_DETAIL_HEADBAR_WORK*   work;

  // ワーク生成
  work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(ZUKAN_DETAIL_HEADBAR_WORK) );

  // 初期化
  work->heap_id     = heap_id;
  work->font        = font;
  work->print_que   = print_que;

  // 状態
  work->state = ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR;
  work->type = ZUKAN_DETAIL_HEADBAR_TYPE_NONE;

  // 初期スクロール位置
  GFL_BG_SetScroll( ZKNDTL_BG_FRAME_S_HEADBAR, GFL_BG_SCROLL_Y_SET, SCROLL_Y_DISAPPEAR );

  // プライオリティ
  GFL_BG_SetPriority( ZKNDTL_BG_FRAME_S_HEADBAR, ZKNDTL_BG_FRAME_PRI_S_HEADBAR );

  // タイトルバー
  Zukan_Detail_Headbar_CreateBase( work );
  Zukan_Detail_Headbar_Create( work );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Headbar_VBlankFunc, work, 1 );

  return work;
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZUKAN_DETAIL_HEADBAR_Exit( ZUKAN_DETAIL_HEADBAR_WORK* work )
{
  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // タイトルバー
  Zukan_Detail_Headbar_Delete( work );
  Zukan_Detail_Headbar_DeleteBase( work );

  // ワーク破棄
  GFL_HEAP_FreeMemory( work );
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZUKAN_DETAIL_HEADBAR_Main( ZUKAN_DETAIL_HEADBAR_WORK* work )
{
  switch( work->state )
  {
  case ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR:
    {
    }
    break;
  case ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR_TO_APPEAR:
    {
      if( work->scroll_wait_count == 0 )
      {
        if( GFL_BG_GetScrollY( ZKNDTL_BG_FRAME_S_HEADBAR ) <= SCROLL_Y_APPEAR )
        {
          GFL_BG_SetScrollReq( ZKNDTL_BG_FRAME_S_HEADBAR, GFL_BG_SCROLL_Y_SET, SCROLL_Y_APPEAR );
          work->state = ZUKAN_DETAIL_HEADBAR_STATE_APPEAR;
        }
        else
        {
          GFL_BG_SetScrollReq( ZKNDTL_BG_FRAME_S_HEADBAR, GFL_BG_SCROLL_Y_DEC, SCROLL_VALUE );
          work->scroll_wait_count = SCROLL_WAIT;
        }
      }
      else
      {
        work->scroll_wait_count--;
      }
    }
    break;
  case ZUKAN_DETAIL_HEADBAR_STATE_APPEAR:
    {
    }
    break;
  case ZUKAN_DETAIL_HEADBAR_STATE_APPEAR_TO_DISAPPEAR:
    {
      if( work->scroll_wait_count == 0 )
      {
        if( GFL_BG_GetScrollY( ZKNDTL_BG_FRAME_S_HEADBAR ) >= SCROLL_Y_DISAPPEAR )
        {
          GFL_BG_SetScrollReq( ZKNDTL_BG_FRAME_S_HEADBAR, GFL_BG_SCROLL_Y_SET, SCROLL_Y_DISAPPEAR );
          work->state = ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR;
        }
        else
        {
          GFL_BG_SetScrollReq( ZKNDTL_BG_FRAME_S_HEADBAR, GFL_BG_SCROLL_Y_INC, SCROLL_VALUE );
          work->scroll_wait_count = SCROLL_WAIT;
        }
      }
      else
      {
        work->scroll_wait_count--;
      }
    }
    break;
  }
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZUKAN_DETAIL_HEADBAR_SetType(
                   ZUKAN_DETAIL_HEADBAR_WORK* work,
                   ZUKAN_DETAIL_HEADBAR_TYPE type )
{
  work->type = type;
  Zukan_Detail_Headbar_Delete( work );
  Zukan_Detail_Headbar_Create( work );
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
ZUKAN_DETAIL_HEADBAR_STATE ZUKAN_DETAIL_HEADBAR_GetState( ZUKAN_DETAIL_HEADBAR_WORK* work )
{
  return work->state;
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZUKAN_DETAIL_HEADBAR_Appear( ZUKAN_DETAIL_HEADBAR_WORK* work )
{
  if( work->state != ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
  {
    work->state = ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR_TO_APPEAR;
    work->scroll_wait_count = 0;
  }
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZUKAN_DETAIL_HEADBAR_Disappear( ZUKAN_DETAIL_HEADBAR_WORK* work )
{
  if( work->state != ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR )
  {
    work->state = ZUKAN_DETAIL_HEADBAR_STATE_APPEAR_TO_DISAPPEAR;
    work->scroll_wait_count = 0;
  }
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Zukan_Detail_Headbar_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_HEADBAR_WORK* work = (ZUKAN_DETAIL_HEADBAR_WORK*)wk;
}

//-------------------------------------
/// タイトルバー
//=====================================
static void Zukan_Detail_Headbar_CreateBase( ZUKAN_DETAIL_HEADBAR_WORK* work )
{
  PALTYPE paltype = ( ZKNDTL_BG_FRAME_S_HEADBAR < GFL_BG_FRAME0_S ) ? (PALTYPE_MAIN_BG) : (PALTYPE_SUB_BG);

  // パレット
  GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FONT, NARC_font_default_nclr,
       paltype,
       0,
       ZKNDTL_BG_PAL_POS_S_HEADBAR * 0x20,
       ZKNDTL_BG_PAL_NUM_S_HEADBAR * 0x20,
       work->heap_id );
  
  // BGフレームのスクリーンの空いている箇所に何も表示がされないようにしておく
  work->dummy_bmpwin = GFL_BMPWIN_Create( ZKNDTL_BG_FRAME_S_HEADBAR, 0, 0, 1, 1,
                            ZKNDTL_BG_PAL_POS_S_HEADBAR, GFL_BMP_CHRAREA_GET_F );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->dummy_bmpwin), 0 );
  GFL_BMPWIN_TransVramCharacter(work->dummy_bmpwin);

  // メッセージ
  work->head_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
                           ARCID_MESSAGE, NARC_message_zkn_dat,
                           work->heap_id );

  // NULL初期化
  work->head_bmpwin = NULL;
}
static void Zukan_Detail_Headbar_DeleteBase( ZUKAN_DETAIL_HEADBAR_WORK* work )
{
  GFL_MSG_Delete( work->head_msgdata );
  GFL_BMPWIN_Delete( work->dummy_bmpwin );
}
static void Zukan_Detail_Headbar_Create( ZUKAN_DETAIL_HEADBAR_WORK* work )
{
  // タイトルバー
  work->head_bmpwin = GFL_BMPWIN_Create( ZKNDTL_BG_FRAME_S_HEADBAR, 0, 0, 32, 3,
                            ZKNDTL_BG_PAL_POS_S_HEADBAR, GFL_BMP_CHRAREA_GET_F );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->head_bmpwin), 5 );
  GFL_BMPWIN_TransVramCharacter(work->head_bmpwin);

  // 文字描画
  if( ZUKAN_DETAIL_HEADBAR_TYPE_INFO <= work->type && work->type <= ZUKAN_DETAIL_HEADBAR_TYPE_FORM )
  {
    STRBUF* strbuf = GFL_MSG_CreateString( work->head_msgdata, ZKN_DETAIL_HEAD_01 + work->type );
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->head_bmpwin ),
        8, 5, strbuf, work->font, PRINTSYS_LSB_Make(0xF,2,0) );
    GFL_STR_DeleteBuffer( strbuf );
  }

  // スクリーン転送
  GFL_BMPWIN_MakeTransWindow_VBlank( work->head_bmpwin );
}
static void Zukan_Detail_Headbar_Delete( ZUKAN_DETAIL_HEADBAR_WORK* work )
{
  if( work->head_bmpwin ) GFL_BMPWIN_Delete( work->head_bmpwin );
  
  // NULL初期化
  work->head_bmpwin = NULL;
}

