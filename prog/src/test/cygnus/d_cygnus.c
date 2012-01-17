//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\test\cygnus\d_cygnus.c
 *	@brief
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.11.26
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG

#include <wchar.h>    // wcslen
#include <gflib.h>
#include "debug_cygnus_menu_setup.h"
#include "system/main.h"
#include "arc/arc_def.h"
#include "debug/cgearpict.naix"
#include "font/font.naix"
#include "print/printsys.h"
#include "gamesystem/game_data.h"

#include "savedata/save_tbl.h"
#include "savedata/wifilist.h"
#include "savedata/dreamworld_data.h"
#include "savedata/mystatus.h"
#include "../../savedata/mystatus_local.h"
#include "savedata/record.h"
#include "savedata/c_gear_data.h"
#include "savedata/c_gear_picture.h"
#include "savedata/save_tbl.h"
#include "savedata/medalbox.h"
#include "../../savedata/medalbox_local.h"
#include "savedata/situation.h"
#include "savedata/mystery_data.h"
#include "savedata/record.h"
#include "system\pms_data.h"
#include "savedata/worldtrade_data.h"
#include "savedata/sodateya_work.h"
#include "savedata/misc.h"
#include "savedata/misc_local.h"
#include "savedata/intrude_save.h"
#include "savedata/system_data_local.h"
#include "savedata/dreamworld_data_local.h"
#include "poke_tool/pokeparty.h"
#include "savedata/wifihistory.h"
#include "savedata/wifihistory_local.h"
#include "savedata/playtime.h"
#include "savedata/playtime_local.h"
#include "savedata/regulation.h"
#include "savedata/symbol_save.h"
#include "savedata/symbol_save_local.h"
#include "savedata/myitem_savedata_local.h"
#include "net/dreamworld_netdata.h"
#include "savedata/zukan_savedata_local.h"
#include "savedata/symbol_save.h"


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

// メニュー項目インデックス
typedef enum
{
  MENU_ITEM_NONE,
  MENU_ITEM_INIT,
  MENU_ITEM_TORUNEROSU,
  MENU_ITEM_BORUTOROSU,
  MENU_ITEM_FLAGCONTROL,
  MENU_ITEM_PRINT,
  MENU_ITEM_EXIT,

  MENU_ITEM_NUM
} MENU_ITEM;

// メニュー項目文字列
static const STRCODE* str_menu[] =
{
  L"出荷状態にする",
  L"CYGNUS書き込み許可にする",
  L"トルネロスをSWANにわたす",
  L"ボルトロスをSWANにわたす",
  L"SWAN側フラグ操作",
  L"デバッグ表示",
  L"戻る",
};

typedef enum {
  STAT_NONE,
  STAT_ERASE,
  STAT_SWAN_WRITE_SUCCESS,
  STAT_SWAN_WRITE_FAILURE,
  STAT_CYGNUS_WRITE_SUCCESS,
  STAT_CYGNUS_WRITE_FAILURE,
  STAT_CYGNUS_FLAGS_FAILURE,

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
};

SDK_COMPILER_ASSERT( STAT_NUM == NELEMS(str_state) );

enum {
  ONETIME_KEY = 0x12345678,
  FLAGS = 0,
};

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
  GFL_BMPWIN* menuBmpWin;
  GFL_BMPWIN* subBmpWin;
  int         curPos; // 選択項目
  SELECT_STATE state; // 選択状態
	SAVE_CONTROL_WORK* pSaveData;
  GAMEDATA      *pGameData;
  CYGNUS_SAVE_CONTROL * cygsave_ctrl;
  CYGNUS_SAVEDATA * cygsave;
  u32 cygBuf[ CYGDATA_AREA_SIZE ];
  u32 flags;
  int bit_pos;
  int subSeq;
  LAST_STATUS last_status;
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


static void CygnusLoad( PROC_WORK * p_wk );
static void CygnusUnload( PROC_WORK * p_wk );

// 処理関数
// メニューの数と同じ

static BOOL SubSeqCygStatusPrint( PROC_WORK * p_wk );
static BOOL SubSeqCygSaveErase( PROC_WORK * p_wk );
static BOOL SubSeqCygSaveInit( PROC_WORK * p_wk );
static BOOL SubSeqSpPokemonSet( PROC_WORK * p_wk, const u32 id );
static BOOL ControlSwanFlagValue( PROC_WORK * p_wk );
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
/**
 *	@brief  初期化
 */
//-----------------------------------------------------------------------------
static void Init( PROC_WORK *p_wk )
{
  // 初期化
  p_wk->curPos = 0;
  p_wk->state = SELECT_STATE_NONE;
  p_wk->subSeq = 0;
  p_wk->flags = FLAGS;
  p_wk->bit_pos = 0;

  // フォントデータハンドラ作成
  p_wk->font = GFL_FONT_Create(
      ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_PGL_DEBUG );

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
  int      trg = GFL_UI_KEY_GetTrg();
  int move_dir = 0;

  if( trg & PAD_KEY_UP ) move_dir = -1;
  if( trg & PAD_KEY_DOWN ) move_dir = 1;
  if( trg & PAD_BUTTON_A ) p_wk->state = SELECT_STATE_DECIDE;
  if( trg & PAD_BUTTON_SELECT ) p_wk->state = SELECT_STATE_EXIT;

  // カーソルが移動した場合
  if( move_dir != 0 )
  {
    // カーソル位置を補正
    p_wk->curPos = p_wk->curPos + move_dir;
    if( p_wk->curPos < 0 ) p_wk->curPos = 0;
    p_wk->curPos = p_wk->curPos % MENU_ITEM_NUM;

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
    STRBUF* strbuf;
    GFL_BMP_DATA* bmp;
    PRINTSYS_LSB lsb;

    // バッファ作成
    strbuf = GFL_STR_CreateBuffer( 256, HEAPID_PGL_DEBUG );
    bmp = GFL_BMPWIN_GetBmp( p_wk->menuBmpWin );

    GFL_BMP_Clear( bmp, 0 );
    // 書き込み
    for( i=0; i<MENU_ITEM_NUM; i++ )
    {

      // 文字色決定
      if( i == p_wk->curPos )
      {
        lsb = PRINTSYS_LSB_Make(1, 3, 0);
      }
      else
      {
        lsb = PRINTSYS_LSB_Make(1, 2, 0);
      }

      // 文字列作成
      createString( strbuf, str_menu[i] );

      // ビットマップに書き込み
      PRINTSYS_PrintColor( bmp, 0, i*16, strbuf, p_wk->font, lsb );

      // バッファ破棄
    }
    lsb = PRINTSYS_LSB_Make(1, 3, 0);
    createString( strbuf, str_state[p_wk->last_status] );
    PRINTSYS_PrintColor( bmp, 30, 160, strbuf, p_wk->font, lsb );

    GFL_STR_DeleteBuffer( strbuf );
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
  int i;
  PRINTSYS_LSB lsb = PRINTSYS_LSB_Make(1, 2, 0);
  GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( p_wk->subBmpWin );
  STRBUF * strbuf = GFL_STR_CreateBuffer( 256, HEAPID_PGL_DEBUG );

  GFL_BMP_Clear( bmp, 0 );
  for (i = 0; i< 16; i ++ )
  {
    const STRCODE * code = (p_wk->flags & (1 << i) ) == 0 ? L"0" : L"1";
    createString( strbuf, code );
    if ( i == p_wk->bit_pos )
    {
      lsb = PRINTSYS_LSB_Make(1, 3, 0);
    }
    else
    {
      lsb = PRINTSYS_LSB_Make(1, 2, 0);
    }
    PRINTSYS_PrintColor( bmp, (16-i)*13, 0, strbuf, p_wk->font, lsb );
  }

  GFL_STR_DeleteBuffer( strbuf );

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
  int i;
  PRINTSYS_LSB lsb = PRINTSYS_LSB_Make(1, 2, 0);
  GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( p_wk->subBmpWin );
  STRBUF * strbuf = GFL_STR_CreateBuffer( 256, HEAPID_PGL_DEBUG );

  GFL_BMP_Clear( bmp, 0 );
  for (i = 0; i< NELEMS(info_str); i ++ )
  {
    createString( strbuf, info_str[i] );
    PRINTSYS_PrintColor( bmp, 0, i*16, strbuf, p_wk->font, lsb );
  }

  {
    CygnusLoad( p_wk );
    createU32Hex( strbuf, CYGNUS_SAVEDATA_GetStatus( p_wk->cygsave ) );
    PRINTSYS_PrintColor( bmp, 18 * 10, 0 * 16, strbuf, p_wk->font, lsb );
    createU32Hex( strbuf, p_wk->cygsave->overwrite_enable_flag );
    PRINTSYS_PrintColor( bmp, 18 * 10, 1 * 16, strbuf, p_wk->font, lsb );
    createU32Hex( strbuf, p_wk->cygsave->onetime_key );
    PRINTSYS_PrintColor( bmp, 18 * 10, 2 * 16, strbuf, p_wk->font, lsb );
    createU32Hex( strbuf, p_wk->cygsave->magic_number );
    PRINTSYS_PrintColor( bmp, 18 * 10, 3 * 16, strbuf, p_wk->font, lsb );
    createU32Hex( strbuf, CYGNUS_SAVEDATA_DEBUG_GetFlags(p_wk->cygsave) );
    PRINTSYS_PrintColor( bmp, 18 * 10, 4 * 16, strbuf, p_wk->font, lsb );
    CygnusUnload( p_wk );
  }

  GFL_STR_DeleteBuffer( strbuf );

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
  case MENU_ITEM_NONE:
    return SubSeqCygSaveErase( p_wk );
  case MENU_ITEM_INIT:
    return SubSeqCygSaveInit( p_wk );
  case MENU_ITEM_PRINT:
    return SubSeqCygStatusPrint( p_wk );
  case MENU_ITEM_TORUNEROSU:
    return SubSeqSpPokemonSet( p_wk, CYGDATA_SP_POKE01 );
  case MENU_ITEM_BORUTOROSU:
    return SubSeqSpPokemonSet( p_wk, CYGDATA_SP_POKE02 );
  case MENU_ITEM_FLAGCONTROL:
    return ControlSwanFlagValue( p_wk );
  }
  return TRUE;  //対応がない場合はすぐに終了
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void CygnusLoad( PROC_WORK * p_wk )
{
  p_wk->cygsave_ctrl = CYGNUSSAVE_CONTROL_Create( HEAPID_PGL_DEBUG );
  CYGNUSSAVE_CONTROL_Load( p_wk->cygsave_ctrl, ONETIME_KEY );
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
//-----------------------------------------------------------------------------
static void CygnusBufSet( PROC_WORK * p_wk, u32 id )
{
  GFL_STD_MemClear( p_wk->cygBuf, sizeof(u32) * CYGDATA_AREA_SIZE );
  switch ( id )
  {
  case CYGDATA_SP_POKE01: p_wk->cygBuf[ id ] = MAGICNUMBER_SP_POKE01; break;
  case CYGDATA_SP_POKE02: p_wk->cygBuf[ id ] = MAGICNUMBER_SP_POKE02; break;
  case CYGDATA_SP_POKE03: p_wk->cygBuf[ id ] = MAGICNUMBER_SP_POKE03; break;
  case CYGDATA_SP_POKE04: p_wk->cygBuf[ id ] = MAGICNUMBER_SP_POKE04; break;
  case CYGDATA_SP_POKE05: p_wk->cygBuf[ id ] = MAGICNUMBER_SP_POKE05; break;
  case CYGDATA_SP_POKE06: p_wk->cygBuf[ id ] = MAGICNUMBER_SP_POKE06; break;
  }
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL SubSeqCygStatusPrint( PROC_WORK * p_wk )
{
  u32 onetime_key = ONETIME_KEY;
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
//-----------------------------------------------------------------------------
static BOOL SubSeqCygSaveErase( PROC_WORK * p_wk )
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
//-----------------------------------------------------------------------------
static BOOL SubSeqCygSaveInit( PROC_WORK * p_wk )
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
    CYGNUS_SAVEDATA_SetSend( p_wk->cygsave, ONETIME_KEY, p_wk->flags  );
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
//-----------------------------------------------------------------------------
static BOOL SubSeqSpPokemonSet( PROC_WORK * p_wk, const u32 id )
{
  switch ( p_wk->subSeq )
  {
  case 0:
    CygnusLoad( p_wk );
    p_wk->subSeq ++;
    break;
  case 1:
    if ( CYGNUS_SAVEDATA_GetStatus( p_wk->cygsave ) != CYGSAVE_STATE_SEND )
    {
      CygnusUnload( p_wk );
      p_wk->last_status = STAT_CYGNUS_WRITE_FAILURE;
      return TRUE;
    }
    if ( CYGNUS_SAVEDATA_DEBUG_GetFlags( p_wk->cygsave ) & (1 << id) )
    {
      CygnusUnload( p_wk );
      p_wk->last_status = STAT_CYGNUS_FLAGS_FAILURE;
      return TRUE;
    }
    CygnusBufSet( p_wk, id );
    CYGNUS_SAVEDATA_DEBUG_Write( p_wk->cygsave, p_wk->cygBuf );
    CYGNUSSAVE_CONTROL_SaveAsyncInit( p_wk->cygsave_ctrl );
    p_wk->subSeq ++;
    break;
  case 2:
    OS_Printf("WRITING FROM CYGNUS...\n");
    if ( CYGNUSSAVE_CONTROL_SaveAsyncMain( p_wk->cygsave_ctrl ) == TRUE )
    {
      CygnusUnload( p_wk );
      p_wk->last_status = STAT_CYGNUS_WRITE_SUCCESS;
      return TRUE;  //終了
    }
    break;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL ControlSwanFlagValue( PROC_WORK * p_wk )
{
  switch ( p_wk->subSeq )
  {
  case 0:
    p_wk->bit_pos = 0;
    UpdateFlagsInfo( p_wk );
    p_wk->subSeq ++;
    /* FALL THROUGH */

  case 1:
    {
      int trg = GFL_UI_KEY_GetTrg();
      int value_dir = 0;
      int move_dir = 0;
      if ( trg & PAD_BUTTON_B ) return TRUE;
      else if ( trg & PAD_KEY_UP ) value_dir = +1;
      else if ( trg & PAD_KEY_DOWN ) value_dir = -1;
      else if ( trg & PAD_KEY_LEFT ) move_dir = -1;
      else if ( trg & PAD_KEY_RIGHT ) move_dir = +1;

      if ( value_dir )
      {
        u32 mask = 1 << p_wk->bit_pos;
        u32 value = p_wk->flags & mask;
        u32 rest  = p_wk->flags & ~mask;
        OS_Printf(" flags --> %08x\n", p_wk->flags );
        OS_Printf("value %08x rest %08x\n", value, rest);
        if ( value ) {
          p_wk->flags = rest;
        } else {
          p_wk->flags |= mask;
        }
        OS_Printf(" flags --> %08x\n", p_wk->flags );
      }
      if ( move_dir )
      {
        p_wk->bit_pos += move_dir;
        if (p_wk->bit_pos >= 16 ) p_wk->bit_pos = 0;
        if (p_wk->bit_pos < 0 ) p_wk->bit_pos = 15;
      }
      if ( value_dir || move_dir )
      {
        UpdateFlagsInfo( p_wk );
      }
      return FALSE;
    }
    break;
  }
  return FALSE;
}



#endif // PM_DEBUG





