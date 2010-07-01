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
//#define USE_MAP_DUMMY_ICON  // これが定義されているとき、MAPでダミーのアイコンを用意しておく


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

// GENERALのパレットアニメ用にパレットを変更する
#define OBJ_PAL_OFFSET_GENERAL_ANIME_NONE  (0)
#define OBJ_PAL_OFFSET_GENERAL_ANIME_EXEC  (2)
#define RES_PAL_POS_GENERAL_ANIME_START    (1)  // リソースのパレットの列番号
#define RES_PAL_POS_GENERAL_ANIME_END      (3)  // リソースのパレットの列番号
#define GENERAL_ANIME_ADD              (0x400)  // FX_CosIdxを使用するので0<= <0x10000

// セルアクターユニットで使えるワーク数
#define CLUNIT_WORK_NUM    (16)

// スクロール
#define SCROLL_Y_DISAPPEAR     (-24)
#define SCROLL_Y_APPEAR        (0)
#define SCROLL_VALUE_OUTSIDE   (3)  // 移動量
#define SCROLL_VALUE_INSIDE    (3)
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
  GENERAL_ICON_CUSTOM_FORM,   // この並び順を使用している箇所があるので、並び順は変更しないこと↓
  GENERAL_ICON_CUSTOM_VOICE,  // ↓
  GENERAL_ICON_CUSTOM_MAP,    // ↓
  GENERAL_ICON_CUSTOM_INFO,   // ↓
  GENERAL_ICON_CUR_D,
  GENERAL_ICON_CUR_U,
  GENERAL_ICON_MAX,
};
// MAP
enum
{
  MAP_ICON_RETURN,
#ifdef USE_MAP_DUMMY_ICON
  MAP_ICON_CUSTOM_DUMMY,  // ダミーのアイコン  // 常に非表示にしておく。これがないと、アイコンを削除して生成する際に、画面が乱れてしまう。
#endif
  MAP_ICON_MAX,
};
// FORM 
enum
{
  FORM_ICON_RETURN,
  FORM_ICON_CUR_R,
  FORM_ICON_CUR_L,
  FORM_ICON_CUSTOM_EXCHANGE,
  FORM_ICON_CUR_D,
  FORM_ICON_CUR_U,
  FORM_ICON_MAX,
};
#define ICON_MAX_MAX (GENERAL_ICON_MAX)  // GENERAL_ICON_MAX, MAP_ICON_MAX, FORM_ICON_MAXの中で最大のもの

typedef struct
{
  int            id;
  GFL_CLACTPOS   pos;
  u16            width;
  ZKNDTL_COMMAND cmd;     // ボタンのアニメが終わった瞬間
  ZKNDTL_COMMAND scmd;    // ボタンを触った瞬間
}
ICON_CONST_SET;

// GENERAL
static const ICON_CONST_SET general_icon_const_set[GENERAL_ICON_MAX] =
{
  { ZKND_TBAR_ICON_RETURN,    { 232, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_RETURN,         ZKNDTL_SCMD_RETURN,        },
  { ZKND_TBAR_ICON_CLOSE,     { 208, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_CLOSE,          ZKNDTL_SCMD_CLOSE,         },
  { ZKND_TBAR_ICON_CHECK,     { 184, ZKND_TBAR_ICON_Y_CHECK },    24,  ZKNDTL_CMD_CHECK,          ZKNDTL_SCMD_CHECK,         },
  { GENERAL_CUSTOM_FORM,      { 144, ZKND_TBAR_ICON_Y       },    32,  ZKNDTL_CMD_FORM,           ZKNDTL_SCMD_FORM,          },
  { GENERAL_CUSTOM_VOICE,     { 112, ZKND_TBAR_ICON_Y       },    32,  ZKNDTL_CMD_VOICE,          ZKNDTL_SCMD_VOICE,         }, 
  { GENERAL_CUSTOM_MAP,       {  80, ZKND_TBAR_ICON_Y       },    32,  ZKNDTL_CMD_MAP,            ZKNDTL_SCMD_MAP,           },
  { GENERAL_CUSTOM_INFO,      {  48, ZKND_TBAR_ICON_Y       },    32,  ZKNDTL_CMD_INFO,           ZKNDTL_SCMD_INFO,          },
  { ZKND_TBAR_ICON_CUR_D,     {  24, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_CUR_D,          ZKNDTL_SCMD_CUR_D,         },
  { ZKND_TBAR_ICON_CUR_U,     {   0, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_CUR_U,          ZKNDTL_SCMD_CUR_U,         },
};
static const GFL_CLACTPOS general_icon_const_pos_no_form[GENERAL_ICON_MAX] =
{
  { 232, ZKND_TBAR_ICON_Y       }, 
  { 208, ZKND_TBAR_ICON_Y       }, 
  { 184, ZKND_TBAR_ICON_Y_CHECK }, 
  { 168, ZKND_TBAR_ICON_Y       }, 
  { 136, ZKND_TBAR_ICON_Y       }, 
  {  96, ZKND_TBAR_ICON_Y       }, 
  {  56, ZKND_TBAR_ICON_Y       }, 
  {  24, ZKND_TBAR_ICON_Y       }, 
  {   0, ZKND_TBAR_ICON_Y       }, 
};
// MAP
static const ICON_CONST_SET map_icon_const_set[MAP_ICON_MAX] =
{
  { ZKND_TBAR_ICON_RETURN,    { 232, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_MAP_RETURN,     ZKNDTL_SCMD_MAP_RETURN,    },
#ifdef USE_MAP_DUMMY_ICON
  { MAP_CUSTOM_DUMMY,         { 104, ZKND_TBAR_ICON_Y       },    48,  ZKNDTL_CMD_NONE,           ZKNDTL_CMD_NONE,           },
#endif
};
// FORM
static const ICON_CONST_SET form_icon_const_set[FORM_ICON_MAX] =
{
  { ZKND_TBAR_ICON_RETURN,    { 232, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_FORM_RETURN,    ZKNDTL_SCMD_FORM_RETURN,   },
  { ZKND_TBAR_ICON_CUR_R,     { 192, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_FORM_CUR_R,     ZKNDTL_SCMD_FORM_CUR_R,    },
  { ZKND_TBAR_ICON_CUR_L,     { 168, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_FORM_CUR_L,     ZKNDTL_SCMD_FORM_CUR_L,    },
  { FORM_CUSTOM_EXCHANGE,     { 104, ZKND_TBAR_ICON_Y       },    48,  ZKNDTL_CMD_FORM_EXCHANGE,  ZKNDTL_SCMD_FORM_EXCHANGE, },
  { ZKND_TBAR_ICON_CUR_D,     {  24, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_FORM_CUR_D,     ZKNDTL_SCMD_FORM_CUR_D,    },
  { ZKND_TBAR_ICON_CUR_U,     {   0, ZKND_TBAR_ICON_Y       },    24,  ZKNDTL_CMD_FORM_CUR_U,     ZKNDTL_SCMD_FORM_CUR_U,    },
};


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
typedef struct
{
  const ICON_CONST_SET*  cset;
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

  // 引数
  BOOL                          form_version;  // TRUEのときフォルムありバージョン

  // ここで作成するもの
  // 状態
  ZUKAN_DETAIL_TOUCHBAR_STATE   state;
  ZUKAN_DETAIL_TOUCHBAR_TYPE    prev_type;
  ZUKAN_DETAIL_TOUCHBAR_TYPE    type;
  ZUKAN_DETAIL_TOUCHBAR_DISP    prev_disp;
  ZUKAN_DETAIL_TOUCHBAR_DISP    disp;
  ZUKAN_DETAIL_TOUCHBAR_DISP    curr_icon_disp_general;
  ZUKAN_DETAIL_TOUCHBAR_SPEED   speed;
  u8                            scroll_wait_count;

  // セルアクターユニット
  GFL_CLUNIT*                   clunit;

  // VBlank中TCB
  GFL_TCB*                      vblank_tcb;

  // タッチバー
  ZKND_TBAR_WORK*               tbwk;

  // ユーザ指定の全体専用のアクティブフラグ
  BOOL                          user_whole_is_active;  // TRUEのときアクティブ

  // BGプライオリティ
  u8                            bg_pri;
  BOOL                          req;     // TRUEのときVBlank中にbg_priを設定する

  // 上下アイコンを表示するか否か
  BOOL                          is_cur_u_d;  // TRUEのとき表示する

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

  // GENERALのパレットアニメ
  u16           pal_anime_general_start[0x10];
  u16           pal_anime_general_end[0x10];
  u16           pal_anime_general_now[0x10];
  int           pal_anime_general_count;
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
static void Zukan_Detail_Touchbar_MainGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
// GENERALのパレットアニメ
static void Zukan_Detail_Touchbar_AnimeBaseInitGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_AnimeBaseExitGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_AnimeInitGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_AnimeExitGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_AnimeMainGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
// 変換
static u8 Zukan_Detail_Touchbar_DispToGeneralIcon( ZUKAN_DETAIL_TOUCHBAR_DISP disp );
static u8 Zukan_Detail_Touchbar_CmdToGeneralIcon( ZKNDTL_COMMAND cmd );
static u8 Zukan_Detail_Touchbar_CmdToDisp( ZKNDTL_COMMAND cmd );

// MAP
static void Zukan_Detail_Touchbar_CreateMap( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_DeleteMap( ZUKAN_DETAIL_TOUCHBAR_WORK* work );

// FORM
static void Zukan_Detail_Touchbar_CreateForm( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_DeleteForm( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
static void Zukan_Detail_Touchbar_MainForm( ZUKAN_DETAIL_TOUCHBAR_WORK* work );


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
ZUKAN_DETAIL_TOUCHBAR_WORK* ZUKAN_DETAIL_TOUCHBAR_Init( HEAPID heap_id, BOOL form_version )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*   work;

  // ワーク生成
  work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(ZUKAN_DETAIL_TOUCHBAR_WORK) );

  // 初期化
  work->heap_id           = heap_id;
  work->form_version      = form_version;
  
  // 状態
  work->state = ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR;
  work->prev_type              = ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL;
  work->type                   = ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL;
  work->prev_disp              = ZUKAN_DETAIL_TOUCHBAR_DISP_MAP;      // work->disp以外なら初期化できちんとなる
  work->disp                   = ZUKAN_DETAIL_TOUCHBAR_DISP_INFO;
  work->curr_icon_disp_general = work->disp;                          // work->dispできちんと初期化
  work->speed = ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE;

  // 初期スクロール位置
  GFL_BG_SetScroll( ZKNDTL_BG_FRAME_M_TOUCHBAR, GFL_BG_SCROLL_Y_SET, SCROLL_Y_DISAPPEAR );

  // プライオリティ
  GFL_BG_SetPriority( ZKNDTL_BG_FRAME_M_TOUCHBAR, ZKNDTL_BG_FRAME_PRI_M_TOUCHBAR );

  // BGプライオリティ
  work->bg_pri = ZKNDTL_BG_FRAME_PRI_M_TOUCHBAR;
  work->req    = FALSE;

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
          if( work->user_whole_is_active ) ZKND_TBAR_SetActiveWhole( work->tbwk, TRUE );
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

  // GENERAL
  if( work->type == ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL )
  {
    Zukan_Detail_Touchbar_MainGeneral( work );
    // GENERALのパレットアニメ
    Zukan_Detail_Touchbar_AnimeMainGeneral( work );
  }
  // FORM
  else if( work->type == ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM )
  {
    Zukan_Detail_Touchbar_MainForm( work );
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
void ZUKAN_DETAIL_TOUCHBAR_SetType(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   ZUKAN_DETAIL_TOUCHBAR_TYPE  type, 
                   ZUKAN_DETAIL_TOUCHBAR_DISP  disp,
                   BOOL                        is_cur_u_d )
{
  work->prev_type = work->type;
  work->type = type;
  work->prev_disp = work->disp;
  work->disp = disp;
  work->is_cur_u_d = is_cur_u_d;

  Zukan_Detail_Touchbar_Delete( work );

  // curr_icon_disp_generalを変更
  work->curr_icon_disp_general = work->disp;
  
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
    case ZUKAN_DETAIL_TOUCHBAR_TYPE_MAP:
    case ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM:
      {
        for( i=0; i<work->icon_set_num; i++ )
        {
          if( icon_id == work->icon_set[i].cset->id )
          {
            cmd = work->icon_set[i].cset->cmd;;
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
ZKNDTL_COMMAND ZUKAN_DETAIL_TOUCHBAR_GetTouch( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  int icon_id = ZKND_TBAR_GetTouch( work->tbwk );
  ZKNDTL_COMMAND cmd = ZKNDTL_CMD_NONE;
  u8 i;

  if( icon_id != ZKND_TBAR_SELECT_NONE )
  {
    switch( work->type )
    {
    case ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL:
    case ZUKAN_DETAIL_TOUCHBAR_TYPE_MAP:
    case ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM:
      {
        for( i=0; i<work->icon_set_num; i++ )
        {
          if( icon_id == work->icon_set[i].cset->id )
          {
            cmd = work->icon_set[i].cset->scmd;
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

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZUKAN_DETAIL_TOUCHBAR_SetVisibleAll(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   BOOL                        is_visible )
{
  ZKND_TBAR_SetVisibleAll( work->tbwk, is_visible );
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
void ZUKAN_DETAIL_TOUCHBAR_SetDispOfGeneral(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   ZUKAN_DETAIL_TOUCHBAR_DISP  disp )
{
  if( work->type == ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL )
  {
    BOOL                       use_temp = FALSE;
    ZUKAN_DETAIL_TOUCHBAR_DISP temp;
   
    work->prev_disp = work->disp;
    work->disp = disp;

    // curr_icon_disp_generalを変えなくていいとき
    if( work->disp == work->curr_icon_disp_general )
    {
      use_temp = TRUE;
      temp     = work->curr_icon_disp_general;
      work->curr_icon_disp_general = ZUKAN_DETAIL_TOUCHBAR_DISP_CURR_ICON_NONE;  // 一時的に変更して、アニメを消されないようにしておく
    }

    // GENERALのパレットアニメ
    Zukan_Detail_Touchbar_AnimeExitGeneral( work );
    Zukan_Detail_Touchbar_AnimeInitGeneral( work );

    // curr_icon_disp_generalを変えなくていいとき
    if( use_temp )
    {
      work->curr_icon_disp_general = temp;  // 一時的に変更したものを、元に戻す
    }
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
void ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   BOOL                        is_visible )
{
  if( work->type == ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM )
  {
    ZKND_TBAR_SetVisible( work->tbwk, work->icon_set[FORM_ICON_CUR_R].cset->id, is_visible );
    ZKND_TBAR_SetVisible( work->tbwk, work->icon_set[FORM_ICON_CUR_L].cset->id, is_visible );
  }
}

//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in]  is_flip  TRUEならチェックを入れる、FALSEならチェックをはずす 
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZUKAN_DETAIL_TOUCHBAR_SetCheckFlipOfGeneral(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   BOOL                        is_flip )
{
  if( work->type == ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL )
  {
    ZKND_TBAR_SetFlip( work->tbwk, work->icon_set[GENERAL_ICON_CHECK].cset->id, is_flip );
  }
}
//------------------------------------------------------------------
/**
 *  @brief        
 *
 *  @param[in,out]   
 *
 *  @retval         TRUEならチェックが入っている、FALSEならチェックがはずれている
 */
//------------------------------------------------------------------
BOOL ZUKAN_DETAIL_TOUCHBAR_GetCheckFlipOfGeneral(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  if( work->type == ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL )
  {
    return ZKND_TBAR_GetFlip( work->tbwk, work->icon_set[GENERAL_ICON_CHECK].cset->id );
  }
  return FALSE;
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
// 見た目をアクティブ状態にしたまま、ユーザ指定の全体専用のアクティブフラグの切り替えを行う
void ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( ZUKAN_DETAIL_TOUCHBAR_WORK* work, BOOL is_active )  // TRUEのときアクティブ  // 切り替わったときのデフォルトはTRUE
{
  work->user_whole_is_active = is_active;

  if( work->user_whole_is_active )
  {
    if( !ZKND_TBAR_GetActiveWhole( work->tbwk ) )
    {
      if( work->state == ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
      {
        ZKND_TBAR_SetActiveWhole( work->tbwk, TRUE );
      }
    }
  }
  else
  {
    if( ZKND_TBAR_GetActiveWhole( work->tbwk ) )
    {
      ZKND_TBAR_SetActiveWhole( work->tbwk, FALSE );
    }
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
// カスタムボタンのパレットを読み込んだ場所を得る
u32 ZUKAN_DETAIL_TOUCHBAR_GetCustomIconPlttRegIdx( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  switch( work->type )
  {
  case ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL:
    {
      return work->general_ncl;
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_TYPE_MAP:
    {
      return work->map_ncl;
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM:
    {
      return work->form_ncl;
    }
    break;
  }
  return GFL_CLGRP_REGISTER_FAILED;  // 登録失敗
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
// BGプライオリティを設定する(OBJのBGプライオリティにもこの値が設定される)
void ZUKAN_DETAIL_TOUCHBAR_SetBgPriority( ZUKAN_DETAIL_TOUCHBAR_WORK* work, u8 pri )
{
  // BG
  //GFL_BG_SetPriority( ZKNDTL_BG_FRAME_M_TOUCHBAR, pri );
  // ここでBGのプライオリティ変更を行うと画面が乱れるので、VBlank中に行うことにする。 
  // OBJはここで変更しても、VBlankで変更してくれるようなので、ここで行ってよい。
  work->bg_pri = pri;
  work->req    = TRUE;

  // OBJ
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      GFL_CLACT_WK_SetBgPri( work->icon_set[i].clwk, pri );
    } 
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
static void Zukan_Detail_Touchbar_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK* work = (ZUKAN_DETAIL_TOUCHBAR_WORK*)wk;

  // BGプライオリティを設定する(OBJのBGプライオリティは既に変更されている)
  if( work->req )
  {
    GFL_BG_SetPriority( ZKNDTL_BG_FRAME_M_TOUCHBAR, work->bg_pri );
    work->req = FALSE;
  }
}

//-------------------------------------
/// タッチバー
//=====================================
static void Zukan_Detail_Touchbar_Create( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
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

  // ユーザ指定の全体専用のアクティブフラグ
  work->user_whole_is_active = TRUE;
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
    GFL_CLACTPOS pos;
    GFL_CLACT_WK_GetWldPos( work->icon_set[i].clwk, &pos );
    pos.y = work->icon_set[i].cset->pos.y;
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

  // ICON_SETにICON_CONST_SETを設定する
  work->icon_set_num = GENERAL_ICON_MAX;
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      work->icon_set[i].cset = &general_icon_const_set[i];
    } 
  }

  // タッチバーの設定
  {
    ZKND_TBAR_SETUP setup;
    
    // アイコンの設定
    // 数分作る
    ZKND_TBAR_ITEM_ICON icon_tbl[GENERAL_ICON_MAX];
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      icon_tbl[i].icon    = work->icon_set[i].cset->id;
      icon_tbl[i].pos     = work->icon_set[i].cset->pos;
      icon_tbl[i].width   = work->icon_set[i].cset->width;
    }
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
        icon_tbl[i].se                = SEQ_SE_SELECT3;            // 押したときにSEならしたいならば、SEの番号
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
        icon_tbl[i].se                = SEQ_SE_SELECT3;            // 押したときにSEならしたいならば、SEの番号
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
        icon_tbl[i].se                = SEQ_SE_SELECT3;            // 押したときにSEならしたいならば、SEの番号
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
        icon_tbl[i].se                = SEQ_SE_SELECT3;            // 押したときにSEならしたいならば、SEの番号
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
      work->icon_set[i].clwk = ZKND_TBAR_GetClwk( work->tbwk, work->icon_set[i].cset->id );
    } 
  }

  // フォルムありバージョンかフォルムなしバージョンか
  {
    if( work->form_version )
    {
      // このままでいい
    }
    else
    {
      u8 i;
      for( i=0; i<GENERAL_ICON_MAX; i++ )
      {
        ZKND_TBAR_SetPos( work->tbwk, work->icon_set[i].cset->id, &general_icon_const_pos_no_form[i] );
      }
      ZKND_TBAR_SetVisible( work->tbwk, work->icon_set[GENERAL_ICON_CUSTOM_FORM].cset->id, FALSE );
    }
  }

  // 現在表示しているところのアイコンを無反応にしておく
  {
    u8 general_icon = Zukan_Detail_Touchbar_DispToGeneralIcon( work->disp ); 
    if( general_icon != GENERAL_ICON_MAX )
      ZKND_TBAR_SetActive( work->tbwk, work->icon_set[general_icon].cset->id,  FALSE );
  }

  // GENERALのパレットアニメ
  Zukan_Detail_Touchbar_AnimeBaseInitGeneral( work );
  Zukan_Detail_Touchbar_AnimeInitGeneral( work );

  // 上下アイコンを表示するか否か
  if( !(work->is_cur_u_d) )
  {
    ZKND_TBAR_SetVisible( work->tbwk, work->icon_set[GENERAL_ICON_CUR_U].cset->id, FALSE );
    ZKND_TBAR_SetVisible( work->tbwk, work->icon_set[GENERAL_ICON_CUR_D].cset->id, FALSE );
  }
}
static void Zukan_Detail_Touchbar_DeleteGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  if( work->tbwk )
  {
    // GENERALのパレットアニメ
    Zukan_Detail_Touchbar_AnimeExitGeneral( work );
    Zukan_Detail_Touchbar_AnimeBaseExitGeneral( work );

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
static void Zukan_Detail_Touchbar_MainGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  u8  icon = GENERAL_ICON_MAX;
  int trg  = GFL_UI_KEY_GetTrg();
  int rept = GFL_UI_KEY_GetRepeat();

  if( rept & PAD_KEY_LEFT )
  {
    switch( work->disp )
    {
    case ZUKAN_DETAIL_TOUCHBAR_DISP_INFO:
      {
        if( ZKND_TBAR_GetVisible( work->tbwk, work->icon_set[GENERAL_ICON_CUSTOM_FORM].cset->id ) )
          icon = GENERAL_ICON_CUSTOM_FORM;
        else
          icon = GENERAL_ICON_CUSTOM_VOICE;
      }
      break;
    case ZUKAN_DETAIL_TOUCHBAR_DISP_MAP:
      {
        icon = GENERAL_ICON_CUSTOM_INFO;
      }
      break;
    case ZUKAN_DETAIL_TOUCHBAR_DISP_VOICE:
      {
        icon = GENERAL_ICON_CUSTOM_MAP;
      }
      break;
    case ZUKAN_DETAIL_TOUCHBAR_DISP_FORM:
      {
        icon = GENERAL_ICON_CUSTOM_VOICE;
      }
      break;
    }
  }
  else if( rept & PAD_KEY_RIGHT )
  {
    switch( work->disp )
    {
    case ZUKAN_DETAIL_TOUCHBAR_DISP_INFO:
      {
        icon = GENERAL_ICON_CUSTOM_MAP;
      }
      break;
    case ZUKAN_DETAIL_TOUCHBAR_DISP_MAP:
      {
        icon = GENERAL_ICON_CUSTOM_VOICE;
      }
      break;
    case ZUKAN_DETAIL_TOUCHBAR_DISP_VOICE:
      {
        if( ZKND_TBAR_GetVisible( work->tbwk, work->icon_set[GENERAL_ICON_CUSTOM_FORM].cset->id ) )
          icon = GENERAL_ICON_CUSTOM_FORM;
        else
          icon = GENERAL_ICON_CUSTOM_INFO;
      }
      break;
    case ZUKAN_DETAIL_TOUCHBAR_DISP_FORM:
      {
        icon = GENERAL_ICON_CUSTOM_INFO;
      }
      break;
    }
  }

  // アイコンをキーで押したことにする
  if( icon != GENERAL_ICON_MAX )
  {
    ZKND_TBAR_PushByKey( work->tbwk, work->icon_set[icon].cset->id );
    //return;  // こんなところでreturnしたら、ここより下にある「GENERALのパレットアニメ」の箇所を通らないフレームができてしまい、よくない！
  } 


  if( trg & ( PAD_KEY_UP | PAD_KEY_DOWN ) )
  {
    // 押したときは、ちゃんとタッチバーで判別するので、何もしない
  }
  else
  {
    if( rept & PAD_KEY_UP )
    {
      ZKND_TBAR_PushByKey( work->tbwk, work->icon_set[GENERAL_ICON_CUR_U].cset->id );
    }
    else if( rept & PAD_KEY_DOWN )
    {
      ZKND_TBAR_PushByKey( work->tbwk, work->icon_set[GENERAL_ICON_CUR_D].cset->id );
    }
  }


  // GENERALのパレットアニメ
  {
    ZKNDTL_COMMAND cmd = ZUKAN_DETAIL_TOUCHBAR_GetTrg( work );
    ZKNDTL_COMMAND scmd = ZUKAN_DETAIL_TOUCHBAR_GetTouch( work );

    if( cmd != ZKNDTL_CMD_NONE )
    {
      u8 general_icon = Zukan_Detail_Touchbar_CmdToGeneralIcon( cmd );
      u8 disp = Zukan_Detail_Touchbar_CmdToDisp( cmd );

      if(    general_icon != GENERAL_ICON_MAX
          && disp != ZUKAN_DETAIL_TOUCHBAR_DISP_CURR_ICON_NONE )
      {
        u8 i;
        for( i= GENERAL_ICON_CUSTOM_FORM; i<=GENERAL_ICON_CUSTOM_INFO; i++ )
        {
          ZKND_TBAR_SetActive( work->tbwk, work->icon_set[i].cset->id, ( i != general_icon ) );
        }
        work->curr_icon_disp_general = disp;
        Zukan_Detail_Touchbar_AnimeInitGeneral( work );
      }
    }

    if( scmd != ZKNDTL_CMD_NONE )
    {
      switch( scmd )
      {
      case ZKNDTL_SCMD_INFO:
      case ZKNDTL_SCMD_MAP:
      case ZKNDTL_SCMD_VOICE:
      case ZKNDTL_SCMD_FORM:
        {
          Zukan_Detail_Touchbar_AnimeExitGeneral( work );
        }
        break;
      }
    }
  }
}

// GENERALのパレットアニメ
static void Zukan_Detail_Touchbar_AnimeBaseInitGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  NNSG2dPaletteData* pal_data;
  void* buf = GFL_ARC_UTIL_LoadPalette( ARCID_ZUKAN_GRA, NARC_zukan_gra_info_info_obj_NCLR, &pal_data, work->heap_id );
  u16* raw_data = pal_data->pRawData;
  GFL_STD_MemCopy( &raw_data[RES_PAL_POS_GENERAL_ANIME_START*0x10], work->pal_anime_general_start, 0x20 );
  GFL_STD_MemCopy( &raw_data[RES_PAL_POS_GENERAL_ANIME_END*0x10], work->pal_anime_general_end, 0x20 );
  GFL_HEAP_FreeMemory( buf );
  work->pal_anime_general_count = 0;
}
static void Zukan_Detail_Touchbar_AnimeBaseExitGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  // 何もしない
}
static void Zukan_Detail_Touchbar_AnimeInitGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  u8 icon = GENERAL_ICON_MAX;
  GFL_CLWK* clwk;
  switch( work->curr_icon_disp_general )
  {
  case ZUKAN_DETAIL_TOUCHBAR_DISP_INFO:   icon = GENERAL_ICON_CUSTOM_INFO;    break;
  case ZUKAN_DETAIL_TOUCHBAR_DISP_MAP:    icon = GENERAL_ICON_CUSTOM_MAP;     break;
  case ZUKAN_DETAIL_TOUCHBAR_DISP_VOICE:  icon = GENERAL_ICON_CUSTOM_VOICE;   break;
  case ZUKAN_DETAIL_TOUCHBAR_DISP_FORM:   icon = GENERAL_ICON_CUSTOM_FORM;    break;
  }
  if( icon != GENERAL_ICON_MAX )
  {
    clwk = ZKND_TBAR_GetClwk( work->tbwk, work->icon_set[icon].cset->id );
    //GFL_CLACT_WK_SetPlttOffs( clwk, OBJ_PAL_OFFSET_GENERAL_ANIME_EXEC, CLWK_PLTTOFFS_MODE_PLTT_TOP );  // これでも同じようにパレットを変えられるのだが、↓つづく
    GFL_CLACT_WK_SetPlttOffs( clwk, OBJ_PAL_OFFSET_GENERAL_ANIME_EXEC, CLWK_PLTTOFFS_MODE_OAM_COLOR );
  }
}
static void Zukan_Detail_Touchbar_AnimeExitGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  u8 icon = GENERAL_ICON_MAX;
  GFL_CLWK* clwk;
  switch( work->curr_icon_disp_general )
  {
  case ZUKAN_DETAIL_TOUCHBAR_DISP_INFO:   icon = GENERAL_ICON_CUSTOM_INFO;    break;
  case ZUKAN_DETAIL_TOUCHBAR_DISP_MAP:    icon = GENERAL_ICON_CUSTOM_MAP;     break;
  case ZUKAN_DETAIL_TOUCHBAR_DISP_VOICE:  icon = GENERAL_ICON_CUSTOM_VOICE;   break;
  case ZUKAN_DETAIL_TOUCHBAR_DISP_FORM:   icon = GENERAL_ICON_CUSTOM_FORM;    break;
  }
  if( icon != GENERAL_ICON_MAX )
  {
    clwk = ZKND_TBAR_GetClwk( work->tbwk, work->icon_set[icon].cset->id );
    //GFL_CLACT_WK_SetPlttOffs( clwk, OBJ_PAL_OFFSET_GENERAL_ANIME_NONE, CLWK_PLTTOFFS_MODE_PLTT_TOP );  // つづき↑元に戻したらデザイナーさんのパレットアニメが発生しなくなってしまった。
    GFL_CLACT_WK_SetPlttOffs( clwk, OBJ_PAL_OFFSET_GENERAL_ANIME_NONE, CLWK_PLTTOFFS_MODE_OAM_COLOR );

    work->curr_icon_disp_general = ZUKAN_DETAIL_TOUCHBAR_DISP_CURR_ICON_NONE;
  }
}
static void Zukan_Detail_Touchbar_AnimeMainGeneral( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  u8 i;
  fx16 cos;

  if( work->pal_anime_general_count + GENERAL_ANIME_ADD >= 0x10000 )
  {
    work->pal_anime_general_count = work->pal_anime_general_count + GENERAL_ANIME_ADD - 0x10000;
  }
  else
  {
    work->pal_anime_general_count = work->pal_anime_general_count + GENERAL_ANIME_ADD;
  }
  cos = ( FX_CosIdx( work->pal_anime_general_count ) + FX16_ONE ) / 2;  // 0<= <=1にしておく

  for( i=0; i<0x10; i++ )
  {
    u8 s_r = ( work->pal_anime_general_start[i] & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    u8 s_g = ( work->pal_anime_general_start[i] & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    u8 s_b = ( work->pal_anime_general_start[i] & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;
    u8 e_r = ( work->pal_anime_general_end[i]   & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    u8 e_g = ( work->pal_anime_general_end[i]   & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    u8 e_b = ( work->pal_anime_general_end[i]   & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;

    u8 r = s_r + (((e_r-s_r)*cos)>>FX16_SHIFT);
    u8 g = s_g + (((e_g-s_g)*cos)>>FX16_SHIFT);
    u8 b = s_b + (((e_b-s_b)*cos)>>FX16_SHIFT);

    work->pal_anime_general_now[i] = GX_RGB( r, g, b );
  }

  if( ZKNDTL_BG_FRAME_M_TOUCHBAR < GFL_BG_FRAME0_S )
  {
    NNS_GfdRegisterNewVramTransferTask(
        NNS_GFD_DST_2D_OBJ_PLTT_MAIN,
        ( OBJ_PAL_POS_M_CUSTOM + OBJ_PAL_OFFSET_GENERAL_ANIME_EXEC ) * 0x20,
        work->pal_anime_general_now,
        0x20 );
  }
  else
  {
    NNS_GfdRegisterNewVramTransferTask(
        NNS_GFD_DST_2D_BG_PLTT_SUB ,
        ( OBJ_PAL_POS_M_CUSTOM + OBJ_PAL_OFFSET_GENERAL_ANIME_EXEC ) * 0x20,
        work->pal_anime_general_now,
        0x20 );
  }
}

// 変換
static u8 Zukan_Detail_Touchbar_DispToGeneralIcon( ZUKAN_DETAIL_TOUCHBAR_DISP disp )
{
  // 変換できないときGENERAL_ICON_MAXを返す
  switch( disp )
  {
  case ZUKAN_DETAIL_TOUCHBAR_DISP_INFO:
    {
      return GENERAL_ICON_CUSTOM_INFO;
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_DISP_MAP:
    {
      return GENERAL_ICON_CUSTOM_MAP;
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_DISP_VOICE:
    {
      return GENERAL_ICON_CUSTOM_VOICE;
    }
    break;
  case ZUKAN_DETAIL_TOUCHBAR_DISP_FORM:
    {
      return GENERAL_ICON_CUSTOM_FORM;
    }
    break;
  default:
    {
      return GENERAL_ICON_MAX;
    }
    break;
  }
}
static u8 Zukan_Detail_Touchbar_CmdToGeneralIcon( ZKNDTL_COMMAND cmd )
{
  // 変換できないときGENERAL_ICON_MAXを返す
  switch( cmd )
  {
  case ZKNDTL_CMD_INFO:
  case ZKNDTL_SCMD_INFO:
    {
      return GENERAL_ICON_CUSTOM_INFO;
    }
    break;
  case ZKNDTL_CMD_MAP:
  case ZKNDTL_SCMD_MAP:
    {
      return GENERAL_ICON_CUSTOM_MAP;
    }
    break;
  case ZKNDTL_CMD_VOICE:
  case ZKNDTL_SCMD_VOICE:
    {
      return GENERAL_ICON_CUSTOM_VOICE;
    }
    break;
  case ZKNDTL_CMD_FORM:
  case ZKNDTL_SCMD_FORM:
    {
      return GENERAL_ICON_CUSTOM_FORM;
    }
    break;
  default:
    {
      return GENERAL_ICON_MAX;
    }
    break;
  }
}
static u8 Zukan_Detail_Touchbar_CmdToDisp( ZKNDTL_COMMAND cmd )
{
  // 変換できないときZUKAN_DETAIL_TOUCHBAR_DISP_CURR_ICON_NONEを返す
  switch( cmd )
  {
  case ZKNDTL_CMD_INFO:
  case ZKNDTL_SCMD_INFO:
    {
      return ZUKAN_DETAIL_TOUCHBAR_DISP_INFO;
    }
    break;
  case ZKNDTL_CMD_MAP:
  case ZKNDTL_SCMD_MAP:
    {
      return ZUKAN_DETAIL_TOUCHBAR_DISP_MAP;
    }
    break;
  case ZKNDTL_CMD_VOICE:
  case ZKNDTL_SCMD_VOICE:
    {
      return ZUKAN_DETAIL_TOUCHBAR_DISP_VOICE;
    }
    break;
  case ZKNDTL_CMD_FORM:
  case ZKNDTL_SCMD_FORM:
    {
      return ZUKAN_DETAIL_TOUCHBAR_DISP_FORM;
    }
    break;
  default:
    {
      return ZUKAN_DETAIL_TOUCHBAR_DISP_CURR_ICON_NONE;
    }
    break;
  }
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

  // ICON_SETにICON_CONST_SETを設定する
  work->icon_set_num = MAP_ICON_MAX;
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      work->icon_set[i].cset = &map_icon_const_set[i];
    } 
  }

  // タッチバーの設定
  {
    ZKND_TBAR_SETUP setup;
 
    // アイコンの設定
    // 数分作る
    ZKND_TBAR_ITEM_ICON icon_tbl[MAP_ICON_MAX];
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      icon_tbl[i].icon    = work->icon_set[i].cset->id;
      icon_tbl[i].pos     = work->icon_set[i].cset->pos;
      icon_tbl[i].width   = work->icon_set[i].cset->width;
    }
#ifdef USE_MAP_DUMMY_ICON
    // 以下カスタムボタンならば入れなくてはいけない情報
    // カスタムボタン
    {
      u8 i;
      for( i=MAP_ICON_CUSTOM_DUMMY; i<=MAP_ICON_CUSTOM_DUMMY; i++ )
      {
        icon_tbl[i].cg_idx            = work->map_ncg;   // キャラリソース
        icon_tbl[i].plt_idx           = work->map_ncl;   // パレットリソース
        icon_tbl[i].cell_idx          = work->map_nce;   // セルリソース
        icon_tbl[i].active_anmseq     = 14;              // アクティブのときのアニメ
        icon_tbl[i].noactive_anmseq   = 14;              // ノンアクティブのときのアニメ
        icon_tbl[i].push_anmseq       = 20;              // 押したときのアニメ（STOPになっていること）
        icon_tbl[i].key               = 0;               // キーで押したときに動作させたいならば、ボタン番号
        icon_tbl[i].se                = 0;               // 押したときにSEならしたいならば、SEの番号
      }
    }
#endif

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
      work->icon_set[i].clwk = ZKND_TBAR_GetClwk( work->tbwk, work->icon_set[i].cset->id );
    } 
  }

#ifdef USE_MAP_DUMMY_ICON
  // ダミーのアイコンを非表示にする
  {
    ZKND_TBAR_SetVisible( work->tbwk, work->icon_set[MAP_ICON_CUSTOM_DUMMY].cset->id, FALSE );
  }
#endif
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
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, work->heap_id );
    work->form_ncl = GFL_CLGRP_PLTT_RegisterEx(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCLR,
                            draw_type,
                            OBJ_PAL_POS_M_CUSTOM * 0x20, 0, OBJ_PAL_NUM_M_CUSTOM,
                            work->heap_id );	
    work->form_ncg = GFL_CLGRP_CGR_Register(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCGR,
                            FALSE,
                            draw_type,
                            work->heap_id );
    work->form_nce = GFL_CLGRP_CELLANIM_Register(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCER,
                            NARC_zukan_gra_info_info_obj_NANR,
                            work->heap_id );
    GFL_ARC_CloseDataHandle( handle );
  }

  // ICON_SETにICON_CONST_SETを設定する
  work->icon_set_num = FORM_ICON_MAX;
  {
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      work->icon_set[i].cset = &form_icon_const_set[i];
    } 
  }

  // タッチバーの設定
  {
    ZKND_TBAR_SETUP setup;
    
    // アイコンの設定
    // 数分作る
    ZKND_TBAR_ITEM_ICON icon_tbl[FORM_ICON_MAX];
    u8 i;
    for( i=0; i<work->icon_set_num; i++ )
    {
      icon_tbl[i].icon    = work->icon_set[i].cset->id;
      icon_tbl[i].pos     = work->icon_set[i].cset->pos;
      icon_tbl[i].width   = work->icon_set[i].cset->width;
    }
    // 以下カスタムボタンならば入れなくてはいけない情報
    // カスタムボタン
    {
      u8 i;
      for( i=FORM_ICON_CUSTOM_EXCHANGE; i<=FORM_ICON_CUSTOM_EXCHANGE; i++ )
      {
        icon_tbl[i].cg_idx            = work->form_ncg;         // キャラリソース
        icon_tbl[i].plt_idx           = work->form_ncl;         // パレットリソース
        icon_tbl[i].cell_idx          = work->form_nce;         // セルリソース
        icon_tbl[i].active_anmseq     = 14;                         // アクティブのときのアニメ
        icon_tbl[i].noactive_anmseq   = 14;                        // ノンアクティブのときのアニメ
        icon_tbl[i].push_anmseq       = 20;                        // 押したときのアニメ（STOPになっていること）
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
      work->icon_set[i].clwk = ZKND_TBAR_GetClwk( work->tbwk, work->icon_set[i].cset->id );
    } 
  }

  // Key設定を追加する
  {
    ZKND_TBAR_SetUseKey( work->tbwk, work->icon_set[FORM_ICON_CUR_R].cset->id, ( PAD_KEY_RIGHT | PAD_BUTTON_R ) );
    ZKND_TBAR_SetUseKey( work->tbwk, work->icon_set[FORM_ICON_CUR_L].cset->id, ( PAD_KEY_LEFT  | PAD_BUTTON_L ) );
  }

  // 上下アイコンを表示するか否か
  if( !(work->is_cur_u_d) )
  {
    ZKND_TBAR_SetVisible( work->tbwk, work->icon_set[FORM_ICON_CUR_U].cset->id, FALSE );
    ZKND_TBAR_SetVisible( work->tbwk, work->icon_set[FORM_ICON_CUR_D].cset->id, FALSE );
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
static void Zukan_Detail_Touchbar_MainForm( ZUKAN_DETAIL_TOUCHBAR_WORK* work )
{
  int trg  = GFL_UI_KEY_GetTrg();
  int rept = GFL_UI_KEY_GetRepeat();

  if( trg & ( PAD_KEY_UP | PAD_KEY_DOWN | PAD_KEY_LEFT | PAD_KEY_RIGHT | PAD_BUTTON_L | PAD_BUTTON_R ) )
  {
    // 押したときは、ちゃんとタッチバーで判別するので、何もしない
  }
  else
  {
    if( rept & PAD_KEY_UP )
    {
      ZKND_TBAR_PushByKey( work->tbwk, work->icon_set[FORM_ICON_CUR_U].cset->id );
    }
    else if( rept & PAD_KEY_DOWN )
    {
      ZKND_TBAR_PushByKey( work->tbwk, work->icon_set[FORM_ICON_CUR_D].cset->id );
    }
    else if( rept & ( PAD_KEY_LEFT | PAD_BUTTON_L ) )
    {
      ZKND_TBAR_PushByKey( work->tbwk, work->icon_set[FORM_ICON_CUR_L].cset->id );
    }
    else if( rept & ( PAD_KEY_RIGHT | PAD_BUTTON_R ) )
    {
      ZKND_TBAR_PushByKey( work->tbwk, work->icon_set[FORM_ICON_CUR_R].cset->id );
    }
  }
}

