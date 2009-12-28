//============================================================================================
/**
 * @file		box2_seq.c
 * @brief		ボックス画面 メインシーケンス処理
 * @author	Hiroyuki Nakamura
 * @date		09.10.05
 *
 *	モジュール名：BOX2SEQ
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/main.h"
#include "system/wipe.h"
#include "system/gfl_use.h"
#include "item/item.h"
#include "poke_tool/monsno_def.h"

#include "msg/msg_boxmenu.h"

#include "box2_main.h"
#include "box2_bmp.h"
#include "box2_obj.h"
#include "box2_seq.h"
#include "box2_ui.h"
#include "box2_bgwfrm.h"


//============================================================================================
//	定数定義
//============================================================================================

// サブプロセスタイプ
enum {
	SUB_PROC_TYPE_POKESTATUS = 0,		// ステータス画面
	SUB_PROC_TYPE_BAG,							// バッグ画面
	SUB_PROC_TYPE_NAMEIN,						// 文字入力
};

// サブプロセス呼び出しモード
enum {
	SUB_PROC_MODE_MENU_BOX = 0,		// ボックスのポケモンのメニューから
	SUB_PROC_MODE_MENU_PARTY,			// 手持ちのポケモンのメニューから

	SUB_PROC_MODE_TB_BOX,					// ボックスのポケモンのタッチバーから
	SUB_PROC_MODE_TB_BOX_GET,			// ボックスのポケモンのタッチバーから（掴んでいる時）

	SUB_PROC_MODE_TB_PARTY,				// 手持ちのポケモンのタッチバーから
	SUB_PROC_MODE_TB_PARTY_GET,		// 手持ちのポケモンのタッチバーから（掴んでいる時）
};

// サブプロセス設定関数
typedef struct {
	pBOX2_FUNC	call;		// 呼び出し
	pBOX2_FUNC	exit;		// 終了
	u32	rcvSeq;					// 復帰シーケンス
}SUB_PROC_DATA;

// はい・いいえ処理
typedef struct {
	pBOX2_FUNC	yes;
	pBOX2_FUNC	no;
}YESNO_DATA;

// はい・いいえID
enum {
	YESNO_ID_ITEMGET = 0,			// 道具預かり
	YESNO_ID_POKEFREE,				// 逃がす
	YESNO_ID_ITEM_RET_BAG,		// アイテムしまう
	YESNO_ID_BOX_END,					// ボックス終了（ボタン）
	YESNO_ID_BOX_END_CANCEL,	// ボックス終了（キャンセル）
	YESNO_ID_SLEEP,						// 寝かせる
};

// トレイアイコンスクロールワーク
typedef struct {
	s16	cnt;
	s16	mv;
}SEQWK_TRAYICON_SCROLL;

//「ポケモンを預ける」配置シーケンス
enum {
	SEQ_PTOUT_PUT_POKEMOVE = 0,		// アイコン移動設定
	SEQ_PTOUT_PUT_PTFRM_RET,			// 手持ちフレームイン（キャンセル時）
	SEQ_PTOUT_PUT_ICON_RET,				// アイコン配置（キャンセル時）

	SEQ_PTOUT_PUT_PTFRM_IN,				// 手持ちフレームイン
	SEQ_PTOUT_PUT_ICON_PUT,				// アイコン配置
	SEQ_PTOUT_PUT_DATA_CHG,				// データ入れ替え

	SEQ_PTOUT_PUT_END,						// 終了
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( BOX2_SYS_WORK * syswk );
static int MainSeq_Release( BOX2_SYS_WORK * syswk );
static int MainSeq_Wipe( BOX2_SYS_WORK * syswk );
static int MainSeq_PaletteFade( BOX2_SYS_WORK * syswk );
static int MainSeq_Wait( BOX2_SYS_WORK * syswk );
static int MainSeq_VFunc( BOX2_SYS_WORK * syswk );
static int MainSeq_TrgWait( BOX2_SYS_WORK * syswk );
static int MainSeq_YesNo( BOX2_SYS_WORK * syswk );
static int MainSeq_ButtonAnm( BOX2_SYS_WORK * syswk );
static int MainSeq_SubProcCall( BOX2_SYS_WORK * syswk );
static int MainSeq_SubProcMain( BOX2_SYS_WORK * syswk );
static int MainSeq_Start( BOX2_SYS_WORK * syswk );
static int MainSeq_CursorMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeMenuRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_TrayScrollTouch( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxEndDirect( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeTrayPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMenuClose( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeGetInit( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeGetMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeGetExit( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeGetDataChange( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeGetDataChangeErr( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeGetPartyChange( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyInit( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyMenuClose( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyPokeGetInit( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyPokeGetMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyPokeGetExit( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyPokeGetBoxListChange( BOX2_SYS_WORK * syswk );
static int MainSeq_BattleBoxMain( BOX2_SYS_WORK * syswk );
static int MainSeq_BattleBoxTrayPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_BattleBoxMenuClose( BOX2_SYS_WORK * syswk );
static int MainSeq_BattleBoxPartyInit( BOX2_SYS_WORK * syswk );
static int MainSeq_BattleBoxPartyMain( BOX2_SYS_WORK * syswk );
static int MainSeq_BattleBoxPartyEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_BattleBoxPartyMenuClose( BOX2_SYS_WORK * syswk );
static int MainSeq_BattleBoxPartyPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInMain( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInMenuClose( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInSelect( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInAction( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutMain( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutMenuClose( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutErrRcvMain( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutPutInit( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutPutMain( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutPutEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutErrRcvPut( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemMenuClose( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemBagInAnm( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeIconMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemGetInit( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemGetMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemGetEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemBoxPartyIconMovePokeAdd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemBoxPartyIconMoveEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemBoxPartyChgError( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemKeyGetChangeCkack( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemPartyMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemPartyMenuClose( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemPartyIconMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemPartyIconMovePokeAdd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemPartyIconMoveChange( BOX2_SYS_WORK * syswk );
static int MainSeq_SleepMain( BOX2_SYS_WORK * syswk );
static int MainSeq_SleepPokeSelect( BOX2_SYS_WORK * syswk );
static int MainSeq_SleepMenuSet( BOX2_SYS_WORK * syswk );
static int MainSeq_SleepMenuClose( BOX2_SYS_WORK * syswk );
static int MainSeq_StatusCall( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemMenuCheck( BOX2_SYS_WORK * syswk );
static int MainSeq_StatusRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_StrInRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_BagRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemSetMsg( BOX2_SYS_WORK * syswk );
static int MainSeq_MarkingInit( BOX2_SYS_WORK * syswk );
static int MainSeq_MarkingMain( BOX2_SYS_WORK * syswk );
static int MainSeq_MarkingEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_MarkingCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_PokeFreeInit( BOX2_SYS_WORK * syswk );
static int MainSeq_PokeFreeAnm( BOX2_SYS_WORK * syswk );
static int MainSeq_PokeFreeEnter( BOX2_SYS_WORK * syswk );
static int MainSeq_PokeFreeError( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaInit( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaMain( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaExit( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaJumpInit( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaJumpMain( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaJumpExit( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaJumpRet( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaWallPaperInit( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaWallPaperMain( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaWallPaperChange( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaNameInCall( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxEndInit( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxEndCancelInit( BOX2_SYS_WORK * syswk );

static int ChangeSequence( BOX2_SYS_WORK * syswk, int next );
static int FadeInSet( BOX2_SYS_WORK * syswk, int next );
static int FadeOutSet( BOX2_SYS_WORK * syswk, int next );
static int WaitSet( BOX2_SYS_WORK * syswk, int next, int wait );
static int VFuncSet( BOX2_SYS_WORK * syswk, void * func, int next );
static int VFuncReqSet( BOX2_SYS_WORK * syswk, int next );
static int SetCursorMoveRet( BOX2_SYS_WORK * syswk, int next );

static int YesNoSet( BOX2_SYS_WORK * syswk, u32 id );
static int BoxArg_ItemGetYes( BOX2_SYS_WORK * syswk );
static int BoxArg_PokeFreeYes( BOX2_SYS_WORK * syswk );
static int BoxArg_PokeFreeNo( BOX2_SYS_WORK * syswk );
static int BoxItemArg_RetBagYes( BOX2_SYS_WORK * syswk );
static int BoxArg_BoxEndYes( BOX2_SYS_WORK * syswk );
static int BoxArg_BoxEndNo( BOX2_SYS_WORK * syswk );
static int BoxArg_SleepYes( BOX2_SYS_WORK * syswk );
static int BoxArg_SleepNo( BOX2_SYS_WORK * syswk );

static int BgButtonAnmSet( BOX2_SYS_WORK * syswk, u32 wfrmID, int next );
static int MarkingButtonAnmSet( BOX2_SYS_WORK * syswk, u32 no, int next );
static int ObjButtonAnmSet( BOX2_SYS_WORK * syswk, u32 objID, u32 anm, int next );

static int SubProcSet( BOX2_SYS_WORK * syswk, u8 type );
static int RcvBag_ModeSeiri( BOX2_SYS_WORK * syswk );
static int RcvBag_ModeItem( BOX2_SYS_WORK * syswk );
static int RcvBag_ModeBattleBox( BOX2_SYS_WORK * syswk );
static int RcvStatus_ModeTureteiku( BOX2_SYS_WORK * syswk );
static int RcvStatus_ModeAzukeru( BOX2_SYS_WORK * syswk );
static int RcvStatus_ModeSeiri( BOX2_SYS_WORK * syswk );
static int RcvStatus_ModeBattleBox( BOX2_SYS_WORK * syswk );

static int TrayLeftScrollSet( BOX2_SYS_WORK * syswk, int next );
static int TrayRightScrollSet( BOX2_SYS_WORK * syswk, int next );
static int TrayPokeGet( BOX2_SYS_WORK * syswk, u32 pos );

static int GetBattleBoxTrayPoke( BOX2_SYS_WORK * syswk, u32 pos );
static int TrayPokeGetPartyIn( BOX2_SYS_WORK * syswk, u32 pos );
static int TrayPokeGetKey( BOX2_SYS_WORK * syswk, u32 pos, int ret );
static int PartyPokeGet( BOX2_SYS_WORK * syswk, u32 pos );
static int GetBattleBoxPartyPoke( BOX2_SYS_WORK * syswk, u32 pos );
static int PartyPokeGetDrop( BOX2_SYS_WORK * syswk, u32 pos );
static int PartyPokeGetKey( BOX2_SYS_WORK * syswk, u32 pos, int ret );
static int BoxPartyPokeGet( BOX2_SYS_WORK * syswk, u32 pos, int next );
static int PartyPokeMoveGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static int PokeMoveGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static int PokeMovePutKey( BOX2_SYS_WORK * syswk, u32 pos, int next );
static void PokeMoveWorkAlloc( BOX2_SYS_WORK * syswk );
static void PokeMoveWorkFree( BOX2_SYS_WORK * syswk );

static int TrayItemGet( BOX2_SYS_WORK * syswk, u32 pos );
static int TrayItemGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static int BoxPartyItemGet( BOX2_SYS_WORK * syswk, u32 pos );
static int BoxPartyItemGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static int BoxPartyItemPutKeyCancel( BOX2_SYS_WORK * syswk );
static int BoxPartyItemPutKey( BOX2_SYS_WORK * syswk, u32 pos );
static int PartyItemGet( BOX2_SYS_WORK * syswk, u32 pos );
static int PartyItemGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static void ItemMoveWorkAlloc( BOX2_SYS_WORK * syswk );
static void ItemMoveWorkFree( BOX2_SYS_WORK * syswk );

static void PartyFrmSet( BOX2_SYS_WORK * syswk );
static void PartyFrmSetRight( BOX2_SYS_WORK * syswk );
static void PartyFrmSet_PartyOut( BOX2_SYS_WORK * syswk );
static int BoxMoveTrayScrollSet( BOX2_SYS_WORK * syswk, u32 mode );
static void CreateTrayIconScrollWork( BOX2_SYS_WORK * syswk );
static int BoxMoveFrmInSet( BOX2_SYS_WORK * syswk, int next );
static int BoxMoveFrmOutSet( BOX2_SYS_WORK * syswk, int next );
static int PokeMenuOutSet( BOX2_SYS_WORK * syswk, int next );
void BOX2SEQ_ItemModeMenuSet( BOX2_SYS_WORK * syswk, s16 item );

static int PartyOutCheck( BOX2_SYS_WORK * syswk );
static void MarkingSwitch( BOX2_SYS_WORK * syswk, u32 pos );
static int SetWallPaperChange( BOX2_SYS_WORK * syswk, u32 pos );
static void BoxEndPassiveSet( BOX2_SYS_WORK * syswk );


//============================================================================================
//	グローバル変数
//============================================================================================

// メインシーケンステーブル
static const pBOX2_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Wipe,
	MainSeq_PaletteFade,
	MainSeq_Wait,
	MainSeq_VFunc,

	MainSeq_TrgWait,

	MainSeq_YesNo,
	MainSeq_ButtonAnm,

	MainSeq_SubProcCall,
	MainSeq_SubProcMain,

	MainSeq_Start,
	MainSeq_CursorMoveRet,
	MainSeq_ArrangePokeMenuRcv,
	MainSeq_TrayScrollTouch,
	MainSeq_BoxEndDirect,

	// ボックス整理
	MainSeq_ArrangeMain,
	MainSeq_ArrangeTrayPokeChgEnd,
	MainSeq_ArrangeMenuClose,

	MainSeq_ArrangePokeGetInit,
	MainSeq_ArrangePokeGetMain,
	MainSeq_ArrangePokeGetExit,
	MainSeq_ArrangePokeGetDataChange,
	MainSeq_ArrangePokeGetDataChangeErr,
	MainSeq_ArrangePokeGetPartyChange,

	MainSeq_ArrangePartyInit,
	MainSeq_ArrangePartyMain,
	MainSeq_ArrangePartyEnd,
	MainSeq_ArrangePartyMenuClose,
	MainSeq_ArrangePartyPokeChgEnd,

	MainSeq_ArrangePartyPokeGetInit,
	MainSeq_ArrangePartyPokeGetMain,
	MainSeq_ArrangePartyPokeGetExit,
	MainSeq_ArrangePartyPokeGetBoxListChange,

	// バトルボックス
	MainSeq_BattleBoxMain,
	MainSeq_BattleBoxTrayPokeChgEnd,
	MainSeq_BattleBoxMenuClose,

	MainSeq_BattleBoxPartyInit,
	MainSeq_BattleBoxPartyMain,
	MainSeq_BattleBoxPartyEnd,
	MainSeq_BattleBoxPartyMenuClose,
	MainSeq_BattleBoxPartyPokeChgEnd,

	// つれていく
	MainSeq_PartyInMain,
	MainSeq_PartyInMenuClose,
	MainSeq_PartyInSelect,
	MainSeq_PartyInAction,
	MainSeq_PartyInPokeChgEnd,

	// あずける
	MainSeq_PartyOutMain,
	MainSeq_PartyOutMenuClose,
	MainSeq_PartyOutErrRcvMain,

	MainSeq_PartyOutPutInit,
	MainSeq_PartyOutPutMain,
	MainSeq_PartyOutPutEnd,
	MainSeq_PartyOutErrRcvPut,
	MainSeq_PartyOutPokeChgEnd,

	// 持ち物整理
	MainSeq_ItemMain,
	MainSeq_ItemMenuClose,
	MainSeq_ItemBagInAnm,
	MainSeq_ItemArrangeIconMoveRet,
	MainSeq_ItemGetInit,
	MainSeq_ItemGetMain,
	MainSeq_ItemGetEnd,
	MainSeq_ItemBoxPartyIconMovePokeAdd,
	MainSeq_ItemBoxPartyIconMoveEnd,
	MainSeq_ItemBoxPartyChgError,
	MainSeq_ItemKeyGetChangeCkack,
	MainSeq_ItemPartyMain,
	MainSeq_ItemPartyMenuClose,
	MainSeq_ItemPartyIconMoveRet,
	MainSeq_ItemPartyIconMovePokeAdd,
	MainSeq_ItemPartyIconMoveChange,

	// 寝かせる
	MainSeq_SleepMain,
	MainSeq_SleepPokeSelect,
	MainSeq_SleepMenuSet,
	MainSeq_SleepMenuClose,

	// サブプロセス
	MainSeq_StatusCall,
	MainSeq_ItemMenuCheck,

	MainSeq_StatusRcv,
	MainSeq_StrInRcv,
	MainSeq_BagRcv,
	MainSeq_ItemSetMsg,

	// マーキング
	MainSeq_MarkingInit,
	MainSeq_MarkingMain,
	MainSeq_MarkingEnd,
	MainSeq_MarkingCurMoveRet,
	// 逃がす
	MainSeq_PokeFreeInit,
	MainSeq_PokeFreeAnm,
	MainSeq_PokeFreeEnter,
	MainSeq_PokeFreeError,

	// ボックステーマ変更
	MainSeq_BoxThemaInit,
	MainSeq_BoxThemaMain,
	MainSeq_BoxThemaExit,
	MainSeq_BoxThemaJumpInit,
	MainSeq_BoxThemaJumpMain,
	MainSeq_BoxThemaJumpExit,
	MainSeq_BoxThemaJumpRet,
	MainSeq_BoxThemaWallPaperInit,
	MainSeq_BoxThemaWallPaperMain,
	MainSeq_BoxThemaWallPaperChange,
	MainSeq_BoxThemaNameInCall,

	// 終了処理
	MainSeq_BoxEndInit,
	MainSeq_BoxEndCancelInit,

//	MainSeq_PartyInMainRcv,
//	MainSeq_PartyInPartyFrmIn,
//	MainSeq_PartyInPokeMoveSet,
//	MainSeq_PartyInPartyFrmOut,
//	MainSeq_PartyInRetFrmIn,
//	MainSeq_PartyInMainCurMoveRet,
//	MainSeq_PartyInMenuCloseEnd,
//	MainSeq_PartyInPokeMenuRcv,
/*
	MainSeq_PartyOutStartCheck,
	MainSeq_PartyOutStartError,
	MainSeq_PartyOutStartRetFrmIn,
	MainSeq_PartyOutMenuFrmIn,
	MainSeq_PartyOutBoxSelectStart,
	MainSeq_PartyOutBoxSelectMain,
	MainSeq_PartyOutBoxMenuFrmOut,
	MainSeq_PartyOutEndRetFrmOut,
	MainSeq_PartyOutBoxMoveFrmOut,
	MainSeq_PartyOutEndRetFrmIn,
	MainSeq_PartyOutEnterButton,
	MainSeq_PartyOutCheck,
	MainSeq_PartyOutEnterPartyFrmRight,
	MainSeq_PartyOutEnterTrayScroll,
	MainSeq_PartyOutEnterPokeMove,
	MainSeq_PartyOutEnterPartyFrmLeft,
	MainSeq_PartyOutEnterRetIn,
	MainSeq_PartyOutComp,
*/
/*
	MainSeq_PartyOutMainCurMoveRet,
	MainSeq_PartyOutMenuCloseEnd,
	MainSeq_PartyOutBoxSelCurMoveRet,
//	MainSeq_PartyOutPokeMenuRcv,
*/
/*
	MainSeq_ItemArrangeMainCurMoveRet,
	MainSeq_ItemArrangeRetBagSet,
	MainSeq_ItemArrangeRetBagCheck,

	MainSeq_ItemArrangeRetBagCancel,
	MainSeq_ItemArrangeRetBagEnd,
	MainSeq_ItemArrangeGetBagCheck,
	MainSeq_ItemArrangeItemSetMsg,

	MainSeq_ItemArrangeIconMovePokeAdd,
	MainSeq_ItemArrangeIconMoveChange,
	MainSeq_ItemArrangeIconMoveChangeEnd,

	MainSeq_ItemArrangeBoxPartyChgMainCurMoveRet,
//	MainSeq_ItemArrangeBoxPartyIconMoveSubFrmOut,

	MainSeq_ItemArrangeKeyGetMoveMain,
	MainSeq_ItemArrangeKeyGetMoveMainCurMoveRet,

	MainSeq_ItemArrangePartyMainCurMoveRet,

	MainSeq_ItemArrangePartyIconMovePokeAddEnd,

	MainSeq_ItemArrangePartyIconMoveChangeEnd,
	MainSeq_ItemArrangePartyIconMoveErr,
*/

//	MainSeq_ArrangePartyPokeFrmIn,
//	MainSeq_ArrangePartyStart,
//	MainSeq_ArrangePartyPokeFrmOut,
//	MainSeq_ArrangeBoxPartyButtonIn,
//	MainSeq_ArrangeBoxPartyChgStart,
//	MainSeq_ArrangeBoxPartyChgMain,
//	MainSeq_ArrangeBoxPartyPokeChgEnd,

//	MainSeq_ArrangeBoxPartyChgError,
//	MainSeq_ArrangePartyFrmMoveREnd,
//	MainSeq_ArrangePartyFrmMoveLEnd,
//	MainSeq_ArrangeBoxMoveFrmIn,
//	MainSeq_ArrangeExitButtonFrmIn,
//	MainSeq_ArrangeBoxMoveFrmOutSet,
//	MainSeq_ArrangeBoxMoveFrmOut,
//	MainSeq_ArrangeBoxRetButtonIn,
//	MainSeq_ArrangeBoxThemaButtonOut,
//	MainSeq_ArrangeBoxThemaTrayIn,
//	MainSeq_ArrangeBoxThemaExitIn,
//	MainSeq_ArrangeBoxThemaMenuIn,
//	MainSeq_ArrangeBoxThemaMsgSet,
//	MainSeq_ArrangeBoxThemaExitOut,
//	MainSeq_ArrangeBoxThemaMenuOut,
//	MainSeq_ArrangeBoxThemaTrayOut,
//	MainSeq_ArrangeBoxThemaButtonIn,
//	MainSeq_ArrangeBoxThemaChgMain,
//	MainSeq_BoxThemaChgCurMoveRet,
/*
	MainSeq_ArrangeWallPaperFrmIn,
	MainSeq_ArrangeWallPaperMenuIn,
	MainSeq_ArrangeWallPaperChgStart,
	MainSeq_ArrangeWallPaperChgMain,
	MainSeq_ArrangeWallPaperMenuOut,
	MainSeq_ArrangeWallPaperFrmOut,
	MainSeq_ArrangeWallPaperChgEnd,
	MainSeq_ArrangeWallPaperChange,
	MainSeq_ArrangeWallPaperWhiteOut,
	MainSeq_ArrangeWallPaperWhiteIn,
	MainSeq_WallPaperChgCurMoveRet,
*/

//	MainSeq_ArrangeBoxPartyChgMainCurMoveRet,
//	MainSeq_ArrangePartyMainCurMoveRet,
//	MainSeq_BoxArrangePokeMenuRcv,
//	MainSeq_BoxArrangePartyMenuRcv,
//	MainSeq_ArrangeItemSetEggError,
//	MainSeq_ArrangeItemGetMailError,

//	MainSeq_KeyGetPokeMoveMainCurMoveRet,

//	MainSeq_ArrangePartyButton,
//	MainSeq_ArrangeMoveButton,
//	MainSeq_MarkingButton,
//	MainSeq_MarkingEndButton,
//	MainSeq_FreeButton,

//	MainSeq_PartyChgButton,
//	MainSeq_ArrangePartyCloseButton,
//	MainSeq_PartyEndButton,
//	MainSeq_ArrangeBoxMoveButton,
//	MainSeq_BoxMoveEndButton,
//	MainSeq_BoxThemaTrayChgButton,
//	MainSeq_BoxThemaWallPaperButton,
//	MainSeq_ArrangeWallPaperChgButton,
//	MainSeq_ArrangeWallPaperChgCancelButton,
//	MainSeq_PartyInCloseButton,
//	MainSeq_PartyOutButton,
//	MainSeq_PartyOutCloseButton,
//	MainSeq_ItemArrangePartyButton,
//	MainSeq_ItemArrangeMoveButton,
//	MainSeq_ItemArrangeBagButton,
//	MainSeq_ItemArrangeCloseButton,
//	MainSeq_ItemArrangeBoxMoveButton,
//	MainSeq_ItemChgButton,
//	MainSeq_ItemArrangePartyCloseButton,
};

// サブプロセス設定
static const SUB_PROC_DATA SubProcFunc[] =
{
	// ステータス画面
	{ BOX2MAIN_PokeStatusCall, BOX2MAIN_PokeStatusExit, BOX2SEQ_MAINSEQ_STATUS_RCV },
	// バッグ画面
	{ BOX2MAIN_BagCall, BOX2MAIN_BagExit, BOX2SEQ_MAINSEQ_BAG_RCV },
	// 文字入力
	{ BOX2MAIN_NameInCall, BOX2MAIN_NameInExit, BOX2SEQ_MAINSEQ_STRIN_RCV },
};

// はい・いいえデータテーブル
static const YESNO_DATA YesNoFunc[] =
{
	// ボックス整理・道具を預かる？
	{ BoxArg_ItemGetYes, MainSeq_ArrangePokeMenuRcv },
	// ボックス整理・ポケモンを逃がす？
	{ BoxArg_PokeFreeYes, BoxArg_PokeFreeNo },
	// 道具整理・アイテムしまう？
//	{ BoxItemArg_RetBagYes, MainSeq_ItemArrangeRetBagCancel },
	{ BoxItemArg_RetBagYes, MainSeq_ArrangePokeMenuRcv },
	// ボックス終了する？（ボタン）
	{ BoxArg_BoxEndYes, BoxArg_BoxEndNo },
	// ボックス終了する？（キャンセル）
	{ BoxArg_BoxEndNo, BoxArg_BoxEndYes },
	// 寝かせる？
	{ BoxArg_SleepYes, BoxArg_SleepNo },
};

// メニュー文字列テーブル：ボックス整理メイン
static const BOX2BMP_BUTTON_LIST PokeMenuStrTbl[] =
{
	{ mes_boxbutton_02_08, BOX2BMP_BUTTON_TYPE_WHITE },		// つかむ
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_02, BOX2BMP_BUTTON_TYPE_WHITE },		// もちもの
	{ mes_boxbutton_02_03, BOX2BMP_BUTTON_TYPE_WHITE },		// マーキング
	{ mes_boxbutton_02_04, BOX2BMP_BUTTON_TYPE_WHITE },		// にがす
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_WHITE }		// やめる
};

// メニュー文字列テーブル：つれていく
static const BOX2BMP_BUTTON_LIST PartyInMenuStrTbl[] =
{
	{ mes_boxbutton_02_05, BOX2BMP_BUTTON_TYPE_WHITE },		// つれていく
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_03, BOX2BMP_BUTTON_TYPE_WHITE },		// マーキング
	{ mes_boxbutton_02_04, BOX2BMP_BUTTON_TYPE_WHITE },		// にがす
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_WHITE }		// やめる
};

// メニュー文字列テーブル：あずける
static const BOX2BMP_BUTTON_LIST PartyOutMenuStrTbl[] =
{
	{ mes_boxbutton_02_06, BOX2BMP_BUTTON_TYPE_WHITE },		// あずける
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_03, BOX2BMP_BUTTON_TYPE_WHITE },		// マーキング
	{ mes_boxbutton_02_04, BOX2BMP_BUTTON_TYPE_WHITE },		// にがす
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_WHITE }		// やめる
};

// メニュー文字テーブル：ボックステーマ変更
static const BOX2BMP_BUTTON_LIST ThemaMenuStrTbl[] =
{
	{ mes_boxmenu_03_01, BOX2BMP_BUTTON_TYPE_WHITE },			// ジャンプする
	{ mes_boxmenu_03_02, BOX2BMP_BUTTON_TYPE_WHITE },			// かべがみ
	{ mes_boxmenu_03_03, BOX2BMP_BUTTON_TYPE_WHITE },			// なまえ
	{ mes_boxmenu_03_04, BOX2BMP_BUTTON_TYPE_WHITE },			// やめる
};

// メニュー文字テーブル：壁紙メニュー
static const BOX2BMP_BUTTON_LIST WallPaperMenuStrTbl[] =
{
	{ mes_boxmenu_04_01, BOX2BMP_BUTTON_TYPE_WHITE },			// ふうけい１
	{ mes_boxmenu_04_02, BOX2BMP_BUTTON_TYPE_WHITE },			// ふうけい２
	{ mes_boxmenu_04_03, BOX2BMP_BUTTON_TYPE_WHITE },			// ふうけい３
	{ mes_boxmenu_04_04, BOX2BMP_BUTTON_TYPE_WHITE },			// エトセトラ
	{ mes_boxmenu_04_31, BOX2BMP_BUTTON_TYPE_WHITE },			// もどる
};

// メニュー文字テーブル：壁紙メニュー（ふうけい１）
static const BOX2BMP_BUTTON_LIST WPLabel1MenuStrTbl[] =
{
	{ mes_boxmenu_04_07, BOX2BMP_BUTTON_TYPE_WHITE },			// もり
	{ mes_boxmenu_04_08, BOX2BMP_BUTTON_TYPE_WHITE },			// シティ
	{ mes_boxmenu_04_09, BOX2BMP_BUTTON_TYPE_WHITE },			// さばく
	{ mes_boxmenu_04_10, BOX2BMP_BUTTON_TYPE_WHITE },			// サバンナ
	{ mes_boxmenu_04_31, BOX2BMP_BUTTON_TYPE_WHITE },			// もどる
};

// メニュー文字テーブル：壁紙メニュー（ふうけい２）
static const BOX2BMP_BUTTON_LIST WPLabel2MenuStrTbl[] =
{
	{ mes_boxmenu_04_11, BOX2BMP_BUTTON_TYPE_WHITE },			// いわやま
	{ mes_boxmenu_04_12, BOX2BMP_BUTTON_TYPE_WHITE },			// かざん
	{ mes_boxmenu_04_13, BOX2BMP_BUTTON_TYPE_WHITE },			// ゆきやま
	{ mes_boxmenu_04_14, BOX2BMP_BUTTON_TYPE_WHITE },			// どうくつ
	{ mes_boxmenu_04_31, BOX2BMP_BUTTON_TYPE_WHITE },			// もどる
};

// メニュー文字テーブル：壁紙メニュー（ふうけい３）
static const BOX2BMP_BUTTON_LIST WPLabel3MenuStrTbl[] =
{
	{ mes_boxmenu_04_15, BOX2BMP_BUTTON_TYPE_WHITE },			// うみべ
	{ mes_boxmenu_04_16, BOX2BMP_BUTTON_TYPE_WHITE },			// かいてい
	{ mes_boxmenu_04_17, BOX2BMP_BUTTON_TYPE_WHITE },			// かわ
	{ mes_boxmenu_04_18, BOX2BMP_BUTTON_TYPE_WHITE },			// そら
	{ mes_boxmenu_04_31, BOX2BMP_BUTTON_TYPE_WHITE },			// もどる
};

// メニュー文字テーブル：壁紙メニュー（エトセトラ）
static const BOX2BMP_BUTTON_LIST WPLabel4MenuStrTbl[] =
{
	{ mes_boxmenu_04_19, BOX2BMP_BUTTON_TYPE_WHITE },			// ポケセン
	{ mes_boxmenu_04_20, BOX2BMP_BUTTON_TYPE_WHITE },			// きんぞく
	{ mes_boxmenu_04_21, BOX2BMP_BUTTON_TYPE_WHITE },			// チェック
	{ mes_boxmenu_04_22, BOX2BMP_BUTTON_TYPE_WHITE },			// シンプル
	{ mes_boxmenu_04_31, BOX2BMP_BUTTON_TYPE_WHITE },			// もどる
};

// メニュー文字列テーブル：バトルボックス
static const BOX2BMP_BUTTON_LIST BattleBoxMenuStrTbl[] =
{
	{ mes_boxbutton_02_09, BOX2BMP_BUTTON_TYPE_WHITE },		// いどうする
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_02, BOX2BMP_BUTTON_TYPE_WHITE },		// もちもの
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_WHITE }		// やめる
};

// メニュー文字列テーブル：持ち物整理・持ち物あり
static const BOX2BMP_BUTTON_LIST ItemMenuStrTbl[] =
{
	{ mes_boxbutton_08_03, BOX2BMP_BUTTON_TYPE_WHITE },		// いどうする
	{ mes_boxbutton_08_01, BOX2BMP_BUTTON_TYPE_WHITE },		// バッグへ
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_WHITE }		// やめる
};

// メニュー文字列テーブル：持ち物整理・持ち物なし
static const BOX2BMP_BUTTON_LIST ItemNoneMenuStrTbl[] =
{
	{ mes_boxbutton_08_02, BOX2BMP_BUTTON_TYPE_WHITE },		// もたせる
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_WHITE }		// やめる
};

// メニュー文字列テーブル：ボックス整理メイン
static const BOX2BMP_BUTTON_LIST SleepMenuStrTbl[] =
{
	{ mes_boxbutton_02_10, BOX2BMP_BUTTON_TYPE_WHITE },		// ねかせる
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_WHITE }		// やめる
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス制御
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		seq			メインシーケンス
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = 終了"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2SEQ_Main( BOX2_SYS_WORK * syswk, int * seq )
{
	*seq = MainSeq[*seq]( syswk );

	if( *seq == BOX2SEQ_MAINSEQ_END ){
		return FALSE;
	}

	if( syswk->app != NULL ){
//		BGWINFRM_MoveMain( syswk->app->wfrm );
//		BOX2BGWFRM_SubDispWinFrmMove( syswk );
		BOX2BMP_PrintUtilTrans( syswk->app );
		BOX2OBJ_AnmMain( syswk->app );
	}

	return TRUE;
}


//============================================================================================
//============================================================================================
//	メインシーケンス
//============================================================================================
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス画面初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Init( BOX2_SYS_WORK * syswk )
{
	FS_EXTERN_OVERLAY(ui_common);

	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

	// 割り込み停止
//	sys_VBlankFuncChange( NULL, NULL );
//	sys_HBlankIntrStop();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// サブ画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

	// アプリヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BOX_APP, 0x80000 );
	syswk->app = GFL_HEAP_AllocClearMemory( HEAPID_BOX_APP, sizeof(BOX2_APP_WORK) );

//	syswk->app->ppd_ah      = GFL_ARC_OpenDataHandle( ARCID_PERSONAL, HEAPID_BOX_APP );
	syswk->app->pokeicon_ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_BOX_APP );

	GFL_UI_KEY_GetRepeatSpeed( &syswk->app->key_repeat_speed, &syswk->app->key_repeat_wait );
	GFL_UI_KEY_SetRepeatSpeed( 6, 6 );

	BOX2MAIN_VramBankSet();
	BOX2MAIN_BgInit( syswk );
	BOX2MAIN_PaletteFadeInit( syswk );

	BOX2MAIN_BgGraphicLoad( syswk );

	BOX2MAIN_MsgInit( syswk );

	BOX2BMP_Init( syswk );

	BOX2OBJ_Init( syswk );
	BOX2OBJ_TrayPokeIconChange( syswk );
	BOX2OBJ_PartyPokeIconChange( syswk );

	BOX2MAIN_WallPaperSet(
		syswk, BOX2MAIN_GetWallPaperNumber( syswk, syswk->tray ), BOX2MAIN_TRAY_SCROLL_NONE );

	BOX2BGWFRM_Init( syswk );

	BOX2BMP_TitlePut( syswk );
	BOX2BMP_DefStrPut( syswk );

	BOX2OBJ_TrayIconCgxMakeAll( syswk );

	BOX2UI_CursorMoveInit( syswk );

	BOX2MAIN_YesNoWinInit( syswk );

//	syswk->app->vtask = VIntrTCB_Add( BOX2MAIN_VBlank, syswk, 0 );

	BOX2MAIN_SetBlendAlpha( TRUE );

	BOX2MAIN_InitVBlank( syswk );

	return syswk->next_seq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス画面解放処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_RELEASE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Release( BOX2_SYS_WORK * syswk )
{
	FS_EXTERN_OVERLAY(ui_common);

	BOX2MAIN_ExitVBlank( syswk );

/*
	BOX2MAIN_KeyTouchStatusSet( syswk );

	TCB_Delete( syswk->app->vtask );



	BOX2MAIN_BgFrameWorkExit( syswk->app );
*/
	BOX2MAIN_YesNoWinExit( syswk );

	BOX2UI_CursorMoveExit( syswk );

	BOX2BGWFRM_Exit( syswk->app );

	BOX2OBJ_Exit( syswk->app );

	BOX2BMP_Exit( syswk );

	BOX2MAIN_MsgExit( syswk );

	BOX2MAIN_PaletteFadeExit( syswk );
	BOX2MAIN_BgExit( syswk );

	GFL_UI_KEY_SetRepeatSpeed( syswk->app->key_repeat_speed, syswk->app->key_repeat_wait );

	GFL_ARC_CloseDataHandle( syswk->app->pokeicon_ah );
//	GFL_ARC_CloseDataHandle( syswk->app->ppd_ah );

	GFL_HEAP_FreeMemory( syswk->app );
	GFL_HEAP_DeleteHeap( HEAPID_BOX_APP );
	syswk->app = NULL;

	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

	return syswk->next_seq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ワイプ（フェード）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_WIPE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Wipe( BOX2_SYS_WORK * syswk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return syswk->app->wipe_seq;
	}
	return BOX2SEQ_MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：パレットフェード
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PALETTE_FADE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PaletteFade( BOX2_SYS_WORK * syswk )
{
	if( PaletteFadeCheck(syswk->app->pfd) == 0 ){
		return syswk->next_seq;
	}
	return BOX2SEQ_MAINSEQ_PALETTE_FADE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ウェイト
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_WAIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Wait( BOX2_SYS_WORK * syswk )
{
	if( syswk->app->wait == 0 ){
		return syswk->next_seq;
	}else{
		syswk->app->wait--;
	}
	return BOX2SEQ_MAINSEQ_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：VBlank処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_VFUNC
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_VFunc( BOX2_SYS_WORK * syswk )
{
	if( syswk->app->vfunk.func != NULL ){
		pBOX2_FUNC func = syswk->app->vfunk.func;
		if( func( syswk ) == 0 ){
			syswk->app->vfunk.func = NULL;
//			return MainSeq[syswk->app->vnext_seq]( syswk );
			return syswk->app->vnext_seq;
		}
	}
/*
	if( syswk->app->vfunk.func == NULL ){
		return MainSeq[syswk->next_seq]( syswk );
	}
*/
	return BOX2SEQ_MAINSEQ_VFUNC;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：キー・タッチ待ち
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_TRGWAIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_TrgWait( BOX2_SYS_WORK * syswk )
{
	if( GFL_UI_TP_GetTrg() == TRUE ){
//		Snd_SePlay( SE_BOX2_DECIDE );
		return syswk->next_seq;
	}

	if( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_CANCEL | PAD_BUTTON_DECIDE ) ){
//		Snd_SePlay( SE_BOX2_DECIDE );
		return syswk->next_seq;
	}

	return BOX2SEQ_MAINSEQ_TRGWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：はい・いいえ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_YESNO
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_YesNo( BOX2_SYS_WORK * syswk )
{
	APP_TASKMENU_UpdateMenu( syswk->app->ynWork );
	if( APP_TASKMENU_IsFinish( syswk->app->ynWork ) == TRUE ){
		APP_TASKMENU_CloseMenu( syswk->app->ynWork );
		GFL_UI_KEY_SetRepeatSpeed( 6, 6 );
		if( APP_TASKMENU_GetCursorPos( syswk->app->ynWork ) == 0 ){
			return YesNoFunc[syswk->app->ynID].yes( syswk );
		}else{
			return YesNoFunc[syswk->app->ynID].no( syswk );
		}
	}
	return BOX2SEQ_MAINSEQ_YESNO;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボタンアニメ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BUTTON_ANM
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ButtonAnm( BOX2_SYS_WORK * syswk )
{
	if( BOX2MAIN_ButtonAnmMain( syswk ) == FALSE ){
		return syswk->next_seq;
	}
	return BOX2SEQ_MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：サブプロセス呼び出し
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_SUB_PROCCALL
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SubProcCall( BOX2_SYS_WORK * syswk )
{
	SubProcFunc[syswk->subProcType].call( syswk );
	return BOX2SEQ_MAINSEQ_SUB_PROCMAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：サブプロセス終了待ち
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_SUB_PROCMAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SubProcMain( BOX2_SYS_WORK * syswk )
{
	SubProcFunc[syswk->subProcType].exit( syswk );
	syswk->next_seq = SubProcFunc[syswk->subProcType].rcvSeq;
	return BOX2SEQ_MAINSEQ_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：開始処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_START
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Start( BOX2_SYS_WORK * syswk )
{
	int	seq;

//	Snd_SePlay( SE_BOX2_LOG_IN );

	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:		// あずける
		PartyFrmSet_PartyOut( syswk );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );
		seq = BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
		break;

	case BOX_MODE_TURETEIKU:	// つれていく
		BOX2MAIN_PokeInfoPut( syswk, 0 );
		seq = BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
		break;

	case BOX_MODE_SEIRI:		// ボックスせいり
		BOX2MAIN_PokeInfoPut( syswk, 0 );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		seq = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		break;

	case BOX_MODE_ITEM:			// アイテム整理
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		BOX2MAIN_PokeInfoPut( syswk, 0 );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		seq = BOX2SEQ_MAINSEQ_ITEM_MAIN;
		break;

	case BOX_MODE_BATTLE:		// バトルボックス
		if( PokeParty_GetPokeCount(syswk->dat->pokeparty) == 0 ){
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			BOX2MAIN_PokeInfoPut( syswk, 0 );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_MAIN, 0 );
			seq = BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
		}else{
			PartyFrmSet_PartyOut( syswk );
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			BOX2BGWFRM_BoxListButtonOn( syswk->app );
			BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_PARTY, 0 );
			seq = BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
		}
		break;

	case BOX_MODE_SLEEP:		// ねかせる
		BOX2MAIN_PokeInfoPut( syswk, 0 );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		seq = BOX2SEQ_MAINSEQ_SLEEP_MAIN;
		break;
	}

	return FadeInSet( syswk, seq );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：カーソル移動後の処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_CURSORMOVE_RET
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_CursorMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
	if( syswk->app->msg_put == 1 ){
		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			if( syswk->app->get_item == ITEM_DUMMY_DATA ){
				BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_SET, BOX2BMPWIN_ID_MSG1 );
			}else{
				BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_INIT, BOX2BMPWIN_ID_MSG1 );
			}
		}else{
			BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		}
		syswk->app->msg_put = 0;
	}
	return syswk->next_seq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ポケモンメニュー表示状態へ復帰
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeMenuRcv( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		switch( syswk->dat->callMode ){
		case BOX_MODE_TURETEIKU:
			BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 5 );
			break;
		case BOX_MODE_AZUKERU:
			BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 5 );
			break;
		case BOX_MODE_SEIRI:
			BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
			break;
		case BOX_MODE_ITEM:
			BOX2SEQ_ItemModeMenuSet( syswk, syswk->app->get_item );
			break;
		case BOX_MODE_BATTLE:
			BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
			break;
		case BOX_MODE_SLEEP:
			BOX2BMP_MenuStrPrint( syswk, SleepMenuStrTbl, 2 );
			break;
		default:
			break;
		}
		if( syswk->app->poke_free_err == 1 ){
			syswk->app->poke_free_err = 0;
			if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
			}else{
				BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
			}
		}
		BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV );

	case 1:
		syswk->app->sub_seq = 0;
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
		switch( syswk->dat->callMode ){
		case BOX_MODE_TURETEIKU:
			BOX2BMP_PokeSelectMsgPut(
				syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
			return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;

		case BOX_MODE_AZUKERU:
			BOX2BMP_PokeSelectMsgPut(
				syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
			return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;

		case BOX_MODE_SEIRI:
			BOX2BMP_PokeSelectMsgPut(
				syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
			if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
			}else{
				return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
			}
			break;

		case BOX_MODE_ITEM:
			if( syswk->app->get_item == ITEM_DUMMY_DATA ){
				BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_SET, BOX2BMPWIN_ID_MSG1 );
			}else{
				BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_INIT, BOX2BMPWIN_ID_MSG1 );
			}
			if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				return BOX2SEQ_MAINSEQ_ITEM_MAIN;
			}else{
				return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
			}

		case BOX_MODE_BATTLE:
			BOX2BMP_PokeSelectMsgPut(
				syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
			if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
			}else{
				return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
			}
			break;

		case BOX_MODE_SLEEP:
			BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
			return BOX2SEQ_MAINSEQ_SLEEP_MAIN;

		default:
			break;
		}
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：タッチでのトレイアイコンスクロール
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_TrayScrollTouch( BOX2_SYS_WORK * syswk )
{
	SEQWK_TRAYICON_SCROLL * seqwk;
	u32	x, y;
	u32	oy;

	if( BOX2UI_HitCheckTrayScroll( &x, &y ) == FALSE ){
		GFL_HEAP_FreeMemory( syswk->app->seqwk );
		return syswk->next_seq;
	}

	seqwk = syswk->app->seqwk;

	oy = syswk->app->tpy;

	syswk->app->tpy = y;

	if( GFL_STD_Abs(oy-y) >= 3 ){
		seqwk->cnt = GFL_STD_Abs( oy - y ) / 8;
		if( y < oy ){
			seqwk->mv = 1;
			BOX2MAIN_InitTrayIconScroll( syswk, 1 );
			if( syswk->next_seq == BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN ){
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUp, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}else{
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUpJump, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}
		}
		if( y > oy ){
			seqwk->mv = -1;
			BOX2MAIN_InitTrayIconScroll( syswk, -1 );
			if( syswk->next_seq == BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN ){
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDown, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}else{
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDownJump, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}
		}
	}

	if( seqwk->cnt != 0 ){
		seqwk->cnt--;
		if( seqwk->mv == 1 ){
			BOX2MAIN_InitTrayIconScroll( syswk, 1 );
			if( syswk->next_seq == BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN ){
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUp, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}else{
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUpJump, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}
		}
		if( seqwk->mv == -1 ){
			BOX2MAIN_InitTrayIconScroll( syswk, -1 );
			if( syswk->next_seq == BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN ){
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDown, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}else{
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDownJump, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}
		}
	}

	return BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：終了処理（メッセージなし）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXEND_DIRECT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxEndDirect( BOX2_SYS_WORK * syswk )
{
	return BoxArg_BoxEndYes( syswk );
}


//============================================================================================
//	ボックス整理
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス整理メイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitTrayPokeTrg();
	if( ret != GFL_UI_TP_HIT_NONE ){
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return TrayPokeGet( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				BOX2BGWFRM_TemochiButtonOn( syswk->app );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_MAIN) );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
			}
		}
	}

	ret = BOX2UI_BoxArrangeMain( syswk );

	switch( ret ){
	case BOX2UI_ARRANGE_MAIN_NAME:		// 30: ボックス名
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );

	case BOX2UI_ARRANGE_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_NAME );
		return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );

	case BOX2UI_ARRANGE_MAIN_RIGHT:	// 32: トレイ切り替え矢印・右
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_NAME );
		return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );

	case BOX2UI_ARRANGE_MAIN_PARTY:	// 33: 手持ちポケモン
//		Snd_SePlay( SE_BOX2_OPEN_PARTY_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_INIT) );

	case BOX2UI_ARRANGE_MAIN_RETURN1:	// 34: 戻る１
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_ARRANGE_MAIN_RETURN2:	// 35: 戻る２
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_ARRANGE_MAIN_GET:			// 36: つかむ
/*
//		Snd_SePlay( SE_BOX2_DECIDE );
		if( GFL_UI_TP_GetTrg() == FALSE ){
			syswk->quick_mode = 1;
		}
		syswk->quick_get = syswk->get_pos;
		return YStatusButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU1, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT );
*/
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU1, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT) );

	case BOX2UI_ARRANGE_MAIN_STATUS:	// 37: ようすをみる
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
		syswk->get_tray    = syswk->tray;
		syswk->cur_rcv_pos = BOX2UI_ARRANGE_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_ARRANGE_MAIN_ITEM:		// 38: もちもの
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
		syswk->cur_rcv_pos = BOX2UI_ARRANGE_MAIN_ITEM;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );

	case BOX2UI_ARRANGE_MAIN_MARKING:	// 39: マーキング
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_MARKING_INIT) );

	case BOX2UI_ARRANGE_MAIN_FREE:		// 40: にがす
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_POKEFREE_INIT) );

	case BOX2UI_ARRANGE_MAIN_CLOSE:		// 41: とじる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ARRANGE_MENU_CLOSE );

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_ARRANGE_MAIN_GET &&
					pos != BOX2UI_ARRANGE_MAIN_STATUS &&
					pos != BOX2UI_ARRANGE_MAIN_ITEM &&
					pos != BOX2UI_ARRANGE_MAIN_MARKING &&
					pos != BOX2UI_ARRANGE_MAIN_FREE &&
					pos != BOX2UI_ARRANGE_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_MAIN) );

	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_ARRANGE_MAIN_GET &&
					pos != BOX2UI_ARRANGE_MAIN_STATUS &&
					pos != BOX2UI_ARRANGE_MAIN_ITEM &&
					pos != BOX2UI_ARRANGE_MAIN_MARKING &&
					pos != BOX2UI_ARRANGE_MAIN_FREE &&
					pos != BOX2UI_ARRANGE_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ARRANGE_MAIN_GET, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_GET );
//			return TrayPokeGetKeyArrange( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
			return TrayPokeGetKey( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
		}
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：トレイのポケモン移動後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_TRAYPOKE_CHG_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeTrayPokeChgEnd( BOX2_SYS_WORK * syswk )
{
//	Snd_SePlay( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_GET );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「ボックス整理」メニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_MENU_CLOSE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2BGWFRM_TemochiButtonOn( syswk->app );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「つかむ」初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeGetInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		syswk->get_tray   = syswk->tray;	// 取得したボックス
		syswk->box_mv_pos = syswk->tray;	// 現在参照しているボックス
//		syswk->box_mv_flg = 1;						// ボックス移動フラグ
//		syswk->pokechg_mode = 0;
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
//		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//		BOX2MAIN_GetPokeData( syswk, syswk->tray, syswk->get_pos );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT );

	case 1:
		syswk->app->sub_seq++;
		return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT );

	case 2:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );
		syswk->app->old_cur_pos = syswk->get_pos;
		syswk->app->sub_seq = 0;
		return PokeMoveGetKey( syswk, syswk->get_pos );
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「つかむ」メイン処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeGetMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u32	x, y;

	// ポケモンドラッグチェック
	if( syswk->poke_get_key == 0 ){
		ret = BOX2UI_HitTrayPokeTrg();
		if( ret != GFL_UI_TP_HIT_NONE ){
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//				Snd_SePlay( SE_BOX2_POKE_CATCH );
				GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
				BOX2BGWFRM_TemochiButtonOff( syswk->app );
//				CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PGT_PARTY );
				BOX2MAIN_PokeInfoPut( syswk, ret );
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				return BoxPartyPokeGet( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
			}
		}
	}

	// タッチトレイスクロールチェック
	if( BOX2UI_HitCheckTrayScrollStart( &x, &y ) == TRUE ){
		syswk->app->tpy = y;
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
		CreateTrayIconScrollWork( syswk );
		return BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH;
	}

	// キー・タッチチェック
	ret = BOX2UI_ArrangePokeGetMain( syswk );

	switch( ret ){
	case BOX2UI_ARRANGE_PGT_NAME:		// 30: ボックス名
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		// 未取得
		if( syswk->poke_get_key == 0 ){
//		Snd_SePlay( SE_BOX2_DECIDE );
			BOX2MAIN_PokeInfoOff( syswk );
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );
		}
		break;

	case BOX2UI_ARRANGE_PGT_LEFT:		// 31: ボックス切り替え矢印（左）
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
			BOX2MAIN_PokeSelectOff( syswk );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PGT_NAME );
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		// 取得中
		}else{
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case BOX2UI_ARRANGE_PGT_RIGHT:	// 32: ボックス切り替え矢印（右）
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
			BOX2MAIN_PokeSelectOff( syswk );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PGT_NAME );
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		// 取得中
		}else{
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case BOX2UI_ARRANGE_PGT_PARTY:	// 39: 手持ちポケモン
		BOX2MAIN_PokeInfoOff( syswk );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_PARTY_CHANGE) );

	case BOX2UI_ARRANGE_PGT_STATUS:	// 40: ステータス
		if( syswk->poke_get_key == 0 ){
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				break;
			}else{
				if( BOX2MAIN_PokeParaGet( syswk, pos, syswk->tray, ID_PARA_poke_exist, NULL ) == 0 ){
					break;
				}
			}
			syswk->get_tray = syswk->tray;
			syswk->get_pos  = pos;
			syswk->subProcMode = SUB_PROC_MODE_TB_BOX;
		}else{
			syswk->subProcMode = SUB_PROC_MODE_TB_BOX_GET;
			syswk->tb_status_pos = CURSORMOVE_PosGet( syswk->app->cmwk );
		}
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_STATUS, 1, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_ARRANGE_PGT_RET:		// 41: 戻る
		if( syswk->poke_get_key == 0 ){
			// ボックス整理メインへ
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT) );
		}
		break;

	case CURSORMOVE_CANCEL:					// キャンセル
		if( syswk->poke_get_key == 1 ){
			// 取得中なので失敗
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
			CURSORMOVE_MoveTableBitOn( syswk->app->cmwk, BOX2UI_ARRANGE_PGT_PARTY );
			return PokeMovePutKey( syswk, BOX2MAIN_GETPOS_NONE, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}else{
			// ボックス整理メインへ
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT) );
		}
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PGT_TRAY2 ){
			BOX2MAIN_InitTrayIconScroll( syswk, -1 );
			return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDown, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PGT_TRAY5 ){
			BOX2MAIN_InitTrayIconScroll( syswk, 1 );
			return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUp, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PGT_NAME ){
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PGT_NAME ){
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( syswk->poke_get_key == 0 ){
				if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					BOX2MAIN_PokeInfoPut( syswk, pos );
				}else{
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
			if( pos >= BOX2UI_ARRANGE_PGT_TRAY2 && pos <= BOX2UI_ARRANGE_PGT_TRAY5 ){
				BOX2OBJ_ChangeTrayName( syswk, pos-BOX2UI_ARRANGE_PGT_TRAY2, TRUE );
			}else{
				BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
			}
		}
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN) );

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case BOX2UI_ARRANGE_PGT_TRAY1:	// 33: トレイアイコン
		BOX2MAIN_InitTrayIconScroll( syswk, -1 );
		return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDown, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );

	case BOX2UI_ARRANGE_PGT_TRAY6:	// 38: トレイアイコン
		BOX2MAIN_InitTrayIconScroll( syswk, 1 );
		return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUp, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );

	case BOX2UI_ARRANGE_PGT_TRAY2:	// 34: トレイアイコン
	case BOX2UI_ARRANGE_PGT_TRAY3:	// 35: トレイアイコン
	case BOX2UI_ARRANGE_PGT_TRAY4:	// 36: トレイアイコン
	case BOX2UI_ARRANGE_PGT_TRAY5:	// 37: トレイアイコン
		BOX2OBJ_ChangeTrayName( syswk, ret-BOX2UI_ARRANGE_PGT_TRAY2, TRUE );
		// 取得中
		if( syswk->poke_get_key == 1 ){
			CURSORMOVE_MoveTableBitOn( syswk->app->cmwk, BOX2UI_ARRANGE_PGT_PARTY );
			return PokeMovePutKey( syswk, BOX2OBJ_POKEICON_PUT_MAX+ret-BOX2UI_ARRANGE_PGT_TRAY2, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		// 未取得
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			syswk->app->chg_tray_pos = syswk->box_mv_pos + ret - BOX2UI_ARRANGE_PGT_TRAY2;
			if( syswk->app->chg_tray_pos >= syswk->trayMax ){
				syswk->app->chg_tray_pos -= syswk->trayMax;
			}
			if( syswk->app->chg_tray_pos != syswk->tray ){
				return BoxMoveTrayScrollSet( syswk, 0 );
			}
		}
		break;

	default:
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		// 取得中
		if( syswk->poke_get_key == 1 ){
			CURSORMOVE_MoveTableBitOn( syswk->app->cmwk, BOX2UI_ARRANGE_PGT_PARTY );
			return PokeMovePutKey( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		// 未取得
		}else{
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				syswk->get_tray = syswk->tray;	// 取得したボックス
				syswk->get_pos  = ret;
				BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
//				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
//				BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
//				BOX2MAIN_GetPokeData( syswk, syswk->tray, syswk->get_pos );
//				BOX2OBJ_TrayIconCgxMake( syswk, syswk->tray );
//				BOX2OBJ_TrayIconCgxTrans( syswk, syswk->tray );
				return PokeMoveGetKey( syswk, syswk->get_pos );
			}
		}
		break;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「つかむ」終了処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeGetExit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
		BOX2MAIN_PokeInfoOff( syswk );
		syswk->app->sub_seq++;
		return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT );

	case 1:
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos >= BOX2UI_ARRANGE_PGT_NAME ){
				pos = 0;
			}
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, pos );
			if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}
		}
		syswk->app->sub_seq = 0;
		return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ち・トレイ入れ替え後（キー操作）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeGetDataChange( BOX2_SYS_WORK * syswk )
{
	u8	pos;
//	BOX2MAIN_POKEMOVE_WORK * work;

//	work = syswk->app->vfunk.work;

//	BOX2MAIN_PutPokeData( syswk, syswk->tray, syswk->app->poke_put_key );

	BOX2MAIN_PokeDataMove( syswk );

/*
	// 参照しているのトレイ
	if( syswk->app->poke_put_key < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_TrayIconCgxMake( syswk, syswk->tray );
		BOX2OBJ_TrayIconCgxTrans( syswk, syswk->tray );
		if( syswk->tray != syswk->get_tray ){
			BOX2OBJ_TrayIconCgxMake( syswk, syswk->get_tray );
			BOX2OBJ_TrayIconCgxTrans( syswk, syswk->get_tray );
		}
	// 別トレイ
	}else if( syswk->app->poke_put_key >= BOX2OBJ_POKEICON_PUT_MAX ){
		u16	put_pos;
		u16	tray;
		put_pos = syswk->app->poke_put_key - BOX2OBJ_POKEICON_PUT_MAX;
		tray    = syswk->box_mv_pos + put_pos;
		if( tray >= syswk->trayMax ){
			tray -= syswk->trayMax;
		}
		BOX2OBJ_TrayIconCgxMake( syswk, tray );
		BOX2OBJ_TrayIconCgxTransPos( syswk, tray, put_pos+1 );
	}
*/

	syswk->poke_get_key = 0;
	syswk->get_tray = BOX2MAIN_GETPOS_NONE;

	/*
	if( ( put_pos & BOX2MAIN_BOXMOVE_FLG ) == 0 ){
		BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );
	}
	*/
	PokeMoveWorkFree( syswk );

	pos = CURSORMOVE_PosGet( syswk->app->cmwk );

	if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
		if( pos < BOX2OBJ_POKEICON_TRAY_MAX &&
				BOX2MAIN_PokeParaGet(syswk,pos,syswk->tray,ID_PARA_poke_exist,NULL) != 0 ){
			BOX2MAIN_PokeInfoPut( syswk, pos );
			BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );
		}
	}else{
		if( pos < BOX2OBJ_POKEICON_PUT_MAX &&
				BOX2MAIN_PokeParaGet(syswk,pos,syswk->tray,ID_PARA_poke_exist,NULL) != 0 ){
			BOX2MAIN_PokeInfoPut( syswk, pos );
			BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );
		}
	}

	switch( syswk->app->mv_err_code ){
	case BOX2MAIN_ERR_CODE_MAIL:			// メールを持っている
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_PutPokeMoveErrMsg( syswk, BOX2MAIN_ERR_CODE_MAIL, BOX2BMPWIN_ID_MSG1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE_ERR;

	case BOX2MAIN_ERR_CODE_BATTLE:		// 戦えるポケモンがいなくなる
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_PutPokeMoveErrMsg( syswk, BOX2MAIN_ERR_CODE_BATTLE, BOX2BMPWIN_ID_MSG1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE_ERR;

	case BOX2MAIN_ERR_CODE_EGG:				// タマゴはえらべない（バトルボックス専用）
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_PutPokeMoveErrMsg( syswk, BOX2MAIN_ERR_CODE_EGG, BOX2BMPWIN_ID_MSG1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE_ERR;
	}

	if( syswk->dat->callMode == BOX_MODE_SEIRI ){
		if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
		}else{
			BOX2BGWFRM_BoxListButtonOn( syswk->app );
		}
	}

//	return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
	return syswk->next_seq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ポケモン配置時のエラー処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE_ERR
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeGetDataChangeErr( BOX2_SYS_WORK * syswk )
{
	if( MainSeq_TrgWait( syswk ) == BOX2SEQ_MAINSEQ_TRGWAIT ){
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE_ERR;
	}

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );

	if( syswk->dat->callMode == BOX_MODE_SEIRI ){
		if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
		}else{
			BOX2BGWFRM_BoxListButtonOn( syswk->app );
		}
	}

	syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_NONE;

	return syswk->next_seq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：トレイの「つかむ」から手持ちの「つかむ」へきりかえ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_PARTY_CHANGE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeGetPartyChange( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		BOX2BGWFRM_TemochiButtonOff( syswk->app );
		syswk->app->sub_seq++;
		return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_PARTY_CHANGE );

	case 1:
		BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		BOX2OBJ_PartyPokeIconFrmInSet( syswk );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_PARTY_CHANGE );

	case 2:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, BOX2UI_ARRANGE_PTGT_BOXLIST );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_PTGT_BOXLIST;
		syswk->app->sub_seq = 0;
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_PARTY_CHANGE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ち選択初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_PARTY_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

		BOX2BGWFRM_PartyPokeFrameInitPutLeft( syswk->app->wfrm );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		BOX2OBJ_PartyPokeIconFrmInSet( syswk );
		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
		}
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_INIT );

	case 1:
		syswk->app->sub_seq = 0;
		BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_PARTY, BOX2UI_ITEM_PARTY_POKE1 );
			return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
		}
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_POKE1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ち選択メイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	// タッチ取得チェック
	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return PartyPokeGet( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				BOX2BGWFRM_BoxListButtonOn( syswk->app );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN) );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
			}
		}
	}

	// キー・タッチチェック
	ret = BOX2UI_BoxArrangePartyMoveMain( syswk );

	switch( ret ){
	case BOX2UI_ARRANGE_PARTY_BOXLIST:	// 06:「ボックスリスト」
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END) );

	case BOX2UI_ARRANGE_PARTY_RETURN1:	// 07: 戻る１（ボックス画面終了）
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_ARRANGE_PARTY_RETURN2:	// 08: 戻る２
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_ARRANGE_PARTY_GET:				// 09: つかむ
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU1, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT) );

	case BOX2UI_ARRANGE_PARTY_STATUS:			// 10: ようすをみる
		syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
		syswk->get_tray    = syswk->tray;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_ARRANGE_PARTY_ITEM:				// 11: もちもの
		syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );

	case BOX2UI_ARRANGE_PARTY_MARKING:		// 12: マーキング
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_MARKING_INIT) );

	case BOX2UI_ARRANGE_PARTY_FREE:				// 13: にがす
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_POKEFREE_INIT) );

	case BOX2UI_ARRANGE_PARTY_CLOSE:			// 14: やめる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MENU_CLOSE );

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
	case CURSORMOVE_CURSOR_ON:			// カーソル表示
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_PARTY_BOXLIST ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}else if( pos == BOX2UI_ARRANGE_PARTY_BOXLIST ){
				BOX2MAIN_PokeInfoOff( syswk );
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN) );

	case CURSORMOVE_CANCEL:							// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ARRANGE_PARTY_GET, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PARTY_GET );
			return PartyPokeGetKey( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN );
		}
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ち選択終了処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyEnd( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END );

	case 1:
		syswk->app->sub_seq = 0;
//		BOX2MAIN_PokeInfoPut( syswk, 0 );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_MAIN, BOX2UI_ITEM_MAIN_PARTY );
			return BOX2SEQ_MAINSEQ_ITEM_MAIN;
		}
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_PARTY );
		return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MENU_CLOSE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2BGWFRM_BoxListButtonOn( syswk->app );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ちポケモン入れ替え後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_PARTYPOKE_CHG_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyPokeChgEnd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	pos;

	work = syswk->app->vfunk.work;
	pos  = work->get_pos;

//	Snd_SePlay( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
//	BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PARTY_GET );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ち「つかむ」初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyPokeGetInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			syswk->get_tray = BOX2MAIN_GETPOS_NONE;	// 取得したボックス
		}else{
			syswk->get_tray = syswk->tray;					// 取得したボックス
		}
//		syswk->box_mv_pos = syswk->tray;	// 現在参照しているボックス
//		syswk->box_mv_flg = 1;						// ボックス移動フラグ
//		syswk->pokechg_mode = 0;
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
//		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//		BOX2MAIN_GetPokeData( syswk, syswk->tray, syswk->get_pos );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );

	case 1:
		syswk->app->sub_seq++;
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			BOX2BGWFRM_PartyPokeFrameRightMoveSet( syswk->app->wfrm );
//			return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrmRight, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );
		}else{
			BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
			BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
			BOX2OBJ_PartyPokeIconFrmInSet( syswk );
		}
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );

	case 2:
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
		syswk->app->old_cur_pos = syswk->get_pos;
		syswk->app->sub_seq = 0;
		return PartyPokeMoveGetKey( syswk, syswk->get_pos );
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ち「つかむ」メイン処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyPokeGetMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	// ポケモンドラッグチェック
	if( syswk->poke_get_key == 0 ){
		// トレイのポケモン
		ret = BOX2UI_HitTrayPokeTrg();
		if( ret != GFL_UI_TP_HIT_NONE ){
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//				Snd_SePlay( SE_BOX2_POKE_CATCH );
				GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
				BOX2BGWFRM_BoxListButtonOff( syswk->app );
				BOX2MAIN_PokeInfoPut( syswk, ret );
				return BoxPartyPokeGet( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
			}
		}
		// 手持ちのポケモン
		ret = BOX2UI_HitCheckTrgPartyPokeRight();
		if( ret != GFL_UI_TP_HIT_NONE ){
			ret += BOX2OBJ_POKEICON_TRAY_MAX;
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//				Snd_SePlay( SE_BOX2_POKE_CATCH );
				GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
				BOX2BGWFRM_BoxListButtonOff( syswk->app );
				BOX2MAIN_PokeInfoPut( syswk, ret );
				return BoxPartyPokeGet( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
			}
		}
	}

	// キー・タッチチェック
	ret = BOX2UI_ArrangePartyPokeGetMain( syswk );

	switch( ret ){
	case BOX2UI_ARRANGE_PTGT_NAME:		// 36: ボックス名
		// 未取得
		if( syswk->poke_get_key == 0 ){
//		Snd_SePlay( SE_BOX2_DECIDE );
			BOX2MAIN_PokeInfoOff( syswk );
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_LEFT:		// 37: ボックス切り替え矢印（左）
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_NAME );
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		// 取得中
		}else{
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_RIGHT:		// 38: ボックス切り替え矢印（右）
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_NAME );
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		// 取得中
		}else{
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_BOXLIST:	// 39: ボックスリスト
		BOX2MAIN_PokeInfoOff( syswk );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXLIST_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_BOXLIST_CHANGE) );

	case BOX2UI_ARRANGE_PTGT_STATUS:		// 40: ステータス
		if( syswk->poke_get_key == 0 ){
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos >= BOX2OBJ_POKEICON_PUT_MAX ){
				break;
			}else{
				if( BOX2MAIN_PokeParaGet( syswk, pos, syswk->tray, ID_PARA_poke_exist, NULL ) == 0 ){
					break;
				}
			}
			syswk->get_tray = syswk->tray;
			syswk->get_pos  = pos;
			syswk->subProcMode = SUB_PROC_MODE_TB_PARTY;
		}else{
			syswk->subProcMode = SUB_PROC_MODE_TB_PARTY_GET;
			syswk->tb_status_pos = CURSORMOVE_PosGet( syswk->app->cmwk );
		}
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_STATUS, 1, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_ARRANGE_PTGT_RET:		// 41: 戻る
		if( syswk->poke_get_key == 0 ){
			// ボックス整理メインへ
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT) );
		}
		break;

	case CURSORMOVE_CANCEL:					// キャンセル
		if( syswk->poke_get_key == 1 ){
			// 取得中なので失敗
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
			if( syswk->dat->callMode == BOX_MODE_SEIRI ){
				CURSORMOVE_MoveTableBitOn( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
			}
			return PokeMovePutKey( syswk, BOX2MAIN_GETPOS_NONE, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		}else{
			// ボックス整理メインへ
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT) );
		}
		break;

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PTGT_NAME ){
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PTGT_NAME ){
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_CURSOR_ON:			// カーソル表示
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		if( syswk->poke_get_key == 0 ){
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2OBJ_POKEICON_PUT_MAX ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
			}
		}
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN) );

	default:
		// 取得中
		if( syswk->poke_get_key == 1 ){
			if( syswk->dat->callMode == BOX_MODE_SEIRI ){
				CURSORMOVE_MoveTableBitOn( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
			}
			return PokeMovePutKey( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		// 未取得
		}else{
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				syswk->get_tray = syswk->tray;	// 取得したボックス
				syswk->get_pos  = ret;
				BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
//				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
//				BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
//				BOX2MAIN_GetPokeData( syswk, syswk->tray, syswk->get_pos );
//				BOX2OBJ_TrayIconCgxMake( syswk, syswk->tray );
//				BOX2OBJ_TrayIconCgxTrans( syswk, syswk->tray );
				return PartyPokeMoveGetKey( syswk, syswk->get_pos );
			}
		}
		break;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ち「つかむ」終了処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyPokeGetExit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
		BOX2MAIN_PokeInfoOff( syswk );
		if( syswk->dat->callMode == BOX_MODE_BATTLE && syswk->pokechg_mode == 0 ){
			BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
			syswk->app->sub_seq = 2;
		}else{
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
			BOX2BGWFRM_PartyPokeFrameLeftMoveSet( syswk->app->wfrm );
			syswk->app->sub_seq = 1;
		}
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT );

	case 1:
		syswk->app->sub_seq = 0;
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2OBJ_POKEICON_TRAY_MAX || pos >= BOX2OBJ_POKEICON_PUT_MAX ){
				pos = 0;
			}else{
				pos -= BOX2OBJ_POKEICON_TRAY_MAX;
			}
			BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			if( syswk->dat->callMode == BOX_MODE_SEIRI ){
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, pos );
				return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
			}else{
				BOX2BGWFRM_BoxListButtonOn( syswk->app );
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_PARTY, pos );
				return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
			}
		}

	case 2:
		syswk->app->sub_seq = 0;
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				pos = 0;
			}
			BOX2MAIN_PokeInfoPut( syswk, pos );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_MAIN, pos );
			return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
		}
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ちの「つかむ」からトレイの「つかむ」へきりかえ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_BOXLIST_CHANGE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyPokeGetBoxListChange( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_BoxListButtonOff( syswk->app );
		BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_BOXLIST_CHANGE );

	case 1:
		syswk->box_mv_pos = syswk->tray;	// 現在参照しているボックス
		syswk->app->sub_seq++;
		return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_BOXLIST_CHANGE );

	case 2:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, BOX2UI_ARRANGE_PGT_PARTY );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_PGT_PARTY;
		syswk->app->sub_seq = 0;
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_BOXLIST_CHANGE;
}


//============================================================================================
//	バトルボックス
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：バトルボックスメイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BattleBoxMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitTrayPokeTrg();
	if( ret != GFL_UI_TP_HIT_NONE ){
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return GetBattleBoxTrayPoke( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				BOX2BGWFRM_TemochiButtonOn( syswk->app );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN) );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
			}
		}
	}

	ret = BOX2UI_BattleBoxMain( syswk );

	switch( ret ){
	case BOX2UI_BATTLEBOX_MAIN_NAME:		// 30: ボックス名
		break;

	case BOX2UI_BATTLEBOX_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_MAIN_NAME );
		return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );

	case BOX2UI_BATTLEBOX_MAIN_RIGHT:		// 32: トレイ切り替え矢印・右
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_MAIN_NAME );
		return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );

	case BOX2UI_BATTLEBOX_MAIN_PARTY:		// 33: バトルボックス
//		Snd_SePlay( SE_BOX2_OPEN_PARTY_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_INIT) );

	case BOX2UI_BATTLEBOX_MAIN_RETURN1:	// 34: 戻る１
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_BATTLEBOX_MAIN_RETURN2:	// 35: 戻る２
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_BATTLEBOX_MAIN_GET:			// 36: いどうする
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->pokechg_mode = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT) );

	case BOX2UI_BATTLEBOX_MAIN_STATUS:	// 37: ようすをみる
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
		syswk->get_tray    = syswk->tray;
		syswk->cur_rcv_pos = BOX2UI_BATTLEBOX_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_BATTLEBOX_MAIN_ITEM:		// 38: もちもの
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
		syswk->cur_rcv_pos = BOX2UI_BATTLEBOX_MAIN_ITEM;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );

	case BOX2UI_BATTLEBOX_MAIN_CLOSE:		// 39: やめる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_BATTLEBOX_MENU_CLOSE );

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_BATTLEBOX_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_BATTLEBOX_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_BATTLEBOX_MAIN_GET &&
						pos != BOX2UI_BATTLEBOX_MAIN_STATUS &&
						pos != BOX2UI_BATTLEBOX_MAIN_ITEM &&
						pos != BOX2UI_BATTLEBOX_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN) );

	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_BATTLEBOX_MAIN_GET &&
						pos != BOX2UI_BATTLEBOX_MAIN_STATUS &&
						pos != BOX2UI_BATTLEBOX_MAIN_ITEM &&
						pos != BOX2UI_BATTLEBOX_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_BATTLEBOX_MAIN_GET, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_MAIN_GET );
//			return TrayPokeGetKeyArrange( syswk, ret, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );
			return TrayPokeGetKey( syswk, ret, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );
		}
	}

	return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：トレイのポケモン移動後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BATTLEBOX_TRAYPOKE_CHG_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BattleBoxTrayPokeChgEnd( BOX2_SYS_WORK * syswk )
{
//	Snd_SePlay( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_MAIN_GET );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
	return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「バトルボックス」メニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BATTLEBOX_MENU_CLOSE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BattleBoxMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2BGWFRM_TemochiButtonOn( syswk->app );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「バトルボックス」手持ち選択初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BattleBoxPartyInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

		BOX2BGWFRM_PartyPokeFrameInitPutLeft( syswk->app->wfrm );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		BOX2OBJ_PartyPokeIconFrmInSet( syswk );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_INIT );

	case 1:
		syswk->app->sub_seq = 0;
		BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_PARTY, BOX2UI_BATTLEBOX_PARTY_POKE1 );
		return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
	}

	return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「バトルボックス」手持ち選択メイン処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BattleBoxPartyMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	// タッチ取得チェック
	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return GetBattleBoxPartyPoke( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				BOX2BGWFRM_BoxListButtonOn( syswk->app );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN) );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
			}
		}
	}

	// キー・タッチチェック
	ret = BOX2UI_BattleBoxPartyMain( syswk );

	switch( ret ){
	case BOX2UI_BATTLEBOX_PARTY_BOXLIST:		// 06:「ボックスリスト」
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_END) );

	case BOX2UI_BATTLEBOX_PARTY_RETURN1:		// 07: 戻る１
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_BATTLEBOX_PARTY_RETURN2:		// 08: 戻る２
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_BATTLEBOX_PARTY_GET:				// 09: いどうする
		syswk->pokechg_mode = 1;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT) );

	case BOX2UI_BATTLEBOX_PARTY_STATUS:			// 10: ようすをみる
		syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
		syswk->get_tray    = syswk->tray;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_BATTLEBOX_PARTY_ITEM:				// 11: もちもの
		syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );

	case BOX2UI_BATTLEBOX_PARTY_CLOSE:			// 12: やめる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MENU_CLOSE );

	case CURSORMOVE_CANCEL:							// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
	case CURSORMOVE_CURSOR_ON:			// カーソル表示
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_BATTLEBOX_PARTY_BOXLIST ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}else if( pos == BOX2UI_BATTLEBOX_PARTY_BOXLIST ){
				BOX2MAIN_PokeInfoOff( syswk );
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN) );

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_BATTLEBOX_PARTY_GET, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_PARTY_GET );
			return PartyPokeGetKey( syswk, ret, BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN );
		}
	}

	return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「バトルボックス」手持ち選択終了処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BattleBoxPartyEnd( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_END );

	case 1:
		syswk->app->sub_seq = 0;
//		BOX2MAIN_PokeInfoPut( syswk, 0 );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_MAIN, BOX2UI_BATTLEBOX_MAIN_PARTY );
		return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
	}

	return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_END;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「バトルボックス」手持ちメニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MENU_CLOSE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BattleBoxPartyMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2BGWFRM_BoxListButtonOn( syswk->app );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ちポケモン入れ替え後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BATTLEBOX_PARTYPOKE_CHG_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BattleBoxPartyPokeChgEnd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	pos;

	work = syswk->app->vfunk.work;
	pos  = work->get_pos;

//	Snd_SePlay( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
//	BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_PARTY_GET );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
	return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
}


//============================================================================================
//	ポケモンをつれていく
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「ポケモンをつれていく」メイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYIN_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitTrayPokeTrg();
	if( ret != GFL_UI_TP_HIT_NONE ){
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 5 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return TrayPokeGetPartyIn( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_PARTYIN_MAIN) );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
			}
		}
	}

	ret = BOX2UI_PartyInMain( syswk );

	switch( ret ){
	case BOX2UI_PTIN_MAIN_NAME:				// 30: ボックス名
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );

	case BOX2UI_PTIN_MAIN_LEFT:				// 31: トレイ切り替え矢印・左
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTIN_MAIN_NAME );
		return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );

	case BOX2UI_PTIN_MAIN_RIGHT:			// 32: トレイ切り替え矢印・右
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTIN_MAIN_NAME );
		return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );

	case BOX2UI_PTIN_MAIN_RETURN1:		// 33: 戻る１
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_PTIN_MAIN_RETURN2:		// 34: 戻る２
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_PTIN_MAIN_TSURETEIKU:	// 35: つれていく
//			Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_PARTYIN_SELECT) );

	case BOX2UI_PTIN_MAIN_STATUS:			// 36: ようすをみる
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->get_tray    = syswk->tray;
		syswk->cur_rcv_pos = BOX2UI_PTIN_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_PTIN_MAIN_MARKING:		// 37: マーキング
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_MARKING_INIT) );

	case BOX2UI_PTIN_MAIN_FREE:				// 38: にがす
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_POKEFREE_INIT) );

	case BOX2UI_PTIN_MAIN_CLOSE:			// 39: とじる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_PARTYIN_MENU_CLOSE );

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_PTIN_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_PTIN_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_PTIN_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_PTIN_MAIN_TSURETEIKU &&
						pos != BOX2UI_PTIN_MAIN_STATUS &&
						pos != BOX2UI_PTIN_MAIN_MARKING &&
						pos != BOX2UI_PTIN_MAIN_FREE &&
						pos != BOX2UI_PTIN_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_PARTYIN_MAIN) );

	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_PTIN_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_PTIN_MAIN_TSURETEIKU &&
						pos != BOX2UI_PTIN_MAIN_STATUS &&
						pos != BOX2UI_PTIN_MAIN_MARKING &&
						pos != BOX2UI_PTIN_MAIN_FREE &&
						pos != BOX2UI_PTIN_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 5 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_PTIN_MAIN_TSURETEIKU, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTIN_MAIN_TSURETEIKU );
			return TrayPokeGetKey( syswk, ret, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );
		}
	}

	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「つれていく」メニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYIN_MENU_CLOSE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_PARTYIN_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「つれていく」メニュー
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYIN_SELECT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInSelect( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_PARTYIN_SELECT );

	case 1:
		// 手持ちに空きがあるか
		if( PokeParty_GetPokeCount( syswk->dat->pokeparty ) != TEMOTI_POKEMAX ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_PARTYIN_ACTION );
		}
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_ERR, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
	}

	return BOX2SEQ_MAINSEQ_PARTYIN_SELECT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「つれていく」動作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYIN_ACTION
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInAction( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_PartyPokeIconFrmInSetRight( syswk );
		BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYIN_ACTION );

	case 1:
		syswk->get_tray = syswk->tray;
		BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		PokeMoveWorkAlloc( syswk );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyInPokeMove, BOX2SEQ_MAINSEQ_PARTYIN_ACTION );

	case 2:
//		Snd_SePlay( SE_BOX2_POKE_PUT );
		BOX2MAIN_PokeDataMove( syswk );
		PokeMoveWorkFree( syswk );
		BOX2MAIN_PokeInfoOff( syswk );
		BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYIN_ACTION );

	case 3:
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		syswk->get_pos = BOX2MAIN_GETPOS_NONE;
		syswk->app->sub_seq = 0;
		return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
	}

	return BOX2SEQ_MAINSEQ_PARTYIN_ACTION;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「つれていく」ポケモン配置後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYIN_POKECHG_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInPokeChgEnd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	pos;

	work = syswk->app->vfunk.work;
	pos  = work->get_pos;

//	Snd_SePlay( SE_BOX2_POKE_PUT );

	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );

//	BOX2BMP_SysWinVanishEz( syswk->app, BOX2BMPWIN_ID_MSG4 );
/*
	if( syswk->get_pos == BOX2MAIN_GETPOS_NONE ){
		BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
	}else{
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
		pos = BOX2UI_PTIN_MAIN_TSURETEIKU;
	}
*/
	BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );

	if( syswk->get_pos != BOX2MAIN_GETPOS_NONE ){
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		pos = BOX2UI_PTIN_MAIN_TSURETEIKU;
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, pos );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}


//============================================================================================
//	ポケモンを預ける
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「ポケモンをあずける」手持ち選択メイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYOUT_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 5 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return PartyPokeGetDrop( syswk, ret );
//			return PartyPokeGet( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_PARTYOUT_MAIN) );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
			}
		}
	}

	ret = BOX2UI_PartyOutMain( syswk );

	switch( ret ){
	case BOX2UI_PTOUT_MAIN_RETURN1:		// 06: 戻る１
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_PTOUT_MAIN_RETURN2:		// 07: 戻る２
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_PTOUT_MAIN_AZUKERU:		// 08: あずける
//		Snd_SePlay( SE_BOX2_DECIDE );
//		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, BOX2SEQ_MAINSEQ_PARTYOUT_BUTTON );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT) );

	case BOX2UI_PTOUT_MAIN_STATUS:		// 09: ようすをみる
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->cur_rcv_pos = BOX2UI_PTOUT_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_PTOUT_MAIN_MARKING:		// 10: マーキング
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_MARKING_INIT) );

	case BOX2UI_PTOUT_MAIN_FREE:			// 11: にがす
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_POKEFREE_INIT) );

	case BOX2UI_PTOUT_MAIN_CLOSE:			// 12: とじる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_PARTYOUT_MENU_CLOSE );

	case CURSORMOVE_NO_MOVE_UP:				// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:			// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_LEFT:			// 十字キー左が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_RIGHT:		// 十字キー右が押されたが、移動なし
	case CURSORMOVE_NONE:							// 動作なし
		break;

	case CURSORMOVE_CURSOR_ON:				// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_PTOUT_MAIN_RETURN1 ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}else{
				if( pos != BOX2UI_PTOUT_MAIN_AZUKERU &&
						pos != BOX2UI_PTOUT_MAIN_STATUS &&
						pos != BOX2UI_PTOUT_MAIN_MARKING &&
						pos != BOX2UI_PTOUT_MAIN_FREE &&
						pos != BOX2UI_PTOUT_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	case CURSORMOVE_CURSOR_MOVE:			// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_PTOUT_MAIN_RETURN1 ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}else{
				if( pos != BOX2UI_PTOUT_MAIN_AZUKERU &&
						pos != BOX2UI_PTOUT_MAIN_STATUS &&
						pos != BOX2UI_PTOUT_MAIN_MARKING &&
						pos != BOX2UI_PTOUT_MAIN_FREE &&
						pos != BOX2UI_PTOUT_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_PARTYOUT_MAIN) );

	case CURSORMOVE_CANCEL:						// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 5 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_PTOUT_MAIN_AZUKERU, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTOUT_MAIN_AZUKERU );
			return PartyPokeGetKey( syswk, ret, BOX2SEQ_MAINSEQ_PARTYOUT_MAIN );
		}
	}

	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「あずける」メニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYOUT_MENU_CLOSE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_PARTYOUT_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「あずける」エラー復帰
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYOUT_ERR_RCV_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutErrRcvMain( BOX2_SYS_WORK * syswk )
{
	CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「あずける」配置選択初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutPutInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT );

	case 1:
		switch( PartyOutCheck(syswk) ){
		case 0:		// 預けられる
			syswk->poke_get_key = 1;
			BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeIconPriChg( syswk->app, BOX2OBJ_POKEICON_GET_POS, BOX2OBJ_POKEICON_PRI_CHG_GET );
			BOX2UI_PutHandCursor( syswk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
			syswk->app->sub_seq++;
			return VFuncSet( syswk, BOX2MAIN_VFuncPokeMoveGetKey, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT );
		case 1:		// 戦えるポケモンがいなくなる
			BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_ERR_RCV_MAIN;
			syswk->app->sub_seq = 0;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		case 2:		// メールを持っている
			BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_MAIL, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_ERR_RCV_MAIN;
			syswk->app->sub_seq = 0;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}
		break;

	case 2:
		BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT );

	case 3:
		BOX2UI_PartyOutBoxSelCursorMoveSet( syswk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncCursorMove, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT );

	case 4:
		GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_INIT, BOX2BMPWIN_ID_MSG3 );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_BOXSEL, 0 );
		syswk->app->sub_seq = 0;
		return BOX2SEQ_MAINSEQ_PARTYOUT_PUT_MAIN;
	}

	return BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「あずける」配置選択メイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYOUT_PUT_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutPutMain( BOX2_SYS_WORK * syswk )
{
	switch( CURSORMOVE_MainCont(syswk->app->cmwk) ){
	case BOX2UI_PTOUT_BOXSEL_NAME:		// 00: トレイ名
	case BOX2UI_PTOUT_BOXSEL_TRAY:		// 01: トレイ
		if( BOXDAT_GetEmptySpaceTray( syswk->dat->sv_box, syswk->tray ) == 0 ){
			BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_BOXMAX, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_ERR_RCV_PUT;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}else{
			int	pos, tray;
			tray = syswk->tray;
			pos  = 0;
			BOXDAT_GetEmptyTrayNumberAndPos( syswk->dat->sv_box, &tray, &pos );
			syswk->app->poke_put_key = pos;
			syswk->get_tray = syswk->tray;
			syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_TRAY;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );
		}
		break;

	case BOX2UI_PTOUT_BOXSEL_LEFT:		// 02: トレイ切り替え矢印・左
	case CURSORMOVE_NO_MOVE_LEFT:			// 十字キー左が押されたが、移動なし
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_MAIN );

	case BOX2UI_PTOUT_BOXSEL_RIGHT:		// 03: トレイ切り替え矢印・右
	case CURSORMOVE_NO_MOVE_RIGHT:		// 十字キー右が押されたが、移動なし
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_MAIN );


	case BOX2UI_PTOUT_BOXSEL_RETURN:	// 04: 戻る
	case CURSORMOVE_CANCEL:						// キャンセル
		syswk->app->poke_put_key = BOX2MAIN_GETPOS_NONE;
		syswk->get_tray = syswk->tray;
		syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END) );

	case CURSORMOVE_CURSOR_ON:				// カーソル表示
	case CURSORMOVE_CURSOR_MOVE:			// 移動
	case CURSORMOVE_NO_MOVE_UP:				// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:			// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:							// 動作なし
		break;
	}

	return BOX2SEQ_MAINSEQ_PARTYOUT_PUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「あずける」配置終了処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutPutEnd( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case SEQ_PTOUT_PUT_POKEMOVE:
		// キャンセル
		if( syswk->app->poke_put_key == BOX2MAIN_GETPOS_NONE ){
			BOX2UI_PartyOutBoxSelCursorMovePutSet( syswk, syswk->get_pos );
			syswk->app->sub_seq = SEQ_PTOUT_PUT_PTFRM_RET;
		// 実行
		}else{
			BOX2UI_PartyOutBoxSelCursorMovePutSet( syswk, syswk->app->poke_put_key );
			syswk->app->sub_seq = SEQ_PTOUT_PUT_ICON_PUT;
		}
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		return VFuncSet( syswk, BOX2MAIN_VFuncCursorMove, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );

	case SEQ_PTOUT_PUT_PTFRM_RET:
		GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_ICON_RET;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );

	case SEQ_PTOUT_PUT_ICON_RET:
		PokeMoveWorkAlloc( syswk );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_END;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMovePutKey, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE );

	case SEQ_PTOUT_PUT_PTFRM_IN:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_END;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );

	case SEQ_PTOUT_PUT_ICON_PUT:
		GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
		PokeMoveWorkAlloc( syswk );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_DATA_CHG;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMovePutKey2, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );

	case SEQ_PTOUT_PUT_DATA_CHG:
		{
			BOX2MAIN_POKEMOVE_WORK * work;
			work = syswk->app->vfunk.work;
//			Snd_SePlay( SE_BOX2_POKE_PUT );
			BOX2MAIN_PokeDataMove( syswk );
			BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );
			PokeMoveWorkFree( syswk );
		}
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
		BOX2MAIN_PokeInfoOff( syswk );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_PTFRM_IN;
		break;

	case SEQ_PTOUT_PUT_END:
		syswk->poke_get_key = 0;
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_MAIN, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
		syswk->app->sub_seq = 0;
		return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
	}

	return BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「あずける」配置エラー復帰
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYOUT_ERR_RCV_PUT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutErrRcvPut( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_INIT, BOX2BMPWIN_ID_MSG3 );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
	return BOX2SEQ_MAINSEQ_PARTYOUT_PUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「あずける」配置後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_PARTYOUT_POKECHG_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutPokeChgEnd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	pos;

	work = syswk->app->vfunk.work;
	pos  = work->get_pos;

//	Snd_SePlay( SE_BOX2_POKE_PUT );

	BOX2MAIN_PokeDataMove( syswk );
	BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );
	PokeMoveWorkFree( syswk );

//	BOX2BMP_SysWinVanishEz( syswk->app, BOX2BMPWIN_ID_MSG4 );

	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );

	if( syswk->get_pos == BOX2MAIN_GETPOS_NONE ){
//		BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
		CURSORMOVE_PosSet( syswk->app->cmwk, 0 );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );
	}else{
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTOUT_MAIN_AZUKERU );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	}

	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}


//============================================================================================
//	道具整理
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：道具整理メイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitTrayPokeTrg();
	if( ret != GFL_UI_TP_HIT_NONE ){
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return TrayItemGet( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
				u8	pos = syswk->get_pos;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				return MainSeq_ItemMenuClose( syswk );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_ITEM_MAIN;
			}
		}
	}

	ret = BOX2UI_BoxItemArrangeMain( syswk );

	switch( ret ){
	case BOX2UI_ITEM_MAIN_NAME:		// 30: ボックス名
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );

	case BOX2UI_ITEM_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_MAIN_NAME );
		return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEM_MAIN );

	case BOX2UI_ITEM_MAIN_RIGHT:		// 32: トレイ切り替え矢印・右
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_MAIN_NAME );
		return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEM_MAIN );

	case BOX2UI_ITEM_MAIN_PARTY:				// 33: 手持ちポケモン
//		Snd_SePlay( SE_BOX2_OPEN_PARTY_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_INIT) );
		break;
// 旧		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_BUTTON );

	case BOX2UI_ITEM_MAIN_RETURN1:			// 34: 戻る１
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_ITEM_MAIN_RETURN2:			// 35: 戻る２
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_ITEM_MAIN_MENU1:				// 36: いどうする
//		Snd_SePlay( SE_BOX2_OPEN_PARTY_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEMGET_INIT) );
/*
	case BOX2UI_ITEM_A_MAIN_CHANGE:	// 34: 持ち物整理
//		Snd_SePlay( SE_BOX2_OPEN_PARTY_TRAY );
		return YStatusButtonAnmSet( syswk, BOX2MAIN_WINFRM_MV_BTN, BOX2SEQ_MAINSEQ_ITEMARRANGE_MOVE_BUTTON );
*/

	case BOX2UI_ITEM_MAIN_MENU2:				// 37: バッグへ or もたせる
/*
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEMARRANGE_BAG_BUTTON );
*/
		if( syswk->app->get_item == ITEM_DUMMY_DATA ){
			syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
			syswk->cur_rcv_pos = BOX2UI_ITEM_MAIN_MENU2;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
		}else{
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
		}
		break;

	case BOX2UI_ITEM_MAIN_CLOSE:				// 38: とじる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ITEM_MENU_CLOSE );
//		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_CLOSE_BTN, BOX2SEQ_MAINSEQ_ITEMARRANGE_CLOSE_BUTTON );

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ITEM_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEM_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ITEM_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEM_MAIN );
		}
		break;

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_ITEM_MAIN_MENU1 &&
						pos != BOX2UI_ITEM_MAIN_MENU2 &&
						pos != BOX2UI_ITEM_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_ITEM_MAIN_MENU1 &&
						pos != BOX2UI_ITEM_MAIN_MENU2 &&
						pos != BOX2UI_ITEM_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_MAIN) );

	case CURSORMOVE_CANCEL:					// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			return TrayItemGetKey( syswk, ret );
		}
		break;
	}

	return BOX2SEQ_MAINSEQ_ITEM_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_MENU_CLOSE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2BGWFRM_TemochiButtonOn( syswk->app );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeMenuClose, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：バッグに戻した後の処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_BAGIN_ANM
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemBagInAnm( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEM_BAGIN_ANM;
	}

	BOX2SEQ_ItemModeMenuSet( syswk, ITEM_DUMMY_DATA );

	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_RET, BOX2BMPWIN_ID_MSG1 );
	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	}
	syswk->app->get_item = ITEM_DUMMY_DATA;
//	syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_SET;
//	return BOX2SEQ_MAINSEQ_TRGWAIT;
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：アイテムアイコン移動後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_RET
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeIconMoveRet( BOX2_SYS_WORK * syswk )
{
//	ItemMoveWorkFree( syswk );

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_MAIN_MENU2 );
	}else{
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_MAIN_MENU1 );
	}
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	return BOX2SEQ_MAINSEQ_ITEM_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：アイテム移動初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEMGET_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemGetInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			syswk->pokechg_mode = 1;
		}else{
			syswk->pokechg_mode = 0;
		}
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeMenuClose, BOX2SEQ_MAINSEQ_ITEMGET_INIT );

	case 1:
		syswk->app->sub_seq++;
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			BOX2BGWFRM_PartyPokeFrameRightMoveSet( syswk->app->wfrm );
		}else{
			BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
			BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
			BOX2OBJ_PartyPokeIconFrmInSet( syswk );
		}
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ITEMGET_INIT );

	case 2:
		syswk->app->sub_seq = 0;
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
		CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
		CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_STATUS );
		return BoxPartyItemGetKey( syswk, syswk->get_pos );
	}

	return BOX2SEQ_MAINSEQ_ITEMGET_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：アイテム移動メイン処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEMGET_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemGetMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	if( syswk->poke_get_key == 0 ){
		// 手持ち
		ret = BOX2UI_HitCheckTrgPartyPokeRight();
		if( ret != GFL_UI_TP_HIT_NONE ){
			ret += BOX2OBJ_POKEICON_TRAY_MAX;
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				BOX2MAIN_PokeInfoPut( syswk, ret );
				BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
				return BoxPartyItemGet( syswk, ret );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
			}
		}
		// ボックス
		ret = BOX2UI_HitTrayPokeTrg();
		if( ret != GFL_UI_TP_HIT_NONE ){
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				BOX2MAIN_PokeInfoPut( syswk, ret );
				BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
				return BoxPartyItemGet( syswk, ret );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
			}
		}
	}

	// キー・タッチチェック
	ret = BOX2UI_ArrangePartyPokeGetMain( syswk );

	switch( ret ){
	case BOX2UI_ARRANGE_PTGT_NAME:		// 36: ボックス名
		// 未取得
		if( syswk->poke_get_key == 0 ){
//		Snd_SePlay( SE_BOX2_DECIDE );
			BOX2MAIN_PokeInfoOff( syswk );
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_LEFT:		// 37: ボックス切り替え矢印（左）
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_NAME );
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		// 取得中
		}else{
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_RIGHT:		// 38: ボックス切り替え矢印（右）
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_NAME );
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		// 取得中
		}else{
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_RET:		// 41: 戻る
		if( syswk->poke_get_key == 0 ){
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEMGET_END) );
		}
		break;

	case CURSORMOVE_CANCEL:					// キャンセル
		if( syswk->poke_get_key == 1 ){
			return BoxPartyItemPutKey( syswk, BOX2MAIN_GETPOS_NONE );
		}else{
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEMGET_END) );
		}
		break;

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PTGT_NAME ){
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PTGT_NAME ){
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_BOXLIST:	// 39: ボックスリスト
	case BOX2UI_ARRANGE_PTGT_STATUS:	// 40: ステータス
	case CURSORMOVE_NO_MOVE_UP:				// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:			// 十字キー下が押されたが、移動なし
	case CURSORMOVE_CURSOR_ON:				// カーソル表示
	case CURSORMOVE_NONE:							// 動作なし
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		if( syswk->poke_get_key == 0 ){
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2OBJ_POKEICON_PUT_MAX ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
			}
		}
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEMGET_MAIN) );

	default:
		// 取得中
		if( syswk->poke_get_key == 1 ){
			return BoxPartyItemPutKey( syswk, ret );
		// 未取得
		}else{
			return BoxPartyItemGetKey( syswk, ret );
		}
		break;
	}

	return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：アイテム移動終了処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEMGET_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemGetEnd( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2MAIN_PokeInfoOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		if( syswk->pokechg_mode == 0 ){
			BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
		}else{
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			BOX2BGWFRM_PartyPokeFrameLeftMoveSet( syswk->app->wfrm );
		}
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ITEMGET_END );

	case 1:
		syswk->app->sub_seq = 0;
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
		if( syswk->pokechg_mode == 0 ){
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos >= BOX2OBJ_POKEICON_PUT_MAX ){
				pos = 0;
			}else if( pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				pos = 0;
			}
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_MAIN, pos );
			BOX2MAIN_PokeInfoPut( syswk, pos );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			return BOX2SEQ_MAINSEQ_ITEM_MAIN;
		}else{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos >= BOX2OBJ_POKEICON_PUT_MAX ){
				pos = 0;
			}else if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				pos = 0;
			}else{
				pos -= BOX2OBJ_POKEICON_TRAY_MAX;
			}
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_PARTY, pos );
			BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2BGWFRM_BoxListButtonOn( syswk->app );
			return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
		}
	}

	return BOX2SEQ_MAINSEQ_ITEMGET_END;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：持ち物交換
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_ICONMOVE_POKEADD
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemBoxPartyIconMovePokeAdd( BOX2_SYS_WORK * syswk )
{
	POKEMON_PASO_PARAM * ppp;
	BOX2MAIN_ITEMMOVE_WORK * mvwk;
	u16	item;

	BOX2OBJ_PokeCursorVanish( syswk, FALSE );

	mvwk = syswk->app->vfunk.work;

	// アイコンの移動先が元と同じ
	if( mvwk->put_pos == syswk->get_pos &&
			( syswk->get_tray == BOX2MAIN_GETPOS_NONE || syswk->get_tray == syswk->tray ) ){
		u32	set_pos = mvwk->set_pos;
		ItemMoveWorkFree( syswk );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		if( ITEM_CheckMail( syswk->app->get_item ) == TRUE && set_pos != syswk->get_pos ){
//			Snd_SePlay( SE_BOX2_WARNING );
			BOX2BMP_MailMoveErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_CHG_ERROR;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}else{
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
		}
	}

	BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );

	// 移動先にアイテムをセット
	item = BOX2MAIN_PokeParaGet( syswk, mvwk->put_pos, syswk->tray, ID_PARA_item, NULL );
	BOX2MAIN_PokeParaPut( syswk, mvwk->put_pos, syswk->tray, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, mvwk->put_pos );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, mvwk->put_pos,
			syswk->app->pokeicon_id[mvwk->put_pos] );
	}
	BOX2MAIN_PokeInfoPut( syswk, mvwk->put_pos );
	CURSORMOVE_PosSet( syswk->app->cmwk, mvwk->put_pos );

	// 移動元にアイテムをセット
	syswk->app->get_item = item;
	BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->get_tray, ID_PARA_item, syswk->app->get_item );
	if( syswk->get_tray == BOX2MAIN_GETPOS_NONE || syswk->get_tray == syswk->tray ){
		ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos,
				syswk->app->pokeicon_id[syswk->get_pos] );
		}
	}

	// 移動先がアイテムを持っていなかった
	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		syswk->get_pos = mvwk->put_pos;
		ItemMoveWorkFree( syswk );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

	// 移動元と移動先のトレイが違う
	if( syswk->get_tray != BOX2MAIN_GETPOS_NONE && syswk->get_tray != syswk->tray ){
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		syswk->get_pos = mvwk->put_pos;
		ItemMoveWorkFree( syswk );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}


	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );
//	Snd_SePlay( SE_BOX2_POKE_CATCH );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemIconChgTouch, BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_ICONMOVE_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：持ち物交換終了
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_ICONMOVE_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemBoxPartyIconMoveEnd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_ITEMMOVE_WORK * mvwk;
	u16	item;
	u16	get, set;

	mvwk = syswk->app->vfunk.work;

	BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );

	get = syswk->get_pos;
	set = mvwk->set_pos;

	syswk->get_pos = mvwk->put_pos;

	ItemMoveWorkFree( syswk );

	item = syswk->app->get_item;
	syswk->app->get_item = ITEM_DUMMY_DATA;

	if( ITEM_CheckMail( item ) == TRUE && set != get ){
//		Snd_SePlay( SE_BOX2_WARNING );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		BOX2BMP_MailMoveErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_CHG_ERROR;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}

	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	{
		u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
		if( pos < BOX2OBJ_POKEICON_PUT_MAX ){
			BOX2MAIN_PokeInfoPut( syswk, pos );
		}
	}

	return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：入れ替えエラーメッセージ表示後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_CHG_ERROR
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemBoxPartyChgError( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanishTrans( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：持ち物交換（キー操作）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_KEYGET_CHANGE_CHECK
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemKeyGetChangeCkack( BOX2_SYS_WORK * syswk )
{
	POKEMON_PASO_PARAM * ppp;
	u16	item;

	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->app->poke_put_key, FALSE );

	// 初期位置と同じ
	if( syswk->app->poke_put_key == syswk->app->get_item_init_pos &&
			( syswk->get_tray == BOX2MAIN_GETPOS_NONE || syswk->get_tray == syswk->tray ) ){
		syswk->app->get_item = ITEM_DUMMY_DATA;
//		BOX2MAIN_PokeInfoPut( syswk, syswk->app->poke_put_key );
//		BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
//		return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

	// 持っているアイテムを取得
	item = BOX2MAIN_PokeParaGet( syswk, syswk->app->poke_put_key, syswk->tray, ID_PARA_item, NULL );
	// アイテムを持たせる
	BOX2MAIN_PokeParaPut( syswk, syswk->app->poke_put_key, syswk->tray, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->app->poke_put_key );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, syswk->app->poke_put_key,
			syswk->app->pokeicon_id[syswk->app->poke_put_key] );
	}
	BOX2MAIN_PokeInfoPut( syswk, syswk->app->poke_put_key );

	// 取得したアイテムを元のポケモンに持たせる
	syswk->app->get_item = item;
	BOX2MAIN_PokeParaPut( syswk, syswk->app->get_item_init_pos, syswk->get_tray, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->app->get_item_init_pos );
	if( syswk->get_tray == BOX2MAIN_GETPOS_NONE || syswk->get_tray == syswk->tray ){
		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->app->get_item_init_pos,
				syswk->app->pokeicon_id[syswk->app->get_item_init_pos] );
		}
	}

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
//		BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
//	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

	syswk->poke_get_key = 1;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	BOX2OBJ_ItemIconPokePutHand( syswk->app, syswk->app->poke_put_key, BOX2MAIN_POKEMOVE_MODE_ALL );
	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeBoxPartyIconGetKey, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：持ち物整理・手持ちメイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemPartyMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return PartyItemGet( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
				u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				return MainSeq_ItemPartyMenuClose( syswk );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret-BOX2OBJ_POKEICON_TRAY_MAX );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
			}
		}
	}

	ret = BOX2UI_BoxItemArrangePartyMoveMain( syswk );

	switch( ret ){

	case BOX2UI_ITEM_PARTY_BOXLIST:		// 06: ボックスリスト
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END) );

	case BOX2UI_ITEM_PARTY_RETURN1:		// 07: 戻る１
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_ITEM_PARTY_RETURN2:		// 08: 戻る２
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_ITEM_PARTY_MENU1:			// 09: いどうする
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEMGET_INIT) );
/*
	case BOX2UI_ITEM_PARTY_BOXLIST:	//「いれかえ」
//		Snd_SePlay( SE_BOX2_DECIDE );
		return PartyButtonAnmSet( syswk, 0, BOX2SEQ_MAINSEQ_ITEM_CHG_BUTTON );
*/

	case BOX2UI_ITEM_PARTY_MENU2:			// 10: バッグへ or もたせる
		if( syswk->app->get_item == ITEM_DUMMY_DATA ){
			syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
			syswk->cur_rcv_pos = BOX2UI_ITEM_PARTY_MENU2;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
		}else{
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
		}
		break;

	case BOX2UI_ITEM_PARTY_CLOSE:			// 11: とじる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ITEM_PARTY_MENU_CLOSE );

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
	case CURSORMOVE_CURSOR_ON:			// カーソル表示
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_PARTY_BOXLIST ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}else if( pos == BOX2UI_ARRANGE_PARTY_BOXLIST ){
				BOX2MAIN_PokeInfoOff( syswk );
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN) );

	case CURSORMOVE_CANCEL:					// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			return PartyItemGetKey( syswk, ret );
		}
	}

	return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_PARTY_MENU_CLOSE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemPartyMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2BGWFRM_BoxListButtonOn( syswk->app );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeMenuClose, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ちポケモンのアイテム移動開始
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_RET
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemPartyIconMoveRet( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_ITEMMOVE_WORK * mvwk;

	mvwk = syswk->app->vfunk.work;

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		ItemMoveWorkFree( syswk );
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_SET, BOX2BMPWIN_ID_MSG1 );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU2 );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}

	if( mvwk->put_pos == syswk->get_pos ){
		u32	set_pos = mvwk->set_pos;
		ItemMoveWorkFree( syswk );
		if( ITEM_CheckMail( syswk->app->get_item ) == TRUE && set_pos != syswk->get_pos ){
			BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
			BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_MAIL_MOVE, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
		}
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_INIT, BOX2BMPWIN_ID_MSG1 );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU1 );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}

	if( ITEM_CheckMail( syswk->app->get_item ) == TRUE ){
		u32	set_pos = mvwk->set_pos;
		ItemMoveWorkFree( syswk );
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_MAIL_MOVE, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
	}

	BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );

	// アウトライン消去
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	// アイテムアイコン縮小
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );

	return BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_POKEADD;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ちポケモンのアイテム移動
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_POKEADD
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemPartyIconMovePokeAdd( BOX2_SYS_WORK * syswk )
{
	POKEMON_PASO_PARAM * ppp;
	BOX2MAIN_ITEMMOVE_WORK * mvwk;
	u16	item;

	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_POKEADD;
	}

	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );

	mvwk = syswk->app->vfunk.work;

	// 移動先にアイテムをセット
	item = BOX2MAIN_PokeParaGet( syswk, mvwk->put_pos, syswk->tray, ID_PARA_item, NULL );
	BOX2MAIN_PokeParaPut( syswk, mvwk->put_pos, syswk->tray, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, mvwk->put_pos );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, mvwk->put_pos,
			syswk->app->pokeicon_id[mvwk->put_pos] );
	}
	BOX2MAIN_PokeInfoPut( syswk, mvwk->put_pos );

	// 移動元にアイテムをセット
	syswk->app->get_item = item;
	BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->get_tray, ID_PARA_item, syswk->app->get_item );
	if( syswk->get_tray == BOX2MAIN_GETPOS_NONE || syswk->get_tray == syswk->tray ){
		ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos,
				syswk->app->pokeicon_id[syswk->get_pos] );
		}
	}

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		syswk->get_pos = mvwk->put_pos;
		ItemMoveWorkFree( syswk );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
//			return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_POKEADD_END );
//		return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_POKEADD_END;
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}

//	Snd_SePlay( SE_BOX2_POKE_CATCH );

	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncItemIconPartyChange, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_CHANGE) );
//	return VFuncSet( syswk, BOX2MAIN_VFuncItemIconPartyChange, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：手持ちポケモンのアイテム交換
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_CHANGE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemPartyIconMoveChange( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
//		Snd_SePlay( SE_BOX2_POKE_PUT );
		{
			BOX2MAIN_ITEMMOVE_WORK * mvwk = syswk->app->vfunk.work;
			BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );
		}
		// アウトライン消去
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		// アイテムアイコン縮小
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
/*
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			syswk->get_pos = mvwk->put_pos;
		}
*/
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
		ItemMoveWorkFree( syswk );
		syswk->app->sub_seq++;
		break;

	case 1:
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
			break;
		}
//		return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_CHANGE_END;
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_CHANGE;
}


//============================================================================================
//	寝かせる
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「寝かせる」メイン処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_SLEEP_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SleepMain( BOX2_SYS_WORK * syswk )
{

	u32	ret;
	u8	old;

	old = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = BOX2UI_SleepMain( syswk );

	switch( ret ){
	case BOX2UI_SLEEP_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_SLEEP_MAIN_NAME );
		return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_SLEEP_MAIN );

	case BOX2UI_SLEEP_MAIN_RIGHT:		// 32: トレイ切り替え矢印・右
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
//		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_SLEEP_MAIN_NAME );
		return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_SLEEP_MAIN );

	case BOX2UI_SLEEP_MAIN_RETURN:	// 33: 戻る
//		Snd_SePlay( SE_BOX2_DECIDE );
//		BOX2MAIN_PokeInfoOff( syswk );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		syswk->dat->retTray = BOX_RET_SEL_NONE;
		syswk->dat->retPoke = BOX_RET_SEL_NONE;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_SLEEP_MAIN_SET:			// 34: ねかせる
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_SLEEP_MENU_SET) );

	case BOX2UI_SLEEP_MAIN_CLOSE:		// 35: やめる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_SLEEP_MENU_CLOSE );

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_SLEEP_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_SLEEP_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_SLEEP_MAIN_NAME ){
//			Snd_SePlay( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_SLEEP_MAIN );
		}
		break;

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_SLEEP_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_SLEEP_MAIN_SET &&
						pos != BOX2UI_SLEEP_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_SLEEP_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_SLEEP_MAIN_SET &&
						pos != BOX2UI_SLEEP_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_SLEEP_MAIN) );

	case CURSORMOVE_CANCEL:					// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		syswk->dat->retTray = BOX_RET_SEL_NONE;
		syswk->dat->retPoke = BOX_RET_SEL_NONE;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case BOX2UI_SLEEP_MAIN_NAME:		// 30: ボックス名
	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			syswk->get_pos = ret;
			BOX2BMP_MenuStrPrint( syswk, SleepMenuStrTbl, 2 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_PokeCursorAdd( syswk );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_SLEEP_MAIN_SET );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				return MainSeq_SleepPokeSelect( syswk );
			}else{
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
				BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
				return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_SLEEP_POKE_SELECT );
			}
		}else{
			BOX2MAIN_PokeInfoPut( syswk, ret );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, ret, ret );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_SLEEP_MAIN) );
			}
		}
	}

	return BOX2SEQ_MAINSEQ_SLEEP_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：寝かせるポケモン選択
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_SLEEP_POKE_SELECT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SleepPokeSelect( BOX2_SYS_WORK * syswk )
{
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
	return BOX2SEQ_MAINSEQ_SLEEP_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「ねかせる」選択後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_SLEEP_MENU_SET
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SleepMenuSet( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_SLEEP_MENU_SET );

	case 1:
		if( BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_tamago_flag, NULL ) != 0 ){
			BOX2BMP_PutPokeMoveErrMsg( syswk, BOX2MAIN_ERR_CODE_EGG, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
		}
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		BOX2BMP_SleepSelectMsgPut( syswk );
		syswk->app->sub_seq = 0;
		return YesNoSet( syswk, YESNO_ID_SLEEP );
	}

	return BOX2SEQ_MAINSEQ_SLEEP_MENU_SET;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_SLEEP_MENU_CLOSE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SleepMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_SLEEP_MAIN) );
}


//============================================================================================
//	サブプロセス
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ステータス画面呼び出し
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_STATUS_CALL
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_StatusCall( BOX2_SYS_WORK * syswk )
{
	return SubProcSet( syswk, SUB_PROC_TYPE_POKESTATUS );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「もちもの」メニューチェック
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemMenuCheck( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		if( BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_tamago_flag, NULL ) == 0 ){
			u32 item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );
			// 何も持っていない
			if( item == ITEM_DUMMY_DATA ){
//				syswk->cur_rcv_pos = BOX2UI_ARRANGE_MAIN_ITEM;
				return SubProcSet( syswk, SUB_PROC_TYPE_BAG );
			}
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			// メールを持っている
			if( ITEM_CheckMail( item ) == TRUE ){
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				syswk->app->sub_seq = 1;
				return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
			}
			BOX2BGWFRM_PokeMenuOff( syswk->app->wfrm );
			BOX2BMP_ItemGetCheckMsgPut( syswk, item, BOX2BMPWIN_ID_MSG1 );
			if( syswk->dat->callMode == BOX_MODE_ITEM ){
				return YesNoSet( syswk, YESNO_ID_ITEM_RET_BAG );
			}else{
				return YesNoSet( syswk, YESNO_ID_ITEMGET );
			}
		}
		// タマゴ
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq = 2;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );

	case 1:		// メールを持っている
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2BMP_MailGetErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );

	case 2:		// タマゴ
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_EGG, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
	}

	return BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ステータス画面からの復帰
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_STATUS_RCV
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_StatusRcv( BOX2_SYS_WORK * syswk )
{
	int	next;

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:		// あずける
		next = RcvStatus_ModeAzukeru( syswk );
		break;
	case BOX_MODE_TURETEIKU:	// つれていく
		next = RcvStatus_ModeTureteiku( syswk );
		break;
	case BOX_MODE_SEIRI:			// せいり
		next = RcvStatus_ModeSeiri( syswk );
		break;
/*
	case BOX_MODE_ITEM:				// どうぐせいり
		next = RcvStatus_ModeItem( syswk );
		break;
*/
	case BOX_MODE_BATTLE:			// バトルボックス
		next = RcvStatus_ModeBattleBox( syswk );
		break;
	}

	return FadeInSet( syswk, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：文字入力画面からの復帰
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_STRIN_RCV
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_StrInRcv( BOX2_SYS_WORK * syswk )
{
	int	next;

	switch( syswk->dat->callMode ){
	case BOX_MODE_TURETEIKU:	// つれていく
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTIN_MAIN, BOX2UI_PTIN_MAIN_NAME );
		syswk->app->old_cur_pos = BOX2UI_PTIN_MAIN_NAME;
		next = BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
		break;

	case BOX_MODE_SEIRI:			// せいり
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_NAME );
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_MAIN_NAME;
		next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		break;

	case BOX_MODE_ITEM:				// どうぐせいり
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_MAIN, BOX2UI_ITEM_MAIN_NAME );
		syswk->app->old_cur_pos = BOX2UI_ITEM_MAIN_NAME;
		next = BOX2SEQ_MAINSEQ_ITEM_MAIN;
		break;
	}

	return FadeInSet( syswk, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：バッグ画面からの復帰
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BAG_RCV
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BagRcv( BOX2_SYS_WORK * syswk )
{
	int	next;

	switch( syswk->dat->callMode ){
	case BOX_MODE_SEIRI:			// せいり
		next = RcvBag_ModeSeiri( syswk );
		break;
	case BOX_MODE_ITEM:				// どうぐせいり
		next = RcvBag_ModeItem( syswk );
		break;
	case BOX_MODE_BATTLE:			// バトルボックス
		next = RcvBag_ModeBattleBox( syswk );
		break;
	}

	return FadeInSet( syswk, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：道具をもたせたメッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEMSET_MSG
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemSetMsg( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_ItemSetMsgPut( syswk, syswk->subRet, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
}


//============================================================================================
//	マーキング
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：マーキング処理初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_MARKING_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MarkingInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_MARKING_INIT );

	case 1:
		syswk->app->pokeMark = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_mark, NULL );
//		BOX2BMP_MarkingButtonPut( syswk->app );
		BOX2MAIN_MainDispMarkingChange( syswk, syswk->app->pokeMark );
		BOX2BGWFRM_MarkingFrameInSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncMarkingFrameMove, BOX2SEQ_MAINSEQ_MARKING_INIT );

	case 2:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_MARKING, 0 );

		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );

		BOX2BMP_MarkingMsgPut( syswk, BOX2BMPWIN_ID_MSG1 );
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_MARKING_MAIN );
	}

	return BOX2SEQ_MAINSEQ_MARKING_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：マーキング処理メイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_MARKING_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MarkingMain( BOX2_SYS_WORK * syswk )
{
	switch( CURSORMOVE_MainCont(syswk->app->cmwk) ){
	case BOX2UI_MARKING_MARK1:		// 00: ●
//		Snd_SePlay( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 0 );
		break;
	case BOX2UI_MARKING_MARK2:		// 01: ▲
//		Snd_SePlay( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 1 );
		break;
	case BOX2UI_MARKING_MARK3:		// 02: ■
//		Snd_SePlay( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 2 );
		break;
	case BOX2UI_MARKING_MARK4:		// 03: ハート
//		Snd_SePlay( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 3 );
		break;
	case BOX2UI_MARKING_MARK5:		// 04: ★
//		Snd_SePlay( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 4 );
		break;
	case BOX2UI_MARKING_MARK6:		// 05: ◆
//		Snd_SePlay( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 5 );
		break;

	case BOX2UI_MARKING_ENTER:		// 06:「けってい」
		{
			u8	mark = syswk->app->pokeMark;
			BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_mark, mark );
			BOX2MAIN_SubDispMarkingChange( syswk, mark );
		}
//		Snd_SePlay( SE_BOX2_DECIDE );
		return MarkingButtonAnmSet( syswk, 0, BOX2SEQ_MAINSEQ_MARKING_END );

	case BOX2UI_MARKING_CANCEL:	// 07:「やめる」
	case CURSORMOVE_CANCEL:			// キャンセル
//		Snd_SePlay( SE_BOX2_CANCEL );
		return MarkingButtonAnmSet( syswk, 1, BOX2SEQ_MAINSEQ_MARKING_END );

	case CURSORMOVE_CURSOR_MOVE:	// 移動
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_MARKING_CURMOVE_RET );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;
	}

	return BOX2SEQ_MAINSEQ_MARKING_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：マーキング処理終了
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_MARKING_END
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MarkingEnd( BOX2_SYS_WORK * syswk )
{
	// キー操作設定
	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_MAIN, BOX2UI_PTOUT_MAIN_MARKING );
		break;
	case BOX_MODE_TURETEIKU:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTIN_MAIN, BOX2UI_PTIN_MAIN_MARKING );
		break;
	case BOX_MODE_SEIRI:
	default:
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_MARKING );
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_MARKING );
		}
		break;
	}
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );

/*
	if( syswk->box_mv_flg != 0 ){
		BOX2BMP_BoxMoveNameWrite( syswk, 0 );
	}
*/

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	}
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );

	BOX2BGWFRM_MarkingFrameOutSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncMarkingFrameMove, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：マーキング処理カーソル移動後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_MARKING_CURMOVE_RET
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MarkingCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
	return BOX2SEQ_MAINSEQ_MARKING_MAIN;
}


//============================================================================================
//	逃がす
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ポケモンを逃がす開始
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_POKEFREE_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PokeFreeInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_POKEFREE_INIT );

	case 1:
		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			POKEMON_PARAM * pp;
			u16	item;
			u32	pos;

			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
			BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );

			pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;

			// 手持ち１
			if( BOX2MAIN_BattlePokeCheck( syswk, pos ) == FALSE ){
//				Snd_SePlay( SE_BOX2_WARNING );
				BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
				syswk->app->poke_free_err = 1;
				syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
				return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
			}
			// メール
			pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos );
			item = PP_Get( pp, ID_PARA_item, NULL );
			if( ITEM_CheckMail( item ) == TRUE ){
//				Snd_SePlay( SE_BOX2_WARNING );
				BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_MAIL, BOX2BMPWIN_ID_MSG1 );
				syswk->app->poke_free_err = 1;
				syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
				return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
			}
		}else{
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		}
		{
			POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
			// タマゴ
			if( PPP_Get( ppp, ID_PARA_tamago_flag, NULL ) != 0 ){
//				Snd_SePlay( SE_BOX2_WARNING );
				BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_EGG, BOX2BMPWIN_ID_MSG1 );
				syswk->app->poke_free_err = 1;
				syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
				return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
			}
		}
		BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_CHECK, BOX2BMPWIN_ID_MSG1 );
		return ChangeSequence( syswk, YesNoSet(syswk,YESNO_ID_POKEFREE) );
	}

	return BOX2SEQ_MAINSEQ_POKEFREE_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ポケモンを逃がすアニメ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_POKEFREE_ANM
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PokeFreeAnm( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeFreeWazaCheck( syswk );

	if( BOX2OBJ_PokeIconFreeMove( syswk->app->seqwk ) == FALSE ){
		BOX2MAIN_POKEFREE_WORK * wk;
		u8	waza_flg;
		wk = syswk->app->seqwk;
		waza_flg = wk->check_flg;
		if( waza_flg != 0 ){
			// 戻す処理へ
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_POKEFREE_ERROR );
		}else{
			// 消す処理へ
			BOX2OBJ_PokeIconFreeEnd( syswk->app->seqwk );
			BOX2MAIN_PokeFreeExit( syswk );
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_POKEFREE_ENTER );
		}
	}
	return BOX2SEQ_MAINSEQ_POKEFREE_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ポケモンを逃がす実行処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_POKEFREE_ENTER
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PokeFreeEnter( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ENTER, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_POKEFREE_ENTER;
		syswk->app->sub_seq++;
		return BOX2SEQ_MAINSEQ_TRGWAIT;

	case 1:
		BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_BY, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_POKEFREE_ENTER;
		syswk->app->sub_seq++;
		return BOX2SEQ_MAINSEQ_TRGWAIT;

	case 2:
		BOX2MAIN_PokeDataClear( syswk, syswk->tray, syswk->get_pos );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2MAIN_PokeInfoOff( syswk );

		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			// トレイアイコン更新
			BOX2OBJ_TrayIconCgxMake( syswk, syswk->tray );
			BOX2OBJ_TrayIconCgxTrans( syswk, syswk->tray );
			if( syswk->dat->callMode == BOX_MODE_TURETEIKU ){
/*
				BOX2BMP_PokeSelectMsgPut(
					syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
*/
				syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
			}else{
				syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
				BOX2BGWFRM_TemochiButtonOn( syswk->app );
			}

			CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

			BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );

			syswk->get_pos = BOX2MAIN_GETPOS_NONE;

			syswk->app->sub_seq = 0;
			return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, syswk->next_seq );
		}else{
			PokeMoveWorkAlloc( syswk );
			syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
			syswk->app->sub_seq++;
			return VFuncSet( syswk, BOX2MAIN_VFuncPartyPokeFreeSort, BOX2SEQ_MAINSEQ_POKEFREE_ENTER );
		}

	case 3:
		syswk->app->sub_seq = 0;
		PokeMoveWorkFree( syswk );
/*
		if( syswk->box_mv_flg != 0 ){
			BOX2BMP_BoxMoveNameWrite( syswk, 0 );
		}
*/
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		syswk->get_pos = BOX2MAIN_GETPOS_NONE;
		if( syswk->dat->callMode == BOX_MODE_AZUKERU ){
			BOX2BMP_PokeSelectMsgPut(
				syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG1 );
			return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
		}
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
	}

	return BOX2SEQ_MAINSEQ_POKEFREE_ENTER;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ポケモンを逃がす失敗処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_POKEFREE_ERROR
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PokeFreeError( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		if( BOX2OBJ_PokeIconFreeErrorMove( syswk->app->seqwk ) == FALSE ){
			BOX2OBJ_PokeIconFreeErrorEnd( syswk->app->seqwk );
			BOX2MAIN_PokeFreeExit( syswk );
			BOX2OBJ_PokeCursorAdd( syswk );
			BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_RETURN, BOX2BMPWIN_ID_MSG1 );
//			BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );
			syswk->next_seq = BOX2SEQ_MAINSEQ_POKEFREE_ERROR;
			syswk->app->sub_seq++;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}
		break;

	case 1:
		BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_FEAR, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_POKEFREE_ERROR;
		syswk->app->sub_seq++;
		return BOX2SEQ_MAINSEQ_TRGWAIT;

	case 2:
		syswk->app->sub_seq = 0;
		return BoxArg_PokeFreeNo( syswk );
	}

	return BOX2SEQ_MAINSEQ_POKEFREE_ERROR;
}


//============================================================================================
//	ボックステーマ変更
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックステーマ変更初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		syswk->app->sub_seq++;
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		// メニュー
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
			BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
			BOX2MAIN_ResetTouchBar( syswk );
			BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
			return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );
		}
		// トレイフレーム
		if( BOX2BGWFRM_CheckBoxMoveFrm(syswk->app->wfrm) == TRUE ){
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
			BOX2BGWFRM_TemochiButtonOff( syswk->app );
			return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );
		}
		// 手持ちフレーム
		if( BOX2BGWFRM_CheckPartyPokeFrameRight(syswk->app->wfrm) == TRUE ){
			BOX2BGWFRM_BoxListButtonOff( syswk->app );
			BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
			return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );
		}
		BOX2BGWFRM_TemochiButtonOff( syswk->app );

	case 1:
		BOX2BMP_MenuStrPrint( syswk, ThemaMenuStrTbl, 4 );
		BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );

	case 2:
		syswk->app->sub_seq = 0;
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2BMP_BoxThemaMsgPut( syswk, BOX2BMP_MSGID_THEMA_INIT, BOX2BMPWIN_ID_MSG1 );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BOXTHEMA_CHG, 0 );
		return BOX2SEQ_MAINSEQ_BOXTHEMA_MAIN;
	}

	return BOX2SEQ_MAINSEQ_BOXTHEMA_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックステーマ変更メイン処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaMain( BOX2_SYS_WORK * syswk )
{
	switch( CURSORMOVE_MainCont(syswk->app->cmwk) ){
	case BOX2UI_BOXTHEMA_MENU1:			// 00: ジャンプする
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_INIT) );

	case BOX2UI_BOXTHEMA_MENU2:			// 01: かべがみ
		syswk->app->wp_menu = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );

	case BOX2UI_BOXTHEMA_MENU3:			// 02: なまえ
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_BOXTHEMA_NAMEIN_CALL );

	case BOX2UI_BOXTHEMA_MENU4:			// 03: やめる
	case CURSORMOVE_CANCEL:					// キャンセル
//		Snd_SePlay( SE_BOX2_CANCEL );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT) );

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_MAIN) );
	}

	return BOX2SEQ_MAINSEQ_BOXTHEMA_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックステーマ変更終了処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaExit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT );

	case 1:
		syswk->app->sub_seq = 0;
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		if( syswk->dat->callMode == BOX_MODE_SEIRI ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_NAME );
			return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}else if( syswk->dat->callMode == BOX_MODE_TURETEIKU ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTIN_MAIN, BOX2UI_PTIN_MAIN_NAME );
			return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
		}else if( syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_MAIN, BOX2UI_ITEM_MAIN_NAME );
			return BOX2SEQ_MAINSEQ_ITEM_MAIN;
		}
	}

	return BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス切り替え初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaJumpInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_INIT );

	case 1:
		syswk->box_mv_pos = syswk->tray;	// 現在参照しているボックス
		syswk->app->sub_seq++;
		return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_INIT );

	case 2:
		syswk->app->sub_seq = 0;
		BOX2OBJ_ChangeTrayName( syswk, 0, TRUE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BOXJUMP_MAIN, BOX2UI_BOXJUMP_TRAY2 );
		syswk->app->old_cur_pos = BOX2UI_BOXJUMP_TRAY2;
		BOX2BMP_BoxThemaMsgPut( syswk, BOX2BMP_MSGID_THEMA_JUMP, BOX2BMPWIN_ID_MSG2 );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		return BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN;
	}

	return BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス切り替えメイン処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaJumpMain( BOX2_SYS_WORK * syswk )
{
	u32	x, y;
	u32	ret;

	// タッチトレイスクロールチェック
	if( BOX2UI_HitCheckTrayScrollStart( &x, &y ) == TRUE ){
		syswk->app->tpy = y;
		syswk->next_seq = BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN;
		CreateTrayIconScrollWork( syswk );
		return BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH;
	}

	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	switch( ret ){
	case BOX2UI_BOXJUMP_TRAY1:		// 00: トレイアイコン
		BOX2MAIN_InitTrayIconScroll( syswk, -1 );
		return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDownJump, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN );

	case BOX2UI_BOXJUMP_TRAY6:		// 05: トレイアイコン
		BOX2MAIN_InitTrayIconScroll( syswk, 1 );
		return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUpJump, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN );

	case BOX2UI_BOXJUMP_TRAY2:		// 01: トレイアイコン
	case BOX2UI_BOXJUMP_TRAY3:		// 02: トレイアイコン
	case BOX2UI_BOXJUMP_TRAY4:		// 03: トレイアイコン
	case BOX2UI_BOXJUMP_TRAY5:		// 04: トレイアイコン
		syswk->app->chg_tray_pos = syswk->box_mv_pos + ret - BOX2UI_BOXJUMP_TRAY2;
		if( syswk->app->chg_tray_pos >= syswk->trayMax ){
			syswk->app->chg_tray_pos -= syswk->trayMax;
		}
		if( syswk->app->chg_tray_pos != syswk->tray ){
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
			return BoxMoveTrayScrollSet( syswk, 1 );
		}
		break;

	case BOX2UI_BOXJUMP_RET:			// 06: 戻る
	case CURSORMOVE_CANCEL:					// キャンセル
//		Snd_SePlay( SE_BOX2_CANCEL );
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_EXIT) );

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN) );

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_BOXJUMP_TRAY2 ){
			BOX2MAIN_InitTrayIconScroll( syswk, -1 );
			return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDownJump, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_BOXJUMP_TRAY5 ){
			BOX2MAIN_InitTrayIconScroll( syswk, 1 );
			return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUpJump, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN );
		}
		break;
	}

	return BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス切り替え終了処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_EXIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaJumpExit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG2 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		syswk->app->sub_seq++;
		return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_EXIT );

	case 1:
		syswk->app->sub_seq = 0;
//		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
//		BOX2BGWFRM_TemochiButtonOn( syswk->app );
//		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_NAME );
//		return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;

		if( syswk->dat->callMode == BOX_MODE_SEIRI ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_NAME );
			return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}else if( syswk->dat->callMode == BOX_MODE_TURETEIKU ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTIN_MAIN, BOX2UI_PTIN_MAIN_NAME );
			return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
		}else if( syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_MAIN, BOX2UI_ITEM_MAIN_NAME );
			return BOX2SEQ_MAINSEQ_ITEM_MAIN;
		}
	}

	return BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_EXIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス切り替え後
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_RET
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaJumpRet( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_ChangeTrayName( syswk, CURSORMOVE_PosGet(syswk->app->cmwk)-BOX2UI_BOXJUMP_TRAY2, TRUE );
	return BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：壁紙選択初期処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaWallPaperInit( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT );

	case 1:
		if( syswk->app->wp_menu == 0 ){
			BOX2BMP_MenuStrPrint( syswk, WallPaperMenuStrTbl, 5 );
		}else if( syswk->app->wp_menu == 1 ){
			BOX2BMP_MenuStrPrint( syswk, WPLabel1MenuStrTbl, 5 );
		}else if( syswk->app->wp_menu == 2 ){
			BOX2BMP_MenuStrPrint( syswk, WPLabel2MenuStrTbl, 5 );
		}else if( syswk->app->wp_menu == 3 ){
			BOX2BMP_MenuStrPrint( syswk, WPLabel3MenuStrTbl, 5 );
		}else if( syswk->app->wp_menu == 4 ){
			BOX2BMP_MenuStrPrint( syswk, WPLabel4MenuStrTbl, 5 );
		}
		BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT );

	case 2:
		syswk->app->sub_seq = 0;
		if( syswk->app->wp_menu == 0 ){
			BOX2BMP_BoxThemaMsgPut( syswk, BOX2BMP_MSGID_THEMA_MES, BOX2BMPWIN_ID_MSG1 );
		}else{
			BOX2BMP_BoxThemaMsgPut( syswk, BOX2BMP_MSGID_THEMA_WALL, BOX2BMPWIN_ID_MSG1 );
		}
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_WALLPAPER_CHG, 0 );
		return BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_MAIN;

	}

	return BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：壁紙選択メイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_MAIN
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaWallPaperMain( BOX2_SYS_WORK * syswk )
{
	switch( CURSORMOVE_MainCont(syswk->app->cmwk) ){
	case BOX2UI_WALLPAPER_MENU1:			// 00: メニュー１
		if( syswk->app->wp_menu == 0 ){
			syswk->app->wp_menu = 1;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		return SetWallPaperChange( syswk, 0 );

	case BOX2UI_WALLPAPER_MENU2:			// 01: メニュー２
		if( syswk->app->wp_menu == 0 ){
			syswk->app->wp_menu = 2;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		return SetWallPaperChange( syswk, 1 );

	case BOX2UI_WALLPAPER_MENU3:			// 02: メニュー３
		if( syswk->app->wp_menu == 0 ){
			syswk->app->wp_menu = 3;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		return SetWallPaperChange( syswk, 2 );

	case BOX2UI_WALLPAPER_MENU4:			// 03: メニュー４
		if( syswk->app->wp_menu == 0 ){
			syswk->app->wp_menu = 4;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		return SetWallPaperChange( syswk, 3 );

	case BOX2UI_WALLPAPER_MENU5:			// 04: やめる
	case CURSORMOVE_CANCEL:						// キャンセル
//		Snd_SePlay( SE_BOX2_CANCEL );
		if( syswk->app->wp_menu == 0 ){
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT) );
		}
		syswk->app->wp_menu = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );

	case CURSORMOVE_CURSOR_MOVE:			// 移動
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_MAIN) );
	}

	return BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：壁紙変更
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_CHANGE
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaWallPaperChange( BOX2_SYS_WORK * syswk )
{
	switch( syswk->app->sub_seq ){
	case 0:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		PaletteWorkSet_VramCopy( syswk->app->pfd, FADE_MAIN_BG, 0, FADE_PAL_ALL_SIZE );
		PaletteFadeReq( syswk->app->pfd, PF_BIT_MAIN_BG, 0xc000, 0, 0, 16, 0x7fff, GFUser_VIntr_GetTCBSYS() );
		syswk->next_seq = BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_CHANGE;
		syswk->app->sub_seq++;
		return BOX2SEQ_MAINSEQ_PALETTE_FADE;

	case 1:
		BOX2MAIN_WallPaperChange( syswk, syswk->app->wallpaper_pos );
		BOXDAT_SetWallPaperNumber( syswk->dat->sv_box, syswk->tray, syswk->app->wallpaper_pos );
		syswk->app->sub_seq++;
		break;

	case 2:
		PaletteFadeReq( syswk->app->pfd, PF_BIT_MAIN_BG, 0xc000, 0, 16, 0, 0x7fff, GFUser_VIntr_GetTCBSYS() );
		syswk->next_seq = BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_CHANGE;
		syswk->app->sub_seq++;
		return BOX2SEQ_MAINSEQ_PALETTE_FADE;

	case 3:
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
		syswk->app->sub_seq = 0;
		return BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_MAIN;
	}

	return BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_CHANGE;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：文字入力画面呼び出し
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXTHEMA_NAMEIN_CALL
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaNameInCall( BOX2_SYS_WORK * syswk )
{
	return SubProcSet( syswk, SUB_PROC_TYPE_NAMEIN );
}


//============================================================================================
//	終了処理
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス終了「はい・いいえ」セット（ボタン）
 *
 * @param		syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXEND_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxEndInit( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BMP_BoxEndMsgPut( syswk, BOX2BMPWIN_ID_MSG1, TRUE );
	BoxEndPassiveSet( syswk );
	return YesNoSet( syswk, YESNO_ID_BOX_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス終了「はい・いいえ」セット（キャンセル）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxEndCancelInit( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BMP_BoxEndMsgPut( syswk, BOX2BMPWIN_ID_MSG1, FALSE );
	BoxEndPassiveSet( syswk );
	return YesNoSet( syswk, YESNO_ID_BOX_END_CANCEL );
}


//============================================================================================
//============================================================================================
//	その他
//============================================================================================
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーケンス切り替え
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		次のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int ChangeSequence( BOX2_SYS_WORK * syswk, int next )
{
	syswk->app->sub_seq = 0;
	return next;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードイン設定
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int FadeInSet( BOX2_SYS_WORK * syswk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_BOX_APP );

	syswk->app->wipe_seq = next;

	return BOX2SEQ_MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウト設定
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int FadeOutSet( BOX2_SYS_WORK * syswk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_BOX_APP );

	syswk->app->wipe_seq = next;

	return BOX2SEQ_MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ウェイト設定
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		ウェイト後のシーケンス
 * @param		wait		ウェイト
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WaitSet( BOX2_SYS_WORK * syswk, int next, int wait )
{
	syswk->next_seq  = next;
	syswk->app->wait = wait;
	return BOX2SEQ_MAINSEQ_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数セット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		func		VBLANK関数
 * @param		next		VBLANK関数終了後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int VFuncSet( BOX2_SYS_WORK * syswk, void * func, int next )
{
	syswk->app->vnext_seq = next;
	BOX2MAIN_VFuncSet( syswk->app, func );
	return BOX2SEQ_MAINSEQ_VFUNC;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		設定済みのVBLANK関数開始リクエスト
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		VBLANK関数終了後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int VFuncReqSet( BOX2_SYS_WORK * syswk, int next )
{
	syswk->app->vnext_seq = next;
	BOX2MAIN_VFuncReqSet( syswk->app );
	return BOX2SEQ_MAINSEQ_VFUNC;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動後のシーケンスを設定
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		カーソル移動後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetCursorMoveRet( BOX2_SYS_WORK * syswk, int next )
{
	syswk->next_seq = next;
	return BOX2SEQ_MAINSEQ_CURSORMOVE_RET;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえセット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		id			はい・いいえＩＤ
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int YesNoSet( BOX2_SYS_WORK * syswk, u32 id )
{
	syswk->app->ynID = id;
	BOX2MAIN_YesNoWinSet( syswk, 0 );
	GFL_UI_KEY_SetRepeatSpeed( syswk->app->key_repeat_speed, syswk->app->key_repeat_wait );
	return BOX2SEQ_MAINSEQ_YESNO;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理：道具を預かる >> はい
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_ItemGetYes( BOX2_SYS_WORK * syswk )
{
	u32 item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );

	// アイテムチェック
	if( MYITEM_AddItem( syswk->dat->myitem, item, 1, HEAPID_BOX_APP ) == TRUE ){
		POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

		BOX2BMP_ItemGetMsgPut( syswk, item, BOX2BMPWIN_ID_MSG1 );
		item = 0;
		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, item );
		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos,
				syswk->app->pokeicon_id[syswk->get_pos] );
		}
		BOX2MAIN_PokeInfoRewrite( syswk, syswk->get_pos );
	}else{
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2BMP_ItemGetErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
	}
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理：ポケモンを逃がす >> はい
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_PokeFreeYes( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2MAIN_PokeFreeCreate( syswk );
	BOX2OBJ_PokeIconFreeStart( syswk->app->seqwk );

//	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );

	return BOX2SEQ_MAINSEQ_POKEFREE_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理：ポケモンを逃がす >> いいえ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_PokeFreeNo( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeCursorVanish( syswk, TRUE );

	BOX2BMP_SysWinVanishTrans( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV) );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理：バッグへ戻す >> はい
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxItemArg_RetBagYes( BOX2_SYS_WORK * syswk )
{
	if( MYITEM_AddItem( syswk->dat->myitem, syswk->app->get_item, 1, HEAPID_BOX_APP ) == TRUE ){
		POKEMON_PASO_PARAM * ppp;
		u16	item;

		ppp  = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
		item = ITEM_DUMMY_DATA;

		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, item );

		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange( syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
		}
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
//		BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
		return BOX2SEQ_MAINSEQ_ITEM_BAGIN_ANM;
	}

//	Snd_SePlay( SE_BOX2_WARNING );
	BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_MAX, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理：ボックス終了 >> はい
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_BoxEndYes( BOX2_SYS_WORK * syswk )
{
//	Snd_SePlay( SE_BOX2_LOG_OFF );
	syswk->next_seq = BOX2SEQ_MAINSEQ_END;
	return FadeOutSet( syswk, BOX2SEQ_MAINSEQ_RELEASE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理：ボックス終了 >> いいえ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_BoxEndNo( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:		// あずける
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
		return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;

	case BOX_MODE_TURETEIKU:	// つれていく
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;

	case BOX_MODE_SEIRI:		// せいり
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
			BOX2BGWFRM_BoxListButtonOn( syswk->app );
			return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
		}else{
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}
		break;

	case BOX_MODE_ITEM:			// どうぐせいり
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, FALSE );
			BOX2BGWFRM_BoxListButtonOn( syswk->app );
			return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
		}else{
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, FALSE );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			return BOX2SEQ_MAINSEQ_ITEM_MAIN;
		}
		break;

	case BOX_MODE_BATTLE:		// バトルボックス
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
			BOX2BGWFRM_BoxListButtonOn( syswk->app );
			return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
		}else{
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
		}
		break;

	case BOX_MODE_SLEEP:	// 寝かせる
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		return BOX2SEQ_MAINSEQ_SLEEP_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理：寝かせる >> はい
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_SleepYes( BOX2_SYS_WORK * syswk )
{
	syswk->dat->retTray = syswk->tray;
	syswk->dat->retPoke = syswk->get_pos;
	syswk->next_seq = BOX2SEQ_MAINSEQ_END;
	return FadeOutSet( syswk, BOX2SEQ_MAINSEQ_RELEASE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理：寝かせる >> いいえ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_SleepNo( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG4 );
	BOX2MAIN_ResetTouchBar( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return BOX2SEQ_MAINSEQ_SLEEP_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧボタンアニメセット
 *
 * @param		syswk			ボックス画面システムワーク
 * @param		wfrmID		ＢＧウィンドウフレームＩＤ
 * @param		next			ボタンアニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BgButtonAnmSet( BOX2_SYS_WORK * syswk, u32 wfrmID, int next )
{
	u16	sx, sy;
	s8	px, py;

	BGWINFRM_PosGet( syswk->app->wfrm, wfrmID, &px, &py );
	BGWINFRM_SizeGet( syswk->app->wfrm, wfrmID, &sx, &sy );

	syswk->app->bawk.btn_mode = BOX2MAIN_BTN_ANM_MODE_BG;
	syswk->app->bawk.btn_id   = BGWINFRM_BGFrameGet( syswk->app->wfrm, wfrmID );
	syswk->app->bawk.btn_pal1 = BOX2MAIN_BG_PAL_SELWIN + 1;
	syswk->app->bawk.btn_pal2 = BOX2MAIN_BG_PAL_SELWIN;
	syswk->app->bawk.btn_seq  = 0;
	syswk->app->bawk.btn_cnt  = 0;

	syswk->app->bawk.btn_px   = px;
	syswk->app->bawk.btn_py   = py;
	syswk->app->bawk.btn_sx   = sx;
	syswk->app->bawk.btn_sy   = sy;

	syswk->next_seq = next;
	return BOX2SEQ_MAINSEQ_BUTTON_ANM;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		マーキングフレーム用ＢＧボタンアニメセット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		no			ボタン番号
 * @param		next		ボタンアニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MarkingButtonAnmSet( BOX2_SYS_WORK * syswk, u32 no, int next )
{
	syswk->app->bawk.btn_mode = BOX2MAIN_BTN_ANM_MODE_BG;
	syswk->app->bawk.btn_id   = BGWINFRM_BGFrameGet( syswk->app->wfrm, BOX2MAIN_WINFRM_MARK );
	syswk->app->bawk.btn_pal1 = BOX2MAIN_BG_PAL_SELWIN + 1;
	syswk->app->bawk.btn_pal2 = BOX2MAIN_BG_PAL_SELWIN;
	syswk->app->bawk.btn_seq  = 0;
	syswk->app->bawk.btn_cnt  = 0;

	if( no == 0 ){
		syswk->app->bawk.btn_py = 14;
	}else{
		syswk->app->bawk.btn_py = 17;
	}
	syswk->app->bawk.btn_px = 21;
	syswk->app->bawk.btn_sx = 11;
	syswk->app->bawk.btn_sy = 3;

	syswk->next_seq = next;
	return BOX2SEQ_MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪボタンアニメセット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		objID		ＢＧウィンドウフレームＩＤ
 * @param		anm			アニメ番号
 * @param		next		ボタンアニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int ObjButtonAnmSet( BOX2_SYS_WORK * syswk, u32 objID, u32 anm, int next )
{
	u16	sx, sy;
	s8	px, py;

	syswk->app->bawk.btn_mode = BOX2MAIN_BTN_ANM_MODE_OBJ;
	syswk->app->bawk.btn_id   = objID;
	syswk->app->bawk.btn_pal1 = anm;
	syswk->app->bawk.btn_pal2 = 0;
	syswk->app->bawk.btn_seq  = 0;
	syswk->app->bawk.btn_cnt  = 0;

	syswk->app->bawk.btn_px   = 0;
	syswk->app->bawk.btn_py   = 0;
	syswk->app->bawk.btn_sx   = 0;
	syswk->app->bawk.btn_sy   = 0;

	syswk->next_seq = next;
	return BOX2SEQ_MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		サブプロセスセット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		type		サブプロセスタイプ
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SubProcSet( BOX2_SYS_WORK * syswk, u8 type )
{
	syswk->subProcType = type;
	syswk->next_seq = BOX2SEQ_MAINSEQ_SUB_PROCCALL;
	return FadeOutSet( syswk, BOX2SEQ_MAINSEQ_RELEASE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		バッグ画面復帰処理【ボックス整理】
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int RcvBag_ModeSeiri( BOX2_SYS_WORK * syswk )
{
	int	next;

	switch( syswk->subProcMode ){
	case SUB_PROC_MODE_MENU_BOX:			// ボックスのポケモンのメニューから
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		break;

	case SUB_PROC_MODE_MENU_PARTY:		// 手持ちのポケモンのメニューから
		PartyFrmSet( syswk );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_ITEM );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
		break;
	}

	BOX2OBJ_PokeCursorAdd( syswk );

	if( syswk->subRet == 0 ){
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		return next;
	}

	if( syswk->subRet == ITEM_HAKKINDAMA &&
			BOX2MAIN_PokeParaGet(syswk,syswk->get_pos,syswk->tray,ID_PARA_monsno,NULL) != MONSNO_GIRATHINA ){
		syswk->subRet = 0;
	}else if( ITEM_CheckMail( syswk->subRet ) == FALSE ){
		POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, syswk->subRet );
		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
		}
		MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP_L );
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	return BOX2SEQ_MAINSEQ_ITEMSET_MSG;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		バッグ画面復帰処理【持ち物整理】
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int RcvBag_ModeItem( BOX2_SYS_WORK * syswk )
{
	int	next;

	switch( syswk->subProcMode ){
	case SUB_PROC_MODE_MENU_BOX:			// ボックスのポケモンのメニューから
//		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
		next = BOX2SEQ_MAINSEQ_ITEM_MAIN;
		break;

	case SUB_PROC_MODE_MENU_PARTY:		// 手持ちのポケモンのメニューから
		PartyFrmSet( syswk );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
//		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_PARTY, BOX2UI_ITEM_PARTY_MENU2 );
		next = BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
		break;
	}

	BOX2SEQ_ItemModeMenuSet( syswk, syswk->subRet );

	if( syswk->subRet == 0 ){
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_SET, BOX2BMPWIN_ID_MSG1 );
		// ボックスのポケモンのメニューから
		if( syswk->subProcMode == SUB_PROC_MODE_MENU_BOX ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		// 手持ちのポケモンのメニューから
		}else{
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
		}
		return next;
	}

	if( syswk->subRet == ITEM_HAKKINDAMA &&
			BOX2MAIN_PokeParaGet(syswk,syswk->get_pos,syswk->tray,ID_PARA_monsno,NULL) != MONSNO_GIRATHINA ){
		syswk->subRet = 0;
	}else if( ITEM_CheckMail( syswk->subRet ) == FALSE ){
		POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, syswk->subRet );
		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
		}
		MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP_L );

		// アイテムアイコンセット
		syswk->app->get_item = syswk->subRet;
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );
		BOX2OBJ_ItemIconCursorOn( syswk->app );
	}

	// ボックスのポケモンのメニューから
	if( syswk->subProcMode == SUB_PROC_MODE_MENU_BOX ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
	// 手持ちのポケモンのメニューから
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	return BOX2SEQ_MAINSEQ_ITEMSET_MSG;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		バッグ画面復帰処理【バトルボックス】
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int RcvBag_ModeBattleBox( BOX2_SYS_WORK * syswk )
{
	int	next;

	switch( syswk->subProcMode ){
	case SUB_PROC_MODE_MENU_BOX:			// ボックスのポケモンのメニューから
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		next = BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
		break;

	case SUB_PROC_MODE_MENU_PARTY:		// 手持ちのポケモンのメニューから
		PartyFrmSet( syswk );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_PARTY, BOX2UI_BATTLEBOX_PARTY_ITEM );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		next = BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
		break;
	}

	BOX2OBJ_PokeCursorAdd( syswk );

	if( syswk->subRet == 0 ){
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		return next;
	}

	if( syswk->subRet == ITEM_HAKKINDAMA &&
			BOX2MAIN_PokeParaGet(syswk,syswk->get_pos,syswk->tray,ID_PARA_monsno,NULL) != MONSNO_GIRATHINA ){
		syswk->subRet = 0;
	}else if( ITEM_CheckMail( syswk->subRet ) == FALSE ){
		POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, syswk->subRet );
		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
		}
		MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP_L );
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	return BOX2SEQ_MAINSEQ_ITEMSET_MSG;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ステータス画面復帰処理【連れて行く】
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int RcvStatus_ModeTureteiku( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 5 );
	BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
//	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTIN_MAIN, BOX2UI_PTIN_MAIN_STATUS );
	BOX2OBJ_PokeCursorAdd( syswk );
	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ステータス画面復帰処理【預ける】
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int RcvStatus_ModeAzukeru( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 5 );
	BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
//		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_STATUS );
	PartyFrmSet( syswk );
	BOX2OBJ_PokeCursorAdd( syswk );
	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ステータス画面復帰処理【ボックス整理】
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int RcvStatus_ModeSeiri( BOX2_SYS_WORK * syswk )
{
	switch( syswk->subProcMode ){
	case SUB_PROC_MODE_MENU_BOX:			// ボックスのポケモンのメニューから
		BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		BOX2OBJ_PokeCursorAdd( syswk );
		return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;

	case SUB_PROC_MODE_MENU_PARTY:		// 手持ちのポケモンのメニューから
		BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_STATUS );
		PartyFrmSet( syswk );
		BOX2OBJ_PokeCursorAdd( syswk );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;

	case SUB_PROC_MODE_TB_BOX:				// ボックスのポケモンのタッチバーから
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
		BOX2MAIN_InitBoxMoveFrameScroll( syswk );
		while( BOX2MAIN_VFuncBoxMoveFrmIn(syswk) ){}
		BOX2OBJ_EndTrayCursorScroll( syswk );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;

	case SUB_PROC_MODE_TB_BOX_GET:		// ボックスのポケモンのタッチバーから（掴んでいる時）
		if( syswk->tray == syswk->get_tray ){
			BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		}else{
			BOX2OBJ_PokeIconChange( syswk, syswk->get_tray, syswk->get_pos, syswk->app->pokeicon_id[BOX2OBJ_POKEICON_GET_POS] );
			BOX2OBJ_Vanish( syswk->app, syswk->app->pokeicon_id[BOX2OBJ_POKEICON_GET_POS], TRUE );
		}
		BOX2MAIN_HandGetPokeSet( syswk );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->tb_status_pos );
		BOX2MAIN_InitBoxMoveFrameScroll( syswk );
		while( BOX2MAIN_VFuncBoxMoveFrmIn(syswk) ){}
		BOX2OBJ_EndTrayCursorScroll( syswk );
		if( syswk->tb_status_pos >= BOX2UI_ARRANGE_PGT_TRAY2 && syswk->tb_status_pos <= BOX2UI_ARRANGE_PGT_TRAY5 ){
			BOX2OBJ_ChangeTrayName( syswk, syswk->tb_status_pos-BOX2UI_ARRANGE_PGT_TRAY2, TRUE );
		}
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;

	case SUB_PROC_MODE_TB_PARTY:			// 手持ちのポケモンのタッチバーから
		PartyFrmSetRight( syswk );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;

	case SUB_PROC_MODE_TB_PARTY_GET:	// 手持ちのポケモンのタッチバーから（掴んでいる時）
		PartyFrmSetRight( syswk );
		BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		BOX2MAIN_HandGetPokeSet( syswk );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->tb_status_pos );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;
	}

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ステータス画面復帰処理【バトルボックス】
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int RcvStatus_ModeBattleBox( BOX2_SYS_WORK * syswk )
{
	switch( syswk->subProcMode ){
	case SUB_PROC_MODE_MENU_BOX:			// ボックスのポケモンのメニューから
		BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_MAIN, BOX2UI_BATTLEBOX_MAIN_STATUS );
		BOX2OBJ_PokeCursorAdd( syswk );
		return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;

	case SUB_PROC_MODE_MENU_PARTY:		// 手持ちのポケモンのメニューから
		BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_PARTY, BOX2UI_BATTLEBOX_PARTY_STATUS );
		PartyFrmSet( syswk );
		BOX2OBJ_PokeCursorAdd( syswk );
		return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;

	case SUB_PROC_MODE_TB_PARTY:			// 手持ちのポケモンのタッチバーから
		PartyFrmSetRight( syswk );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
//		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;

	case SUB_PROC_MODE_TB_PARTY_GET:	// 手持ちのポケモンのタッチバーから（掴んでいる時）
		PartyFrmSetRight( syswk );
		BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		BOX2MAIN_HandGetPokeSet( syswk );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->tb_status_pos );
		CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;
	}

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ切り替え・左
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		切り替え後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayLeftScrollSet( BOX2_SYS_WORK * syswk, int next )
{
	BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_L );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncTrayScrollLeft, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ切り替え・右
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		切り替え後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayRightScrollSet( BOX2_SYS_WORK * syswk, int next )
{
	BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_R );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncTrayScrollRight, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイのポケモンを取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayPokeGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos  = pos;
	syswk->get_tray = syswk->tray;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	PokeMoveWorkAlloc( syswk );

	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
//	BOX2BGWFRM_TemochiButtonOn( syswk->app );

	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_TRAY;
	syswk->next_seq  = BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;	// 掴んだままボックス移動に移行する場合はここにいく
//	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMove, BOX2SEQ_MAINSEQ_ARRANGE_TRAYPOKE_CHG_END );
	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMoveBoxParty, BOX2SEQ_MAINSEQ_ARRANGE_TRAYPOKE_CHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイのポケモンを取得【バトルボックス】
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int GetBattleBoxTrayPoke( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos  = pos;
	syswk->get_tray = syswk->tray;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	PokeMoveWorkAlloc( syswk );

	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
//	BOX2BGWFRM_TemochiButtonOn( syswk->app );

	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_TRAY;
	syswk->next_seq  = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;	// 掴んだままボックス移動に移行する場合はここにいく
	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMoveBattleBoxMain, BOX2SEQ_MAINSEQ_BATTLEBOX_TRAYPOKE_CHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイのポケモンを取得【連れて行く】
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayPokeGetPartyIn( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;
	syswk->get_tray = syswk->tray;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
	BOX2OBJ_PartyPokeIconFrmInSetRight( syswk );

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, BOX2OBJ_POKEICON_GET_POS, FALSE );

/*
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
	if( PokeParty_GetPokeCount( syswk->dat->pokeparty ) != TEMOTI_POKEMAX ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_TSURETEIKU );
	}else{
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_TSURETEIKU_ERR );
	}
*/

	PokeMoveWorkAlloc( syswk );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );

	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
	return VFuncSet( syswk, BOX2MAIN_GetTrayPokeMoveDrop, BOX2SEQ_MAINSEQ_PARTYIN_POKECHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイのポケモンを取得（キー操作）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 * @param		ret			取得後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayPokeGetKey( BOX2_SYS_WORK * syswk, u32 pos, int ret )
{
	syswk->get_pos = pos;
	syswk->app->msg_put = 1;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
	BOX2OBJ_PokeCursorAdd( syswk );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,ret) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンを取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyPokeGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	POKEMON_PARAM * pp;

	syswk->get_pos  = pos;
	syswk->get_tray = syswk->tray;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos-BOX2OBJ_POKEICON_TRAY_MAX );

/*
	// 手持ち１匹
	if( BOX2MAIN_BattlePokeCheck( syswk, pos - BOX2OBJ_POKEICON_TRAY_MAX ) == FALSE &&
			BOXDAT_GetPokeExistCount2( syswk->dat->sv_box, syswk->tray ) == 0 ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_BATTLE_POKE_ERR );
	// メール
	}else if( ITEM_CheckMail( PP_Get(pp,ID_PARA_item,NULL) ) == TRUE ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_MAIL_ERR );
	}else{
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_AZUKERU );
	}
*/

	PokeMoveWorkAlloc( syswk );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
	syswk->next_seq  = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;	// 掴んで手持ち外に配置する場合はここに移行する
	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMoveParty, BOX2SEQ_MAINSEQ_ARRANGE_PARTYPOKE_CHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンを取得【バトルボックス】
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int GetBattleBoxPartyPoke( BOX2_SYS_WORK * syswk, u32 pos )
{
	POKEMON_PARAM * pp;

	syswk->get_pos  = pos;
	syswk->get_tray = syswk->tray;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos-BOX2OBJ_POKEICON_TRAY_MAX );

	PokeMoveWorkAlloc( syswk );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
	syswk->next_seq  = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;	// 掴んで手持ち外に配置する場合はここに移行する
	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMoveParty, BOX2SEQ_MAINSEQ_BATTLEBOX_PARTYPOKE_CHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンを取得【預ける】
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyPokeGetDrop( BOX2_SYS_WORK * syswk, u32 pos )
{
	POKEMON_PARAM * pp;
	u32	i;

	syswk->get_pos = pos;

/*
	for( i=BOX2OBJ_POKEICON_TRAY_MAX; i<BOX2OBJ_POKEICON_MAX; i++ ){
		if( i == syswk->get_pos ){
			BOX2OBJ_PokeIconPriChg( syswk->app, i, BOX2OBJ_POKEICON_PRI_CHG_GET );
		}else{
			BOX2OBJ_PokeIconPriChg( syswk->app, i, BOX2OBJ_POKEICON_PRI_CHG_PUT );
		}
	}
	BOX2OBJ_PokeCursorAdd( syswk );
*/
	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, BOX2OBJ_POKEICON_GET_POS, FALSE );
//	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );

	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos - BOX2OBJ_POKEICON_TRAY_MAX );

/*
	// 手持ち１匹
	if( BOX2MAIN_BattlePokeCheck( syswk, pos - BOX2OBJ_POKEICON_TRAY_MAX ) == FALSE ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_BATTLE_POKE_ERR );
	// メール
	}else if( ITEM_CheckMail( PP_Get(pp,ID_PARA_item,NULL) ) == TRUE ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_MAIL_ERR );
	}else{
		if( BOXDAT_GetEmptySpaceTray( syswk->dat->sv_box, syswk->tray ) == 0 ){
			BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_AZUKERU_ERR );
		}else{
			BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_AZUKERU );
		}
	}
*/

	PokeMoveWorkAlloc( syswk );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
	return VFuncSet( syswk, BOX2MAIN_GetPartyPokeMoveDrop, BOX2SEQ_MAINSEQ_PARTYOUT_POKECHG_END );
//	syswk->next_seq  = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;	// 掴んで手持ち外に配置する場合はここに移行する
//	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMoveParty, BOX2SEQ_MAINSEQ_ARRANGE_PARTYPOKE_CHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンを取得（キー操作）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 * @param		ret			取得後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyPokeGetKey( BOX2_SYS_WORK * syswk, u32 pos, int ret )
{
	syswk->get_pos = pos;
	syswk->app->msg_put = 1;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
	BOX2OBJ_PokeCursorAdd( syswk );

//	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
//	BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,ret) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ/手持ちポケモンを取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 * @param		next		取得後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyPokeGet( BOX2_SYS_WORK * syswk, u32 pos, int next )
{
	syswk->get_tray = syswk->tray;	// 取得したボックス
	syswk->get_pos  = pos;
//				syswk->poke_get_key = 1;

	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeIconPriChg( syswk->app, BOX2OBJ_POKEICON_GET_POS, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );
//	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );

	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );

	PokeMoveWorkAlloc( syswk );
	syswk->next_seq  = next;
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMoveBoxParty, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンを「つかむ」（ボックス<->手持ち入れ替え用）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyPokeMoveGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;
	syswk->poke_get_key = 1;

	BOX2OBJ_PokeIconPriChg( syswk->app, BOX2OBJ_POKEICON_GET_POS, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );
	BOX2BGWFRM_BoxListButtonOff( syswk->app );
	CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );

/*
	if( syswk->quick_mode == 0 ){
		BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );
	}
*/

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMoveGetKey, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンを「つかむ」（ボックス<->トレイ移動用）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PokeMoveGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;
	syswk->poke_get_key = 1;

	BOX2OBJ_PokeIconPriChg( syswk->app, BOX2OBJ_POKEICON_GET_POS, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );
	BOX2BGWFRM_TemochiButtonOff( syswk->app );
	CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PGT_PARTY );

/*
	if( syswk->quick_mode == 0 ){
		BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );
	}
*/

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMoveGetKey, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「つかむ」て取得したポケモンを配置
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			配置位置
 * @param		next		配置後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PokeMovePutKey( BOX2_SYS_WORK * syswk, u32 pos, int next )
{
	// トレイ
	if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
	// 手持ち
	}else if( pos < BOX2OBJ_POKEICON_PUT_MAX ){
	// 別トレイへ
	}else if( pos != BOX2MAIN_GETPOS_NONE ){
		u32	tray = syswk->box_mv_pos + pos - BOX2OBJ_POKEICON_PUT_MAX;
		if( tray >= syswk->trayMax ){
			tray -= syswk->trayMax;
		}
		// 現在参照しているトレイと同じ
		if( tray == syswk->tray ){
			return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
		}
		// 選択したトレイがいっぱい
		if( BOXDAT_GetEmptySpaceTray( syswk->dat->sv_box, tray ) == 0 ){
			return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
		}
	}

	syswk->app->poke_put_key = pos;
	syswk->next_seq = next;
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
	PokeMoveWorkAlloc( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMovePutKey, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンアイコン動作ワーク取得
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeMoveWorkAlloc( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
//	u32	i;

	work = GFL_HEAP_AllocClearMemory( HEAPID_BOX_APP, sizeof(BOX2MAIN_POKEMOVE_WORK) );

/*
	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
		work->dat[i].pp = PokemonParam_AllocWork( HEAPID_BOX_APP );
	}
	work->cnt = 0;
*/

	syswk->app->vfunk.work = work;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンアイコン動作ワーク解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeMoveWorkFree( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
//	u32	i;

	work = syswk->app->vfunk.work;

/*
	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
		GFL_HEAP_FreeMemory( work->dat[i].pp );
	}
*/

	GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイのアイテムを取得（タッチ）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayItemGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	u16	tmp;

	tmp = syswk->app->get_item;

	syswk->get_pos  = pos;
	syswk->get_tray = syswk->tray;
	syswk->app->msg_put = 1;

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );

	BOX2BGWFRM_PokeMenuOff( syswk->app->wfrm );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
//	BOX2BGWFRM_TemochiButtonOn( syswk->app );

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
//		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );
//		BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );
	}else{
		if( BOX2OBJ_VanishCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		}
	}

	{
		s16	x, y;

		BOX2OBJ_PokeIconDefaultPosGet( syswk->get_pos, &x, &y, BOX2MAIN_POKEMOVE_MODE_TRAY );
		syswk->app->tpx = x + 8;
		syswk->app->tpy = y + 8;
	}

	ItemMoveWorkAlloc( syswk );

	return VFuncSet( syswk, BOX2MAIN_VFuncItemGetTouch, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_RET) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイのアイテムを取得（キー操作）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayItemGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos  = pos;
	syswk->get_tray = syswk->tray;

	syswk->app->msg_put = 1;

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );

	BOX2SEQ_ItemModeMenuSet( syswk, syswk->app->get_item );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );

		BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ITEM_MAIN_MENU1, CURSORMOVE_PosGet(syswk->app->cmwk) );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_MAIN_MENU1 );

//		BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );
	}else{
		BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ITEM_MAIN_MENU2, CURSORMOVE_PosGet(syswk->app->cmwk) );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_MAIN_MENU2 );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeGetKey, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ/手持ちアイテムを取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyItemGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;
	if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		syswk->get_tray = syswk->tray;
	}else{
		syswk->get_tray = BOX2MAIN_GETPOS_NONE;
	}

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

//	Snd_SePlay( SE_BOX2_POKE_CATCH );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );

//	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );

//	BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );

	{
		s16	x, y;

		BOX2OBJ_PokeIconDefaultPosGet( syswk->get_pos, &x, &y, BOX2MAIN_POKEMOVE_MODE_ALL );
		syswk->app->tpx = x + 8;
		syswk->app->tpy = y + 8;
	}

	ItemMoveWorkAlloc( syswk );

//	return VFuncSet( syswk, BOX2MAIN_VFuncItemMoveTouch, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_ICONMOVE_SUBFRM_OUT );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemMoveTouch, BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_ICONMOVE_POKEADD );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手カーソルでトレイ/手持ちアイテムを取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyItemGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;
	syswk->app->get_item_init_pos = pos;
	if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		syswk->get_tray = syswk->tray;
	}else{
		syswk->get_tray = BOX2MAIN_GETPOS_NONE;
	}

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

//	Snd_SePlay( SE_BOX2_POKE_CATCH );

	syswk->poke_get_key = 1;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	BOX2OBJ_ItemIconPokePutHand( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );

//	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );

	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );

//	BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );

//	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeBoxPartyIconGetKey, BOX2SEQ_MAINSEQ_ITEMARRANGE_KEYGET_MOVE_MAIN );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeBoxPartyIconGetKey, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手カーソルでアイテムを配置・キャンセル処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyItemPutKeyCancel( BOX2_SYS_WORK * syswk )
{
	u32	pos = CURSORMOVE_PosGet( syswk->app->cmwk );

	if( pos < BOX2OBJ_POKEICON_PUT_MAX ){
		BOX2MAIN_PokeInfoPut( syswk, pos );
	}else{
		BOX2MAIN_PokeInfoOff( syswk );
	}

	ItemMoveWorkAlloc( syswk );
	return VFuncSet(
			syswk,
			BOX2MAIN_VFuncItemIconPutKeyCancel,
			BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_ICONMOVE_END );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手カーソルでアイテムを配置
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			配置位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyItemPutKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->app->poke_put_key = pos;
	syswk->poke_get_key = 0;

	{	// アイテムアイコンの位置を補正する
		s16	x, y;
		BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, &x, &y, CLSYS_DEFREND_MAIN );
		BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_ITEMICON, x, y+8, CLSYS_DEFREND_MAIN );
	}

	if( syswk->app->poke_put_key == BOX2MAIN_GETPOS_NONE ){
		return BoxPartyItemPutKeyCancel( syswk );
	}else if( BOX2MAIN_PokeItemMoveCheck( syswk, syswk->get_pos, pos ) == FALSE ){
		return BoxPartyItemPutKeyCancel( syswk );
	}

//	BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncItemIconPutKey, BOX2SEQ_MAINSEQ_ITEM_KEYGET_CHANGE_CHECK );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンのアイテムを取得（タッチ）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyItemGet( BOX2_SYS_WORK * syswk, u32 pos )
{
//	u16	tmp;

//	tmp = syswk->app->get_item;

	syswk->get_pos  = pos;
	syswk->get_tray = BOX2MAIN_GETPOS_NONE;
//	syswk->app->msg_put = 1;

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );

/*
	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		if( tmp != 0 || syswk->app->get_item != 0 ){
			BOX2BGWFRM_PokeMenuOff( syswk->app->wfrm );
		}else if( syswk->app->get_item != 0 ){
			BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		}
	}
	BOX2BMP_ItemArrangeMenuStrPrint( syswk, syswk->app->get_item );
*/
	BOX2BGWFRM_PokeMenuOff( syswk->app->wfrm );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
//	BOX2BGWFRM_BoxListButtonOn( syswk->app );

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
//		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );
//		BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );
/*
		if( ITEM_CheckMail( syswk->app->get_item ) == TRUE ){
			BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_MAIL_MOVE );
		}else{
			BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_ITEM_MOVE );
		}
*/
	}else{
		if( BOX2OBJ_VanishCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		}
	}

	{
		s16	x, y;

		BOX2OBJ_PokeIconDefaultPosGet( syswk->get_pos, &x, &y, BOX2MAIN_POKEMOVE_MODE_PARTY );
		syswk->app->tpx = x + 8;
		syswk->app->tpy = y + 8;
	}

	ItemMoveWorkAlloc( syswk );

//	return VFuncSet( syswk, BOX2MAIN_VFuncItemPartyGetTouch, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_RET) );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemPartyGetTouch, BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンのアイテムを取得（キー操作・持ち物整理）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyItemGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos  = pos;
	syswk->get_tray = syswk->tray;
	syswk->app->msg_put = 1;

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );

	BOX2SEQ_ItemModeMenuSet( syswk, syswk->app->get_item );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );
//		BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );
		BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ITEM_PARTY_MENU1, CURSORMOVE_PosGet(syswk->app->cmwk) );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU1 );
	}else{
		BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ITEM_PARTY_MENU2, CURSORMOVE_PosGet(syswk->app->cmwk) );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU2 );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeGetKey, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		アイテムアイコン動作ワーク取得
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ItemMoveWorkAlloc( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_ITEMMOVE_WORK * work;

	work = GFL_HEAP_AllocMemory( HEAPID_BOX_APP, sizeof(BOX2MAIN_ITEMMOVE_WORK) );

	syswk->app->vfunk.work = work;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		アイテムアイコン動作ワーク解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ItemMoveWorkFree( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちフレームセット（左）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PartyFrmSet( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFramePut( syswk->app->wfrm );
	BOX2OBJ_PartyPokeIconFrmSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちフレームセット（右）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PartyFrmSetRight( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFramePutRight( syswk->app->wfrm );
	BOX2OBJ_PartyPokeIconFrmSetRight( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちフレームセット【預ける】
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PartyFrmSet_PartyOut( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PartyPokeIconFrmSet( syswk );
	BOX2BGWFRM_PartyPokeFramePut( syswk->app->wfrm );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス切り替え（トレイアイコン選択）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		mode		スクロールモード
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveTrayScrollSet( BOX2_SYS_WORK * syswk, u32 mode )
{
	void * func;
	u32	dir;

	dir = BOX2MAIN_GetTrayScrollDir( syswk, syswk->tray, syswk->app->chg_tray_pos );

	syswk->tray = syswk->app->chg_tray_pos;

	BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
	BOX2MAIN_WallPaperSet( syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), dir );

	if( dir == BOX2MAIN_TRAY_SCROLL_R ){
		func = BOX2MAIN_VFuncTrayScrollRight;
	}else{
		func = BOX2MAIN_VFuncTrayScrollLeft;
	}

	switch( mode ){
	case 0:		//「つかむ」中のトレイ選択
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
		break;

	case 1:		// ボックスジャンプ
		syswk->next_seq = BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_RET;
		break;
	}

	return VFuncSet( syswk, func, syswk->next_seq );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイアイコンスクロールワーク作成
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CreateTrayIconScrollWork( BOX2_SYS_WORK * syswk )
{
	SEQWK_TRAYICON_SCROLL * wk = GFL_HEAP_AllocMemoryLo( HEAPID_BOX_APP, sizeof(SEQWK_TRAYICON_SCROLL) );
	wk->cnt = 0;
	wk->mv  = 0;
	syswk->app->seqwk = wk;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス移動フレームインセット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		フレーム移動後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveFrmInSet( BOX2_SYS_WORK * syswk, int next )
{
/*
	BOX2OBJ_BoxMoveObjInit( syswk );
//	BOX2OBJ_BoxMoveCursorSet( syswk );
	BOX2OBJ_InitBoxMoveCursorPos( syswk->app );
	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameWrite( syswk, 0 );

	BOX2BGWFRM_BoxMoveFrmInSet( syswk->app->wfrm );
*/
	BOX2MAIN_InitBoxMoveFrameScroll( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveFrmIn, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス移動フレームアウトセット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		フレーム移動後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveFrmOutSet( BOX2_SYS_WORK * syswk, int next )
{
	BOX2BGWFRM_BoxMoveFrmOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveFrmOut, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンメニューアウトセット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		フレーム移動後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PokeMenuOutSet( BOX2_SYS_WORK * syswk, int next )
{
	syswk->get_pos = BOX2MAIN_GETPOS_NONE;
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メニューセット【持ち物整理】
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		item		アイテム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2SEQ_ItemModeMenuSet( BOX2_SYS_WORK * syswk, s16 item )
{
	if( item != ITEM_DUMMY_DATA ){
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			CURSORMOVE_MoveTableBitOn( syswk->app->cmwk, BOX2UI_ITEM_MAIN_MENU1 );
		}else{
			CURSORMOVE_MoveTableBitOn( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU1 );
		}
		BOX2BMP_MenuStrPrint( syswk,ItemMenuStrTbl, 3 );
	}else{
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ITEM_MAIN_MENU1 );
		}else{
			CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU1 );
		}
		BOX2BMP_MenuStrPrint( syswk,ItemNoneMenuStrTbl, 2 );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちに移動できるか
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval	"0 = 移動可"
 * @retval	"1 = 戦えるポケモンがいなくなる"
 * @retval	"2 = メールを持っている"
 */
//--------------------------------------------------------------------------------------------
static int PartyOutCheck( BOX2_SYS_WORK * syswk )
{
	POKEMON_PARAM * pp;
	u32	pos;
	u16	item;

	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;

	// 手持ち１
	if( BOX2MAIN_BattlePokeCheck( syswk, pos ) == FALSE ){
		return 1;
	}
	// メール
	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos );
	item = PP_Get( pp, ID_PARA_item, NULL );
	if( ITEM_CheckMail( item ) == TRUE ){
		return 2;
	}

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		マーキング変更
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			変更位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MarkingSwitch( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->app->pokeMark ^= (1<<pos);
	BOX2MAIN_MainDispMarkingChange( syswk, syswk->app->pokeMark );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙変更
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			壁紙選択位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static int SetWallPaperChange( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->app->wallpaper_pos = ( syswk->app->wp_menu - 1 ) * 4 + pos;
	return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2+pos, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_CHANGE) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス終了確認時のパッシブセット
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxEndPassiveSet( BOX2_SYS_WORK * syswk )
{
	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:		// あずける
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
		break;

	case BOX_MODE_TURETEIKU:	// つれていく
	case BOX_MODE_ITEM:				// どうぐせいり
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		break;

	case BOX_MODE_SEIRI:			// せいり
	case BOX_MODE_BATTLE:			// バトルボックス
	case BOX_MODE_SLEEP:			// ねかせる
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
		}else{
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		}
		break;
	}
}










//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
//	旧処理
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモン逃がした後の処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangePartyPokeFreeEnd( BOX2_SYS_WORK * syswk )
{
	PokeMoveWorkFree( syswk );
	if( syswk->box_mv_flg != 0 ){
		BOX2BMP_BoxMoveNameWrite( syswk, 0 );
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	syswk->get_pos = BOX2MAIN_GETPOS_NONE;

	if( syswk->dat->callMode == BOX_MODE_AZUKERU ){
		BOX2BMP_PokeSelectMsgPut(
			syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
		return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」メイン処理へ復帰
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutPokeMenuRcv( BOX2_SYS_WORK * syswk )
{
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちフレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangePartyPokeFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFrameInitPutLeft( syswk->app->wfrm );
	BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
	BOX2OBJ_PartyPokeIconFrmInSet( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_START );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」手持ちポケモン選択処理へ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangePartyStart( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_PARTY, BOX2UI_ITEM_PARTY_POKE1 );
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_POKE1 );
	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangePartyPokeFrmOut( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_BUTTON_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス/手持ちボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxPartyButtonIn( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFrameOff( syswk );

	BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );

//	BOX2BMP_ButtonPutTemochi( syswk );
//	BOX2BMP_ButtonPutIdou( syswk );

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_MAIN, BOX2UI_ITEM_MAIN_PARTY );
//		return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEM_MAIN );
		return BOX2SEQ_MAINSEQ_ITEM_MAIN;
	}

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_PARTY );
//	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ち・トレイ入れ替えメイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxPartyChgStart( BOX2_SYS_WORK * syswk )
{
	// トレイ選択から戻るときのデフォルト位置設定
	syswk->app->old_cur_pos = BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;

	if( syswk->quick_mode != 0 ){
		int	ret;
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->quick_get );
		syswk->get_pos = syswk->quick_get;
		ret = PokeMoveGetKey( syswk, syswk->quick_get );
		syswk->quick_mode = 0;
		return ret;
	}else{
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->quick_get );
		BOX2MAIN_PokeInfoPut( syswk, syswk->quick_get );
	}

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」ポケモン移動メイン処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxPartyChgMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	// 手持ち
	ret = BOX2UI_HitCheckTrgPartyPokeRight();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return BoxPartyPokeGet( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			CURSORMOVE_PosSet( syswk->app->cmwk, ret );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
				return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
			}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
				return YStatusButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
			}
			return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
		}
	}

	// ボックス
	ret = BOX2UI_HitTrayPokeTrg();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return BoxPartyPokeGet( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			CURSORMOVE_PosSet( syswk->app->cmwk, ret );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
				return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
			}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
				return YStatusButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
			}
			return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
		}
	}


	ret = BOX2UI_BoxArrangePokeMove( syswk );

#ifdef	BTS_2299_TEST
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
//↑[GS_CONVERT_TAG]
		test_flg_2299 = 1;
		test_pos_2299 = 0;
	}
	if( test_flg_2299 == 1 ){
		if( test_pos_2299 != 0 ){
			test_pos_2299 = 0;
		}else{
			test_pos_2299 = 2;
		}
		ret = test_pos_2299;
		CURSORMOVE_PosSet( syswk->app->cmwk, ret );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	}
#endif	// BTS_2299_TEST

	switch( ret ){
	case BOX2UI_ARRANGE_MOVE_TRAY1:	// 37: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeSelectOff( syswk );
		return BoxMoveButtonFrmIn( syswk, 0 );

	case BOX2UI_ARRANGE_MOVE_TRAY2:	// 38: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeSelectOff( syswk );
		return BoxMoveButtonFrmIn( syswk, 1 );

	case BOX2UI_ARRANGE_MOVE_TRAY3:	// 39: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeSelectOff( syswk );
		return BoxMoveButtonFrmIn( syswk, 2 );

	case BOX2UI_ARRANGE_MOVE_TRAY4:	// 40: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeSelectOff( syswk );
		return BoxMoveButtonFrmIn( syswk, 3 );

	case BOX2UI_ARRANGE_MOVE_TRAY5:	// 41: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeSelectOff( syswk );
		return BoxMoveButtonFrmIn( syswk, 4 );

	case BOX2UI_ARRANGE_MOVE_TRAY6:	// 42: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeSelectOff( syswk );
		return BoxMoveButtonFrmIn( syswk, 5 );

	case BOX2UI_ARRANGE_MOVE_LEFT:		// 43: トレイ矢印（左）
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BoxMoveTrayIconChgOnly( syswk, -1 );
		CURSORMOVE_PosSet( syswk->app->cmwk, (syswk->box_mv_pos%BOX2OBJ_TRAYICON_MAX)+BOX2UI_ARRANGE_MOVE_TRAY1 );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
			return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
		}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
			return YStatusButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
		}
		return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;

	case BOX2UI_ARRANGE_MOVE_RIGHT:	// 44: トレイ矢印（右）
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BoxMoveTrayIconChgOnly( syswk, 1 );
		CURSORMOVE_PosSet( syswk->app->cmwk, (syswk->box_mv_pos%BOX2OBJ_TRAYICON_MAX)+BOX2UI_ARRANGE_MOVE_TRAY1 );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
			return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
		}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
			return YStatusButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
		}
		return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;

	case BOX2UI_ARRANGE_MOVE_CHANGE:	// 45: ボックスをきりかえる
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXMV_BTN, BOX2SEQ_MAINSEQ_ARRANGE_BOX_MOVE_BUTTON );

	case CURSORMOVE_CURSOR_MOVE:	// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MOVE_TRAY1 ){
				if( BOX2MAIN_PokeInfoPut( syswk, pos ) == TRUE ){
					if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == FALSE ){
						BOX2BMP_ButtonPutYStatus( syswk );
						BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
					}
				}else{
					if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
						BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
					}
				}
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
				if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
					BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		if( syswk->app->vfunk.freq != NULL ){
			return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN_CURMOVE_RET );
		}
		break;

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
	case CURSORMOVE_NONE:			// 動作なし
		break;

	case BOX2UI_ARRANGE_MOVE_RETURN:	// 36: やめる
//		Snd_SePlay( SE_BOX2_CLOSE_PARTY_TRAY );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
		return PartyButtonAnmSet( syswk, 1, BOX2SEQ_MAINSEQ_BOX_MOVE_END_BUTTON );

	case CURSORMOVE_CANCEL:				// キャンセル
		if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == FALSE ){
//			Snd_SePlay( SE_BOX2_CLOSE_PARTY_TRAY );
			return PartyButtonAnmSet( syswk, 1, BOX2SEQ_MAINSEQ_BOX_MOVE_END_BUTTON );
		}else{
//			Snd_SePlay( SE_BOX2_CANCEL );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN );
		}
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return PokeMoveGetKey( syswk, ret );
		}
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ち・トレイ入れ替え後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxPartyPokeChgEnd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	put_pos;
	u32	set_pos;

	work = syswk->app->vfunk.work;
	put_pos = work->put_pos;
	set_pos = work->set_pos;

//	Snd_SePlay( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
	if( ( put_pos & BOX2MAIN_BOXMOVE_FLG ) == 0 ){
		BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );
	}
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );

	if( syswk->box_mv_flg != 0 ){
		BOX2BMP_BoxMoveNameWrite( syswk, 0 );
	}
	if( BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_poke_exist, NULL ) == 0 ){
		BOX2MAIN_PokeInfoOff( syswk );
	}else{
		if( put_pos != BOX2MAIN_GETPOS_NONE ){
			if( ( put_pos & BOX2MAIN_BOXMOVE_FLG ) != 0 ){
				BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
			}
		}else{
			if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX && set_pos != syswk->get_pos ){
				u32	party_pos;
				POKEMON_PARAM * pp;

				party_pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, party_pos );

				if( ITEM_CheckMail( PP_Get(pp,ID_PARA_item,NULL) ) == TRUE ){
//					Snd_SePlay( SE_BOX2_WARNING );
					BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_MAIL, BOX2BMPWIN_ID_MSG1 );
					syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_ERROR;
					return BOX2SEQ_MAINSEQ_TRGWAIT;
				}else if( BOX2MAIN_BattlePokeCheck( syswk, party_pos ) == FALSE ){
//					Snd_SePlay( SE_BOX2_WARNING );
					BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
					syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_ERROR;
					return BOX2SEQ_MAINSEQ_TRGWAIT;
				}
			}

			if( set_pos != BOX2MAIN_GETPOS_NONE &&
				( set_pos & BOX2MAIN_BOXMOVE_FLG ) != 0 &&
				BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos)+(set_pos^BOX2MAIN_BOXMOVE_FLG) != syswk->tray ){
				if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_BOXMAX, BOX2BMPWIN_ID_MSG1 );
				}else if( BOX2MAIN_BattlePokeCheck( syswk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX ) == TRUE ){
					BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_BOXMAX, BOX2BMPWIN_ID_MSG1 );
				}else{
					BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
				}
//				Snd_SePlay( SE_BOX2_WARNING );
				syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_ERROR;
				return BOX2SEQ_MAINSEQ_TRGWAIT;
			}
		}
	}

	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：入れ替えエラーメッセージ表示後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxPartyChgError( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanishTrans( syswk->app, BOX2BMPWIN_ID_MSG1 );

	if( syswk->quick_mode != 0 ){
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
		BOX2MAIN_PokeSelectOff( syswk );
		return BoxMoveEndStart( syswk );
	}

	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちフレーム右移動後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangePartyFrmMoveREnd( BOX2_SYS_WORK * syswk )
{
	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	}

	if( BOX2MAIN_PokeParaGet( syswk, syswk->quick_get, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
		BOX2BMP_ButtonPutYStatus( syswk );
		BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
	}

	syswk->box_mv_flg = 1;		// ボックス移動フラグ
	syswk->pokechg_mode = 1;

	syswk->box_mv_pos = syswk->tray;

	return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_START );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちフレーム左移動後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangePartyFrmMoveLEnd( BOX2_SYS_WORK * syswk )
{
	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_PARTY, BOX2UI_ITEM_PARTY_POKE1 );
		BOX2MAIN_PokeInfoPut(
			syswk, CURSORMOVE_PosGet(syswk->app->cmwk)+BOX2OBJ_POKEICON_TRAY_MAX );
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}

	if( syswk->quick_mode != 0 ){
		BOX2UI_CursorMoveChange(
			syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, syswk->quick_get-BOX2OBJ_POKEICON_TRAY_MAX );
	}else{
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_POKE1 );
	}
	BOX2MAIN_PokeInfoPut(
		syswk, CURSORMOVE_PosGet(syswk->app->cmwk)+BOX2OBJ_POKEICON_TRAY_MAX );
	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxMoveFrmIn( BOX2_SYS_WORK * syswk )
{
	syswk->box_mv_pos = syswk->tray;

	return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_EXIT_BUTTON_FRMIN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：やめるフレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeExitButtonFrmIn( BOX2_SYS_WORK * syswk )
{
//	この処理を手持ちフレームインに変更

	BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
	BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
	BOX2OBJ_PartyPokeIconFrmInSetRight( syswk );

	BOX2BMP_ButtonPutYStatus( syswk );
	if( BOX2MAIN_PokeParaGet( syswk, syswk->quick_get, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
		BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_START );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームアウト設定
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxMoveFrmOutSet( BOX2_SYS_WORK * syswk )
{
	return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXMOVE_FRMOUT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxMoveFrmOut( BOX2_SYS_WORK * syswk )
{
	if( syswk->pokechg_mode == 1 ){
		return PartyFrmMoveLeft( syswk );
	}
	BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_BOX_RET_BUTTON_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス/もどるボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxRetButtonIn( BOX2_SYS_WORK * syswk )
{
	u32	id;
	u32	pos;
	int	next;

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		id   = BOX2UI_INIT_ID_ITEM_MAIN;
		pos  = BOX2UI_ITEM_A_MAIN_CHANGE;
		next = BOX2SEQ_MAINSEQ_ITEM_MAIN;
	}else{
		id   = BOX2UI_INIT_ID_ARRANGE_MAIN;
//		pos  = BOX2UI_ARRANGE_MAIN_CHANGE;
		next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		if( syswk->quick_mode != 0 ){
			pos = syswk->quick_get;
			BOX2MAIN_PokeInfoPut( syswk, pos );
		}
	}

	BOX2UI_CursorMoveChange( syswk, id, pos );

//	BOX2BMP_ButtonPutTemochi( syswk );
//	BOX2BMP_ButtonPutIdou( syswk );

//	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, next );
	return next;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaChgMain( BOX2_SYS_WORK * syswk )
{
	switch( CURSORMOVE_MainCont( syswk->app->cmwk ) ){
	case BOX2UI_BOXTHEMA_CHG_TRAY1:	// 00: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BoxThemaTrayIconSel( syswk, 0 );

	case BOX2UI_BOXTHEMA_CHG_TRAY2:	// 01: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BoxThemaTrayIconSel( syswk, 1 );

	case BOX2UI_BOXTHEMA_CHG_TRAY3:	// 02: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BoxThemaTrayIconSel( syswk, 2 );

	case BOX2UI_BOXTHEMA_CHG_TRAY4:	// 03: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BoxThemaTrayIconSel( syswk, 3 );

	case BOX2UI_BOXTHEMA_CHG_TRAY5:	// 04: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BoxThemaTrayIconSel( syswk, 4 );

	case BOX2UI_BOXTHEMA_CHG_TRAY6:	// 05: トレイアイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BoxThemaTrayIconSel( syswk, 5 );


	case BOX2UI_BOXTHEMA_CHG_LEFT:		// 06: トレイ矢印（左）
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		return BoxThemaTrayIconChg( syswk, -1 );

	case BOX2UI_BOXTHEMA_CHG_RIGHT:	// 07: トレイ矢印（右）
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		return BoxThemaTrayIconChg( syswk, 1 );

	case BOX2UI_BOXTHEMA_CHG_MOVE:		// 08: ボックスをきりかえる
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
		if( syswk->tray == syswk->box_mv_pos ){
//			Snd_SePlay( SE_BOX2_DISABLE_BUTTON );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXMV_MENU, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN );
		}
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXMV_MENU, BOX2SEQ_MAINSEQ_BOXTHEMA_TRAYCHG_BUTTON );

	case BOX2UI_BOXTHEMA_CHG_WP:		// 09: かべがみ
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_BUTTON );

	case BOX2UI_BOXTHEMA_CHG_NANE:		// 10: なまえ
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_BOXTHEMA_NAMEIN_CALL);

	case CURSORMOVE_CURSOR_MOVE:		// 移動
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		if( syswk->app->vfunk.freq != NULL ){
//			Snd_SePlay( SE_BOX2_MOVE_CURSOR );
			return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_CURMOVE_RET );
		}
		break;

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	case BOX2UI_BOXTHEMA_CHG_CANCEL:	// 11: やめる
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_CANCEL );
//		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_EXIT_OUT );
		return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_EXIT_OUT;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更開始セット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaButtonOut( BOX2_SYS_WORK * syswk )
{
//	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_TRAY_IN );
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_TRAY_IN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームインセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaTrayIn( BOX2_SYS_WORK * syswk )
{
	syswk->box_mv_pos = syswk->tray;

	return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_EXIT_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「やめる」フレームインセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaExitIn( BOX2_SYS_WORK * syswk )
{
	return BoxMoveExitButtonInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_MENU_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更メニューイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaMenuIn( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_BoxThemaMenuPrint( syswk );
	BOX2BGWFRM_BoxThemaMenuInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxThemaMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_MSG_SET );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更メッセージセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaMsgSet( BOX2_SYS_WORK * syswk )
{
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BOXTHEMA_CHG, syswk->cur_rcv_pos );

	if( syswk->cur_rcv_pos >= BOX2UI_BOXTHEMA_CHG_TRAY1 &&
		syswk->cur_rcv_pos <= BOX2UI_BOXTHEMA_CHG_TRAY6 ){
//		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
	}else{
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	}

	BOX2BMP_BoxThemaMsgPut( syswk, BOX2BMP_MSGID_THEMA_INIT, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「やめる」フレームアウトセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaExitOut( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
	return BoxMoveExitButtonOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_MENU_OUT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更メニューアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaMenuOut( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_BoxThemaMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxThemaMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_TRAY_OUT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaTrayOut( BOX2_SYS_WORK * syswk )
{
	return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_BUTTON_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更終了＞ポケモン選択へ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxThemaButtonIn( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	if( syswk->dat->callMode == BOX_MODE_TURETEIKU ){
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTIN_MAIN, BOX2UI_PTIN_MAIN_NAME );
//		return VFuncSet( syswk, BOX2MAIN_VFuncExitButtonMove, BOX2SEQ_MAINSEQ_PARTYIN_MAIN_RCV );
		return VFuncSet( syswk, BOX2MAIN_VFuncExitButtonMove, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );
//	↑仮だけど、たぶんこの関数は消えるので。
	}

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_MAIN, BOX2UI_ITEM_MAIN_NAME );
	}else{
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_NAME );
	}


//	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_BOXARRANGE_POKE_MENU_RCV );
	return BOX2SEQ_MAINSEQ_BOXARRANGE_POKE_MENU_RCV;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙フレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperFrmIn( BOX2_SYS_WORK * syswk )
{
	syswk->app->wallpaper_pos = BOX2MAIN_GetWallPaperNumber( syswk, syswk->box_mv_pos );

	BOX2OBJ_WallPaperChgObjInit( syswk );
	BOX2OBJ_WallPaperCursorSet( syswk );
	BOX2OBJ_WallPaperNameSet( syswk );
	BOX2BMP_WallPaperNameWrite( syswk );

	BOX2BGWFRM_BoxThemaMenuOutSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncWallPaperFrmIn, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_MENU_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更メニューイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperMenuIn( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_WallPaperChgMenuPrint( syswk );
	BOX2BGWFRM_BoxMoveMenuInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveMenuIn, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_START );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙選択開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperChgStart( BOX2_SYS_WORK * syswk )
{
	syswk->app->old_cur_pos = syswk->app->wallpaper_pos % BOX2OBJ_WPICON_MAX;
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_WALLPAPER_CHG, syswk->app->old_cur_pos );
//	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );

	BOX2BMP_BoxThemaMsgPut( syswk, BOX2BMP_MSGID_THEMA_WALL, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙選択メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperChgMain( BOX2_SYS_WORK * syswk )
{
	switch( CURSORMOVE_MainCont( syswk->app->cmwk ) ){
	case BOX2UI_WALLPAPER_CHG_ICON1:		// 00: 壁紙アイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		WallPaperPosSet( syswk, 0 );
		return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;

	case BOX2UI_WALLPAPER_CHG_ICON2:		// 01: 壁紙アイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		WallPaperPosSet( syswk, 1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;

	case BOX2UI_WALLPAPER_CHG_ICON3:		// 02: 壁紙アイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		WallPaperPosSet( syswk, 2 );
		return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;

	case BOX2UI_WALLPAPER_CHG_ICON4:		// 03: 壁紙アイコン
//		Snd_SePlay( SE_BOX2_DECIDE );
		WallPaperPosSet( syswk, 3 );
		return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;

	case BOX2UI_WALLPAPER_CHG_LEFT:		// 04: トレイ矢印（左）
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		WallPaperPageChange( syswk, -1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;

	case BOX2UI_WALLPAPER_CHG_RIGHT:		// 05: トレイ矢印（右）
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		WallPaperPageChange( syswk, 1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;

	case BOX2UI_WALLPAPER_CHG_ENTER:		// 06: 変更
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
		return WallPaperChange( syswk );

	case CURSORMOVE_CURSOR_MOVE:			// 移動
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		if( syswk->app->vfunk.freq != NULL ){
			return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_WALLPAPER_CURMOVE_RET );
		}
		break;

	case BOX2UI_WALLPAPER_CHG_CANCEL:		// 07: キャンセル
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	case CURSORMOVE_CANCEL:
//		Snd_SePlay( SE_BOX2_CANCEL );
//		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_CANCEL_BUTTON );
		return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_CANCEL_BUTTON;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙メニューアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperMenuOut( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_BoxMoveMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveMenuOut, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_FRM_OUT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙フレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperFrmOut( BOX2_SYS_WORK * syswk )
{
	return VFuncSet( syswk, BOX2MAIN_VFuncWallPaperFrmOut, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_END );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperChgEnd( BOX2_SYS_WORK * syswk )
{
	return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_MENU_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperChange( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_WallPaperChange( syswk, syswk->app->wallpaper_pos );
#if 0
	// 追加壁紙はプラチナ以降の番号を使用している
	if( syswk->app->wallpaper_pos < BOX_NORMAL_WALLPAPER_MAX ){
		BOXDAT_SetWallPaperNumber( syswk->dat->sv_box, syswk->tray, syswk->app->wallpaper_pos );
	}else{
		BOXDAT_SetWallPaperNumber(
			syswk->dat->sv_box, syswk->tray,
			syswk->app->wallpaper_pos - BOX_NORMAL_WALLPAPER_MAX + BOX_TOTAL_WALLPAPER_MAX_PL );
	}
#endif
	BOXDAT_SetWallPaperNumber( syswk->dat->sv_box, syswk->tray, syswk->app->wallpaper_pos );
	BOX2OBJ_WallPaperCursorSet( syswk );
	BOX2OBJ_TrayIconCgxMake( syswk, syswk->tray );

	return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_WHITE_IN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更ホワイトアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperWhiteOut( BOX2_SYS_WORK * syswk )
{
	PaletteWorkSet_VramCopy( syswk->app->pfd, FADE_MAIN_BG, 0, FADE_PAL_ALL_SIZE );
	PaletteFadeReq( syswk->app->pfd, PF_BIT_MAIN_BG, 0xc000, 0, 0, 16, 0x7fff, GFUser_VIntr_GetTCBSYS() );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHANGE;
	return BOX2SEQ_MAINSEQ_PALETTE_FADE;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更ホワイトイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperWhiteIn( BOX2_SYS_WORK * syswk )
{
	PaletteFadeReq( syswk->app->pfd, PF_BIT_MAIN_BG, 0xc000, 0, 16, 0, 0x7fff, GFUser_VIntr_GetTCBSYS() );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;
	return BOX2SEQ_MAINSEQ_PALETTE_FADE;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_BoxArrangePokeMenuRcv( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	// 道具整理
	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		return BOX2SEQ_MAINSEQ_ITEM_MAIN;
	}
	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ち選択メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_BoxArrangePartyMenuRcv( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyInMainRcv( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」手持ちポケモンフレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyInPartyFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PartyPokeIconFrmInSetRight( syswk );
	BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
	BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYIN_POKEMOVE_SET );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」ポケモン移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyInPokeMoveSet( BOX2_SYS_WORK * syswk )
{
	PokeMoveWorkAlloc( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyInPokeMove, BOX2SEQ_MAINSEQ_PARTYIN_PARTY_FRM_OUT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」手持ちポケモンフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyInPartyFrmOut( BOX2_SYS_WORK * syswk )
{
	const CURSORMOVE_DATA * pw;
	u8	x, y;

//	Snd_SePlay( SE_BOX2_POKE_PUT );

	CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );

	pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, syswk->get_pos );
	x = pw->px;
	y = pw->py;
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );

	syswk->get_pos = BOX2MAIN_GETPOS_NONE;

	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );

	BOX2MAIN_PokeInfoOff( syswk );

	BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYIN_RET_FRM_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」戻るボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyInRetFrmIn( BOX2_SYS_WORK * syswk )
{
	return VFuncSet( syswk, BOX2MAIN_VFuncExitButtonMove, BOX2SEQ_MAINSEQ_PARTYIN_MAIN_RCV );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyInPokeMenuRcv( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けられるかをチェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutStartCheck( BOX2_SYS_WORK * syswk )
{
	POKEMON_PARAM * pp;
	u32	pos;
	u16	item;

	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;

	// 手持ち１
	if( BOX2MAIN_BattlePokeCheck( syswk, pos ) == FALSE ){
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_START_ERROR;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}
	// メール
	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos );
	item = PP_Get( pp, ID_PARA_item, NULL );
	if( ITEM_CheckMail( item ) == TRUE ){
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_MAIL, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_START_ERROR;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}

	syswk->box_mv_pos = syswk->tray;

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );

	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeOutBoxFrmIn, BOX2SEQ_MAINSEQ_PARTYOUT_START_RET_FRM_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けるエラー
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutStartError( BOX2_SYS_WORK * syswk )
{
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV) );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択「やめる」ボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutStartRetFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeOutBoxObjInit( syswk );
	BOX2OBJ_BoxMoveCursorSet( syswk );
	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameWrite( syswk, 0 );
	return BoxMoveExitButtonInSet( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_MENUFRM_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択メニューイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutMenuFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_PartyOutMenuPrint( syswk );
	BOX2BGWFRM_BoxMoveMenuInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveMenuIn, BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_START );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutBoxSelectStart( BOX2_SYS_WORK * syswk )
{
	syswk->app->old_cur_pos = syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_BOXSEL, syswk->app->old_cur_pos );
//	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_INIT, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutBoxSelectMain( BOX2_SYS_WORK * syswk )
{
	switch( CURSORMOVE_MainCont( syswk->app->cmwk ) ){
	case BOX2UI_PTOUT_BOXSEL_TRAY1:	// トレイアイコン
		return PokeOutTraySelect( syswk, 0 );

	case BOX2UI_PTOUT_BOXSEL_TRAY2:	// トレイアイコン
		return PokeOutTraySelect( syswk, 1 );

	case BOX2UI_PTOUT_BOXSEL_TRAY3:	// トレイアイコン
		return PokeOutTraySelect( syswk, 2 );

	case BOX2UI_PTOUT_BOXSEL_TRAY4:	// トレイアイコン
		return PokeOutTraySelect( syswk, 3 );

	case BOX2UI_PTOUT_BOXSEL_TRAY5:	// トレイアイコン
		return PokeOutTraySelect( syswk, 4 );

	case BOX2UI_PTOUT_BOXSEL_TRAY6:	// トレイアイコン
		return PokeOutTraySelect( syswk, 5 );

	case BOX2UI_PTOUT_BOXSEL_LEFT:		// トレイ矢印（左）
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		return PokeOutTrayIconChg( syswk, -1 );

	case BOX2UI_PTOUT_BOXSEL_RIGHT:	// トレイ矢印（右）
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		return PokeOutTrayIconChg( syswk, 1 );

	case BOX2UI_PTOUT_BOXSEL_ENTER:	//「ポケモンをあずける」
		return PartyOutEnter( syswk );

	case CURSORMOVE_CURSOR_MOVE:	// 移動
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		if( syswk->app->vfunk.freq != NULL ){
			return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_BOXSEL_CURMOVE_RET );
		}
		break;

	case BOX2UI_PTOUT_BOXSEL_CANCEL:	//「やめる」
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );

	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_CANCEL );
		return PartyOutEnd( syswk, FALSE );
	}

	return BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択メニューアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutBoxMenuFrmOut( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_BoxMoveMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveMenuOut, BOX2SEQ_MAINSEQ_PARTYOUT_END_RET_FRM_OUT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択「やめる」ボタンアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutEndRetFrmOut( BOX2_SYS_WORK * syswk )
{
	return BoxMoveExitButtonOutSet( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_BOXMOVE_FRM_OUT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択トレイフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutBoxMoveFrmOut( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeOutBoxObjVanish( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeOutBoxFrmOut, BOX2SEQ_MAINSEQ_PARTYOUT_END_RET_FRM_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けるメイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutEndRetFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_MAIN, BOX2UI_PTOUT_MAIN_AZUKERU );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	BOX2OBJ_PokeCursorAdd( syswk );
	BOX2OBJ_PokeCursorVanish( syswk, TRUE );

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );

	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所決定
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutEnterButton( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_BoxMoveMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyOutButtonMove, BOX2SEQ_MAINSEQ_PARTYOUT_CHECK );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けられるかをチェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutCheck( BOX2_SYS_WORK * syswk )
{

	if( BOXDAT_GetPokeExistCount( syswk->dat->sv_box, syswk->box_mv_pos ) == BOX_MAX_POS ){
		BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_BOXMAX, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_START_RET_FRM_IN;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );

	BOX2OBJ_PokeOutBoxObjVanish( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeOutBoxFrmOut, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_PARTYFRM_RIGHT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンフレームを右へ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutEnterPartyFrmRight( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_AzukeruPartyPokeFrameRightMoveSet( syswk->app->wfrm );
//	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrmRight, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_TRAY_SCROLL );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_TRAY_SCROLL );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けるトレイへスクロール
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutEnterTrayScroll( BOX2_SYS_WORK * syswk )
{
	void * func;
	u32	tmp;

	tmp = syswk->tray;
	syswk->tray = syswk->box_mv_pos;

	if( tmp == syswk->box_mv_pos ){
		return BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_POKE_MOVE;
	}else if( tmp > syswk->box_mv_pos ){
		BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
		BOX2MAIN_WallPaperSet(
			syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), BOX2MAIN_TRAY_SCROLL_L );

		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		func = BOX2MAIN_VFuncTrayScrollLeft;
	}else{
		BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
		BOX2MAIN_WallPaperSet(
			syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), BOX2MAIN_TRAY_SCROLL_R );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		func = BOX2MAIN_VFuncTrayScrollRight;
	}
	return VFuncSet( syswk, func, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_POKE_MOVE );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモン移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutEnterPokeMove( BOX2_SYS_WORK * syswk )
{
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;

	PokeMoveWorkAlloc( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyOutPokeMove, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_PARTYFRM_LEFT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンフレームを左へ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutEnterPartyFrmLeft( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work = syswk->app->vfunk.work;

//	Snd_SePlay( SE_BOX2_POKE_PUT );

	BOX2MAIN_PokeDataMove( syswk );

	BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );

	PokeMoveWorkFree( syswk );

	BOX2BGWFRM_AzukeruPartyPokeFrameLeftMoveSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_RET_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もどる」ボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutEnterRetIn( BOX2_SYS_WORK * syswk )
{
	return VFuncSet( syswk, BOX2MAIN_VFuncExitButtonMove, BOX2SEQ_MAINSEQ_PARTYOUT_COMP );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける完了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutComp( BOX2_SYS_WORK * syswk )
{
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_MAIN, BOX2UI_PTOUT_MAIN_POKE1 );

	BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );

	BOX2MAIN_PokeInfoPut( syswk, BOX2UI_PTOUT_MAIN_POKE1+BOX2OBJ_POKEICON_TRAY_MAX );

	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );

	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」ポケモン移動メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxPartyChgMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」手持ちポケモンメイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangePartyMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」メイン処理へ戻る（メッセージ表示あり）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutMenuCloseEnd( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」ボックス選択処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutBoxSelCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_BoxThemaChgCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_WallPaperChgCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyInMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」メイン処理へ戻る（メッセージあり）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyInMenuCloseEnd( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
	BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持たせたあとの処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeRetBagSet( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_VanishCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
	}

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_END );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ITEM_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持ち物をバッグに戻せるかをチェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeRetBagCheck( BOX2_SYS_WORK * syswk )
{
	if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		// メール
		if( ITEM_CheckMail( syswk->app->get_item ) == TRUE ){
			const CURSORMOVE_DATA * pw;
			u16	pos;
			u8	x, y;
			pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, pos );
			x = pw->px;
			y = pw->py;
			BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );

//			Snd_SePlay( SE_BOX2_WARNING );
			BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_MAIL, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_SET;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}
	}

	BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_RET_CHECK, BOX2BMPWIN_ID_MSG1 );
	return YesNoSet( syswk, YESNO_ID_ITEM_RET_BAG );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：バッグに戻すキャンセル処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeRetBagCancel( BOX2_SYS_WORK * syswk )
{
	const CURSORMOVE_DATA * pw;
	u16	pos;
	u8	x, y;

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		pos = syswk->get_pos;
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	}else{
		pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, pos );
	pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, pos );
	x = pw->px;
	y = pw->py;
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );

	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
	BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_END );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：バッグへ戻す終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeRetBagEnd( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_END;
	}
	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}
	return BOX2SEQ_MAINSEQ_ITEM_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：タマゴは持てないメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeGetBagCheck( BOX2_SYS_WORK * syswk )
{
	const CURSORMOVE_DATA * pw;
	u16	pos;
	u8	x, y;

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		pos = syswk->get_pos;
	}else{
		pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
	}
	CURSORMOVE_PosSet( syswk->app->cmwk, pos );
	pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, pos );
	x = pw->px;
	y = pw->py;
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );

//	Snd_SePlay( SE_BOX2_WARNING );
	BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_EGG, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_SET;
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持たせたメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeItemSetMsg( BOX2_SYS_WORK * syswk )
{
	const CURSORMOVE_DATA * pw;
	u16	pos;
	u8	x, y;

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		pos = syswk->get_pos;
	}else{
		pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
	}
	CURSORMOVE_PosSet( syswk->app->cmwk, pos );
	pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, pos );
	x = pw->px;
	y = pw->py;
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );

	BOX2BMP_ItemSetMsgPut( syswk, syswk->subRet, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_SET;
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：アイテムをセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeIconMovePokeAdd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_ITEMMOVE_WORK * mvwk;
	POKEMON_PASO_PARAM * ppp;
	u16	item;

	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_POKEADD;
	}

	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );

	mvwk = syswk->app->vfunk.work;

	// 移動先にアイテムをセット
	item = BOX2MAIN_PokeParaGet( syswk, mvwk->put_pos, syswk->tray, ID_PARA_item, NULL );
	BOX2MAIN_PokeParaPut( syswk, mvwk->put_pos, syswk->tray, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, mvwk->put_pos );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, mvwk->put_pos,
			syswk->app->pokeicon_id[mvwk->put_pos] );
	}
	BOX2MAIN_PokeInfoPut( syswk, mvwk->put_pos );

	// 移動元にアイテムをセット
	syswk->app->get_item = item;
	BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->get_tray, ID_PARA_item, syswk->app->get_item );
	if( syswk->get_tray == BOX2MAIN_GETPOS_NONE || syswk->get_tray == syswk->tray ){
		ppp = BOX2MAIN_PPPGet( syswk, syswk->get_tray, syswk->get_pos );
		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange(
				syswk, syswk->get_tray, syswk->get_pos,
				syswk->app->pokeicon_id[syswk->get_pos] );
		}
	}

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		syswk->get_pos = mvwk->put_pos;
		ItemMoveWorkFree( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
//		return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEM_MAIN );
		return BOX2SEQ_MAINSEQ_ITEM_MAIN;
	}

//	Snd_SePlay( SE_BOX2_POKE_CATCH );

	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemIconChange, BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_CHANGE );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：アイテムアイコン交換動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeIconMoveChange( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_ITEMMOVE_WORK * mvwk;

	mvwk = syswk->app->vfunk.work;

	BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );

//	Snd_SePlay( SE_BOX2_POKE_PUT );

	// アウトライン消去
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	// アイテムアイコン縮小
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );

	CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	syswk->get_pos = mvwk->put_pos;

	ItemMoveWorkFree( syswk );

	return BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_CHANGE_END;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：アイテム交換終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeIconMoveChangeEnd( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_CHANGE_END;
	}

	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );

//	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEM_MAIN );
	return BOX2SEQ_MAINSEQ_ITEM_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeBoxPartyChgMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：上画面アイテムフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeBoxPartyIconMoveSubFrmOut( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
	return BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_ICONMOVE_POKEADD;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：キーでアイテム入れ替え
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeKeyGetMoveMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_BoxItemArrangeMoveHand( syswk );

	if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == FALSE ){
		ret = CURSORMOVE_CANCEL;
	}

	switch( ret ){
	case CURSORMOVE_CURSOR_MOVE:	// 移動
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_KEYGET_MOVE_MAIN_CURMOVE_RET );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	case CURSORMOVE_NONE:			// 動作なし
		break;

	case BOX2UI_ARRANGE_MOVE_RETURN:	// 36: やめる
	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_POKE_PUT );
		return BoxPartyItemPutKey( syswk, BOX2MAIN_GETPOS_NONE );

	default:
//		Snd_SePlay( SE_BOX2_POKE_PUT );
		return BoxPartyItemPutKey( syswk, ret );
	}

	return BOX2SEQ_MAINSEQ_ITEMARRANGE_KEYGET_MOVE_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」キー取得メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeKeyGetMoveMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_KEYGET_MOVE_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちもをせいりする」手持ちポケモン選択メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangePartyMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンのアイテム移動終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangePartyIconMovePokeAddEnd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンのアイテム交換終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangePartyIconMoveChangeEnd( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_CHANGE_END;
	}

	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );

//	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_POKEADD_END );
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_POKEADD_END;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンのアイテム移動エラー
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangePartyIconMoveErr( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG4 );
	BGWINFRM_FrameOn( syswk->app->wfrm, BOX2MAIN_WINFRM_MENU5 );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU1 );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス整理：タマゴにアイテムを持たせられないメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeItemSetEggError( BOX2_SYS_WORK * syswk )
{
//	Snd_SePlay( SE_BOX2_WARNING );
	BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_EGG, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス整理：メールを預かれないメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeItemGetMailError( BOX2_SYS_WORK * syswk )
{
//	Snd_SePlay( SE_BOX2_WARNING );
	BOX2BMP_MailGetErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手取得中のポケモン移動後にキー操作に戻す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_KeyGetPokeMoveMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
	return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」手持ちポケモンボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangePartyButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	return BoxPartyButtonOutSet( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」ポケモン移動ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeMoveButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	return BoxMoveButtonOutSet( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「マーキング」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_MarkingButton( BOX2_SYS_WORK * syswk )
{
//	return MarkingStartSet( syswk );
	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_MARKING_INIT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「マーキング」終了ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_MarkingEndButton( BOX2_SYS_WORK * syswk )
{
	return MarkingScrollOutSet( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「にがす」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_FreeButton( BOX2_SYS_WORK * syswk )
{
//	return PokeFreeStartSet( syswk );
	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_POKEFREE_INIT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモン「いれかえ」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyChgButton( BOX2_SYS_WORK * syswk )
{
	if( syswk->quick_get == 0 ){
		BOX2MAIN_PokeSelectOff( syswk );
	}else{
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	}
	return PartyFrmMoveRight( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモン閉じるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangePartyCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN_CURMOVE_RET );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモン「やめる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyEndButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	return PartyFrmOutSet( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」ボックス切り替えボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeBoxMoveButton( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	return BoxMoveTrayScrollSet( syswk, 3 );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモン入れ替え終了ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_BoxMoveEndButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	return BoxMoveEndStart( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ設定「ボックスをきりかえる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_BoxThemaTrayChgButton( BOX2_SYS_WORK * syswk )
{
	return BoxMoveTrayScrollSet( syswk, 1 );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ設定「かべがみ」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_BoxThemaWallPaperButton( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_FRM_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ設定「かべがみをはりかえる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperChgButton( BOX2_SYS_WORK * syswk )
{
	return BoxMoveTrayScrollSet( syswk, 2 );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ設定壁紙「やめる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ArrangeWallPaperChgCancelButton( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, FALSE );
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	}
	syswk->app->old_cur_pos = syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;
	syswk->cur_rcv_pos = BOX2UI_BOXTHEMA_CHG_WP;
	return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_MENU_OUT;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」閉じるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyInCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, BOX2SEQ_MAINSEQ_PARTYIN_MENU_CLOSE_END );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」預けるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutButton( BOX2_SYS_WORK * syswk )
{
	return PartyOutInit( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」閉じるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_PartyOutCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, BOX2SEQ_MAINSEQ_PARTYOUT_MENU_CLOSE_END );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」手持ちポケモンボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangePartyButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	return BoxPartyButtonOutSet( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」ポケモン移動ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeMoveButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	return BoxMoveButtonOutSet( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」メニューボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeBagButton( BOX2_SYS_WORK * syswk )
{
	return ItemArrangeMenuStart( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」閉じるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
	}
	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeMenuClose, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN_CURMOVE_RET );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」ボックス切り替えボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangeBoxMoveButton( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	return BoxMoveTrayScrollSet( syswk, 4 );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」手持ちいれかえボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemChgButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	return PartyFrmMoveRight( syswk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」ボックス切り替えボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_ItemArrangePartyCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
	}
	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeMenuClose, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN_CURMOVE_RET );
}
*/

/*
static int MainSeq_ArrangePokeGetTouch( BOX2_SYS_WORK * syswk )
{
#if 0
//	Snd_SePlay( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_GET );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
#endif
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	put_pos;
	u32	set_pos;

	work = syswk->app->vfunk.work;
	put_pos = work->put_pos;
	set_pos = work->set_pos;

	PokeMoveWorkFree( syswk );
//		work->get_pos = BOX2MAIN_GETPOS_NONE;
//		work->put_pos = BOX2MAIN_GETPOS_NONE;

	if( put_pos == BOX2MAIN_GETPOS_NONE ){
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_CLOSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

//	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_GET );
//	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_TOUCH;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * トレイのポケモンを取得（キー操作・ボックス整理）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int TrayPokeGetKeyArrange( BOX2_SYS_WORK * syswk, u32 pos, int ret )
{
	syswk->get_pos = pos;
	syswk->app->msg_put = 1;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
	BOX2OBJ_PokeCursorAdd( syswk );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );

	BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,ret) );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MarkingScrollOutSet( BOX2_SYS_WORK * syswk )
{
	// キー操作設定
	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_MAIN, BOX2UI_PTOUT_MAIN_MARKING );
		break;
	case BOX_MODE_TURETEIKU:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTIN_MAIN, BOX2UI_PTIN_MAIN_MARKING );
		break;
	case BOX_MODE_SEIRI:
	default:
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_MARKING );
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_MARKING );
		}
		break;
	}
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG2 );
	if( syswk->box_mv_flg != 0 ){
		BOX2BMP_BoxMoveNameWrite( syswk, 0 );
	}

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	}
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );

	BOX2BGWFRM_MarkingFrameOutSet( syswk->app->wfrm );

	if( syswk->dat->callMode == BOX_MODE_SEIRI && syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		return VFuncSet( syswk, BOX2MAIN_VFuncMarkingFrameMove, BOX2SEQ_MAINSEQ_MARKING_END );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncMarkingFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * マーキング開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int MarkingStartSet( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, ChangeSeq(syswk,BOX2SEQ_MAINSEQ_MARKING_INIT) );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 逃がす開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PokeFreeStartSet( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_START );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ボックス/手持ちボタンアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int BoxPartyButtonOutSet( BOX2_SYS_WORK * syswk )
{
	syswk->party_sel_flg = 1;

	BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

//	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTYPOKE_FRMIN );
//	return BOX2SEQ_MAINSEQ_ARRANGE_PARTYPOKE_FRMIN;
	return 0;	// ↑上を消したので、とりあえず。
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PartyFrmOutSet( BOX2_SYS_WORK * syswk )
{
	syswk->party_sel_flg = 0;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == FALSE ){
		return MainSeq_ArrangePartyPokeFrmOut( syswk );
	}

	BOX2MAIN_PokeSelectOff( syswk );
	return PokeMenuOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTYPOKE_FRMOUT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちフレームを右へ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PartyFrmMoveRight( BOX2_SYS_WORK * syswk )
{
	syswk->get_pos = BOX2MAIN_GETPOS_NONE;
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_PartyPokeFrameRightMoveSet( syswk->app->wfrm );
#if 0
	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	}
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrmRight, BOX2SEQ_MAINSEQ_ARRANGE_PARTYFRM_MOVE_R_END );
#endif
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTYFRM_MOVE_R_END );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちフレームを左へ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PartyFrmMoveLeft( BOX2_SYS_WORK * syswk )
{
	syswk->get_pos = BOX2MAIN_GETPOS_NONE;
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	BOX2BGWFRM_PartyPokeFrameLeftMoveSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTYFRM_MOVE_L_END );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ボックス/手持ちボタンアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int BoxMoveButtonOutSet( BOX2_SYS_WORK * syswk )
{
	syswk->box_mv_flg = 1;		// ボックス移動フラグ
	syswk->pokechg_mode = 0;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );


//	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXMOVE_FRMIN );
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXMOVE_FRMIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ボックス移動終了開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int BoxMoveEndStart( BOX2_SYS_WORK * syswk )
{
	syswk->box_mv_flg = 0;		// ボックス移動フラグ

	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	if( BOX2BGWFRM_BoxMoveButtonCheck(syswk->app->wfrm) == TRUE ){
		return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXMOVE_FRMOUT_SET );
	}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
		return YStatusButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXMOVE_FRMOUT_SET );
	}

	return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXMOVE_FRMOUT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタンフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int YStatusButtonFrmOut( BOX2_SYS_WORK * syswk, int next )
{
	BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, next );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタンフレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int BoxMoveButtonFrmIn( BOX2_SYS_WORK * syswk, u32 ret )
{
	int	next;
	u32	tmp;

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		next = BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}else{
		next = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
	}

	// トレイ名と数表示
	tmp = BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + ret;
	if( tmp != syswk->box_mv_pos ){
		syswk->box_mv_pos = tmp;
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2BMP_BoxMoveNameWrite( syswk, 0 );
	}

	// 現在のトレイと同じ
	if( syswk->box_mv_pos == syswk->tray ){
		// ボタンが表示済みなら
		if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
//			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
			return BoxMoveButtonFrmOut( syswk, next );
		}
		if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
//			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
			BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
			return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, next );
		}
//		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
		return next;
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_CHANGE );
	if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == TRUE ){
		const CURSORMOVE_DATA * pw;
		u8	x, y;
		pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_CHANGE );
		x = pw->px;
		y = pw->py;
		BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
	}

	if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
		return next;
	}

	BOX2BMP_BoxMoveButtonPrint( syswk );

	if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
	}
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonMove, next );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタンフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int BoxMoveButtonFrmOut( BOX2_SYS_WORK * syswk, int next )
{
	BOX2BGWFRM_BoxMoveButtonOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, next );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更開始セット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	フレーム移動後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int BoxThemaChgStart( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	syswk->app->old_cur_pos = syswk->tray % BOX2OBJ_TRAYICON_MAX;
	syswk->cur_rcv_pos = syswk->app->old_cur_pos;

	if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
		return MainSeq_ArrangeBoxThemaButtonOut( syswk );
	}
	return PokeMenuOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_TRAY_IN );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更トレイアイコン選択
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	ret		選択位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int BoxThemaTrayIconSel( BOX2_SYS_WORK * syswk, u32 ret )
{
	const CURSORMOVE_DATA * pw;
	u8	pos;
	u8	x, y;

	syswk->box_mv_pos = BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + ret;
	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameWrite( syswk, 0 );

	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
	syswk->app->old_cur_pos = pos;
	CURSORMOVE_PosSetEx( syswk->app->cmwk, BOX2UI_BOXTHEMA_CHG_MOVE, pos, pos );
	pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, BOX2UI_BOXTHEMA_CHG_MOVE );
	x = pw->px;
	y = pw->py;
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更・トレイ切り替え
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	mv		移動方向
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int BoxThemaTrayIconChg( BOX2_SYS_WORK * syswk, s8 mv )
{
	BoxMoveTrayIconChgOnly( syswk, mv );
	CURSORMOVE_PosSet( syswk->app->cmwk, (syswk->box_mv_pos%BOX2OBJ_TRAYICON_MAX) );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 壁紙ページ切り替え
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	mv		移動方向
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void WallPaperPageChange( BOX2_SYS_WORK * syswk, s32 mv )
{
	s32	pos;

	pos = syswk->app->wallpaper_pos + BOX2OBJ_WPICON_MAX * mv;

	if( pos < 0 ){
		pos += BOX_TOTAL_WALLPAPER_MAX;
	}else if( pos >= BOX_TOTAL_WALLPAPER_MAX ){
		pos -= BOX_TOTAL_WALLPAPER_MAX;
	}
	syswk->app->wallpaper_pos = pos;

	// OBJ変更
	BOX2OBJ_WallPaperObjChange( syswk );
	// カーソル位置
	BOX2OBJ_WallPaperCursorSet( syswk );
	// 名前変更
	BOX2BMP_WallPaperNameWrite( syswk );

	CURSORMOVE_PosSet( syswk->app->cmwk, (syswk->app->wallpaper_pos%BOX2OBJ_WPICON_MAX) );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 壁紙選択位置変更
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		選択位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void WallPaperPosSet( BOX2_SYS_WORK * syswk, u32 pos )
{
	const CURSORMOVE_DATA * pw;
	u8	x, y;

	syswk->app->wallpaper_pos = syswk->app->wallpaper_pos / BOX2OBJ_WPICON_MAX * BOX2OBJ_WPICON_MAX + pos;
	BOX2OBJ_WallPaperNameSet( syswk );
	BOX2BMP_WallPaperNameWrite( syswk );

	syswk->app->old_cur_pos = CURSORMOVE_PosGet( syswk->app->cmwk );
	CURSORMOVE_PosSetEx( syswk->app->cmwk, BOX2UI_WALLPAPER_CHG_ENTER, syswk->app->old_cur_pos, syswk->app->old_cur_pos );
	pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, BOX2UI_WALLPAPER_CHG_ENTER );
	x = pw->px;
	y = pw->py;
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 壁紙変更
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int WallPaperChange( BOX2_SYS_WORK * syswk )
{
	if( syswk->app->wallpaper_pos >= BOX_NORMAL_WALLPAPER_MAX ){
		if( BOXDAT_GetDaisukiKabegamiFlag( syswk->dat->sv_box, syswk->app->wallpaper_pos-BOX_NORMAL_WALLPAPER_MAX ) == FALSE ){
//			Snd_SePlay( SE_BOX2_DISABLE_BUTTON );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXMV_MENU, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN );
		}
	}

	if( syswk->app->wallpaper_pos == BOX2MAIN_GetWallPaperNumber(syswk,syswk->box_mv_pos) ){
//		Snd_SePlay( SE_BOX2_DISABLE_BUTTON );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXMV_MENU, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN );
	}
	if( syswk->box_mv_pos != syswk->tray ){
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXMV_MENU, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_BUTTON );
	}
//	Snd_SePlay( SE_BOX2_DECIDE );
	return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXMV_MENU, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_WHITE_OUT );
}
*/


//--------------------------------------------------------------------------------------------
/**
 * ポケモン預けるメニュー開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PartyOutInit( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_PARTYOUT_START_CHECK );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 預けるトレイ決定
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	ret		指定位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PokeOutTraySelect( BOX2_SYS_WORK * syswk, u32 ret )
{
	const CURSORMOVE_DATA * pw;
	u8	pos;
	u8	x, y;

	syswk->box_mv_pos = BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + ret;

	if( GFL_UI_TP_GetTrg() == FALSE ){
//↑[GS_CONVERT_TAG]
		if( BOXDAT_GetPokeExistCount( syswk->dat->sv_box, syswk->box_mv_pos ) == BOX_MAX_POS ){
//			Snd_SePlay( SE_BOX2_DISABLE_BUTTON );
		}else{
//			Snd_SePlay( SE_BOX2_DECIDE );
		}
		return BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_BUTTON;
	}

//	Snd_SePlay( SE_BOX2_DECIDE );

	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameWrite( syswk, 0 );

	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
	syswk->app->old_cur_pos = pos;
	CURSORMOVE_PosSetEx( syswk->app->cmwk, BOX2UI_PTOUT_BOXSEL_ENTER, pos, pos );
	pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, BOX2UI_PTOUT_BOXSEL_ENTER );
	x = pw->px;
	y = pw->py;
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	return BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 預けるトレイ切り替え
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	mv		切り替え方向
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PokeOutTrayIconChg( BOX2_SYS_WORK * syswk, s8 mv )
{
	BoxMoveTrayIconChgOnly( syswk, mv );
	CURSORMOVE_PosSet( syswk->app->cmwk, (syswk->box_mv_pos%BOX2OBJ_TRAYICON_MAX) );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_MAIN;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 預ける「やめる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	flg		※未使用
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PartyOutEnd( BOX2_SYS_WORK * syswk, BOOL flg )
{
//	return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_PARTYOUT_BOXMENU_FRM_OUT );
	return BOX2SEQ_MAINSEQ_PARTYOUT_BOXMENU_FRM_OUT;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 預ける実行
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PartyOutEnter( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );

	if( BOXDAT_GetPokeExistCount( syswk->dat->sv_box, syswk->box_mv_pos ) == BOX_MAX_POS ){
//		Snd_SePlay( SE_BOX2_DISABLE_BUTTON );
	}else{
//		Snd_SePlay( SE_BOX2_DECIDE );
	}
	return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXMV_MENU, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_BUTTON );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 持ち物整理・メニュー処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int ItemArrangeMenuStart( BOX2_SYS_WORK * syswk )
{
	int	next;

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		{
			POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
			if( PPP_Get( ppp, ID_PARA_tamago_flag, NULL ) == 0 ){
				syswk->cur_rcv_pos = BOX2UI_ITEM_MAIN_MENU1;
				return SubProcSet( syswk, SUB_PROC_TYPE_BAG );
			}
		}
		next = BOX2SEQ_MAINSEQ_ITEMARRANGE_GET_BAG_CHECK;
	}else{
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		next = BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_CHECK;
	}

	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, next );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータス用のＢＧボタンアニメセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	wfrmID	ＢＧウィンドウフレームＩＤ
 * @param	next	ボタンアニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int YStatusButtonAnmSet( BOX2_SYS_WORK * syswk, u32 wfrmID, int next )
{
	u16	sx, sy;
	s8	px, py;

	BGWINFRM_PosGet( syswk->app->wfrm, wfrmID, &px, &py );
	BGWINFRM_SizeGet( syswk->app->wfrm, wfrmID, &sx, &sy );

	syswk->app->bawk.btn_mode = BOX2MAIN_BTN_ANM_MODE_BG;
	syswk->app->bawk.btn_id   = BGWINFRM_BGFrameGet( syswk->app->wfrm, wfrmID );
	syswk->app->bawk.btn_pal1 = 3;
	syswk->app->bawk.btn_pal2 = 2;
	syswk->app->bawk.btn_seq  = 0;
	syswk->app->bawk.btn_cnt  = 0;

	syswk->app->bawk.btn_px   = px;
	syswk->app->bawk.btn_py   = py;
	syswk->app->bawk.btn_sx   = sx;
	syswk->app->bawk.btn_sy   = sy;

	syswk->next_seq = next;
	return BOX2SEQ_MAINSEQ_BUTTON_ANM;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス名表示
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	ret			選択位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxMoveNameChange( BOX2_SYS_WORK * syswk, u32 ret )
{
	u32	tmp;

	tmp = BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + ret;
	if( syswk->box_mv_pos == tmp ){
		return;
	}
	syswk->box_mv_pos = tmp;

	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameWrite( syswk, 0 );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちフレーム用ＢＧボタンアニメセット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		no			ボタン番号
 * @param		next		ボタンアニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static int PartyButtonAnmSet( BOX2_SYS_WORK * syswk, u32 no, int next )
{
	syswk->app->bawk.btn_mode = BOX2MAIN_BTN_ANM_MODE_BG;
	syswk->app->bawk.btn_id   = BGWINFRM_BGFrameGet( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY );
	syswk->app->bawk.btn_pal1 = 3;
	syswk->app->bawk.btn_pal2 = 2;
	syswk->app->bawk.btn_seq  = 0;
	syswk->app->bawk.btn_cnt  = 0;

	if( no == 0 ){
		syswk->app->bawk.btn_px = 3;
	}else{
		syswk->app->bawk.btn_px = 22;
	}
	syswk->app->bawk.btn_py = 20;
	syswk->app->bawk.btn_sx = 9;
	syswk->app->bawk.btn_sy = 4;

	syswk->next_seq = next;
	return BOX2SEQ_MAINSEQ_BUTTON_ANM;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス移動・トレイ切り替え矢印
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		mv			移動方向
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxMoveTrayIconChgOnly( BOX2_SYS_WORK * syswk, s8 mv )
{
	s32	pos;

	pos = syswk->box_mv_pos + BOX2OBJ_TRAYICON_MAX * mv;
	if( pos < 0 ){
		pos += syswk->trayMax;
	}else if( pos >= syswk->trayMax ){
		pos -= syswk->trayMax;
	}
	syswk->box_mv_pos = pos;

	BOX2OBJ_TrayIconChange( syswk );
//	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2OBJ_BoxMoveCursorSet( syswk );
	BOX2BMP_BoxMoveNameWrite( syswk, 0 );
}
*/
