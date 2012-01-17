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

#include "savedata/wifi_negotiation.h"

#include "savedata/wifilist.h"
#include "savedata/wifilist_local.h"

#include "savedata/cygnus_save_control.h"

extern MYITEM_PTR* SaveData_GetMyItem(SAVE_CONTROL_WORK * sv);
extern WIFI_LIST* SaveData_GetWifiListData(SAVE_CONTROL_WORK * sv);

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
  MENU_ITEM_INIT,
  MENU_ITEM_PRINT,
  MENU_ITEM_TORUNEROSU,
  MENU_ITEM_BORUTOROSU,
  MENU_ITEM_EXIT,

  MENU_ITEM_NUM
} MENU_ITEM;

// メニュー項目文字列
static const STRCODE* str_menu[] =
{
  L"しょきか",
  L"表示",
  L"トルネロス",
  L"ボルトロス",
  L"戻る",
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
  GFL_BMPWIN* bmpWin;
  int         curPos; // 選択項目
  SELECT_STATE state; // 選択状態
	SAVE_CONTROL_WORK* pSaveData;
  GAMEDATA      *pGameData;
  CYGNUS_SAVE_CONTROL * cygsave_ctrl;
  CYGNUS_SAVEDATA * cygsave;
  u32 cygBuf[ CYGDATA_AREA_SIZE ];
  int subSeq;
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
static void SetNextFunc( PROC_WORK *p_wk );
static BOOL CallSubSeq( u32 id, PROC_WORK *p_wk );

// 処理関数
// メニューの数と同じ

static BOOL SubSeqCygStatusPrint( PROC_WORK * p_wk );
static BOOL SubSeqCygSaveInit( PROC_WORK * p_wk );
static BOOL SubSeqSpPokemonSet( PROC_WORK * p_wk, const u32 id );
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
      p_wk->state = SELECT_STATE_NONE;
      p_wk->subSeq = 0;
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

  // 次のプロセスを設定
  SetNextFunc( p_wk );

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

  // フォントデータハンドラ作成
  p_wk->font = GFL_FONT_Create(
      ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_PGL_DEBUG );

  // ビットマップウィンドウを作成
  p_wk->bmpWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );

  // メニューリストの更新
  UpdateMenuList( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了
 */
//-----------------------------------------------------------------------------
static void Exit( PROC_WORK *p_wk )
{
  // ビットマップウィンドウの破棄
  GFL_BMPWIN_Delete( p_wk->bmpWin );

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

//----------------------------------------------------------------------------
/**
 *	@brief  メニューリスト更新
 */
//-----------------------------------------------------------------------------
static void UpdateMenuList( PROC_WORK *p_wk )
{
  // ビットマップウィンドウに文字を書き込む
  {
    int i, len;
    STRBUF* strbuf;
    STRCODE strcode[256];
    GFL_BMP_DATA* bmp;
    PRINTSYS_LSB lsb;


    // 書き込み
    for( i=0; i<MENU_ITEM_NUM; i++ )
    {
      // バッファ作成
      strbuf = GFL_STR_CreateBuffer( 256, HEAPID_PGL_DEBUG );

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
      len = wcslen( str_menu[i] );
      GFL_STD_MemCopy( str_menu[i], strcode, sizeof( STRCODE )*len );
      strcode[len] = GFL_STR_GetEOMCode();
      GFL_STR_SetStringCode( strbuf, strcode );

      // ビットマップに書き込み
      bmp = GFL_BMPWIN_GetBmp( p_wk->bmpWin );
      PRINTSYS_PrintColor( bmp, 0, i*16, strbuf, p_wk->font, lsb );

      // バッファ破棄
      GFL_STR_DeleteBuffer( strbuf );
    }
  }

  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( p_wk->bmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( p_wk->bmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
}

//----------------------------------------------------------------------------
/**
 *	@brief  選択した処理
 */
//-----------------------------------------------------------------------------
static void SetNextFunc( PROC_WORK *p_wk )
{
}


//-----------------------------------------------------------------------------
/**
 * @brie  サブシーケンスの呼び出し
 */
//-----------------------------------------------------------------------------
static BOOL CallSubSeq( u32 id, PROC_WORK *p_wk )
{
  switch( id ){
  case MENU_ITEM_INIT:
    return SubSeqCygSaveInit( p_wk );
  case MENU_ITEM_PRINT:
    return SubSeqCygStatusPrint( p_wk );
  case MENU_ITEM_TORUNEROSU:
    return SubSeqSpPokemonSet( p_wk, CYGDATA_SP_POKE01 );
  case MENU_ITEM_BORUTOROSU:
    return SubSeqSpPokemonSet( p_wk, CYGDATA_SP_POKE02 );
  }
  return TRUE;  //対応がない場合はすぐに終了
}

//============================================================================================
//============================================================================================
enum {
  ONETIME_KEY = 0x12345678,
  FLAGS = 0,
};

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
static BOOL SubSeqCygSaveInit( PROC_WORK * p_wk )
{
  switch ( p_wk->subSeq )
  {
  case 0:
    CygnusLoad( p_wk );
    p_wk->subSeq ++;
    break;
  case 1:
    CYGNUS_SAVEDATA_SetSend( p_wk->cygsave, ONETIME_KEY, FLAGS  );
    CYGNUSSAVE_CONTROL_SaveAsyncInit( p_wk->cygsave_ctrl );
    p_wk->subSeq ++;
    break;
  case 2:
    OS_Printf("WRITING FROM SWAN...\n");
    if ( CYGNUSSAVE_CONTROL_SaveAsyncMain( p_wk->cygsave_ctrl ) == TRUE )
    {
      OS_Printf("FINISHED WRITING FROM SWAN\n");
      CygnusUnload( p_wk );
      p_wk->state = SELECT_STATE_NONE;
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
      OS_Printf("CAN'T WRITE FROM CYGNUS\n");
      CygnusUnload( p_wk );
      p_wk->state = SELECT_STATE_NONE;
      break;
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
      OS_Printf("FINISHED WRITING FROM CYGNUS\n");
      CygnusUnload( p_wk );
      p_wk->state = SELECT_STATE_NONE;
      return TRUE;  //終了
    }
    break;
  }
  return FALSE;
}

#endif // PM_DEBUG





