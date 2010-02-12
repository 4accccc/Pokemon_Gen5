//============================================================================
/**
 *  @file   zukan_detail_touchbar.c
 *  @brief  図鑑詳細画面共通のタッチバー
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_TOUCHBAR
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "zukan_detail_def.h"
#include "zknd_tbar.h"
#include "zukan_detail_touchbar.h"

// アーカイブ
#include "arc_def.h"
#include "zukan_gra.naix"
#include "townmap_gra.naix"

// サウンド
#include "sound/wb_sound_data.sadl"

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_CUSTOM             = 3,
};
// 位置
enum
{
  OBJ_PAL_POS_M_CUSTOM             = ZKNDTL_OBJ_PAL_POS_M_TOUCHBAR + ZKND_TBAR_OBJ_PLT_NUM,
};

// セルアクターユニットで使えるワーク数
#define CLUNIT_WORK_NUM    (16)

// スクロール
#define SCROLL_Y_DISAPPEAR     (-24)
#define SCROLL_Y_APPEAR        (0)
#define SCROLL_VALUE_OUTSIDE   (1)  // 移動量
#define SCROLL_VALUE_INSIDE    (2)
#define SCROLL_WAIT_OUTSIDE    (0)  // 0=毎フレーム移動; 1=移動した後に1フレーム待つ; 2=移動した後に2フレーム待つ;
#define SCROLL_WAIT_INSIDE     (0)
static const int scroll_value[ZUKAN_DETAIL_TOUCHBAR_SPEED_MAX] =
{
  SCROLL_VALUE_OUTSIDE,
  SCROLL_VALUE_INSIDE,
};
static const u8 scroll_wait[ZUKAN_DETAIL_TOUCHBAR_SPEED_MAX] =
{
  SCROLL_WAIT_OUTSIDE,
  SCROLL_WAIT_INSIDE,
};

// カスタムボタン
// GENERAL
enum
{
  GENERAL_CUSTOM_INFO    = ZKND_TBAR_ICON_CUSTOM,
  GENERAL_CUSTOM_MAP,
  GENERAL_CUSTOM_VOICE,
  GENERAL_CUSTOM_FORM,
};
// MAP
enum
{
  MAP_CUSTOM_DUMMY       = ZKND_TBAR_ICON_CUSTOM,
};
// FORM
enum
{
  FORM_CUSTOM_EXCHANGE    = ZKND_TBAR_ICON_CUSTOM,
};

// ボタン列挙
// GENERAL
enum
{
  GENERAL_ICON_RETURN,
  GENERAL_ICON_CLOSE,
  GENERAL_ICON_CHECK,
  GENERAL_ICON_CUSTOM_FORM,
  GENERAL_ICON_CUSTOM_VOICE,
  GENERAL_ICON_CUSTOM_MAP,
  GENERAL_ICON_CUSTOM_INFO,
  GENERAL_ICON_CUR_D,
  GENERAL_ICON_CUR_U,
  GENERAL_ICON_MAX,
};
// MAP
enum
{
  MAP_ICON_RETURN,
  MAP_ICON_MAX,
};
// FORM 
enum
{
  FORM_ICON_RETURN,
  FORM_ICON_CUR_R,
  FORM_ICON_CUR_L,
  FORM_ICON_CUR_D,
  FORM_ICON_CUR_U,
  FORM_ICON_CUSTOM_EXCHANGE,
  FORM_ICON_MAX,
};
#define ICON_MAX_MAX (GENERAL_ICON_MAX)  // GENERAL_ICON_MAX, MAP_ICON_MAX, FORM_ICON_MAXの中で最大のもの

// GENERAL 
static const int general_icon_id[GENERAL_ICON_MAX] =
{
  ZKND_TBAR_ICON_RETURN,
  ZKND_TBAR_ICON_CLOSE,
  ZKND_TBAR_ICON_CHECK,
  GENERAL_CUSTOM_FORM,
  GENERAL_CUSTOM_VOICE,
  GENERAL_CUSTOM_MAP,
  GENERAL_CUSTOM_INFO,
  ZKND_TBAR_ICON_CUR_D,
  ZKND_TBAR_ICON_CUR_U,
};
// MAP
static const int map_icon_id[MAP_ICON_MAX] =
{
  ZKND_TBAR_ICON_RETURN,
};
// FORM
static const int form_icon_id[FORM_ICON_MAX] =
{
  ZKND_TBAR_ICON_RETURN,
  ZKND_TBAR_ICON_CUR_R,
  ZKND_TBAR_ICON_CUR_L,
  ZKND_TBAR_ICON_CUR_D,
  ZKND_TBAR_ICON_CUR_U,
  FORM_CUSTOM_EXCHANGE,
};

// GENERAL 
static const GFL_CLACTPOS general_icon_pos[GENERAL_ICON_MAX] =
{
  { 232, ZKND_TBAR_ICON_Y       },
  { 208, ZKND_TBAR_ICON_Y       },
  { 184, ZKND_TBAR_ICON_Y_CHECK },
  { 144, ZKND_TBAR_ICON_Y       },
  { 112, ZKND_TBAR_ICON_Y       },
  {  80, ZKND_TBAR_ICON_Y       },
  {  48, ZKND_TBAR_ICON_Y       },
  {  24, ZKND_TBAR_ICON_Y       },
  {   0, ZKND_TBAR_ICON_Y       },
};
// MAP
static const GFL_CLACTPOS map_icon_pos[MAP_ICON_MAX] =
{
  { 232, ZKND_TBAR_ICON_Y },
};
// FORM
static const GFL_CLACTPOS form_icon_pos[FORM_ICON_MAX] =
{
  { 232, ZKND_TBAR_ICON_Y },
  { 208, ZKND_TBAR_ICON_Y },
  { 184, ZKND_TBAR_ICON_Y },
  { 160, ZKND_TBAR_ICON_Y },
  { 136, ZKND_TBAR_ICON_Y },
  {   0, ZKND_TBAR_ICON_Y },
};

// GENERAL 
static const ZKNDTL_COMMAND general_command[GENERAL_ICON_MAX] =
{
  ZKNDTL_CMD_RETURN,
  ZKNDTL_CMD_CLOSE,
  ZKNDTL_CMD_CHECK,
  ZKNDTL_CMD_FORM,
  ZKNDTL_CMD_VOICE,
  ZKNDTL_CMD_MAP,
  ZKNDTL_CMD_INFO,
  ZKNDTL_CMD_CUR_D,
  ZKNDTL_CMD_CUR_U,
};
// MAP
static const ZKNDTL_COMMAND map_command[MAP_ICON_MAX] =
{
  ZKNDTL_CMD_MAP_RETURN,
};
// FORM
static const ZKNDTL_COMMAND form_command[FORM_ICON_MAX] =
{
  ZKNDTL_CMD_FORM_RETURN,
  ZKNDTL_CMD_FORM_CUR_R,
  ZKNDTL_CMD_FORM_CUR_L,
  ZKNDTL_CMD_FORM_CUR_D,
  ZKNDTL_CMD_FORM_CUR_U,
  ZKNDTL_CMD_FORM_EXCHANGE,
};


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
typedef struct
{
  int                    id;
  const GFL_CLACTPOS*    pos;
  GFL_CLWK*              clwk;
}
ICON_SET;

//-------------------------------------
/// ワーク
//=====================================
struct _ZUKAN_DETAIL_TOUCHBAR_WORK
{
  // 他のところから借用するもの
  HEAPID                        heap_id;
 
  // ここで作成するもの
  // 状態
  ZUKAN_DETAIL_TOUCHBAR_STATE   state;
  ZUKAN_DETAIL_TOUCHBAR_TYPE    prev_type;
  ZUKAN_DETAIL_TOUCHBAR_TYPE    type;
  ZUKAN_DETAIL_TOUCHBAR_DISP    disp;
  ZUKAN_DETAIL_TOUCHBAR_SPEED   speed;
  u8                            scroll_wait_count;

  // セルアクターユニット
  GFL_CLUNIT*                   clunit;

  // VBlank中TCB
  GFL_TCB*                      vblank_tcb;

  // タッチバー
  ZKND_TBAR_WORK*               tbwk;

  // カスタムボタン
  // GENERAL
  u32    general_ncl;
  u32    general_ncg;
  u32    general_nce;
  // MAP
  u32    map_ncl;
  u32    map_ncg;
  u32    map_nce;
  // FORM
  u32    form_ncl;
  u32    form_ncg;
  u32    form_nce;

  // ICON_SET
  u8            icon_set_num;
  ICON_SET      icon_set[ICON_MAX_MAX];
  s16           icon_ofs_pos_y;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Touchbar_VBlankFunc( GFL_TCB* tcb, void* wk );

// タッチバー
static void Zukan_Detail_Touchbar_Create( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_Delete( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_SetIconOfsPosY( ZUKAN_DETAIL_TOUCHBAR_WORK* work, s16 ofs_pos_y );

// GENERAL
static void Zukan_Detail_Touchbar_CreateGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_DeleteGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work );

// MAP
static void Zukan_Detail_Touchbar_CreateMap( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_DeleteMap( ZUKAN_DETAIL_TOUCHBAR_WORK* work );

// FORM
static void Zukan_Detail_Touchbar_CreateForm( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_DeleteForm( ZUKAN_DETAIL_TOUCHBAR_WORK* work );


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
ZUKAN_DETAIL_TOUCHBAR_WORK* ZUKAN_DETAIL_TOUCHBAR_Init( HEAPID heap_id )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*   work;

  // ワーク生成
  work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(ZUKAN_DETAIL_TOUCHBAR_WORK) );

  // 初期化
  work->heap_id           = heap_id;

  // 状態
  work->state = ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR;
  work->prev_type = ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL;
  work->type      = ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL;
  work->disp      = ZUKAN_DETAIL_TOUCHBAR_DISP_INFO;
  work->speed = ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE;

  // 初期スクロール位置
  GFL_BG_SetScroll( ZKNDTL_BG_FRAME_M_TOUCHBAR, GFL_BG_SCROLL_Y_SET, SCROLL_Y_DISAPPEAR );

  // プライオリティ
  GFL_BG_SetPriority( ZKNDTL_BG_FRAME_M_TOUCHBAR, ZKNDTL_BG_FRAME_PRI_M_TOUCHBAR );

  // セルアクターユニット
	work->clunit	= GFL_CLACT_UNIT_Create( CLUNIT_WORK_NUM, 0, work->heap_id );
	GFL_CLACT_UNIT_SetDefaultRend( work->clunit );

  // NULL初期化
  work->tbwk = NULL;

  // タッチバー
  Zukan_Detail_Touchbar_Create( work );
  ZKND_TBAR_SetActiveWhole( work->tbwk, FALSE );
  work->icon_ofs_pos_y = -SCROLL_Y_DISAPPEAR;
  Zukan_Detail_Touchbar_SetIconOfsPosY( work, work->icon_ofs_pos_y );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Touchbar_VBlankFunc, work, 1 );

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
void ZUKAN_DETAIL_TOUCHBAR_Exit( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );
  
  // タッチバー
  Zukan_Detail_Touchbar_Delete( work );

  // セルアクターユニット
	GFL_CLACT_UNIT_Delete( work->clunit );

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
void ZUKAN_DETAIL_TOUCHBAR_Main( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  switch( work->state )
  {
  case ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR:
    {
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR_TO_APPEAR:
    {
      if( work->scroll_wait_count == 0 )
      {
        if( GFL_BG_GetScrollY( ZKNDTL_BG_FRAME_M_TOUCHBAR ) >= SCROLL_Y_APPEAR )
        {
          GFL_BG_SetScrollReq( ZKNDTL_BG_FRAME_M_TOUCHBAR, GFL_BG_SCROLL_Y_SET, SCROLL_Y_APPEAR );
          work->icon_ofs_pos_y = SCROLL_Y_APPEAR;
          Zukan_Detail_Touchbar_SetIconOfsPosY( work, work->icon_ofs_pos_y );
          work->state = ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR;
          ZKND_TBAR_SetActiveWhole( work->tbwk, TRUE );
        }
        else
        {
          GFL_BG_SetScrollReq( ZKNDTL_BG_FRAME_M_TOUCHBAR, GFL_BG_SCROLL_Y_INC, scroll_value[work->speed] );
          work->icon_ofs_pos_y -= scroll_value[work->speed];
          Zukan_Detail_Touchbar_SetIconOfsPosY( work, work->icon_ofs_pos_y );
          work->scroll_wait_count = scroll_wait[work->speed];
        }
      }
      else
      {
        work->scroll_wait_count--;
      }
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR:
    {
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR_TO_DISAPPEAR:
    {
      if( work->scroll_wait_count == 0 )
      {
        if( GFL_BG_GetScrollY( ZKNDTL_BG_FRAME_M_TOUCHBAR ) <= SCROLL_Y_DISAPPEAR )
        {
          GFL_BG_SetScrollReq( ZKNDTL_BG_FRAME_M_TOUCHBAR, GFL_BG_SCROLL_Y_SET, SCROLL_Y_DISAPPEAR );
          work->icon_ofs_pos_y = -SCROLL_Y_DISAPPEAR;
          Zukan_Detail_Touchbar_SetIconOfsPosY( work, work->icon_ofs_pos_y );
          work->state = ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR;
        }
        else
        {
          GFL_BG_SetScrollReq( ZKNDTL_BG_FRAME_M_TOUCHBAR, GFL_BG_SCROLL_Y_DEC, scroll_value[work->speed] );
          work->icon_ofs_pos_y += scroll_value[work->speed];
          Zukan_Detail_Touchbar_SetIconOfsPosY( work, work->icon_ofs_pos_y );
          work->scroll_wait_count = scroll_wait[work->speed];
        }
      }
      else
      {
        work->scroll_wait_count--;
      }
    }
    break;
  }

  ZKND_TBAR_Main( work->tbwk );
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
void ZUKAN_DETAIL_TOUCHBAR_SetType(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   ZUKAN_DETAIL_TOUCHBAR_TYPE  type, 
                   ZUKAN_DETAIL_TOUCHBAR_DISP  disp )
{
  work->prev_type = work->type;
  work->type = type;
  work->disp = disp;

  Zukan_Detail_Touchbar_Delete( work );
  Zukan_Detail_Touchbar_Create( work );
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
ZUKAN_DETAIL_TOUCHBAR_STATE ZUKAN_DETAIL_TOUCHBAR_GetState( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
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
void ZUKAN_DETAIL_TOUCHBAR_Appear( ZUKAN_DETAIL_TOUCHBAR_WORK* work, ZUKAN_DETAIL_TOUCHBAR_SPEED speed )
{
  work->speed = speed;

  if( work->state != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
  {
    work->state = ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR_TO_APPEAR;
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
void ZUKAN_DETAIL_TOUCHBAR_Disappear( ZUKAN_DETAIL_TOUCHBAR_WORK* work, ZUKAN_DETAIL_TOUCHBAR_SPEED speed )
{
  work->speed = speed;

  if( work->state != ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR )
  {
    work->state = ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR_TO_DISAPPEAR;
    work->scroll_wait_count = 0;
    
    ZKND_TBAR_SetActiveWhole( work->tbwk, FALSE );
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
ZKNDTL_COMMAND ZUKAN_DETAIL_TOUCHBAR_GetTrg( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  int icon_id = ZKND_TBAR_GetTrg( work->tbwk );
  ZKNDTL_COMMAND cmd = ZKNDTL_CMD_NONE;
  u8 i;

  if( icon_id != ZKND_TBAR_SELECT_NONE )
  {
    switch( work->type )
    {
    case ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL:
      {
        for( i=0; i<work->icon_set_num; i++ )
        {
          if( icon_id == work->icon_set[i].id )
          {
            cmd = general_command[i];
            break;
          }
        }
      }
      break;
    case ZUKAN_DETAIL_TOUCHBAR_TYPE_MAP:
      {
        for( i=0; i<work->icon_set_num; i++ )
        {
          if( icon_id == work->icon_set[i].id )
          {
            cmd = map_command[i];
            break;
          }
        }
      }
      break;
    case ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM:
      {
        for( i=0; i<work->icon_set_num; i++ )
        {
          if( icon_id == work->icon_set[i].id )
          {
            cmd = form_command[i];
            break;
          }
        }
      }
      break;
    }
  }

  return cmd;
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
void ZUKAN_DETAIL_TOUCHBAR_Unlock( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  ZKND_TBAR_UnlockWhole( work->tbwk );
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Zukan_Detail_Touchbar_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK* work = (ZUKAN_DETAIL_TOUCHBAR_WORK*)wk;
}

//-------------------------------------
/// タッチバー
//=====================================
static void Zukan_Detail_Touchbar_Create( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  Zukan_Detail_Touchbar_Delete( work );

  switch( work->type )
  {
  case ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL:
    {
      Zukan_Detail_Touchbar_CreateGeneral( work );
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_TYPE_MAP:
    {
      Zukan_Detail_Touchbar_CreateMap( work );
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM:
    {
      Zukan_Detail_Touchbar_CreateForm( work );
    }
    break;
  }
}
static void Zukan_Detail_Touchbar_Delete( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  switch( work->prev_type )
  {
  case ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL:
    {
      Zukan_Detail_Touchbar_DeleteGeneral( work );
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_TYPE_MAP:
    {
      Zukan_Detail_Touchbar_DeleteMap( work );
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM:
    {
      Zukan_Detail_Touchbar_DeleteForm( work );
    }
    break;
  }
}

static void Zukan_Detail_Touchbar_SetIconOfsPosY( ZUKAN_DETAIL_TOUCHBAR_WORK* work, s16 ofs_pos_y )
{
  u8 i;
  for( i=0; i<work->icon_set_num; i++ )
  {
    GFL_CLACTPOS pos = *(work->icon_set[i].pos);
    pos.y += ofs_pos_y;
    GFL_CLACT_WK_SetWldPos( work->icon_set[i].clwk, &pos );
  }
}

//-------------------------------------
/// GENERAL
//=====================================
static void Zukan_Detail_Touchbar_CreateGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  CLSYS_DRAW_TYPE draw_type;
  
  // メイン or サブ
  draw_type = (ZKNDTL_BG_FRAME_M_TOUCHBAR < GFL_BG_FRAME0_S) ? (CLSYS_DRAW_MAIN) : (CLSYS_DRAW_SUB);
 
  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, work->heap_id );
    work->general_ncl = GFL_CLGRP_PLTT_RegisterEx(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCLR,
                            draw_type,
                            OBJ_PAL_POS_M_CUSTOM * 0x20, 0, OBJ_PAL_NUM_M_CUSTOM,
                            work->heap_id );	
    work->general_ncg = GFL_CLGRP_CGR_Register(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCGR,
                            FALSE,
                            draw_type,
                            work->heap_id );
    work->general_nce = GFL_CLGRP_CELLANIM_Register(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCER,
                            NARC_zukan_gra_info_info_obj_NANR,
                            work->heap_id );
    GFL_ARC_CloseDataHandle( handle );
  }

  // ICON_SETのidを設定する
  work->icon_set_num = GENERAL_ICON_MAX;
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      work->icon_set[i].id = general_icon_id[i];
      work->icon_set[i].pos = &(general_icon_pos[i]);
    } 
  }

  // タッチバーの設定
  {
    ZKND_TBAR_SETUP setup;
    
    // アイコンの設定
    // 数分作る
    ZKND_TBAR_ITEM_ICON icon_tbl[] =
    {
      {
        ZKND_TBAR_ICON_RETURN,
        { 232, ZKND_TBAR_ICON_Y       },
        24,
      },
      {	
        ZKND_TBAR_ICON_CLOSE,
        { 208, ZKND_TBAR_ICON_Y       },
        24,
      },
      {	
        ZKND_TBAR_ICON_CHECK,
        { 184, ZKND_TBAR_ICON_Y_CHECK },
        24,
      },

      // カスタム↓
      {	
        GENERAL_CUSTOM_FORM,
        { 144, ZKND_TBAR_ICON_Y },
        32,
      },
      {	
        GENERAL_CUSTOM_VOICE,
        { 112, ZKND_TBAR_ICON_Y },
        32,
      },
      {	
        GENERAL_CUSTOM_MAP,
        { 80, ZKND_TBAR_ICON_Y },
        32,
      },
      {
        GENERAL_CUSTOM_INFO,
        { 48, ZKND_TBAR_ICON_Y },
        32,
      },
      // カスタム↑

      {
        ZKND_TBAR_ICON_CUR_D,
        { 24, ZKND_TBAR_ICON_Y },
       24,
      },
      {	
        ZKND_TBAR_ICON_CUR_U,
        { 0, ZKND_TBAR_ICON_Y },
        24,
      },
    };

    // 以下カスタムボタンならば入れなくてはいけない情報
    // カスタムボタン
    {
      u8 i;

      i=GENERAL_ICON_CUSTOM_FORM;
      {
        icon_tbl[i].cg_idx            = work->general_ncg;         // キャラリソース
        icon_tbl[i].plt_idx           = work->general_ncl;         // パレットリソース
        icon_tbl[i].cell_idx          = work->general_nce;         // セルリソース
        icon_tbl[i].active_anmseq     = 3;                         // アクティブのときのアニメ
        icon_tbl[i].noactive_anmseq   = 3;                        // ノンアクティブのときのアニメ
        icon_tbl[i].push_anmseq       = 7;                        // 押したときのアニメ（STOPになっていること）
        icon_tbl[i].key               = 0;                         // キーで押したときに動作させたいならば、ボタン番号
        icon_tbl[i].se                = SEQ_SE_DECIDE1;            // 押したときにSEならしたいならば、SEの番号
      }
      i=GENERAL_ICON_CUSTOM_VOICE;
      {
        icon_tbl[i].cg_idx            = work->general_ncg;         // キャラリソース
        icon_tbl[i].plt_idx           = work->general_ncl;         // パレットリソース
        icon_tbl[i].cell_idx          = work->general_nce;         // セルリソース
        icon_tbl[i].active_anmseq     = 2;                         // アクティブのときのアニメ
        icon_tbl[i].noactive_anmseq   = 2;                        // ノンアクティブのときのアニメ
        icon_tbl[i].push_anmseq       = 6;                        // 押したときのアニメ（STOPになっていること）
        icon_tbl[i].key               = 0;                         // キーで押したときに動作させたいならば、ボタン番号
        icon_tbl[i].se                = SEQ_SE_DECIDE1;            // 押したときにSEならしたいならば、SEの番号
      }
      i=GENERAL_ICON_CUSTOM_MAP;
      {
        icon_tbl[i].cg_idx            = work->general_ncg;         // キャラリソース
        icon_tbl[i].plt_idx           = work->general_ncl;         // パレットリソース
        icon_tbl[i].cell_idx          = work->general_nce;         // セルリソース
        icon_tbl[i].active_anmseq     = 1;                         // アクティブのときのアニメ
        icon_tbl[i].noactive_anmseq   = 1;                        // ノンアクティブのときのアニメ
        icon_tbl[i].push_anmseq       = 5;                        // 押したときのアニメ（STOPになっていること）
        icon_tbl[i].key               = 0;                         // キーで押したときに動作させたいならば、ボタン番号
        icon_tbl[i].se                = SEQ_SE_DECIDE1;            // 押したときにSEならしたいならば、SEの番号
      }
      i=GENERAL_ICON_CUSTOM_INFO;
      {
        icon_tbl[i].cg_idx            = work->general_ncg;         // キャラリソース
        icon_tbl[i].plt_idx           = work->general_ncl;         // パレットリソース
        icon_tbl[i].cell_idx          = work->general_nce;         // セルリソース
        icon_tbl[i].active_anmseq     = 0;                         // アクティブのときのアニメ
        icon_tbl[i].noactive_anmseq   = 0;                        // ノンアクティブのときのアニメ
        icon_tbl[i].push_anmseq       = 4;                        // 押したときのアニメ（STOPになっていること）
        icon_tbl[i].key               = 0;                         // キーで押したときに動作させたいならば、ボタン番号
        icon_tbl[i].se                = SEQ_SE_DECIDE1;            // 押したときにSEならしたいならば、SEの番号
      }
    }

    // 設定構造体
    // さきほどの窓情報＋リソース情報をいれる
    GFL_STD_MemClear( &setup, sizeof(ZKND_TBAR_SETUP) );

    setup.p_item		= icon_tbl;				                             // 上の窓情報
    setup.item_num	= NELEMS(icon_tbl);                            // いくつ窓があるか
    setup.p_unit		= work->clunit;										             // OBJ読み込みのためのCLUNIT
    setup.bar_frm 	= ZKNDTL_BG_FRAME_M_TOUCHBAR;		               // BG読み込みのためのBG面
    setup.bg_plt		= ZKNDTL_BG_PAL_POS_M_TOUCHBAR;                // BGパレット
    setup.obj_plt  	= ZKNDTL_OBJ_PAL_POS_M_TOUCHBAR;	           	 // OBJパレット
    setup.mapping	  = ZKNDTL_OBJ_MAPPING_M;   	                   // マッピングモード

    work->tbwk = ZKND_TBAR_Init( &setup, work->heap_id );
  }

  // ICON_SETのclwkを設定する
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      work->icon_set[i].clwk = ZKND_TBAR_GetClwk( work->tbwk, work->icon_set[i].id );
    } 
  }
}
static void Zukan_Detail_Touchbar_DeleteGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  if( work->tbwk )
  {
    // タッチバー破棄
    ZKND_TBAR_Exit( work->tbwk );

    // リソース破棄
    {	
      GFL_CLGRP_PLTT_Release( work->general_ncl );
      GFL_CLGRP_CGR_Release( work->general_ncg );
      GFL_CLGRP_CELLANIM_Release( work->general_nce );
    }
  }

  // NULL初期化
  work->tbwk = NULL;
}

//-------------------------------------
/// MAP
//=====================================
static void Zukan_Detail_Touchbar_CreateMap( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  CLSYS_DRAW_TYPE draw_type;
  
  // メイン or サブ
  draw_type = (ZKNDTL_BG_FRAME_M_TOUCHBAR < GFL_BG_FRAME0_S) ? (CLSYS_DRAW_MAIN) : (CLSYS_DRAW_SUB);
 
  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, work->heap_id );
    work->map_ncl = GFL_CLGRP_PLTT_RegisterEx(
                            handle,
                            NARC_townmap_gra_tmap_obj_NCLR,
                            draw_type,
                            OBJ_PAL_POS_M_CUSTOM * 0x20, 2, OBJ_PAL_NUM_M_CUSTOM,
                            work->heap_id );	
    work->map_ncg = GFL_CLGRP_CGR_Register(
                            handle,
                            NARC_townmap_gra_tmap_obj_d_NCGR,
                            FALSE,
                            draw_type,
                            work->heap_id );
    work->map_nce = GFL_CLGRP_CELLANIM_Register(
                            handle,
                            NARC_townmap_gra_tmap_obj_d_NCER,
                            NARC_townmap_gra_tmap_obj_d_NANR,
                            work->heap_id );
    GFL_ARC_CloseDataHandle( handle );
  }

  // ICON_SETのidを設定する
  work->icon_set_num = MAP_ICON_MAX;
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      work->icon_set[i].id = map_icon_id[i];
      work->icon_set[i].pos = &(map_icon_pos[i]);
    } 
  }

  // タッチバーの設定
  {
    ZKND_TBAR_SETUP setup;
    
    // アイコンの設定
    // 数分作る
    ZKND_TBAR_ITEM_ICON icon_tbl[] =
    {
      {
        ZKND_TBAR_ICON_RETURN,
        { 232, ZKND_TBAR_ICON_Y },
        24,
      },
    };

    // 設定構造体
    // さきほどの窓情報＋リソース情報をいれる
    GFL_STD_MemClear( &setup, sizeof(ZKND_TBAR_SETUP) );

    setup.p_item		= icon_tbl;				                             // 上の窓情報
    setup.item_num	= NELEMS(icon_tbl);                            // いくつ窓があるか
    setup.p_unit		= work->clunit;										             // OBJ読み込みのためのCLUNIT
    setup.bar_frm 	= ZKNDTL_BG_FRAME_M_TOUCHBAR;		               // BG読み込みのためのBG面
    setup.bg_plt		= ZKNDTL_BG_PAL_POS_M_TOUCHBAR;                // BGパレット
    setup.obj_plt  	= ZKNDTL_OBJ_PAL_POS_M_TOUCHBAR;	           	 // OBJパレット
    setup.mapping	  = ZKNDTL_OBJ_MAPPING_M;   	                   // マッピングモード

    work->tbwk = ZKND_TBAR_Init( &setup, work->heap_id );
  }

  // ICON_SETのclwkを設定する
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      work->icon_set[i].clwk = ZKND_TBAR_GetClwk( work->tbwk, work->icon_set[i].id );
    } 
  }
}
static void Zukan_Detail_Touchbar_DeleteMap( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  if( work->tbwk )
  {
    // タッチバー破棄
    ZKND_TBAR_Exit( work->tbwk );

    // リソース破棄
    {	
      GFL_CLGRP_PLTT_Release( work->map_ncl );
      GFL_CLGRP_CGR_Release( work->map_ncg );
      GFL_CLGRP_CELLANIM_Release( work->map_nce );
    }
  }

  // NULL初期化
  work->tbwk = NULL;
}

//-------------------------------------
/// FORM
//=====================================
static void Zukan_Detail_Touchbar_CreateForm( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  CLSYS_DRAW_TYPE draw_type;
  
  // メイン or サブ
  draw_type = (ZKNDTL_BG_FRAME_M_TOUCHBAR < GFL_BG_FRAME0_S) ? (CLSYS_DRAW_MAIN) : (CLSYS_DRAW_SUB);
 
  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, work->heap_id );
    work->form_ncl = GFL_CLGRP_PLTT_RegisterEx(
                            handle,
                            NARC_townmap_gra_tmap_obj_NCLR,
                            draw_type,
                            OBJ_PAL_POS_M_CUSTOM * 0x20, 2, OBJ_PAL_NUM_M_CUSTOM,
                            work->heap_id );	
    work->form_ncg = GFL_CLGRP_CGR_Register(
                            handle,
                            NARC_townmap_gra_tmap_obj_d_NCGR,
                            FALSE,
                            draw_type,
                            work->heap_id );
    work->form_nce = GFL_CLGRP_CELLANIM_Register(
                            handle,
                            NARC_townmap_gra_tmap_obj_d_NCER,
                            NARC_townmap_gra_tmap_obj_d_NANR,
                            work->heap_id );
    GFL_ARC_CloseDataHandle( handle );
  }

  // ICON_SETのidを設定する
  work->icon_set_num = FORM_ICON_MAX;
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      work->icon_set[i].id = form_icon_id[i];
      work->icon_set[i].pos = &(form_icon_pos[i]);
    } 
  }

  // タッチバーの設定
  {
    ZKND_TBAR_SETUP setup;
    
    // アイコンの設定
    // 数分作る
    ZKND_TBAR_ITEM_ICON icon_tbl[] =
    {
      {
        ZKND_TBAR_ICON_CLOSE,
        { 232, ZKND_TBAR_ICON_Y },
        24,
      },
      {	
        ZKND_TBAR_ICON_CUR_R,
        { 208, ZKND_TBAR_ICON_Y },
        24,
      },
      {	
        ZKND_TBAR_ICON_CUR_L,
        { 184, ZKND_TBAR_ICON_Y },
        24,
      },
      {	
        ZKND_TBAR_ICON_CUR_D,
        { 160, ZKND_TBAR_ICON_Y },
        24,
      },
      {	
        ZKND_TBAR_ICON_CUR_U,
        { 136, ZKND_TBAR_ICON_Y },
        24,
      },
      
      // カスタム↓
      {	
        FORM_CUSTOM_EXCHANGE,
        { 0, ZKND_TBAR_ICON_Y },
        32,
      },
      // カスタム↑

    };

    // 以下カスタムボタンならば入れなくてはいけない情報
    // カスタムボタン
    {
      u8 i;
      for( i=FORM_ICON_CUSTOM_EXCHANGE; i<=FORM_ICON_CUSTOM_EXCHANGE; i++ )
      {
        icon_tbl[i].cg_idx            = work->form_ncg;         // キャラリソース
        icon_tbl[i].plt_idx           = work->form_ncl;         // パレットリソース
        icon_tbl[i].cell_idx          = work->form_nce;         // セルリソース
        icon_tbl[i].active_anmseq     = 8;                         // アクティブのときのアニメ
        icon_tbl[i].noactive_anmseq   = 12;                        // ノンアクティブのときのアニメ
        icon_tbl[i].push_anmseq       = 10;                        // 押したときのアニメ（STOPになっていること）
        icon_tbl[i].key               = PAD_BUTTON_A;              // キーで押したときに動作させたいならば、ボタン番号
        icon_tbl[i].se                = SEQ_SE_DECIDE1;            // 押したときにSEならしたいならば、SEの番号
      }
    }

    // 設定構造体
    // さきほどの窓情報＋リソース情報をいれる
    GFL_STD_MemClear( &setup, sizeof(ZKND_TBAR_SETUP) );

    setup.p_item		= icon_tbl;				                             // 上の窓情報
    setup.item_num	= NELEMS(icon_tbl);                            // いくつ窓があるか
    setup.p_unit		= work->clunit;										             // OBJ読み込みのためのCLUNIT
    setup.bar_frm 	= ZKNDTL_BG_FRAME_M_TOUCHBAR;		               // BG読み込みのためのBG面
    setup.bg_plt		= ZKNDTL_BG_PAL_POS_M_TOUCHBAR;                // BGパレット
    setup.obj_plt  	= ZKNDTL_OBJ_PAL_POS_M_TOUCHBAR;	           	 // OBJパレット
    setup.mapping	  = ZKNDTL_OBJ_MAPPING_M;   	                   // マッピングモード

    work->tbwk = ZKND_TBAR_Init( &setup, work->heap_id );
  }

  // ICON_SETのclwkを設定する
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      work->icon_set[i].clwk = ZKND_TBAR_GetClwk( work->tbwk, work->icon_set[i].id );
    } 
  }

  /*
  {
    // パレットの0番から読み込んでいれば、
    // そのパレットをどこに配置していようと
    // パレットのオフセットをしてやる必要はない。

    // 1番のパレットを使っているOBJ。
    // そのパレットの1番を8番に読み込む。
    u32 ncl, ncg, nce;
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heap_id );
    ncl = GFL_CLGRP_PLTT_RegisterEx(
                    handle,
                    NARC_townmap_gra_tmap_obj_NCLR,
                    draw_type,
                    8 * 0x20, 1, 1,
                    heap_id );	
    ncg = GFL_CLGRP_CGR_Register(
                    handle,
                    NARC_townmap_gra_tmap_obj_d_NCGR,
                    FALSE,
                    draw_type,
                    heap_id );
    nce = GFL_CLGRP_CELLANIM_Register(
                    handle,
                    NARC_townmap_gra_tmap_obj_d_NCER,
                    NARC_townmap_gra_tmap_obj_d_NANR,
                    heap_id );
    GFL_ARC_CloseDataHandle( handle );

    {
      static GFL_CLWK* clwk;
		  GFL_CLWK_DATA cldata;
		  GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		  cldata.pos_x	= 64;
	  	cldata.pos_y	= 64;
      cldata.anmseq = 6;
      clwk = GFL_CLACT_WK_Create( work->clunit, ncg, ncl, nce, &cldata, CLSYS_DEFREND_MAIN, heap_id );
		  GFL_CLACT_WK_SetAutoAnmFlag( clwk, TRUE );
      // パレットのオフセットをしておかないと、9番のパレットを使って描画される。
      GFL_CLACT_WK_SetPlttOffs( clwk, 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );  // 8番のパレットが使われる。
      //GFL_CLACT_WK_SetPlttOffs( clwk, 0, CLWK_PLTTOFFS_MODE_OAM_COLOR );  // 9番のパレットが使われる。
      //GFL_CLACT_WK_SetPlttOffs( clwk, 8, CLWK_PLTTOFFS_MODE_OAM_COLOR );  // どこが使われるかよく分からない。
      //GFL_CLACT_WK_SetPlttOffs( clwk, 1, CLWK_PLTTOFFS_MODE_OAM_COLOR );  // 10番のパレットが使われる。
      // 疑問
      // 例えば1番と2番のパレットを2本使うようなOBJがあった場合、
      // GFL_CLACT_WK_SetPlttOffs( clwk, 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );としたら、ちゃんと8番と9番を使うだろうか？
      // うまくいかず、8番しか使わない、なんてことにはならないだろうか？
    }
  }
  */
}
static void Zukan_Detail_Touchbar_DeleteForm( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  if( work->tbwk )
  {
    // タッチバー破棄
    ZKND_TBAR_Exit( work->tbwk );

    // リソース破棄
    {	
      GFL_CLGRP_PLTT_Release( work->form_ncl );
      GFL_CLGRP_CGR_Release( work->form_ncg );
      GFL_CLGRP_CELLANIM_Release( work->form_nce );
    }
  }

  // NULL初期化
  work->tbwk = NULL;
}

