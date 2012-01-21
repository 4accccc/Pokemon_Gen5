//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\test\cygnus\d_cygnus.c
 *	@brief  CYGNUS用デバッグ機能
 *	@author	tamada
 *	@date		2012.01.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG

#include <wchar.h>    // wcslen
#include <gflib.h>
#include "debug_cygnus_menu_setup.h"
#include "system/main.h"
#include "arc/arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "print/printsys.h"
#include "gamesystem/game_data.h"
#include "item/itemsym.h"             //ITEM_DATA_MAX

#include "savedata/save_tbl.h"
#include "savedata/cygnus_data.h"
#include "savedata/dummy3ds_savedata.h"

#include "savedata/cygnus_save_control.h"

FS_EXTERN_OVERLAY(cygnus_debug);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
// 選択状態
typedef enum
{
  SELECT_STATE_NONE,    // 未選択
  SELECT_STATE_DECIDE,  // 決定
  SELECT_STATE_EXIT,    // 終了
} SELECT_STATE;

typedef enum {
  PAGE_0,
  PAGE_1,

  PAGE_NUM,
}PAGE_ID;

// メニュー項目インデックス
typedef enum
{
  MENU_ITEM_STATE_FIRST,
  MENU_ITEM_STATE_CYGNUS_READY,
  MENU_ITEM_STATE_CYGNUS_PUT,
  MENU_ITEM_CLEAR_GIFT,
  MENU_ITEM_SPPOKE_CTRL,
  MENU_ITEM_ADD_POKE,

  MENU_ITEM_ADD_ITEM,
  MENU_ITEM_ONETIMEKEY,
  MENU_ITEM_SWANFLAGS,
  MENU_ITEM_PRINT,
  MENU_ITEM_EXIT,

  MENU_ITEM_NUM
} MENU_ITEM;

enum {
  //1ページあたりの項目数
  MENU_ITEM_PER_PAGE = 6,
  //ページ数
  MENU_PAGE_NUM = MATH_ROUNDUP( MENU_ITEM_NUM, MENU_ITEM_PER_PAGE ) / MENU_ITEM_PER_PAGE,
};

// メニュー項目文字列
static const STRCODE* str_menu[] =
{
  L"A.未初期化状態にする",
  L"B.CYGNUS書き込み可能にする（受取る）",
  L"C.SWAN受け取り可能にする（渡す）",
  L"渡すものを　クリア",
  L"渡す伝説ポケ　操作",
  L"渡すポケモン　追加",
  L"渡すどうぐ　追加",

  L"SWAN側ワンタイムキー操作",
  L"SWAN側フラグ操作",
  L"デバッグ表示",
  L"戻る",
};
SDK_COMPILER_ASSERT( MENU_ITEM_NUM == NELEMS(str_menu) );

typedef enum {
  STAT_NONE,
  STAT_ERASE,
  STAT_SWAN_WRITE_SUCCESS,
  STAT_SWAN_WRITE_FAILURE,
  STAT_CYGNUS_WRITE_SUCCESS,
  STAT_CYGNUS_WRITE_FAILURE,
  STAT_CYGNUS_FLAGS_FAILURE,
  STAT_CYGNUS_EMPTY_FAILURE,
  STAT_CYGNUS_CLEAR_GIFTS,

  STAT_NUM,
}LAST_STATUS;

static const STRCODE * const str_state[] = 
{
  L"----",
  L"消去",
  L"SWANから書き込み成功",
  L"SWANから書き込み失敗",
  L"CYGNUSから書き込み成功",
  L"CYGNUSから書き込み失敗",
  L"FLAG指定により渡せない",
  L"渡すものがない",
  L"渡すものをクリア",
};

SDK_COMPILER_ASSERT( STAT_NUM == NELEMS(str_state) );


static const struct {
  u32 flag_id;
}SPPOKE_TABLE[] = {
  {
    CYGFLAG_SP_POKE01,
  },
  {
    CYGFLAG_SP_POKE02,
  },
  {
    CYGFLAG_SP_POKE03,
  },
  {
    CYGFLAG_SP_POKE04,
  },
  {
    CYGFLAG_SP_POKE05,
  },
  {
    CYGFLAG_SP_POKE06,
  },
  {
    CYGFLAG_SP_POKE07,
  },
  {
    CYGFLAG_SP_POKE08,
  },
};

enum {
  CYGNUS_ITEM_NUM = 6,
  CYGNUS_POKE_NUM = 6,
};

#define STRONG_LSB  PRINTSYS_LSB_Make( 1, 3, 0 )
#define NORMAL_LSB  PRINTSYS_LSB_Make( 1, 2, 0 )
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	プロセスワーク
//=====================================
typedef struct {
  GFL_FONT*   font;
  GFL_MSGDATA*  itemname_man;
  GFL_MSGDATA*  pokename_man;
  STRBUF * tmpBuf;
  GFL_BMPWIN* menuBmpWin;
  GFL_BMPWIN* subBmpWin;
  int         page;
  int         curPos; // 選択項目
  SELECT_STATE state; // 選択状態
	SAVE_CONTROL_WORK* pSaveData;
  GAMEDATA      *pGameData;
  CYGNUS_SAVE_CONTROL * cygsave_ctrl;
  CYGNUS_SAVEDATA * cygsave;
  DUMMY3DS_SAVEDATA * inner_save;

  u32 cygBuf[ CYGDATA_AREA_SIZE ];
  u32 swan_flags;
  u32 swan_key;

  int subSeq;
  int subCurPos;
  LAST_STATUS last_status;

  u32 cygnus_sppoke_bits;
  u32 cygnus_poke[ CYGNUS_POKE_NUM ];
  u32 cygnus_item[ CYGNUS_ITEM_NUM ];

  //flag control;
  int bit_pos;
  u32 len;
  u32 flag_value;

  //add item
  int sel_item_pos;
  u16 item_id;
  u16 item_num;

  //add poke
  int sel_mons_pos;
  u16 monsno;
  u16 form_no;
  u16 sex;
} PROC_WORK;

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

static void Init( PROC_WORK *p_wk );
static void Exit( PROC_WORK *p_wk );
static void Main( PROC_WORK *p_wk );
static void UpdateMenuList( PROC_WORK *p_wk );
static void UpdateCygnusInfo( PROC_WORK * p_wk );
static BOOL CallSubSeq( u32 id, PROC_WORK *p_wk );

static void UpdateSpPokeList( PROC_WORK *p_wk );

static void CygnusLoad( PROC_WORK * p_wk );
static void CygnusUnload( PROC_WORK * p_wk );

// 処理関数
// メニューの数と同じ

static BOOL SubSeqCygStatusPrint( PROC_WORK * p_wk );
static BOOL SubSeqStateFirst( PROC_WORK * p_wk );
static BOOL SubSeqStateCygnusReady( PROC_WORK * p_wk );
static BOOL SubSeqStateCygnusPut( PROC_WORK * p_wk );
static BOOL SubSeqClearGifts( PROC_WORK * p_wk );
static BOOL SubSeqSpPokeControl( PROC_WORK *p_wk );
static BOOL ControlSwanFlagValue( PROC_WORK * p_wk );
static BOOL SubSeqAddItem( PROC_WORK *p_wk );
static BOOL SubSeqAddPokemon( PROC_WORK *p_wk );
static BOOL SubSeqControlOneTimeKey( PROC_WORK * p_wk );

//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_CYGNUS_MainProcFunc_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  PROC_WORK* p_wk = NULL;

  // ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PGL_DEBUG, 0x80000 );

  // ワークを作成
  p_wk = GFL_PROC_AllocWork( proc, sizeof( PROC_WORK ), HEAPID_PGL_DEBUG );
  p_wk->pGameData = GAMEDATA_Create( HEAPID_PGL_DEBUG );
  p_wk->pSaveData = SaveControl_GetPointer();  //デバッグ
  p_wk->inner_save = SaveData_GetDummy3ds( p_wk->pSaveData );

  // 初期化処理
  DEBUG_CYGNUS_MENU_SETUP_Init( HEAPID_PGL_DEBUG );
  Init( p_wk );

  return GFL_PROC_RES_FINISH;
}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_CYGNUS_MainProcFunc_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  PROC_WORK* p_wk = mywk;

  switch( p_wk->state )
  {
  case SELECT_STATE_NONE:
    p_wk->subSeq = 0;
    Main( p_wk );
    break;
  case SELECT_STATE_DECIDE:
    if ( p_wk->curPos == MENU_ITEM_EXIT )
    {
      return GFL_PROC_RES_FINISH;
    }
    // カーソルの位置で決定
    if ( CallSubSeq( p_wk->curPos, p_wk ) )
    {
      UpdateCygnusInfo( p_wk );
      UpdateMenuList( p_wk );
      OS_Printf("LAST_STATUS:%d\n", p_wk->last_status);
      p_wk->last_status = STAT_NONE;
      p_wk->subSeq = 0;
      p_wk->state = SELECT_STATE_NONE;
    }
    break;
  case SELECT_STATE_EXIT:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}


//============================================================================================
/**
 * @brief 終了関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_CYGNUS_MainProcFunc_End( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  PROC_WORK* p_wk = mywk;

  // 終了処理
  Exit( p_wk );
  DEBUG_CYGNUS_MENU_SETUP_Exit();

  GAMEDATA_Delete(p_wk->pGameData);

  // ワークを破棄
  GFL_PROC_FreeWork( proc );

  // ヒープを破棄
  GFL_HEAP_DeleteHeap( HEAPID_PGL_DEBUG );

  return GFL_PROC_RES_FINISH;
}


//============================================================================================
/**
 * @brief プロセス定義データ
 */
//============================================================================================
const GFL_PROC_DATA DebugCygnusProcData =
{
  DEBUG_CYGNUS_MainProcFunc_Init,
  DEBUG_CYGNUS_MainProcFunc_Main,
  DEBUG_CYGNUS_MainProcFunc_End,
};


//=============================================================================
/**
 *					static関数
*/
//=============================================================================

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void clearGifts( PROC_WORK * p_wk )
{
  p_wk->cygnus_sppoke_bits = 0;
  GFL_STD_MemClear( p_wk->cygnus_poke, sizeof(p_wk->cygnus_poke) );
  GFL_STD_MemClear( p_wk->cygnus_item, sizeof(p_wk->cygnus_item) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 */
//-----------------------------------------------------------------------------
static void Init( PROC_WORK *p_wk )
{
  // 初期化
  p_wk->page = 0;
  p_wk->curPos = 0;
  p_wk->state = SELECT_STATE_NONE;
  p_wk->subSeq = 0;
  p_wk->swan_flags = 
    DUMMY3DS_SAVEDATA_GetData( p_wk->inner_save, DUMMY3DS_SAVEDATA_PARAM_FLAG);
  p_wk->swan_key =
    DUMMY3DS_SAVEDATA_GetData( p_wk->inner_save, DUMMY3DS_SAVEDATA_PARAM_ONETIME_KEY);

  clearGifts( p_wk );

  p_wk->bit_pos = 0;

  // フォントデータハンドラ作成
  p_wk->font = GFL_FONT_Create(
      ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_PGL_DEBUG );
  // メッセージデータハンドラ作成
  p_wk->itemname_man = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_itemname_dat, HEAPID_PGL_DEBUG);
  p_wk->pokename_man = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monsname_dat, HEAPID_PGL_DEBUG);

  p_wk->tmpBuf = GFL_STR_CreateBuffer( 256, HEAPID_PGL_DEBUG );

  // ビットマップウィンドウを作成
  p_wk->menuBmpWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );
  p_wk->subBmpWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_S, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );

  // メニューリストの更新
  UpdateMenuList( p_wk );
  UpdateCygnusInfo( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了
 */
//-----------------------------------------------------------------------------
static void Exit( PROC_WORK *p_wk )
{
  // ビットマップウィンドウの破棄
  GFL_BMPWIN_Delete( p_wk->menuBmpWin );
  GFL_BMPWIN_Delete( p_wk->subBmpWin );

  GFL_STR_DeleteBuffer( p_wk->tmpBuf );

  GFL_MSG_Delete( p_wk->itemname_man );
  GFL_MSG_Delete( p_wk->pokename_man );
  // フォントデータハンドラ破棄
  GFL_FONT_Delete( p_wk->font );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メイン
 */
//-----------------------------------------------------------------------------
static void Main( PROC_WORK *p_wk )
{
  int trg = GFL_UI_KEY_GetTrg();
  int cursor_move = 0;

  if( trg & PAD_KEY_UP ) cursor_move = -1;
  if( trg & PAD_KEY_DOWN ) cursor_move = 1;
  if( trg & PAD_KEY_LEFT ) cursor_move = -MENU_ITEM_PER_PAGE;
  if( trg & PAD_KEY_RIGHT ) cursor_move = MENU_ITEM_PER_PAGE;
  if( trg & PAD_BUTTON_A ) p_wk->state = SELECT_STATE_DECIDE;
  if( trg & PAD_BUTTON_SELECT ) p_wk->state = SELECT_STATE_EXIT;

  // カーソルが移動した場合
  if( cursor_move != 0 )
  {
    // カーソル位置を補正
    p_wk->curPos = p_wk->curPos + cursor_move;
    if( p_wk->curPos < 0 ) p_wk->curPos += MENU_ITEM_NUM;
    p_wk->curPos = p_wk->curPos % MENU_ITEM_NUM;

    p_wk->page = p_wk->curPos / MENU_ITEM_PER_PAGE;

    // メニュー項目更新
    UpdateMenuList( p_wk );
  }
}

//============================================================================================
//
//    表示まわり
//
//============================================================================================
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void createString( STRBUF * strbuf, const STRCODE * src_str )
{
  int len;
  STRCODE strcode[256];

  len = wcslen( src_str );
  if (len >= 256 ) len = 255;
  GFL_STD_MemCopy( src_str, strcode, sizeof(STRCODE) * len );
  strcode[len] = GFL_STR_GetEOMCode();
  GFL_STR_SetStringCode( strbuf, strcode );
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void createU32Hex( STRBUF * strbuf, u32 value )
{
  int len;
  STRCODE strcode[256];

  swprintf( strcode, 16, L"%08x", value );
  strcode[8] = GFL_STR_GetEOMCode();
  GFL_STR_SetStringCode( strbuf, strcode );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メニューリスト更新
 */
//-----------------------------------------------------------------------------
static void UpdateMenuList( PROC_WORK *p_wk )
{
  // ビットマップウィンドウに文字を書き込む
  {
    int i;
    GFL_BMP_DATA* bmp;
    PRINTSYS_LSB lsb;

    // バッファ作成
    bmp = GFL_BMPWIN_GetBmp( p_wk->menuBmpWin );

    GFL_BMP_Clear( bmp, 0 );
    // 書き込み
    for( i=0; i<MENU_ITEM_PER_PAGE && i+p_wk->page * MENU_ITEM_PER_PAGE < MENU_ITEM_NUM; i++ )
    {
      int item_id = p_wk->page * MENU_ITEM_PER_PAGE + i;

      // 文字色決定
      if( item_id == p_wk->curPos )
      {
        lsb = STRONG_LSB;
      }
      else
      {
        lsb = NORMAL_LSB;
      }

      // 文字列作成
      createString( p_wk->tmpBuf, str_menu[item_id] );

      // ビットマップに書き込み
      PRINTSYS_PrintColor( bmp, 0, i*16, p_wk->tmpBuf, p_wk->font, lsb );

      // バッファ破棄
    }
    lsb = STRONG_LSB;
    createString( p_wk->tmpBuf, str_state[p_wk->last_status] );
    PRINTSYS_PrintColor( bmp, 30, 160, p_wk->tmpBuf, p_wk->font, lsb );
  }

  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( p_wk->menuBmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( p_wk->menuBmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void UpdateFlagsInfo( PROC_WORK * p_wk )
{
  u32 len = p_wk->len;
  u32 value = p_wk->flag_value;
  int i;
  PRINTSYS_LSB lsb = NORMAL_LSB;
  GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( p_wk->subBmpWin );

  GFL_BMP_Clear( bmp, 0 );
  for (i = 0; i< len; i ++ )
  {
    int bit = (value & (1 << i) ) == 0 ? 0 : 1;
    STRTOOL_SetNumber( p_wk->tmpBuf, bit, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    if ( i == p_wk->bit_pos )
    {
      lsb = STRONG_LSB;
    }
    else
    {
      lsb = NORMAL_LSB;
    }
    PRINTSYS_PrintColor( bmp, (len-i)*13, 0, p_wk->tmpBuf, p_wk->font, lsb );
  }

  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( p_wk->subBmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( p_wk->subBmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void UpdateCygnusInfo( PROC_WORK * p_wk )
{
  static const STRCODE * const info_str[] = {
    L"status",
    L"上書き許可",
    L"ワンタイムキー",
    L"マジックナンバー",
    L"情報フラグ",
  };
  static const STRCODE * const state_str[] = {
    L"A.未初期化状態",
    L"B.CYGNUS書込みOK",
    L"C.SWAN受取りOK",
  };
  SDK_COMPILER_ASSERT( NELEMS(state_str) == CYGSAVE_STATE_NUM );

  int i;
  PRINTSYS_LSB lsb = NORMAL_LSB;
  GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( p_wk->subBmpWin );

  GFL_BMP_Clear( bmp, 0 );
  for (i = 0; i< NELEMS(info_str); i ++ )
  {
    createString( p_wk->tmpBuf, info_str[i] );
    PRINTSYS_PrintColor( bmp, 0, i*16, p_wk->tmpBuf, p_wk->font, lsb );
  }

  {
    CygnusLoad( p_wk );
    createString( p_wk->tmpBuf, state_str[ CYGNUS_SAVEDATA_GetStatus( p_wk->cygsave ) ] );
    PRINTSYS_PrintColor( bmp, 18 * 7, 0 * 16, p_wk->tmpBuf, p_wk->font, lsb );
    createU32Hex( p_wk->tmpBuf, p_wk->cygsave->overwrite_enable_flag );
    PRINTSYS_PrintColor( bmp, 18 * 10, 1 * 16, p_wk->tmpBuf, p_wk->font, lsb );
    createU32Hex( p_wk->tmpBuf, p_wk->cygsave->onetime_key );
    PRINTSYS_PrintColor( bmp, 18 * 10, 2 * 16, p_wk->tmpBuf, p_wk->font, lsb );
    createU32Hex( p_wk->tmpBuf, p_wk->cygsave->magic_number );
    PRINTSYS_PrintColor( bmp, 18 * 10, 3 * 16, p_wk->tmpBuf, p_wk->font, lsb );
    createU32Hex( p_wk->tmpBuf, CYGNUS_SAVEDATA_DEBUG_GetFlags(p_wk->cygsave) );
    PRINTSYS_PrintColor( bmp, 18 * 10, 4 * 16, p_wk->tmpBuf, p_wk->font, lsb );
    CygnusUnload( p_wk );
  }


  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( p_wk->subBmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( p_wk->subBmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
}


//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
/**
 * @brie  サブシーケンスの呼び出し
 */
//-----------------------------------------------------------------------------
static BOOL CallSubSeq( u32 id, PROC_WORK *p_wk )
{
  switch( id ){
  case MENU_ITEM_STATE_FIRST:
    return SubSeqStateFirst( p_wk );
  case MENU_ITEM_STATE_CYGNUS_READY:
    return SubSeqStateCygnusReady( p_wk );
  case MENU_ITEM_PRINT:
    return SubSeqCygStatusPrint( p_wk );
  case MENU_ITEM_SPPOKE_CTRL:
    return SubSeqSpPokeControl( p_wk );
  case MENU_ITEM_CLEAR_GIFT:
    return SubSeqClearGifts( p_wk );
  case MENU_ITEM_ADD_POKE:
    return SubSeqAddPokemon( p_wk );
  case MENU_ITEM_ADD_ITEM:
    return SubSeqAddItem( p_wk );
  case MENU_ITEM_STATE_CYGNUS_PUT:
    return SubSeqStateCygnusPut( p_wk );
  case MENU_ITEM_ONETIMEKEY:
    return SubSeqControlOneTimeKey( p_wk );
  case MENU_ITEM_SWANFLAGS:
    return ControlSwanFlagValue( p_wk );
  }
  return TRUE;  //対応がない場合はすぐに終了
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void CygnusLoad( PROC_WORK * p_wk )
{
  p_wk->cygsave_ctrl = CYGNUSSAVE_CONTROL_Create( HEAPID_PGL_DEBUG );
  CYGNUSSAVE_CONTROL_Load( p_wk->cygsave_ctrl );
  p_wk->cygsave = CYGNUSSAVE_CONTROL_GetPtr( p_wk->cygsave_ctrl );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void CygnusUnload( PROC_WORK * p_wk )
{
  CYGNUSSAVE_CONTROL_Delete( p_wk->cygsave_ctrl );
  p_wk->cygsave = NULL;
  p_wk->cygsave_ctrl = NULL;
}

//-----------------------------------------------------------------------------
/**
 * @brief CYGNUSから送りつけるデータを生成する
 * @todo  特殊ポケモン以外もフラグで抑止するのか検討
 */
//-----------------------------------------------------------------------------
static LAST_STATUS CygnusBufSet( PROC_WORK * p_wk )
{
  int buf_id;
  BOOL add_flag = FALSE;

  //バッファクリア
  GFL_STD_MemClear( p_wk->cygBuf, sizeof(u32) * CYGDATA_AREA_SIZE );

  //特殊ポケモンのバッファ設定
  for ( buf_id = 0; buf_id < NELEMS(SPPOKE_TABLE); buf_id ++ )
  {
    if ( p_wk->cygnus_sppoke_bits & (1 << buf_id ) )
    {
      if ( CYGNUS_SAVEDATA_DEBUG_GetFlags( p_wk->cygsave ) & (1 << buf_id) )
      { //swan側から許可されていない場合は失敗
        return STAT_CYGNUS_FLAGS_FAILURE;
      }
      {
        u32 flag = SPPOKE_TABLE[buf_id].flag_id;
        p_wk->cygBuf[ buf_id ] = CYGNUS_DATA_GetMagicNumByFlag( flag );
        OS_Printf("Cygnus Add Sp Poke(%d) %08x\n", buf_id, p_wk->cygBuf[ buf_id ] );
        add_flag = TRUE;
      }
    }
  }

  //通常ポケモンの追加
  for ( buf_id = 0; buf_id < CYGNUS_POKE_NUM; buf_id ++ )
  {
    if ( p_wk->cygnus_poke[buf_id] == 0 ) break;
    OS_Printf("Cygnus Add Poke(%d) %08x\n", buf_id, p_wk->cygnus_poke[buf_id] );
    p_wk->cygBuf[ CYGDATA_POKE01 + buf_id ] = p_wk->cygnus_poke[buf_id];
    add_flag = TRUE;
  }

  //どうぐの追加
  for ( buf_id = 0; buf_id < CYGNUS_ITEM_NUM; buf_id ++ )
  {
    if ( p_wk->cygnus_item[buf_id] == 0 ) break;
    OS_Printf("Cygnus Add Item(%d) %08x\n", buf_id, p_wk->cygnus_item[buf_id] );
    p_wk->cygBuf[ CYGDATA_ITEMID01 + buf_id ] = p_wk->cygnus_item[buf_id];
    add_flag = TRUE;
  }

  if ( add_flag ) return STAT_CYGNUS_WRITE_SUCCESS;
  return STAT_CYGNUS_EMPTY_FAILURE;
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL SubSeqCygStatusPrint( PROC_WORK * p_wk )
{
  u32 onetime_key = p_wk->swan_key;
  CYGSAVE_STATE state;
  CygnusLoad( p_wk );
  OS_Printf("overwrite_enable_flag: %d\n", p_wk->cygsave->overwrite_enable_flag );
  OS_Printf("onetime_key: %08x\n", p_wk->cygsave->onetime_key );
  OS_Printf("magic_number: %08x\n", p_wk->cygsave->magic_number );
  OS_Printf("flags: %08x\n", CYGNUS_SAVEDATA_DEBUG_GetFlags( p_wk->cygsave ) );
  state = CYGNUS_SAVEDATA_Receive( p_wk->cygsave, onetime_key, p_wk->cygBuf );
  {
    int i,j;
    for (i = 0; i < 4; i ++ )
    {
      for (j = 0; j < 8; j ++ )
      {
        OS_Printf("%08x,", p_wk->cygBuf[ i * 8 + j ] );
      }
      OS_PutString("\n");
    }
  }
  CygnusUnload( p_wk );
  return TRUE;
}
//-----------------------------------------------------------------------------
/// サブシーケンス：工場出荷状態にする
//-----------------------------------------------------------------------------
static BOOL SubSeqStateFirst( PROC_WORK * p_wk )
{
  switch ( p_wk->subSeq )
  {
  case 0:
    CygnusLoad( p_wk );
    p_wk->subSeq ++;
    break;
  case 1:
    CYGNUS_SAVEDATA_Init( p_wk->cygsave );
    CYGNUSSAVE_CONTROL_SaveAsyncInit( p_wk->cygsave_ctrl );
    p_wk->subSeq ++;
    break;
  case 2:
    OS_Printf("ERASE BACKUP...\n");
    if ( CYGNUSSAVE_CONTROL_SaveAsyncMain( p_wk->cygsave_ctrl ) == TRUE )
    {
      CygnusUnload( p_wk );
      p_wk->last_status = STAT_ERASE;
      return TRUE;  //終了
    }
    break;
  }
  return FALSE;
}
//-----------------------------------------------------------------------------
/// サブシーケンス：Cygnusが書き込める状態にする
//-----------------------------------------------------------------------------
static BOOL SubSeqStateCygnusReady( PROC_WORK * p_wk )
{
  switch ( p_wk->subSeq )
  {
  case 0:
    CygnusLoad( p_wk );
    p_wk->subSeq ++;
    break;
  case 1:
    if ( CYGNUS_SAVEDATA_GetStatus( p_wk->cygsave ) == CYGSAVE_STATE_SEND )
    {
      CygnusUnload( p_wk );
      p_wk->last_status = STAT_SWAN_WRITE_FAILURE;
      return TRUE;
    }
    CYGNUS_SAVEDATA_SetSend( p_wk->cygsave, p_wk->swan_key, p_wk->swan_flags  );
    CYGNUSSAVE_CONTROL_SaveAsyncInit( p_wk->cygsave_ctrl );
    p_wk->subSeq ++;
    break;
  case 2:
    OS_Printf("WRITING FROM SWAN...\n");
    if ( CYGNUSSAVE_CONTROL_SaveAsyncMain( p_wk->cygsave_ctrl ) == TRUE )
    {
      CygnusUnload( p_wk );
      p_wk->last_status = STAT_SWAN_WRITE_SUCCESS;
      return TRUE;  //終了
    }
    break;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
/// サブシーケンス：渡すものをクリア
//-----------------------------------------------------------------------------
static BOOL SubSeqClearGifts( PROC_WORK * p_wk )
{
  clearGifts( p_wk );
  p_wk->last_status = STAT_CYGNUS_CLEAR_GIFTS;
  return TRUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief サブシーケンス：CYGNUSから送るデータの生成、書き込み
 */
//-----------------------------------------------------------------------------
static BOOL SubSeqStateCygnusPut( PROC_WORK * p_wk )
{
  LAST_STATUS get_status;

  switch ( p_wk->subSeq )
  {
  case 0:
    CygnusLoad( p_wk );
    p_wk->subSeq ++;
    break;
  case 1:
    if ( CYGNUS_SAVEDATA_GetStatus( p_wk->cygsave ) != CYGSAVE_STATE_SEND )
    { //CYGNUS書き込み許可状態でない場合は失敗
      CygnusUnload( p_wk );
      p_wk->last_status = STAT_CYGNUS_WRITE_FAILURE;
      return TRUE;
    }

    get_status = CygnusBufSet( p_wk );
    if ( get_status != STAT_CYGNUS_WRITE_SUCCESS )
    { //何も渡せなかった場合は失敗
      CygnusUnload( p_wk );
      p_wk->last_status = get_status;
      return TRUE;
    }
    CYGNUS_SAVEDATA_DEBUG_Write( p_wk->cygsave, p_wk->cygBuf );
    CYGNUSSAVE_CONTROL_SaveAsyncInit( p_wk->cygsave_ctrl );
    p_wk->subSeq ++;
    break;
  case 2:
    OS_Printf("WRITING FROM CYGNUS...\n");
    if ( CYGNUSSAVE_CONTROL_SaveAsyncMain( p_wk->cygsave_ctrl ) == TRUE )
    {
      CygnusUnload( p_wk );
      //渡したものをクリア
      clearGifts( p_wk );
      p_wk->last_status = STAT_CYGNUS_WRITE_SUCCESS;
      return TRUE;  //終了
    }
    break;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef enum {
  CTRL_FLAG_RESULT_CONTINUE = 0,
  CTRL_FLAG_RESULT_DECIDE,
  CTRL_FLAG_RESULT_CANCEL,
}CTRL_FLAG_RESULT;

//-----------------------------------------------------------------------------
/// フラグ操作  初期化
//-----------------------------------------------------------------------------
static void controlFlagInit( PROC_WORK * p_wk, u32 value, u32 len )
{
  p_wk->bit_pos = 0;
  p_wk->len = len;
  p_wk->flag_value = value;
  UpdateFlagsInfo( p_wk );
}
//-----------------------------------------------------------------------------
/// フラグ操作  メイン
//-----------------------------------------------------------------------------
static CTRL_FLAG_RESULT controlFlagMain( PROC_WORK * p_wk )
{
  int trg = GFL_UI_KEY_GetTrg();
  int value_dir = 0;
  int move_dir = 0;
  if ( trg & PAD_BUTTON_A ) return CTRL_FLAG_RESULT_DECIDE;
  if ( trg & PAD_BUTTON_B ) return CTRL_FLAG_RESULT_CANCEL;
  else if ( trg & PAD_KEY_UP    ) value_dir = +1;
  else if ( trg & PAD_KEY_DOWN  ) value_dir = -1;
  else if ( trg & PAD_KEY_LEFT  ) move_dir  = +1;
  else if ( trg & PAD_KEY_RIGHT ) move_dir  = -1;

  if ( value_dir )
  {
    u32 mask = 1 << p_wk->bit_pos;
    u32 value = p_wk->flag_value & mask;
    u32 rest  = p_wk->flag_value & ~mask;
    if ( value ) {
      p_wk->flag_value = rest;
    } else {
      p_wk->flag_value |= mask;
    }
  }
  if ( move_dir )
  {
    p_wk->bit_pos += move_dir;
    if (p_wk->bit_pos >= p_wk->len ) p_wk->bit_pos = 0;
    if (p_wk->bit_pos < 0 ) p_wk->bit_pos = p_wk->len - 1;
  }
  if ( value_dir || move_dir )
  {
    UpdateFlagsInfo( p_wk );
  }
  return CTRL_FLAG_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/// サブシーケンス：Swan側の受け取り制御フラグの操作
//-----------------------------------------------------------------------------
static BOOL ControlSwanFlagValue( PROC_WORK * p_wk )
{
  switch ( p_wk->subSeq )
  {
  case 0:
    controlFlagInit( p_wk, p_wk->swan_flags, 16 );
    p_wk->subSeq ++;
    /* FALL THROUGH */

  case 1:
    {
      CTRL_FLAG_RESULT res = controlFlagMain( p_wk );
      if ( res != CTRL_FLAG_RESULT_CONTINUE )
      {
        p_wk->swan_flags = p_wk->flag_value;
        return TRUE;
      }
    }
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/// 情報表示：特殊ポケモン操作
//-----------------------------------------------------------------------------
static void UpdateSpPokeList( PROC_WORK *p_wk )
{
  // ビットマップウィンドウに文字を書き込む
  {
    int i;
    GFL_BMP_DATA* bmp;

    // バッファ作成
    bmp = GFL_BMPWIN_GetBmp( p_wk->subBmpWin );

    GFL_BMP_Clear( bmp, 0 );
    // 書き込み
    for( i=0; i<NELEMS(SPPOKE_TABLE); i++ )
    {
      // 文字色決定
      PRINTSYS_LSB lsb = ( i == p_wk->subCurPos )?  STRONG_LSB : NORMAL_LSB;

      { // 文字列作成
        u16 monsno;
        u8 p_form_no, p_sex;
        CYGNUS_DATA_GetMonsNoByFlag( SPPOKE_TABLE[i].flag_id, &monsno, &p_form_no, &p_sex );
        GFL_MSG_GetString( p_wk->pokename_man, monsno, p_wk->tmpBuf );
        PRINTSYS_PrintColor( bmp, 0, i*16, p_wk->tmpBuf, p_wk->font, lsb );
      }

      // 設定状態の表示
      {
        int bit = ( p_wk->cygnus_sppoke_bits & (1 << i) ) ? 1 : 0;
        STRTOOL_SetNumber( p_wk->tmpBuf, bit, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
        PRINTSYS_PrintColor( bmp, 13*7, i*16, p_wk->tmpBuf, p_wk->font, lsb );
      }
    }
  }

  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( p_wk->subBmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( p_wk->subBmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
}

//----------------------------------------------------------------------------
/// サブシーケンス：特殊ポケモン操作
//----------------------------------------------------------------------------
static BOOL SubSeqSpPokeControl( PROC_WORK *p_wk )
{
  int      trg = GFL_UI_KEY_GetTrg();
  int move_dir = 0;

  switch ( p_wk->subSeq )
  {
  case 0:
    p_wk->subCurPos = 0;
    UpdateSpPokeList( p_wk );
    p_wk->subSeq ++;
    break;
  case 1:
    if( trg & PAD_KEY_UP ) { move_dir = -1; }
    else if( trg & PAD_KEY_DOWN ) { move_dir = 1; }
    else if( trg & PAD_BUTTON_A ) {
      p_wk->cygnus_sppoke_bits ^= ( 1 << p_wk->subCurPos);
      UpdateSpPokeList( p_wk );
    }
    else if( trg & PAD_BUTTON_B ) return TRUE;
    break;
  }

  // カーソルが移動した場合
  if( move_dir != 0 )
  {
    // カーソル位置を補正
    p_wk->subCurPos = p_wk->subCurPos + move_dir;
    if( p_wk->subCurPos < 0 ) p_wk->subCurPos = 0;
    p_wk->subCurPos = p_wk->subCurPos % NELEMS(SPPOKE_TABLE);

    // メニュー項目更新
    UpdateSpPokeList( p_wk );
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/// 情報表示：アイテム追加
//----------------------------------------------------------------------------
static void UpdateCygnusItemInfo( PROC_WORK * p_wk )
{
  int i;
  PRINTSYS_LSB id_lsb = NORMAL_LSB;
  PRINTSYS_LSB no_lsb = NORMAL_LSB;
  GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( p_wk->subBmpWin );

  GFL_BMP_Clear( bmp, 0 );
  for (i = 0; i< CYGNUS_ITEM_NUM; i ++ )
  {
    u32 v = p_wk->cygnus_item[ i ];
    u16 item_id = CYGNUS_DATA_GET_ITEM_NO( v );
    u16 item_num = CYGNUS_DATA_GET_ITEM_NUM( v );
    if ( item_id == 0 ) break;
    STRTOOL_SetNumber( p_wk->tmpBuf, item_id, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*0, i*16, p_wk->tmpBuf, p_wk->font, id_lsb );
    GFL_MSG_GetString( p_wk->itemname_man, item_id, p_wk->tmpBuf );
    PRINTSYS_PrintColor( bmp, 13*4, i*16, p_wk->tmpBuf, p_wk->font, id_lsb );
    STRTOOL_SetNumber( p_wk->tmpBuf, item_num, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*14, i*16, p_wk->tmpBuf, p_wk->font, no_lsb );
  }
  if ( i != CYGNUS_ITEM_NUM )
  {
    if ( p_wk->subCurPos == 0 )
    {
      id_lsb = STRONG_LSB;
      no_lsb = NORMAL_LSB;
    } else {
      id_lsb = NORMAL_LSB;
      no_lsb = STRONG_LSB;
    }
    STRTOOL_SetNumber( p_wk->tmpBuf, p_wk->item_id, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*0, i*16, p_wk->tmpBuf, p_wk->font, id_lsb );
    GFL_MSG_GetString( p_wk->itemname_man, p_wk->item_id, p_wk->tmpBuf );
    PRINTSYS_PrintColor( bmp, 13*4, i*16, p_wk->tmpBuf, p_wk->font, id_lsb );
    STRTOOL_SetNumber( p_wk->tmpBuf, p_wk->item_num, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*14, i*16, p_wk->tmpBuf, p_wk->font, no_lsb );
  }

  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( p_wk->subBmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( p_wk->subBmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
}

//----------------------------------------------------------------------------
/// サブシーケンス：アイテム追加
//----------------------------------------------------------------------------
static BOOL SubSeqAddItem( PROC_WORK *p_wk )
{
  int trg = GFL_UI_KEY_GetTrg();
  int move_dir = 0;
  int pos_chg = FALSE;
  switch ( p_wk->subSeq )
  {
  case 0:
    {
      int i;
      p_wk->sel_item_pos = -1;
      for ( i=0; i<CYGNUS_ITEM_NUM; i++)
      {
        u32 v = p_wk->cygnus_item[ i ];
        if (CYGNUS_DATA_GET_ITEM_NO(v) == 0)
        {
          p_wk->sel_item_pos = i;
          break;
        }
      }
    }

    p_wk->subCurPos = 0;
    p_wk->item_id = 0;
    p_wk->item_num = 0;
    UpdateCygnusItemInfo(p_wk);
    p_wk->subSeq ++;
    break;
  case 1:
    if ( trg & PAD_BUTTON_B ) return TRUE;
    //一杯まで設定した場合はBキャンセル以外できない
    if ( p_wk->sel_item_pos == -1 ) return FALSE;

    if ( trg & PAD_BUTTON_A && p_wk->item_id != 0 && p_wk->item_num != 0 )
    {
      p_wk->cygnus_item[p_wk->sel_item_pos] = (p_wk->item_id << 16) | p_wk->item_num;
      return TRUE;
    }

    if ( trg & PAD_KEY_DOWN ) move_dir = -1;
    if ( trg & PAD_KEY_UP )   move_dir = +1;
    if ( trg & PAD_BUTTON_L ) move_dir = -10;
    if ( trg & PAD_BUTTON_R ) move_dir = +10;
    if ( trg & (PAD_KEY_LEFT |PAD_KEY_RIGHT) ) pos_chg = TRUE;
    if ( pos_chg )
    {
      p_wk->subCurPos = !(p_wk->subCurPos);
    }
    else if (move_dir != 0 )
    {
      if (p_wk->subCurPos == 0)
      {
        p_wk->item_id += move_dir;
        if (p_wk->item_id >= ITEM_DATA_MAX ) p_wk->item_id = 0;
      }
      else
      {
        p_wk->item_num += move_dir;
        if (p_wk->item_num > 999 ) p_wk->item_num = 0;
      }
    }
    if ( move_dir != 0 || pos_chg ) UpdateCygnusItemInfo( p_wk );
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/// 情報表示：ポケモン追加
//----------------------------------------------------------------------------
static void UpdateCygnusPokeInfo( PROC_WORK * p_wk )
{
  int i;
  PRINTSYS_LSB mons_lsb = NORMAL_LSB;
  PRINTSYS_LSB form_lsb = NORMAL_LSB;
  PRINTSYS_LSB sex_lsb = NORMAL_LSB;
  GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( p_wk->subBmpWin );

  GFL_BMP_Clear( bmp, 0 );
  for (i = 0; i< CYGNUS_POKE_NUM; i ++ )
  {
    u32 v = p_wk->cygnus_poke[ i ];
    u16 monsno = CYGNUS_DATA_GET_POKE_NO( v );
    u8 formno = CYGNUS_DATA_GET_FORM_NO( v );
    u8 sexno = CYGNUS_DATA_GET_SEX( v );
    if ( monsno == 0 ) break;
    STRTOOL_SetNumber( p_wk->tmpBuf, monsno, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*0, i*16, p_wk->tmpBuf, p_wk->font, mons_lsb );
    GFL_MSG_GetString( p_wk->pokename_man, monsno, p_wk->tmpBuf );
    PRINTSYS_PrintColor( bmp, 13*4, i*16, p_wk->tmpBuf, p_wk->font, mons_lsb );
    STRTOOL_SetNumber( p_wk->tmpBuf, formno, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*10, i*16, p_wk->tmpBuf, p_wk->font, form_lsb );
    STRTOOL_SetNumber( p_wk->tmpBuf, sexno, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*15, i*16, p_wk->tmpBuf, p_wk->font, sex_lsb );
  }
  if ( i != CYGNUS_POKE_NUM )
  {
    switch ( p_wk->subCurPos )
    {
    case 0:
      mons_lsb = STRONG_LSB;
      form_lsb = NORMAL_LSB;
      sex_lsb  = NORMAL_LSB;
      break;
    case 1:
      mons_lsb = NORMAL_LSB;
      form_lsb = STRONG_LSB;
      sex_lsb  = NORMAL_LSB;
      break;
    case 2:
      mons_lsb = NORMAL_LSB;
      form_lsb = NORMAL_LSB;
      sex_lsb  = STRONG_LSB;
      break;
    }
    STRTOOL_SetNumber( p_wk->tmpBuf, p_wk->monsno, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*0, i*16, p_wk->tmpBuf, p_wk->font, mons_lsb );
    GFL_MSG_GetString( p_wk->pokename_man, p_wk->monsno, p_wk->tmpBuf );
    PRINTSYS_PrintColor( bmp, 13*4, i*16, p_wk->tmpBuf, p_wk->font, mons_lsb );
    STRTOOL_SetNumber( p_wk->tmpBuf, p_wk->form_no, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*10, i*16, p_wk->tmpBuf, p_wk->font, form_lsb );
    STRTOOL_SetNumber( p_wk->tmpBuf, p_wk->sex, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*15, i*16, p_wk->tmpBuf, p_wk->font, sex_lsb );
  }

  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( p_wk->subBmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( p_wk->subBmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
}

//----------------------------------------------------------------------------
/// サブシーケンス：ポケモン追加
//----------------------------------------------------------------------------
static BOOL SubSeqAddPokemon( PROC_WORK *p_wk )
{
  int trg = GFL_UI_KEY_GetTrg();
  int move_dir = 0;
  int pos_chg_dir = 0;
  switch ( p_wk->subSeq )
  {
  case 0:
    {
      int i;
      p_wk->sel_mons_pos = -1;
      for ( i=0; i<CYGNUS_POKE_NUM; i++)
      {
        u32 v = p_wk->cygnus_poke[ i ];
        if (CYGNUS_DATA_GET_POKE_NO(v) == 0)
        {
          p_wk->sel_mons_pos = i;
          break;
        }
      }
    }

    p_wk->subCurPos = 0;
    p_wk->monsno = 0;
    p_wk->form_no = 0;
    p_wk->sex = 0;
    UpdateCygnusPokeInfo(p_wk);
    p_wk->subSeq ++;
    break;
  case 1:
    if ( trg & PAD_BUTTON_B ) return TRUE;
    //一杯まで設定した場合はBキャンセル以外できない
    if ( p_wk->sel_mons_pos == -1 ) return FALSE;

    if ( trg & PAD_BUTTON_A && p_wk->monsno != 0 )
    {
      p_wk->cygnus_poke[p_wk->sel_mons_pos]
        = (p_wk->monsno << 16) | (p_wk->form_no << 8) | p_wk->sex;
      return TRUE;
    }

    if ( trg & PAD_KEY_DOWN ) move_dir = -1;
    if ( trg & PAD_KEY_UP )   move_dir = +1;
    if ( trg & PAD_BUTTON_L ) move_dir = -10;
    if ( trg & PAD_BUTTON_R ) move_dir = +10;
    if ( trg & PAD_KEY_LEFT ) pos_chg_dir = -1;
    if ( trg & PAD_KEY_RIGHT) pos_chg_dir = +1;
    if ( pos_chg_dir != 0 )
    {
      p_wk->subCurPos += pos_chg_dir;
      if (p_wk->subCurPos < 0 ) p_wk->subCurPos = 0;
      if (p_wk->subCurPos > 2 ) p_wk->subCurPos = 2;
    }
    else if (move_dir != 0 )
    {
      switch ( p_wk->subCurPos )
      {
      case 0:
        p_wk->monsno += move_dir;
        if (p_wk->monsno >= MONSNO_END ) p_wk->monsno = 0;
        break;
      case 1:
        p_wk->form_no += move_dir;
        if (p_wk->form_no > 99 ) p_wk->form_no = 0;
        break;
      case 2:
        p_wk->sex = !(p_wk->sex);
        break;
      }
    }
    if ( move_dir != 0 || pos_chg_dir != 0 ) UpdateCygnusPokeInfo( p_wk );
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
enum {
  ONETIME_KEY_KETA = 8,
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void UpdateOnetimeKeyInfo( PROC_WORK * p_wk )
{
  int i;
  GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( p_wk->subBmpWin );
  GFL_BMP_Clear( bmp, 0 );
  for (i = 0; i< ONETIME_KEY_KETA; i ++ )
  {
    u32 shift = (i * 4);
    u32 mask1 =  0xf << shift;
    s32 value = (p_wk->swan_key & mask1) >> shift;
    PRINTSYS_LSB lsb = NORMAL_LSB;
    if ( i == p_wk->subCurPos ) lsb = STRONG_LSB;
    STRTOOL_SetHexNumber( p_wk->tmpBuf, value, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    PRINTSYS_PrintColor( bmp, 13*(ONETIME_KEY_KETA - 1 - i), 0, p_wk->tmpBuf, p_wk->font, lsb );
  }

  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( p_wk->subBmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( p_wk->subBmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static BOOL SubSeqControlOneTimeKey( PROC_WORK * p_wk )
{
  int trg = GFL_UI_KEY_GetTrg();
  int move_dir = 0;
  int updown = 0;
  switch ( p_wk->subSeq )
  {
  case 0:
    UpdateOnetimeKeyInfo( p_wk );
    p_wk->subSeq ++;
    break;
  case 1:
    if ( trg & PAD_BUTTON_B ) return TRUE;
    if ( trg & PAD_KEY_LEFT ) move_dir = +1;
    if ( trg & PAD_KEY_RIGHT ) move_dir = -1;
    if ( trg & PAD_KEY_DOWN ) updown = -1;
    if ( trg & PAD_KEY_UP ) updown = 1;

    if ( updown )
    {
      u32 shift = (p_wk->subCurPos * 4);
      u32 mask1 =  0xf << shift;
      s32 value = (p_wk->swan_key & mask1) >> shift;
      value += updown;
      if ( value < 0 ) value = 0xf;
      if ( value > 0xf ) value = 0;
      p_wk->swan_key &= ~mask1;
      p_wk->swan_key |= ( value << shift );
    }
    else if ( move_dir )
    {
      p_wk->subCurPos += move_dir;
      if ( p_wk->subCurPos < 0 ) p_wk->subCurPos = ONETIME_KEY_KETA - 1;
    }
    if ( move_dir || updown )
    {
      UpdateOnetimeKeyInfo( p_wk );
    }
  }
  return FALSE;
}

#endif // PM_DEBUG





