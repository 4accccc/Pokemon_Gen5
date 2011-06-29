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

FS_EXTERN_OVERLAY( outside_save );

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

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// プロセス
//=====================================
static GFL_PROC_RESULT Key_System_Proc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Key_System_Proc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Key_System_Proc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
/// シーケンス
//=====================================
//メインシーケンス
static void KeySys_Seq_Get_Main( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_App_Main( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//サブシーケンス
static void KeySys_Seq_App_MainMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_App_Irc( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_App_LockList( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_App_Difficulty( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_App_Wfbc( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_Both_RecvDemo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_App_SendDemo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//フェードシーケンス
static void KeySys_Seq_FadeInBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_FadeOutBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//画面構築シーケンス
static void KeySys_Seq_CreateDisplayMainMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_CreateDisplayIrc( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_CreateDisplayLockList( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_CreateDisplayDifficulty( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_CreateDisplayWfbc( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_CreateDisplayRecvDemo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_CreateDisplaySendDemo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Seq_DeleteDisplay( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
/// 画面作成・破棄
//=====================================
//メインメニュー
static void KeySys_Display_CreateMainMenu( void *p_wk_adrs, HEAPID heapID );
static BOOL KeySys_Display_IsEndCreateMainMenu( void *p_wk_adrs );
static void KeySys_Display_DeleteMainMenu( void *p_wk_adrs );
//通信中
static void KeySys_Display_CreateIrc( void *p_wk_adrs, HEAPID heapID );
static BOOL KeySys_Display_IsEndCreateIrc( void *p_wk_adrs );
static void KeySys_Display_DeleteIrc( void *p_wk_adrs );
//ロックリスト
static void KeySys_Display_CreateLockList( void *p_wk_adrs, HEAPID heapID );
static BOOL KeySys_Display_IsEndCreateLockList( void *p_wk_adrs );
static void KeySys_Display_DeleteLockList( void *p_wk_adrs );
//なんいど
static void KeySys_Display_CreateDifficulty( void *p_wk_adrs, HEAPID heapID );
static BOOL KeySys_Display_IsEndCreateDifficulty( void *p_wk_adrs );
static void KeySys_Display_DeleteDifficulty( void *p_wk_adrs );
//まちへんこう
static void KeySys_Display_CreateWfbc( void *p_wk_adrs, HEAPID heapID );
static BOOL KeySys_Display_IsEndCreateWfbc( void *p_wk_adrs );
static void KeySys_Display_DeleteWfbc( void *p_wk_adrs );
//貰ったデモ
static void KeySys_Display_CreateRecvDemo( void *p_wk_adrs, HEAPID heapID );
static BOOL KeySys_Display_IsEndCreateRecvDemo( void *p_wk_adrs );
static void KeySys_Display_DeleteRecvDemo( void *p_wk_adrs );
//送り中デモ
static void KeySys_Display_CreateSendDemo( void *p_wk_adrs, HEAPID heapID );
static BOOL KeySys_Display_IsEndCreateSendDemo( void *p_wk_adrs );
static void KeySys_Display_DeleteSendDemo( void *p_wk_adrs );

//-------------------------------------
/// BG
//=====================================
static void KeySys_Bg_Load( KEYSYS_BG_WORK *p_wk, HEAPID heapID );
static void KeySys_Bg_Main( KEYSYS_BG_WORK *p_wk );

//-------------------------------------
/// OBJ
//=====================================
static void KeySys_Obj_Init( KEYSYS_OBJ_WORK *p_wk, const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic, HEAPID heapID );
static void KeySys_Obj_Load( KEYSYS_OBJ_WORK *p_wk, HEAPID heapID );
static void KeySys_Obj_Release( KEYSYS_OBJ_WORK *p_wk );
static GFL_CLWK * KeySys_Obj_AddClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id, HEAPID heapID );
static void KeySys_Obj_RemoveClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id );
static GFL_CLWK * KeySys_Obj_GetClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id );

//-------------------------------------
/// STACK
//=====================================
static void KeySys_Stack_Init( KEYSYS_STACK *p_wk, HEAPID heapID );
static void KeySys_Stack_Push( KEYSYS_STACK *p_wk, u32 num );
static u32 KeySys_Stack_Pop( KEYSYS_STACK *p_wk );
static u32 KeySys_Stack_GetNow( const KEYSYS_STACK *cp_wk );
static u32 KeySys_Stack_GetStackNum( const KEYSYS_STACK *cp_wk );

//-------------------------------------
///	キー格納情報
//=====================================
static void KeySys_KeyData_Init( KEYSYS_KEY_DATA *p_wk, const KEY_SYSTEM_SAVE *cp_sv );
static void KeySys_KeyData_SetSave( const KEYSYS_KEY_DATA *cp_wk, KEY_SYSTEM_SAVE *p_sv );
static BOOL KeySys_KeyData_IsGet( const KEYSYS_KEY_DATA *cp_self, const KEYSYS_KEY_DATA *cp_other,  BOOL recv_or_send );
static void KeySys_KeyData_Merge( KEYSYS_KEY_DATA *p_self, const KEYSYS_KEY_DATA *cp_other );

//-------------------------------------
///	プレイヤーデータ
//=====================================
static void KeySys_PlayerData_Init( KEYSYS_PLAYER_DATA *p_wk, GAMEDATA *p_gamedata );

//-------------------------------------
/// 選択メニュー
//=====================================
typedef enum
{
  KEYSYS_SELMENU_TYPE_DIFFICULTY,
  KEYSYS_SELMENU_TYPE_WFBC,
}KEYSYS_SELMENU_TYPE;
static void KeySys_SelMenu_Init( KEYSYS_SELECT_MENU *p_wk, KEYSYS_SELMENU_TYPE type, KEYSYS_OBJ_WORK *p_obj, KEYSYS_MENU_WORK **pp_menu, GFL_MSGDATA *p_msg, GFL_FONT *p_font, KEY_SYSTEM_SAVE *p_sv, GAMEDATA *p_gamedata, HEAPID heapID );
static void KeySys_SelMenu_Exit( KEYSYS_SELECT_MENU *p_wk );
static void KeySys_SelMenu_Main( KEYSYS_SELECT_MENU *p_wk );
static BOOL KeySys_SelMenu_IsDrawEnd( const KEYSYS_SELECT_MENU *cp_wk );
static s32 KeySys_SelMenu_GetSelect( const KEYSYS_SELECT_MENU *cp_wk );
static BOOL KeySys_SelMenu_IsDecide( const KEYSYS_SELECT_MENU *cp_wk );
static void KeySys_SelMenu_ReStart( KEYSYS_SELECT_MENU *p_wk );
static KEYSYS_SELECT_MENU_RET KeySys_SelMenu_GetUnSelected( const KEYSYS_SELECT_MENU *cp_wk );
static s32  KeySys_SelMenu_GetUnSelectedIndex( const KEYSYS_SELECT_MENU *cp_wk );
static BOOL KeySys_SelMenu_DecideCallBack( int index, void *p_wk_adrs );

static void KeySys_SelMenu_Difficulty_Init( KEYSYS_SELECT_MENU *p_wk, KEYSYS_MENU_PARAM *p_param );
static KEYSYS_SELECT_MENU_RET KeySys_SelMenu_Difficulty_DecideCallBack( int index, void *p_wk_adrs );
static void KeySys_SelMenu_Wfbc_Init( KEYSYS_SELECT_MENU *p_wk, KEYSYS_MENU_PARAM *p_param );
static KEYSYS_SELECT_MENU_RET KeySys_SelMenu_Wfbc_DecideCallBack( int index, void *p_wk_adrs );

//-------------------------------------
///	デモ
//=====================================
static void KeySys_Demo_Init( KEYSYS_DEMO_WORK *p_wk );

//-------------------------------------
/// メッセージ取得
//=====================================
static u16 KeySys_Mes_GetDifficultInfoMessage( const KEY_SYSTEM_SAVE *cp_save );
static u16 KeySys_Mes_GetWfbcInfoMessage( const KEY_SYSTEM_SAVE *cp_save );
static u16 KeySys_Mes_GetLockListInfoMessage( const KEY_SYSTEM_SAVE *cp_save, u16 cursor );
static BOOL KeySys_Mes_CreateKeyMessage( KEY_SYSTEM_WORK *p_wk, KEY_SYSTEM_TYPE index, BOOL recv_or_send );

//-------------------------------------
/// その他
//=====================================
static void KeySys_NetCallBack_IrcEnd( void *p_wk_adrs );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
const GFL_PROC_DATA KEY_SYSTEM_ProcData = 
{
  Key_System_Proc_Init,
  Key_System_Proc_Main,
  Key_System_Proc_Exit,
};
//-------------------------------------
/// ディスプレイ作成テーブル
//=====================================
enum
{
  KEYSYS_DISPLAY_APP_MAIN_MENU,
  KEYSYS_DISPLAY_APP_IRC,
  KEYSYS_DISPLAY_APP_LOCK_LIST,
  KEYSYS_DISPLAY_APP_DIFFICULTY,
  KEYSYS_DISPLAY_APP_WFBC,
  KEYSYS_DISPLAY_BOTH_RECV_DEMO,
  KEYSYS_DISPLAY_APP_SEND_DEMO,
  KEYSYS_DISPLAY_MAX
};
static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl[KEYSYS_DISPLAY_MAX] =
{
  //KEYSYS_DISPLAY_APP_MAIN_MENU
  {
    KeySys_Display_CreateMainMenu,
    KeySys_Display_IsEndCreateMainMenu,
    KeySys_Display_DeleteMainMenu,
    NULL,
  },
  //KEYSYS_DISPLAY_APP_IRC
  {
    KeySys_Display_CreateIrc,
    KeySys_Display_IsEndCreateIrc,
    KeySys_Display_DeleteIrc,
    NULL,
  },
  //KEYSYS_DISPLAY_APP_LOCK_LIST
  {
    KeySys_Display_CreateLockList,
    KeySys_Display_IsEndCreateLockList,
    KeySys_Display_DeleteLockList,
    NULL,
  },
  //KEYSYS_DISPLAY_APP_DIFFICULTY,
  {
    KeySys_Display_CreateDifficulty,
    KeySys_Display_IsEndCreateDifficulty,
    KeySys_Display_DeleteDifficulty,
    NULL,
  },
  //KEYSYS_DISPLAY_APP_WFBC,
  {
    KeySys_Display_CreateWfbc,
    KeySys_Display_IsEndCreateWfbc,
    KeySys_Display_DeleteWfbc,
    NULL,
  },
  //KEYSYS_DISPLAY_BOTH_RECV_DEMO,
  {
    KeySys_Display_CreateRecvDemo,
    KeySys_Display_IsEndCreateRecvDemo,
    KeySys_Display_DeleteRecvDemo,
    NULL,
  },
  //KEYSYS_DISPLAY_APP_SEND_DEMO
  {
    KeySys_Display_CreateSendDemo,
    KeySys_Display_IsEndCreateSendDemo,
    KeySys_Display_DeleteSendDemo,
    NULL,
  },
};
//=============================================================================
/**
 *					プロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Key_System_Proc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(outside_save) );

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_KEY_SYSTEM, 0x40000 );
  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(KEY_SYSTEM_WORK), HEAPID_KEY_SYSTEM );
  GFL_STD_MemClear( p_wk, sizeof(KEY_SYSTEM_WORK) );
  p_wk->p_param = p_param_adrs;

  //もし引数がNULLならばタイトルから呼び出された＝APPモード
  if( p_param_adrs == NULL )
  {
    p_wk->p_param = GFL_HEAP_AllocClearMemory(HEAPID_KEY_SYSTEM,sizeof(KEY_SYSTEM_PARAM) );
    p_wk->p_param->mode = KEY_SYSTEM_MODE_APP;
    p_wk->p_param->p_gamedata    = GAMEDATA_Create( HEAPID_KEY_SYSTEM );
    ZONEDATA_Open( HEAPID_KEY_SYSTEM );
  }

  //管理外セーブシステム
  p_wk->p_outside_sv = OutsideSave_SystemLoad( HEAPID_KEY_SYSTEM );
  {
    OUTSIDE_PERMANENT *p_per = OutsideSave_GetPermanentPtr( p_wk->p_outside_sv );
    p_wk->p_key_sv = OUTSIDE_PERMANENT_GetKeySystem( p_per );
  }

  //自分の情報を設定
  KeySys_KeyData_Init( &p_wk->key_data[KEYSYS_DATA_SELF], p_wk->p_key_sv );
  KeySys_PlayerData_Init( &p_wk->player_data[KEYSYS_DATA_SELF], p_wk->p_param->p_gamedata );


  //グラフィック初期化
  p_wk->p_graphic = KEY_SYSTEM_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_KEY_SYSTEM );

  //ネット初期化
  {
    KEYSYS_NET_INIT_PARAM param;
    param.net_type = KEYSYS_NET_TYPE_IRCWIRELESS;
    param.p_gamedata = p_wk->p_param->p_gamedata;
    p_wk->p_net = KEYSYS_NET_Init( &param, HEAPID_KEY_SYSTEM );
  }

  //共通モジュールの初期化
  p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_KEY_SYSTEM );
  p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_KEY_SYSTEM );
  p_wk->p_msg		  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_key_system_dat, HEAPID_KEY_SYSTEM );
  p_wk->p_word    = WORDSET_Create( HEAPID_KEY_SYSTEM );
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 128, HEAPID_KEY_SYSTEM );

  //BG
  KeySys_Bg_Load( &p_wk->bg, HEAPID_KEY_SYSTEM );

  //OBJ
  KeySys_Obj_Init( &p_wk->obj, p_wk->p_graphic, HEAPID_KEY_SYSTEM );
  KeySys_Obj_Load( &p_wk->obj, HEAPID_KEY_SYSTEM );

  //選択履歴
  KeySys_Stack_Init( &p_wk->select, HEAPID_KEY_SYSTEM );

  //シーケンス
  {
    //初回シーケンス
    static const KEYSYS_SEQ_FUNCTION sc_init_seq[KEY_SYSTEM_MODE_MAX]  =
    {
      KeySys_Seq_Get_Main,//KEY_SYSTEM_MODE_GET
      KeySys_Seq_App_Main,//KEY_SYSTEM_MODE_APP
    };
    GF_ASSERT( p_wk->p_param->mode < KEY_SYSTEM_MODE_MAX );
    p_wk->p_seq = KEYSYS_SEQ_Init( 5, p_wk, sc_init_seq[p_wk->p_param->mode], HEAPID_KEY_SYSTEM );
  }

  //ディスプレイ作成
  p_wk->p_display = KEYSYS_DISPLAY_Init( p_wk, HEAPID_KEY_SYSTEM );
 
  //上画面、下部テキスト作成
  p_wk->p_text = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 1, 19, 30, 4, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, HEAPID_KEY_SYSTEM );
  KEYSYS_TEXT_SetColor( p_wk->p_text, PRINTSYS_LSB_Make( 1, 2, 0xF ) );
  KEYSYS_TEXT_WriteWindowFrame( p_wk->p_text, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M );

  //上画面タイトル作成
  p_wk->p_title = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 1,0,14, 2, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, HEAPID_KEY_SYSTEM );
  KEYSYS_TEXT_SetColor( p_wk->p_title, PRINTSYS_LSB_Make( 1, 2, 0 ) );

#ifdef PM_DEBUG
  //デバッグウィンドウ
  DEBUGWIN_InitProc( KEYSYS_BG_FRAME_TEXT_M, p_wk->p_font );
  DEBUGWIN_KEY_SYSTEM_Init( p_wk->p_key_sv, HEAPID_KEY_SYSTEM );
#endif //PM_DEBUG

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Key_System_Proc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

#ifdef PM_DEBUG
  //デバッグウィンドウ
  DEBUGWIN_KEY_SYSTEM_Exit();
  DEBUGWIN_ExitProc();
#endif //PM_DEBUG

  //テキスト破棄
  if( p_wk->p_title )
  {
    KEYSYS_TEXT_Exit( p_wk->p_title );
    p_wk->p_title = NULL;
  }

  //テキスト破棄
  if( p_wk->p_text )
  {
    KEYSYS_TEXT_Exit( p_wk->p_text );
    p_wk->p_text = NULL;
  }

  //ディスプレイ
  KEYSYS_DISPLAY_Exit( p_wk->p_display );

  //シーケンス破棄
  KEYSYS_SEQ_Exit( p_wk->p_seq );

  //OBJ破棄
  KeySys_Obj_Release( &p_wk->obj );

  //共通モジュール破棄
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  WORDSET_Delete( p_wk->p_word );
	GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

  //通信処理破棄
  KEYSYS_NET_Exit( p_wk->p_net );

  //グラフィック破棄
  KEY_SYSTEM_GRAPHIC_Exit( p_wk->p_graphic );

  //管理外セーブ
  OutsideSave_SystemUnload( p_wk->p_outside_sv );

  //もし引数がNULLならばタイトルから呼び出された＝APPモード
  if( p_param_adrs == NULL )
  {
    ZONEDATA_Close();
    GAMEDATA_Delete( p_wk->p_param->p_gamedata );
    GFL_HEAP_FreeMemory( p_wk->p_param );

    //タイトルに戻る
    OS_ResetSystem(0);
  }

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_KEY_SYSTEM );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(outside_save) );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Key_System_Proc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //通信エラー処理
  {
    KEYSYS_NET_ERROR_TYPE err = KEYSYS_NET_HandleError( p_wk->p_net );
    switch( err )
    {
    case KEYSYS_NET_ERROR_TYPE_NONE:        //エラーが起こっていません。
      /*  なにもしない  */
      break;
    case KEYSYS_NET_ERROR_TYPE_RETURN:      //再度通信をすればつながります。
      KEYSYS_NET_ClearError( p_wk->p_net );
      return GFL_PROC_RES_CONTINUE;
    default:
      GF_ASSERT(0);
    case KEYSYS_NET_ERROR_TYPE_DISCONNECT:  //アプリ終了orWi-Fiから切断してください。
      return GFL_PROC_RES_FINISH;
    }
  }

  //シーケンスメイン
  KEYSYS_SEQ_Main( p_wk->p_seq );

  //グラフィック描画
  KEY_SYSTEM_GRAPHIC_2D_Draw( p_wk->p_graphic );

  //画面作成・破棄メイン
  KEYSYS_DISPLAY_Main( p_wk->p_display );

  //BG処理メイン
  KeySys_Bg_Main( &p_wk->bg );

  //キューメイン
  PRINTSYS_QUE_Main( p_wk->p_que );

  //タイトル描画
  KEYSYS_TEXT_Draw( p_wk->p_title );

  //テキスト描画
  if( p_wk->p_text )
  {
    KEYSYS_TEXT_Draw( p_wk->p_text );
  }

  //テキストセット描画
  if( p_wk->p_textset )
  {
    KEYSYS_TEXTSET_Draw( p_wk->p_textset );
  }

  //メニュー描画
  if( p_wk->p_menu )
  {
    KEYSYS_MENU_Draw( p_wk->p_menu );
  }

  //リスト描画
  if( p_wk->p_list )
  {
    KEYSYS_LIST_Draw( p_wk->p_list );
  }

  //通信処理
  KEYSYS_NET_Main( p_wk->p_net );

  //終了チェック
  if( KEYSYS_SEQ_IsEnd( p_wk->p_seq ) )
  {
    return GFL_PROC_RES_FINISH;
  }
  else
  {
    return GFL_PROC_RES_CONTINUE;
  }

}

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  取得モード  メイン処理
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_Get_Main( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_FADEIN,
    SEQ_MAIN,
    SEQ_FADEOUT,
    SEQ_END,
  };

  switch( *p_seq )
  {
  case SEQ_FADEIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeInBlack );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_FADEOUT:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeOutBlack );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_End( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アプリモード  メイン処理
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_App_Main( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_FADEIN,
    SEQ_MAIN,
    SEQ_FADEOUT,
    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    //最初はメインメニュー作成
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplayMainMenu );
    (*p_seq)++;
    break;
  case SEQ_FADEIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeInBlack );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_App_MainMenu );
    (*p_seq)++;
    break;
  case SEQ_FADEOUT:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeOutBlack );
    (*p_seq)++;
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_End( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アプリモード  メインメニュー
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_App_MainMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_MAIN,
    SEQ_DISPLAY_DELETE,
    SEQ_SELECT_CALL,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    //最初はメインメニュー作成
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplayMainMenu );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    KEYSYS_MENU_Main( p_wk->p_menu );

    if( KEYSYS_MENU_IsChangeSelect( p_wk->p_menu ) )
    {
      if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
      {
        KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_19+KEYSYS_MENU_GetSelect( p_wk->p_menu ), KEYSYS_TEXT_TYPE_QUE ); 
      }
    }

    if( KEYSYS_MENU_IsDecide( p_wk->p_menu ) )
    {
      p_wk->ret = KEYSYS_MENU_GetSelect( p_wk->p_menu );

      if( p_wk->ret == 2 )//もどるのときはFADE後まで画面を残すので消さない
      {
        *p_seq = SEQ_END;
      }
      else
      {
        (*p_seq)++;
      }
    }
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;

  case SEQ_SELECT_CALL:
    switch( p_wk->ret )
    {
    case 0://キーの通信
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_App_Irc );
      break;
    case 1://ロックリスト
      KeySys_Stack_Push( &p_wk->select, p_wk->ret );
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_App_LockList );
      break;
    default:
      GF_ASSERT(0);
    }

    //もう一度メインメニュー
    (*p_seq ) = 0;
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アプリモード  通信
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_App_Irc( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_YESNO_START,
    SEQ_YESNO_WAIT,

    SEQ_DISPLAY_CREATE,
    SEQ_NET_START,
    SEQ_NET_START_WAIT,
    SEQ_NET_SEND_NAME_START,
    SEQ_NET_SEND_NAME_WAIT,
    SEQ_NET_SEND_KEY_START,
    SEQ_NET_SEND_KEY_WAIT,
    SEQ_NET_EXIT,
    SEQ_NET_EXIT_WAIT,
    SEQ_DISPLAY_DELETE,
    SEQ_CALL_DEMO1,
    SEQ_CALL_DEMO2,
    SEQ_SAVE_MSG,
    SEQ_SAVE_START,
    SEQ_SAVE_END,
    SEQ_COMPLETE_MSG_START,
    SEQ_COMPLETE_MSG_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_YESNO_START:
    {
      KEYSYS_LIST_SETUP setup;
      GFL_STD_MemClear( &setup, sizeof(KEYSYS_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.frm     = KEYSYS_BG_FRAME_TEXT_M;
      setup.font_plt = KEYSYS_PLT_BG_FONT_M;
      setup.frm_plt = KEYSYS_PLT_BG_FRAME_M;
      setup.frm_chr = KEYSYS_CGR_TEXT_M;
      setup.strID[0]  = KEYSYS_STR_58;
      setup.strID[1]  = KEYSYS_STR_59;
      setup.list_max  = 2;
      setup.is_cancel = TRUE;
      setup.cancel_idx = 1;
      setup.default_idx = 0;
      p_wk->p_list = KEYSYS_LIST_Init( &setup, HEAPID_KEY_SYSTEM );
      
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_60, KEYSYS_TEXT_TYPE_QUE ); 
      (*p_seq)++;
    }
    break;
  case SEQ_YESNO_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      u32 ret = KEYSYS_LIST_Main(p_wk->p_list );
      if( ret != KEYSYS_LIST_SELECT_NULL )
      {
        KEYSYS_LIST_Exit(p_wk->p_list);
        p_wk->p_list = NULL;

        switch( ret )
        {
        case 0:
          *p_seq = SEQ_DISPLAY_CREATE;
          break;
        default:
          GF_ASSERT(0);
        case 1:
          *p_seq = SEQ_END;
          break;
        }
      }
    }
    break;

  case SEQ_DISPLAY_CREATE:
    p_wk->ret = 0;
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplayIrc );
    (*p_seq)++;
    break;
  case SEQ_NET_START:
    {
      KEYSYS_NET_SEQ_PARAM_IW_CONNECT param;
      GFL_STD_MemClear( &param, sizeof(KEYSYS_NET_SEQ_PARAM_IW_CONNECT) );
      param.p_wk_adrs = p_wk;
      param.IrcEnd    = KeySys_NetCallBack_IrcEnd;
      p_wk->ret       = FALSE;
      KEYSYS_NET_RequestState( p_wk->p_net, KEYSYS_NET_REQ_CONNECT_IRCWIRELESS, &param );
    }
    (*p_seq)++;
    break;
  case SEQ_NET_START_WAIT:
    if( p_wk->ret )
    {
      GFL_CLWK *p_wave_d = KeySys_Obj_GetClwk( &p_wk->obj, KEYSYS_CLWK_WAVE_D );
      GFL_CLWK *p_wave_u = KeySys_Obj_GetClwk( &p_wk->obj, KEYSYS_CLWK_WAVE_U );
      GFL_CLWK *p_ds_d = KeySys_Obj_GetClwk( &p_wk->obj, KEYSYS_CLWK_DS_D );
      GFL_CLWK *p_ds_u = KeySys_Obj_GetClwk( &p_wk->obj, KEYSYS_CLWK_DS_U );
      GFL_CLACT_WK_SetDrawEnable( p_wave_d, FALSE );
      GFL_CLACT_WK_SetDrawEnable( p_wave_u, FALSE );
      GFL_CLACT_WK_SetAnmSeq( p_ds_d, 4 );
      GFL_CLACT_WK_SetAnmSeq( p_ds_u, 5 );

      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_49, KEYSYS_TEXT_TYPE_WAIT );

      p_wk->ret = FALSE;
    }
    else
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL )
      {
        (*p_seq) = SEQ_NET_EXIT;
      }
    }
    if( KEYSYS_NET_GetState( p_wk->p_net ) == KEYSYS_NET_STATE_IRCWIRELESS_WAIT )
    {
      p_wk->is_parent = GFL_NET_IsParentMachine();
      (*p_seq)++;
    }
    break;

  case SEQ_NET_SEND_NAME_START:
    {
      KEYSYS_NET_SEQ_PARAM_IW_SEND param;
      param.size    = sizeof( KEYSYS_PLAYER_DATA );
      param.cp_data = &p_wk->player_data[KEYSYS_DATA_SELF];
      KEYSYS_NET_RequestState( p_wk->p_net, KEYSYS_NET_REQ_SENDATA_BOTH_IRCWIRELESS, &param );
      (*p_seq)++;
    }
    break;
  case SEQ_NET_SEND_NAME_WAIT:
    if( KEYSYS_NET_GetState( p_wk->p_net ) == KEYSYS_NET_STATE_IRCWIRELESS_WAIT )
    {
      p_wk->ret = KEYSYS_NET_GetRecvData( p_wk->p_net, &p_wk->player_data[KEYSYS_DATA_OTHER], sizeof( KEYSYS_PLAYER_DATA ) );
      GF_ASSERT( p_wk->ret );
      (*p_seq)++;
    }
    break;
  case SEQ_NET_SEND_KEY_START:
    {
      KEYSYS_NET_SEQ_PARAM_IW_SEND param;
      param.size    = sizeof( KEYSYS_KEY_DATA );
      param.cp_data = &p_wk->key_data[KEYSYS_DATA_SELF];
      KEYSYS_NET_RequestState( p_wk->p_net, KEYSYS_NET_REQ_SENDATA_BOTH_IRCWIRELESS, &param );
      (*p_seq)++;
    }
    break;
  case SEQ_NET_SEND_KEY_WAIT:
    if( KEYSYS_NET_GetState( p_wk->p_net ) == KEYSYS_NET_STATE_IRCWIRELESS_WAIT )
    {
      p_wk->ret = KEYSYS_NET_GetRecvData( p_wk->p_net, &p_wk->key_data[KEYSYS_DATA_OTHER], sizeof( KEYSYS_KEY_DATA ) );
      GF_ASSERT( p_wk->ret );
      (*p_seq)++;
    }
    break;
  case SEQ_NET_EXIT:
    KEYSYS_NET_RequestState( p_wk->p_net, KEYSYS_NET_REQ_DISCONNECT_IRCWIRELESS, NULL );
    (*p_seq)++;
    break;
  case SEQ_NET_EXIT_WAIT:
    if( KEYSYS_NET_GetState( p_wk->p_net ) == KEYSYS_NET_STATE_WAIT )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    if( p_wk->ret )
    {
      (*p_seq)++;
    }
    else
    {
      *p_seq = SEQ_END;
    }
    break;
  case SEQ_CALL_DEMO1:
    if( p_wk->is_parent )
    {
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_Both_RecvDemo );
    }
    else
    {
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_App_SendDemo );
    }
    (*p_seq)++;
    break;
  case SEQ_CALL_DEMO2:
    if( !p_wk->is_parent )
    {
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_Both_RecvDemo );
    }
    else
    {
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_App_SendDemo );
    }
    (*p_seq)++;
    break;

  case SEQ_SAVE_MSG:
    //受信していたらセーブ、していなかったら終わり
    if( KeySys_KeyData_IsGet( &p_wk->key_data[KEYSYS_DATA_SELF], &p_wk->key_data[KEYSYS_DATA_OTHER], TRUE ) )
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_30, KEYSYS_TEXT_TYPE_WAIT );
      (*p_seq)++;
    }
    else
    {
      *p_seq = SEQ_COMPLETE_MSG_START;
    }
    break;
  case SEQ_SAVE_START:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      KeySys_KeyData_Merge( &p_wk->key_data[KEYSYS_DATA_SELF], &p_wk->key_data[KEYSYS_DATA_OTHER] );
      KeySys_KeyData_SetSave( &p_wk->key_data[KEYSYS_DATA_SELF], p_wk->p_key_sv );
      OutsideSave_SaveAsyncInit( p_wk->p_outside_sv );
      p_wk->cnt = 0;
      (*p_seq)++;
    }
    break;
  case SEQ_SAVE_END: 
    p_wk->cnt++;
    if( OutsideSave_SaveAsyncMain( p_wk->p_outside_sv ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_COMPLETE_MSG_START:
    if( p_wk->cnt++ > KEYSYS_SAVE_MSG_SYNC )
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_46, KEYSYS_TEXT_TYPE_STREAM_FIX );
    (*p_seq)++;
    }
    break;
  case SEQ_COMPLETE_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アプリモード  キーリスト
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_App_LockList( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_MAIN,
    SEQ_DISPLAY_DELETE,
    SEQ_SELECT_CALL,
    SEQ_SELECT_RET,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    //最初はメインメニュー作成
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplayLockList );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    KEYSYS_MENU_Main( p_wk->p_menu );

    if( KEYSYS_MENU_IsChangeSelect( p_wk->p_menu ) )
    {
      if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
      {
        KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KeySys_Mes_GetLockListInfoMessage( p_wk->p_key_sv, KEYSYS_MENU_GetSelect( p_wk->p_menu ) ), KEYSYS_TEXT_TYPE_QUE );
      }
    }

    if( KEYSYS_MENU_IsDecide( p_wk->p_menu ) )
    {
      p_wk->ret = KEYSYS_MENU_GetSelect( p_wk->p_menu );
      (*p_seq)++;
    }
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;

  case SEQ_SELECT_CALL:
    switch( p_wk->ret )
    {
    case 0://なんいど
      KeySys_Stack_Push( &p_wk->select, p_wk->ret );
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_App_Difficulty );
      *p_seq = SEQ_SELECT_RET;
      break;
    case 1://まちのきりかえ
      KeySys_Stack_Push( &p_wk->select, p_wk->ret );
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_App_Wfbc );
      *p_seq = SEQ_SELECT_RET;
      break;
    case 2://もどる
      KEYSYS_SEQ_Return( p_seqwk );
      break;
    default:
      GF_ASSERT(0);
    }
    break;


  case SEQ_SELECT_RET:
    //もう一度メインメニュー
    (*p_seq ) = 0;
    break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アプリモード  難易度変更
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_App_Difficulty( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_MAIN,

    SEQ_SAVE_MSG,
    SEQ_SAVE_START,
    SEQ_SAVE_MAIN,
    SEQ_SAVE_END_MSG,
    SEQ_RET_START,
    SEQ_RET_WAIT,

    SEQ_UNSELECT_MSG_START,
    SEQ_UNSELECT_MSG_WAIT,

    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplayDifficulty );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    KeySys_SelMenu_Main( &p_wk->selmenu );

    if( KeySys_SelMenu_IsDecide( &p_wk->selmenu ) )
    {
      if( KeySys_SelMenu_GetSelect( &p_wk->selmenu ) == 3 )
      {
        (*p_seq) = SEQ_DISPLAY_DELETE;
      }
      else
      {
        (*p_seq) = SEQ_SAVE_MSG;
      }
    }
    else if( KeySys_SelMenu_GetUnSelected( &p_wk->selmenu ) == KEYSYS_SELECT_MENU_RET_UNSELECT )
    {
      (*p_seq ) = SEQ_UNSELECT_MSG_START;
    }
    break;

  case SEQ_SAVE_MSG:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_30, KEYSYS_TEXT_TYPE_WAIT );
    (*p_seq)++;
    break;
  case SEQ_SAVE_START:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      OutsideSave_SaveAsyncInit( p_wk->p_outside_sv );
      p_wk->cnt = 0;
      (*p_seq)++;
    }
    break;
  case SEQ_SAVE_MAIN:
    p_wk->cnt++;
    if( OutsideSave_SaveAsyncMain( p_wk->p_outside_sv ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_SAVE_END_MSG:
    if( p_wk->cnt++ > KEYSYS_SAVE_MSG_SYNC )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_RET_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KeySys_Mes_GetDifficultInfoMessage(p_wk->p_key_sv), KEYSYS_TEXT_TYPE_QUE );
    (*p_seq)++;
    break;
  case SEQ_RET_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      KeySys_SelMenu_ReStart( &p_wk->selmenu );
      (*p_seq) = SEQ_MAIN;
    }
    break;

  case SEQ_UNSELECT_MSG_START:
    switch( KeySys_SelMenu_GetUnSelectedIndex( &p_wk->selmenu ) )
    {
    case KEY_SYSTEM_DIFFICULT_EASY:
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_54, KEYSYS_TEXT_TYPE_QUE );
      break;
    case KEY_SYSTEM_DIFFICULT_HARD:
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_55, KEYSYS_TEXT_TYPE_QUE );
      break;
    }
    (*p_seq)++;
    break;

  case SEQ_UNSELECT_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      if( GFL_UI_KEY_GetTrg() )
      {
        (*p_seq) = SEQ_RET_START;
      }
    }
    break;

  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アプリモード まちへんこう 
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_App_Wfbc( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_MAIN,

    SEQ_SAVE_MSG,
    SEQ_SAVE_START,
    SEQ_SAVE_MAIN,
    SEQ_SAVE_END_MSG,
    SEQ_RET_START,
    SEQ_RET_WAIT,

    SEQ_UNSELECT_MSG_START,
    SEQ_UNSELECT_MSG_WAIT,

    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplayWfbc );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    KeySys_SelMenu_Main( &p_wk->selmenu );

    if( KeySys_SelMenu_IsDecide( &p_wk->selmenu ) )
    {
      if( KeySys_SelMenu_GetSelect( &p_wk->selmenu ) == 2 )
      {
        (*p_seq) = SEQ_DISPLAY_DELETE;
      }
      else
      {
        (*p_seq) = SEQ_SAVE_MSG;
      }
    }
    else if( KeySys_SelMenu_GetUnSelected( &p_wk->selmenu ) == KEYSYS_SELECT_MENU_RET_UNSELECT || KeySys_SelMenu_GetUnSelected( &p_wk->selmenu ) == KEYSYS_SELECT_MENU_RET_SP_UNSELECT )
    {
      (*p_seq ) = SEQ_UNSELECT_MSG_START;
    }
    break;

  case SEQ_SAVE_MSG:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_30, KEYSYS_TEXT_TYPE_WAIT );
    (*p_seq)++;
    break;
  case SEQ_SAVE_START:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      OutsideSave_SaveAsyncInit( p_wk->p_outside_sv );
      p_wk->cnt = 0;
      (*p_seq)++;
    }
    break;
  case SEQ_SAVE_MAIN:
    p_wk->cnt++;
    if( OutsideSave_SaveAsyncMain( p_wk->p_outside_sv ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_SAVE_END_MSG:
    if( p_wk->cnt++ > KEYSYS_SAVE_MSG_SYNC )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_RET_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KeySys_Mes_GetWfbcInfoMessage(p_wk->p_key_sv ), KEYSYS_TEXT_TYPE_QUE );
    (*p_seq)++;
    break;
  case SEQ_RET_WAIT:

    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      KeySys_SelMenu_ReStart( &p_wk->selmenu );
      (*p_seq) = SEQ_MAIN;
    }
    break;

  case SEQ_UNSELECT_MSG_START:
    if( KeySys_SelMenu_GetUnSelected( &p_wk->selmenu ) == KEYSYS_SELECT_MENU_RET_SP_UNSELECT )
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_61, KEYSYS_TEXT_TYPE_QUE );
    }
    else
    {
      if( KeySys_SelMenu_GetUnSelectedIndex( &p_wk->selmenu ) == KEY_SYSTEM_WFBC_REVERSE )
      {
        if( GET_VERSION() == VERSION_W2 )
        {
          KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_57, KEYSYS_TEXT_TYPE_QUE );
        }
        else
        {
          KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_56, KEYSYS_TEXT_TYPE_QUE );
        }
      }
    }
      (*p_seq)++;
    break;

  case SEQ_UNSELECT_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      if( GFL_UI_KEY_GetTrg() )
      {
        (*p_seq) = SEQ_RET_START;
      }
    }
    break;

  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_Both_RecvDemo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CHECK_RECV,

    SEQ_NOTRECV_MSG_START,
    SEQ_NOTRECV_MSG_WAIT,

    SEQ_DISPLAY_CREATE,
    SEQ_RECV_WAIT,
    SEQ_MSG_START,
    SEQ_MSG_END,
    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_CHECK_RECV:
    if( KeySys_KeyData_IsGet( &p_wk->key_data[KEYSYS_DATA_SELF], &p_wk->key_data[KEYSYS_DATA_OTHER], TRUE ) )
    {
      (*p_seq) = SEQ_DISPLAY_CREATE;
    }
    else
    {
      (*p_seq) = SEQ_NOTRECV_MSG_START;
    }
    break;

  case SEQ_NOTRECV_MSG_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_48, KEYSYS_TEXT_TYPE_STREAM_FIX );
    (*p_seq)++;
    break;
  case SEQ_NOTRECV_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_DISPLAY_CREATE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplayRecvDemo );
    p_wk->cnt = 0;
    p_wk->ret = 0;
    KeySys_Demo_Init( &p_wk->demo );
    (*p_seq)++;
    break;
  case SEQ_RECV_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      p_wk->cnt = 0;
      (*p_seq)++;
    }
    break;
  case SEQ_MSG_START:
    while( 1 )
    {
      if( p_wk->ret >= KEY_SYSTEM_TYPE_MAX )
      {
        *p_seq = SEQ_DISPLAY_DELETE;
        break;
      }

      if( KeySys_Mes_CreateKeyMessage( p_wk, p_wk->ret, TRUE ) )
      {
        KEYSYS_TEXT_PrintBuf( p_wk->p_text, p_wk->p_strbuf, KEYSYS_TEXT_TYPE_STREAM_FIX );
        (*p_seq)++;
        break;
      }

      p_wk->ret++;

    }
    break;
  case SEQ_MSG_END:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      p_wk->cnt = 0;
      p_wk->ret++;
      *p_seq = SEQ_MSG_START;
    }
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  送信デモ
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_App_SendDemo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CHECK_SEND,

    SEQ_NOTSEND_MSG_START,
    SEQ_NOTSEND_MSG_WAIT,

    SEQ_DISPLAY_CREATE,
    SEQ_RECV_WAIT,
    SEQ_MSG_START,
    SEQ_MSG_END,
    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {

  case SEQ_CHECK_SEND:
    if( KeySys_KeyData_IsGet( &p_wk->key_data[KEYSYS_DATA_SELF], &p_wk->key_data[KEYSYS_DATA_OTHER], FALSE ) )
    {
      *p_seq = SEQ_DISPLAY_CREATE;
    }
    else
    {
      *p_seq = SEQ_NOTSEND_MSG_START;
    }
    break;

  case SEQ_NOTSEND_MSG_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_47, KEYSYS_TEXT_TYPE_STREAM_FIX );
    (*p_seq)++;
    break;
  case SEQ_NOTSEND_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_DISPLAY_CREATE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplaySendDemo );
    p_wk->cnt = 0;
    p_wk->ret = 0;
    (*p_seq)++;
    break;
  case SEQ_RECV_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      p_wk->cnt = 0;
      (*p_seq)++;
    }
    break;
  case SEQ_MSG_START:
    while( 1 )
    {
      if( p_wk->ret >= KEY_SYSTEM_TYPE_MAX )
      {
        *p_seq = SEQ_DISPLAY_DELETE;
        break;
      }

      if( KeySys_Mes_CreateKeyMessage( p_wk, p_wk->ret, FALSE ) )
      {
        KEYSYS_TEXT_PrintBuf( p_wk->p_text, p_wk->p_strbuf, KEYSYS_TEXT_TYPE_STREAM_FIX );
        (*p_seq)++;
        break;
      }

      p_wk->ret++;

    }
    break;
  case SEQ_MSG_END:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      p_wk->cnt = 0;
      p_wk->ret++;
      *p_seq = SEQ_MSG_START;
    }
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_FadeInBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
	enum
	{	
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		(*p_seq)++;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
		  (*p_seq)++;
		}
		break;

	case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

	default:
		GF_ASSERT(0);
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_FadeOutBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
	enum
	{
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};	

	switch( *p_seq )
	{	
	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    (*p_seq)++;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
      (*p_seq)++;
		}
		break;

	case SEQ_EXIT:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

	default:
		GF_ASSERT(0);
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  メインメニュー作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_CreateDisplayMainMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    //最初はメインメニュー作成
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl[KEYSYS_DISPLAY_APP_MAIN_MENU], HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  IRC通信画面作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_CreateDisplayIrc( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl[KEYSYS_DISPLAY_APP_IRC], HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ロックリスト画面作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_CreateDisplayLockList( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl[KEYSYS_DISPLAY_APP_LOCK_LIST], HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  難易度画面作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_CreateDisplayDifficulty( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl[KEYSYS_DISPLAY_APP_DIFFICULTY], HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  WFBC画面作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_CreateDisplayWfbc( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl[KEYSYS_DISPLAY_APP_WFBC], HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ画面作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_CreateDisplayRecvDemo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl[KEYSYS_DISPLAY_BOTH_RECV_DEMO], HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信デモ画面作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_CreateDisplaySendDemo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl[KEYSYS_DISPLAY_APP_SEND_DEMO], HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ディスプレイ破棄
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_DeleteDisplay( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_DELETE_START,
    SEQ_DISPLAY_DELETE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_DELETE_START:
    KEYSYS_DISPLAY_Delete( p_wk->p_display );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_DELETE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  メインメニュー作成
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_CreateMainMenu( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //タイトル
  KEYSYS_TEXT_Print( p_wk->p_title, p_wk->p_msg, KEYSYS_STR_00, KEYSYS_TEXT_TYPE_QUE );

  //メニュー追加
  {
    int i;
    KEYSYS_MENU_PARAM menu_param;
    GFL_STD_MemClear( &menu_param, sizeof(KEYSYS_MENU_PARAM) );
    menu_param.frm = KEYSYS_BG_FRAME_TEXT_M;
    menu_param.font_plt = KEYSYS_PLT_BG_FONT_M;
    menu_param.frm_chr  = KEYSYS_CGR_MENU_M;
    menu_param.frm_plt  = KEYSYS_PLT_BG_MENU_NORMAL_M;
    menu_param.p_msg    = p_wk->p_msg;
    menu_param.p_font   = p_wk->p_font;
    menu_param.max      = 3;
    if( KeySys_Stack_GetStackNum( &p_wk->select ) == 0 )
    {
      menu_param.cursor   = 0;
    }
    else
    {
      menu_param.cursor   = KeySys_Stack_GetNow( &p_wk->select );
      KeySys_Stack_Pop( &p_wk->select );
    }
    for( i = 0; i < menu_param.max; i++ )
    {
      menu_param.wnd_data[i].w  = 26;
      menu_param.wnd_data[i].h  = 2;
      menu_param.wnd_data[i].x  = 3;
      menu_param.wnd_data[i].y  = 4 + (i*(menu_param.wnd_data[i].h+2+1)); //+2はフレーム分+1は間隔分
    }
    menu_param.wnd_data[0].strID = KEYSYS_STR_01;
    menu_param.wnd_data[1].strID = KEYSYS_STR_02;
    menu_param.wnd_data[2].strID = KEYSYS_STR_06;

    p_wk->p_menu = KEYSYS_MENU_Init( &menu_param, heapID );

    //メッセージ
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_19+menu_param.cursor, KEYSYS_TEXT_TYPE_QUE );
  }

  //下画面
  {
    static KEYSYS_TEXTSET_WND_DATA wnd_data[]  =
    {
      {
        1,
        1,
        18,
        2,
        KEYSYS_STR_12,
      },
      {
        1,
        4,
        18,
        2,
        0,
      },
      {
        1,
        7,
        18,
        2,
        0,
      },
      {
        1,
        10,
        18,
        2,
        0,
      },
      {
        1,
        13,
        18,
        2,
        0,
      },
      {
        1,
        16,
        18,
        2,
        0,
      },
      {
        1,
        19,
        18,
        2,
        0,
      },
    };
    int i;
    int cnt = 0;
    static const u16 s_msg_tbl[2][KEY_SYSTEM_TYPE_MAX] =
    {
      {
        KEYSYS_STR_13,
        KEYSYS_STR_14,
        KEYSYS_STR_18,  //WF
      },
      {
        KEYSYS_STR_13,
        KEYSYS_STR_14,
        KEYSYS_STR_15,  //BC
      }
    };

    for( i = 0; i < KEY_SYSTEM_TYPE_MAX; i++ )
    {
      if( p_wk->key_data[KEYSYS_DATA_SELF].is_have[i] )
      {
        if( GET_VERSION() == VERSION_W2 )
        {
          wnd_data[ cnt+1 ].strID = s_msg_tbl[0][i];
        }
        else
        {
          wnd_data[ cnt+1 ].strID = s_msg_tbl[1][i];
        }
        cnt++;
      }
    }

    GF_ASSERT( cnt+1 <= NELEMS(wnd_data) );

    p_wk->p_textset = KEYSYS_TEXTSET_Init( wnd_data, cnt+1, KEYSYS_BG_FRAME_TEXT_S, KEYSYS_PLT_BG_FONT_S, p_wk->p_font, p_wk->p_msg, heapID );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  メインメニュー作成完了待ち
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Display_IsEndCreateMainMenu( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  BOOL is_end_title  = KEYSYS_TEXT_IsMsgEnd( p_wk->p_title );
  BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );
  BOOL is_end_menu = KEYSYS_MENU_IsDrawEnd( p_wk->p_menu );
  BOOL is_end_textset = KEYSYS_TEXTSET_IsDrawEnd( p_wk->p_textset );

  if( is_end_title && is_end_text && is_end_menu && is_end_textset )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_S );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  メインメニュー破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_DeleteMainMenu( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //テキストセット
  KEYSYS_TEXTSET_Exit( p_wk->p_textset );
  p_wk->p_textset = NULL;

  //メニュー破棄
  KEYSYS_MENU_Exit( p_wk->p_menu );
  p_wk->p_menu = NULL;    
  
  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_S );

}
//----------------------------------------------------------------------------
/**
 *	@brief  通信待機中  作成
 *
 *	@param	void *p_wk_adrs ワークアドレス 
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_CreateIrc( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //メッセージ
  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_22, KEYSYS_TEXT_TYPE_WAIT );

  KeySys_Obj_AddClwk( &p_wk->obj, KEYSYS_CLWK_WAVE_D, heapID );
  KeySys_Obj_AddClwk( &p_wk->obj, KEYSYS_CLWK_WAVE_U, heapID );
  KeySys_Obj_AddClwk( &p_wk->obj, KEYSYS_CLWK_DS_D, heapID );
  KeySys_Obj_AddClwk( &p_wk->obj, KEYSYS_CLWK_DS_U, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  通信待機中  作成待ち
 *
 *	@param	void *p_wk_adrs   ワークアドレス
 *
 *	@return　TRUE作成完了 FALSE破棄
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Display_IsEndCreateIrc( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );

  if( is_end_text )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  通信待機中  破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_DeleteIrc( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  KeySys_Obj_RemoveClwk( &p_wk->obj, KEYSYS_CLWK_DS_U );
  KeySys_Obj_RemoveClwk( &p_wk->obj, KEYSYS_CLWK_DS_D );
  KeySys_Obj_RemoveClwk( &p_wk->obj, KEYSYS_CLWK_WAVE_U );
  KeySys_Obj_RemoveClwk( &p_wk->obj, KEYSYS_CLWK_WAVE_D );

  KEYSYS_TEXT_EndWait( p_wk->p_text );

}
//----------------------------------------------------------------------------
/**
 *	@brief  ロックリスト画面  作成
 *
 *	@param	void *p_wk_adrs ワークアドレス 
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_CreateLockList( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //メニュー追加
  {
    int i;
    KEYSYS_MENU_PARAM menu_param;
    GFL_STD_MemClear( &menu_param, sizeof(KEYSYS_MENU_PARAM) );
    menu_param.frm = KEYSYS_BG_FRAME_TEXT_M;
    menu_param.font_plt = KEYSYS_PLT_BG_FONT_M;
    menu_param.frm_chr  = KEYSYS_CGR_MENU_M;
    menu_param.frm_plt  = KEYSYS_PLT_BG_MENU_NORMAL_M;
    menu_param.p_msg    = p_wk->p_msg;
    menu_param.p_font   = p_wk->p_font;
    menu_param.max      = 3;
    if( KeySys_Stack_GetStackNum( &p_wk->select ) == 1 )
    {
      menu_param.cursor   = 0;
    }
    else
    {
      menu_param.cursor   = KeySys_Stack_GetNow( &p_wk->select );
      KeySys_Stack_Pop( &p_wk->select );
    }
    for( i = 0; i < menu_param.max; i++ )
    {
      menu_param.wnd_data[i].w  = 26;
      menu_param.wnd_data[i].h  = 2;
      menu_param.wnd_data[i].x  = 3;
      menu_param.wnd_data[i].y  = 4 + (i*(menu_param.wnd_data[i].h+2+1)); //+2はフレーム分+1は間隔分
    }
    menu_param.wnd_data[0].strID = KEYSYS_STR_04;
    menu_param.wnd_data[1].strID = KEYSYS_STR_05;
    menu_param.wnd_data[2].strID = KEYSYS_STR_06;
    p_wk->p_menu = KEYSYS_MENU_Init( &menu_param, heapID );


    //メッセージ
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KeySys_Mes_GetLockListInfoMessage( p_wk->p_key_sv, menu_param.cursor ), KEYSYS_TEXT_TYPE_QUE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ロックリスト画面  作成待ち
 *
 *	@param	void *p_wk_adrs   ワークアドレス
 *
 *	@return　TRUE作成完了 FALSE破棄
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Display_IsEndCreateLockList( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );
  BOOL is_end_menu = KEYSYS_MENU_IsDrawEnd( p_wk->p_menu );

  if( is_end_text && is_end_menu )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ロックリスト画面  破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_DeleteLockList( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //メニュー破棄
  KEYSYS_MENU_Exit( p_wk->p_menu );
  p_wk->p_menu = NULL;

  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
}

//----------------------------------------------------------------------------
/**
 *	@brief  難易度  作成
 *
 *	@param	void *p_wk_adrs ワークアドレス 
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_CreateDifficulty( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //メッセージ
  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KeySys_Mes_GetDifficultInfoMessage(p_wk->p_key_sv), KEYSYS_TEXT_TYPE_QUE );


  //メニュー追加
  KeySys_SelMenu_Init( &p_wk->selmenu, KEYSYS_SELMENU_TYPE_DIFFICULTY, &p_wk->obj, &p_wk->p_menu, p_wk->p_msg, p_wk->p_font, p_wk->p_key_sv, p_wk->p_param->p_gamedata, heapID );

}
//----------------------------------------------------------------------------
/**
 *	@brief  難易度  作成待ち
 *
 *	@param	void *p_wk_adrs   ワークアドレス
 *
 *	@return　TRUE作成完了 FALSE破棄
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Display_IsEndCreateDifficulty( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );
  BOOL is_end_menu = KeySys_SelMenu_IsDrawEnd( &p_wk->selmenu );

  if( is_end_text && is_end_menu )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  難易度  破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_DeleteDifficulty( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //メニュー破棄
  KeySys_SelMenu_Exit( &p_wk->selmenu );

  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
}
//----------------------------------------------------------------------------
/**
 *	@brief  まちへんこう  作成
 *
 *	@param	void *p_wk_adrs ワークアドレス 
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_CreateWfbc( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;
  
  //メッセージ
  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KeySys_Mes_GetWfbcInfoMessage(p_wk->p_key_sv), KEYSYS_TEXT_TYPE_QUE );

  //メニュー追加
  KeySys_SelMenu_Init( &p_wk->selmenu, KEYSYS_SELMENU_TYPE_WFBC, &p_wk->obj, &p_wk->p_menu, p_wk->p_msg, p_wk->p_font, p_wk->p_key_sv, p_wk->p_param->p_gamedata, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  まちへんこう  作成待ち
 *
 *	@param	void *p_wk_adrs   ワークアドレス
 *
 *	@return　TRUE作成完了 FALSE破棄
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Display_IsEndCreateWfbc( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );
  BOOL is_end_menu = KeySys_SelMenu_IsDrawEnd( &p_wk->selmenu );

  if( is_end_text && is_end_menu )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  まちへんこう  破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_DeleteWfbc( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //メニュー破棄
  KeySys_SelMenu_Exit( &p_wk->selmenu );

  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
}
//----------------------------------------------------------------------------
/**
 *	@brief  キー受信デモ  作成
 *
 *	@param	void *p_wk_adrs ワークアドレス 
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_CreateRecvDemo( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  GFL_CLWK *p_key = KeySys_Obj_AddClwk( &p_wk->obj, KEYSYS_CLWK_KEY, heapID );

  //メッセージ
  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_38, KEYSYS_TEXT_TYPE_STREAM_FIX );
  
}
//----------------------------------------------------------------------------
/**
 *	@brief  キー受信デモ  作成待ち
 *
 *	@param	void *p_wk_adrs   ワークアドレス
 *
 *	@return　TRUE作成完了 FALSE破棄
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Display_IsEndCreateRecvDemo( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  キー受信デモ 破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_DeleteRecvDemo( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  KeySys_Obj_RemoveClwk( &p_wk->obj, KEYSYS_CLWK_KEY );

  KEYSYS_TEXT_EndWait( p_wk->p_text );

  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
}
//----------------------------------------------------------------------------
/**
 *	@brief  キー送信デモ  作成
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_CreateSendDemo( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //上画面
  {
    static KEYSYS_TEXTSET_WND_DATA wnd_data[]  =
    {
      {
        1,
        1+2,
        18,
        2,
        KEYSYS_STR_12,
      },
      {
        1,
        4+2,
        18,
        2,
        0,
      },
      {
        1,
        7+2,
        18,
        2,
        0,
      },
      {
        1,
        10+2,
        18,
        2,
        0,
      },
    };
    int i;
    int cnt = 0;
    static const u16 s_msg_tbl[2][KEY_SYSTEM_TYPE_MAX] =
    {
      {
        KEYSYS_STR_13,
        KEYSYS_STR_14,
        KEYSYS_STR_18,  //WF
      },
      {
        KEYSYS_STR_13,
        KEYSYS_STR_14,
        KEYSYS_STR_15,  //BC
      }
    };

    for( i = 0; i < KEY_SYSTEM_TYPE_MAX; i++ )
    {
      if( p_wk->key_data[KEYSYS_DATA_SELF].is_have[i] )
      {
        if( GET_VERSION() == VERSION_W2 )
        {
          wnd_data[ cnt+1 ].strID = s_msg_tbl[0][i];
        }
        else
        {
          wnd_data[ cnt+1 ].strID = s_msg_tbl[1][i];
        }
        cnt++;
      }
    }

    GF_ASSERT( cnt+1 <= NELEMS(wnd_data) );

    p_wk->p_textset = KEYSYS_TEXTSET_Init( wnd_data, cnt+1, KEYSYS_BG_FRAME_TEXT_M, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, p_wk->p_msg, heapID );
  }

  //メッセージ
  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_31, KEYSYS_TEXT_TYPE_STREAM_FIX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  キー送信デモ  作成街
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *
 *	@return TRUE作成完了  FALSE破棄
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Display_IsEndCreateSendDemo( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );
  BOOL is_end_textset = KEYSYS_TEXTSET_IsDrawEnd( p_wk->p_textset );

  if( is_end_textset )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  キー送信デモ破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_DeleteSendDemo( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //テキストセット
  KEYSYS_TEXTSET_Exit( p_wk->p_textset );
  p_wk->p_textset = NULL;

  KEYSYS_TEXT_EndWait( p_wk->p_text );

  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
}

//=============================================================================
/**
 *      BG処理
 */
//=============================================================================
#define KEYSYS_BG_SCROLL_SPEED     (-FX32_CONST(0.2))
//----------------------------------------------------------------------------
/**
 *	@brief  BG読み込み
 *
 *	@param	KEYSYS_WORK *p_wk  ワーク
 *	@param	heapID  読み込み用ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Bg_Load( KEYSYS_BG_WORK *p_wk, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_BG_WORK));

  //最初にクリアキャラセット
  GFL_BG_SetClearCharacter( KEYSYS_BG_FRAME_TEXT_M, 0x20, KEYSYS_CGR_CLEAR_M*0x20, heapID );

  {
    int i;
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_KEY_SYSTEM, heapID );

    //----背景----
    //PLT
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_key_system_gra_back00_NCLR,
        PALTYPE_MAIN_BG, KEYSYS_PLT_BG_BACK_M*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_key_system_gra_back00_NCLR,
        PALTYPE_SUB_BG, KEYSYS_PLT_BG_BACK_S*0x20, 0x20, heapID );
    //CHR
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_key_system_gra_back00_NCGR, 
        KEYSYS_BG_FRAME_BACK_M, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_key_system_gra_back00_NCGR, 
        KEYSYS_BG_FRAME_BACK_S, 0, 0, FALSE, heapID );
    //SCR
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_key_system_gra_back00_NSCR,
        KEYSYS_BG_FRAME_BACK_M, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_key_system_gra_back00_NSCR,
        KEYSYS_BG_FRAME_BACK_S, 0, 0, FALSE, heapID );

    //----メニュー----
    //PLT
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_key_system_gra_menu_frame_NCLR,
        PALTYPE_MAIN_BG, KEYSYS_PLT_BG_MENU_NORMAL_M*0x20, 0x20*2, heapID );

    //CHR
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_key_system_gra_menu_frame_NCGR, 
        KEYSYS_BG_FRAME_TEXT_M, KEYSYS_CGR_MENU_M, 0, FALSE, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  //フレーム
  BmpWinFrame_GraphicSet( KEYSYS_BG_FRAME_TEXT_M, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M, MENU_TYPE_SYSTEM, HEAPID_KEY_SYSTEM );

  //フォント
  {	
    ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_FONT, heapID );

    //上下画面フォントパレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_MAIN_BG, KEYSYS_PLT_BG_FONT_M*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_SUB_BG, KEYSYS_PLT_BG_FONT_S*0x20, 0x20, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  BGメイン処理
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_Bg_Main( KEYSYS_BG_WORK *p_wk )
{
  p_wk->scroll_now += KEYSYS_BG_SCROLL_SPEED;
  GFL_BG_SetScrollReq( KEYSYS_BG_FRAME_BACK_M, GFL_BG_SCROLL_Y_SET, p_wk->scroll_now >> FX32_SHIFT );
  GFL_BG_SetScrollReq( KEYSYS_BG_FRAME_BACK_S, GFL_BG_SCROLL_Y_SET, p_wk->scroll_now >> FX32_SHIFT );

}

//----------------------------------------------------------------------------
/**
 *	@brief OBJ初期化
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 *	@param	*p_graphic  グラフィック
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Obj_Init( KEYSYS_OBJ_WORK *p_wk, const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_OBJ_WORK));
  p_wk->p_clunit = KEY_SYSTEM_GRAPHIC_GetClunit( cp_graphic );
}
//----------------------------------------------------------------------------
/**
 *	@brief  OBJに読み込み処理
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Obj_Load( KEYSYS_OBJ_WORK *p_wk, HEAPID heapID )
{
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_KEY_SYSTEM, heapID );

    p_wk->res[KEYSYS_RES_OBJ_NCG] =
      GFL_CLGRP_CGR_Register( p_handle, NARC_key_system_gra_obj_NCGR,
          FALSE, CLSYS_DRAW_MAIN, heapID );

    p_wk->res[KEYSYS_RES_OBJ_NCL] =
      GFL_CLGRP_PLTT_RegisterEx(
          p_handle ,NARC_key_system_gra_obj_NCLR, CLSYS_DRAW_MAIN, 0, KEYSYS_PLT_OBJ_COMMON_M, 1, heapID );

    p_wk->res[KEYSYS_RES_OBJ_NCE] =
      GFL_CLGRP_CELLANIM_Register(
      p_handle, NARC_key_system_gra_obj_NCER, NARC_key_system_gra_obj_NANR, heapID  );

    GFL_ARC_CloseDataHandle(p_handle);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  OBJ読み込み破棄
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_Obj_Release( KEYSYS_OBJ_WORK *p_wk )
{
  GFL_CLGRP_PLTT_Release( p_wk->res[KEYSYS_RES_OBJ_NCL] );
  GFL_CLGRP_CGR_Release( p_wk->res[KEYSYS_RES_OBJ_NCG] );
  GFL_CLGRP_CELLANIM_Release( p_wk->res[KEYSYS_RES_OBJ_NCE] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  CLWKを追加
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 *	@param	id      作成するもの
 *	@param	heapID  ヒープID
 *
 *	@return GFL_CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK * KeySys_Obj_AddClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id, HEAPID heapID )
{
  GFL_CLWK_DATA clwk_data;
  CLSYS_DRAW_TYPE draw_type;

  GF_ASSERT( p_wk->p_clwk[ id ] == NULL );

  switch( id )
  {
  case KEYSYS_CLWK_WAVE_D:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 128;
    clwk_data.pos_y = 113;
    clwk_data.anmseq  = 0;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 1;
    break;

  case KEYSYS_CLWK_WAVE_U:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 128;
    clwk_data.pos_y = 77;
    clwk_data.anmseq  = 1;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 1;
    break;

  case KEYSYS_CLWK_DS_D:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 57;
    clwk_data.pos_y = 154;
    clwk_data.anmseq  = 2;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 1;
    break;

  case KEYSYS_CLWK_DS_U:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 200;
    clwk_data.pos_y = 38;
    clwk_data.anmseq  = 3;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 1;
    break;

  case KEYSYS_CLWK_LOCK_00:
  case KEYSYS_CLWK_LOCK_01:
  case KEYSYS_CLWK_LOCK_02:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 0;
    clwk_data.pos_y = 0;
    clwk_data.anmseq  = 7;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 0;
    break;

  case KEYSYS_CLWK_MARK:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 200;
    clwk_data.pos_y = 38;
    clwk_data.anmseq  = 6;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 0;
    break;
    
  case KEYSYS_CLWK_KEY:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x   = 128;
    clwk_data.pos_y   = 98;
    clwk_data.anmseq  = 8;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 0;
    break;

  default:
    GF_ASSERT(0);
    return NULL;
  }

  p_wk->p_clwk[ id ] = GFL_CLACT_WK_Create( p_wk->p_clunit,
                                              p_wk->res[KEYSYS_RES_OBJ_NCG],
                                              p_wk->res[KEYSYS_RES_OBJ_NCL],
                                              p_wk->res[KEYSYS_RES_OBJ_NCE],
                                              &clwk_data, draw_type, heapID );

  GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[ id ], TRUE );
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[ id ], TRUE );
  GFL_CLACT_WK_SetAffineParam( p_wk->p_clwk[ id ], CLSYS_AFFINETYPE_DOUBLE );

  return p_wk->p_clwk[ id ];
}
//----------------------------------------------------------------------------
/**
 *	@brief CLWKを破棄
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 *	@param	id 破棄するもの
 */
//-----------------------------------------------------------------------------
static void KeySys_Obj_RemoveClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id )
{
  if( p_wk->p_clwk[id] ) 
  {
    GFL_CLACT_WK_Remove( p_wk->p_clwk[id] );
    p_wk->p_clwk[id] = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  CLWKを取得
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk　ワーク
 *	@param	id  取得するもの
 *
 *	@return GFL_CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK * KeySys_Obj_GetClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id )
{
  return p_wk->p_clwk[ id ];
}

//=============================================================================
/**
 *      STACK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  スタック  初期化
 *
 *	@param	KEYSYS_STACK *p_wk あw－く
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Stack_Init( KEYSYS_STACK *p_wk, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_STACK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スタックに値をPush
 *
 *	@param	KEYSYS_STACK *p_wk ワーク
 *	@param	num 選んだもの
 */
//-----------------------------------------------------------------------------
static void KeySys_Stack_Push( KEYSYS_STACK *p_wk, u32 num )
{
  p_wk->stack[ p_wk->pos++ ] = num;

  GF_ASSERT( p_wk->pos < KEYSYS_STACK_MAX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スタック  値をPop
 *
 *	@param	const KEYSYS_STACK *cp_wk ワーク
 *
 *	@return 値
 */
//-----------------------------------------------------------------------------
static u32 KeySys_Stack_Pop( KEYSYS_STACK *p_wk )
{
  GF_ASSERT( p_wk->pos != 0 );
  return p_wk->stack[ --p_wk->pos ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  スタック  現在の値を取得
 *
 *	@param	const KEYSYS_STACK *cp_wk ワーク
 *
 *	@return 値
 */
//-----------------------------------------------------------------------------
static u32 KeySys_Stack_GetNow( const KEYSYS_STACK *cp_wk )
{
  GF_ASSERT( cp_wk->pos != 0 );
  return cp_wk->stack[ cp_wk->pos - 1 ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  スタック数を取得
 *
 *	@param	const KEYSYS_STACK *cp_wk   ワーク
 *
 *	@return スタック数
 */
//-----------------------------------------------------------------------------
static u32 KeySys_Stack_GetStackNum( const KEYSYS_STACK *cp_wk )
{
  return cp_wk->pos;
}
//=============================================================================
/**
 *      キーデータ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ワークに所持情報をセット
 *
 *	@param	KEYSYS_KEY_DATA *p_wk　ワーク
 *	@param	*p_sv キーセーブ
 */
//-----------------------------------------------------------------------------
static void KeySys_KeyData_Init( KEYSYS_KEY_DATA *p_wk, const KEY_SYSTEM_SAVE *cp_sv )
{
  int i;

  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_KEY_DATA) );
  p_wk->version = GET_VERSION();

  for( i = 0; i < KEY_SYSTEM_TYPE_MAX; i++ )
  {
    p_wk->is_have[i] = KEY_SYSTEM_SAVE_IsHave( cp_sv, i );
    p_wk->is_use[i] = KEY_SYSTEM_SAVE_IsUse( cp_sv, i );
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  ワークからセーブに所持設定
 *
 *	@param	KEYSYS_KEY_DATA *p_wk ワーク
 *	@param	*p_sv キーセーブ
 */
//-----------------------------------------------------------------------------
static void KeySys_KeyData_SetSave( const KEYSYS_KEY_DATA *cp_wk, KEY_SYSTEM_SAVE *p_sv )
{
  int i;
  for( i = 0; i < KEY_SYSTEM_TYPE_MAX; i++ )
  {
    if( cp_wk->is_have[i] )
    {
      KEY_SYSTEM_SAVE_SetHave( p_sv, i );
    }
    if( cp_wk->is_use[i] )
    {
      KEY_SYSTEM_SAVE_SetUse( p_sv, i );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  送／受信データを取得したかどうか
 *
 *	@param	const KEY_SYSTEM_WORK *cp_wk  ワーク
 *	@param	recv_or_send  TRUEで受信  FALSEで送信
 *
 *	@return TRUE取得  FALSE取得していない
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_KeyData_IsGet( const KEYSYS_KEY_DATA *cp_self, const KEYSYS_KEY_DATA *cp_other,  BOOL recv_or_send )
{
  int i;
  int cnt = 0;

  for( i = 0; i < KEY_SYSTEM_TYPE_MAX; i++ )
  {
    if( recv_or_send )
    {
      //受信
      if( cp_self->is_use[ i ] == FALSE
    && cp_other->is_have[ i ] == TRUE )
      {
        if( i == KEY_SYSTEM_TYPE_CITY_WFBC )
        {
          if( cp_self->version != cp_other->version )
          {
            cnt++;
          }
        }
        else
        {
          cnt++;
        }
      }
    }
    else
    {
      //送信
      if( cp_self->is_have[ i ] == TRUE
    && cp_other->is_use[ i ] == FALSE )
      {
        if( i == KEY_SYSTEM_TYPE_CITY_WFBC )
        {
          if( cp_self->version != cp_other->version )
          {
            cnt++;
          }
        }
        else
        {
          cnt++;
        }
      }
    }
  }

  return cnt != 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  相手のデータを自分へマージ
 *
 *	@param	KEYSYS_KEY_DATA *p_self   自分のワーク
 *	@param	KEY_SYSTEM_WORK *cp_other 相手のワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_KeyData_Merge( KEYSYS_KEY_DATA *p_self, const KEYSYS_KEY_DATA *cp_other )
{
  int i;
  for( i = 0; i < KEY_SYSTEM_TYPE_MAX;i++ )
  {
    if( cp_other->is_have[i] )
    {
      if( i == KEY_SYSTEM_TYPE_CITY_WFBC )
      {
        if( p_self->version != cp_other->version )
        {
          p_self->is_use[i] = TRUE;
        }
      }
      else
      {
        p_self->is_use[i] = TRUE;
      }
    }
  }
}
//=============================================================================
/**
 *      プレイヤーデータ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤーデータを初期化
 *
 *	@param	KEYSYS_PLAYER_DATA *p_wk  プレイヤーデータ
 *	@param	*p_gamedata ゲームデータ
 */
//-----------------------------------------------------------------------------
static void KeySys_PlayerData_Init( KEYSYS_PLAYER_DATA *p_wk, GAMEDATA *p_gamedata )
{
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_PLAYER_DATA) );
  p_wk->is_save_exist = SaveData_GetExistFlag( p_sv );
  if( p_wk->is_save_exist )
  {
    MYSTATUS *p_my = GAMEDATA_GetMyStatus( p_gamedata );
    GFL_STD_MemCopy( p_my, p_wk->mystatus_buff, MYSTATUS_SAVE_SIZE );
  }
}
//=============================================================================
/**
 *      STACK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief　選択メニュー  初期化
 *
 *	@param	KEYSYS_SELECT_MENU *p_wk  ワーク
 *	@param	type    選択タイプ
 *	@param	*p_obj  OBJ管理
 *	@param	**pp_menu メニューポインタのワーク
 *	@param	*p_msg    メッセージ
 *	@param	*p_font   フォント
 *	@param	*p_sv     キーシステムセーブ
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_SelMenu_Init( KEYSYS_SELECT_MENU *p_wk, KEYSYS_SELMENU_TYPE type, KEYSYS_OBJ_WORK *p_obj, KEYSYS_MENU_WORK **pp_menu, GFL_MSGDATA *p_msg, GFL_FONT *p_font, KEY_SYSTEM_SAVE *p_sv, GAMEDATA *p_gamedata, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_SELECT_MENU) );
  p_wk->p_obj = p_obj;
  p_wk->pp_menu = pp_menu;
  p_wk->p_key_sv = p_sv;
  p_wk->p_gamedata = p_gamedata;


  switch( type )
  {
  case KEYSYS_SELMENU_TYPE_DIFFICULTY:
    p_wk->Init = KeySys_SelMenu_Difficulty_Init;
    break;
  case KEYSYS_SELMENU_TYPE_WFBC:
    p_wk->Init = KeySys_SelMenu_Wfbc_Init;
    break;
  default:
    GF_ASSERT(0);
  }

  {
    GFL_CLWK *p_clwk;
    int i;
    for( i = KEYSYS_CLWK_LOCK_00; i <=KEYSYS_CLWK_LOCK_02; i++ )
    {
      p_clwk = KeySys_Obj_AddClwk( p_obj, i, heapID );
      GFL_CLACT_WK_SetDrawEnable( p_clwk,FALSE );
    }
    p_clwk = KeySys_Obj_AddClwk( p_obj, KEYSYS_CLWK_MARK, heapID );
    GFL_CLACT_WK_SetDrawEnable( p_clwk,FALSE );
  }

  {
    int i;
    KEYSYS_MENU_PARAM menu_param;
    GFL_STD_MemClear( &menu_param, sizeof(KEYSYS_MENU_PARAM) );
    menu_param.frm = KEYSYS_BG_FRAME_TEXT_M;
    menu_param.font_plt = KEYSYS_PLT_BG_FONT_M;
    menu_param.frm_chr  = KEYSYS_CGR_MENU_M;
    menu_param.frm_plt  = KEYSYS_PLT_BG_MENU_NORMAL_M;
    menu_param.p_msg    = p_msg;
    menu_param.p_font   = p_font;
    menu_param.cursor   = 0;
    menu_param.p_wk_adrs = p_wk;
    menu_param.DecideCallBack = KeySys_SelMenu_DecideCallBack;

    //あとで最大値いれる
    for( i = 0; i < KEYSYS_MENU_WINDOW_MAX; i++ )
    {
      menu_param.wnd_data[i].w      = 15;
      menu_param.wnd_data[i].h      = 2;
      menu_param.wnd_data[i].x      = 16;
      menu_param.wnd_data[i].y      = 2+(i*(menu_param.wnd_data[0].h+2));

      if( KEYSYS_CLWK_LOCK_00+i <= KEYSYS_CLWK_LOCK_02 )
      {
        GFL_CLACTPOS pos;
        GFL_CLWK *p_clwk = KeySys_Obj_GetClwk( p_obj, KEYSYS_CLWK_LOCK_00+i );
        pos.x = (menu_param.wnd_data[i].x + menu_param.wnd_data[i].w - 1) * 8;
        pos.y = (menu_param.wnd_data[i].y+1) * 8;
        GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DRAW_MAIN );
      }
    }

    p_wk->Init( p_wk, &menu_param );

    menu_param.wnd_data[ menu_param.max-1 ].x += 3;
    menu_param.wnd_data[ menu_param.max-1 ].w -= 3;

    *p_wk->pp_menu = KEYSYS_MENU_Init( &menu_param, heapID );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	KEYSYS_SELECT_MENU *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_SelMenu_Exit( KEYSYS_SELECT_MENU *p_wk )
{
  KEYSYS_MENU_Exit( *p_wk->pp_menu );
  *p_wk->pp_menu = NULL;

  KeySys_Obj_RemoveClwk( p_wk->p_obj, KEYSYS_CLWK_LOCK_02 );
  KeySys_Obj_RemoveClwk( p_wk->p_obj, KEYSYS_CLWK_LOCK_01 );
  KeySys_Obj_RemoveClwk( p_wk->p_obj, KEYSYS_CLWK_LOCK_00 );
  KeySys_Obj_RemoveClwk( p_wk->p_obj, KEYSYS_CLWK_MARK );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン
 *
 *	@param	KEYSYS_SELECT_MENU *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_SelMenu_Main( KEYSYS_SELECT_MENU *p_wk )
{
  KEYSYS_MENU_Main( *p_wk->pp_menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画完了チェック
 *
 *	@param	const KEYSYS_SELECT_MENU *cp_wk   ワーク
 *
 *	@return　TRUE描画完了 FALSE描画中
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_SelMenu_IsDrawEnd( const KEYSYS_SELECT_MENU *cp_wk )
{
  return KEYSYS_MENU_IsDrawEnd( *cp_wk->pp_menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief  選んでいるものを取得
 *
 *	@param	const KEYSYS_SELECT_MENU *cp_wk   ワーク
 *
 *	@return 選んだ物
 */
//-----------------------------------------------------------------------------
static s32 KeySys_SelMenu_GetSelect( const KEYSYS_SELECT_MENU *cp_wk )
{
  return KEYSYS_MENU_GetSelect( *cp_wk->pp_menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief　決定チェック
 *
 *	@param	const KEYSYS_SELECT_MENU *cp_wk ワーク 
 *
 *	@return TRUE決定した  FALSE決定してない
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_SelMenu_IsDecide( const KEYSYS_SELECT_MENU *cp_wk )
{
  return KEYSYS_MENU_IsDecide( *cp_wk->pp_menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief  再スタート
 *
 *	@param	KEYSYS_SELECT_MENU *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_SelMenu_ReStart( KEYSYS_SELECT_MENU *p_wk )
{
  p_wk->unselected        = KEYSYS_SELECT_MENU_RET_NONE;
  p_wk->unselected_index  = 0;
  KEYSYS_MENU_ReStart( *p_wk->pp_menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief　押せない物を決定した
 *
 *	@param	const KEYSYS_SELECT_MENU *cp_wk ワーク
 *
 *	@return TRUEならば押した。  FALSE押していない
 */
//-----------------------------------------------------------------------------
static KEYSYS_SELECT_MENU_RET KeySys_SelMenu_GetUnSelected( const KEYSYS_SELECT_MENU *cp_wk )
{
  return cp_wk->unselected;
}
//----------------------------------------------------------------------------
/**
 *	@brief  押せないものを押したときのインデックス
 *
 *	@param	const KEYSYS_SELECT_MENU *cp_wk ワーク
 *
 *	@return インデックス
 */
//-----------------------------------------------------------------------------
static s32  KeySys_SelMenu_GetUnSelectedIndex( const KEYSYS_SELECT_MENU *cp_wk )
{
  return cp_wk->unselected_index;
}
//----------------------------------------------------------------------------
/**
 *	@brief  押せるか押せないかを決めるコールバック
 *
 *	@param	int index   インデックス
 *	@param	*p_wk_adrs  ワークアドレス
 *
 *	@return TRUE押せる  FALSEで押せない
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_SelMenu_DecideCallBack( int index, void *p_wk_adrs )
{
  KEYSYS_SELECT_MENU *p_wk = p_wk_adrs;
  KEYSYS_SELECT_MENU_RET ret = p_wk->DecideCallBack( index, p_wk );
  switch( ret )
  {
  case KEYSYS_SELECT_MENU_RET_SELECTED:
    return FALSE;
  case KEYSYS_SELECT_MENU_RET_SP_UNSELECT:
  case KEYSYS_SELECT_MENU_RET_UNSELECT:
    p_wk->unselected = ret;
    p_wk->unselected_index = index;
    return FALSE;
  default:
    GF_ASSERT(0);
  case KEYSYS_SELECT_MENU_RET_DECIDE:
    return TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  タイプ別初期化
 *
 *	@param	KEYSYS_SELECT_MENU *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_SelMenu_Difficulty_Init( KEYSYS_SELECT_MENU *p_wk, KEYSYS_MENU_PARAM *p_param )
{
  //メニューの設定
  p_param->max      = 4;
  p_param->wnd_data[0].strID  = KEYSYS_STR_07;
  p_param->wnd_data[1].strID  = KEYSYS_STR_08;
  p_param->wnd_data[2].strID  = KEYSYS_STR_09;
  p_param->wnd_data[3].strID  = KEYSYS_STR_06;
  p_wk->DecideCallBack = KeySys_SelMenu_Difficulty_DecideCallBack;

  //OBJの設定
  {
    if( !KEY_SYSTEM_SAVE_IsUse( p_wk->p_key_sv, KEY_SYSTEM_TYPE_DIFFICULT_EASY ) )
    {
      GFL_CLWK *p_clwk = KeySys_Obj_GetClwk( p_wk->p_obj, KEYSYS_CLWK_LOCK_00+KEY_SYSTEM_DIFFICULT_EASY );
      GFL_CLACT_WK_SetDrawEnable( p_clwk,TRUE );
    }
    if( !KEY_SYSTEM_SAVE_IsUse( p_wk->p_key_sv, KEY_SYSTEM_TYPE_DIFFICULT_HARD ) )
    {
      GFL_CLWK *p_clwk = KeySys_Obj_GetClwk( p_wk->p_obj, KEYSYS_CLWK_LOCK_00+KEY_SYSTEM_DIFFICULT_HARD );
      GFL_CLACT_WK_SetDrawEnable( p_clwk,TRUE );
    }

    {
      GFL_CLACTPOS pos;

      GFL_CLWK *p_clwk = KeySys_Obj_GetClwk( p_wk->p_obj, KEYSYS_CLWK_MARK );
      KEY_SYSTEM_DIFFICULT diffcult = KEY_SYSTEM_SAVE_GetDifficult( p_wk->p_key_sv );

      pos.x = (p_param->wnd_data[diffcult].x-2)*8;
      pos.y = (p_param->wnd_data[diffcult].y+1)*8;
      GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DRAW_MAIN );
      GFL_CLACT_WK_SetDrawEnable( p_clwk,TRUE );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  Aボタンを押したあとの決定コールバック
 *
 *	@param	int index   押したウィンドウのインデックス
 *	@param	*p_wk_adrs  ワークアドレス
 *
 *	@return TRUE決定  FALSE押せない
 */
//-----------------------------------------------------------------------------
static KEYSYS_SELECT_MENU_RET KeySys_SelMenu_Difficulty_DecideCallBack( int index, void *p_wk_adrs )
{
  KEYSYS_SELECT_MENU *p_wk = p_wk_adrs;

  //選択済みは押せない
  if( KEY_SYSTEM_SAVE_GetDifficult( p_wk->p_key_sv ) == index )
  {
    return KEYSYS_SELECT_MENU_RET_SELECTED;
  }
  else if( index < KEY_SYSTEM_DIFFICULT_MAX )
  {
    //ロックされてたら押せない
    switch( index )
    {
    case KEY_SYSTEM_DIFFICULT_EASY:
      if( !KEY_SYSTEM_SAVE_IsUse( p_wk->p_key_sv, KEY_SYSTEM_TYPE_DIFFICULT_EASY ) )
      {
        return KEYSYS_SELECT_MENU_RET_UNSELECT;
      }
      break;
    case KEY_SYSTEM_DIFFICULT_HARD:
      if( !KEY_SYSTEM_SAVE_IsUse( p_wk->p_key_sv, KEY_SYSTEM_TYPE_DIFFICULT_HARD ) )
      {
        return KEYSYS_SELECT_MENU_RET_UNSELECT;
      }
      break;
    }

    //押した！
    KEY_SYSTEM_SAVE_SetDifficult( p_wk->p_key_sv, index );

    //マークの位置変更
    {
      const KEYSYS_MENU_PARAM *cp_param = KEYSYS_MENU_GetParam( *p_wk->pp_menu );

      GFL_CLACTPOS pos;

      GFL_CLWK *p_clwk = KeySys_Obj_GetClwk( p_wk->p_obj, KEYSYS_CLWK_MARK );

      pos.x = (cp_param->wnd_data[index].x-2)*8;
      pos.y = (cp_param->wnd_data[index].y+1)*8;
      GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DRAW_MAIN );
      GFL_CLACT_WK_SetDrawEnable( p_clwk,TRUE );
    }
  }

  return KEYSYS_SELECT_MENU_RET_DECIDE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タイプ別初期化
 *
 *	@param	KEYSYS_SELECT_MENU *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_SelMenu_Wfbc_Init( KEYSYS_SELECT_MENU *p_wk, KEYSYS_MENU_PARAM *p_param )
{
  p_param->max      = 3;
  p_param->wnd_data[0].strID  = GET_VERSION() == VERSION_W2 ?  KEYSYS_STR_11: KEYSYS_STR_10;
  p_param->wnd_data[1].strID  = GET_VERSION() == VERSION_W2 ?  KEYSYS_STR_10: KEYSYS_STR_11;
  p_param->wnd_data[2].strID  = KEYSYS_STR_06;
  p_wk->DecideCallBack = KeySys_SelMenu_Wfbc_DecideCallBack;

  //OBJの設定
  {
    if( !KEY_SYSTEM_SAVE_IsUse( p_wk->p_key_sv, KEY_SYSTEM_TYPE_CITY_WFBC) )
    {
      GFL_CLWK *p_clwk = KeySys_Obj_GetClwk( p_wk->p_obj, KEYSYS_CLWK_LOCK_01 );
      GFL_CLACT_WK_SetDrawEnable( p_clwk,TRUE );
    }

    {
      GFL_CLACTPOS pos;

      GFL_CLWK *p_clwk = KeySys_Obj_GetClwk( p_wk->p_obj, KEYSYS_CLWK_MARK );
      KEY_SYSTEM_WFBC wfbc = KEY_SYSTEM_SAVE_GetWfbc( p_wk->p_key_sv );

      pos.x = (p_param->wnd_data[wfbc].x-2)*8;
      pos.y = (p_param->wnd_data[wfbc].y+1)*8;
      GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DRAW_MAIN );
      GFL_CLACT_WK_SetDrawEnable( p_clwk,TRUE );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  Aボタンを押したあとの決定コールバック
 *
 *	@param	int index   押したウィンドウのインデックス
 *	@param	*p_wk_adrs  ワークアドレス
 *
 *	@return TRUE決定  FALSE押せない
 */
//-----------------------------------------------------------------------------
static KEYSYS_SELECT_MENU_RET KeySys_SelMenu_Wfbc_DecideCallBack( int index, void *p_wk_adrs )
{
  KEYSYS_SELECT_MENU *p_wk = p_wk_adrs;
  KEY_SYSTEM_WFBC wfbc = KEY_SYSTEM_SAVE_GetWfbc( p_wk->p_key_sv );

  //選択済みは押せない
  if( wfbc == index )
  {
    return KEYSYS_SELECT_MENU_RET_SELECTED;
  }
  else if( index < KEY_SYSTEM_WFBC_MAX )
  {
    const LOCATION *cp_locate = GAMEDATA_GetStartLocation(p_wk->p_gamedata);

    //今WFBCにいるなら押せない
    if( ZONEDATA_IsWfbc( ZONEDATA_GetGroupID(cp_locate->zone_id) ) )
    {
      return KEYSYS_SELECT_MENU_RET_SP_UNSELECT;
    }

    //決定
    if( index == KEY_SYSTEM_WFBC_REVERSE
    && !KEY_SYSTEM_SAVE_IsUse( p_wk->p_key_sv, KEY_SYSTEM_TYPE_CITY_WFBC ) )
    {
      return KEYSYS_SELECT_MENU_RET_UNSELECT;
    }

    //押した
    KEY_SYSTEM_SAVE_SetWfbc( p_wk->p_key_sv, index );

    //マークの位置変更
    {
      const KEYSYS_MENU_PARAM *cp_param = KEYSYS_MENU_GetParam( *p_wk->pp_menu );

      GFL_CLACTPOS pos;

      GFL_CLWK *p_clwk = KeySys_Obj_GetClwk( p_wk->p_obj, KEYSYS_CLWK_MARK );

      pos.x = (cp_param->wnd_data[index].x-2)*8;
      pos.y = (cp_param->wnd_data[index].y+1)*8;
      GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DRAW_MAIN );
      GFL_CLACT_WK_SetDrawEnable( p_clwk,TRUE );
    }
  }

  return KEYSYS_SELECT_MENU_RET_DECIDE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  デモワーク初期化
 *
 *	@param	KEYSYS_DEMO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_Demo_Init( KEYSYS_DEMO_WORK *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof( KEYSYS_DEMO_WORK ) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  難易度設定の説明文取得
 *
 *	@param	const KEY_SYSTEM_SAVE *cp_save キーシステムセーブ
 *
 *	@return メッセージ
 */
//-----------------------------------------------------------------------------
static u16 KeySys_Mes_GetDifficultInfoMessage( const KEY_SYSTEM_SAVE *cp_save )
{
  KEY_SYSTEM_DIFFICULT diffcult = KEY_SYSTEM_SAVE_GetDifficult( cp_save );
  return KEYSYS_STR_23 + diffcult;
}
//----------------------------------------------------------------------------
/**
 *	@brief  街設定の説明文取得
 *
 *	@param	const KEY_SYSTEM_SAVE *cp_save  キーシステムセーブ
 *
 *	@return メッセージ
 */
//-----------------------------------------------------------------------------
static u16 KeySys_Mes_GetWfbcInfoMessage( const KEY_SYSTEM_SAVE *cp_save )
{
  KEY_SYSTEM_WFBC wfbc = KEY_SYSTEM_SAVE_GetWfbc( cp_save );
  if( GET_VERSION() == VERSION_W2 )
  {
    return KEYSYS_STR_27 - wfbc;
  }
  else
  {
    return KEYSYS_STR_26 + wfbc;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ロックリストの説明文取得
 *
 *	@param	const KEY_SYSTEM_SAVE *cp_save  キーシステムセーブ
 *	@param	cursor  カーソル
 *
 *	@return メッセージ
 */
//-----------------------------------------------------------------------------
static u16 KeySys_Mes_GetLockListInfoMessage( const KEY_SYSTEM_SAVE *cp_save, u16 cursor )
{
  switch( cursor )
  {
  case 0:
    return KeySys_Mes_GetDifficultInfoMessage( cp_save );
  case 1:
    return KeySys_Mes_GetWfbcInfoMessage( cp_save );
  case 2:
    return KEYSYS_STR_28;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  キーのメッセージを作成（p_wk->p_strburに格納）
 *
 *	@param	const KEY_SYSTEM_WORK *p_wk  ワーク
 *	@param	index     貰ったキーインデックス  0なら簡単キー 1なら難易度キー 2なら街キー
 *	@param  recv_or_send TRUEなら受信 FALSEなら送信
 *
 *	@return TRUE貰った  FALSE貰っていない
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Mes_CreateKeyMessage( KEY_SYSTEM_WORK *p_wk, KEY_SYSTEM_TYPE index, BOOL recv_or_send )
{
  STRBUF *p_str;
  BOOL is_ok = FALSE;

  GF_ASSERT( index < KEY_SYSTEM_TYPE_MAX );

  if( recv_or_send )
  {
    //受信
    is_ok = ( p_wk->key_data[ KEYSYS_DATA_SELF ].is_use[ KEY_SYSTEM_TYPE_DIFFICULT_EASY + index ] == FALSE
    && p_wk->key_data[ KEYSYS_DATA_OTHER ].is_have[ KEY_SYSTEM_TYPE_DIFFICULT_EASY + index ] == TRUE );
  }
  else
  {
    //送信
    is_ok = ( p_wk->key_data[ KEYSYS_DATA_SELF ].is_have[ KEY_SYSTEM_TYPE_DIFFICULT_EASY + index ] == TRUE
    && p_wk->key_data[ KEYSYS_DATA_OTHER ].is_use[ KEY_SYSTEM_TYPE_DIFFICULT_EASY + index ] == FALSE );
  }

  if( index == KEY_SYSTEM_TYPE_CITY_WFBC )
  {
    if( p_wk->key_data[KEYSYS_DATA_SELF].version == p_wk->key_data[KEYSYS_DATA_OTHER].version )
    {
      is_ok = FALSE;
    }
  }

  if( is_ok )
  {
    u16 strID;
    if( p_wk->player_data[ KEYSYS_DATA_OTHER ].is_save_exist )
    {
      strID = recv_or_send ? KEYSYS_STR_39: KEYSYS_STR_32;
    }
    else
    {
      strID = recv_or_send ? KEYSYS_STR_42: KEYSYS_STR_35;
    }

    if( index == KEY_SYSTEM_TYPE_CITY_WFBC )
    {
      //メッセージが、簡単キー、難しいキー、WFキー、BCキーの順番なので、
      //ブラックの場合はインデックスをひとつずらしてやる
      if( GET_VERSION() == VERSION_W2 )
      {
        index = 2;
      }
      else
      {
        index = 3;
      }
    }

    strID += index;


    p_str = GFL_MSG_CreateString( p_wk->p_msg, strID );

    if( p_wk->player_data[ KEYSYS_DATA_OTHER ].is_save_exist )
    {
      WORDSET_RegisterPlayerName( p_wk->p_word, 0, (const MYSTATUS*)p_wk->player_data[KEYSYS_DATA_OTHER].mystatus_buff );
    }
    WORDSET_ExpandStr( p_wk->p_word, p_wk->p_strbuf, p_str );
    GFL_STR_DeleteBuffer( p_str );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  赤外線からワイヤレスに変わった瞬間のコールバック
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_NetCallBack_IrcEnd( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;
  p_wk->ret = TRUE;
}
