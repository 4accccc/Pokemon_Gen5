//============================================================================================
/**
 * @file		zukan.c
 * @brief		図鑑画面
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZUKAN
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "app/zukan.h"

#include "zukan_common.h"
#include "top/zukantop.h"
#include "list/zukanlist.h"
#include "search/zukansearch.h"
#include "detail/zukan_detail.h"
#include "habitat_zone/habitat_zone.h"

#include "poke_tool/monsnum_def.h"  // for MONSNO_数字

#include "gamesystem/pm_season.h"

//============================================================================================
//	定数定義
//============================================================================================

// メインシーケンス
enum {
	SEQ_LOCAL_PROC_WAIT,	      // 各処理終了待ち

	SEQ_TOP_CALL,					      // トップ呼び出し
	SEQ_TOP_END,					      // トップ終了後

	SEQ_LIST_CALL,				      // リスト呼び出し
	SEQ_LIST_END,					      // リスト終了後

	SEQ_INFO_CALL,				      // 情報呼び出し(INFO,MAP,VOICE,FORMのどれかを呼び出す)
	SEQ_INFO_END,					      // 情報終了後

	SEQ_SEARCH_CALL,			      // 検索呼び出し
	SEQ_SEARCH_END,				      // 検索終了後

	SEQ_HABITAT_ZONE_CALL,      // 生息地リストゾーン詳細画面呼出
	SEQ_HABITAT_ZONE_END,       // 生息地リストゾーン詳細画面終了後

	SEQ_HABITAT_ZONE_LIST_CALL, // 生息地リストゾーンリスト画面呼出
	SEQ_HABITAT_ZONE_LIST_END,  // 生息地リストゾーンリスト画面終了後

	SEQ_PROC_FINISH,			// 図鑑終了
};

// 図鑑メインワーク
typedef struct {
	ZUKAN_PARAM * prm;			// 外部データ

	GFL_PROCSYS * localProc;						// ローカルＰＲＯＣ
	GFL_PROC_MAIN_STATUS	procStatus;		// ローカルＰＲＯＣの戻り値

	u16 * list;			// リストデータ
	u16	listMax;		// リストデータ数
	u16	listMode;		// リストモード

  u16 *habitatLst;	// リストデータ
	u16	habitatListMax;		    // リストデータ数

	ZKNCOMM_LIST_SORT	sort;		// ソートデータ

	int	mainSeq;			// メインシーケンス
	int nextSeq;			// 次のシーケンス

  ZUKAN_DETAIL_TYPE   detail_type;    // SEQ_INFO_CALLのときのみ使用
  BOOL                is_habitat_call;// 生息地リストからの呼び出し 
  BOOL                is_info_call;   // 詳細画面からの呼び出し

	void * work;			// 各処理のワーク
}ZUKAN_MAIN_WORK;

typedef int (*pZUKAN_FUNC)(ZUKAN_MAIN_WORK*);


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT ZukanProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

static void FreeListData( ZUKAN_MAIN_WORK * wk );
static void FreeHabitatListData( ZUKAN_MAIN_WORK * wk );

static int MainSeq_LocalProcWait( ZUKAN_MAIN_WORK * wk );

static int MainSeq_CallTop( ZUKAN_MAIN_WORK * wk );
static int MainSeq_EndTop( ZUKAN_MAIN_WORK * wk );

static int MainSeq_CallList( ZUKAN_MAIN_WORK * wk );
static int MainSeq_EndList( ZUKAN_MAIN_WORK * wk );

static int MainSeq_CallSearch( ZUKAN_MAIN_WORK * wk );
static int MainSeq_EndSearch( ZUKAN_MAIN_WORK * wk );

static int MainSeq_CallDetail( ZUKAN_MAIN_WORK * wk );
static int MainSeq_EndDetail( ZUKAN_MAIN_WORK * wk );

static int MainSeq_CallHabitatZone( ZUKAN_MAIN_WORK * wk );
static int MainSeq_EndHabitatZone( ZUKAN_MAIN_WORK * wk );

static int MainSeq_CallHabitatZoneList( ZUKAN_MAIN_WORK * wk );
static int MainSeq_EndHabitatZoneList( ZUKAN_MAIN_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================

// PROCデータ
const GFL_PROC_DATA ZUKAN_ProcData = {
	ZukanProc_Init,
	ZukanProc_Main,
	ZukanProc_End
};

// メインシーケンス関数テーブル
static const pZUKAN_FUNC MainSeq[] = {
	MainSeq_LocalProcWait,	// 各処理の終了待ち

	MainSeq_CallTop,				// トップ呼び出し
	MainSeq_EndTop,					// トップ終了後

	MainSeq_CallList,				// リスト呼び出し
	MainSeq_EndList,				// リスト終了後

	MainSeq_CallDetail,			// 情報呼び出し
	MainSeq_EndDetail,			// 情報終了後

	MainSeq_CallSearch,			// 検索呼び出し
	MainSeq_EndSearch,			// 検索終了後
  
	MainSeq_CallHabitatZone,// 生息地リストゾーン詳細画面呼出
	MainSeq_EndHabitatZone,	// 生息地リストゾーン詳細画面呼出終了後
  
	MainSeq_CallHabitatZoneList,// 生息地リストリスト画面呼出
	MainSeq_EndHabitatZoneList,	// 生息地リストゾーンリスト画面呼出終了後
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：初期化
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT ZukanProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	ZUKAN_MAIN_WORK * wk;

//	OS_Printf( "↓↓↓↓↓　図鑑処理開始　↓↓↓↓↓\n" );

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_SYS, 0x10000 );

	wk = GFL_PROC_AllocWork( proc, sizeof(ZUKAN_MAIN_WORK), HEAPID_ZUKAN_SYS );
	GFL_STD_MemClear( wk, sizeof(ZUKAN_MAIN_WORK) );

	wk->prm = pwk;

	wk->localProc = GFL_PROC_LOCAL_boot( HEAPID_ZUKAN_SYS );

	wk->listMax  = ZKNCOMM_MakeDefaultList( wk->prm->savedata, &wk->list, HEAPID_ZUKAN_SYS );
	wk->listMode = ZKNLIST_CALL_NORMAL;
	ZKNCOMM_InitDefaultMons( wk->prm->savedata, wk->list, wk->listMax );
	ZKNCOMM_ResetSortData( wk->prm->savedata, &wk->sort );
  
  // ゾーン詳細画面のデフォルト季節と地形を設定する
  {
    SAVE_CONTROL_WORK *p_svcont = GAMEDATA_GetSaveControlWork( wk->prm->gamedata );
    HABITATLIST_SAVE *p_habitat_sv = SaveData_GetHabitatListSave( p_svcont );
    HABITATLIST_SAVE_SetDefaultSeason( p_habitat_sv,PMSEASON_GetRealTimeSeasonID() );
    HABITATLIST_SAVE_SetDefaultGround( p_habitat_sv, HABITAT_GROUND_LAND );
  }


	if( wk->prm->callMode == ZUKAN_MODE_TOP ){
		wk->mainSeq = SEQ_TOP_CALL;
	}else if( wk->prm->callMode == ZUKAN_MODE_LIST ){
    if( ZUKANSAVE_GetZukanMode( wk->prm->savedata ) == ZUKAN_MODE_HABITAT ){
      wk->mainSeq = SEQ_HABITAT_ZONE_LIST_CALL;
    }
    else
    {
      wk->mainSeq = SEQ_LIST_CALL;
    }
	}else if( wk->prm->callMode == ZUKAN_MODE_INFO ){
		wk->mainSeq = SEQ_INFO_CALL;
    wk->detail_type = ZUKAN_DETAIL_TYPE_INFO;
	}else if( wk->prm->callMode == ZUKAN_MODE_MAP ){
		wk->mainSeq = SEQ_INFO_CALL;
    wk->detail_type = ZUKAN_DETAIL_TYPE_MAP;
	}else if( wk->prm->callMode == ZUKAN_MODE_VOICE ){
		wk->mainSeq = SEQ_INFO_CALL;
    wk->detail_type = ZUKAN_DETAIL_TYPE_VOICE;
	}else if( wk->prm->callMode == ZUKAN_MODE_FORM ){
		wk->mainSeq = SEQ_INFO_CALL;
    wk->detail_type = ZUKAN_DETAIL_TYPE_FORM;
	}else if( wk->prm->callMode == ZUKAN_MODE_HABITAT_LIST ){
		wk->mainSeq = SEQ_HABITAT_ZONE_LIST_CALL;
	}else if( wk->prm->callMode == ZUKAN_MODE_HABITAT_ZONE ){
		wk->mainSeq = SEQ_HABITAT_ZONE_CALL;
	}else{
		wk->mainSeq = SEQ_SEARCH_CALL;
	}

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：メイン
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT ZukanProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	ZUKAN_MAIN_WORK * wk = mywk;

	if( wk->mainSeq == SEQ_PROC_FINISH ){
		return GFL_PROC_RES_FINISH;
	}

	wk->procStatus = GFL_PROC_LOCAL_Main( wk->localProc );

	wk->mainSeq = MainSeq[wk->mainSeq]( wk );

	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：終了
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT ZukanProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	ZUKAN_MAIN_WORK * wk = mywk;

	FreeListData( wk );
  FreeHabitatListData(wk);

	GFL_PROC_LOCAL_Exit( wk->localProc );

	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_ZUKAN_SYS );

//	OS_Printf( "↑↑↑↑↑　図鑑処理終了　↑↑↑↑↑\n" );

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リスト解放
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void FreeListData( ZUKAN_MAIN_WORK * wk )
{
	if( wk->list != NULL ){
		GFL_HEAP_FreeMemory( wk->list );
		wk->list = NULL;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  生息地用リスト画面
 *
 *	@param	* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void FreeHabitatListData( ZUKAN_MAIN_WORK * wk )
{
	if( wk->habitatLst != NULL ){
		GFL_HEAP_FreeMemory( wk->habitatLst );
		wk->habitatLst = NULL;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		各処理の終了待ちへ
 *
 * @param		wk		図鑑メインワーク
 * @param		next	終了後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int NextProcCall( ZUKAN_MAIN_WORK * wk, int next )
{
	wk->nextSeq = next;
	return SEQ_LOCAL_PROC_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：各処理の終了待ち
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_LocalProcWait( ZUKAN_MAIN_WORK * wk )
{
	if( wk->procStatus != GFL_PROC_MAIN_VALID ){
		return wk->nextSeq;
	}
	return SEQ_LOCAL_PROC_WAIT;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：トップ画面呼び出し
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_CallTop( ZUKAN_MAIN_WORK * wk )
{
	ZUKANTOP_DATA * top;

	wk->work = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_SYS, sizeof(ZUKANTOP_DATA) );
	top = wk->work;

	top->gamedata = wk->prm->gamedata;
	top->mystatus = wk->prm->mystatus;

	GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(zukan_top), &ZUKANTOP_ProcData, wk->work );

	return NextProcCall( wk, SEQ_TOP_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：トップ画面終了後
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EndTop( ZUKAN_MAIN_WORK * wk )
{
	ZUKANTOP_DATA * top;
	int	ret;

	top = wk->work;

	switch( top->retMode ){
	case ZKNTOP_RET_EXIT:			// 図鑑終了
		wk->prm->retMode = ZUKAN_RET_NORMAL;
		ret = SEQ_PROC_FINISH;
		break;

	case ZKNTOP_RET_EXIT_X:		// 図鑑を終了してメニューを閉じる
		wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
		ret = SEQ_PROC_FINISH;
		break;

	case ZKNTOP_RET_LIST:			// リスト画面へ
    if( ZUKANSAVE_GetZukanMode( wk->prm->savedata ) == ZUKAN_MODE_HABITAT ){
      ret = SEQ_HABITAT_ZONE_LIST_CALL;
    }
    else
    {
      ret = SEQ_LIST_CALL;
    }
		break;
	}

	GFL_HEAP_FreeMemory( wk->work );

	return ret;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：リスト画面呼び出し
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_CallList( ZUKAN_MAIN_WORK * wk )
{
	ZUKANLIST_DATA * list;

	wk->work = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_SYS, sizeof(ZUKANLIST_DATA) );
	list = wk->work;

	list->gamedata = wk->prm->gamedata;
	list->savedata = wk->prm->savedata;

  // 生息地リストモードで呼ばれていはいけない
  if( ZUKANSAVE_GetZukanMode( list->savedata ) == ZUKAN_MODE_HABITAT ){
    // 念のため呼ばれた場合はイッシュ図鑑にしておく
    ZUKANSAVE_SetZukanMode( wk->prm->savedata , ZUKAN_MODE_LOCAL);
    GF_ASSERT( 0 );
  }

	list->callMode = wk->listMode;
	if( wk->listMode == ZKNLIST_CALL_NORMAL && ZKNCOMM_CheckLocalListNumberZero(wk->prm->savedata) == FALSE ){
		list->list    = &wk->list[1];
		list->listMax = wk->listMax-1;
	}else{
		list->list    = wk->list;
		list->listMax = wk->listMax;
	}
	GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(zukan_list), &ZUKANLIST_ProcData, wk->work );

	return NextProcCall( wk, SEQ_LIST_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：リスト画面終了後
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EndList( ZUKAN_MAIN_WORK * wk )
{
	ZUKANLIST_DATA * list;
	int	ret;

	list = wk->work;

	switch( list->retMode ){
	case ZKNLIST_RET_EXIT:		// 図鑑終了
		if( wk->listMode == ZKNLIST_CALL_NORMAL ){
			wk->prm->retMode = ZUKAN_RET_NORMAL;
			ret = SEQ_PROC_FINISH;
		}else{
			FreeListData( wk );
			wk->listMax  = ZKNCOMM_MakeDefaultList( wk->prm->savedata, &wk->list, HEAPID_ZUKAN_SYS );
			wk->listMode = ZKNLIST_CALL_NORMAL;
			ret = SEQ_LIST_CALL;
		}
		break;

	case ZKNLIST_RET_EXIT_X:	// 図鑑を終了してメニューを閉じる
		wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
		ret = SEQ_PROC_FINISH;
		break;

	case ZKNLIST_RET_MODE_CHANGE:	// 図鑑モードを切り替える
		FreeListData( wk );
		wk->listMax = ZKNCOMM_MakeDefaultList( wk->prm->savedata, &wk->list, HEAPID_ZUKAN_SYS );
		ret = SEQ_LIST_CALL;
		break;

	case ZKNLIST_RET_INFO:		// 詳細画面へ
		ret = SEQ_INFO_CALL;
    wk->detail_type = ZUKAN_DETAIL_TYPE_INFO;
		break;

	case ZKNLIST_RET_SEARCH:	// 検索画面へ
		if( wk->listMode == ZKNLIST_CALL_SEARCH ){
			FreeListData( wk );
			wk->listMax = ZKNCOMM_MakeDefaultList( wk->prm->savedata, &wk->list, HEAPID_ZUKAN_SYS );
		}
		ret = SEQ_SEARCH_CALL;
		break;

  case ZKNLIST_RET_HABITAT_LIST:
    ret = SEQ_HABITAT_ZONE_LIST_CALL;
    break;

	default:
		wk->prm->retMode = ZUKAN_RET_NORMAL;
		ret = SEQ_PROC_FINISH;
	}

	GFL_HEAP_FreeMemory( wk->work );

	return ret;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：検索画面呼び出し
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_CallSearch( ZUKAN_MAIN_WORK * wk )
{
	ZUKANSEARCH_DATA * search;

	wk->work = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_SYS, sizeof(ZUKANSEARCH_DATA) );
	search = wk->work;

	search->gamedata = wk->prm->gamedata;
	search->savedata = wk->prm->savedata;
	search->sort     = &wk->sort;
	search->list     = NULL;
	search->listMax  = 0;
	GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(zukan_search), &ZUKANSEARCH_ProcData, wk->work );

	return NextProcCall( wk, SEQ_SEARCH_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：検索画面終了後
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EndSearch( ZUKAN_MAIN_WORK * wk )
{
	ZUKANSEARCH_DATA * search;
	int	ret;

	search = wk->work;

	switch( search->retMode ){
	case ZKNSEARCH_RET_START:		// 検索実行
		FreeListData( wk );
    
    // Yボタンで起動し、図鑑モードが生息地だった場合は戻り先は地方図鑑
    if( ZUKANSAVE_GetZukanMode( wk->prm->savedata ) == ZUKAN_MODE_HABITAT ){
      ZUKANSAVE_SetZukanMode( wk->prm->savedata , ZUKAN_MODE_LOCAL );
    }

		wk->list     = search->list;
		wk->listMax  = search->listMax;
		wk->listMode = ZKNLIST_CALL_SEARCH;
		ret = SEQ_LIST_CALL;
		break;
	
	case ZKNSEARCH_RET_CANCEL:	// キャンセル
    // Yボタンで起動し、図鑑モードが生息地だった場合は戻り先は地方図鑑
    if( ZUKANSAVE_GetZukanMode( wk->prm->savedata ) == ZUKAN_MODE_HABITAT ){
      ZUKANSAVE_SetZukanMode( wk->prm->savedata , ZUKAN_MODE_LOCAL );
    }
#ifdef	BUGFIX_BTS7745_20100712
		wk->listMode = ZKNLIST_CALL_NORMAL;
#endif	// BUGFIX_BTS7745_20100712
		ret = SEQ_LIST_CALL;
		break;

	case ZKNSEARCH_RET_EXIT_X:	// 図鑑を終了してメニューを閉じる
	default:
		wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
		ret = SEQ_PROC_FINISH;
		break;
	}

	GFL_HEAP_FreeMemory( wk->work );

	return ret;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：図鑑詳細画面呼び出し
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_CallDetail( ZUKAN_MAIN_WORK * wk )
{
  ZUKAN_DETAIL_PARAM*   detail;

  GAMEDATA*             gamedata       = wk->prm->gamedata;
  ZUKAN_SAVEDATA*       zukan_savedata = GAMEDATA_GetZukanSave( gamedata );
  u16                   monsno         = ZUKANSAVE_GetDefaultMons( zukan_savedata );

	detail = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_SYS, sizeof(ZUKAN_DETAIL_PARAM) );
  
  detail->gamedata = wk->prm->gamedata;
  detail->type     = wk->detail_type;
  detail->no       = 0;
  detail->is_habitat_call = wk->is_habitat_call;

  // 使用するリストデータの切替
  if( wk->is_habitat_call == TRUE )
  {
    detail->list  = wk->habitatLst;
    detail->num   = wk->habitatListMax;
  }
  else
  {
    detail->list     = wk->list;
    detail->num      = wk->listMax;
  }

  {
    u16 i;
    for( i=0; i<detail->num; i++ )
    {
      if( detail->list[i] == monsno )
      {
        detail->no = i;
        break;
      }
    }
  }

	GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(zukan_detail), &ZUKAN_DETAIL_ProcData, detail );

  wk->work = detail;	
  return NextProcCall( wk, SEQ_INFO_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：図鑑詳細画面終了後
 *
 * @param		wk		図鑑メインワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EndDetail( ZUKAN_MAIN_WORK * wk )
{
	ZUKAN_DETAIL_PARAM*   detail;

  GAMEDATA*             gamedata       = wk->prm->gamedata;
  ZUKAN_SAVEDATA*       zukan_savedata = GAMEDATA_GetZukanSave( gamedata );
  u16                   monsno;

	int	                  ret;

	detail = wk->work;

  // 生息地リストから呼び出されている場合は初期化しない
  if( wk->is_habitat_call == FALSE)
  {
    wk->is_info_call = FALSE; // 呼び出し指定初期化
  }

  {
    monsno = detail->list[detail->no];
    ZUKANSAVE_SetDefaultMons( zukan_savedata, monsno );
  }

  switch( detail->ret )
  {
  case ZUKAN_DETAIL_RET_CLOSE:
    {
		  wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
      ret = SEQ_PROC_FINISH;
    }
    break;
  case ZUKAN_DETAIL_RET_CALL_HABITAT:
    {
      ret = SEQ_HABITAT_ZONE_CALL;
      wk->is_info_call = TRUE;
    }
    break;
  case ZUKAN_DETAIL_RET_RETURN:
    {
      if( wk->is_habitat_call == FALSE )
      {
        // Yボタンで起動し、図鑑モードが生息地だった場合は戻り先は地方図鑑
        if( ZUKANSAVE_GetZukanMode( wk->prm->savedata ) == ZUKAN_MODE_HABITAT ){
          ZUKANSAVE_SetZukanMode( wk->prm->savedata , ZUKAN_MODE_LOCAL );
        }
        wk->prm->retMode = ZUKAN_RET_NORMAL;
        ret = SEQ_LIST_CALL;
      }
      else
      {
        wk->prm->retMode = ZUKAN_RET_NORMAL;
        ret = SEQ_HABITAT_ZONE_CALL;
      }
    }
    break;
  }
   
  // 生息地用リストデータが作成されていた場合は削除
  if( wk->is_habitat_call == TRUE )
  {
    FreeHabitatListData( wk );
  }
		
	GFL_HEAP_FreeMemory( wk->work );

	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  生息地リスト：ゾーン詳細画面呼出
 *
 *	@param	* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int MainSeq_CallHabitatZone( ZUKAN_MAIN_WORK * wk )
{
  ZUKANHABITAT_ZONE_DATA * p_zone_info;
	SAVE_CONTROL_WORK *p_svcont = GAMEDATA_GetSaveControlWork( wk->prm->gamedata );
  HABITATLIST_SAVE *p_habitat_sv = SaveData_GetHabitatListSave( p_svcont );

	wk->work = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_SYS, sizeof(ZUKANHABITAT_ZONE_DATA) );
	GFL_STD_MemClear( wk->work, sizeof(ZUKANHABITAT_ZONE_DATA) );
	p_zone_info = wk->work;
	p_zone_info->p_gdata = wk->prm->gamedata;

  // リストをデータを作成
  if( (wk->is_info_call == TRUE) && (wk->is_habitat_call == FALSE) )
  {
    ONOUE_Printf("生息地リスト：単一呼出%d\n",0);
    ZKNCOMM_MakeHabitatZoneListData( p_habitat_sv,p_zone_info ,TRUE );
  }
  else if( (wk->is_info_call == TRUE) && (wk->is_habitat_call == TRUE) )
  {
    ONOUE_Printf("生息地リスト：単一呼出%d\n",0);
    ZKNCOMM_MakeHabitatZoneListData( p_habitat_sv,p_zone_info ,TRUE );
  }
  else
  {
    ONOUE_Printf("生息地リスト：セーブからリスト生成\n");
    ZKNCOMM_MakeHabitatZoneListData( p_habitat_sv,p_zone_info ,FALSE );
  }
  
	GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(zukan_habitat_zone),
                                      &ZUKANHABITAT_ZONE_ProcData, p_zone_info );
  return NextProcCall( wk, SEQ_HABITAT_ZONE_END );
}

//----------------------------------------------------------------------------
/**
 *	@brief  生息地リスト：ゾーン詳細画面終了後
 *
 *	@param	* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int MainSeq_EndHabitatZone( ZUKAN_MAIN_WORK * wk )
{
  int ret;
  ZUKANHABITAT_ZONE_DATA * p_zone_info;
  p_zone_info = wk->work;

  switch( p_zone_info->ret_mode ){
  case HABITAT_ZONE_RET_EXIT:		// 図鑑終了
    wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
    ret = SEQ_PROC_FINISH;
    wk->is_habitat_call = FALSE;
    break;

  case HABITAT_ZONE_RET_RETURN:
    if( wk->is_info_call == TRUE )
    {
      ONOUE_Printf("詳細画面へ戻る\n");
      wk->detail_type = ZUKAN_DETAIL_TYPE_MAP;
      ret = SEQ_INFO_CALL;
    }
    else
    {
      // Yボタンからきた場合は、図鑑モードが全国もしくはイッシュになっている可能性がある
      ZUKANSAVE_SetZukanMode( wk->prm->savedata , ZUKAN_MODE_HABITAT);
      ret = SEQ_HABITAT_ZONE_LIST_CALL;
      ONOUE_Printf("リスト画面へ戻る\n");
    }
    wk->is_habitat_call = FALSE;
    break;

  case HABITAT_ZONE_RET_INFO:
    {
      SAVE_CONTROL_WORK *p_svcont = GAMEDATA_GetSaveControlWork( wk->prm->gamedata );
      HABITATLIST_SAVE *p_habitat_sv = SaveData_GetHabitatListSave( p_svcont );
      ret = SEQ_INFO_CALL;
      wk->detail_type = ZUKAN_DETAIL_TYPE_INFO;
      wk->is_habitat_call = TRUE; // 生息地リストモードに切り替え

      // 指定ゾーンリストを作成
      wk->habitatListMax = ZKNCOMM_MakeHabitatList( wk->prm->savedata,p_habitat_sv,&wk->habitatLst, HEAPID_ZUKAN_SYS );
    }
    break;

  default:
    GF_ASSERT(0);
    wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
    ret = SEQ_PROC_FINISH;
    wk->is_habitat_call = FALSE;
    break;
  }
		
  // ワーク開放
	GFL_HEAP_FreeMemory( wk->work );

  // 終了モードの設定
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  生息地リスト：リスト画面呼出
 *
 *	@param	* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int MainSeq_CallHabitatZoneList( ZUKAN_MAIN_WORK * wk )
{
  ZUKANHABITAT_LIST_DATA * p_zone_list;
	SAVE_CONTROL_WORK *p_svcont = GAMEDATA_GetSaveControlWork( wk->prm->gamedata );
  HABITATLIST_SAVE *p_habitat_sv = SaveData_GetHabitatListSave( p_svcont );

	wk->work = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_SYS, sizeof(ZUKANHABITAT_LIST_DATA) );
	GFL_STD_MemClear( wk->work, sizeof(ZUKANHABITAT_LIST_DATA) );
	p_zone_list = wk->work;
	p_zone_list->p_gdata = wk->prm->gamedata;

  // リストデータを作成する
  FreeListData( wk );
  FreeHabitatListData( wk );
  ZKNCOMM_MakeHabitatListData( p_habitat_sv,p_zone_list );

#ifdef PM_DEBUG
  if( p_zone_list->list_size == 0 )
  {
    GF_ASSERT(0);
    GFL_HEAP_FreeMemory( wk->work );
    ZUKANSAVE_SetZukanMode( wk->prm->savedata , ZUKAN_MODE_LOCAL);
    wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
    return NextProcCall( wk, SEQ_PROC_FINISH );
  }
#endif

  GFL_PROC_LOCAL_CallProc( wk->localProc,
      FS_OVERLAY_ID(zukan_habitat_list), &ZUKANHABITAT_LIST_ProcData, p_zone_list );
  return NextProcCall( wk, SEQ_HABITAT_ZONE_LIST_END );
}

//----------------------------------------------------------------------------
/**
 *	@brief  生息地リスト：リスト画面終了後
 *
 *	@param	* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int MainSeq_EndHabitatZoneList( ZUKAN_MAIN_WORK * wk )
{
	int ret;
  ZUKANHABITAT_LIST_DATA * zone_list;
  
  zone_list = wk->work;

  switch( zone_list->ret_mode )
  {
  case HABITAT_LIST_RET_EXIT:
    ret = SEQ_PROC_FINISH;
    wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
    break;
  case HABITAT_LIST_RET_RETURN:
    ret = SEQ_PROC_FINISH;
    wk->prm->retMode = ZUKAN_RET_NORMAL;
    break;
  case HABITAT_LIST_RET_CALL_ZONE_INFO:
    {
      // デフォルト季節を今の季節に変更
      {
        SAVE_CONTROL_WORK *p_svcont = GAMEDATA_GetSaveControlWork( wk->prm->gamedata );
        HABITATLIST_SAVE *p_habitat_sv = SaveData_GetHabitatListSave( p_svcont );
        HABITATLIST_SAVE_SetDefaultSeason( p_habitat_sv,PMSEASON_GetRealTimeSeasonID() );
        HABITATLIST_SAVE_SetDefaultGround( p_habitat_sv, HABITAT_GROUND_LAND );
      }
      ZUKANSAVE_SetZukanMode( wk->prm->savedata , ZUKAN_MODE_HABITAT);
      ret = SEQ_HABITAT_ZONE_CALL;
    }
    break;
  case HABITAT_LIST_RET_ZENKOKU:
    {
      // 現在のリストモードは必ず生息地リスト
      if( ZUKANSAVE_GetZukanMode( wk->prm->savedata ) != ZUKAN_MODE_HABITAT ){
        GF_ASSERT(0);
      }
      ZUKANSAVE_SetZukanMode( wk->prm->savedata , ZUKAN_MODE_ZENKOKU);
      FreeListData( wk );
      wk->listMax = ZKNCOMM_MakeDefaultList( wk->prm->savedata, &wk->list, HEAPID_ZUKAN_SYS );
      ret = SEQ_LIST_CALL;
    }
    break;
  case HABITAT_LIST_RET_LOCAL:
    {
      // 現在のリストモードは必ず生息地リスト
      if( ZUKANSAVE_GetZukanMode( wk->prm->savedata ) != ZUKAN_MODE_HABITAT ){
        GF_ASSERT(0);
      }
      ZUKANSAVE_SetZukanMode( wk->prm->savedata , ZUKAN_MODE_LOCAL);
      FreeListData( wk );
      wk->listMax = ZKNCOMM_MakeDefaultList( wk->prm->savedata, &wk->list, HEAPID_ZUKAN_SYS );
      ret = SEQ_LIST_CALL;
    }
    break;
  default:
    GF_ASSERT(0);
    ret = SEQ_PROC_FINISH;
    wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
    break;
  }

  // ワーク開放
  GFL_HEAP_FreeMemory( wk->work );
  
  return ret;
}

