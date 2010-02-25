//=============================================================================
/**
 *
 *	@file		debug_hosaka_menu.c
 *	@brief  保坂デバッグメニュー
 *	@author		hosaka genya
 *	@data		2009.10.21
 *
 */
//=============================================================================

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//static BOOL TESTMODE_ITEM_SelectIntro( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectUNSelect( TESTMODE_WORK* work, const int idx );
static BOOL TESTMODE_ITEM_SelectCmmBtlDemoEnd( TESTMODE_WORK* work, const int idx );
static BOOL TESTMODE_ITEM_SelectCmmBtlDemoEndMulti( TESTMODE_WORK* work, const int idx );
static BOOL TESTMODE_ITEM_SelectCmmBtlDemoStart( TESTMODE_WORK* work, const int idx );
static BOOL TESTMODE_ITEM_SelectCmmBtlDemoStartMulti( TESTMODE_WORK* work, const int idx );
static BOOL TESTMODE_ITEM_SelectPmsSelect( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectPmsDirect( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectPmsPeculiar( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectPmsSingle( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectPmsDouble( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectMicTest( TESTMODE_WORK *work , const int idx );

// メニューリスト
static TESTMODE_MENU_LIST menuHosaka[] = 
{
//	{L"イントロデモ",TESTMODE_ITEM_SelectIntro },
  {L"国連フロア選択",TESTMODE_ITEM_SelectUNSelect},
  {L"通信バトル後デモ",TESTMODE_ITEM_SelectCmmBtlDemoEnd},
  {L"通信バトル後マルチ",TESTMODE_ITEM_SelectCmmBtlDemoEndMulti},
  {L"通信バトル前デモ",TESTMODE_ITEM_SelectCmmBtlDemoStart},
  {L"通信バトル前マルチ",TESTMODE_ITEM_SelectCmmBtlDemoStartMulti},
	{L"かんい会話選択",TESTMODE_ITEM_SelectPmsSelect },
	{L"かんい会話直呼び",TESTMODE_ITEM_SelectPmsDirect },
	{L"かんい会話固定",TESTMODE_ITEM_SelectPmsPeculiar },
	{L"かんい会話一単語",TESTMODE_ITEM_SelectPmsSingle },
	{L"かんい会話二単語",TESTMODE_ITEM_SelectPmsDouble },
	{L"マイクテスト",TESTMODE_ITEM_SelectMicTest },
	
	{L"もどる"				,TESTMODE_ITEM_BackTopMenu },
};

//=============================================================================
/**
 *								static関数
 */
//=============================================================================
#include "app/un_select.h"

static BOOL TESTMODE_ITEM_SelectUNSelect( TESTMODE_WORK* work, const int idx )
{
  UN_SELECT_PARAM* initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(UN_SELECT_PARAM) );

	TESTMODE_COMMAND_ChangeProc(work, FS_OVERLAY_ID(un_select), &UNSelectProcData, initParam);

  return TRUE;
}

#if 0
// イントロデモ
#include "demo/intro.h"
static BOOL TESTMODE_ITEM_SelectIntro( TESTMODE_WORK *work , const int idx )
{
  INTRO_PARAM* initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(INTRO_PARAM) );

  initParam->save_ctrl = SaveControl_GetPointer();

	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(intro), &ProcData_Intro, initParam);

  return TRUE;
}
#endif
  
#include "demo/comm_btl_demo.h"

// デバッグ用のパラメータ設定
//@TODO ALLOCしっぱなし
// ワーク生成
static void debug_param( COMM_BTL_DEMO_PARAM* prm )
{ 
  int i;

  HOSAKA_Printf("in param type = %d \n", prm->type);
  
  prm->result = GFUser_GetPublicRand( COMM_BTL_DEMO_RESULT_MAX );

  HOSAKA_Printf( "result = %d \n", prm->result );

  for( i=0; i<COMM_BTL_DEMO_TRDATA_MAX; i++ )
  {
    prm->trainer_data[i].server_version = GFUser_GetPublicRand(2);

    {
      //@TODO ALLOCしっぱなし
      MYSTATUS* st = GFL_HEAP_AllocMemoryLo( GFL_HEAPID_APP, MyStatus_GetWorkSize() );

      MyStatus_Copy( SaveData_GetMyStatus( SaveControl_GetPointer() ), st );

      if( GFUser_GetPublicRand(2) == 0 )
      {
        STRCODE debugname[8] = L"ブラック";
        debugname[4] = GFL_STR_GetEOMCode();

        MyStatus_SetMySex( st, PM_MALE );
        MyStatus_SetMyName( st, debugname);
      }
      else
      {
        STRCODE debugname[8] = L"ホワイト";
        debugname[4] = GFL_STR_GetEOMCode();

        MyStatus_SetMySex( st, PM_FEMALE );
        MyStatus_SetMyName( st, debugname);
      }

      prm->trainer_data[i].mystatus = st;
    }

#if 0
 // 廃止予定
//    prm->trainer_data[i].trsex = PM_FEMALE;//(GFUser_GetPublicRand(2)==0) ? PM_MALE : PM_FEMALE;  
    // トレーナー名
    {
      //終端コードを追加してからSTRBUFに変換
      STRCODE debugname[32] = L"とうふ";
      
      debugname[3] = GFL_STR_GetEOMCode();

      prm->trainer_data[i].str_trname = GFL_STR_CreateBuffer( sizeof(STRCODE)*10, GFL_HEAPID_APP );
      GFL_STR_SetStringCode( prm->trainer_data[i].str_trname, debugname );
    }
#endif
    
    // デバッグポケパーティー
    {
      POKEPARTY *party;
      int poke_cnt;
      int p;

      party = PokeParty_AllocPartyWork( GFL_HEAPID_APP );

      if( prm->type == COMM_BTL_DEMO_TYPE_NORMAL_END || prm->type == COMM_BTL_DEMO_TYPE_NORMAL_START )
      {
        Debug_PokeParty_MakeParty( party );
      }
      else
      {
        static const int pokemax=3;
        PokeParty_Init(party, pokemax);
        for (p = 0; p < pokemax; p++) 
        {
          POKEMON_PARAM* pp = GFL_HEAP_AllocMemoryLo( GFL_HEAPID_APP , POKETOOL_GetWorkSize() );
          PP_Clear(pp);
          PP_Setup( pp, 392+p, 99, 0 );
          PokeParty_Add(party, pp);
          GFL_HEAP_FreeMemory(pp);
        }
      }
      
      prm->trainer_data[i].party = party;

      poke_cnt = PokeParty_GetPokeCount( prm->trainer_data[i].party );

      if( prm->type == COMM_BTL_DEMO_TYPE_NORMAL_END || prm->type == COMM_BTL_DEMO_TYPE_MULTI_END )
      {
        // 対戦後のポケモンの状態異常
        for( p=0; p<poke_cnt; p++ )
        {
          POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, p );
          switch( GFUser_GetPublicRand(3) )
          {
          case 0: PP_SetSick( pp, POKESICK_DOKU ); break; // 毒
          case 1: PP_Put(pp, ID_PARA_hp, 0 ); break; // 瀕死
          }
          HOSAKA_Printf("poke [%d] condition=%d \n",p, PP_Get( pp, ID_PARA_condition, NULL ) );
        }
      }

      HOSAKA_Printf("[%d] server_version=%d trsex=%d poke_cnt=%d \n",i, 
          prm->trainer_data[i].server_version,
          MyStatus_GetMySex( prm->trainer_data[i].mystatus ),
          poke_cnt );
    }
  }
}


static BOOL TESTMODE_ITEM_SelectCmmBtlDemoEnd( TESTMODE_WORK* work, const int idx )
{
  COMM_BTL_DEMO_PARAM* initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(COMM_BTL_DEMO_PARAM) );

  initParam->type = COMM_BTL_DEMO_TYPE_NORMAL_END;
  debug_param(initParam);

	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(comm_btl_demo), &CommBtlDemoProcData, initParam);

  return TRUE;
}

static BOOL TESTMODE_ITEM_SelectCmmBtlDemoEndMulti( TESTMODE_WORK* work, const int idx )
{
  COMM_BTL_DEMO_PARAM* initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(COMM_BTL_DEMO_PARAM) );

  initParam->type = COMM_BTL_DEMO_TYPE_MULTI_END;
  debug_param(initParam);

	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(comm_btl_demo), &CommBtlDemoProcData, initParam);

  return TRUE;
}

static BOOL TESTMODE_ITEM_SelectCmmBtlDemoStart( TESTMODE_WORK* work, const int idx )
{ 
  COMM_BTL_DEMO_PARAM* initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(COMM_BTL_DEMO_PARAM) );

  initParam->type = COMM_BTL_DEMO_TYPE_NORMAL_START;
  debug_param(initParam);

	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(comm_btl_demo), &CommBtlDemoProcData, initParam);

  return TRUE;
}

static BOOL TESTMODE_ITEM_SelectCmmBtlDemoStartMulti( TESTMODE_WORK* work, const int idx )
{
  COMM_BTL_DEMO_PARAM* initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(COMM_BTL_DEMO_PARAM) );

  initParam->type = COMM_BTL_DEMO_TYPE_MULTI_START;
  debug_param(initParam);

	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(comm_btl_demo), &CommBtlDemoProcData, initParam);

  return TRUE;
}


// 簡易会話
#include "app/pms_select.h"
#include "app/pms_input.h"
FS_EXTERN_OVERLAY(pmsinput);

//#define TESTMODE_PMS_USE_PROC

#ifdef TESTMODE_PMS_USE_PROC
typedef struct
{
  HEAPID                parent_heap_id;
  TestMode_SelectFunc   func;
}
TESTMODE_PMS_PARAM;

typedef struct
{
  HEAPID                heap_id;
  GFL_PROCSYS*          local_procsys;
  void*                 local_proc_param;
}
TESTMODE_PMS_WORK;

static TESTMODE_PMS_PARAM*  TESTMODE_PMS_CreateParam( HEAPID heap_id, TestMode_SelectFunc func );
static void                 TESTMODE_PMS_DeleteParam( TESTMODE_PMS_PARAM* param );

static TESTMODE_PMS_PARAM*  TESTMODE_PMS_CreateParam( HEAPID heap_id, TestMode_SelectFunc func )
{
  TESTMODE_PMS_PARAM* param = GFL_HEAP_AllocClearMemory( heap_id, sizeof(TESTMODE_PMS_PARAM) );
  param->parent_heap_id   = heap_id;
  param->func             = func;
  return param;
}
static void                 TESTMODE_PMS_DeleteParam( TESTMODE_PMS_PARAM* param )
{
  GFL_HEAP_FreeMemory( param );
}

static GFL_PROC_RESULT TESTMODE_PMS_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT TESTMODE_PMS_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT TESTMODE_PMS_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
const GFL_PROC_DATA  TESTMODE_PMS_ProcData =
{
  TESTMODE_PMS_ProcInit,
  TESTMODE_PMS_ProcMain,
  TESTMODE_PMS_ProcExit,
};

static GFL_PROC_RESULT TESTMODE_PMS_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  TESTMODE_PMS_PARAM*  param    = (TESTMODE_PMS_PARAM*)pwk;
  TESTMODE_PMS_WORK*   work;
  
  GFL_HEAP_CreateHeap( param->parent_heap_id, HEAPID_UI_DEBUG, 0x10000 );
  work = GFL_PROC_AllocWork( proc, sizeof(TESTMODE_PMS_WORK), HEAPID_UI_DEBUG );
  
  work->heap_id = HEAPID_UI_DEBUG;
  work->local_procsys = GFL_PROC_LOCAL_boot( work->heap_id );
  
  return GFL_PROC_RES_FINISH;
}
static GFL_PROC_RESULT TESTMODE_PMS_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  TESTMODE_PMS_PARAM*  param    = (TESTMODE_PMS_PARAM*)pwk;
  TESTMODE_PMS_WORK*   work     = (TESTMODE_PMS_WORK*)mywk;

  GFL_PROC_LOCAL_Exit( work->local_procsys );

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_UI_DEBUG );
  
  return GFL_PROC_RES_FINISH;
}
static GFL_PROC_RESULT TESTMODE_PMS_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  TESTMODE_PMS_PARAM*  param    = (TESTMODE_PMS_PARAM*)pwk;
  TESTMODE_PMS_WORK*   work     = (TESTMODE_PMS_WORK*)mywk;

  GFL_PROC_MAIN_STATUS  local_proc_status   =  GFL_PROC_LOCAL_Main( work->local_procsys );
  if( local_proc_status == GFL_PROC_MAIN_VALID ) return GFL_PROC_RES_CONTINUE;

  switch(*seq)
  {
  case 0:
    {
      if( param->func == TESTMODE_ITEM_SelectPmsSelect )
      {
        PMS_SELECT_PARAM* initParam = GFL_HEAP_AllocClearMemory( work->heap_id, sizeof(PMS_SELECT_PARAM) );
        initParam->save_ctrl = SaveControl_GetPointer();
        GFL_PROC_LOCAL_CallProc( work->local_procsys, FS_OVERLAY_ID(pmsinput), &ProcData_PMSSelect, initParam );
        work->local_proc_param = initParam;
      }
      else if( param->func == TESTMODE_ITEM_SelectPmsDirect )
      {
        PMS_DATA data;
        PMSI_PARAM* pmsi;
        PMSDAT_Init( &data, 0 );
        pmsi = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, NULL, TRUE, SaveControl_GetPointer(), work->heap_id );  // デコ文字OK
        //pmsi = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, NULL, FALSE, SaveControl_GetPointer(), work->heap_id );  // デコ文字禁止
        GFL_PROC_LOCAL_CallProc( work->local_procsys, FS_OVERLAY_ID(pmsinput), &ProcData_PMSInput, pmsi );
        work->local_proc_param = pmsi;
      }
      else if( param->func == TESTMODE_ITEM_SelectPmsPeculiar )
      {
        PMS_DATA data;
        PMSI_PARAM* pmsi;
        PMSDAT_Init( &data, 0 );
        //pmsi = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, &data, TRUE, SaveControl_GetPointer(), work->heap_id );  // デコ文字OK
        pmsi = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, &data, FALSE, SaveControl_GetPointer(), work->heap_id );  // デコ文字禁止
        GFL_PROC_LOCAL_CallProc( work->local_procsys, FS_OVERLAY_ID(pmsinput), &ProcData_PMSInput, pmsi );
        work->local_proc_param = pmsi;
      }
      else if( param->func == TESTMODE_ITEM_SelectPmsSingle )
      {
        PMS_DATA data;
        PMSI_PARAM* pmsi;
        PMSDAT_Init( &data, 0 );
        pmsi = PMSI_PARAM_Create( PMSI_MODE_SINGLE, PMSI_GUIDANCE_DEFAULT, NULL, FALSE, SaveControl_GetPointer(), work->heap_id );  // dataは送らない
        GFL_PROC_LOCAL_CallProc( work->local_procsys, FS_OVERLAY_ID(pmsinput), &ProcData_PMSInput, pmsi );
        work->local_proc_param = pmsi;
      }
      else if( param->func == TESTMODE_ITEM_SelectPmsDouble )
      {
        PMS_DATA data;
        PMSI_PARAM* pmsi;
        PMSDAT_Init( &data, 0 );
        //pmsi = PMSI_PARAM_Create( PMSI_MODE_DOUBLE, PMSI_GUIDANCE_DEFAULT, &data, TRUE, SaveControl_GetPointer(), work->heap_id );  // dataを送ってみる
        pmsi = PMSI_PARAM_Create( PMSI_MODE_DOUBLE, PMSI_GUIDANCE_DEFAULT, NULL, TRUE, SaveControl_GetPointer(), work->heap_id );  // 単語モードは文章固定に未対応らしい pmsi_param.c
        GFL_PROC_LOCAL_CallProc( work->local_procsys, FS_OVERLAY_ID(pmsinput), &ProcData_PMSInput, pmsi );
        work->local_proc_param = pmsi;
      }
      (*seq)++;
    }
    break;
  case 1:
    {
      if( local_proc_status != GFL_PROC_MAIN_VALID )
      {
        if( param->func == TESTMODE_ITEM_SelectPmsSelect )
        {
          PMS_SELECT_PARAM* initParam = (PMS_SELECT_PARAM*)work->local_proc_param;
          GFL_HEAP_FreeMemory( initParam );
        }
        else if( param->func == TESTMODE_ITEM_SelectPmsDirect )
        {
          PMSI_PARAM* pmsi = (PMSI_PARAM*)work->local_proc_param;
          PMSI_PARAM_Delete( pmsi );
        }
        else if( param->func == TESTMODE_ITEM_SelectPmsPeculiar )
        {
          PMSI_PARAM* pmsi = (PMSI_PARAM*)work->local_proc_param;
          PMSI_PARAM_Delete( pmsi );
        }
        else if( param->func == TESTMODE_ITEM_SelectPmsSingle )
        {
          PMSI_PARAM* pmsi = (PMSI_PARAM*)work->local_proc_param;
          PMSI_PARAM_Delete( pmsi );
        }
        else if( param->func == TESTMODE_ITEM_SelectPmsDouble )
        {
          PMSI_PARAM* pmsi = (PMSI_PARAM*)work->local_proc_param;
          PMSI_PARAM_Delete( pmsi );
        }
        (*seq)++;
      }
    }
    break;
  case 2:
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}
#endif  // #ifdef TESTMODE_PMS_USE_PROC

// 簡易会話 選択
static BOOL TESTMODE_ITEM_SelectPmsSelect( TESTMODE_WORK *work , const int idx )
{
#ifndef TESTMODE_PMS_USE_PROC
  PMS_SELECT_PARAM* initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(PMS_SELECT_PARAM) );

  initParam->save_ctrl = SaveControl_GetPointer();

	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(pmsinput), &ProcData_PMSSelect, initParam);
	return TRUE;
#else
  TESTMODE_PMS_PARAM* param = TESTMODE_PMS_CreateParam( GFL_HEAPID_APP, TESTMODE_ITEM_SelectPmsSelect );
	TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(testmode), &TESTMODE_PMS_ProcData, param );
	return TRUE;
#endif
}


// 簡易会話 直呼び
static BOOL TESTMODE_ITEM_SelectPmsDirect( TESTMODE_WORK *work , const int idx )
{
#ifndef TESTMODE_PMS_USE_PROC
  PMS_DATA data;
  PMSI_PARAM* pmsi;

/*
  {
    PMSDAT_Init( &data, 0 );
    pmsi = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, NULL, TRUE, SaveControl_GetPointer(), GFL_HEAPID_APP );  // デコ文字OK
  }
*/
/*
  {
    PMSDAT_Init( &data, 0 );
    pmsi = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, NULL, FALSE, SaveControl_GetPointer(), GFL_HEAPID_APP );  // デコ文字禁止
  }
*/
  {
    PMSDAT_Init( &data, PMS_TYPE_BATTLE_WON );  // 戦闘勝利の定型文でスタート
    pmsi = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, NULL, FALSE, SaveControl_GetPointer(), GFL_HEAPID_APP );  // デコ文字禁止
    PMSI_PARAM_SetInitializeDataSentence( pmsi, &data );  // 戦闘勝利の定型文でスタート
  }

	TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(pmsinput), &ProcData_PMSInput, pmsi );

  return TRUE;
#else

  TESTMODE_PMS_PARAM* param = TESTMODE_PMS_CreateParam( GFL_HEAPID_APP, TESTMODE_ITEM_SelectPmsDirect );
	TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(testmode), &TESTMODE_PMS_ProcData, param );
	return TRUE;
#endif
}

// 簡易会話 固定
#include "msg\msg_pmss_peculiar.h"
static BOOL TESTMODE_ITEM_SelectPmsPeculiar( TESTMODE_WORK *work , const int idx )
{
#ifndef TESTMODE_PMS_USE_PROC
  PMS_DATA data;
  PMSI_PARAM* pmsi;

  //PMSDAT_Init( &data, PMS_TYPE_PECULIAR );  // 固有の0番目の定型文で、タグは空で初期化される
  //PMSDAT_SetSentence( &data, PMS_TYPE_PECULIAR, pmss_peculiar_08 );  // 固有の7番目の定型文にする(タグに対しては何もされない)
  PMSDAT_InitAndSetSentence( &data, PMS_TYPE_PECULIAR, pmss_peculiar_08 );  // 固有の7番目の定型文で、タグは空で初期化する
  pmsi = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, &data, TRUE, SaveControl_GetPointer(), GFL_HEAPID_APP );  // デコ文字OK
  //pmsi = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, &data, FALSE, SaveControl_GetPointer(), GFL_HEAPID_APP );  // デコ文字禁止

	TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(pmsinput), &ProcData_PMSInput, pmsi );

  return TRUE;
#else
  TESTMODE_PMS_PARAM* param = TESTMODE_PMS_CreateParam( GFL_HEAPID_APP, TESTMODE_ITEM_SelectPmsPeculiar );
	TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(testmode), &TESTMODE_PMS_ProcData, param );
	return TRUE;
#endif
}
	
// 簡易会話 一単語
static BOOL TESTMODE_ITEM_SelectPmsSingle( TESTMODE_WORK *work , const int idx )
{
#ifndef TESTMODE_PMS_USE_PROC
  PMS_DATA data;
  PMSI_PARAM* pmsi;

  PMSDAT_Init( &data, 0 );
  pmsi = PMSI_PARAM_Create( PMSI_MODE_SINGLE, PMSI_GUIDANCE_DEFAULT, NULL, FALSE, SaveControl_GetPointer(), GFL_HEAPID_APP );  // dataは送らない

	TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(pmsinput), &ProcData_PMSInput, pmsi );

  return TRUE;
#else

  TESTMODE_PMS_PARAM* param = TESTMODE_PMS_CreateParam( GFL_HEAPID_APP, TESTMODE_ITEM_SelectPmsSingle );
	TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(testmode), &TESTMODE_PMS_ProcData, param );
	return TRUE;
#endif
}

// 簡易会話 二単語
static BOOL TESTMODE_ITEM_SelectPmsDouble( TESTMODE_WORK *work , const int idx )
{
#ifndef TESTMODE_PMS_USE_PROC
  PMS_DATA data;
  PMSI_PARAM* pmsi;

  PMSDAT_Init( &data, 0 );
  //pmsi = PMSI_PARAM_Create( PMSI_MODE_DOUBLE, PMSI_GUIDANCE_DEFAULT, &data, TRUE, SaveControl_GetPointer(), GFL_HEAPID_APP );  // dataを送ってみる
  pmsi = PMSI_PARAM_Create( PMSI_MODE_DOUBLE, PMSI_GUIDANCE_DEFAULT, NULL, TRUE, SaveControl_GetPointer(), GFL_HEAPID_APP );  // 単語モードは文章固定に未対応らしい pmsi_param.c

	TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(pmsinput), &ProcData_PMSInput, pmsi );

  return TRUE;
#else
  TESTMODE_PMS_PARAM* param = TESTMODE_PMS_CreateParam( GFL_HEAPID_APP, TESTMODE_ITEM_SelectPmsDouble );
	TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(testmode), &TESTMODE_PMS_ProcData, param );
	return TRUE;
#endif
}


// マイクテスト
#include "app/mictest.h"
static BOOL TESTMODE_ITEM_SelectMicTest( TESTMODE_WORK *work , const int idx )
{
	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(mictest), &TitleMicTestProcData, NULL );
	return TRUE;
}

