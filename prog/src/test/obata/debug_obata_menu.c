#include <wchar.h>    // wcslen
#include <gflib.h>
#include "debug_obata_menu_setup.h"
#include "system/main.h" 
#include "arc/arc_def.h"
#include "font/font.naix"
#include "print/printsys.h"


FS_EXTERN_OVERLAY(obata_debug);
extern const GFL_PROC_DATA DebugObataDemoTestMainProcData;
extern const GFL_PROC_DATA DebugObata3DSoundMainProcData;
extern const GFL_PROC_DATA DebugObataIcaTestMainProcData;
extern const GFL_PROC_DATA DebugObataElboardMainProcData;
extern const GFL_PROC_DATA DebugObataElboard2MainProcData;


//============================================================================================
/**
 * @breif 定数
 */
//============================================================================================
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
  MENU_ITEM_DEMO_TEST,
  MENU_ITEM_3D_SOUND,
  MENU_ITEM_ICA_TEST,
  MENU_ITEM_ELBOARD,
  MENU_ITEM_ELBOARD2,
  MENU_ITEM_DAMMY,
  MENU_ITEM_NUM
} MENU_ITEM;

// メニュー項目文字列
static const STRCODE* str_menu[] = 
{
  L"デモテスト",
  L"3Dサウンド",
  L"ICA再生テスト",
  L"電光掲示板",
  L"電光掲示板2",
  L"戻る",
};


//============================================================================================
/**
 * @breif ワーク
 */
//============================================================================================
typedef struct
{
  GFL_FONT* font;
  GFL_BMPWIN* bmpWin;
  int curPos; // 選択項目
  SELECT_STATE state; // 選択状態
}
PROC_WORK;


//============================================================================================
/**
 * @brief プロトタイプ宣言
 */
//============================================================================================
static void Initialize( PROC_WORK* work );
static void Finalize( PROC_WORK* work );
static void Main( PROC_WORK* work );
static void Draw( PROC_WORK* work );
static void UpdateMenuList( PROC_WORK* work );
static void SetNextProc( PROC_WORK* work );


//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_MENU_MainProcFunc_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = NULL;

  // ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OBATA_DEBUG, 0x100000 );

	// ワークを作成
	work = GFL_PROC_AllocWork( proc, sizeof( PROC_WORK ), HEAPID_OBATA_DEBUG );

  // 初期化処理
  DEBUG_OBATA_MENU_SETUP_Init( HEAPID_OBATA_DEBUG );
  Initialize( work );

	return GFL_PROC_RES_FINISH;
}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_MENU_MainProcFunc_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = mywk;

  switch( *seq )
  {
  case 0:
    Main( work );
    Draw( work ); 
    if( work->state != SELECT_STATE_NONE )
    {
      ++( *seq );
    }
    break;
  case 1:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}


//============================================================================================
/**
 * @brief 終了関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_MENU_MainProcFunc_End( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = mywk;

  // 次のプロセスを設定
  SetNextProc( work );

  // 終了処理
  Finalize( work );
  DEBUG_OBATA_MENU_SETUP_Exit();

	// ワークを破棄
	GFL_PROC_FreeWork( proc );

  // ヒープを破棄
  GFL_HEAP_DeleteHeap( HEAPID_OBATA_DEBUG );

	return GFL_PROC_RES_FINISH;
} 


//============================================================================================
/**
 * @brief プロセス定義データ
 */
//============================================================================================
const GFL_PROC_DATA DebugObataMenuMainProcData = 
{
	DEBUG_OBATA_MENU_MainProcFunc_Init,
	DEBUG_OBATA_MENU_MainProcFunc_Main,
	DEBUG_OBATA_MENU_MainProcFunc_End,
};


//============================================================================================
/**
 * @brief 非公開関数の定義
 */
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @breif 初期化
 */
//-------------------------------------------------------------------------------------------- 
static void Initialize( PROC_WORK* work )
{
  // 初期化
  work->curPos = 0;
  work->state = SELECT_STATE_NONE;

  // フォントデータハンドラ作成
  work->font = GFL_FONT_Create( 
      ARCID_FONT, NARC_font_large_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_OBATA_DEBUG );

  // ビットマップウィンドウを作成
  work->bmpWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );

  // メニューリストの更新
  UpdateMenuList( work );
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 終了
 */
//-------------------------------------------------------------------------------------------- 
static void Finalize( PROC_WORK* work )
{ 
  // ビットマップウィンドウの破棄
  GFL_BMPWIN_Delete( work->bmpWin );

  // フォントデータハンドラ破棄
  GFL_FONT_Delete( work->font );
}

//--------------------------------------------------------------------------------------------
/**
 * @breif メイン処理 
 */
//-------------------------------------------------------------------------------------------- 
static void Main( PROC_WORK* work )
{
  int      trg = GFL_UI_KEY_GetTrg();
  int move_dir = 0;

  if( trg & PAD_KEY_UP ) move_dir = -1;
  if( trg & PAD_KEY_DOWN ) move_dir = 1;
  if( trg & PAD_BUTTON_A ) work->state = SELECT_STATE_DECIDE;
  if( trg & PAD_BUTTON_SELECT ) work->state = SELECT_STATE_EXIT;

  // カーソルが移動した場合
  if( move_dir != 0 )
  {
    // カーソル位置を補正
    work->curPos = work->curPos + move_dir;
    if( work->curPos < 0 ) work->curPos = 0;
    work->curPos = work->curPos % MENU_ITEM_NUM;

    // メニュー項目更新
    UpdateMenuList( work );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 描画
 */
//-------------------------------------------------------------------------------------------- 
static void Draw( PROC_WORK* work )
{
}

//--------------------------------------------------------------------------------------------
/**
 * @breif メニューリストを更新する
 */
//-------------------------------------------------------------------------------------------- 
static void UpdateMenuList( PROC_WORK* work )
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
      strbuf = GFL_STR_CreateBuffer( 256, HEAPID_OBATA_DEBUG );

      // 文字色決定
      if( i == work->curPos )
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
      bmp = GFL_BMPWIN_GetBmp( work->bmpWin );
      PRINTSYS_PrintColor( bmp, 0, i*16, strbuf, work->font, lsb );

      // バッファ破棄
      GFL_STR_DeleteBuffer( strbuf );
    }
  }

  // ビットマップウィンドウ内部バッファをVRAMへ転送
  GFL_BMPWIN_TransVramCharacter( work->bmpWin );

  // ビットマップ用スクリーンをBGスクリーンバッファに作成
  GFL_BMPWIN_MakeScreen( work->bmpWin );

  // BGスクリーンデータを反映させる
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 次のプロセスを設定する
 */
//--------------------------------------------------------------------------------------------
static void SetNextProc( PROC_WORK* work )
{
  // カーソルの位置で決定
  switch( work->curPos )
  {
  case MENU_ITEM_DEMO_TEST:
      GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(obata_debug), &DebugObataDemoTestMainProcData, NULL );
      break;
  case MENU_ITEM_3D_SOUND:
      GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(obata_debug), &DebugObata3DSoundMainProcData, NULL );
      break;
  case MENU_ITEM_ICA_TEST:
      GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(obata_debug), &DebugObataIcaTestMainProcData, NULL );
      break;
  case MENU_ITEM_ELBOARD:
      GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(obata_debug), &DebugObataElboardMainProcData, NULL );
      break;
  case MENU_ITEM_ELBOARD2:
      GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(obata_debug), &DebugObataElboard2MainProcData, NULL );
      break;
  }
}
