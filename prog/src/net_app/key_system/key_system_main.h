//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_main.c
 *	@brief  キーシステムメインソース
 *	@author	Toru=Nagihashi
 *	@data		2011.05.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "gamesystem/game_data.h"
#include "system/bmp_winframe.h"
#include "print/wordset.h"
#include "field/zonedata.h"

//アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "arc/key_system_gra.naix"
#include "msg/msg_key_system.h"
#include "message.naix"

//自分のモジュール
#include "key_system_util.h"
#include "key_system_graphic.h"
#include "key_system_net.h"

//セーブ
#include "savedata/save_outside.h"
#include "savedata/outside_permanent.h"

//デバッグ
#include "test/nagihashi/debugwin_keysystem.h"

//外部公開
#include "net_app/key_system.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//BG面
enum
{
  KEYSYS_BG_FRAME_TEXT_M = GFL_BG_FRAME0_M,
  KEYSYS_BG_FRAME_BACK_M = GFL_BG_FRAME3_M,
  KEYSYS_BG_FRAME_TEXT_S = GFL_BG_FRAME0_S,
  KEYSYS_BG_FRAME_BACK_S = GFL_BG_FRAME3_S,
};

//パレット使用場所
enum
{
  KEYSYS_PLT_BG_BACK_M    = 0,
  KEYSYS_PLT_BG_MENU_NORMAL_M,
  KEYSYS_PLT_BG_MENU_SELECT_M,
  KEYSYS_PLT_BG_FONT_M    = 0xE,
  KEYSYS_PLT_BG_FRAME_M   = 0xF,

  KEYSYS_PLT_BG_BACK_S    = 0,
  KEYSYS_PLT_BG_FONT_S    = 0xE,

  KEYSYS_PLT_OBJ_COMMON_M = 0,
  KEYSYS_PLT_OBJ_BARICON_M = 1,
};

//キャラ管理
enum
{
  //BG_FRAME_TEXT_Mのキャラ
  KEYSYS_CGR_CLEAR_M,
  KEYSYS_CGR_TEXT_M,
  KEYSYS_CGR_MENU_M = KEYSYS_CGR_TEXT_M + 9,
};

//OBJリソース管理
enum
{
  KEYSYS_RES_OBJ_NCL,
  KEYSYS_RES_OBJ_NCG,
  KEYSYS_RES_OBJ_NCE,
  KEYSYS_RES_MAX,
};

//CLWKの種類
typedef enum
{
  KEYSYS_CLWK_WAVE_D,
  KEYSYS_CLWK_WAVE_U,
  KEYSYS_CLWK_DS_D,
  KEYSYS_CLWK_DS_U,
  KEYSYS_CLWK_LOCK_00,
  KEYSYS_CLWK_LOCK_01,
  KEYSYS_CLWK_LOCK_02,
  KEYSYS_CLWK_MARK,
  KEYSYS_CLWK_KEY,
  KEYSYS_CLWK_MAX,
}KEYSYS_CLWK;

enum
{
  KEYSYS_DATA_SELF,
  KEYSYS_DATA_OTHER,
  KEYSYS_DATA_MAX
};

enum
{
  KEYSYS_SAVE_MSG_SYNC  = 60,
};

typedef enum
{
  KEYSYS_SELECT_MENU_RET_NONE,
  KEYSYS_SELECT_MENU_RET_SELECTED,  //現在選択中
  KEYSYS_SELECT_MENU_RET_UNSELECT,  //選択できない
  KEYSYS_SELECT_MENU_RET_DECIDE,  //決定
  KEYSYS_SELECT_MENU_RET_SP_UNSELECT,  //特殊な状態で選択できない
} KEYSYS_SELECT_MENU_RET;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	BGワーク
//=====================================
typedef struct
{
  fx32 scroll_now;
} KEYSYS_BG_WORK;

//-------------------------------------
///	OBJワーク
//=====================================
typedef struct
{
  u32 res[KEYSYS_RES_MAX];
  GFL_CLWK *p_clwk[KEYSYS_CLWK_MAX];
  GFL_CLUNIT *p_clunit;
} KEYSYS_OBJ_WORK;

//-------------------------------------
///	スタック
//=====================================
#define KEYSYS_STACK_MAX (5)
typedef struct
{
  u32 pos;
  u32 stack[KEYSYS_STACK_MAX];
} KEYSYS_STACK;

//-------------------------------------
///	選択メニュー
//=====================================
typedef struct _KEYSYS_SELECT_MENU
{
  GAMEDATA *p_gamedata;
  KEYSYS_MENU_WORK        **pp_menu;
  KEYSYS_OBJ_WORK         *p_obj;
  KEY_SYSTEM_SAVE         *p_key_sv;

  void (*Init)( struct _KEYSYS_SELECT_MENU *p_wk, KEYSYS_MENU_PARAM *p_param );
  KEYSYS_SELECT_MENU_RET (*DecideCallBack)( int index, void *p_wk_adrs );

  KEYSYS_SELECT_MENU_RET unselected;
  int unselected_index;

} KEYSYS_SELECT_MENU;

//-------------------------------------
///	キー情報
//=====================================
typedef struct
{
  BOOL  is_have[ KEY_SYSTEM_TYPE_MAX ]; 
  BOOL  is_use[ KEY_SYSTEM_TYPE_MAX ];
  u32   version;
} KEYSYS_KEY_DATA;

//-------------------------------------
///	プレイヤー情報
//=====================================
typedef struct
{
  BOOL is_save_exist;
  u8   mystatus_buff[MYSTATUS_SAVE_SIZE];
} KEYSYS_PLAYER_DATA;

//-------------------------------------
///	デモワーク
//=====================================
typedef struct
{
  int dummy;
} KEYSYS_DEMO_WORK;

//-------------------------------------
///	メインワーク
//=====================================
typedef struct
{
  KEYSYS_DISPLAY_GENELATER *p_display;

  KEYSYS_SEQ_WORK         *p_seq;

  KEY_SYSTEM_GRAPHIC_WORK *p_graphic;

  KEY_SYSTEM_PARAM        *p_param;

  KEYSYS_TEXT_WORK        *p_text;

  KEYSYS_TEXT_WORK        *p_title;

  KEYSYS_MENU_WORK        *p_menu;

  KEYSYS_LIST_WORK        *p_list;

  KEYSYS_TEXTSET_WORK     *p_textset;

  GFL_FONT                *p_font;

  PRINT_QUE               *p_que;

  GFL_MSGDATA             *p_msg;

  WORDSET                 *p_word;

  KEYSYS_BG_WORK          bg;

  KEYSYS_OBJ_WORK         obj;

  KEYSYS_STACK            select;

  KEYSYS_SELECT_MENU      selmenu;

  OUTSIDE_SAVE_CONTROL    *p_outside_sv;

  KEY_SYSTEM_SAVE         *p_key_sv;

  KEYSYS_NET_WORK         *p_net;

  KEYSYS_KEY_DATA         key_data[ KEYSYS_DATA_MAX ];

  KEYSYS_PLAYER_DATA      player_data[ KEYSYS_DATA_MAX ];

  KEYSYS_DEMO_WORK        demo;

  u32                     ret;  //選択項目一時保存場所

  u32                     cnt;

  STRBUF                  *p_strbuf;

  BOOL                    is_parent;

}KEY_SYSTEM_WORK;

//-------------------------------------
/// SEQ
//=====================================
extern void KeySys_Seq_FadeInBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
extern void KeySys_Seq_FadeOutBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
/// BG
//=====================================
extern void KEYSYS_BG_Load( KEYSYS_BG_WORK *p_wk, HEAPID heapID );
extern void KEYSYS_BG_Main( KEYSYS_BG_WORK *p_wk );

//-------------------------------------
/// OBJ
//=====================================
extern void KEYSYS_OBJ_Init( KEYSYS_OBJ_WORK *p_wk, const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic, HEAPID heapID );
extern void KEYSYS_OBJ_Load( KEYSYS_OBJ_WORK *p_wk, HEAPID heapID );
extern void KEYSYS_OBJ_Release( KEYSYS_OBJ_WORK *p_wk );
extern GFL_CLWK * KEYSYS_OBJ_AddClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id, HEAPID heapID );
extern void KEYSYS_OBJ_RemoveClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id );
extern GFL_CLWK * KEYSYS_OBJ_GetClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id );

//-------------------------------------
/// STACK
//=====================================
extern void KEYSYS_STACK_Init( KEYSYS_STACK *p_wk, HEAPID heapID );
extern void KEYSYS_STACK_Push( KEYSYS_STACK *p_wk, u32 num );
extern u32 KEYSYS_STACK_Pop( KEYSYS_STACK *p_wk );
extern u32 KEYSYS_STACK_GetNow( const KEYSYS_STACK *cp_wk );
extern u32 KEYSYS_STACK_GetStackNum( const KEYSYS_STACK *cp_wk );
