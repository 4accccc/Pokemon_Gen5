//=============================================================================================
/**
 * @file	btlv_core.c
 * @brief	ポケモンWB バトル描画メインモジュール
 * @author	taya
 *
 * @date	2008.10.02	作成
 */
//=============================================================================================
#include <tcbl.h>

#include "print/gf_font.h"
#include "waza_tool/wazano_def.h"		//soga
#include "arc_def.h"
#include "font/font.naix"

#include "battle/btl_common.h"
#include "battle/btl_main.h"
#include "battle/btl_action.h"
#include "battle/btl_calc.h"
#include "battle/btl_util.h"

#include "btlv_scu.h"
#include "btlv_scd.h"
#include "btlv_effect.h"	//soga

#include "btlv_core.h"


/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
	STR_BUFFER_SIZE = 384,
	GENERIC_WORK_SIZE = 128,
};

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BOOL (*pCmdProc)( BTLV_CORE*, int*, void* );

/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/
struct _BTLV_CORE {

	BTL_MAIN_MODULE*					mainModule;
	const BTL_CLIENT*					myClient;
	const BTL_POKE_CONTAINER*	pokeCon;
	u8												myClientID;

	BtlvCmd		processingCmd;
	pCmdProc	mainProc;
	int				mainSeq;
	BTL_PROC	subProc;
	u8				genericWork[ GENERIC_WORK_SIZE ];

	STRBUF*						strBuf;
	GFL_FONT*					fontHandle;
	BTL_ACTION_PARAM*	actionParam;
	u32								procPokeID;

	GFL_TCBLSYS*	tcbl;
	BTLV_SCU*			scrnU;
	BTLV_SCD*			scrnD;

	HEAPID	heapID;
};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void* getGenericWork( BTLV_CORE* core, u32 size );
static BOOL CmdProc_Setup( BTLV_CORE* core, int* seq, void* workBuffer );
static BOOL CmdProc_SelectAction( BTLV_CORE* core, int* seq, void* workBufer );
static BOOL CmdProc_SelectPokemon( BTLV_CORE* core, int* seq, void* workBufer );
static void mainproc_setup( BTLV_CORE* core, pCmdProc proc );
static BOOL mainproc_call( BTLV_CORE* core );
static BOOL subprocDamageEffect( int* seq, void* wk_adrs );
static BOOL subprocDamageDoubleEffect( int* seq, void* wk_adrs );
static BOOL subprocMemberIn( int* seq, void* wk_adrs );
static void setup_core( BTLV_CORE* wk, HEAPID heapID );
static void cleanup_core( BTLV_CORE* wk );



//=============================================================================================
/**
 * 描画メインモジュールの生成
 *
 * @param   mainModule		システムメインモジュールのハンドラ
 * @param   heapID			生成先ヒープID
 *
 * @retval  BTLV_CORE*		生成された描画メインモジュールのハンドラ
 */
//=============================================================================================
BTLV_CORE*  BTLV_Create( BTL_MAIN_MODULE* mainModule, const BTL_CLIENT* client, const BTL_POKE_CONTAINER* pokeCon, HEAPID heapID )
{
	BTLV_CORE* core = GFL_HEAP_AllocMemory( heapID, sizeof(BTLV_CORE) );

	core->mainModule = mainModule;
	core->myClient = client;
	core->pokeCon = pokeCon;
	core->myClientID = BTL_CLIENT_GetClientID( client );
	core->processingCmd = BTLV_CMD_NULL;
	core->heapID = heapID;
	core->strBuf = GFL_STR_CreateBuffer( STR_BUFFER_SIZE, heapID );
	core->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_nftr,
						GFL_FONT_LOADTYPE_FILE, FALSE, heapID );


	core->tcbl = GFL_TCBL_Init( heapID, heapID, 64, 128 );
	core->scrnU = BTLV_SCU_Create( core, core->mainModule, pokeCon, core->tcbl, core->fontHandle, core->myClientID, heapID );
	core->scrnD = BTLV_SCD_Create( core, core->mainModule, pokeCon, core->tcbl, core->fontHandle, core->myClientID, heapID );

	core->mainProc = NULL;
	core->mainSeq = 0;

	BTL_STR_InitSystem( mainModule, client, pokeCon, heapID );
	GFL_UI_TP_Init( heapID );


	return core;
}

//=============================================================================================
/**
 * 描画メインモジュールの破棄
 *
 * @param   core		描画メインモジュール
 *
 */
//=============================================================================================
void BTLV_Delete( BTLV_CORE* core )
{
	GFL_UI_TP_Exit();
	BTL_STR_QuitSystem();

	//エフェクト削除 soga
	BTLV_EFFECT_Exit();

	BTLV_SCD_Delete( core->scrnD );
	BTLV_SCU_Delete( core->scrnU );
	cleanup_core( core );

	GFL_TCBL_Exit( core->tcbl );
	GFL_STR_DeleteBuffer( core->strBuf );
	GFL_FONT_Delete( core->fontHandle );
	GFL_HEAP_FreeMemory( core );
}

//=============================================================================================
/**
 * 描画メインループ
 *
 * @param   core		
 *
 */
//=============================================================================================
void BTLV_CORE_Main( BTLV_CORE* core )
{
	GFL_UI_TP_Main();
	GFL_TCBL_Main( core->tcbl );
	BTLV_EFFECT_Main();		//soga
}


//=============================================================================================
/**
  * 描画コマンド発行
 *
 * @param   core		描画メインモジュールのハンドラ
 * @param   client		関連クライアントモジュールハンドラ
 * @param   cmd			描画コマンド
 *
 */
//=============================================================================================
void BTLV_StartCommand( BTLV_CORE* core, BtlvCmd cmd )
{
	GF_ASSERT( core->processingCmd == BTLV_CMD_NULL );

	{
		static const struct {
			BtlvCmd		cmd;
			pCmdProc	proc;
		}procTbl[] = {
			{ BTLV_CMD_SETUP,						CmdProc_Setup },
			{ BTLV_CMD_SELECT_POKEMON,	CmdProc_SelectPokemon },
		};

		int i;

		for(i=0; i<NELEMS(procTbl); i++)
		{
			if( procTbl[i].cmd == cmd )
			{
				core->processingCmd = cmd;
				core->mainProc = procTbl[i].proc;
				core->mainSeq = 0;
				return;
			}
		}

		GF_ASSERT(0);
	}
}

//=============================================================================================
/**
 * BTLV_StartCommand で受け付けた描画コマンドの終了を待つ
 *
 * @param   core			描画メインモジュールのハンドラ
 *
 * @retval  BOOL		終了していたらTRUE
 */
//=============================================================================================
BOOL BTLV_WaitCommand( BTLV_CORE* core )
{
	if( core->processingCmd != BTLV_CMD_NULL )
	{
		if( core->mainProc(core, &core->mainSeq, core->genericWork) )
		{
			core->processingCmd = BTLV_CMD_NULL;
			core->mainProc = NULL;
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}


static void* getGenericWork( BTLV_CORE* core, u32 size )
{
	GF_ASSERT(size<sizeof(core->genericWork));

	return core->genericWork;
}


static BOOL CmdProc_Setup( BTLV_CORE* core, int* seq, void* workBuffer )
{
	switch( *seq ){
	case 0:
		setup_core( core, core->heapID );
		BTLV_SCU_Setup( core->scrnU );
		BTLV_SCD_Setup( core->scrnD );
		BTLV_EFFECT_Init( 0, core->heapID );		//soga
		(*seq)++;
		break;

	case 1:
		BTLV_SCU_StartBtlIn( core->scrnU );
		(*seq)++;
		break;

	case 2:
		if( BTLV_SCU_WaitBtlIn( core->scrnU ) )
		{
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static BOOL CmdProc_SelectAction( BTLV_CORE* core, int* seq, void* workBufer )
{
	enum {
		SEQ_INIT=0,
		SEQ_SELECT_MAIN,
		SEQ_SEL_FIGHT  = 100,
		SEQ_SEL_ITEM   = 200,
		SEQ_SEL_CHANGE = 300,
		SEQ_SEL_ESCAPE = 400,
	};

	typedef struct {
		u16 idx1;
		u16 idx2;
		u16 maxElems;
		int printArg;
	}SEQ_WORK;

	SEQ_WORK* wk = workBufer;
	BOOL ret = FALSE;

	switch( *seq ){
	case 0:
		wk->printArg = core->procPokeID;
		BTL_STR_MakeStringStd( core->strBuf, BTL_STRID_STD_SelectAction, 1, core->procPokeID );
		BTLV_SCU_StartMsg( core->scrnU, core->strBuf, BTLV_MSGWAIT_NONE );
		(*seq)++;
		break;
	case 1:
		if( BTLV_SCU_WaitMsg(core->scrnU) )
		{
			const BTL_POKEPARAM* bpp = BTL_CLIENT_GetProcPokeData( core->myClient );
			BTLV_SCD_StartActionSelect( core->scrnD, bpp, core->actionParam );
			(*seq)++;
		}
		break;
	case 2:
		if( BTLV_SCD_WaitActionSelect(core->scrnD) )
		{
			return TRUE;
		}
	}
	return FALSE;
}
//--------------------------------------------------------------------------
/**
 * ポケモン選択
 *
 * @param   core		
 * @param   seq			
 * @param   workBufer	
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------------------
static BOOL CmdProc_SelectPokemon( BTLV_CORE* core, int* seq, void* workBufer )
{
	switch( *seq ){
	case 0:
		BTLV_SCD_StartPokemonSelect( core->scrnD );
		(*seq)++;
		break;
	case 1:
		if( BTLV_SCD_WaitPokemonSelect(core->scrnD) )
		{
//			BTLV_SCD_GetSelectAction( core->scrnD, core->actionParam );
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//--------------------------------------------------------------------------
/**
 * メインプロセス設定
 *
 * @param   core		
 * @param   proc		
 *
 */
//--------------------------------------------------------------------------
static void mainproc_setup( BTLV_CORE* core, pCmdProc proc )
{
	GF_ASSERT(core);
	GF_ASSERT_MSG( core->mainProc == NULL, "mainProc=%p\n", core->mainProc );

	core->mainProc = proc;
	core->mainSeq = 0;
}
//--------------------------------------------------------------------------
/**
 * メインプロセスコール
 *
 * @param   core		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------------------
static BOOL mainproc_call( BTLV_CORE* core )
{
	if( core->mainProc != NULL )
	{
		if( core->mainProc(core, &core->mainSeq, core->genericWork) )
		{
			core->mainProc = NULL;
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}


//=============================================================================================
/**
 * アクション選択開始
 *
 * @param   core		
 * @param   dest		[out] 
 *
 */
//=============================================================================================
void BTLV_UI_SelectAction_Start( BTLV_CORE* core, u8 pokeID, BTL_ACTION_PARAM* dest )
{
	core->actionParam = dest;
	core->procPokeID = pokeID;
	mainproc_setup( core, CmdProc_SelectAction );
}
//=============================================================================================
/**
 * アクション選択終了待ち
 *
 * @param   core		
 *
 * @retval  BOOL		終了していたらTRUE
 */
//=============================================================================================
BOOL BTLV_UI_SelectAction_Wait( BTLV_CORE* core )
{
	return mainproc_call( core );
}








void BTLV_StartPokeSelect( BTLV_CORE* core, const BTL_POKESELECT_PARAM* param, BTL_POKESELECT_RESULT* result )
{
	BTLV_SCD_PokeSelect_Start( core->scrnD, param, result );
}

BOOL BTLV_WaitPokeSelect( BTLV_CORE* core )
{
	return BTLV_SCD_PokeSelect_Wait( core->scrnD );
}


//--------------------------------------

//=============================================================================================
/**
 * ワザエフェクト開始
 *
 * @param   core		
 * @param   dst			
 *
 */
//=============================================================================================
void BTLV_ACT_WazaEffect_Start( BTLV_CORE* wk, BtlPokePos atPokePos, BtlPokePos defPokePos, WazaID waza )
{
	BTL_Printf( "WazaEff waza=%d, atPos=%d, defPos=%d\n", waza, atPokePos, defPokePos);

	BTLV_SCU_StartWazaEffect( wk->scrnU, atPokePos, defPokePos, waza );
}
//=============================================================================================
/**
 * ワザエフェクト終了待ち
 *
 * @param   wk		
 *
 * @retval  BOOL		
 */
//=============================================================================================
BOOL BTLV_ACT_WazaEffect_Wait( BTLV_CORE* wk )
{
	return BTLV_SCU_WaitWazaEffect( wk->scrnU );
}

//--------------------------------------

typedef struct {
	BtlTypeAff	affinity;
	u16			damage;
	u16			timer;
	u8			defPokePos;
}WAZA_DMG_ACT_WORK;

//=============================================================================================
/**
 * 単体ワザダメージエフェクト開始
 *
 * @param   wk					
 * @param   defPokePos	
 * @param   damage			
 * @param   aff					
 *
 */
//=============================================================================================
void BTLV_ACT_DamageEffectSingle_Start( BTLV_CORE* wk, BtlPokePos defPokePos, u16 damage, BtlTypeAff aff )
{
	WAZA_DMG_ACT_WORK* subwk = getGenericWork(wk, sizeof(WAZA_DMG_ACT_WORK));

	subwk->affinity = aff;
	subwk->damage = damage;
	subwk->defPokePos = defPokePos;
	subwk->timer = 0;

	BTL_UTIL_SetupProc( &wk->subProc, wk, NULL, subprocDamageEffect );
}
//=============================================================================================
/**
 * 単体ワザダメージエフェクト終了待ち
 *
 * @param   wk		
 *
 * @retval  BOOL		
 */
//=============================================================================================
BOOL BTLV_ACT_DamageEffectSingle_Wait( BTLV_CORE* wk )
{
	return BTL_UTIL_CallProc( &wk->subProc );
}

static BOOL subprocDamageEffect( int* seq, void* wk_adrs )
{
	BTLV_CORE* wk = wk_adrs;
	WAZA_DMG_ACT_WORK* subwk = getGenericWork(wk, sizeof(WAZA_DMG_ACT_WORK));

	switch( *seq ){
	case 0:
		BTLV_SCU_StartWazaDamageAct( wk->scrnU, subwk->defPokePos );


		if( subwk->affinity < BTL_TYPEAFF_100 )
		{
			BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffBad, 0 );
			BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
		}
		else if ( subwk->affinity > BTL_TYPEAFF_100 )
		{
			BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffGood, 0 );
			BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
		}

		(*seq)++;
		break;

	case 1:
		if(	BTLV_SCU_WaitWazaDamageAct(wk->scrnU)
		&&	BTLV_SCU_WaitMsg(wk->scrnU)
		){
			return TRUE;
		}
		break;

	}
	return FALSE;
}
//--------------------------------------

typedef struct {
	BtlTypeAff	affinity;
	u16			damage1;
	u16			damage2;
	u8			defPokePos1;
	u8			defPokePos2;
	u16			timer;
}WAZA_DMG2_ACT_WORK;

//=============================================================================================
/**
 * ２体同時ワザダメージエフェクト開始
 *
 * @param   wk					
 * @param   defPokePos	
 * @param   damage			
 * @param   aff					
 *
 */
//=============================================================================================
void BTLV_ACT_DamageEffectDouble_Start( BTLV_CORE* wk, BtlPokePos defPokePos1, BtlPokePos defPokePos2,
		BtlTypeAff aff )
{
	WAZA_DMG2_ACT_WORK* subwk = getGenericWork(wk, sizeof(WAZA_DMG2_ACT_WORK));

	subwk->affinity = aff;
	subwk->defPokePos1 = defPokePos1;
	subwk->defPokePos2 = defPokePos2;
	subwk->timer = 0;

	BTL_UTIL_SetupProc( &wk->subProc, wk, NULL, subprocDamageDoubleEffect );
}
//=============================================================================================
/**
 * ２体同時ワザダメージエフェクト終了待ち
 *
 * @param   wk		
 *
 * @retval  BOOL		TRUEで終了
 */
//=============================================================================================
BOOL BTLV_ACT_DamageEffectDouble_Wait( BTLV_CORE* wk )
{
	return BTL_UTIL_CallProc( &wk->subProc );
}
static BOOL subprocDamageDoubleEffect( int* seq, void* wk_adrs )
{
	BTLV_CORE* wk = wk_adrs;
	WAZA_DMG2_ACT_WORK* subwk = getGenericWork(wk, sizeof(WAZA_DMG2_ACT_WORK));

	switch( *seq ){
	case 0:
		BTLV_SCU_StartWazaDamageAct( wk->scrnU, subwk->defPokePos1 );
		BTLV_SCU_StartWazaDamageAct( wk->scrnU, subwk->defPokePos2 );

		if( subwk->affinity < BTL_TYPEAFF_100 )
		{
			BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffBad, 0 );
			BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_NONE );
		}
		else if ( subwk->affinity > BTL_TYPEAFF_100 )
		{
			BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffGood, 0 );
			BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_NONE );
		}
		(*seq)++;
		break;

	case 1:
		if(	BTLV_SCU_WaitWazaDamageAct(wk->scrnU)
		&&	BTLV_SCU_WaitMsg(wk->scrnU)
		){
			return TRUE;
		}
		break;

	}
	return FALSE;
}

//--------------------------------------
typedef struct {
	u8  pokePos[ BTL_POS_MAX ];
	u8  pokeCnt;
	u16 seq;
	BtlTypeAffAbout  affAbout;
}DMG_PLURAL_ACT_WORK;


void BTLV_ACT_DamageEffectPlural_Start( BTLV_CORE* wk, u32 pokeCnt, BtlTypeAffAbout affAbout, const u8* pokeID )
{
	DMG_PLURAL_ACT_WORK* subwk = getGenericWork(wk, sizeof(DMG_PLURAL_ACT_WORK));

	subwk->affAbout = affAbout;
	subwk->pokeCnt = pokeCnt;
	BTL_Printf("複数体ダメージ処理 (%d体)\n", pokeCnt);
	subwk->seq = 0;
	{
		u32 i;
		for(i=0; i<pokeCnt; ++i)
		{
			BTL_Printf("  対象ポケID=%d\n", pokeID[i]);
			subwk->pokePos[i] = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, pokeID[i] );
		}
	}
}
BOOL BTLV_ACT_DamageEffectPlural_Wait( BTLV_CORE* wk )
{
	DMG_PLURAL_ACT_WORK* subwk = getGenericWork(wk, sizeof(DMG_PLURAL_ACT_WORK));

	switch( subwk->seq ){
	case 0:
		{
			u32 i;
			for(i=0; i<subwk->pokeCnt; ++i)
			{
				BTLV_SCU_StartWazaDamageAct( wk->scrnU, subwk->pokePos[i] );
			}
			switch( subwk->affAbout ){
			case BTL_TYPEAFF_ABOUT_ADVANTAGE:
				BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffGood, 0 );
				BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_NONE );
				break;
			case BTL_TYPEAFF_ABOUT_DISADVANTAGE:
				BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffBad, 0 );
				BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_NONE );
				break;
			}
			subwk->seq++;
		}
		break;
	case 1:
		if(	BTLV_SCU_WaitWazaDamageAct(wk->scrnU)
		&&	BTLV_SCU_WaitMsg(wk->scrnU)
		){
			return TRUE;
		}
		break;
	}
	return FALSE;

}

//=============================================================================================
/**
 * シンプルHPエフェクト開始
 * ※HPメーターのみ変動。状態異常や天候ダメージに使う。変動後の値は、直接ポケモンデータを参照している。
 *
 * @param   wk				
 * @param   pokePos		
 *
 */
//=============================================================================================
void BTLV_ACT_SimpleHPEffect_Start( BTLV_CORE* wk, BtlPokePos pokePos )
{
	BTLV_SCU_StartHPGauge( wk->scrnU, pokePos );
}
BOOL BTLV_ACT_SimpleHPEffect_Wait( BTLV_CORE* wk )
{
	return BTLV_SCU_WaitHPGauge( wk->scrnU );
}


//=============================================================================================
/**
 * ポケモンひんしアクション開始
 *
 * @param   wk		
 * @param   pos		ひんしになったポケモンの位置ID
 *
 */
//=============================================================================================
void BTLV_StartDeadAct( BTLV_CORE* wk, BtlPokePos pos )
{
	BTLV_SCU_StartDeadAct( wk->scrnU, pos );
}
BOOL BTLV_WaitDeadAct( BTLV_CORE* wk )
{
	return BTLV_SCU_WaitDeadAct( wk->scrnU );
}




//=============================================================================================
/**
 * ポケモン退出アクション開始
 *
 * @param   wk		
 * @param   clientID		
 * @param   memberIdx		
 *
 */
//=============================================================================================
void BTLV_ACT_MemberOut_Start( BTLV_CORE* wk, u8 clientID, u8 memberIdx )
{
	BtlPokePos pos = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, memberIdx );

	BTLV_SCU_StartMemberOutAct( wk->scrnU, clientID, memberIdx, pos );
}
BOOL BTLV_ACT_MemberOut_Wait( BTLV_CORE* wk )
{
	return BTLV_SCU_WaitMemberOutAct( wk->scrnU );
}


//------------------------------------------
typedef struct {
	BtlPokePos  pokePos;
	u8 clientID;
	u8 memberIdx;
	u8 pokeID;
}MEMBER_IN_WORK;

//=============================================================================================
/**
 * ポケモン入れ替えアクション開始
 *
 * @param   wk		
 * @param   clientID		
 * @param   memberIdx		
 *
 */
//=============================================================================================
void BTLV_StartMemberChangeAct( BTLV_CORE* wk, BtlPokePos pos, u8 clientID, u8 memberIdx )
{
	MEMBER_IN_WORK* subwk = getGenericWork( wk, sizeof(MEMBER_IN_WORK) );
	const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, pos );

	subwk->clientID = clientID;
	subwk->memberIdx = memberIdx;
	subwk->pokePos = pos;
	subwk->pokeID = BTL_POKEPARAM_GetID( bpp );

	BTL_Printf("メンバー入場 client=%d, pokePos=%d, memberIdx=%d, pokeID=%d\n",
		clientID, pos, memberIdx, subwk->pokeID );

	BTL_UTIL_SetupProc( &wk->subProc, wk, NULL, subprocMemberIn );
}

BOOL BTLV_WaitMemberChangeAct( BTLV_CORE* wk )
{
	return BTL_UTIL_CallProc( &wk->subProc );
}

static BOOL subprocMemberIn( int* seq, void* wk_adrs )
{
	BTLV_CORE* wk = wk_adrs;
	MEMBER_IN_WORK* subwk = getGenericWork( wk, sizeof(MEMBER_IN_WORK) );

	switch( *seq ){
	case 0:
		{
			u16 strID = BTL_MAIN_IsOpponentClientID(wk->mainModule, wk->myClientID, subwk->clientID)?
					BTL_STRID_STD_PutSingle_Enemy : BTL_STRID_STD_PutSingle;

			BTL_STR_MakeStringStd( wk->strBuf, strID, 1, subwk->pokeID );
			BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_NONE );
			(*seq)++;
		}
		break;

	case 1:
		if( BTLV_SCU_WaitMsg(wk->scrnU) )
		{
			BTLV_SCU_StartPokeIn( wk->scrnU, subwk->pokePos, subwk->clientID, subwk->memberIdx );
			(*seq)++;
		}
		break;
	case 2:
		if( BTLV_SCU_WaitPokeIn(wk->scrnU) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//=============================================================================================
/**
 * メッセージ表示開始
 *
 * @param   wk		
 * @param   strID		
 *
 */
//=============================================================================================
void BTLV_StartMsgStd( BTLV_CORE* wk, u16 strID, const int* args )
{
	BTL_STR_MakeStringStdWithArgArray( wk->strBuf, strID, args );
	BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
//	printf( wk->strBuf );
}

//=============================================================================================
/**
 * メッセージ表示開始
 *
 * @param   wk		
 * @param   strID		
 *
 */
//=============================================================================================
void BTLV_StartMsgSet( BTLV_CORE* wk, u16 strID, const int* args )
{
	BTL_STR_MakeStringSet( wk->strBuf, strID, args );
	BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
//	printf( wk->strBuf );
}

void BTLV_StartMsgWaza( BTLV_CORE* wk, u8 pokeID, u16 waza )
{
	BTL_STR_MakeStringWaza( wk->strBuf, pokeID, waza );
	BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
}

BOOL BTLV_WaitMsg( BTLV_CORE* wk )
{
	return BTLV_SCU_WaitMsg( wk->scrnU );
}



//=============================================================================================
/**
 * とくせいウィンドウ表示開始
 *
 * @param   wk		
 * @param   pos		
 *
 */
//=============================================================================================
void BTLV_StartTokWin( BTLV_CORE* wk, BtlPokePos pos )
{
	BTLV_SCU_DispTokWin( wk->scrnU, pos );
}
BOOL BTLV_StartTokWinWait( BTLV_CORE* wk, BtlPokePos pos )
{
	// @@@ 今はすぐ終わってる
	return TRUE;
}

void BTLV_QuitTokWin( BTLV_CORE* wk, BtlPokePos pos )
{
	BTLV_SCU_HideTokWin( wk->scrnU, pos );
}
BOOL BTLV_QuitTokWinWait( BTLV_CORE* wk, BtlPokePos pos )
{
	// @@@ 今はすぐ終わってる
	return TRUE;
}

//=============================================================================================
/**
 * とくせいウィンドウの内容更新（開始）
 *
 * @param   wk		
 * @param   pos		
 *
 */
//=============================================================================================
void BTLV_TokWin_Renew_Start( BTLV_CORE* wk, BtlPokePos pos )
{
	BTLV_SCU_TokWin_Renew_Start( wk->scrnU, pos );
}
//=============================================================================================
/**
 * とくせいウィンドウの内容更新（終了待ち）
 *
 * @param   wk		
 * @param   pos		
 *
 * @retval  BOOL		
 */
//=============================================================================================
BOOL BTLV_TokWin_Renew_Wait( BTLV_CORE* wk, BtlPokePos pos )
{
	return BTLV_SCU_TokWin_Renew_Wait( wk->scrnU, pos );
}



void BTLV_StartRankDownEffect( BTLV_CORE* wk, u8 pokeID, BppValueID statusType )
{
//	printf("↓↓↓ひゅるる…\n");
}


//=============================================================================================
/**
 * 「つうしんたいきちゅう…」メッセージを表示
 *
 * @param   wk		
 *
 */
//=============================================================================================
void BTLV_StartCommWait( BTLV_CORE* wk )
{
	BTLV_SCD_StartCommWaitInfo( wk->scrnD );
}
BOOL BTLV_WaitCommWait( BTLV_CORE* wk )
{
	return BTLV_SCD_WaitCommWaitInfo( wk->scrnD );
}
void BTLV_ResetCommWaitInfo( BTLV_CORE* wk )
{
	BTLV_SCD_ClearCommWaitInfo( wk->scrnD );
}


//=============================================================================================
/**
 * きのみ食べアクション開始
 *
 * @param   wk		
 * @param   pos		きのみを食べるポケモンの位置
 */
//=============================================================================================
void BTLV_KinomiAct_Start( BTLV_CORE* wk, BtlPokePos pos )
{
	BTLV_SCU_KinomiAct_Start( wk->scrnU, pos );
}
//=============================================================================================
/**
 * きのみ食べアクション終了待ち
 *
 * @param   wk		
 * @param   pos		きのみを食べるポケモンの位置
 */
//=============================================================================================
BOOL BTLV_KinomiAct_Wait( BTLV_CORE* wk, BtlPokePos pos )
{
	return BTLV_SCU_KinomiAct_Wait( wk->scrnU, pos );
}

//-------------------------------------------

static void setup_core( BTLV_CORE* wk, HEAPID heapID )
{
//soga
#if 0
	static const GFL_DISP_VRAM vramBank = {
		GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
	};
#else
	static const GFL_DISP_VRAM vramBank = {
		GX_VRAM_BG_128_D,						// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,				// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,	// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_64_E,						// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,				// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_01_AB,					// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_64K,	// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,	// サブOBJマッピングモード
	};
#endif

	// BGsystem初期化
	GFL_BG_Init( heapID );
	GFL_BMPWIN_Init( heapID );
	GFL_FONTSYS_Init();

	// VRAMバンク設定
	GFL_DISP_SetBank( &vramBank );

	// 各種効果レジスタ初期化
	G2_BlendNone();
	G2S_BlendNone();

	// 上下画面設定
	GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

	//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}

	//3D関連初期化 soga
	{
		GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, heapID, NULL );
		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
		G3X_AlphaBlend( TRUE );
		G3X_EdgeMarking( TRUE );
		G3X_AntiAlias( FALSE );
		G2_SetBG0Priority( 1 );
	}
	//ウインドマスク設定（画面両端のエッジマーキングのゴミを消す）soga
	{
		G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0 |
							   GX_WND_PLANEMASK_BG1 |
							   GX_WND_PLANEMASK_BG2 |
							   GX_WND_PLANEMASK_BG3 |
							   GX_WND_PLANEMASK_OBJ,
							   FALSE );
		G2_SetWndOutsidePlane( GX_WND_PLANEMASK_NONE, FALSE );
		G2_SetWnd0Position( 1, 0, 255, 192 );
		GX_SetVisibleWnd( GX_WNDMASK_W0 );
	}
}

static void cleanup_core( BTLV_CORE* wk )
{
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();

	//3D関連削除 soga
	GFL_G3D_Exit();
}



u8 BTLV_CORE_GetPlayerClientID( const BTLV_CORE* core )
{
	return core->myClientID;
}
