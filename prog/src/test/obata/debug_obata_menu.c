#include <gflib.h>
#include "debug_obata_menu_setup.h"
#include "system/main.h"


//============================================================================================
/**
 * @breif ワーク
 */
//============================================================================================
typedef struct
{
  int temp;
}
PROC_WORK;


//============================================================================================
/**
 * @brief プロトタイプ宣言
 */
//============================================================================================
static void Initialize( PROC_WORK* work );
static void Finalize( PROC_WORK* work );
static BOOL Main( PROC_WORK* work );
static void Draw( PROC_WORK* work );


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
  BOOL end = FALSE;

  switch( *seq )
  {
  case 0:
    end = Main( work );
    Draw( work );
    if( end )
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

  // 終了処理
  Finalize( work );

	// ワークを破棄
	GFL_PROC_FreeWork( proc );

  DEBUG_OBATA_MENU_SETUP_Exit();

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
  GFL_BG_ReserveCharacterArea( GFL_BG_FRAME1_M, 0, 32*24 );
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 終了
 */
//-------------------------------------------------------------------------------------------- 
static void Finalize( PROC_WORK* work )
{ 
}

//--------------------------------------------------------------------------------------------
/**
 * @breif メイン処理 
 */
//-------------------------------------------------------------------------------------------- 
static BOOL Main( PROC_WORK* work )
{
  int trg = GFL_UI_KEY_GetTrg();

  // セレクトで終了
  if( trg & PAD_BUTTON_SELECT ) return TRUE;
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 描画
 */
//-------------------------------------------------------------------------------------------- 
static void Draw( PROC_WORK* work )
{
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
}
