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
#include "sound/pm_sndsys.h"
#include "item/item.h"
#include "poke_tool/monsno_def.h"

#include "msg/msg_boxmenu.h"

#include "box2_main.h"
#include "box2_bmp.h"
#include "box2_obj.h"
#include "box2_seq.h"
#include "box2_ui.h"
#include "box2_bgwfrm.h"
#include "box2_snd_def.h"


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

	SEQ_PTOUT_PUT_CANCEL_END,			// 終了（キャンセル時）
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
static int MainSeq_BattleBoxRock( BOX2_SYS_WORK * syswk );
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
static int MainSeq_ItemPokeSetAnm( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemPokeSetRet( BOX2_SYS_WORK * syswk );
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
static int YesNo_MenuRcv( BOX2_SYS_WORK * syswk );

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

static int TrayLeftScrollSet( BOX2_SYS_WORK * syswk, BOOL btnFlg, int next );
static int TrayRightScrollSet( BOX2_SYS_WORK * syswk, BOOL btnFlg, int next );
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

static int PartyOutCheck( BOX2_SYS_WORK * syswk );
static void MarkingSwitch( BOX2_SYS_WORK * syswk, u32 pos );
static int SetWallPaperChange( BOX2_SYS_WORK * syswk, u32 pos );
static void BoxEndPassiveSet( BOX2_SYS_WORK * syswk );

static int ChgSeqSelectButton( BOX2_SYS_WORK * syswk, int next );
static int ChgSeqSelectTouch( BOX2_SYS_WORK * syswk, int next );


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

	MainSeq_BattleBoxRock,

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
	MainSeq_ItemPokeSetAnm,
	MainSeq_ItemPokeSetRet,
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
	{ BoxArg_ItemGetYes, YesNo_MenuRcv },
	// ボックス整理・ポケモンを逃がす？
	{ BoxArg_PokeFreeYes, BoxArg_PokeFreeNo },
	// 道具整理・アイテムしまう？
	{ BoxItemArg_RetBagYes, YesNo_MenuRcv },
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
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_CANCEL }		// やめる
};

// メニュー文字列テーブル：つれていく
static const BOX2BMP_BUTTON_LIST PartyInMenuStrTbl[] =
{
	{ mes_boxbutton_02_05, BOX2BMP_BUTTON_TYPE_WHITE },		// つれていく
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_03, BOX2BMP_BUTTON_TYPE_WHITE },		// マーキング
	{ mes_boxbutton_02_04, BOX2BMP_BUTTON_TYPE_WHITE },		// にがす
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_CANCEL }		// やめる
};

// メニュー文字列テーブル：あずける
static const BOX2BMP_BUTTON_LIST PartyOutMenuStrTbl[] =
{
	{ mes_boxbutton_02_06, BOX2BMP_BUTTON_TYPE_WHITE },		// あずける
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_03, BOX2BMP_BUTTON_TYPE_WHITE },		// マーキング
	{ mes_boxbutton_02_04, BOX2BMP_BUTTON_TYPE_WHITE },		// にがす
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_CANCEL }		// やめる
};

// メニュー文字テーブル：ボックステーマ変更
static const BOX2BMP_BUTTON_LIST ThemaMenuStrTbl[] =
{
	{ mes_boxmenu_03_01, BOX2BMP_BUTTON_TYPE_WHITE },			// ジャンプする
	{ mes_boxmenu_03_02, BOX2BMP_BUTTON_TYPE_WHITE },			// かべがみ
	{ mes_boxmenu_03_03, BOX2BMP_BUTTON_TYPE_WHITE },			// なまえ
	{ mes_boxmenu_03_04, BOX2BMP_BUTTON_TYPE_CANCEL },		// やめる
};

// メニュー文字テーブル：壁紙メニュー
static const BOX2BMP_BUTTON_LIST WallPaperMenuStrTbl[] =
{
	{ mes_boxmenu_04_01, BOX2BMP_BUTTON_TYPE_WHITE },			// ふうけい１
	{ mes_boxmenu_04_02, BOX2BMP_BUTTON_TYPE_WHITE },			// ふうけい２
	{ mes_boxmenu_04_03, BOX2BMP_BUTTON_TYPE_WHITE },			// ふうけい３
	{ mes_boxmenu_04_04, BOX2BMP_BUTTON_TYPE_WHITE },			// エトセトラ
	{ mes_boxmenu_04_32, BOX2BMP_BUTTON_TYPE_CANCEL },		// もどる
};

// メニュー文字テーブル：壁紙メニュー（追加壁紙あり）
static const BOX2BMP_BUTTON_LIST WallPaperMenuSpecialStrTbl[] =
{
	{ mes_boxmenu_04_23, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル
	{ mes_boxmenu_04_01, BOX2BMP_BUTTON_TYPE_WHITE },			// ふうけい１
	{ mes_boxmenu_04_02, BOX2BMP_BUTTON_TYPE_WHITE },			// ふうけい２
	{ mes_boxmenu_04_03, BOX2BMP_BUTTON_TYPE_WHITE },			// ふうけい３
	{ mes_boxmenu_04_04, BOX2BMP_BUTTON_TYPE_WHITE },			// エトセトラ
	{ mes_boxmenu_04_32, BOX2BMP_BUTTON_TYPE_CANCEL },		// もどる
};

// メニュー文字テーブル：壁紙メニュー（ふうけい１）
static const BOX2BMP_BUTTON_LIST WPLabel1MenuStrTbl[] =
{
	{ mes_boxmenu_04_07, BOX2BMP_BUTTON_TYPE_WHITE },			// もり
	{ mes_boxmenu_04_08, BOX2BMP_BUTTON_TYPE_WHITE },			// シティ
	{ mes_boxmenu_04_09, BOX2BMP_BUTTON_TYPE_WHITE },			// さばく
	{ mes_boxmenu_04_10, BOX2BMP_BUTTON_TYPE_WHITE },			// サバンナ
	{ mes_boxmenu_04_32, BOX2BMP_BUTTON_TYPE_CANCEL },		// もどる
};

// メニュー文字テーブル：壁紙メニュー（ふうけい２）
static const BOX2BMP_BUTTON_LIST WPLabel2MenuStrTbl[] =
{
	{ mes_boxmenu_04_11, BOX2BMP_BUTTON_TYPE_WHITE },			// いわやま
	{ mes_boxmenu_04_12, BOX2BMP_BUTTON_TYPE_WHITE },			// かざん
	{ mes_boxmenu_04_13, BOX2BMP_BUTTON_TYPE_WHITE },			// ゆきやま
	{ mes_boxmenu_04_14, BOX2BMP_BUTTON_TYPE_WHITE },			// どうくつ
	{ mes_boxmenu_04_32, BOX2BMP_BUTTON_TYPE_CANCEL },		// もどる
};

// メニュー文字テーブル：壁紙メニュー（ふうけい３）
static const BOX2BMP_BUTTON_LIST WPLabel3MenuStrTbl[] =
{
	{ mes_boxmenu_04_15, BOX2BMP_BUTTON_TYPE_WHITE },			// うみべ
	{ mes_boxmenu_04_16, BOX2BMP_BUTTON_TYPE_WHITE },			// かいてい
	{ mes_boxmenu_04_17, BOX2BMP_BUTTON_TYPE_WHITE },			// かわ
	{ mes_boxmenu_04_18, BOX2BMP_BUTTON_TYPE_WHITE },			// そら
	{ mes_boxmenu_04_32, BOX2BMP_BUTTON_TYPE_CANCEL },		// もどる
};

// メニュー文字テーブル：壁紙メニュー（エトセトラ）
static const BOX2BMP_BUTTON_LIST WPLabel4MenuStrTbl[] =
{
	{ mes_boxmenu_04_19, BOX2BMP_BUTTON_TYPE_WHITE },			// ポケセン
	{ mes_boxmenu_04_20, BOX2BMP_BUTTON_TYPE_WHITE },			// きんぞく
	{ mes_boxmenu_04_21, BOX2BMP_BUTTON_TYPE_WHITE },			// チェック
	{ mes_boxmenu_04_22, BOX2BMP_BUTTON_TYPE_WHITE },			// シンプル
	{ mes_boxmenu_04_32, BOX2BMP_BUTTON_TYPE_CANCEL },		// もどる
};

// メニュー文字テーブル：壁紙メニュー（スペシャル１のみ）
static const BOX2BMP_BUTTON_LIST WPLabelS01MenuStrTbl[] =
{
	{ mes_boxmenu_04_24, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル１
	{ mes_boxmenu_04_25, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル２
	{ mes_boxmenu_04_26, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル３
	{ mes_boxmenu_04_27, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル４
	{ mes_boxmenu_04_32, BOX2BMP_BUTTON_TYPE_CANCEL },		// もどる
};

// メニュー文字テーブル：壁紙メニュー（スペシャル２あり）
static const BOX2BMP_BUTTON_LIST WPLabelS11MenuStrTbl[] =
{
	{ mes_boxmenu_04_24, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル１
	{ mes_boxmenu_04_25, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル２
	{ mes_boxmenu_04_26, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル３
	{ mes_boxmenu_04_27, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル４
	{ mes_boxmenu_04_34, BOX2BMP_BUTTON_TYPE_WHITE },			// つぎへ
	{ mes_boxmenu_04_32, BOX2BMP_BUTTON_TYPE_CANCEL },		// もどる
};

// メニュー文字テーブル：壁紙メニュー（スペシャル２）
static const BOX2BMP_BUTTON_LIST WPLabelS02MenuStrTbl[] =
{
	{ mes_boxmenu_04_28, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル５
	{ mes_boxmenu_04_29, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル６
	{ mes_boxmenu_04_30, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル７
	{ mes_boxmenu_04_31, BOX2BMP_BUTTON_TYPE_WHITE },			// スペシャル８
	{ mes_boxmenu_04_33, BOX2BMP_BUTTON_TYPE_WHITE },			// つぎへ
	{ mes_boxmenu_04_32, BOX2BMP_BUTTON_TYPE_CANCEL },		// もどる
};

// メニュー文字列テーブル：バトルボックス
static const BOX2BMP_BUTTON_LIST BattleBoxMenuStrTbl[] =
{
	{ mes_boxbutton_02_09, BOX2BMP_BUTTON_TYPE_WHITE },		// いどうする
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_02, BOX2BMP_BUTTON_TYPE_WHITE },		// もちもの
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_CANCEL }		// やめる
};

// メニュー文字列テーブル：持ち物整理・持ち物あり
static const BOX2BMP_BUTTON_LIST ItemMenuStrTbl[] =
{
	{ mes_boxbutton_08_03, BOX2BMP_BUTTON_TYPE_WHITE },		// いどうする
	{ mes_boxbutton_08_01, BOX2BMP_BUTTON_TYPE_WHITE },		// バッグへ
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_CANCEL }		// やめる
};

// メニュー文字列テーブル：持ち物整理・持ち物なし
static const BOX2BMP_BUTTON_LIST ItemNoneMenuStrTbl[] =
{
	{ mes_boxbutton_08_02, BOX2BMP_BUTTON_TYPE_WHITE },		// もたせる
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_CANCEL }		// やめる
};

// メニュー文字列テーブル：ボックス整理メイン
static const BOX2BMP_BUTTON_LIST SleepMenuStrTbl[] =
{
	{ mes_boxbutton_02_10, BOX2BMP_BUTTON_TYPE_WHITE },		// ねかせる
	{ mes_boxbutton_02_07, BOX2BMP_BUTTON_TYPE_CANCEL }		// やめる
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

	syswk->app->pokeicon_ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_BOX_APP );

	GFL_UI_KEY_GetRepeatSpeed( &syswk->app->key_repeat_speed, &syswk->app->key_repeat_wait );
	GFL_UI_KEY_SetRepeatSpeed( 6, 6 );

	BOX2MAIN_VramBankSet();
	BOX2MAIN_BgInit( syswk );
	BOX2MAIN_PaletteFadeInit( syswk );

	BOX2MAIN_BgGraphicLoad( syswk );

	BOX2MAIN_MsgInit( syswk );

	BOX2MAIN_LoadLocalNoList( syswk );

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

	BOX2MAIN_SetBlendAlpha( TRUE );

	GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_BOX_APP );

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

	BOX2MAIN_YesNoWinExit( syswk );

	BOX2UI_CursorMoveExit( syswk );

	BOX2BGWFRM_Exit( syswk->app );

	BOX2OBJ_Exit( syswk->app );

	BOX2BMP_Exit( syswk );

	BOX2MAIN_FreeLocalNoList( syswk );

	BOX2MAIN_MsgExit( syswk );

	BOX2MAIN_PaletteFadeExit( syswk );
	BOX2MAIN_BgExit( syswk );

	GFL_UI_KEY_SetRepeatSpeed( syswk->app->key_repeat_speed, syswk->app->key_repeat_wait );

	GFL_ARC_CloseDataHandle( syswk->app->pokeicon_ah );

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
			return syswk->app->vnext_seq;
		}
	}
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
		PMSND_PlaySE( SE_BOX2_DECIDE );
		GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
		return syswk->next_seq;
	}

	if( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_CANCEL | PAD_BUTTON_DECIDE ) ){
		PMSND_PlaySE( SE_BOX2_DECIDE );
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
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
		u8	pos = APP_TASKMENU_GetCursorPos( syswk->app->ynWork );
		APP_TASKMENU_CloseMenu( syswk->app->ynWork );
		GFL_UI_KEY_SetRepeatSpeed( 6, 6 );
		if( pos == 0 ){
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
	if( syswk->procStatus != GFL_PROC_MAIN_VALID ){
		SubProcFunc[syswk->subProcType].exit( syswk );
		syswk->next_seq = SubProcFunc[syswk->subProcType].rcvSeq;
		return BOX2SEQ_MAINSEQ_INIT;
	}
	return BOX2SEQ_MAINSEQ_SUB_PROCMAIN;
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

	PMSND_PlaySE( SE_BOX2_LOG_IN );

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
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
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
			BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG4 );
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
		BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
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
			break;

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
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
			if( syswk->next_seq == BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN ){
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUp, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}else{
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUpJump, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}
		}
		if( y > oy ){
			seqwk->mv = -1;
			BOX2MAIN_InitTrayIconScroll( syswk, -1 );
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
			if( syswk->next_seq == BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN ){
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUp, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}else{
				return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUpJump, BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH );
			}
		}
		if( seqwk->mv == -1 ){
			BOX2MAIN_InitTrayIconScroll( syswk, -1 );
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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

	// セレクトボタン
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		syswk->get_start_mode = 0;
		return ChgSeqSelectButton( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT );
	}

	ret = BOX2UI_HitCheckTrgTrayPoke();
	if( ret != GFL_UI_TP_HIT_NONE ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return TrayPokeGet( syswk, ret );
		}else{
			CURSORMOVE_PosSet( syswk->app->cmwk, ret );
			BOX2UI_PutHandCursor( syswk, ret );
			BOX2MAIN_PokeInfoOff( syswk );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos;
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				BOX2BGWFRM_TemochiButtonOn( syswk->app );
				return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
			}else{
				return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
			}
		}
	}

	ret = BOX2UI_BoxArrangeMain( syswk );

	switch( ret ){
	case BOX2UI_ARRANGE_MAIN_NAME:		// 30: ボックス名
		PMSND_PlaySE( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );

	case BOX2UI_ARRANGE_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_MAIN_NAME );
		return TrayLeftScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );

	case BOX2UI_ARRANGE_MAIN_RIGHT:	// 32: トレイ切り替え矢印・右
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_MAIN_NAME );
		return TrayRightScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );

	case BOX2UI_ARRANGE_MAIN_PARTY:	// 33: 手持ちポケモン
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_INIT) );

	case BOX2UI_ARRANGE_MAIN_RETURN1:	// 34: 戻る１
		PMSND_PlaySE( SE_BOX2_CLOSE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_ARRANGE_MAIN_RETURN2:	// 35: 戻る２
		PMSND_PlaySE( SE_BOX2_CANCEL );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case BOX2UI_ARRANGE_MAIN_GET:			// 36: つかむ
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->get_start_mode = 0;
		syswk->mv_cnv_mode = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU1, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT) );

	case BOX2UI_ARRANGE_MAIN_STATUS:	// 37: ようすをみる
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
		syswk->get_tray    = syswk->tray;
		syswk->cur_rcv_pos = BOX2UI_ARRANGE_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_ARRANGE_MAIN_ITEM:		// 38: もちもの
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
		syswk->cur_rcv_pos = BOX2UI_ARRANGE_MAIN_ITEM;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );

	case BOX2UI_ARRANGE_MAIN_MARKING:	// 39: マーキング
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_MARKING_INIT) );

	case BOX2UI_ARRANGE_MAIN_FREE:		// 40: にがす
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_POKEFREE_INIT) );

	case BOX2UI_ARRANGE_MAIN_CLOSE:		// 41: とじる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ARRANGE_MENU_CLOSE );

	case BOX2UI_ARRANGE_MAIN_CONV:			// 42: 便利モード
		syswk->get_start_mode = 0;
		return ChgSeqSelectTouch( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT );

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_MAIN) );

	case CURSORMOVE_CANCEL:				// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		// メニュー表示時
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ARRANGE_MENU_CLOSE );
		}
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ARRANGE_MAIN_GET, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_GET );
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
	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_GET );
	BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_MAIN_GET );
	BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
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
		syswk->box_mv_pos = syswk->tray;	// 現在参照しているボックス
		// セレクトボタン
		if( syswk->mv_cnv_mode == 1 ){
			syswk->get_tray = BOX2MAIN_GETPOS_NONE;
			syswk->get_pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				syswk->get_pos = 0;
				BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
			}
		}else{
			syswk->get_tray = syswk->tray;	// 取得したボックス
			BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
			BOX2MAIN_ResetTouchBar( syswk );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		}
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT );

	case 1:
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->app->sub_seq++;
		return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_INIT );

	case 2:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
		BOX2OBJ_SetHandCursorAnm( syswk, BOX2OBJ_ANM_HAND_NORMAL );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );
		syswk->app->old_cur_pos = syswk->get_pos;
		syswk->app->sub_seq = 0;
		// セレクトボタン
		if( syswk->mv_cnv_mode == 1 ){
			if( BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_poke_exist, NULL ) == 0 ){
				BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );
			}
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_BLUE, TRUE );
			return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
		}else if( syswk->mv_cnv_mode == 2 ){
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_BLUE, TRUE );
		}else{
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
		}
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

	if( syswk->mv_cnv_end == 1 ){
		syswk->mv_cnv_mode = 0;
		syswk->mv_cnv_end = 0;
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT );
	}

	// セレクトボタン
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		if( syswk->mv_cnv_mode != 0 && syswk->poke_get_key == 0 ){
			BOX2MAIN_ChgCursorButton( syswk );
			syswk->mv_cnv_mode = 0;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT );
		}
	}

	// ポケモンドラッグチェック
	if( syswk->poke_get_key == 0 ){
		ret = BOX2UI_HitCheckTrgTrayPoke();
		if( ret != GFL_UI_TP_HIT_NONE ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				PMSND_PlaySE( SE_BOX2_POKE_CATCH );
				GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
				BOX2MAIN_PokeInfoPut( syswk, ret );
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				return BoxPartyPokeGet( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				BOX2UI_SetTouchBarIconPokeGet( syswk, ret );
				syswk->app->old_cur_pos = ret;
				BOX2UI_PutHandCursor( syswk, ret );
				BOX2MAIN_PokeInfoOff( syswk );
			}
			return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
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
			PMSND_PlaySE( SE_BOX2_DECIDE );
			BOX2MAIN_PokeInfoOff( syswk );
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );
		}
		break;

	case BOX2UI_ARRANGE_PGT_LEFT:		// 31: ボックス切り替え矢印（左）
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PGT_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_PGT_NAME );
		BOX2UI_SetTouchBarIconPokeGet( syswk, BOX2UI_ARRANGE_PGT_NAME );
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_PGT_NAME;
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
		// 取得中
		}else{
			BOX2OBJ_MovePokeIconHand( syswk );
		}
		return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );

	case BOX2UI_ARRANGE_PGT_RIGHT:	// 32: ボックス切り替え矢印（右）
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PGT_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_PGT_NAME );
		BOX2UI_SetTouchBarIconPokeGet( syswk, BOX2UI_ARRANGE_PGT_NAME );
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_PGT_NAME;
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
		// 取得中
		}else{
			BOX2OBJ_MovePokeIconHand( syswk );
		}
		return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );

	case BOX2UI_ARRANGE_PGT_PARTY:	// 39: 手持ちポケモン
		PMSND_PlaySE( SE_BOX2_DECIDE );
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeInfoOff( syswk );
		}
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
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_STATUS, 1, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_ARRANGE_PGT_RET:		// 41: 戻る
		if( syswk->poke_get_key == 0 ){
			// ボックス整理メインへ
			PMSND_PlaySE( SE_BOX2_CANCEL );
			syswk->mv_cnv_mode = 0;
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT) );
		}
		break;

	case BOX2UI_ARRANGE_PGT_CONV:		// 42: 便利モード
		if( syswk->mv_cnv_mode != 0 && syswk->poke_get_key == 0 ){
			syswk->mv_cnv_mode = 0;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT );
		}
		break;

	case CURSORMOVE_CANCEL:					// キャンセル
		if( syswk->poke_get_key == 1 ){
			// 取得中なので失敗
			return PokeMovePutKey( syswk, BOX2MAIN_GETPOS_NONE, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}else{
			// ボックス整理メインへ
			PMSND_PlaySE( SE_BOX2_CANCEL );
			syswk->mv_cnv_mode = 0;
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT) );
		}
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PGT_TRAY2 ){
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
			BOX2MAIN_InitTrayIconScroll( syswk, -1 );
			return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDown, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PGT_TRAY5 ){
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
			BOX2MAIN_InitTrayIconScroll( syswk, 1 );
			return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUp, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PGT_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PGT_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		break;

	case CURSORMOVE_NONE:						// 動作なし
		break;

	case BOX2UI_ARRANGE_PGT_TRAY1:	// 33: トレイアイコン
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		BOX2MAIN_InitTrayIconScroll( syswk, -1 );
		return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDown, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );

	case BOX2UI_ARRANGE_PGT_TRAY6:	// 38: トレイアイコン
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		BOX2MAIN_InitTrayIconScroll( syswk, 1 );
		return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollUp, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );

	case BOX2UI_ARRANGE_PGT_TRAY2:	// 34: トレイアイコン
	case BOX2UI_ARRANGE_PGT_TRAY3:	// 35: トレイアイコン
	case BOX2UI_ARRANGE_PGT_TRAY4:	// 36: トレイアイコン
	case BOX2UI_ARRANGE_PGT_TRAY5:	// 37: トレイアイコン
		BOX2OBJ_ChangeTrayName( syswk, ret-BOX2UI_ARRANGE_PGT_TRAY2, TRUE );
		// 取得中
		if( syswk->poke_get_key == 1 ){
			return PokeMovePutKey( syswk, BOX2OBJ_POKEICON_PUT_MAX+ret-BOX2UI_ARRANGE_PGT_TRAY2, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		// 未取得
		}else{
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
			return PokeMovePutKey( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
		// 未取得
		}else{
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				syswk->get_tray = syswk->tray;	// 取得したボックス
				syswk->get_pos  = ret;
				BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
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
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
		if( CURSORMOVE_PosGet(syswk->app->cmwk) >= BOX2UI_ARRANGE_PGT_NAME ){
			BOX2MAIN_PokeInfoOff( syswk );
		}
		syswk->app->sub_seq++;
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT );

	case 1:
		if( syswk->get_start_mode == 1 ){
			BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
			BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
			syswk->app->sub_seq++;
			return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_EXIT );
		}else{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos >= BOX2UI_ARRANGE_PGT_NAME ){
				pos = 0;
			}
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, pos );
			BOX2OBJ_SetHandCursorAnm( syswk, BOX2OBJ_ANM_HAND_NORMAL );
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
			BOX2MAIN_PokeInfoPut( syswk, pos );
			syswk->app->sub_seq = 0;
			return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}
		break;

	case 2:
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		syswk->app->sub_seq = 0;
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT;
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
	u8	tb_status;

	BOX2MAIN_PokeDataMove( syswk );

	syswk->poke_get_key = 0;
	syswk->get_tray = BOX2MAIN_GETPOS_NONE;

	if( syswk->app->mv_err_code == BOX2MAIN_ERR_CODE_NONE ){
		pos = CURSORMOVE_PosGet( syswk->app->cmwk );
	}else{
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		BOX2UI_PutHandCursor( syswk, syswk->get_pos );
		BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
		pos = syswk->get_pos;
	}
	syswk->app->old_cur_pos = pos;

	PokeMoveWorkFree( syswk );

	// トレイ移動表示時
	if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
		// 手持ちモードに戻る場合
		if( syswk->next_seq == BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, pos );
			BOX2MAIN_PokeInfoPut( syswk, pos );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;

			if( BOX2MAIN_PokeParaGet(syswk,pos,syswk->tray,ID_PARA_poke_exist,NULL) != 0 ){
				tb_status = BOX2OBJ_TB_ICON_ON;
			}else{
				tb_status = BOX2OBJ_TB_ICON_PASSIVE;
			}
		}else{
			if( pos < BOX2OBJ_POKEICON_TRAY_MAX &&
					BOX2MAIN_PokeParaGet(syswk,pos,syswk->tray,ID_PARA_poke_exist,NULL) != 0 ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
				tb_status = BOX2OBJ_TB_ICON_ON;
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
				tb_status = BOX2OBJ_TB_ICON_PASSIVE;
			}
		}
	// 手持ち表示時
	}else if( BOX2BGWFRM_CheckPartyPokeFrameRight(syswk->app->wfrm) == TRUE ){
		// トレイモードに戻る場合
		if( syswk->next_seq == BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN ){
			pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos > BOX2UI_ARRANGE_PGT_RIGHT ){
				pos = syswk->get_pos;
			}
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, pos );
			BOX2MAIN_PokeInfoPut( syswk, pos );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;

			if( BOX2MAIN_PokeParaGet(syswk,pos,syswk->tray,ID_PARA_poke_exist,NULL) != 0 ){
				tb_status = BOX2OBJ_TB_ICON_ON;
			}else{
				tb_status = BOX2OBJ_TB_ICON_PASSIVE;
			}
		}else{
			if( pos < BOX2OBJ_POKEICON_PUT_MAX &&
					BOX2MAIN_PokeParaGet(syswk,pos,syswk->tray,ID_PARA_poke_exist,NULL) != 0 ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
				tb_status = BOX2OBJ_TB_ICON_ON;
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
				tb_status = BOX2OBJ_TB_ICON_PASSIVE;
			}
		}
	// その他
	}else{
		if( pos < BOX2OBJ_POKEICON_PUT_MAX &&
				BOX2MAIN_PokeParaGet(syswk,pos,syswk->tray,ID_PARA_poke_exist,NULL) != 0 ){
			BOX2MAIN_PokeInfoPut( syswk, pos );
			tb_status = BOX2OBJ_TB_ICON_ON;
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			tb_status = BOX2OBJ_TB_ICON_PASSIVE;
		}
	}

	// 通常時は即終了する
	if( syswk->mv_cnv_mode == 0 ){
		syswk->mv_cnv_end = 1;
		tb_status = syswk->tb_status_btn;
	}

	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, tb_status );

	switch( syswk->app->mv_err_code ){
	case BOX2MAIN_ERR_CODE_MAIL:			// メールを持っている
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_PutPokeMoveErrMsg( syswk, BOX2MAIN_ERR_CODE_MAIL, BOX2BMPWIN_ID_MSG1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE_ERR;

	case BOX2MAIN_ERR_CODE_BATTLE:		// 戦えるポケモンがいなくなる
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_PutPokeMoveErrMsg( syswk, BOX2MAIN_ERR_CODE_BATTLE, BOX2BMPWIN_ID_MSG1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE_ERR;

	case BOX2MAIN_ERR_CODE_EGG:				// タマゴはえらべない（バトルボックス専用）
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_PutPokeMoveErrMsg( syswk, BOX2MAIN_ERR_CODE_EGG, BOX2BMPWIN_ID_MSG1 );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE_ERR;

	case BOX2MAIN_ERR_CODE_ROCK:			// ロックされている（バトルボックス専用）
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_BattleBoxRockMsgPut( syswk, 0 );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE_ERR;
	}

	if( syswk->dat->callMode == BOX_MODE_SEIRI ){
		if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
		}else{
			BOX2BGWFRM_BoxListButtonOn( syswk->app );
		}
	}

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

	if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == TRUE ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
	}

	// ロックされている（バトルボックス専用）
	if( syswk->app->mv_err_code == BOX2MAIN_ERR_CODE_ROCK ){
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG4 );
	}else{
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	}
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
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->app->sub_seq++;
		return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_PARTY_CHANGE );

	case 1:
		BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		if( syswk->poke_get_key == 1 ){
			BOX2OBJ_PartyPokeIconFrmInSet( syswk, 1 );
		}else{
			BOX2OBJ_PartyPokeIconFrmInSet( syswk, 0 );
		}
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_PARTY_CHANGE );

	case 2:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, BOX2UI_ARRANGE_PTGT_PARTY_POKE );
		BOX2UI_SetTouchBarIconPokeGet( syswk, BOX2UI_ARRANGE_PTGT_PARTY_POKE );
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeInfoPut( syswk, BOX2UI_ARRANGE_PTGT_PARTY_POKE );
		}
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_PTGT_PARTY_POKE;
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
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );

		BOX2BGWFRM_PartyPokeFrameInitPutLeft( syswk->app->wfrm );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		BOX2OBJ_PartyPokeIconFrmInSet( syswk, 0 );
		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
		}
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
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

	// セレクトボタン
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		syswk->get_start_mode = 1;
		return ChgSeqSelectButton( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );
	}

	// タッチ取得チェック
	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 6 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return PartyPokeGet( syswk, ret );
		}else{
			CURSORMOVE_PosSet( syswk->app->cmwk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2UI_PutHandCursor( syswk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2MAIN_PokeInfoOff( syswk );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				BOX2BGWFRM_BoxListButtonOn( syswk->app );
				return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN );
			}else{
				return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
			}
		}
	}

	// キー・タッチチェック
	ret = BOX2UI_BoxArrangePartyMoveMain( syswk );

	switch( ret ){
	case BOX2UI_ARRANGE_PARTY_BOXLIST:	// 06:「ボックスリスト」
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END) );

	case BOX2UI_ARRANGE_PARTY_RETURN1:	// 07: 戻る１（ボックス画面終了）
		PMSND_PlaySE( SE_BOX2_CLOSE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_ARRANGE_PARTY_RETURN2:	// 08: 戻る２
		PMSND_PlaySE( SE_BOX2_CANCEL );
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END) );

	case BOX2UI_ARRANGE_PARTY_GET:				// 09: つかむ
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->get_start_mode = 1;
		syswk->mv_cnv_mode = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU1, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT) );

	case BOX2UI_ARRANGE_PARTY_STATUS:			// 10: ようすをみる
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
		syswk->get_tray    = syswk->tray;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_ARRANGE_PARTY_ITEM:				// 11: もちもの
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );

	case BOX2UI_ARRANGE_PARTY_MARKING:		// 12: マーキング
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_MARKING_INIT) );

	case BOX2UI_ARRANGE_PARTY_FREE:				// 13: にがす
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_POKEFREE_INIT) );

	case BOX2UI_ARRANGE_PARTY_CLOSE:			// 14: やめる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MENU_CLOSE );

	case BOX2UI_ARRANGE_PARTY_CONV:				// 15: 便利モード
		syswk->get_start_mode = 1;
		return ChgSeqSelectTouch( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN) );

	case CURSORMOVE_CANCEL:							// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MENU_CLOSE );
		}
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END) );

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
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
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END );

	case 1:
		syswk->app->sub_seq = 0;
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

	PMSND_PlaySE( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PARTY_GET );
	BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_PARTY_GET );
	BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
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
		// セレクトボタン
		if( syswk->mv_cnv_mode == 1 ){
			syswk->get_tray = BOX2MAIN_GETPOS_NONE;	// 取得したボックス
			syswk->get_pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
				if( syswk->get_pos >= TEMOTI_POKEMAX ){
					syswk->get_pos = BOX2OBJ_POKEICON_TRAY_MAX;
				}else{
					syswk->get_pos += BOX2OBJ_POKEICON_TRAY_MAX;
				}
			}else{
				if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					syswk->get_pos = 0;
				}
			}
			BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		}else{
			if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
				syswk->get_tray = BOX2MAIN_GETPOS_NONE;	// 取得したボックス
			}else{
				syswk->get_tray = syswk->tray;					// 取得したボックス
			}
			BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
			BOX2MAIN_ResetTouchBar( syswk );
			BOX2BGWFRM_BoxListButtonOn( syswk->app );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		}
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );

	case 1:
		// バトルボックスがロックされているとき
		if( syswk->dat->callMode == BOX_MODE_BATTLE && syswk->dat->bbRockFlg == TRUE ){
			PMSND_PlaySE( SE_BOX2_WARNING );
			BOX2BMP_BattleBoxRockMsgPut( syswk, 0 );
			syswk->mv_cnv_mode  = 0;
			syswk->app->sub_seq = 0;
			syswk->next_seq     = BOX2SEQ_MAINSEQ_BATTLEBOX_ROCK;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}

		syswk->app->sub_seq++;
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			BOX2BGWFRM_PartyPokeFrameRightMoveSet( syswk->app->wfrm );
		}else{
			BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
			BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
			BOX2OBJ_PartyPokeIconFrmInSet( syswk, 0 );
		}
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );

	case 2:
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
		BOX2OBJ_SetHandCursorAnm( syswk, BOX2OBJ_ANM_HAND_NORMAL );
		syswk->app->old_cur_pos = syswk->get_pos;
		syswk->app->sub_seq = 0;
		// セレクトボタン
		if( syswk->mv_cnv_mode == 1 ){
			if( BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_poke_exist, NULL ) == 0 ){
				BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );
			}
			if( syswk->dat->callMode == BOX_MODE_BATTLE ){
				BOX2BGWFRM_TemochiButtonOff( syswk->app );
				BOX2BGWFRM_BoxListButtonOff( syswk->app );
				CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
			}
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_BLUE, TRUE );
			return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;
		}else if( syswk->mv_cnv_mode == 2 ){
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_BLUE, TRUE );
		}else{
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
		}
		BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
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

	if( syswk->mv_cnv_end == 1 ){
		syswk->mv_cnv_mode = 0;
		syswk->mv_cnv_end = 0;
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT );
	}

	// セレクトボタン
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		if( syswk->mv_cnv_mode != 0 && syswk->poke_get_key == 0 ){
			BOX2MAIN_ChgCursorButton( syswk );
			syswk->mv_cnv_mode = 0;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT );
		}
	}

	// ポケモンドラッグチェック
	if( syswk->poke_get_key == 0 ){
		// トレイのポケモン
		ret = BOX2UI_HitCheckTrgTrayPoke();
		if( ret != GFL_UI_TP_HIT_NONE ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				PMSND_PlaySE( SE_BOX2_POKE_CATCH );
				GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
				BOX2MAIN_PokeInfoPut( syswk, ret );
				return BoxPartyPokeGet( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				BOX2UI_SetTouchBarIconPokeGet( syswk, ret );
				syswk->app->old_cur_pos = ret;
				BOX2UI_PutHandCursor( syswk, ret );
				BOX2MAIN_PokeInfoOff( syswk );
			}
			return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;
		}
		// 手持ちのポケモン
		ret = BOX2UI_HitCheckTrgPartyPokeRight();
		if( ret != GFL_UI_TP_HIT_NONE ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			ret += BOX2OBJ_POKEICON_TRAY_MAX;
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				PMSND_PlaySE( SE_BOX2_POKE_CATCH );
				GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
				BOX2MAIN_PokeInfoPut( syswk, ret );
				return BoxPartyPokeGet( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				BOX2UI_SetTouchBarIconPokeGet( syswk, ret );
				syswk->app->old_cur_pos = ret;
				BOX2UI_PutHandCursor( syswk, ret );
				BOX2MAIN_PokeInfoOff( syswk );
			}
			return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;
		}
	}

	// キー・タッチチェック
	ret = BOX2UI_ArrangePartyPokeGetMain( syswk );

	switch( ret ){
	case BOX2UI_ARRANGE_PTGT_NAME:		// 36: ボックス名
		// 未取得
		if( syswk->poke_get_key == 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
			BOX2MAIN_PokeInfoOff( syswk );
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_LEFT:		// 37: ボックス切り替え矢印（左）
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_PTGT_NAME );
		BOX2UI_SetTouchBarIconPokeGet( syswk, BOX2UI_ARRANGE_PTGT_NAME );
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_PTGT_NAME;
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
		// 取得中
		}else{
			BOX2OBJ_MovePokeIconHand( syswk );
		}
		return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );

	case BOX2UI_ARRANGE_PTGT_RIGHT:		// 38: ボックス切り替え矢印（右）
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_PTGT_NAME );
		BOX2UI_SetTouchBarIconPokeGet( syswk, BOX2UI_ARRANGE_PTGT_NAME );
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_PTGT_NAME;
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
		// 取得中
		}else{
			BOX2OBJ_MovePokeIconHand( syswk );
		}
		return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );

	case BOX2UI_ARRANGE_PTGT_BOXLIST:	// 39: ボックスリスト
		PMSND_PlaySE( SE_BOX2_DECIDE );
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeInfoOff( syswk );
		}
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
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_STATUS, 1, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_ARRANGE_PTGT_RET:		// 41: 戻る
		if( syswk->poke_get_key == 0 ){
			// ボックス整理メインへ
			PMSND_PlaySE( SE_BOX2_CANCEL );
			syswk->mv_cnv_mode = 0;
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT) );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_CONV:	// 42: 便利モード
		if( syswk->mv_cnv_mode != 0 && syswk->poke_get_key == 0 ){
			syswk->mv_cnv_mode = 0;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT );
		}
		break;

	case CURSORMOVE_CANCEL:					// キャンセル
		if( syswk->poke_get_key == 1 ){
			// 取得中なので失敗
			return PokeMovePutKey( syswk, BOX2MAIN_GETPOS_NONE, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		}else{
			// ボックス整理メインへ
			PMSND_PlaySE( SE_BOX2_CANCEL );
			syswk->mv_cnv_mode = 0;
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_EXIT) );
		}
		break;

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PTGT_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PTGT_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
			return PokeMovePutKey( syswk, ret, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN );
		// 未取得
		}else{
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				syswk->get_tray = syswk->tray;	// 取得したボックス
				syswk->get_pos  = ret;
				BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
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
		if( syswk->get_start_mode == 0 ){
			BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
			syswk->app->sub_seq = 2;
		}else{
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
			BOX2BGWFRM_PartyPokeFrameLeftMoveSet( syswk->app->wfrm );
			syswk->app->sub_seq = 1;
		}
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
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
			BOX2OBJ_SetHandCursorAnm( syswk, BOX2OBJ_ANM_HAND_NORMAL );
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
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
			BOX2OBJ_SetHandCursorAnm( syswk, BOX2OBJ_ANM_HAND_NORMAL );
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
			if( syswk->dat->callMode == BOX_MODE_BATTLE ){
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_MAIN, pos );
				return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
			}else{
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, pos );
				return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
			}
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
		BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_BOXLIST_CHANGE );

	case 1:
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
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

	// セレクトボタン
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		syswk->get_start_mode = 0;
		return ChgSeqSelectButton( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );
	}

	ret = BOX2UI_HitCheckTrgTrayPoke();
	if( ret != GFL_UI_TP_HIT_NONE ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			syswk->get_start_mode = 0;
			return GetBattleBoxTrayPoke( syswk, ret );
		}else{
			CURSORMOVE_PosSet( syswk->app->cmwk, ret );
			BOX2UI_PutHandCursor( syswk, ret );
			BOX2MAIN_PokeInfoOff( syswk );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos;
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				BOX2BGWFRM_TemochiButtonOn( syswk->app );
				return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );
			}else{
				return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
			}
		}
	}

	ret = BOX2UI_BattleBoxMain( syswk );

	switch( ret ){
	case BOX2UI_BATTLEBOX_MAIN_NAME:		// 30: ボックス名
		PMSND_PlaySE( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );

	case BOX2UI_BATTLEBOX_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_BATTLEBOX_MAIN_NAME );
		return TrayLeftScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );

	case BOX2UI_BATTLEBOX_MAIN_RIGHT:		// 32: トレイ切り替え矢印・右
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_BATTLEBOX_MAIN_NAME );
		return TrayRightScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );

	case BOX2UI_BATTLEBOX_MAIN_PARTY:		// 33: バトルボックス
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_INIT) );

	case BOX2UI_BATTLEBOX_MAIN_RETURN1:	// 34: 戻る１
		PMSND_PlaySE( SE_BOX2_CLOSE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_BATTLEBOX_MAIN_RETURN2:	// 35: 戻る２
		PMSND_PlaySE( SE_BOX2_CANCEL );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case BOX2UI_BATTLEBOX_MAIN_GET:			// 36: いどうする
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->get_start_mode = 0;
		syswk->mv_cnv_mode = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT) );

	case BOX2UI_BATTLEBOX_MAIN_STATUS:	// 37: ようすをみる
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
		syswk->get_tray    = syswk->tray;
		syswk->cur_rcv_pos = BOX2UI_BATTLEBOX_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_BATTLEBOX_MAIN_ITEM:		// 38: もちもの
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
		syswk->cur_rcv_pos = BOX2UI_BATTLEBOX_MAIN_ITEM;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );

	case BOX2UI_BATTLEBOX_MAIN_CLOSE:		// 39: やめる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_BATTLEBOX_MENU_CLOSE );

	case BOX2UI_BATTLEBOX_MAIN_CONV:		// 40: 便利モード
		syswk->get_start_mode = 0;
		return ChgSeqSelectTouch( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_BATTLEBOX_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_BATTLEBOX_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN) );

	case CURSORMOVE_CANCEL:				// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		// メニュー表示中
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_BATTLEBOX_MENU_CLOSE );
		}
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_BATTLEBOX_MAIN_GET, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_MAIN_GET );
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
	PMSND_PlaySE( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_MAIN_GET );
	BOX2UI_PutHandCursor( syswk, BOX2UI_BATTLEBOX_MAIN_GET );
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
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );

		BOX2BGWFRM_PartyPokeFrameInitPutLeft( syswk->app->wfrm );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		BOX2OBJ_PartyPokeIconFrmInSet( syswk, 0 );
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
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

	// セレクトボタン
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		syswk->get_start_mode = 1;
		return ChgSeqSelectButton( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );
	}

	// タッチ取得チェック
	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, BattleBoxMenuStrTbl, 4 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return GetBattleBoxPartyPoke( syswk, ret );
		}else{
			CURSORMOVE_PosSet( syswk->app->cmwk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2UI_PutHandCursor( syswk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2MAIN_PokeInfoOff( syswk );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				BOX2BGWFRM_BoxListButtonOn( syswk->app );
				return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN );
			}else{
				return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
			}
		}
	}

	// キー・タッチチェック
	ret = BOX2UI_BattleBoxPartyMain( syswk );

	switch( ret ){
	case BOX2UI_BATTLEBOX_PARTY_BOXLIST:		// 06:「ボックスリスト」
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_END) );

	case BOX2UI_BATTLEBOX_PARTY_RETURN1:		// 07: 戻る１
		PMSND_PlaySE( SE_BOX2_CLOSE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_BATTLEBOX_PARTY_RETURN2:		// 08: 戻る２
		PMSND_PlaySE( SE_BOX2_CANCEL );
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_END) );

	case BOX2UI_BATTLEBOX_PARTY_GET:				// 09: いどうする
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->get_start_mode = 1;
		syswk->mv_cnv_mode = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT) );

	case BOX2UI_BATTLEBOX_PARTY_STATUS:			// 10: ようすをみる
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
		syswk->get_tray    = syswk->tray;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_BATTLEBOX_PARTY_ITEM:				// 11: もちもの
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );

	case BOX2UI_BATTLEBOX_PARTY_CLOSE:			// 12: やめる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MENU_CLOSE );

	case BOX2UI_BATTLEBOX_PARTY_CONV:				// 13: 便利モード
		syswk->get_start_mode = 1;
		return ChgSeqSelectTouch( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_INIT );

	case CURSORMOVE_CANCEL:							// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		// メニュー表示中
		if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MENU_CLOSE );
		}
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_END) );

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN) );

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
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
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_END );

	case 1:
		syswk->app->sub_seq = 0;
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

	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_BATTLEBOX_PARTY_GET );
	BOX2UI_PutHandCursor( syswk, BOX2UI_BATTLEBOX_PARTY_GET );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
	return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：バトルボックスがロックされているとき
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BATTLEBOX_ROCK
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BattleBoxRock( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG4 );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2MAIN_ResetTouchBar( syswk );
	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	}

	if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
		BOX2UI_PutHandCursor( syswk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		return BOX2SEQ_MAINSEQ_BATTLEBOX_PARTY_MAIN;
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
	BOX2UI_PutHandCursor( syswk, syswk->get_pos );
	BOX2BGWFRM_TemochiButtonOn( syswk->app );
	return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
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

	ret = BOX2UI_HitCheckTrgTrayPoke();
	if( ret != GFL_UI_TP_HIT_NONE ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 5 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return TrayPokeGetPartyIn( syswk, ret );
		}else{
			CURSORMOVE_PosSet( syswk->app->cmwk, ret );
			BOX2UI_PutHandCursor( syswk, ret );
			BOX2MAIN_PokeInfoOff( syswk );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );
			}else{
				return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
			}
		}
	}

	ret = BOX2UI_PartyInMain( syswk );

	switch( ret ){
	case BOX2UI_PTIN_MAIN_NAME:				// 30: ボックス名
		PMSND_PlaySE( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );

	case BOX2UI_PTIN_MAIN_LEFT:				// 31: トレイ切り替え矢印・左
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTIN_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_PTIN_MAIN_NAME );
		return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );

	case BOX2UI_PTIN_MAIN_RIGHT:			// 32: トレイ切り替え矢印・右
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTIN_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_PTIN_MAIN_NAME );
		return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );

	case BOX2UI_PTIN_MAIN_RETURN1:		// 33: 戻る１
		PMSND_PlaySE( SE_BOX2_CLOSE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_PTIN_MAIN_RETURN2:		// 34: 戻る２
		PMSND_PlaySE( SE_BOX2_CANCEL );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case BOX2UI_PTIN_MAIN_TSURETEIKU:	// 35: つれていく
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_PARTYIN_SELECT) );

	case BOX2UI_PTIN_MAIN_STATUS:			// 36: ようすをみる
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->get_tray    = syswk->tray;
		syswk->cur_rcv_pos = BOX2UI_PTIN_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_PTIN_MAIN_MARKING:		// 37: マーキング
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_MARKING_INIT) );

	case BOX2UI_PTIN_MAIN_FREE:				// 38: にがす
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_POKEFREE_INIT) );

	case BOX2UI_PTIN_MAIN_CLOSE:			// 39: とじる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_PARTYIN_MENU_CLOSE );

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_PTIN_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_PTIN_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_PARTYIN_MAIN) );

	case CURSORMOVE_CANCEL:				// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		// メニュー表示中
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_PARTYIN_MENU_CLOSE );
		}
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
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
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_PARTYIN_SELECT );

	case 1:
		// 手持ちに空きがあるか
		if( PokeParty_GetPokeCount( syswk->dat->pokeparty ) != TEMOTI_POKEMAX ){
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_PARTYIN_ACTION );
		}
		PMSND_PlaySE( SE_BOX2_WARNING );
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
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
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
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		BOX2MAIN_PokeDataMove( syswk );
		PokeMoveWorkFree( syswk );
		BOX2MAIN_PokeInfoOff( syswk );
		BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYIN_ACTION );

	case 3:
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		BOX2UI_PutHandCursor( syswk, syswk->get_pos );
		BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
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

	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );

	BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );

	if( syswk->get_pos != BOX2MAIN_GETPOS_NONE ){
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		pos = BOX2UI_PTIN_MAIN_TSURETEIKU;
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, pos );
	BOX2UI_PutHandCursor( syswk, pos );

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
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 5 );
			}
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return PartyPokeGetDrop( syswk, ret );
		}else{
			CURSORMOVE_PosSet( syswk->app->cmwk, ret-BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2UI_PutHandCursor( syswk, ret+BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2MAIN_PokeInfoOff( syswk );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_PARTYOUT_MAIN );
			}else{
				return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
			}
		}
	}

	ret = BOX2UI_PartyOutMain( syswk );

	switch( ret ){
	case BOX2UI_PTOUT_MAIN_RETURN1:		// 06: 戻る１
		PMSND_PlaySE( SE_BOX2_CLOSE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_PTOUT_MAIN_RETURN2:		// 07: 戻る２
		PMSND_PlaySE( SE_BOX2_CANCEL );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case BOX2UI_PTOUT_MAIN_AZUKERU:		// 08: あずける
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT) );

	case BOX2UI_PTOUT_MAIN_STATUS:		// 09: ようすをみる
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->cur_rcv_pos = BOX2UI_PTOUT_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_STATUS_CALL );

	case BOX2UI_PTOUT_MAIN_MARKING:		// 10: マーキング
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_MARKING_INIT) );

	case BOX2UI_PTOUT_MAIN_FREE:			// 11: にがす
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_POKEFREE_INIT) );

	case BOX2UI_PTOUT_MAIN_CLOSE:			// 12: とじる
		PMSND_PlaySE( SE_BOX2_CANCEL );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_PARTYOUT_MAIN) );

	case CURSORMOVE_CANCEL:						// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		// メニュー表示時
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_PARTYOUT_MENU_CLOSE );
		}
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
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
	BOX2UI_PutHandCursor( syswk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
	BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
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
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT );

	case 1:
		switch( PartyOutCheck(syswk) ){
		case 0:		// 預けられる
			syswk->poke_get_key = 1;
			BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeIconPriChg( syswk->app, BOX2OBJ_POKEICON_GET_POS, BOX2OBJ_POKEICON_PRI_CHG_GET );
			BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
			BOX2UI_PutHandCursor( syswk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
			syswk->app->sub_seq++;
			return VFuncSet( syswk, BOX2MAIN_VFuncPokeMoveGetKey, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_INIT );
		case 1:		// 戦えるポケモンがいなくなる
			PMSND_PlaySE( SE_BOX2_WARNING );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
			BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_ERR_RCV_MAIN;
			syswk->app->sub_seq = 0;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		case 2:		// メールを持っている
			PMSND_PlaySE( SE_BOX2_WARNING );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
			BOX2BMP_PokeSelectMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_MAIL, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_ERR_RCV_MAIN;
			syswk->app->sub_seq = 0;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}
		break;

	case 2:
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
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
	u32	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	if( ret == CURSORMOVE_NONE ){
		if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L ){
			BOX2MAIN_ChgCursorButton( syswk );
			ret = BOX2UI_PTOUT_BOXSEL_LEFT;
		}else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R ){
			BOX2MAIN_ChgCursorButton( syswk );
			ret = BOX2UI_PTOUT_BOXSEL_RIGHT;
		}
	}

	switch( ret ){
	case BOX2UI_PTOUT_BOXSEL_NAME:		// 00: トレイ名
	case BOX2UI_PTOUT_BOXSEL_TRAY:		// 01: トレイ
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTOUT_BOXSEL_NAME );
		if( BOXDAT_GetEmptySpaceTray( syswk->dat->sv_box, syswk->tray ) == 0 ){
			PMSND_PlaySE( SE_BOX2_WARNING );
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
			PMSND_PlaySE( SE_BOX2_DECIDE );
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );
		}
		break;

	case BOX2UI_PTOUT_BOXSEL_LEFT:		// 02: トレイ切り替え矢印・左
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTOUT_BOXSEL_NAME );
	case CURSORMOVE_NO_MOVE_LEFT:			// 十字キー左が押されたが、移動なし
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_MAIN );

	case BOX2UI_PTOUT_BOXSEL_RIGHT:		// 03: トレイ切り替え矢印・右
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTOUT_BOXSEL_NAME );
	case CURSORMOVE_NO_MOVE_RIGHT:		// 十字キー右が押されたが、移動なし
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_MAIN );


	case BOX2UI_PTOUT_BOXSEL_RETURN:	// 04: 戻る
	case CURSORMOVE_CANCEL:						// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		syswk->app->poke_put_key = BOX2MAIN_GETPOS_NONE;
		syswk->get_tray = syswk->tray;
		syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END) );

	case CURSORMOVE_CURSOR_ON:				// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		break;

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
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_ICON_RET;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );

	case SEQ_PTOUT_PUT_ICON_RET:
		PokeMoveWorkAlloc( syswk );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_CANCEL_END;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMovePutKey, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );

	case SEQ_PTOUT_PUT_PTFRM_IN:
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_END;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );

	case SEQ_PTOUT_PUT_ICON_PUT:
		GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
		PokeMoveWorkAlloc( syswk );
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_DATA_CHG;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMovePutKey2, BOX2SEQ_MAINSEQ_PARTYOUT_PUT_END );

	case SEQ_PTOUT_PUT_DATA_CHG:
		{
			BOX2MAIN_POKEMOVE_WORK * work;
			work = syswk->app->vfunk.work;
			BOX2MAIN_PokeDataMove( syswk );
			PokeMoveWorkFree( syswk );
		}
		BOX2MAIN_PokeInfoOff( syswk );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
		syswk->app->sub_seq = SEQ_PTOUT_PUT_PTFRM_IN;
		break;

	case SEQ_PTOUT_PUT_CANCEL_END:
		PokeMoveWorkFree( syswk );
	case SEQ_PTOUT_PUT_END:
		syswk->poke_get_key = 0;
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_MAIN, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
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

	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );

	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );

	if( syswk->get_pos == BOX2MAIN_GETPOS_NONE ){
		CURSORMOVE_PosSet( syswk->app->cmwk, 0 );
		BOX2UI_PutHandCursor( syswk, 0 );
		BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );
	}else{
		BOX2BMP_PokeSelectMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG1 );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTOUT_MAIN_AZUKERU );
		BOX2UI_PutHandCursor( syswk, BOX2UI_PTOUT_MAIN_AZUKERU );
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

	// セレクトボタン
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		int	next = ChgSeqSelectButton( syswk, BOX2SEQ_MAINSEQ_ITEMGET_INIT );
		if( syswk->mv_cnv_mode == 1 ){
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				syswk->get_pos = pos;
			}else{
				syswk->get_pos = 0;
			}
		}
		return next;
	}

	ret = BOX2UI_HitCheckTrgTrayPoke();
	if( ret != GFL_UI_TP_HIT_NONE ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return TrayItemGet( syswk, ret );
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, ret, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				return MainSeq_ItemMenuClose( syswk );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				BOX2UI_PutHandCursor( syswk, ret );
				return BOX2SEQ_MAINSEQ_ITEM_MAIN;
			}
		}
	}

	ret = BOX2UI_BoxItemArrangeMain( syswk );

	switch( ret ){
	case BOX2UI_ITEM_MAIN_NAME:		// 30: ボックス名
		PMSND_PlaySE( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );

	case BOX2UI_ITEM_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ITEM_MAIN_NAME );
		return TrayLeftScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_ITEM_MAIN );

	case BOX2UI_ITEM_MAIN_RIGHT:		// 32: トレイ切り替え矢印・右
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ITEM_MAIN_NAME );
		return TrayRightScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_ITEM_MAIN );

	case BOX2UI_ITEM_MAIN_PARTY:				// 33: 手持ちポケモン
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_INIT) );

	case BOX2UI_ITEM_MAIN_RETURN1:			// 34: 戻る１
		PMSND_PlaySE( SE_BOX2_CLOSE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_ITEM_MAIN_RETURN2:			// 35: 戻る２
		PMSND_PlaySE( SE_BOX2_CANCEL );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case BOX2UI_ITEM_MAIN_MENU1:				// 36: いどうする
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->mv_cnv_mode = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEMGET_INIT) );

	case BOX2UI_ITEM_MAIN_MENU2:				// 37: バッグへ or もたせる
		PMSND_PlaySE( SE_BOX2_DECIDE );
		if( syswk->app->get_item == ITEM_DUMMY_DATA ){
			syswk->subProcMode = SUB_PROC_MODE_MENU_BOX;
			syswk->cur_rcv_pos = BOX2UI_ITEM_MAIN_MENU2;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
		}else{
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
		}
		break;

	case BOX2UI_ITEM_MAIN_CLOSE:				// 38: とじる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ITEM_MENU_CLOSE );

	case BOX2UI_ITEM_MAIN_CONV:					// 39: 便利モード
		{
			int	next = ChgSeqSelectTouch( syswk, BOX2SEQ_MAINSEQ_ITEMGET_INIT );
			if( syswk->mv_cnv_mode == 1 ){
				u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
				if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					syswk->get_pos = pos;
				}else{
					syswk->get_pos = 0;
				}
			}
			return next;
		}
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ITEM_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_ITEM_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ITEM_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, TRUE, BOX2SEQ_MAINSEQ_ITEM_MAIN );
		}
		break;

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_MAIN) );

	case CURSORMOVE_CANCEL:					// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		// メニュー表示時
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ITEM_MENU_CLOSE );
		}
		syswk->dat->retMode = BOX_END_MODE_MENU;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
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

	syswk->next_seq = BOX2SEQ_MAINSEQ_ITEM_POKESET_RET;

	return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：バッグから持たせた後、アイコンを消す動作へ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_POKESET_ANM
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemPokeSetAnm( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
	}else{
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
	}
	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeIconClose, BOX2SEQ_MAINSEQ_ITEM_POKESET_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：アイコンを消した後、メイン処理へ復帰
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_ITEM_POKESET_RET
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemPokeSetRet( BOX2_SYS_WORK * syswk )
{
	int	next;

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );

	// カーソル位置をポケモン位置に
	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		BOX2UI_PutHandCursor( syswk, syswk->get_pos );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		next = BOX2SEQ_MAINSEQ_ITEM_MAIN;
	}else{
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
		BOX2UI_PutHandCursor( syswk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		next = BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}
	// カーソル表示
	BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
	BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );

	return next;
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
	int	pos;

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		pos = BOX2UI_ITEM_MAIN_MENU2;
	}else{
		pos = BOX2UI_ITEM_MAIN_MENU1;
	}
	CURSORMOVE_PosSet( syswk->app->cmwk, pos );
	BOX2UI_PutHandCursor( syswk, pos );
	BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );

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
		syswk->app->sub_seq++;
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );

		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			syswk->pokechg_mode = 1;
		}else{
			syswk->pokechg_mode = 0;
		}
		// セレクトボタン
		if( syswk->mv_cnv_mode == 1 ){
			syswk->get_tray = BOX2MAIN_GETPOS_NONE;
			syswk->app->get_item = ITEM_DUMMY_DATA;
			BOX2MAIN_ResetTouchBar( syswk );
			BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		}else{
			if( syswk->mv_cnv_mode == 2 ){
				if( syswk->app->get_item == ITEM_DUMMY_DATA ){
					syswk->get_tray = BOX2MAIN_GETPOS_NONE;
					syswk->mv_cnv_mode = 1;
				}
			}
			BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
			BOX2MAIN_ResetTouchBar( syswk );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
			BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
			return VFuncSet( syswk, BOX2MAIN_VFuncItemGetMenuClose, BOX2SEQ_MAINSEQ_ITEMGET_INIT );
		}

	case 1:
		syswk->app->sub_seq++;
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
			BOX2BGWFRM_PartyPokeFrameRightMoveSet( syswk->app->wfrm );
		}else{
			BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
			BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
			BOX2OBJ_PartyPokeIconFrmInSet( syswk, 0 );
		}
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ITEMGET_INIT );

	case 2:
		syswk->app->sub_seq = 0;
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
		BOX2OBJ_SetHandCursorAnm( syswk, BOX2OBJ_ANM_HAND_NORMAL );
		CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
		CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_STATUS );
		if( syswk->mv_cnv_mode == 1 ){
			BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_BLUE, TRUE );
			return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
		}else if( syswk->mv_cnv_mode == 2 ){
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_BLUE, TRUE );
		}else{
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
		}
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
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

	if( syswk->mv_cnv_end == 1 ){
		syswk->mv_cnv_mode = 0;
		syswk->mv_cnv_end = 0;
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ITEMGET_END );
	}

	// セレクトボタン
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		if( syswk->mv_cnv_mode != 0 && syswk->poke_get_key == 0 ){
			BOX2MAIN_ChgCursorButton( syswk );
			syswk->mv_cnv_mode = 0;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ITEMGET_END );
		}
	}

	if( syswk->poke_get_key == 0 ){
		// 手持ち
		ret = BOX2UI_HitCheckTrgPartyPokeRight();
		if( ret != GFL_UI_TP_HIT_NONE ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			ret += BOX2OBJ_POKEICON_TRAY_MAX;
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				BOX2MAIN_PokeInfoPut( syswk, ret );
				BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
				return BoxPartyItemGet( syswk, ret );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
			}
		}
		// ボックス
		ret = BOX2UI_HitCheckTrgTrayPoke();
		if( ret != GFL_UI_TP_HIT_NONE ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
			if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				BOX2MAIN_PokeInfoPut( syswk, ret );
				BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
				return BoxPartyItemGet( syswk, ret );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
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
			PMSND_PlaySE( SE_BOX2_DECIDE );
			BOX2MAIN_PokeInfoOff( syswk );
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_INIT );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_LEFT:		// 37: ボックス切り替え矢印（左）
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_PTGT_NAME );
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
		// 取得中
		}else{
			BOX2OBJ_MovePokeIconHand( syswk );
		}
		return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );

	case BOX2UI_ARRANGE_PTGT_RIGHT:		// 38: ボックス切り替え矢印（右）
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ARRANGE_PTGT_NAME );
		// 未取得
		if( syswk->poke_get_key == 0 ){
			BOX2MAIN_PokeSelectOff( syswk );
		// 取得中
		}else{
			BOX2OBJ_MovePokeIconHand( syswk );
		}
		return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );

	case BOX2UI_ARRANGE_PTGT_RET:		// 41: 戻る
		if( syswk->poke_get_key == 0 ){
			PMSND_PlaySE( SE_BOX2_CANCEL );
			syswk->mv_cnv_mode = 0;
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEMGET_END) );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_CONV:	// 42: 便利モード
		if( syswk->mv_cnv_mode != 0 && syswk->poke_get_key == 0 ){
			syswk->mv_cnv_mode = 0;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_ITEMGET_END );
		}
		break;

	case CURSORMOVE_CANCEL:					// キャンセル
		if( syswk->poke_get_key == 1 ){
			return BoxPartyItemPutKey( syswk, BOX2MAIN_GETPOS_NONE );
		}else{
			PMSND_PlaySE( SE_BOX2_CANCEL );
			syswk->mv_cnv_mode = 0;
			return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEMGET_END) );
		}
		break;

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PTGT_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_ARRANGE_PTGT_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_ITEMGET_MAIN );
		}
		break;

	case BOX2UI_ARRANGE_PTGT_BOXLIST:	// 39: ボックスリスト
	case BOX2UI_ARRANGE_PTGT_STATUS:	// 40: ステータス
	case CURSORMOVE_NO_MOVE_UP:				// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:			// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:							// 動作なし
		break;

	case CURSORMOVE_CURSOR_ON:				// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ITEMGET_END );

	case 1:
		syswk->app->sub_seq = 0;
		BOX2OBJ_SetHandCursorAnm( syswk, BOX2OBJ_ANM_HAND_NORMAL );
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
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

	if( syswk->mv_cnv_mode == 0 ){
		syswk->mv_cnv_end = 1;
	}

	BOX2OBJ_PokeCursorVanish( syswk, FALSE );

	mvwk = syswk->app->vfunk.work;

	// アイコンの移動先が元と同じ
	if( mvwk->put_pos == syswk->get_pos &&
			( syswk->get_tray == BOX2MAIN_GETPOS_NONE || syswk->get_tray == syswk->tray ) ){
		u32	set_pos = mvwk->set_pos;
		ItemMoveWorkFree( syswk );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		if( ITEM_CheckMail( syswk->app->get_item ) == TRUE && set_pos != syswk->get_pos ){
			PMSND_PlaySE( SE_BOX2_WARNING );
			BOX2BMP_MailMoveErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_CHG_ERROR;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}else{
			return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
		}
	}

	if( syswk->get_tray == syswk->tray || syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );
	}
	BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );

	// 移動先にアイテムをセット
	item = BOX2MAIN_PokeParaGet( syswk, mvwk->put_pos, syswk->tray, ID_PARA_item, NULL );
	BOX2MAIN_PokeParaPut( syswk, mvwk->put_pos, syswk->tray, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, mvwk->put_pos );
	if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
		BOX2MAIN_FormChangeRenew( syswk, mvwk->put_pos );
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
		if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
			BOX2MAIN_FormChangeRenew( syswk, syswk->get_pos );
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos,
				syswk->app->pokeicon_id[syswk->get_pos] );
		}
	}

	// 移動先がアイテムを持っていなかった
	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		syswk->get_pos = mvwk->put_pos;
		ItemMoveWorkFree( syswk );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );
	PMSND_PlaySE( SE_BOX2_POKE_CATCH );
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

	if( syswk->mv_cnv_mode == 0 ){
		syswk->mv_cnv_end = 1;
	}

	mvwk = syswk->app->vfunk.work;

	BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );

	get = syswk->get_pos;
	set = mvwk->set_pos;

	syswk->get_pos = mvwk->put_pos;

	ItemMoveWorkFree( syswk );

	item = syswk->app->get_item;
	syswk->app->get_item = ITEM_DUMMY_DATA;

	if( ITEM_CheckMail( item ) == TRUE && set != get ){
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_MailMoveErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_CHG_ERROR;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}

	BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );

	{
		u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
		if( pos < BOX2OBJ_POKEICON_PUT_MAX ){
			BOX2MAIN_PokeInfoPut( syswk, pos );
		}
	}

	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );

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
	if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == TRUE ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
	}
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

	if( syswk->mv_cnv_mode == 0 ){
		syswk->mv_cnv_end = 1;
	}

	// 初期位置と同じ
	if( syswk->app->poke_put_key == syswk->app->get_item_init_pos &&
			( syswk->get_tray == BOX2MAIN_GETPOS_NONE || syswk->get_tray == syswk->tray ) ){
		syswk->app->get_item = ITEM_DUMMY_DATA;
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

	// 取得位置が表示されているとき
	if( syswk->get_tray == syswk->tray || syswk->app->get_item_init_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->app->get_item_init_pos, TRUE );
	}
	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->app->poke_put_key, FALSE );

	// 持っているアイテムを取得
	item = BOX2MAIN_PokeParaGet( syswk, syswk->app->poke_put_key, syswk->tray, ID_PARA_item, NULL );
	// アイテムを持たせる
	BOX2MAIN_PokeParaPut( syswk, syswk->app->poke_put_key, syswk->tray, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->app->poke_put_key );
	if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
		BOX2MAIN_FormChangeRenew( syswk, syswk->app->poke_put_key );
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, syswk->app->poke_put_key,
			syswk->app->pokeicon_id[syswk->app->poke_put_key] );
	}
	BOX2MAIN_PokeInfoPut( syswk, syswk->app->poke_put_key );

	// 取得したアイテムを元のポケモンに持たせる
	syswk->app->get_item = item;
	BOX2MAIN_PokeParaPut( syswk, syswk->app->get_item_init_pos, syswk->get_tray, ID_PARA_item, syswk->app->get_item );
	if( syswk->get_tray == BOX2MAIN_GETPOS_NONE || syswk->get_tray == syswk->tray ){
		ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->app->get_item_init_pos );
		if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
			BOX2MAIN_FormChangeRenew( syswk, syswk->app->get_item_init_pos );
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->app->get_item_init_pos,
				syswk->app->pokeicon_id[syswk->app->get_item_init_pos] );
		}
	}

	// 移動先がアイテムを持っていなかったら
	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

	ItemMoveWorkAlloc( syswk );
	{
		BOX2MAIN_ITEMMOVE_WORK * mvwk = syswk->app->vfunk.work;
		mvwk->put_pos = syswk->app->poke_put_key;
	}

	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );
	PMSND_PlaySE( SE_BOX2_POKE_CATCH );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemIconChgTouch, BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_ICONMOVE_END );

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

	// セレクトボタン
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		int	next = ChgSeqSelectButton( syswk, BOX2SEQ_MAINSEQ_ITEMGET_INIT );
		if( syswk->mv_cnv_mode == 1 ){
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2OBJ_POKEICON_MINE_MAX ){
				syswk->get_pos = pos + BOX2OBJ_POKEICON_TRAY_MAX;
			}else{
				syswk->get_pos = BOX2OBJ_POKEICON_TRAY_MAX;
			}
		}
		return next;
	}

	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return PartyItemGet( syswk, ret );
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
				u8	pos = ret - BOX2OBJ_POKEICON_TRAY_MAX;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				return MainSeq_ItemPartyMenuClose( syswk );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret-BOX2OBJ_POKEICON_TRAY_MAX );
				BOX2UI_PutHandCursor( syswk, ret-BOX2OBJ_POKEICON_TRAY_MAX );
				return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
			}
		}
	}

	ret = BOX2UI_BoxItemArrangePartyMoveMain( syswk );

	switch( ret ){

	case BOX2UI_ITEM_PARTY_BOXLIST:		// 06: ボックスリスト
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END) );

	case BOX2UI_ITEM_PARTY_RETURN1:		// 07: 戻る１
		PMSND_PlaySE( SE_BOX2_CLOSE );
		syswk->dat->retMode = BOX_END_MODE_C_GEAR;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_END, 8, BOX2SEQ_MAINSEQ_BOXEND_DIRECT );

	case BOX2UI_ITEM_PARTY_RETURN2:		// 08: 戻る２
		PMSND_PlaySE( SE_BOX2_CANCEL );
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END) );

	case BOX2UI_ITEM_PARTY_MENU1:			// 09: いどうする
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->mv_cnv_mode = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEMGET_INIT) );

	case BOX2UI_ITEM_PARTY_MENU2:			// 10: バッグへ or もたせる
		PMSND_PlaySE( SE_BOX2_DECIDE );
		if( syswk->app->get_item == ITEM_DUMMY_DATA ){
			syswk->subProcMode = SUB_PROC_MODE_MENU_PARTY;
			syswk->cur_rcv_pos = BOX2UI_ITEM_PARTY_MENU2;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
		}else{
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
		}
		break;

	case BOX2UI_ITEM_PARTY_CLOSE:			// 11: とじる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ITEM_PARTY_MENU_CLOSE );

	case BOX2UI_ITEM_PARTY_CONV:			// 12: 便利モード
		{
			int	next = ChgSeqSelectTouch( syswk, BOX2SEQ_MAINSEQ_ITEMGET_INIT );
			if( syswk->mv_cnv_mode == 1 ){
				u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
				if( pos < BOX2OBJ_POKEICON_MINE_MAX ){
					syswk->get_pos = pos + BOX2OBJ_POKEICON_TRAY_MAX;
				}else{
					syswk->get_pos = BOX2OBJ_POKEICON_TRAY_MAX;
				}
			}
			return next;
		}
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN) );

	case CURSORMOVE_CANCEL:					// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		// メニュー表示中
		if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_ITEM_PARTY_MENU_CLOSE );
		}
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ARRANGE_PARTY_END) );

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
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
//		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ITEM_PARTY_MENU2 );
		BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}

	if( mvwk->put_pos == syswk->get_pos ){
		u32	set_pos = mvwk->set_pos;
		ItemMoveWorkFree( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU1 );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ITEM_PARTY_MENU1 );
		if( ITEM_CheckMail( syswk->app->get_item ) == TRUE && set_pos != syswk->get_pos ){
			BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
			BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_MAIL_MOVE, BOX2BMPWIN_ID_MSG1 );
			PMSND_PlaySE( SE_BOX2_WARNING );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
		}
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_INIT, BOX2BMPWIN_ID_MSG1 );
//		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU1 );
//		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}

	if( ITEM_CheckMail( syswk->app->get_item ) == TRUE ){
		u32	set_pos = mvwk->set_pos;
		ItemMoveWorkFree( syswk );
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_MAIL_MOVE, BOX2BMPWIN_ID_MSG1 );
		PMSND_PlaySE( SE_BOX2_WARNING );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_PARTY_MENU1 );
		BOX2UI_PutHandCursor( syswk, BOX2UI_ITEM_PARTY_MENU1 );
		BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
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
	if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
		BOX2MAIN_FormChangeRenew( syswk, mvwk->put_pos );
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
		if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
			BOX2MAIN_FormChangeRenew( syswk, syswk->get_pos );
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
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEM_PARTY_MAIN;
	}

	PMSND_PlaySE( SE_BOX2_POKE_CATCH );

	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncItemIconPartyChange, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_ITEM_PARTY_ICONMOVE_CHANGE) );
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
		PMSND_PlaySE( SE_BOX2_POKE_PUT );
		{
			BOX2MAIN_ITEMMOVE_WORK * mvwk = syswk->app->vfunk.work;
			BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );
		}
		// アウトライン消去
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		// アイテムアイコン縮小
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
		ItemMoveWorkFree( syswk );
		syswk->app->sub_seq++;
		break;

	case 1:
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
			break;
		}
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

	ret = BOX2UI_SleepMain( syswk );

	switch( ret ){
	case BOX2UI_SLEEP_MAIN_NAME:		// 30: ボックス名
		BOX2MAIN_PokeSelectOff( syswk );
		break;

	case BOX2UI_SLEEP_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_SLEEP_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_SLEEP_MAIN_NAME );
		return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_SLEEP_MAIN );

	case BOX2UI_SLEEP_MAIN_RIGHT:		// 32: トレイ切り替え矢印・右
		PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_SLEEP_MAIN_NAME );
		BOX2UI_PutHandCursor( syswk, BOX2UI_SLEEP_MAIN_NAME );
		return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_SLEEP_MAIN );

	case BOX2UI_SLEEP_MAIN_RETURN:	// 33: 戻る
		PMSND_PlaySE( SE_BOX2_CANCEL );
		syswk->dat->retMode = BOX_END_MODE_MENU;
		syswk->dat->retTray = BOX_RET_SEL_NONE;
		syswk->dat->retPoke = BOX_RET_SEL_NONE;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case BOX2UI_SLEEP_MAIN_SET:			// 34: ねかせる
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_SLEEP_MENU_SET) );

	case BOX2UI_SLEEP_MAIN_CLOSE:		// 35: やめる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_SLEEP_MENU_CLOSE );

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_SLEEP_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayLeftScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_SLEEP_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_SLEEP_MAIN_NAME ){
			PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );
			BOX2MAIN_PokeSelectOff( syswk );
			return TrayRightScrollSet( syswk, FALSE, BOX2SEQ_MAINSEQ_SLEEP_MAIN );
		}
		break;

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_SLEEP_MAIN) );

	case CURSORMOVE_CANCEL:					// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		// メニュー表示中
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, BOX2SEQ_MAINSEQ_SLEEP_MENU_CLOSE );
		}
		syswk->dat->retMode = BOX_END_MODE_MENU;
		syswk->dat->retTray = BOX_RET_SEL_NONE;
		syswk->dat->retPoke = BOX_RET_SEL_NONE;
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NONE:						// 動作なし
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
			PMSND_PlaySE( SE_BOX2_DECIDE );
			syswk->get_pos = ret;
			BOX2BMP_MenuStrPrint( syswk, SleepMenuStrTbl, 2 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_PokeCursorAdd( syswk );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_SLEEP_MAIN_SET );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				return MainSeq_SleepPokeSelect( syswk );
			}else{
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_SLEEP_MAIN_SET, ret );
				BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuMove, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_SLEEP_POKE_SELECT) );
			}
		}else{
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
				BOX2MAIN_ResetTouchBar( syswk );
				return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_SLEEP_MAIN );
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
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_SLEEP_MENU_SET );

	case 1:
		if( BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_tamago_flag, NULL ) != 0 ){
			BOX2BMP_PutPokeMoveErrMsg( syswk, BOX2MAIN_ERR_CODE_EGG, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
			return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
		}
		if( BOX2MAIN_CheckSleep( syswk, syswk->get_pos ) == FALSE ){
			BOX2BMP_PutPokeMoveErrMsg( syswk, BOX2MAIN_ERR_CODE_SLEEP, BOX2BMPWIN_ID_MSG4 );
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
		// バトルボックスがロックされている
		if( syswk->dat->callMode == BOX_MODE_BATTLE && syswk->dat->bbRockFlg == TRUE ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
			syswk->app->sub_seq = 3;
			return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ITEM_MENU_CHECK );
		}

		if( BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_tamago_flag, NULL ) == 0 ){
			u32 item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );
			// 何も持っていない
			if( item == ITEM_DUMMY_DATA ){
				return SubProcSet( syswk, SUB_PROC_TYPE_BAG );
			}
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, FALSE );
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
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2BMP_MailGetErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );

	case 2:		// タマゴ
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_EGG, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
		return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );

	case 3:		// バトルボックスがロックされている
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2BMP_BattleBoxRockMsgPut( syswk, 1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_BATTLEBOX_ROCK;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
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
	BOX2OBJ_SetHandCursorAnm( syswk, BOX2OBJ_ANM_HAND_NORMAL );
	if( syswk->mv_cnv_mode == 0 ){
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
	}else{
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_BLUE, TRUE );
	}

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

	BOX2MAIN_PokeInfoOff( syswk );

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

	case BOX_MODE_BATTLE:			// バトルボックス
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_MAIN, BOX2UI_BATTLEBOX_MAIN_NAME );
		syswk->app->old_cur_pos = BOX2UI_BATTLEBOX_MAIN_NAME;
		next = BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
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
	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		syswk->next_seq = BOX2SEQ_MAINSEQ_ITEM_POKESET_ANM;
	}else{
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	}
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
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_MARKING_INIT );

	case 1:
		syswk->app->pokeMark = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_mark, NULL );
		BOX2MAIN_MainDispMarkingChange( syswk, syswk->app->pokeMark );
		BOX2BGWFRM_MarkingFrameInSet( syswk->app->wfrm );
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncMarkingFrameMove, BOX2SEQ_MAINSEQ_MARKING_INIT );

	case 2:
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_MARKING, 0 );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
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
		PMSND_PlaySE( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 0 );
		break;
	case BOX2UI_MARKING_MARK2:		// 01: ▲
		PMSND_PlaySE( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 1 );
		break;
	case BOX2UI_MARKING_MARK3:		// 02: ■
		PMSND_PlaySE( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 2 );
		break;
	case BOX2UI_MARKING_MARK4:		// 03: ハート
		PMSND_PlaySE( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 3 );
		break;
	case BOX2UI_MARKING_MARK5:		// 04: ★
		PMSND_PlaySE( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 4 );
		break;
	case BOX2UI_MARKING_MARK6:		// 05: ◆
		PMSND_PlaySE( SE_BOX2_DECIDE );
		MarkingSwitch( syswk, 5 );
		break;

	case BOX2UI_MARKING_ENTER:		// 06:「けってい」
		{
			u8	mark = syswk->app->pokeMark;
			BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_mark, mark );
			BOX2MAIN_SubDispMarkingChange( syswk, mark );
		}
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return MarkingButtonAnmSet( syswk, 0, BOX2SEQ_MAINSEQ_MARKING_END );

	case BOX2UI_MARKING_CANCEL:	// 07:「やめる」
		PMSND_PlaySE( SE_BOX2_CANCEL );
		return MarkingButtonAnmSet( syswk, 1, BOX2SEQ_MAINSEQ_MARKING_END );

	case CURSORMOVE_CANCEL:			// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		return MarkingButtonAnmSet( syswk, 1, BOX2SEQ_MAINSEQ_MARKING_END );

	case CURSORMOVE_CURSOR_MOVE:	// 移動
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_MARKING_CURMOVE_RET );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTIN_MAIN, BOX2UI_PTIN_MAIN_MARKING );
		break;
	case BOX_MODE_SEIRI:
	default:
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_MARKING );
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_MARKING );
		}
		break;
	}
	BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	}
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );

	BOX2BGWFRM_MarkingFrameOutSet( syswk->app->wfrm );

	PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );

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
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, FALSE );
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
				PMSND_PlaySE( SE_BOX2_WARNING );
				BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
				syswk->app->poke_free_err = 1;
				syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
				return ChangeSequence( syswk, BOX2SEQ_MAINSEQ_TRGWAIT );
			}
			// メール
			pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos );
			item = PP_Get( pp, ID_PARA_item, NULL );
			if( ITEM_CheckMail( item ) == TRUE ){
				PMSND_PlaySE( SE_BOX2_WARNING );
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
				PMSND_PlaySE( SE_BOX2_WARNING );
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
				syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
			}else{
				syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
				BOX2BGWFRM_TemochiButtonOn( syswk->app );
			}

			CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
			BOX2UI_PutHandCursor( syswk, syswk->get_pos );
			BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );

			BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );

			BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );

			syswk->get_pos = BOX2MAIN_GETPOS_NONE;

			syswk->app->sub_seq = 0;
			return syswk->next_seq;
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
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
		BOX2UI_PutHandCursor( syswk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
		BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );

		syswk->get_pos = BOX2MAIN_GETPOS_NONE;
		if( syswk->dat->callMode == BOX_MODE_AZUKERU ){
			BOX2BMP_PokeSelectMsgPut(
				syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG1 );
			return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
		}
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
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
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, FALSE );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		// メニュー
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
			if( BOX2OBJ_VanishCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
				BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
			}
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
		syswk->mv_cnv_mode = 0;
		BOX2OBJ_SetHandCursorAnm( syswk, BOX2OBJ_ANM_HAND_NORMAL );
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
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_INIT) );

	case BOX2UI_BOXTHEMA_MENU2:			// 01: かべがみ
		PMSND_PlaySE( SE_BOX2_DECIDE );
		syswk->app->wp_menu = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );

	case BOX2UI_BOXTHEMA_MENU3:			// 02: なまえ
		PMSND_PlaySE( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_BOXTHEMA_NAMEIN_CALL );

	case BOX2UI_BOXTHEMA_MENU4:			// 03: やめる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT) );

	case CURSORMOVE_CANCEL:					// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT) );

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT );

	case 1:
		syswk->app->sub_seq = 0;
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );
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
		}else if( syswk->dat->callMode == BOX_MODE_BATTLE ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_MAIN, BOX2UI_BATTLEBOX_MAIN_NAME );
			return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
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
		syswk->app->jump_tray = 1;
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_INIT );

	case 1:
		syswk->box_mv_pos = syswk->tray;	// 現在参照しているボックス
		syswk->app->sub_seq++;
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
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
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
		return BOX2SEQ_MAINSEQ_TRAYSCROLL_TOUCH;
	}

	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	switch( ret ){
	case BOX2UI_BOXJUMP_TRAY1:		// 00: トレイアイコン
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		BOX2MAIN_InitTrayIconScroll( syswk, -1 );
		return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDownJump, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN );

	case BOX2UI_BOXJUMP_TRAY6:		// 05: トレイアイコン
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		}else{
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
			BOX2OBJ_ChangeTrayName( syswk, ret-BOX2UI_BOXJUMP_TRAY2, TRUE );
		}
		break;

	case BOX2UI_BOXJUMP_RET:			// 06: 戻る
	case CURSORMOVE_CANCEL:					// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		return ObjButtonAnmSet( syswk, BOX2OBJ_ID_TB_CANCEL, 9, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_EXIT) );

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, SetCursorMoveRet(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN) );

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_BOXJUMP_TRAY2 ){
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
			BOX2MAIN_InitTrayIconScroll( syswk, -1 );
			return VFuncSet( syswk, BOX2MAIN_VFuncTrayIconScrollDownJump, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_MAIN );
		}
		break;

	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
		if( CURSORMOVE_PosGet( syswk->app->cmwk ) == BOX2UI_BOXJUMP_TRAY5 ){
			PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		syswk->app->jump_tray = 0;
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG2 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		PMSND_PlaySE( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->app->sub_seq++;
		return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_BOXTHEMA_JUMP_EXIT );

	case 1:
		syswk->app->sub_seq = 0;
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
		BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );

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
		}else if( syswk->dat->callMode == BOX_MODE_BATTLE ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BATTLEBOX_MAIN, BOX2UI_BATTLEBOX_MAIN_NAME );
			return BOX2SEQ_MAINSEQ_BATTLEBOX_MAIN;
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
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		syswk->app->sub_seq++;
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT );

	case 1:
		if( syswk->app->wp_menu == 0 ){
			if( BOXDAT_GetExWallPaperFlag( syswk->dat->sv_box, BOX_EX_WALLPAPER_SET_FLAG_1 ) == TRUE ){
				BOX2BMP_MenuStrPrint( syswk, WallPaperMenuSpecialStrTbl, 6 );
			}else{
				BOX2BMP_MenuStrPrint( syswk, WallPaperMenuStrTbl, 5 );
			}
		}else if( syswk->app->wp_menu == 1 ){
			BOX2BMP_MenuStrPrint( syswk, WPLabel1MenuStrTbl, 5 );
		}else if( syswk->app->wp_menu == 2 ){
			BOX2BMP_MenuStrPrint( syswk, WPLabel2MenuStrTbl, 5 );
		}else if( syswk->app->wp_menu == 3 ){
			BOX2BMP_MenuStrPrint( syswk, WPLabel3MenuStrTbl, 5 );
		}else if( syswk->app->wp_menu == 4 ){
			BOX2BMP_MenuStrPrint( syswk, WPLabel4MenuStrTbl, 5 );
		}else if( syswk->app->wp_menu == 5 ){
			if( BOXDAT_GetExWallPaperFlag( syswk->dat->sv_box, BOX_EX_WALLPAPER_SET_FLAG_2 ) == TRUE ){
				BOX2BMP_MenuStrPrint( syswk, WPLabelS11MenuStrTbl, 6 );
			}else{
				BOX2BMP_MenuStrPrint( syswk, WPLabelS01MenuStrTbl, 5 );
			}
		}else if( syswk->app->wp_menu == 6 ){
			BOX2BMP_MenuStrPrint( syswk, WPLabelS02MenuStrTbl, 6 );
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
		{	// カーソル移動設定
			BOOL	flg = FALSE;
			// メニュー１を無効にする
			if( syswk->app->wp_menu == 0 ){
				if( BOXDAT_GetExWallPaperFlag( syswk->dat->sv_box, BOX_EX_WALLPAPER_SET_FLAG_1 ) == FALSE ){
					flg = TRUE;
				}
			}else if( syswk->app->wp_menu == 5 ){
				if( BOXDAT_GetExWallPaperFlag( syswk->dat->sv_box, BOX_EX_WALLPAPER_SET_FLAG_2 ) == FALSE ){
					flg = TRUE;
				}
			}else if( syswk->app->wp_menu != 6 ){
				flg = TRUE;
			}
			if( flg == TRUE ){
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_WALLPAPER_CHG, BOX2UI_WALLPAPER_MENU2 );
				CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_WALLPAPER_MENU1 );
			}else{
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_WALLPAPER_CHG, BOX2UI_WALLPAPER_MENU1 );
			}
		}
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
		PMSND_PlaySE( SE_BOX2_DECIDE );
		// トップメニュー
		if( syswk->app->wp_menu == 0 ){
			syswk->app->wp_menu = 5;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU1, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		return SetWallPaperChange( syswk, 0 );

	case BOX2UI_WALLPAPER_MENU2:			// 01: メニュー２
		PMSND_PlaySE( SE_BOX2_DECIDE );
		if( syswk->app->wp_menu == 0 ){
			syswk->app->wp_menu = 1;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		return SetWallPaperChange( syswk, 1 );

	case BOX2UI_WALLPAPER_MENU3:			// 02: メニュー３
		PMSND_PlaySE( SE_BOX2_DECIDE );
		if( syswk->app->wp_menu == 0 ){
			syswk->app->wp_menu = 2;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		return SetWallPaperChange( syswk, 2 );

	case BOX2UI_WALLPAPER_MENU4:			// 03: メニュー４
		PMSND_PlaySE( SE_BOX2_DECIDE );
		if( syswk->app->wp_menu == 0 ){
			syswk->app->wp_menu = 3;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		return SetWallPaperChange( syswk, 3 );

	case BOX2UI_WALLPAPER_MENU5:			// 04: メニュー５
		PMSND_PlaySE( SE_BOX2_DECIDE );
		// トップメニュー
		if( syswk->app->wp_menu == 0 ){
			syswk->app->wp_menu = 4;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		// スペシャル１
		if( syswk->app->wp_menu == 5 ){
			if( BOXDAT_GetExWallPaperFlag( syswk->dat->sv_box, BOX_EX_WALLPAPER_SET_FLAG_2 ) == TRUE ){
				syswk->app->wp_menu = 6;
				return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
			}
		}
		// スペシャル２
		if( syswk->app->wp_menu == 6 ){
			syswk->app->wp_menu = 5;
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );
		}
		return SetWallPaperChange( syswk, 4 );

	case BOX2UI_WALLPAPER_MENU6:			// 05: やめる
		PMSND_PlaySE( SE_BOX2_CANCEL );
		if( syswk->app->wp_menu == 0 ){
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT) );
		}
		syswk->app->wp_menu = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );

	case CURSORMOVE_CANCEL:						// キャンセル
		PMSND_PlaySE( SE_BOX2_CANCEL );
		if( syswk->app->wp_menu == 0 ){
			return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_EXIT) );
		}
		syswk->app->wp_menu = 0;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU6, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_INIT) );

	case CURSORMOVE_CURSOR_MOVE:			// 移動
		PMSND_PlaySE( SE_BOX2_MOVE_CURSOR );
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
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
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
		BOX2OBJ_SetHandCursorOnOff( syswk, TRUE );
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
 * @brief		メインシーケンス：ボックス終了「はい・いいえ」セット
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	定義：BOX2SEQ_MAINSEQ_BOXEND_INIT
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxEndInit( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, FALSE );
	BOX2BMP_BoxEndMsgPut( syswk, BOX2BMPWIN_ID_MSG1 );
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
		if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
			BOX2MAIN_FormChangeRenew( syswk, syswk->get_pos );
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos,
				syswk->app->pokeicon_id[syswk->get_pos] );
		}
		BOX2MAIN_PokeInfoRewrite( syswk, syswk->get_pos );
	}else{
		PMSND_PlaySE( SE_BOX2_WARNING );
		BOX2BMP_ItemGetErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
	}

	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
	}else{
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
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

	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
	}else{
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
	}

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

	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
	}else{
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
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
	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
	}else{
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
	}

	if( MYITEM_AddItem( syswk->dat->myitem, syswk->app->get_item, 1, HEAPID_BOX_APP ) == TRUE ){
		POKEMON_PASO_PARAM * ppp;
		u16	item;

		ppp  = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
		item = ITEM_DUMMY_DATA;

		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, item );

		if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
			BOX2MAIN_FormChangeRenew( syswk, syswk->get_pos );
			BOX2OBJ_PokeIconChange( syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
		}
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		return BOX2SEQ_MAINSEQ_ITEM_BAGIN_ANM;
	}

	PMSND_PlaySE( SE_BOX2_WARNING );
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
	PMSND_PlaySE( SE_BOX2_LOG_OFF );
	BOX2MAIN_CheckPartyPerapu( syswk );			// ペラップボイス
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

	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
	}else{
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	}

	BOX2OBJ_PutConvButton( syswk, BOX2OBJ_ANM_CONV_RED, TRUE );

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
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
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
	BOX2MAIN_CheckPartyPerapu( syswk );			// ペラップボイス
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
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );

	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
	}else{
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	}

	return BOX2SEQ_MAINSEQ_SLEEP_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理：メニューに戻る（汎用）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int YesNo_MenuRcv( BOX2_SYS_WORK * syswk )
{
	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
	}else{
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );
	}
	return MainSeq_ArrangePokeMenuRcv( syswk );
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
	BOX2MAIN_InitBgButtonAnm( syswk, wfrmID );
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
	// カーソルの初期状態を設定
	if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == TRUE ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
	}else{
		GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
	}
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

	if( ITEM_CheckMail( syswk->subRet ) == FALSE ){
		POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, syswk->subRet );
		if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
			BOX2MAIN_FormChangeRenew( syswk, syswk->get_pos );
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
		}
		MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP_L );
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );

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
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
		next = BOX2SEQ_MAINSEQ_ITEM_MAIN;
		break;

	case SUB_PROC_MODE_MENU_PARTY:		// 手持ちのポケモンのメニューから
		PartyFrmSet( syswk );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
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

	if( ITEM_CheckMail( syswk->subRet ) == FALSE ){
		POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, syswk->subRet );
		if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
			BOX2MAIN_FormChangeRenew( syswk, syswk->get_pos );
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

	BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );

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

	if( ITEM_CheckMail( syswk->subRet ) == FALSE ){
		POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, syswk->subRet );
		if( BOX2MAIN_PokeItemFormChange( syswk, ppp ) == TRUE ){
			BOX2MAIN_FormChangeRenew( syswk, syswk->get_pos );
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
		}
		MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP_L );
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	BOX2OBJ_SetHandCursorOnOff( syswk, FALSE );

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
		// 手持ち取得モードから開始したとき
		if( syswk->get_start_mode == 1 ){
			BOX2OBJ_PartyPokeIconFrmInSetRight( syswk );
		}
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
		BOX2MAIN_InitBoxMoveFrameScroll( syswk );
		while( BOX2MAIN_VFuncBoxMoveFrmIn(syswk) ){}
		BOX2OBJ_EndTrayCursorScroll( syswk );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;

	case SUB_PROC_MODE_TB_BOX_GET:		// ボックスのポケモンのタッチバーから（掴んでいる時）
		// 手持ち取得時は他の手持ちアイコンを右フレームイン前の位置に移動
		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2OBJ_PartyPokeIconFrmInSetRight( syswk );
			BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		}else if( syswk->tray == syswk->get_tray ){
			BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		}else{
			BOX2OBJ_PokeIconChange( syswk, syswk->get_tray, syswk->get_pos, syswk->app->pokeicon_id[BOX2OBJ_POKEICON_GET_POS] );
		}
		BOX2MAIN_HandGetPokeSet( syswk );
		BOX2OBJ_ChgPokeCursorPriority( syswk, BOX2OBJ_POKEICON_GET_POS );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->tb_status_pos );
		BOX2MAIN_InitBoxMoveFrameScroll( syswk );
		while( BOX2MAIN_VFuncBoxMoveFrmIn(syswk) ){}
		BOX2OBJ_EndTrayCursorScroll( syswk );
		BOX2BGWFRM_TemochiButtonOn( syswk->app );
		if( syswk->tb_status_pos >= BOX2UI_ARRANGE_PGT_TRAY2 && syswk->tb_status_pos <= BOX2UI_ARRANGE_PGT_TRAY5 ){
			BOX2OBJ_ChangeTrayName( syswk, syswk->tb_status_pos-BOX2UI_ARRANGE_PGT_TRAY2, TRUE );
		}
		BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );
		return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;

	case SUB_PROC_MODE_TB_PARTY:			// 手持ちのポケモンのタッチバーから
		PartyFrmSetRight( syswk );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;

	case SUB_PROC_MODE_TB_PARTY_GET:	// 手持ちのポケモンのタッチバーから（掴んでいる時）
		PartyFrmSetRight( syswk );
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX && syswk->tray != syswk->get_tray ){
			BOX2OBJ_PokeIconChange( syswk, syswk->get_tray, syswk->get_pos, syswk->app->pokeicon_id[BOX2OBJ_POKEICON_GET_POS] );
		}else{
			BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		}
		BOX2MAIN_HandGetPokeSet( syswk );
		BOX2OBJ_ChgPokeCursorPriority( syswk, BOX2OBJ_POKEICON_GET_POS );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->tb_status_pos );
		BOX2BGWFRM_BoxListButtonOn( syswk->app );
		BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );
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
		CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;

	case SUB_PROC_MODE_TB_PARTY_GET:	// 手持ちのポケモンのタッチバーから（掴んでいる時）
		PartyFrmSetRight( syswk );
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX && syswk->tray != syswk->get_tray ){
			BOX2OBJ_PokeIconChange( syswk, syswk->get_tray, syswk->get_pos, syswk->app->pokeicon_id[BOX2OBJ_POKEICON_GET_POS] );
		}else{
			BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
		}
		BOX2MAIN_HandGetPokeSet( syswk );
		BOX2OBJ_ChgPokeCursorPriority( syswk, BOX2OBJ_POKEICON_GET_POS );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->tb_status_pos );
		CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
		BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );
		return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_POKEGET_MAIN;
	}

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ切り替え・左
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		btnFlg	ボタンフラグ
 * @param		next		切り替え後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayLeftScrollSet( BOX2_SYS_WORK * syswk, BOOL btnFlg, int next )
{
	BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_L );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		if( btnFlg == TRUE ){
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
		}
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncTrayScrollLeft, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ切り替え・右
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		btnFlg	ボタンフラグ
 * @param		next		切り替え後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayRightScrollSet( BOX2_SYS_WORK * syswk, BOOL btnFlg, int next )
{
	BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_R );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
		BOX2MAIN_ResetTouchBar( syswk );
		if( btnFlg == TRUE ){
			BOX2BGWFRM_TemochiButtonOn( syswk->app );
		}
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
	syswk->get_start_mode = 0;
	syswk->mv_cnv_mode = 0;

	syswk->get_pos  = pos;
	syswk->get_tray = syswk->tray;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	PokeMoveWorkAlloc( syswk );

	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2BGWFRM_TemochiButtonOn( syswk->app );

	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_TRAY;
	syswk->next_seq  = BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;	// 掴んだままボックス移動に移行する場合はここにいく
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
	syswk->get_start_mode = 0;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	PokeMoveWorkAlloc( syswk );

	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );

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

	PokeMoveWorkAlloc( syswk );
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

	syswk->get_start_mode = 1;
	syswk->mv_cnv_mode = 0;

	syswk->get_pos  = pos;
	syswk->get_tray = syswk->tray;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos-BOX2OBJ_POKEICON_TRAY_MAX );

	PokeMoveWorkAlloc( syswk );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	BOX2BGWFRM_BoxListButtonOn( syswk->app );
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
	syswk->get_start_mode = 1;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos-BOX2OBJ_POKEICON_TRAY_MAX );

	PokeMoveWorkAlloc( syswk );
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

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, BOX2OBJ_POKEICON_GET_POS, FALSE );

	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos - BOX2OBJ_POKEICON_TRAY_MAX );

	PokeMoveWorkAlloc( syswk );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
	return VFuncSet( syswk, BOX2MAIN_GetPartyPokeMoveDrop, BOX2SEQ_MAINSEQ_PARTYOUT_POKECHG_END );
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

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );

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
	syswk->app->old_cur_pos = pos;

	BOX2OBJ_GetPokeIcon( syswk->app, syswk->get_pos );
	BOX2OBJ_PokeIconPriChg( syswk->app, BOX2OBJ_POKEICON_GET_POS, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );

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
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );

	if( syswk->dat->callMode == BOX_MODE_BATTLE ){
		BOX2BGWFRM_TemochiButtonOff( syswk->app );
		BOX2BGWFRM_BoxListButtonOff( syswk->app );
		CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
	}

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
	BOX2OBJ_PokeCursorAdd2( syswk, BOX2OBJ_POKEICON_GET_POS );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_ON );

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMoveGetKey, BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「つかむ」で取得したポケモンを配置
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
			PMSND_PlaySE( SE_BOX2_WARNING );
			return BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_MAIN;
		}
		// 選択したトレイがいっぱい
		if( BOXDAT_GetEmptySpaceTray( syswk->dat->sv_box, tray ) == 0 ){
			PMSND_PlaySE( SE_BOX2_WARNING );
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
	syswk->app->vfunk.work = GFL_HEAP_AllocClearMemory( HEAPID_BOX_APP, sizeof(BOX2MAIN_POKEMOVE_WORK) );
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
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
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

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
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
		return BOX2SEQ_MAINSEQ_ITEMGET_MAIN;
	}

	PMSND_PlaySE( SE_BOX2_POKE_CATCH );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );
	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );

	{
		s16	x, y;

		BOX2OBJ_PokeIconDefaultPosGet( syswk->get_pos, &x, &y, BOX2MAIN_POKEMOVE_MODE_ALL );
		syswk->app->tpx = x + 8;
		syswk->app->tpy = y + 8;
	}

	ItemMoveWorkAlloc( syswk );

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

	syswk->poke_get_key = 1;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	BOX2OBJ_ItemIconPokePutHand( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );

	BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );

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

	BOX2OBJ_PokeCursorVanish( syswk, FALSE );

	if( syswk->app->poke_put_key == BOX2MAIN_GETPOS_NONE ){
		return BoxPartyItemPutKeyCancel( syswk );
	}else if( BOX2MAIN_PokeItemMoveCheck( syswk, syswk->get_pos, pos ) == FALSE ){
		return BoxPartyItemPutKeyCancel( syswk );
	}

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
	syswk->get_pos  = pos;
	syswk->get_tray = BOX2MAIN_GETPOS_NONE;

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, syswk->tray, ID_PARA_item, NULL );

	BOX2BGWFRM_PokeMenuOff( syswk->app->wfrm );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_ResetTouchBar( syswk );

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );
		BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
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

	PMSND_PlaySE( SE_BOX2_CHANGE_TRAY );

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
	if( syswk->app->wp_menu <= 4 ||
			( syswk->app->wp_menu == 5 && BOXDAT_GetExWallPaperFlag(syswk->dat->sv_box,BOX_EX_WALLPAPER_SET_FLAG_2) == FALSE ) ){
		syswk->app->wallpaper_pos = ( syswk->app->wp_menu - 1 ) * 4 + pos - 1;
	}else{
		syswk->app->wallpaper_pos = ( syswk->app->wp_menu - 1 ) * 4 + pos;
	}
	return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU1+pos, ChangeSequence(syswk,BOX2SEQ_MAINSEQ_BOXTHEMA_WALLPAPER_CHANGE) );
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		セレクトボタンで便利モードへ移行
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		次のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int ChgSeqSelectButton( BOX2_SYS_WORK * syswk, int next )
{
	PMSND_PlaySE( SE_BOX2_DECIDE );
	if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
		syswk->mv_cnv_mode = 2;
	}else{
		syswk->mv_cnv_mode = 1;
	}
	syswk->mv_cnv_end = 0;
	BOX2MAIN_ChgCursorButton( syswk );
	return ChangeSequence( syswk, next );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチで便利モードへ移行
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		next		次のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int ChgSeqSelectTouch( BOX2_SYS_WORK * syswk, int next )
{
	PMSND_PlaySE( SE_BOX2_DECIDE );
	if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
		syswk->mv_cnv_mode = 2;
	}else{
		syswk->mv_cnv_mode = 1;
	}
	syswk->mv_cnv_end = 0;
	return ChangeSequence( syswk, next );
}
