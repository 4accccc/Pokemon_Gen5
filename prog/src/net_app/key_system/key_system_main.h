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
#include "multiboot/mb_parent_dataconv_sys.h"

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
  KEYSYS_BG_FRAME_TEXT_M  = GFL_BG_FRAME0_M,
  KEYSYS_BG_FRAME_WND_M   = GFL_BG_FRAME2_M,
  KEYSYS_BG_FRAME_BACK_M  = GFL_BG_FRAME3_M,
  KEYSYS_BG_FRAME_TEXT_S  = GFL_BG_FRAME0_S,
  KEYSYS_BG_FRAME_SUB_S   = GFL_BG_FRAME1_S,
  KEYSYS_BG_FRAME_WND_S   = GFL_BG_FRAME2_S,
  KEYSYS_BG_FRAME_BACK_S  = GFL_BG_FRAME3_S,
};

//パレット使用場所
enum
{
  KEYSYS_PLT_BG_BACK_M    = 0,
  KEYSYS_PLT_BG_MENU_NORMAL_M,
  KEYSYS_PLT_BG_MENU_SELECT_A_M,
  KEYSYS_PLT_BG_MENU_SELECT_B_M,
  KEYSYS_PLT_BG_FONT_M    = 0xE,
  KEYSYS_PLT_BG_FRAME_M   = 0xF,

  KEYSYS_PLT_BG_BACK_S    = 0,
  KEYSYS_PLT_BG_DLPLAY_S  = 9,
  KEYSYS_PLT_BG_DLMSG_S   = 0xD,
  KEYSYS_PLT_BG_FONT_S    = 0xE,

  KEYSYS_PLT_OBJ_COMMON_M = 0,
  KEYSYS_PLT_OBJ_BARICON_M,
  KEYSYS_PLT_OBJ_BARICON_A_M,
  KEYSYS_PLT_OBJ_FONT_M,
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
typedef enum
{
  KEYSYS_RES_OBJ_NCL,
  KEYSYS_RES_OBJ_NCG,
  KEYSYS_RES_OBJ_NCE,
  KEYSYS_RES_MAX,
}KEYSYS_RES;

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
  KEYSYS_CLWK_WINDOW,
  KEYSYS_CLWK_KIRAKIRA,
  KEYSYS_CLWK_LOCK_OPEN,
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
  HEAPID heapID;
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
#define KEYSYS_MAP_MAX (5)
#define KEYSYS_MAP_KEY_MAX (4)
typedef struct
{
  struct
  {
    u32 value;
    char key[KEYSYS_MAP_KEY_MAX];
  }data[KEYSYS_MAP_MAX];
} KEYSYS_MAP;

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
typedef struct _KEYSYS_DEMO_WORK
{
  KEYSYS_TEXT_WORK    *p_text;
  KEYSYS_OBJ_WORK     *p_obj;
  GFL_CLWK            *p_key;
  GFL_CLWK            *p_effect;
  GFL_CLWK            *p_window;
  GFL_CLWK            *p_lock;
  KEYSYS_TEXTOBJ_WORK *p_txtobj;
  BMPOAM_SYS_PTR      p_bmpoam;
  u32                 seq;
  u32                 cnt;
  u16                 menu_msg;
  STRBUF              *p_info_msg;
  KEY_SYSTEM_TYPE     key;
  BOOL                is_start;
  GFL_MSGDATA         *p_msg;
  GFL_FONT            *p_font;

  KEYSYS_VELOCITY_WORK  vel;
  KEYSYS_ACLR_WORK      aclr;
  u32                   all_cnt;
  BOOL (*Update)( struct _KEYSYS_DEMO_WORK *p_wk );
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

  KEYSYS_BG_WORK          *p_bg;

  KEYSYS_OBJ_WORK         obj;

  KEYSYS_MAP              select;

  KEYSYS_SELECT_MENU      selmenu;

  OUTSIDE_SAVE_CONTROL    *p_outside_sv;

  KEY_SYSTEM_SAVE         *p_key_sv;

  DATA_CONVERT_SAVE       *p_data_sv;

  WB_SAVE_CONV_ALL_DATA   *p_recv_data;

  KEYSYS_NET_WORK         *p_net;

  MB_PARENT_DATACONV      *p_mb;

  KEYSYS_KEY_DATA         key_data[ KEYSYS_DATA_MAX ];

  KEYSYS_PLAYER_DATA      player_data[ KEYSYS_DATA_MAX ];

  KEYSYS_DEMO_WORK        demo;

  u32                     ret;  //選択項目一時保存場所

  u32                     cnt;

  STRBUF                  *p_strbuf;

  BOOL                    is_parent;
  BOOL                    is_wifi_start;
  STRBUF                  *p_gamesync_id_code;

  void                    *p_proc_param;

  void                    *p_download_data;

}KEY_SYSTEM_WORK;

//-------------------------------------
/// SEQ
//=====================================
extern void KeySys_Seq_FadeInBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
extern void KeySys_Seq_FadeOutBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
extern void KeySys_Seq_FadeOutWhite( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
extern void KeySys_Seq_DeleteDisplay( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
/// BG
//=====================================
extern KEYSYS_BG_WORK * KEYSYS_BG_Init( HEAPID heapID );
extern void KEYSYS_BG_Exit( KEYSYS_BG_WORK *p_wk );
extern void KEYSYS_BG_Load( KEYSYS_BG_WORK *p_wk, HEAPID heapID );
extern void KEYSYS_BG_Main( KEYSYS_BG_WORK *p_wk );

typedef enum
{
  KEYSYS_BG_SCR_TITLE_M,    //タイトル表紙だけ
  KEYSYS_BG_SCR_HAVEKEY_M,  //タイトル＋所持キー
  KEYSYS_BG_SCR_HAVEKEY_S,  //所持キーだけ
}KEYSYS_BG_SCR;
extern void KEYSYS_BG_TransScreen( KEYSYS_BG_WORK *p_wk, int frm, KEYSYS_BG_SCR scr );

//-------------------------------------
/// OBJ
//=====================================
extern void KEYSYS_OBJ_Init( KEYSYS_OBJ_WORK *p_wk, const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic, HEAPID heapID );
extern void KEYSYS_OBJ_Load( KEYSYS_OBJ_WORK *p_wk, HEAPID heapID );
extern void KEYSYS_OBJ_Release( KEYSYS_OBJ_WORK *p_wk );
extern GFL_CLWK * KEYSYS_OBJ_AddClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id, HEAPID heapID );
extern void KEYSYS_OBJ_RemoveClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id );
extern GFL_CLWK * KEYSYS_OBJ_GetClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id );
extern u32 KEYSYS_OBJ_GetResource( const KEYSYS_OBJ_WORK *cp_wk, KEYSYS_RES res );
extern GFL_CLUNIT * KEYSYS_OBJ_GetClunit( KEYSYS_OBJ_WORK *p_wk );

//-------------------------------------
/// MAP
//=====================================
extern void KEYSYS_MAP_Init( KEYSYS_MAP *p_wk, HEAPID heapID );
extern void KEYSYS_MAP_Insert( KEYSYS_MAP *p_wk, const char *cp_key, u32 value );
extern void KEYSYS_MAP_Erase( KEYSYS_MAP *p_wk, const char *cp_key );
extern BOOL KEYSYS_MAP_IsExist( const KEYSYS_MAP *cp_wk, const char *cp_key );
extern u32 KEYSYS_MAP_GetValue( const KEYSYS_MAP *cp_wk, const char *cp_key );
extern void KEYSYS_MAP_Clear( KEYSYS_MAP *p_wk );

//-------------------------------------
/// ETC
//=====================================
extern void KEYSYS_InitTemporaryModules( KEY_SYSTEM_WORK *p_wk, HEAPID heapID );
extern void KEYSYS_ExitTemporaryModules( KEY_SYSTEM_WORK *p_wk );
