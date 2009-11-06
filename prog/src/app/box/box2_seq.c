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

/*
#if 0
//↑[GS_CONVERT_TAG]

#include "gflib/touchpanel.h"
#include "system/wipe.h"
#include "system/snd_tool.h"
#include "msgdata/msg_boxmenu.h"

#include "box2_main.h"
#include "box2_seq.h"
#include "box2_bmp.h"
#include "box2_obj.h"
#include "box2_tp.h"
#include "box2_key.h"
#include "box2_snd_def.h"





//#define	BTS_5436_TEST
#ifdef	BTS_5436_TEST
static u32 test_5436_flg = 0;
#endif	// BTS_5436_TEST
*/


//============================================================================================
//	定数定義
//============================================================================================

// サブプロセスタイプ
enum {
	SUB_PROC_TYPE_POKESTATUS = 0,	// ステータス画面
	SUB_PROC_TYPE_BAG,				// バッグ画面
	SUB_PROC_TYPE_NAMEIN,			// 文字入力
};

// サブプロセス設定関数
typedef struct {
	pBOX2_FUNC	call;	// 呼び出し
	pBOX2_FUNC	exit;	// 終了
	u32	rcvSeq;			// 復帰シーケンス
}SUBGFL_PROC_DATA;
//↑[GS_CONVERT_TAG]

// はい・いいえ処理
typedef struct {
	pBOX2_FUNC	yes;
	pBOX2_FUNC	no;
}YESNO_DATA;

// はい・いいえID
enum {
	YESNO_ID_ITEMGET = 0,		// 道具預かり
	YESNO_ID_POKEFREE,			// 逃がす
	YESNO_ID_ITEM_RET_BAG,		// アイテムしまう
	YESNO_ID_BOX_END,			// ボックス終了（ボタン）
	YESNO_ID_BOX_END_CANCEL,	// ボックス終了（キャンセル）
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

static int MainSeq_ArrangeMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeTrayPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeMenuRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeStatusRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeStrInRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBagRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeItemSetMsg( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMarkingInit( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMarkingFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMarkingStart( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMarking( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMarkingRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeFreeStart( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeFreeAnm( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeFreeEnter1( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeFreeEnter2( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeFreeEnter3( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeFreeError1( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeFreeError2( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePokeFreeError3( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyPokeFreeEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyPokeFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyStart( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyPokeFrmOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxPartyButtonIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxPartyChgStart( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxPartyChgMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxPartyPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxPartyPokeChgEndKey( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxPartyChgError( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyFrmMoveREnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyFrmMoveLEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxMoveFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeExitButtonFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxMoveFrmOutSet( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxMoveFrmOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxRetButtonIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaChgMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaButtonOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaTrayIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaExitIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaMenuIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaMsgSet( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaExitOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaMenuOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaTrayOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxThemaButtonIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperMenuIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperChgStart( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperChgMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperMenuOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperFrmOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperChange( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperWhiteOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperWhiteIn( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMainCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxPartyChgMainCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyMainCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxArrangePokeMenuRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxArrangePartyMenuRcv( BOX2_SYS_WORK * syswk );

static int MainSeq_PartyInMain( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInMainRcv( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInPartyFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInPokeMoveSet( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInPartyFrmOut( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInRetFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInMainCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInMenuCloseEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInPokeMenuRcv( BOX2_SYS_WORK * syswk );

static int MainSeq_PartyOutMain( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutStartCheck( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutStartError( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutStartRetFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutMenuFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutBoxSelectStart( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutBoxSelectMain( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutBoxMenuFrmOut( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutEndRetFrmOut( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutBoxMoveFrmOut( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutEndRetFrmIn( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutEnterButton( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutCheck( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutEnterPartyFrmRight( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutEnterTrayScroll( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutEnterPokeMove( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutEnterPartyFrmLeft( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutEnterRetIn( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutComp( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutPokeChgEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutMainCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutMenuCloseEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutBoxSelCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutPokeMenuRcv( BOX2_SYS_WORK * syswk );

static int MainSeq_MarkingCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaChgCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_WallPaperChgCurMoveRet( BOX2_SYS_WORK * syswk );

static int MainSeq_KeyGetPokeMoveMain( BOX2_SYS_WORK * syswk );
static int MainSeq_KeyGetPokeMoveMainCurMoveRet( BOX2_SYS_WORK * syswk );

static int MainSeq_ItemArrangeMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeMainCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeRetBagSet( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeRetBagCheck( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeRetBagEnter( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeRetBagCancel( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeRetBagEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeGetBagCheck( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeItemSetMsg( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeIconMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeIconMovePokeAdd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeIconMoveChange( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeIconMoveChangeEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeBoxPartyChgMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeBoxPartyChgMainCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeBoxPartyIconMoveSubFrmOut( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeBoxPartyIconMovePokeAdd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeBoxPartyIconMoveEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeBoxPartyChgError( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeKeyGetMoveMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeKeyGetMoveMainCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeKeyGetChangeCkack( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyMain( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyMainCurMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyIconMoveRet( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyIconMovePokeAdd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyIconMovePokeAddEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyIconMoveChange( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyIconMoveChangeEnd( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyIconMoveErr( BOX2_SYS_WORK * syswk );

static int MainSeq_BoxEndInit( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxEndCancelInit( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMoveButton( BOX2_SYS_WORK * syswk );
static int MainSeq_StatusButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemButton( BOX2_SYS_WORK * syswk );
static int MainSeq_MarkingButton( BOX2_SYS_WORK * syswk );
static int MainSeq_MarkingEndButton( BOX2_SYS_WORK * syswk );
static int MainSeq_FreeButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeCloseButton( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyChgButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangePartyCloseButton( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyEndButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeBoxMoveButton( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxMoveEndButton( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaTrayChgButton( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaWallPaperButton( BOX2_SYS_WORK * syswk );
static int MainSeq_BoxThemaNameButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperChgButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeWallPaperChgCancelButton( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInButton( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyInCloseButton( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutButton( BOX2_SYS_WORK * syswk );
static int MainSeq_PartyOutCloseButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeMoveButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeBagButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeCloseButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangeBoxMoveButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemChgButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ItemArrangePartyCloseButton( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeItemSetEggError( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeItemGetMailError( BOX2_SYS_WORK * syswk );
static int MainSeq_ArrangeMoveMenuButton( BOX2_SYS_WORK * syswk );

static int FadeInSet( BOX2_SYS_WORK * syswk, int next );
static int FadeOutSet( BOX2_SYS_WORK * syswk, int next );
static int WaitSet( BOX2_SYS_WORK * syswk, int next, int wait );
static int YesNoSet( BOX2_SYS_WORK * syswk, u32 id );
static int VFuncSet( BOX2_SYS_WORK * syswk, void * func, int next );
static int VFuncReqSet( BOX2_SYS_WORK * syswk, int next );
static int SubProcSet( BOX2_SYS_WORK * syswk, u8 type );

static int TrayLeftScrollSet( BOX2_SYS_WORK * syswk, int next );
static int TrayRightScrollSet( BOX2_SYS_WORK * syswk, int next );
static int TrayPokeGet( BOX2_SYS_WORK * syswk, u32 pos );
static int PartyPokeGet( BOX2_SYS_WORK * syswk, u32 pos );
static void PokeMoveWorkAlloc( BOX2_SYS_WORK * syswk );
static void PokeMoveWorkFree( BOX2_SYS_WORK * syswk );
static int PokeItemCheck( BOX2_SYS_WORK * syswk );
static int MarkingScrollOutSet( BOX2_SYS_WORK * syswk );
static int MarkingStartSet( BOX2_SYS_WORK * syswk );
static int PokeFreeStartSet( BOX2_SYS_WORK * syswk );
static int BoxPartyPokeGet( BOX2_SYS_WORK * syswk, u32 pos );
static int BoxPartyButtonOutSet( BOX2_SYS_WORK * syswk );
static int PartyFrmOutSet( BOX2_SYS_WORK * syswk );
static void PartyFrmSet( BOX2_SYS_WORK * syswk );
static void PartyFrmSetRight( BOX2_SYS_WORK * syswk );
static void PartyFrmSet_PartyOut( BOX2_SYS_WORK * syswk );
static int PartyFrmMoveRight( BOX2_SYS_WORK * syswk );
static int PartyFrmMoveLeft( BOX2_SYS_WORK * syswk );
static int BoxMoveButtonOutSet( BOX2_SYS_WORK * syswk );
static int BoxMoveEndStart( BOX2_SYS_WORK * syswk );
static int BoxMoveMenuFrmIn( BOX2_SYS_WORK * syswk, u32 ret );
static int BoxMoveMenuFrmOut( BOX2_SYS_WORK * syswk, u32 pos );
static int BoxMoveButtonFrmIn( BOX2_SYS_WORK * syswk, u32 ret );
static int BoxMoveButtonFrmOut( BOX2_SYS_WORK * syswk, int next );
static int YStatusButtonFrmOut( BOX2_SYS_WORK * syswk, int next );
static int BoxMoveTrayScrollSet( BOX2_SYS_WORK * syswk, u32 mode );
static void BoxMoveNameChange( BOX2_SYS_WORK * syswk, u32 ret );
static void BoxMoveTrayIconChgOnly( BOX2_SYS_WORK * syswk, s8 mv );
static int BoxMoveFrmInSet( BOX2_SYS_WORK * syswk, int next );
static int BoxMoveFrmOutSet( BOX2_SYS_WORK * syswk, int next );
static int BoxMoveExitButtonInSet( BOX2_SYS_WORK * syswk, int next );
static int BoxMoveExitButtonOutSet( BOX2_SYS_WORK * syswk, int next );
static int PokeMenuOutSet( BOX2_SYS_WORK * syswk, int next );
static int BoxThemaChgStart( BOX2_SYS_WORK * syswk );
static int BoxThemaTrayIconSel( BOX2_SYS_WORK * syswk, u32 ret );
static int BoxThemaTrayIconChg( BOX2_SYS_WORK * syswk, s8 mv );
static void WallPaperPageChange( BOX2_SYS_WORK * syswk, s32 mv );
static void WallPaperPosSet( BOX2_SYS_WORK * syswk, u32 pos );
static int WallPaperChange( BOX2_SYS_WORK * syswk );
static int PartyInPokeCheck( BOX2_SYS_WORK * syswk );
static int PartyOutInit( BOX2_SYS_WORK * syswk );
static int PartyOutEnd( BOX2_SYS_WORK * syswk, BOOL flg );
static int PokeOutTraySelect( BOX2_SYS_WORK * syswk, u32 ret );
static int PokeOutTrayIconChg( BOX2_SYS_WORK * syswk, s8 mv );
static int PartyOutEnter( BOX2_SYS_WORK * syswk );

static int BoxArg_ItemGetYes( BOX2_SYS_WORK * syswk );
static int BoxArg_PokeFreeYes( BOX2_SYS_WORK * syswk );
static int BoxItemArg_RetBagYes( BOX2_SYS_WORK * syswk );
static int BoxArg_BoxEndYes( BOX2_SYS_WORK * syswk );
static int BoxArg_BoxEndNo( BOX2_SYS_WORK * syswk );

static int PartyPokeGetDrop( BOX2_SYS_WORK * syswk, u32 pos );
static int TrayPokeGetDrop( BOX2_SYS_WORK * syswk, u32 pos );

static int PartyPokeGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static int PartyPokeGetKeyArrange( BOX2_SYS_WORK * syswk, u32 pos );

static int TrayPokeGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static int TrayPokeGetKeyArrange( BOX2_SYS_WORK * syswk, u32 pos );

static int PokeMoveGetKey( BOX2_SYS_WORK * syswk, u32 pos );

static int TrayItemGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static int TrayItemGet( BOX2_SYS_WORK * syswk, u32 pos );

static int PartyItemGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static int PartyItemGet( BOX2_SYS_WORK * syswk, u32 pos );

static int ItemArrangeMenuStart( BOX2_SYS_WORK * syswk );

static void ItemMoveWorkAlloc( BOX2_SYS_WORK * syswk );
static void ItemMoveWorkFree( BOX2_SYS_WORK * syswk );

static int BoxPartyItemGet( BOX2_SYS_WORK * syswk, u32 pos );
static int BoxPartyItemGetKey( BOX2_SYS_WORK * syswk, u32 pos );
static int BoxPartyItemPutKey( BOX2_SYS_WORK * syswk, u32 pos );

static int BgButtonAnmSet( BOX2_SYS_WORK * syswk, u32 wfrmID, int next );
static int YStatusButtonAnmSet( BOX2_SYS_WORK * syswk, u32 wfrmID, int next );
static int MarkingButtonAnmSet( BOX2_SYS_WORK * syswk, u32 no, int next );
static int PartyButtonAnmSet( BOX2_SYS_WORK * syswk, u32 no, int next );


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

	MainSeq_ArrangeMain,
	MainSeq_ArrangeTrayPokeChgEnd,
	MainSeq_ArrangePokeMenuRcv,
	MainSeq_ArrangeStatusRcv,
	MainSeq_ArrangeStrInRcv,
	MainSeq_ArrangeBagRcv,
	MainSeq_ArrangeItemSetMsg,
	MainSeq_ArrangeMarkingInit,
	MainSeq_ArrangeMarkingFrmIn,
	MainSeq_ArrangeMarkingStart,
	MainSeq_ArrangeMarking,
	MainSeq_ArrangeMarkingRcv,
	MainSeq_MarkingCurMoveRet,
	MainSeq_ArrangePokeFreeStart,
	MainSeq_ArrangePokeFreeAnm,
	MainSeq_ArrangePokeFreeEnter1,
	MainSeq_ArrangePokeFreeEnter2,
	MainSeq_ArrangePokeFreeEnter3,
	MainSeq_ArrangePokeFreeError1,
	MainSeq_ArrangePokeFreeError2,
	MainSeq_ArrangePokeFreeError3,
	MainSeq_ArrangePartyPokeFreeEnd,
	MainSeq_ArrangePartyPokeFrmIn,
	MainSeq_ArrangePartyStart,
	MainSeq_ArrangePartyMain,
	MainSeq_ArrangePartyPokeFrmOut,
	MainSeq_ArrangeBoxPartyButtonIn,
	MainSeq_ArrangePartyPokeChgEnd,
	MainSeq_ArrangeBoxPartyChgStart,
	MainSeq_ArrangeBoxPartyChgMain,
	MainSeq_ArrangeBoxPartyPokeChgEnd,
	MainSeq_ArrangeBoxPartyPokeChgEndKey,
	MainSeq_ArrangeBoxPartyChgError,
	MainSeq_ArrangePartyFrmMoveREnd,
	MainSeq_ArrangePartyFrmMoveLEnd,
	MainSeq_ArrangeBoxMoveFrmIn,
	MainSeq_ArrangeExitButtonFrmIn,
	MainSeq_ArrangeBoxMoveFrmOutSet,
	MainSeq_ArrangeBoxMoveFrmOut,
	MainSeq_ArrangeBoxRetButtonIn,
	MainSeq_ArrangeBoxThemaButtonOut,
	MainSeq_ArrangeBoxThemaTrayIn,
	MainSeq_ArrangeBoxThemaExitIn,
	MainSeq_ArrangeBoxThemaMenuIn,
	MainSeq_ArrangeBoxThemaMsgSet,
	MainSeq_ArrangeBoxThemaExitOut,
	MainSeq_ArrangeBoxThemaMenuOut,
	MainSeq_ArrangeBoxThemaTrayOut,
	MainSeq_ArrangeBoxThemaButtonIn,
	MainSeq_ArrangeBoxThemaChgMain,
	MainSeq_BoxThemaChgCurMoveRet,
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
	MainSeq_ArrangeMainCurMoveRet,
	MainSeq_ArrangeBoxPartyChgMainCurMoveRet,
	MainSeq_ArrangePartyMainCurMoveRet,
	MainSeq_BoxArrangePokeMenuRcv,
	MainSeq_BoxArrangePartyMenuRcv,
	MainSeq_ArrangeItemSetEggError,
	MainSeq_ArrangeItemGetMailError,

	MainSeq_PartyInMain,
	MainSeq_PartyInMainRcv,
	MainSeq_PartyInPartyFrmIn,
	MainSeq_PartyInPokeMoveSet,
	MainSeq_PartyInPartyFrmOut,
	MainSeq_PartyInRetFrmIn,
	// つれていく仕様変更　7/28
	MainSeq_PartyInPokeChgEnd,
	MainSeq_PartyInMainCurMoveRet,
	MainSeq_PartyInMenuCloseEnd,
	MainSeq_PartyInPokeMenuRcv,

	MainSeq_PartyOutMain,
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
	// あずける仕様変更　7/28
	MainSeq_PartyOutPokeChgEnd,
	MainSeq_PartyOutMainCurMoveRet,
	MainSeq_PartyOutMenuCloseEnd,
	MainSeq_PartyOutBoxSelCurMoveRet,
	MainSeq_PartyOutPokeMenuRcv,

	MainSeq_KeyGetPokeMoveMain,
	MainSeq_KeyGetPokeMoveMainCurMoveRet,

	// 道具整理
	MainSeq_ItemArrangeMain,
	MainSeq_ItemArrangeMainCurMoveRet,
	MainSeq_ItemArrangeRetBagSet,
	MainSeq_ItemArrangeRetBagCheck,
	MainSeq_ItemArrangeRetBagEnter,
	MainSeq_ItemArrangeRetBagCancel,
	MainSeq_ItemArrangeRetBagEnd,
	MainSeq_ItemArrangeGetBagCheck,
	MainSeq_ItemArrangeItemSetMsg,
	MainSeq_ItemArrangeIconMoveRet,
	MainSeq_ItemArrangeIconMovePokeAdd,
	MainSeq_ItemArrangeIconMoveChange,
	MainSeq_ItemArrangeIconMoveChangeEnd,
	MainSeq_ItemArrangeBoxPartyChgMain,
	MainSeq_ItemArrangeBoxPartyChgMainCurMoveRet,
	MainSeq_ItemArrangeBoxPartyIconMoveSubFrmOut,
	MainSeq_ItemArrangeBoxPartyIconMovePokeAdd,
	MainSeq_ItemArrangeBoxPartyIconMoveEnd,
	MainSeq_ItemArrangeBoxPartyChgError,
	MainSeq_ItemArrangeKeyGetMoveMain,
	MainSeq_ItemArrangeKeyGetMoveMainCurMoveRet,
	MainSeq_ItemArrangeKeyGetChangeCkack,
	MainSeq_ItemArrangePartyMain,
	MainSeq_ItemArrangePartyMainCurMoveRet,
	MainSeq_ItemArrangePartyIconMoveRet,
	MainSeq_ItemArrangePartyIconMovePokeAdd,
	MainSeq_ItemArrangePartyIconMovePokeAddEnd,
	MainSeq_ItemArrangePartyIconMoveChange,
	MainSeq_ItemArrangePartyIconMoveChangeEnd,
	MainSeq_ItemArrangePartyIconMoveErr,

	MainSeq_BoxEndInit,
	MainSeq_BoxEndCancelInit,
	MainSeq_ArrangePartyButton,
	MainSeq_ArrangeMoveButton,
	MainSeq_StatusButton,
	MainSeq_ItemButton,
	MainSeq_MarkingButton,
	MainSeq_MarkingEndButton,
	MainSeq_FreeButton,
	MainSeq_ArrangeCloseButton,
	MainSeq_PartyChgButton,
	MainSeq_ArrangePartyCloseButton,
	MainSeq_PartyEndButton,
	MainSeq_ArrangeBoxMoveButton,
	MainSeq_BoxMoveEndButton,
	MainSeq_BoxThemaTrayChgButton,
	MainSeq_BoxThemaWallPaperButton,
	MainSeq_BoxThemaNameButton,
	MainSeq_ArrangeWallPaperChgButton,
	MainSeq_ArrangeWallPaperChgCancelButton,
	MainSeq_PartyInButton,
	MainSeq_PartyInCloseButton,
	MainSeq_PartyOutButton,
	MainSeq_PartyOutCloseButton,
	MainSeq_ItemArrangePartyButton,
	MainSeq_ItemArrangeMoveButton,
	MainSeq_ItemArrangeBagButton,
	MainSeq_ItemArrangeCloseButton,
	MainSeq_ItemArrangeBoxMoveButton,
	MainSeq_ItemChgButton,
	MainSeq_ItemArrangePartyCloseButton,
	MainSeq_ArrangeMoveMenuButton,
};

// サブプロセス設定
static const SUBGFL_PROC_DATA SubProcFunc[] =
//↑[GS_CONVERT_TAG]
{
	// ステータス画面
	{ BOX2MAIN_PokeStatusCall, BOX2MAIN_PokeStatusExit, BOX2SEQ_MAINSEQ_ARRANGE_STATUS_RCV },
	// バッグ画面
	{ BOX2MAIN_BagCall, BOX2MAIN_BagExit, BOX2SEQ_MAINSEQ_ARRANGE_BAG_RCV },
	// 文字入力
	{ BOX2MAIN_NameInCall, BOX2MAIN_NameInExit, BOX2SEQ_MAINSEQ_ARRANGE_STRIN_RCV },
};

// はい・いいえデータテーブル
static const YESNO_DATA YesNoFunc[] =
{
	// ボックス整理・道具を預かる？
	{ BoxArg_ItemGetYes, MainSeq_ArrangePokeMenuRcv },
	// ボックス整理・ポケモンを逃がす？
	{ BoxArg_PokeFreeYes, MainSeq_ArrangePokeFreeError3 },
	// 道具整理・アイテムしまう？
	{ BoxItemArg_RetBagYes, MainSeq_ItemArrangeRetBagCancel },
	// ボックス終了する？（ボタン）
	{ BoxArg_BoxEndYes, BoxArg_BoxEndNo },
	// ボックス終了する？（キャンセル）
	{ BoxArg_BoxEndNo, BoxArg_BoxEndYes },
};

// メニュー文字列テーブル：ボックス整理メイン
static const BOX2BMP_BUTTON_LIST PokeMenuStrTbl[] = {
	{ mes_boxbutton_01_02, BOX2BMP_BUTTON_TYPE_RED },		// ポケモンいどう
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_02, BOX2BMP_BUTTON_TYPE_WHITE },		// もちもの
	{ mes_boxbutton_02_03, BOX2BMP_BUTTON_TYPE_WHITE },		// マーキング
	{ mes_boxbutton_02_04, BOX2BMP_BUTTON_TYPE_WHITE }		// にがす
};

// メニュー文字列テーブル：つれていく
static const BOX2BMP_BUTTON_LIST PartyInMenuStrTbl[] = {
	{ mes_boxbutton_02_05, BOX2BMP_BUTTON_TYPE_WHITE },		// つれていく
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_03, BOX2BMP_BUTTON_TYPE_WHITE },		// マーキング
	{ mes_boxbutton_02_04, BOX2BMP_BUTTON_TYPE_WHITE }		// にがす
};

// メニュー文字列テーブル：あずける
static const BOX2BMP_BUTTON_LIST PartyOutMenuStrTbl[] = {
	{ mes_boxbutton_02_06, BOX2BMP_BUTTON_TYPE_WHITE },		// あずける
	{ mes_boxbutton_02_01, BOX2BMP_BUTTON_TYPE_WHITE },		// ようすをみる
	{ mes_boxbutton_02_03, BOX2BMP_BUTTON_TYPE_WHITE },		// マーキング
	{ mes_boxbutton_02_04, BOX2BMP_BUTTON_TYPE_WHITE }		// にがす
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
		BOX2BGWFRM_SubDispWinFrmMove( syswk );
		BOX2BMP_PrintUtilTrans( syswk->app );
		BOX2OBJ_AnmMain( syswk->app );
	}

	return TRUE;
}



//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボックス画面初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	次のシーケンス
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

	if( syswk->dat->callMode != BOX_MODE_TURETEIKU && syswk->dat->callMode != BOX_MODE_AZUKERU ){
		BOX2BGWFRM_ButtonPutTemochi( syswk );
		BOX2BGWFRM_ButtonPutIdou( syswk );
	}
	BOX2BGWFRM_ButtonPutModoru( syswk );

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

	GFL_ARC_CloseDataHandle( syswk->app->pokeicon_ah );
//	GFL_ARC_CloseDataHandle( syswk->app->ppd_ah );

	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	GFL_HEAP_FreeMemory( syswk->app );
	syswk->app = NULL;

	GFL_HEAP_DeleteHeap( HEAPID_BOX_APP );

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
 * メインシーケンス：パレットフェード
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
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
 * メインシーケンス：ウェイト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
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
 * メインシーケンス：VBlank処理終了待ち
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_VFunc( BOX2_SYS_WORK * syswk )
{
	if( syswk->app->vfunk.func != NULL ){
		pBOX2_FUNC func = syswk->app->vfunk.func;
		if( func( syswk ) == 0 ){
			syswk->app->vfunk.func = NULL;
			return MainSeq[syswk->next_seq]( syswk );
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
 * メインシーケンス：キー・タッチ待ち
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
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
 * メインシーケンス：はい・いいえ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_YesNo( BOX2_SYS_WORK * syswk )
{
/*
	switch( TOUCH_SW_Main( syswk->app->tsw ) ){
	case TOUCH_SW_RET_YES:
		TOUCH_SW_Reset( syswk->app->tsw );
		return YesNoFunc[syswk->app->ynID].yes( syswk );

	case TOUCH_SW_RET_NO:
		TOUCH_SW_Reset( syswk->app->tsw );
		return YesNoFunc[syswk->app->ynID].no( syswk );
	}
*/
	APP_TASKMENU_UpdateMenu( syswk->app->ynWork );
	if( APP_TASKMENU_IsFinish( syswk->app->ynWork ) == TRUE ){
		APP_TASKMENU_CloseMenu( syswk->app->ynWork );
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
 * メインシーケンス：ボタンアニメ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
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
 * メインシーケンス：サブプロセス呼び出し
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SubProcCall( BOX2_SYS_WORK * syswk )
{
	SubProcFunc[syswk->subProcType].call( syswk );
	return BOX2SEQ_MAINSEQ_SUB_PROCMAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：サブプロセス終了待ち
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SubProcMain( BOX2_SYS_WORK * syswk )
{
/*
	if( ProcMain( syswk->subProcFunc ) == FALSE ){
		return BOX2SEQ_MAINSEQ_SUB_PROCMAIN;
	}

	GFL_PROC_Delete( syswk->subProcFunc );
//↑[GS_CONVERT_TAG]
	SubProcFunc[syswk->subProcType].exit( syswk );
	syswk->next_seq = SubProcFunc[syswk->subProcType].rcvSeq;
*/
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
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Start( BOX2_SYS_WORK * syswk )
{
	int	seq;

//	seq = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;

//	Snd_SePlay( SE_BOX2_LOG_IN );

	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:		// あずける
		PartyFrmSet_PartyOut( syswk );
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );

		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );

		BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );

		seq = BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
		break;

	case BOX_MODE_TURETEIKU:	// つれていく
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
		BOX2MAIN_PokeInfoPut( syswk, 0 );
		seq = BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
		break;

	case BOX_MODE_SEIRI:		// ボックスせいり
		BOX2MAIN_PokeInfoPut( syswk, 0 );
		seq = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		break;

	case BOX_MODE_ITEM:			// アイテム整理
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		BOX2MAIN_PokeInfoPut( syswk, 0 );
		seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
		break;
	}

	return FadeInSet( syswk, seq );
}


//============================================================================================
//	ボックス整理
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス整理メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitTrayPokeTrg();
	if( ret != GFL_UI_TP_HIT_NONE ){
		OS_Printf( "get_pos = %d\n", ret );
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
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
				BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
				BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
				BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuCloseKeyArrange, BOX2SEQ_MAINSEQ_ARRANGE_MAIN_CURMOVE_RET );
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
		BOX2MAIN_PokeSelectOff( syswk );
		return BoxThemaChgStart( syswk );

	case BOX2UI_ARRANGE_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_NAME );
		return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );

	case BOX2UI_ARRANGE_MAIN_RIGHT:	// 32: トレイ切り替え矢印・右
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_NAME );
		return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );

	case BOX2UI_ARRANGE_MAIN_PARTY:	// 33: 手持ちポケモン
//		Snd_SePlay( SE_BOX2_OPEN_PARTY_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_BUTTON );

	case BOX2UI_ARRANGE_MAIN_CHANGE:	// 34: ポケモン移動
//		Snd_SePlay( SE_BOX2_OPEN_PARTY_TRAY );
		syswk->quick_mode = 0;
		syswk->quick_get  = 0;
		return YStatusButtonAnmSet( syswk, BOX2MAIN_WINFRM_MV_BTN, BOX2SEQ_MAINSEQ_ARRANGE_MOVE_BUTTON );

	case BOX2UI_ARRANGE_MAIN_MOVE:		// 37: ポケモンいどう（メニュー）
//		Snd_SePlay( SE_BOX2_DECIDE );
		if( GFL_UI_TP_GetTrg() == FALSE ){
//↑[GS_CONVERT_TAG]
			syswk->quick_mode = 1;
		}
		syswk->quick_get = syswk->get_pos;
		return YStatusButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU1, BOX2SEQ_MAINSEQ_ARRANGE_MOVE_MENU_BUTTON );

	case BOX2UI_ARRANGE_MAIN_STATUS:	// 38: ようすをみる
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->cur_rcv_pos = BOX2UI_ARRANGE_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, BOX2SEQ_MAINSEQ_STATUS_BUTTON );

	case BOX2UI_ARRANGE_MAIN_ITEM:		// 39: もちもの
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_ITEM_BUTTON );

	case BOX2UI_ARRANGE_MAIN_MARKING:	// 40: マーキング
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, BOX2SEQ_MAINSEQ_MARKING_BUTTON );

	case BOX2UI_ARRANGE_MAIN_FREE:		// 41: にがす
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_FREE_BUTTON );

	case BOX2UI_ARRANGE_MAIN_CLOSE:	// 42: とじる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_CLOSE_BTN, BOX2SEQ_MAINSEQ_ARRANGE_CLOSE_BUTTON );

	case CURSORMOVE_NONE:				// 動作なし
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos == BOX2UI_ARRANGE_MAIN_NAME ){
				if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
//					Snd_SePlay( SE_BOX2_CHANGE_TRAY );
					BOX2MAIN_PokeSelectOff( syswk );
					return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
				}
				if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
//					Snd_SePlay( SE_BOX2_CHANGE_TRAY );
					BOX2MAIN_PokeSelectOff( syswk );
					return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
				}
			}
		}
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_ARRANGE_MAIN_MOVE &&
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
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_MAIN_CURMOVE_RET );

	case BOX2UI_ARRANGE_MAIN_RETURN:	// 35: もどる
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_ARRANGE_MAIN_MOVE &&
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
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ARRANGE_MAIN_MOVE, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_MOVE );
			return TrayPokeGetKeyArrange( syswk, ret );
		}
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：トレイのポケモン移動後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeTrayPokeChgEnd( BOX2_SYS_WORK * syswk )
{
//	Snd_SePlay( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
	PokeMoveWorkFree( syswk );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MAIN_MOVE );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモンメニュー表示状態へ復帰
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeMenuRcv( BOX2_SYS_WORK * syswk )
{
	int	next;

	BOX2BMP_SysWinVanishTrans( syswk->app, BOX2BMPWIN_ID_MSG1 );

	switch( syswk->dat->callMode ){
	case BOX_MODE_TURETEIKU:
		BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 4 );
		BOX2BMP_PokePartyInMsgPut(
			syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
		next = BOX2SEQ_MAINSEQ_PARTYIN_POKE_MENU_RCV;
		break;

	case BOX_MODE_AZUKERU:
		BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 4 );
		BOX2BMP_PokePartyInMsgPut(
			syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
		next = BOX2SEQ_MAINSEQ_PARTYOUT_POKE_MENU_RCV;
		break;

	case BOX_MODE_SEIRI:
	default:
		BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			next = BOX2SEQ_MAINSEQ_BOXARRANGE_POKE_MENU_RCV;
		}else{
			next = BOX2SEQ_MAINSEQ_BOXARRANGE_PARTY_MENU_RCV;
		}
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

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, next );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ステータス画面からの復帰
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int RcvStatus_ModeTureteiku( BOX2_SYS_WORK * syswk )
{
	int	next;

	if( syswk->y_status_flg == 0 ){
		BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 4 );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );

		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2BMP_PokePartyInMsgPut(
				syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
			next = BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
		}else{
			PartyFrmSet( syswk );
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_STATUS );
			next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
		}

		BOX2OBJ_PokeCursorAdd( syswk );
		BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );

	}else{
		PartyFrmSetRight( syswk );
		BOX2BGWFRM_YStatusButtonPut( syswk->app->wfrm );

		BOX2OBJ_BoxMoveObjInit( syswk );
		BOX2OBJ_BoxMoveCursorSet( syswk );
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2BMP_BoxMoveNameNumWrite( syswk );
		BOX2BGWFRM_BoxMoveFrmInSet( syswk->app->wfrm );
		while( BOX2MAIN_VFuncBoxMoveFrmIn(syswk) ){}

		if( syswk->y_status_hand == 0 ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
			next = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->y_status_pos );
			syswk->app->poke_get_key = 1;
			BOX2MAIN_HandGetPokeSet( syswk );
			BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );
			next = BOX2SEQ_MAINSEQ_KEYGET_POKEMOVE_MAIN;
		}
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;

		if( syswk->pokechg_mode == 1 ){
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		}

		syswk->y_status_flg  = 0;
		syswk->y_status_pos  = 0;
		syswk->y_status_hand = 0;
	}

	return next;
}
static int RcvStatus_ModeAzukeru( BOX2_SYS_WORK * syswk )
{
	int	next;

	if( syswk->y_status_flg == 0 ){
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 4 );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );

		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}else{
			PartyFrmSet_PartyOut( syswk );
			BOX2BMP_PokePartyInMsgPut(
				syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
			next = BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
		}

		BOX2OBJ_PokeCursorAdd( syswk );
		BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );

	}else{
		PartyFrmSetRight( syswk );
		BOX2BGWFRM_YStatusButtonPut( syswk->app->wfrm );

		BOX2OBJ_BoxMoveObjInit( syswk );
		BOX2OBJ_BoxMoveCursorSet( syswk );
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2BMP_BoxMoveNameNumWrite( syswk );
		BOX2BGWFRM_BoxMoveFrmInSet( syswk->app->wfrm );
		while( BOX2MAIN_VFuncBoxMoveFrmIn(syswk) ){}

		if( syswk->y_status_hand == 0 ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
			next = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->y_status_pos );
			syswk->app->poke_get_key = 1;
			BOX2MAIN_HandGetPokeSet( syswk );
			BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );
			next = BOX2SEQ_MAINSEQ_KEYGET_POKEMOVE_MAIN;
		}
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;

		if( syswk->pokechg_mode == 1 ){
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		}

		syswk->y_status_flg  = 0;
		syswk->y_status_pos  = 0;
		syswk->y_status_hand = 0;
	}

	return next;
}
static int RcvStatus_ModeSeiri( BOX2_SYS_WORK * syswk )
{
	int	next;

	if( syswk->y_status_flg == 0 ){
		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			if( syswk->box_mv_flg != 0 ){
				BOX2BGWFRM_BoxMoveFrmPut( syswk );
			}
		}else{
			if( syswk->box_mv_flg != 0 ){
				BOX2BGWFRM_BoxMoveFrmPut( syswk );
			}
		}
		BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
		BOX2BGWFRM_ArrangeUnderButtonDel( syswk );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );

		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}else{
			PartyFrmSet( syswk );
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_STATUS );
			next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
		}

		BOX2OBJ_PokeCursorAdd( syswk );
		BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );

	}else{
		PartyFrmSetRight( syswk );
		BOX2BGWFRM_YStatusButtonPut( syswk->app->wfrm );

		BOX2OBJ_BoxMoveObjInit( syswk );
		BOX2OBJ_BoxMoveCursorSet( syswk );
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2BMP_BoxMoveNameNumWrite( syswk );
		BOX2BGWFRM_BoxMoveFrmInSet( syswk->app->wfrm );
		while( BOX2MAIN_VFuncBoxMoveFrmIn(syswk) ){}

		if( syswk->y_status_hand == 0 ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
			next = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->y_status_pos );
			syswk->app->poke_get_key = 1;
			BOX2MAIN_HandGetPokeSet( syswk );
			BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );
			next = BOX2SEQ_MAINSEQ_KEYGET_POKEMOVE_MAIN;
		}
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;

		if( syswk->pokechg_mode == 1 ){
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		}

		syswk->y_status_flg  = 0;
		syswk->y_status_pos  = 0;
		syswk->y_status_hand = 0;
	}

	return next;
}
static int RcvStatus_ModeItem( BOX2_SYS_WORK * syswk )
{
	int	next;

	if( syswk->y_status_flg == 0 ){
		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			if( syswk->box_mv_flg != 0 ){
				BOX2BGWFRM_BoxMoveFrmPut( syswk );
			}
		}else{
			if( syswk->box_mv_flg != 0 ){
				BOX2BGWFRM_BoxMoveFrmPut( syswk );
			}
		}
		BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
		BOX2BGWFRM_ArrangeUnderButtonDel( syswk );
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );

		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}else{
			PartyFrmSet( syswk );
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_STATUS );
			next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
		}

		BOX2OBJ_PokeCursorAdd( syswk );

	}else{
		PartyFrmSetRight( syswk );
		BOX2BGWFRM_YStatusButtonPut( syswk->app->wfrm );

		BOX2OBJ_BoxMoveObjInit( syswk );
		BOX2OBJ_BoxMoveCursorSet( syswk );
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2BMP_BoxMoveNameNumWrite( syswk );
		BOX2BGWFRM_BoxMoveFrmInSet( syswk->app->wfrm );
		while( BOX2MAIN_VFuncBoxMoveFrmIn(syswk) ){}

		if( syswk->y_status_hand == 0 ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->y_status_pos );
			syswk->app->poke_get_key = 1;
			BOX2MAIN_HandGetPokeSet( syswk );
		}
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;

		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
			next = BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
		}

		if( syswk->pokechg_mode == 1 ){
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		}

		syswk->y_status_flg  = 0;
		syswk->y_status_pos  = 0;
		syswk->y_status_hand = 0;
	}

	return next;
}

static int MainSeq_ArrangeStatusRcv( BOX2_SYS_WORK * syswk )
{
/*
	u8	next;

#ifdef	BTS_5436_TEST
	test_5436_flg = 1;
#endif	// BTS_5436_TEST

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	if( syswk->y_status_flg == 0 ){
		if( syswk->dat->callMode == BOX_MODE_TURETEIKU ){
			BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 4 );
		}else if( syswk->dat->callMode == BOX_MODE_AZUKERU ){
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 4 );
		}else{
			if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
				if( syswk->box_mv_flg != 0 ){
					BOX2BGWFRM_BoxMoveFrmPut( syswk );
				}
			}else{
				if( syswk->box_mv_flg != 0 ){
					BOX2BGWFRM_BoxMoveFrmPut( syswk );
				}
			}
			BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
			BOX2BGWFRM_ArrangeUnderButtonDel( syswk );
		}
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );

		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			if( syswk->dat->callMode == BOX_MODE_TURETEIKU ){
				BOX2BMP_PokePartyInMsgPut(
					syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
				next = BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
			}else{
				next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
			}
		}else{
			if( syswk->dat->callMode == BOX_MODE_AZUKERU ){
				PartyFrmSet_PartyOut( syswk );
				BOX2BMP_PokePartyInMsgPut(
					syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
				next = BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
			}else{
				PartyFrmSet( syswk );
				BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_STATUS );
				next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
			}
		}

		BOX2OBJ_PokeCursorAdd( syswk );

		if( syswk->dat->callMode != BOX_MODE_ITEM ){
			BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );
		}

	}else{
		PartyFrmSetRight( syswk );
		BOX2BGWFRM_YStatusButtonPut( syswk->app->wfrm );

		BOX2OBJ_BoxMoveObjInit( syswk );
		BOX2OBJ_BoxMoveCursorSet( syswk );
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2BMP_BoxMoveNameNumWrite( syswk );
		BOX2BGWFRM_BoxMoveFrmInSet( syswk->app->wfrm );
		while( BOX2MAIN_VFuncBoxMoveFrmIn(syswk) ){}

		if( syswk->y_status_hand == 0 ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->y_status_pos );
			syswk->app->poke_get_key = 1;
			BOX2MAIN_HandGetPokeSet( syswk );
		}
		syswk->app->old_cur_pos = BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;

		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
			next = BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
		}else{
			if( syswk->y_status_hand == 0 ){
				next = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
			}else{
				next = BOX2SEQ_MAINSEQ_KEYGET_POKEMOVE_MAIN;
			}
		}

		if( syswk->dat->callMode != BOX_MODE_ITEM && syswk->y_status_hand != 0 ){
			BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );
		}

		if( syswk->pokechg_mode == 1 ){
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		}

		syswk->y_status_flg  = 0;
		syswk->y_status_pos  = 0;
		syswk->y_status_hand = 0;
	}

	return FadeInSet( syswk, next );
*/
	int	next;

#ifdef	BTS_5436_TEST
	test_5436_flg = 1;
#endif	// BTS_5436_TEST

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
	case BOX_MODE_ITEM:				// どうぐせいり
		next = RcvStatus_ModeItem( syswk );
		break;
	}

	return FadeInSet( syswk, next );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：文字入力画面からの復帰
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeStrInRcv( BOX2_SYS_WORK * syswk )
{
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BOXTHEMA_CHG, BOX2UI_BOXTHEMA_CHG_NANE );
	syswk->app->old_cur_pos = syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;

	BOX2BGWFRM_BoxThemaFrmPut( syswk );
	BOX2BMP_BoxThemaMsgPut( syswk, BOX2BMP_MSGID_THEMA_INIT, BOX2BMPWIN_ID_MSG3 );

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	}

	return FadeInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：バッグ画面からの復帰
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int RcvBag_ModeAzukeru( BOX2_SYS_WORK * syswk )
{
	int	next;

	BOX2OBJ_PokeCursorAdd( syswk );

	if( syswk->subRet == 0 ){
		BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_ITEM );
			next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
		}
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
	}else{
		BOOL err = FALSE;
		if( syswk->subRet == ITEM_HAKKINDAMA &&
			BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_monsno, NULL ) != MONSNO_GIRATHINA ){
			err = TRUE;
		}else{
			POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

			BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, syswk->subRet );
			if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
				BOX2OBJ_PokeIconChange(
					syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
			}
			MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP );
		}

		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_ITEM );
		}
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

		if( err == TRUE ){
			syswk->subRet = 0;	// 下で参照するけど、むしろ０のほうが良い
		}
		next = BOX2SEQ_MAINSEQ_ARRANGE_ITEMSET_MSG;
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );

	return next;
}

static int RcvBag_ModeTureteiku( BOX2_SYS_WORK * syswk )
{
	int	next;

	BOX2OBJ_PokeCursorAdd( syswk );

	if( syswk->subRet == 0 ){
		BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_ITEM );
			next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
		}
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
	}else{
		BOOL err = FALSE;
		if( syswk->subRet == ITEM_HAKKINDAMA &&
			BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_monsno, NULL ) != MONSNO_GIRATHINA ){
			err = TRUE;
		}else{
			POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

			BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, syswk->subRet );
			if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
				BOX2OBJ_PokeIconChange(
					syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
			}
			MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP );
		}

		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_ITEM );
		}
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

		if( err == TRUE ){
			syswk->subRet = 0;	// 下で参照するけど、むしろ０のほうが良い
		}
		next = BOX2SEQ_MAINSEQ_ARRANGE_ITEMSET_MSG;
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );

	return next;
}

static int RcvBag_ModeSeiri( BOX2_SYS_WORK * syswk )
{
	int	next;

	BOX2OBJ_PokeCursorAdd( syswk );

	BOX2BGWFRM_ArrangeUnderButtonDel( syswk );
	BOX2BGWFRM_RetButtonOutPut( syswk->app->wfrm );

	if( syswk->subRet == 0 ){
		BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}else{
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_ITEM );
			next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
		}
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
	}else{
		BOOL err = FALSE;
		if( syswk->subRet == ITEM_HAKKINDAMA &&
			BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_monsno, NULL ) != MONSNO_GIRATHINA ){
			err = TRUE;
		}else{
			POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

			BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, syswk->subRet );
			if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
				BOX2OBJ_PokeIconChange(
					syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
			}
			MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP );
		}

		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_ITEM );
		}
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

		if( err == TRUE ){
			syswk->subRet = 0;	// 下で参照するけど、むしろ０のほうが良い
		}
		next = BOX2SEQ_MAINSEQ_ARRANGE_ITEMSET_MSG;
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );

	return next;
}

static int RcvBag_ModeItem( BOX2_SYS_WORK * syswk )
{
	int	next;

	BOX2BGWFRM_ArrangeUnderButtonDel( syswk );
	BOX2BGWFRM_RetButtonOutPut( syswk->app->wfrm );

	if( syswk->subRet == 0 ){
		BOX2BMP_ItemArrangeMenuStrPrint( syswk, 0 );
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
			next = BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
		}else{
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
			BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_A_PARTY, BOX2UI_ITEM_A_PARTY_MENU1 );
			next = BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
		}
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
	}else{
		BOOL err = FALSE;
		if( syswk->subRet == ITEM_HAKKINDAMA &&
			BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_monsno, NULL ) != MONSNO_GIRATHINA ){
			err = TRUE;
		}else{
			POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

			BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, syswk->subRet );
			if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
				BOX2OBJ_PokeIconChange(
					syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
			}
			MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP );
		}

		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			BOX2UI_CursorMoveChange(
				syswk, BOX2UI_INIT_ID_ITEM_A_PARTY, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
		}else{
			const CURSORMOVE_DATA * pw;
			u8	x, y;
			CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
			pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, syswk->get_pos );
			x = pw->px;
			y = pw->py;
			BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );
		}
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

		if( err == FALSE ){
			syswk->app->get_item = syswk->subRet;
			BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
			if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
				BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
			}else{
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
				BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
			}
			BOX2OBJ_ItemIconCursorAdd( syswk->app );
			BOX2OBJ_ItemIconCursorOn( syswk->app );
		}else{
			syswk->subRet = 0;	// 下で参照するけど、むしろ０のほうが良い
			if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
			}else{
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
			}
		}
		next = BOX2SEQ_MAINSEQ_ITEMARRANGE_ITEMSET_MSG;
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	if( syswk->subRet != 0 ){
		BOX2BGWFRM_SubDispItemFrmInPosSet( syswk->app->wfrm );
	}

	return next;
}

static int MainSeq_ArrangeBagRcv( BOX2_SYS_WORK * syswk )
{
/*
	int	next;

	if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		PartyFrmSet( syswk );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	}
	if( syswk->dat->callMode != BOX_MODE_ITEM ){
		BOX2OBJ_PokeCursorAdd( syswk );
	}

	if( syswk->dat->callMode == BOX_MODE_SEIRI || syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2BGWFRM_ArrangeUnderButtonDel( syswk );
		BOX2BGWFRM_RetButtonOutPut( syswk->app->wfrm );
	}

	if( syswk->subRet == 0 ){
		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2BMP_ItemArrangeMenuStrPrint( syswk, 0 );
			if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
				next = BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
			}else{
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_A_PARTY, BOX2UI_ITEM_A_PARTY_MENU1 );
				next = BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
			}
		}else{
			BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
			if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
			}else{
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_ITEM );
				next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
			}
		}
		BOX2BGWFRM_PokeMenuOpenPosSet( syswk->app->wfrm );
	}else{
		BOOL err = FALSE;
		if( syswk->subRet == ITEM_HAKKINDAMA &&
			BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_monsno, NULL ) != MONSNO_GIRATHINA ){
			err = TRUE;
		}else{
			POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

			BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, syswk->subRet );
			if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
				BOX2OBJ_PokeIconChange(
					syswk, syswk->tray, syswk->get_pos, syswk->app->pokeicon_id[syswk->get_pos] );
			}
			MYITEM_SubItem( syswk->dat->myitem, syswk->subRet, 1, HEAPID_BOX_APP );
		}

		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			if( syswk->dat->callMode == BOX_MODE_ITEM ){
				BOX2UI_CursorMoveChange(
					syswk, BOX2UI_INIT_ID_ITEM_A_PARTY, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
			}else{
				BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_ITEM );
			}
		}else{
			if( syswk->dat->callMode == BOX_MODE_ITEM ){
				const CURSORMOVE_DATA * pw;
				u8	x, y;
				CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
				pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, syswk->get_pos );
				x = pw->px;
				y = pw->py;
				BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );
			}
		}
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			if( err == FALSE ){
				syswk->app->get_item = syswk->subRet;
				BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
				BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
				if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
					BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
				}else{
					BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
					BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
				}
				BOX2OBJ_ItemIconCursorAdd( syswk->app );
				BOX2OBJ_ItemIconCursorOn( syswk->app );
			}else{
				syswk->subRet = 0;	// 下で参照するけど、むしろ０のほうが良い
				if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
				}else{
					BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
				}
			}
			next = BOX2SEQ_MAINSEQ_ITEMARRANGE_ITEMSET_MSG;
		}else{
			if( err == TRUE ){
				syswk->subRet = 0;	// 下で参照するけど、むしろ０のほうが良い
			}
			next = BOX2SEQ_MAINSEQ_ARRANGE_ITEMSET_MSG;
		}
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	if( syswk->dat->callMode != BOX_MODE_ITEM ){
		BOX2BGWFRM_SubDispWazaFrmInPosSet( syswk->app );
	}else{
		if( syswk->subRet != 0 ){
			BOX2BGWFRM_SubDispItemFrmInPosSet( syswk->app->wfrm );
		}
	}

	return FadeInSet( syswk, next );
*/
	int	next;

	if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		PartyFrmSet( syswk );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	}

	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:		// あずける
		next = RcvBag_ModeAzukeru( syswk );
		break;
	case BOX_MODE_TURETEIKU:	// つれていく
		next = RcvBag_ModeTureteiku( syswk );
		break;
	case BOX_MODE_SEIRI:			// せいり
		next = RcvBag_ModeSeiri( syswk );
		break;
	case BOX_MODE_ITEM:				// どうぐせいり
		next = RcvBag_ModeItem( syswk );
		break;
	}

	return FadeInSet( syswk, next );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：道具をもたせたメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeItemSetMsg( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_ItemSetMsgPut( syswk, syswk->subRet, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：マーキングフレーム初期化
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 *
 *	実際はMainSeq_ArrangeMarkingFrmIn()と同じ
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMarkingInit( BOX2_SYS_WORK * syswk )
{
	return MainSeq_ArrangeMarkingFrmIn( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：マーキングフレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMarkingFrmIn( BOX2_SYS_WORK * syswk )
{
	syswk->app->pokeMark = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_mark, NULL );

	BOX2BGWFRM_MarkingFramePut( syswk );
	BOX2BMP_MarkingButtonPut( syswk->app );
	BOX2BGWFRM_MarkingFrameInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncMarkingFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_MARKING_START );
}


//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：マーキング処理開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMarkingStart( BOX2_SYS_WORK * syswk )
{
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_MARKING, 0 );

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );

	BOX2BMP_MarkingMsgPut( syswk, BOX2BMPWIN_ID_MSG2 );
	return BOX2SEQ_MAINSEQ_ARRANGE_MARKING;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：マーキング処理メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMarking( BOX2_SYS_WORK * syswk )
{
	switch( CURSORMOVE_MainCont(syswk->app->cmwk) ){
	case BOX2UI_MARKING_MARK1:		// 00: ●
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2BGWFRM_MarkingSwitch( syswk, 0 );
		break;
	case BOX2UI_MARKING_MARK2:		// 01: ▲
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2BGWFRM_MarkingSwitch( syswk, 1 );
		break;
	case BOX2UI_MARKING_MARK3:		// 02: ■
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2BGWFRM_MarkingSwitch( syswk, 2 );
		break;
	case BOX2UI_MARKING_MARK4:		// 03: ハート
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2BGWFRM_MarkingSwitch( syswk, 3 );
		break;
	case BOX2UI_MARKING_MARK5:		// 04: ★
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2BGWFRM_MarkingSwitch( syswk, 4 );
		break;
	case BOX2UI_MARKING_MARK6:		// 05: ◆
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2BGWFRM_MarkingSwitch( syswk, 5 );
		break;

	case BOX2UI_MARKING_ENTER:		// 06:「けってい」
		{
			u8	mark = syswk->app->pokeMark;
			BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_mark, mark );
			BOX2MAIN_SubDispMarkingChange( syswk, mark );
		}
//		Snd_SePlay( SE_BOX2_DECIDE );
		return MarkingButtonAnmSet( syswk, 0, BOX2SEQ_MAINSEQ_MARKING_END_BUTTON );

	case BOX2UI_MARKING_CANCEL:	// 07:「やめる」
	case CURSORMOVE_CANCEL:			// キャンセル
//		Snd_SePlay( SE_BOX2_CANCEL );
		return MarkingButtonAnmSet( syswk, 1, BOX2SEQ_MAINSEQ_MARKING_END_BUTTON );

	case CURSORMOVE_CURSOR_MOVE:	// 移動
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_MARKING_CURMOVE_RET );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_MARKING;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：マージキングを終了してメニューへ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMarkingRcv( BOX2_SYS_WORK * syswk )
{
	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモンを逃がす開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeFreeStart( BOX2_SYS_WORK * syswk )
{
	POKEMON_PASO_PARAM * ppp;

	if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		POKEMON_PARAM * pp;
		u16	item;
		u32	pos;

		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );

		pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;

		// 手持ち１
		if( BOX2MAIN_BattlePokeCheck( syswk, pos ) == FALSE ){
//			Snd_SePlay( SE_BOX2_WARNING );
			BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
			syswk->app->poke_free_err = 1;
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}
		// メール
		pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos );
		item = PP_Get( pp, ID_PARA_item, NULL );
		if( ITEM_CheckMail( item ) == TRUE ){
//			Snd_SePlay( SE_BOX2_WARNING );
			BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_MAIL, BOX2BMPWIN_ID_MSG1 );
			syswk->app->poke_free_err = 1;
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}
/*	カスタムボールはないハズ
		// カプセル
		if( PP_Get( pp, ID_PARA_cb_id, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_WARNING );
			BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_CAPSULE, BOX2BMPWIN_ID_MSG1 );
			syswk->app->poke_free_err = 1;
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}
*/
	}else{
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
	}

	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

	// タマゴ
	if( PPP_Get( ppp, ID_PARA_tamago_flag, NULL ) != 0 ){
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_EGG, BOX2BMPWIN_ID_MSG1 );
		syswk->app->poke_free_err = 1;
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}

	BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_CHECK, BOX2BMPWIN_ID_MSG1 );
	return YesNoSet( syswk, YESNO_ID_POKEFREE );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモンを逃がすアニメ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeFreeAnm( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeFreeWazaCheck( syswk );

	if( BOX2OBJ_PokeIconFreeMove( syswk->app->seqwk ) == FALSE ){
		BOX2MAIN_POKEFREE_WORK * wk;
		u8	waza_flg;
		wk = syswk->app->seqwk;
		waza_flg = wk->check_flg;
		if( waza_flg != 0 ){
			// 戻す処理へ
			return BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_ERROR1;
		}else{
			// 消す処理へ
			BOX2OBJ_PokeIconFreeEnd( syswk->app->seqwk );
			BOX2MAIN_PokeFreeExit( syswk );
			return BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_ENTER1;
		}
	}
	return BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモンを逃がす実行処理１
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeFreeEnter1( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ENTER, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_ENTER2;
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモンを逃がす実行処理２
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeFreeEnter2( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_BY, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_ENTER3;
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモンを逃がす実行処理３
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeFreeEnter3( BOX2_SYS_WORK * syswk )
{
	void * func;

	BOX2MAIN_PokeDataClear( syswk, syswk->tray, syswk->get_pos );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_PokeInfoOff( syswk );

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		// トレイアイコン更新
		BOX2OBJ_TrayIconCgxMake( syswk, syswk->tray );
		BOX2OBJ_TrayIconCgxTrans( syswk, syswk->tray );
		if( syswk->dat->callMode == BOX_MODE_TURETEIKU ){
			BOX2BMP_PokePartyInMsgPut(
				syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
		}else{
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
			BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
			BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
		}
		BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );

		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );

		syswk->get_pos = BOX2MAIN_GETPOS_NONE;

		func = BOX2MAIN_VFuncPokeFreeMenuMove;

	}else{
		PokeMoveWorkAlloc( syswk );
		syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_PARTYPOKE_FREE_END;
		BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
		func = BOX2MAIN_VFuncPartyPokeFreeSort;
	}

	return VFuncSet( syswk, func, syswk->next_seq );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモン逃がした後の処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyPokeFreeEnd( BOX2_SYS_WORK * syswk )
{
	PokeMoveWorkFree( syswk );
	if( syswk->box_mv_flg != 0 ){
		BOX2BMP_BoxMoveNameNumWrite( syswk );
	}

	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );

	CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	syswk->get_pos = BOX2MAIN_GETPOS_NONE;

	if( syswk->dat->callMode == BOX_MODE_AZUKERU ){
		BOX2BMP_PokePartyInMsgPut(
			syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
		return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモンを逃がす失敗処理１
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeFreeError1( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_PokeIconFreeErrorMove( syswk->app->seqwk ) == FALSE ){
		BOX2OBJ_PokeIconFreeErrorEnd( syswk->app->seqwk );
		BOX2MAIN_PokeFreeExit( syswk );
		BOX2OBJ_PokeCursorAdd( syswk );
		BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_RETURN, BOX2BMPWIN_ID_MSG1 );
		BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_ERROR2;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}
	return BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_ERROR1;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモンを逃がす失敗処理２
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeFreeError2( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_FEAR, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_ERROR3;
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモンを逃がす失敗処理３
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePokeFreeError3( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeCursorVanish( syswk, TRUE );

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeFreeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」メイン処理へ復帰
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutPokeMenuRcv( BOX2_SYS_WORK * syswk )
{
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちフレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyPokeFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFrameLoadArrange( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY );
	BOX2BGWFRM_PartyPokeFrameInitPutLeft( syswk->app->wfrm );
	BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
	BOX2BMP_PartyCngButtonFrmPut( syswk->app );
	BOX2BMP_PartyChangeButtonPut( syswk->app );
	BOX2BMP_ButtonPutYameru( syswk );
	BOX2OBJ_PartyPokeIconFrmInSet( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_START );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」手持ちポケモン選択処理へ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyStart( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_A_PARTY, BOX2UI_ITEM_A_PARTY_POKE1 );
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
	}
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY, BOX2UI_ARRANGE_PARTY_POKE1 );
	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}


//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ち選択メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
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
				BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuCloseKeyArrange, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN_CURMOVE_RET );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
			}
		}
	}

	ret = BOX2UI_BoxArrangePartyMoveMain( syswk );

	switch( ret ){

	case BOX2UI_ARRANGE_PARTY_CHANGE:	//「いれかえ」
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->quick_mode = 0;
		syswk->quick_get  = 0;
		return PartyButtonAnmSet( syswk, 0, BOX2SEQ_MAINSEQ_PARTY_CHG_BUTTON );

	case BOX2UI_ARRANGE_PARTY_MOVE:	//「ポケモンいどう」（メニュー）
//		Snd_SePlay( SE_BOX2_DECIDE );
		if( GFL_UI_TP_GetTrg() == FALSE ){
//↑[GS_CONVERT_TAG]
			syswk->quick_mode = 2;
		}
		syswk->quick_get = syswk->get_pos;
		return YStatusButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU1, BOX2SEQ_MAINSEQ_PARTY_CHG_BUTTON );

	case BOX2UI_ARRANGE_PARTY_STATUS:	//「ようすをみる」
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->cur_rcv_pos = BOX2UI_ARRANGE_PARTY_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, BOX2SEQ_MAINSEQ_STATUS_BUTTON );

	case BOX2UI_ARRANGE_PARTY_ITEM:	//「もちもの」
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_ITEM_BUTTON );

	case BOX2UI_ARRANGE_PARTY_MARKING:	//「マーキング」
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, BOX2SEQ_MAINSEQ_MARKING_BUTTON );

	case BOX2UI_ARRANGE_PARTY_FREE:	//「にがす」
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_FREE_BUTTON );

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos <= BOX2UI_ARRANGE_PARTY_POKE6 ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}else{
				if( pos != BOX2UI_ARRANGE_PARTY_MOVE &&
					pos != BOX2UI_ARRANGE_PARTY_STATUS &&
					pos != BOX2UI_ARRANGE_PARTY_ITEM &&
					pos != BOX2UI_ARRANGE_PARTY_MARKING &&
					pos != BOX2UI_ARRANGE_PARTY_FREE &&
					pos != BOX2UI_ARRANGE_PARTY_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN_CURMOVE_RET );

	case BOX2UI_ARRANGE_PARTY_RETURN:	//「もどる」
	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_CLOSE_PARTY_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_PARTY_END_BUTTON );

	case BOX2UI_ARRANGE_PARTY_CLOSE:	//「とじる」
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_CLOSE_BTN, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_CLOSE_BUTTON );

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos <= BOX2UI_ARRANGE_PARTY_POKE6 ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}else{
				if( pos != BOX2UI_ARRANGE_PARTY_MOVE &&
					pos != BOX2UI_ARRANGE_PARTY_STATUS &&
					pos != BOX2UI_ARRANGE_PARTY_ITEM &&
					pos != BOX2UI_ARRANGE_PARTY_MARKING &&
					pos != BOX2UI_ARRANGE_PARTY_FREE &&
					pos != BOX2UI_ARRANGE_PARTY_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	case CURSORMOVE_NONE:				// 動作なし
		break;

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, PokeMenuStrTbl, 5 );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ARRANGE_PARTY_MOVE, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PARTY_MOVE );
			return PartyPokeGetKeyArrange( syswk, ret );
		}
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyPokeFrmOut( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_BUTTON_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス/手持ちボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxPartyButtonIn( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFrameOff( syswk );

	BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );

	BOX2BMP_ButtonPutTemochi( syswk );
	BOX2BMP_ButtonPutIdou( syswk );
	if( syswk->box_mv_flg == 0 ){
		BOX2BMP_ButtonPutModoru( syswk );
	}

	BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_A_MAIN, BOX2UI_ITEM_A_MAIN_PARTY );
		return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN );
	}

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_PARTY );
	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ARRANGE_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモン入れ替え後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyPokeChgEnd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	pos;

	work = syswk->app->vfunk.work;
	pos  = work->get_pos;

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG4 );

//	Snd_SePlay( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
	BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );
	PokeMoveWorkFree( syswk );

	if( syswk->get_pos == BOX2MAIN_GETPOS_NONE ){
		CURSORMOVE_PosSet( syswk->app->cmwk, pos - BOX2OBJ_POKEICON_TRAY_MAX );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2MAIN_PokeInfoPut( syswk, pos );
	}else{
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_PARTY_MOVE );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ち・トレイ入れ替えメイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
}

// デバッグオートプレイ
//#define	BTS_2299_TEST
#ifdef	BTS_2299_TEST
static int test_flg_2299 = 0;
static int test_pos_2299 = 0;
#endif	// BTS_2299_TEST

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」ポケモン移動メイン処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxPartyChgMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	// 手持ち
	ret = BOX2UI_HitCheckTrgPartyPokeRight();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return BoxPartyPokeGet( syswk, ret );
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
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return BoxPartyPokeGet( syswk, ret );
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

	// Ｙステータスボタン
	if( BOX2UI_YStatusButtonCheck( syswk ) == TRUE ){
		ret = CURSORMOVE_PosGet( syswk->app->cmwk );
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			syswk->get_pos = ret;
			syswk->y_status_flg = 1;
			return YStatusButtonAnmSet( syswk, BOX2MAIN_WINFRM_Y_ST_BTN, BOX2SEQ_MAINSEQ_STATUS_BUTTON );
		}
		return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
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
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return PokeMoveGetKey( syswk, ret );
		}
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ち・トレイ入れ替え後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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
		BOX2BMP_BoxMoveNameNumWrite( syswk );
	}
	if( BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_poke_exist, NULL ) == 0 ){
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
					BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_MAIL, BOX2BMPWIN_ID_MSG1 );
					syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_ERROR;
					return BOX2SEQ_MAINSEQ_TRGWAIT;
/*	カスタムボールはないハズ
				}else if( PP_Get( pp, ID_PARA_cb_id, NULL ) != 0 ){
//					Snd_SePlay( SE_BOX2_WARNING );
					BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_CAPSULE, BOX2BMPWIN_ID_MSG1 );
					syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_ERROR;
					return BOX2SEQ_MAINSEQ_TRGWAIT;
*/
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
					BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_BOXMAX, BOX2BMPWIN_ID_MSG1 );
				}else if( BOX2MAIN_BattlePokeCheck( syswk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX ) == TRUE ){
					BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_BOXMAX, BOX2BMPWIN_ID_MSG1 );
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ち・トレイ入れ替え後（キー操作）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxPartyPokeChgEndKey( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	put_pos, get_pos;
	u8	pos;

	work = syswk->app->vfunk.work;
	get_pos = work->get_pos;
	put_pos = work->put_pos;

//	Snd_SePlay( SE_BOX2_POKE_PUT );
	BOX2MAIN_PokeDataMove( syswk );
	if( ( put_pos & BOX2MAIN_BOXMOVE_FLG ) == 0 ){
		BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );
	}
	PokeMoveWorkFree( syswk );
	if( syswk->box_mv_flg != 0 ){
		BOX2BMP_BoxMoveNameNumWrite( syswk );
	}

	pos = CURSORMOVE_PosGet( syswk->app->cmwk );

	if( syswk->app->cancel_flg == FALSE ){
		if( put_pos == BOX2MAIN_GETPOS_NONE && pos != syswk->get_pos ){
			if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				u32	party_pos;
				POKEMON_PARAM * pp;

				party_pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, party_pos );

				if( ITEM_CheckMail( PP_Get(pp,ID_PARA_item,NULL) ) == TRUE ){
//					Snd_SePlay( SE_BOX2_WARNING );
					CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
					CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
					BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
					BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_MAIL, BOX2BMPWIN_ID_MSG1 );
					syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_ERROR;
					return BOX2SEQ_MAINSEQ_TRGWAIT;
/*	カスタムボールはないハズ
				}else if( PP_Get( pp, ID_PARA_cb_id, NULL ) != 0 ){
//					Snd_SePlay( SE_BOX2_WARNING );
					CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
					CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
					BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
					BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_CAPSULE, BOX2BMPWIN_ID_MSG1 );
					syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_ERROR;
					return BOX2SEQ_MAINSEQ_TRGWAIT;
*/
				}else if( BOX2MAIN_BattlePokeCheck( syswk, party_pos ) == FALSE ){
//					Snd_SePlay( SE_BOX2_WARNING );
					CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
					CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
					BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
					BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
					syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_ERROR;
					return BOX2SEQ_MAINSEQ_TRGWAIT;
				}
			}
			if( pos >= BOX2UI_ARRANGE_MOVE_TRAY1 && pos <= BOX2UI_ARRANGE_MOVE_TRAY6 &&
				BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos)+(pos-BOX2UI_ARRANGE_MOVE_TRAY1) != syswk->tray ){
				if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_BOXMAX, BOX2BMPWIN_ID_MSG1 );
				}else if( BOX2MAIN_BattlePokeCheck( syswk, syswk->get_pos-BOX2OBJ_POKEICON_TRAY_MAX ) == TRUE ){
					BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_BOXMAX, BOX2BMPWIN_ID_MSG1 );
				}else{
					BOX2BMP_PokeFreeMsgPut( syswk, BOX2BMP_MSGID_POKEFREE_ONE, BOX2BMPWIN_ID_MSG1 );
				}
//				Snd_SePlay( SE_BOX2_WARNING );
				CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, FALSE );
				BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
				syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_ERROR;
				return BOX2SEQ_MAINSEQ_TRGWAIT;
			}
			if( syswk->quick_mode == 0 ){
				BOX2MAIN_PokeInfoPut( syswk, CURSORMOVE_PosGet(syswk->app->cmwk) );
			}
		}
	}

	if( syswk->quick_mode != 0 ){
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
		BOX2MAIN_PokeSelectOff( syswk );
		return BoxMoveEndStart( syswk );
	}

	if( pos >= BOX2UI_ARRANGE_MOVE_TRAY1 && pos <= BOX2UI_ARRANGE_MOVE_TRAY6 ){
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
	}else{
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：入れ替えエラーメッセージ表示後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちフレーム右移動後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyFrmMoveREnd( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_PartySelectButtonPut( syswk->app );

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	}

	if( BOX2MAIN_PokeParaGet( syswk, syswk->quick_get, ID_PARA_poke_exist, NULL ) != 0 ){
		BOX2BMP_ButtonPutYStatus( syswk );
		BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
	}

	syswk->box_mv_flg = 1;		// ボックス移動フラグ
	syswk->pokechg_mode = 1;

	syswk->box_mv_pos = syswk->tray;

	return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_START );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちフレーム左移動後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyFrmMoveLEnd( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_PartyChangeButtonPut( syswk->app );

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_A_PARTY, BOX2UI_ITEM_A_PARTY_POKE1 );
		BOX2MAIN_PokeInfoPut(
			syswk, CURSORMOVE_PosGet(syswk->app->cmwk)+BOX2OBJ_POKEICON_TRAY_MAX );
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxMoveFrmIn( BOX2_SYS_WORK * syswk )
{
	syswk->box_mv_pos = syswk->tray;

	return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_EXIT_BUTTON_FRMIN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：やめるフレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeExitButtonFrmIn( BOX2_SYS_WORK * syswk )
{
/*
この処理を手持ちフレームインに変更
*/
	BOX2BGWFRM_PartyPokeFrameLoadArrange( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY );
	BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
	BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
	BOX2BMP_PartyCngButtonFrmPut( syswk->app );
	BOX2BMP_PartySelectButtonPut( syswk->app );
	BOX2OBJ_PartyPokeIconFrmInSetRight( syswk );

	BOX2BMP_ButtonPutYStatus( syswk );
	if( BOX2MAIN_PokeParaGet( syswk, syswk->quick_get, ID_PARA_poke_exist, NULL ) != 0 ){
		BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_START );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームアウト設定
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxMoveFrmOutSet( BOX2_SYS_WORK * syswk )
{
	return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXMOVE_FRMOUT );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxMoveFrmOut( BOX2_SYS_WORK * syswk )
{
	if( syswk->pokechg_mode == 1 ){
		return PartyFrmMoveLeft( syswk );
	}
	BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_BOX_RET_BUTTON_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス/もどるボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxRetButtonIn( BOX2_SYS_WORK * syswk )
{
	u32	id;
	u32	pos;
	int	next;

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		id   = BOX2UI_INIT_ID_ITEM_A_MAIN;
		pos  = BOX2UI_ITEM_A_MAIN_CHANGE;
		next = BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
	}else{
		id   = BOX2UI_INIT_ID_ARRANGE_MAIN;
		pos  = BOX2UI_ARRANGE_MAIN_CHANGE;
		next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		if( syswk->quick_mode != 0 ){
			pos = syswk->quick_get;
			BOX2MAIN_PokeInfoPut( syswk, pos );
		}
	}

	BOX2UI_CursorMoveChange( syswk, id, pos );

	BOX2BMP_ButtonPutTemochi( syswk );
	BOX2BMP_ButtonPutIdou( syswk );
	BOX2BMP_ButtonPutModoru( syswk );

	BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, next );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_BOXTHEMA_NAME_BUTTON );

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
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_EXIT_OUT );
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更開始セット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxThemaButtonOut( BOX2_SYS_WORK * syswk )
{
	if( syswk->dat->callMode == BOX_MODE_TURETEIKU ){
		if( BOX2BGWFRM_CloseButtonPutCheck(syswk->app->wfrm) == FALSE ){
			BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
		}
	}else{
		BOX2BGWFRM_TemochiButtonOutSet( syswk->app->wfrm );
		BOX2BGWFRM_IdouButtonOutSet( syswk->app->wfrm );
		BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_TRAY_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームインセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxThemaTrayIn( BOX2_SYS_WORK * syswk )
{
	syswk->box_mv_pos = syswk->tray;

	return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_EXIT_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「やめる」フレームインセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxThemaExitIn( BOX2_SYS_WORK * syswk )
{
	return BoxMoveExitButtonInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_MENU_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更メニューイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxThemaMenuIn( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_BoxThemaMenuPrint( syswk );
	BOX2BGWFRM_BoxThemaMenuInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxThemaMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_MSG_SET );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更メッセージセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxThemaMsgSet( BOX2_SYS_WORK * syswk )
{
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_BOXTHEMA_CHG, syswk->cur_rcv_pos );

	if( syswk->cur_rcv_pos >= BOX2UI_BOXTHEMA_CHG_TRAY1 &&
		syswk->cur_rcv_pos <= BOX2UI_BOXTHEMA_CHG_TRAY6 ){
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
	}else{
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	}

	BOX2BMP_BoxThemaMsgPut( syswk, BOX2BMP_MSGID_THEMA_INIT, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
}


//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「やめる」フレームアウトセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxThemaExitOut( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
	return BoxMoveExitButtonOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_MENU_OUT );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更メニューアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxThemaMenuOut( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_BoxThemaMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxThemaMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_TRAY_OUT );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス移動フレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxThemaTrayOut( BOX2_SYS_WORK * syswk )
{
	return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_BUTTON_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更終了＞ポケモン選択へ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxThemaButtonIn( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BMP_ButtonPutModoru( syswk );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );

	if( syswk->dat->callMode == BOX_MODE_TURETEIKU ){
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTIN_MAIN, BOX2UI_PTIN_MAIN_NAME );
		return VFuncSet( syswk, BOX2MAIN_VFuncExitButtonMove, BOX2SEQ_MAINSEQ_PARTYIN_MAIN_RCV );
	}

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ITEM_A_MAIN, BOX2UI_ITEM_A_MAIN_NAME );
	}else{
		BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MAIN, BOX2UI_ARRANGE_MAIN_NAME );
	}

	BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_BOXARRANGE_POKE_MENU_RCV );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙フレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperFrmIn( BOX2_SYS_WORK * syswk )
{
	syswk->app->wallpaper_pos = BOX2MAIN_GetWallPaperNumber( syswk, syswk->box_mv_pos );

	BOX2OBJ_WallPaperChgObjInit( syswk );
	BOX2OBJ_WallPaperCursorSet( syswk );
	BOX2OBJ_WallPaperNameSet( syswk );
	BOX2BMP_WallPaperNameWrite( syswk );

	BOX2BGWFRM_BoxThemaMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_WallPaperFrameInSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncWallPaperFrmIn, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_MENU_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更メニューイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperMenuIn( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_WallPaperChgMenuPrint( syswk );
	BOX2BGWFRM_BoxMoveMenuInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveMenuIn, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_START );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙選択開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperChgStart( BOX2_SYS_WORK * syswk )
{
	syswk->app->old_cur_pos = syswk->app->wallpaper_pos % BOX2OBJ_WPICON_MAX;
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_WALLPAPER_CHG, syswk->app->old_cur_pos );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );

	BOX2BMP_BoxThemaMsgPut( syswk, BOX2BMP_MSGID_THEMA_WALL, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙選択メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_CANCEL_BUTTON );
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙メニューアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperMenuOut( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_BoxMoveMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveMenuOut, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_FRM_OUT );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙フレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperFrmOut( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_WallPaperFrameOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncWallPaperFrmOut, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperChgEnd( BOX2_SYS_WORK * syswk )
{
	return BoxMoveFrmInSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_MENU_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperChange( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_WallPaperChange( syswk, syswk->app->wallpaper_pos );
/*
	// 追加壁紙はプラチナ以降の番号を使用している
	if( syswk->app->wallpaper_pos < BOX_NORMAL_WALLPAPER_MAX ){
		BOXDAT_SetWallPaperNumber( syswk->dat->sv_box, syswk->tray, syswk->app->wallpaper_pos );
	}else{
		BOXDAT_SetWallPaperNumber(
			syswk->dat->sv_box, syswk->tray,
			syswk->app->wallpaper_pos - BOX_NORMAL_WALLPAPER_MAX + BOX_TOTAL_WALLPAPER_MAX_PL );
	}
*/
	BOXDAT_SetWallPaperNumber( syswk->dat->sv_box, syswk->tray, syswk->app->wallpaper_pos );
	BOX2OBJ_WallPaperCursorSet( syswk );
	BOX2OBJ_TrayIconCgxMake( syswk, syswk->tray );

	return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_WHITE_IN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更ホワイトアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperWhiteOut( BOX2_SYS_WORK * syswk )
{
	PaletteWorkSet_VramCopy( syswk->app->pfd, FADE_MAIN_BG, 0, FADE_PAL_ALL_SIZE );
	PaletteFadeReq( syswk->app->pfd, PF_BIT_MAIN_BG, 0xc000, 0, 0, 16, 0x7fff, GFUser_VIntr_GetTCBSYS() );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHANGE;
	return BOX2SEQ_MAINSEQ_PALETTE_FADE;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更ホワイトイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperWhiteIn( BOX2_SYS_WORK * syswk )
{
	PaletteFadeReq( syswk->app->pfd, PF_BIT_MAIN_BG, 0xc000, 0, 16, 0, 0x7fff, GFUser_VIntr_GetTCBSYS() );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;
	return BOX2SEQ_MAINSEQ_PALETTE_FADE;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxArrangePokeMenuRcv( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	// 道具整理
	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
	}
	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ち選択メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxArrangePartyMenuRcv( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}


//============================================================================================
//	ポケモンをつれていく
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitTrayPokeTrg();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 4 );
			}
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return TrayPokeGetDrop( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuCloseKey, BOX2SEQ_MAINSEQ_PARTYIN_MENU_CLOSE_END );
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
	case BOX2UI_PTIN_MAIN_TSURETEIKU:	//「つれていく」
		if( PokeParty_GetPokeCount( syswk->dat->pokeparty ) != TEMOTI_POKEMAX ){
//			Snd_SePlay( SE_BOX2_DECIDE );
		}else{
//			Snd_SePlay( SE_BOX2_WARNING );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, BOX2SEQ_MAINSEQ_PARTYIN_BUTTON );

	case BOX2UI_PTIN_MAIN_STATUS:		//「ようすをみる」
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->cur_rcv_pos = BOX2UI_PTIN_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_STATUS_BUTTON );

	case BOX2UI_PTIN_MAIN_MARKING:		//「マーキング」
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, BOX2SEQ_MAINSEQ_MARKING_BUTTON );

	case BOX2UI_PTIN_MAIN_FREE:		//「にがす」
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_FREE_BUTTON );

	case BOX2UI_PTIN_MAIN_CLOSE:		//「とじる」
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
//		Snd_SePlay( SE_BOX2_CANCEL );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_CLOSE_BTN, BOX2SEQ_MAINSEQ_PARTYIN_CLOSE_BUTTON );

	case BOX2UI_PTIN_MAIN_NAME:		// ボックス名
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
		return BoxThemaChgStart( syswk );

	case BOX2UI_PTIN_MAIN_LEFT:		// トレイ切り替え矢印・左
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTIN_MAIN_NAME );
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
		return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );

	case BOX2UI_PTIN_MAIN_RIGHT:		// トレイ切り替え矢印・右
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTIN_MAIN_NAME );
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
		return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );

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
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_PARTYIN_MAIN_CURMOVE_RET );

	case BOX2UI_PTIN_MAIN_RETURN:		//「もどる」
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case CURSORMOVE_NONE:			// 動作なし
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos == BOX2UI_PTIN_MAIN_NAME ){
				if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
//↑[GS_CONVERT_TAG]
//					Snd_SePlay( SE_BOX2_CHANGE_TRAY );
					BOX2MAIN_PokeSelectOff( syswk );
					return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );
				}
				if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
//↑[GS_CONVERT_TAG]
//					Snd_SePlay( SE_BOX2_CHANGE_TRAY );
					BOX2MAIN_PokeSelectOff( syswk );
					return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_PARTYIN_MAIN );
				}
			}
		}
		break;

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
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, PartyInMenuStrTbl, 4 );
			BOX2MAIN_PokeInfoPut( syswk, ret );

			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_PTIN_MAIN_TSURETEIKU, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTIN_MAIN_TSURETEIKU );
			return TrayPokeGetKey( syswk, ret );
		}
	}

	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInMainRcv( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
	return MainSeq_PartyInPokeMenuRcv( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」手持ちポケモンフレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInPartyFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PartyPokeIconFrmInSetRight( syswk );
	BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
	BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_PARTYIN_POKEMOVE_SET );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」ポケモン移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInPokeMoveSet( BOX2_SYS_WORK * syswk )
{
	PokeMoveWorkAlloc( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyInPokeMove, BOX2SEQ_MAINSEQ_PARTYIN_PARTY_FRM_OUT );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」手持ちポケモンフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」戻るボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInRetFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncExitButtonMove, BOX2SEQ_MAINSEQ_PARTYIN_MAIN_RCV );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：つれていくポケモンをドロップ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
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

	BOX2BMP_SysWinVanishEz( syswk->app, BOX2BMPWIN_ID_MSG4 );

	if( syswk->get_pos == BOX2MAIN_GETPOS_NONE ){
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
	}else{
		BOX2BMP_PokePartyInMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
		pos = BOX2UI_PTIN_MAIN_TSURETEIKU;
	}
	CURSORMOVE_PosSet( syswk->app->cmwk, pos );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInPokeMenuRcv( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}


//============================================================================================
//	ポケモンを預ける
//============================================================================================

// デバッグオートプレイ
//#define	BTS_090509_AUTO_PLAY
#ifdef	BTS_090509_AUTO_PLAY
static u32 test_rect_flg = 0;
static u32 test_rect_pos = 0;
static u32 test_rect_cnt = 0;
#endif

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」手持ち選択メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

#ifdef	BTS_090509_AUTO_PLAY
	if( test_rect_flg != 0 ){
		if( test_rect_cnt != 0 ){
			test_rect_cnt--;
		}else{
			test_rect_cnt = GFUser_GetPublicRand(GFUSER_RAND_PAST_MAX)() & 0x3f;
//↑[GS_CONVERT_TAG]
			test_rect_pos++;
			if( test_rect_pos >= 6 ){
				test_rect_pos = 0;
			}
			ret = BOX2OBJ_POKEICON_TRAY_MAX + test_rect_pos;
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 4 );
			}
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return PartyPokeGetDrop( syswk, ret );
		}
	}
#endif

	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
				BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 4 );
			}
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
#ifdef	BTS_090509_AUTO_PLAY
			OS_Printf( "\n■■■■■■　デバッグオートプレイ開始 ver.090516-　■■■■■■\n" );
			test_rect_flg = 1;
			test_rect_cnt = GFUser_GetPublicRand(GFUSER_RAND_PAST_MAX)() & 0x3f;
//↑[GS_CONVERT_TAG]
#endif
			return PartyPokeGetDrop( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
				u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BMP_ButtonPutModoru( syswk );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				return VFuncSet( syswk, BOX2MAIN_VFuncMenuCloseKey, BOX2SEQ_MAINSEQ_PARTYOUT_MENU_CLOSE_END );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
			}
		}
	}

	ret = BOX2UI_PartyOutMain( syswk );

#ifdef	BTS_5436_TEST
	if( test_5436_flg == 1 ){
		ret = BOX2UI_PTOUT_MAIN_STATUS;
	}
#endif	// BTS_5436_TEST

	switch( ret ){
	case BOX2UI_PTOUT_MAIN_AZUKERU:	//「あずける」
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU2, BOX2SEQ_MAINSEQ_PARTYOUT_BUTTON );

	case BOX2UI_PTOUT_MAIN_STATUS:		//「ようすをみる」
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->cur_rcv_pos = BOX2UI_PTOUT_MAIN_STATUS;
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU3, BOX2SEQ_MAINSEQ_STATUS_BUTTON );

	case BOX2UI_PTOUT_MAIN_MARKING:	//「マーキング」
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU4, BOX2SEQ_MAINSEQ_MARKING_BUTTON );

	case BOX2UI_PTOUT_MAIN_FREE:		//「にがす」
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_FREE_BUTTON );

	case BOX2UI_PTOUT_MAIN_CLOSE:		//「とじる」
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
//		Snd_SePlay( SE_BOX2_CANCEL );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_CLOSE_BTN, BOX2SEQ_MAINSEQ_PARTYOUT_CLOSE_BUTTON );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos <= BOX2UI_PTOUT_MAIN_RETURN ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;

	case CURSORMOVE_CURSOR_MOVE:	// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos <= BOX2UI_PTOUT_MAIN_RETURN ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_MAIN_CURMOVE_RET );

	case BOX2UI_PTOUT_MAIN_RETURN:		//「もどる」
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case CURSORMOVE_NONE:			// 動作なし
		break;

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2BMP_MenuStrPrint( syswk, PartyOutMenuStrTbl, 4 );
			BOX2MAIN_PokeInfoPut( syswk, ret );

			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_PTOUT_MAIN_AZUKERU, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTOUT_MAIN_AZUKERU );
			return PartyPokeGetKey( syswk, ret );
		}
	}

	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けられるかをチェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_MAIL, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_START_ERROR;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}
/*	カスタムボールはないハズ
	// カプセル
	if( PP_Get( pp, ID_PARA_cb_id, NULL ) != 0 ){
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_CAPSULE, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_START_ERROR;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}
*/
	syswk->box_mv_pos = syswk->tray;

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );

	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeOutBoxFrmIn, BOX2SEQ_MAINSEQ_PARTYOUT_START_RET_FRM_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けるエラー
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutStartError( BOX2_SYS_WORK * syswk )
{
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeFreeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択「やめる」ボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutStartRetFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeOutBoxObjInit( syswk );
	BOX2OBJ_BoxMoveCursorSet( syswk );
	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameNumWrite( syswk );
	return BoxMoveExitButtonInSet( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_MENUFRM_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択メニューイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutMenuFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_PartyOutMenuPrint( syswk );
	BOX2BGWFRM_BoxMoveMenuInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveMenuIn, BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_START );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutBoxSelectStart( BOX2_SYS_WORK * syswk )
{
	syswk->app->old_cur_pos = syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_BOXSEL, syswk->app->old_cur_pos );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_INIT, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択メニューアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutBoxMenuFrmOut( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_BoxMoveMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveMenuOut, BOX2SEQ_MAINSEQ_PARTYOUT_END_RET_FRM_OUT );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択「やめる」ボタンアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutEndRetFrmOut( BOX2_SYS_WORK * syswk )
{
	return BoxMoveExitButtonOutSet( syswk, BOX2SEQ_MAINSEQ_PARTYOUT_BOXMOVE_FRM_OUT );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所選択トレイフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutBoxMoveFrmOut( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeOutBoxObjVanish( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeOutBoxFrmOut, BOX2SEQ_MAINSEQ_PARTYOUT_END_RET_FRM_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けるメイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutEndRetFrmIn( BOX2_SYS_WORK * syswk )
{
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_MAIN, BOX2UI_PTOUT_MAIN_AZUKERU );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	BOX2OBJ_PokeCursorAdd( syswk );
	BOX2OBJ_PokeCursorVanish( syswk, TRUE );

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );

	return BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける場所決定
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutEnterButton( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_BoxMoveMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyOutButtonMove, BOX2SEQ_MAINSEQ_PARTYOUT_CHECK );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けられるかをチェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutCheck( BOX2_SYS_WORK * syswk )
{

	if( BOXDAT_GetPokeExistCount( syswk->dat->sv_box, syswk->box_mv_pos ) == BOX_MAX_POS ){
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYOUT_BOXMAX, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_PARTYOUT_START_RET_FRM_IN;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );

	BOX2OBJ_PokeOutBoxObjVanish( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeOutBoxFrmOut, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_PARTYFRM_RIGHT );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンフレームを右へ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutEnterPartyFrmRight( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_AzukeruPartyPokeFrameRightMoveSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrmRight, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_TRAY_SCROLL );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けるトレイへスクロール
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモン移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutEnterPokeMove( BOX2_SYS_WORK * syswk )
{
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;

	PokeMoveWorkAlloc( syswk );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyOutPokeMove, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_PARTYFRM_LEFT );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンフレームを左へ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutEnterPartyFrmLeft( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work = syswk->app->vfunk.work;

//	Snd_SePlay( SE_BOX2_POKE_PUT );

	BOX2MAIN_PokeDataMove( syswk );

	BOX2MAIN_FormChangeSheimi( syswk, work->get_pos, work->put_pos );

	PokeMoveWorkFree( syswk );

	BOX2BGWFRM_AzukeruPartyPokeFrameLeftMoveSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrmLeft, BOX2SEQ_MAINSEQ_PARTYOUT_ENTER_RET_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もどる」ボタンイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutEnterRetIn( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_ButtonPutModoru( syswk );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncExitButtonMove, BOX2SEQ_MAINSEQ_PARTYOUT_COMP );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預ける完了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutComp( BOX2_SYS_WORK * syswk )
{
	BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_PTOUT_MAIN, BOX2UI_PTOUT_MAIN_POKE1 );

	BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );

	BOX2MAIN_PokeInfoPut( syswk, BOX2UI_PTOUT_MAIN_POKE1+BOX2OBJ_POKEICON_TRAY_MAX );

	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );

	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：預けるポケモンをドロップ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
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

	BOX2BMP_SysWinVanishEz( syswk->app, BOX2BMPWIN_ID_MSG4 );

	if( syswk->get_pos == BOX2MAIN_GETPOS_NONE ){
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
		CURSORMOVE_PosSet( syswk->app->cmwk, 0 );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		BOX2MAIN_PokeInfoPut( syswk, BOX2OBJ_POKEICON_TRAY_MAX );
	}else{
		BOX2BMP_PokePartyInMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_PTOUT_MAIN_AZUKERU );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	}

	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」ポケモン移動メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxPartyChgMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」手持ちポケモンメイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」メイン処理へ戻る（メッセージ表示あり）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutMenuCloseEnd( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」ボックス選択処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutBoxSelCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：マーキング処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MarkingCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ARRANGE_MARKING;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ変更処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaChgCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：壁紙変更処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_WallPaperChgCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」メイン処理へ戻る（メッセージあり）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInMenuCloseEnd( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
	return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;
}


//============================================================================================
//	道具整理
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：道具整理メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitTrayPokeTrg();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return TrayItemGet( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
				u8	pos = syswk->get_pos;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
				BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
				BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				if( syswk->app->get_item != ITEM_DUMMY_DATA ){
					BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
				}
				return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeMenuClose, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN_CURMOVE_RET );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
			}
		}
	}

	ret = BOX2UI_BoxItemArrangeMain( syswk );

	switch( ret ){
	case BOX2UI_ITEM_A_MAIN_NAME:		// 30: ボックス名
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		return BoxThemaChgStart( syswk );

	case BOX2UI_ITEM_A_MAIN_LEFT:		// 31: トレイ切り替え矢印・左
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_A_MAIN_NAME );
		return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN );

	case BOX2UI_ITEM_A_MAIN_RIGHT:		// 32: トレイ切り替え矢印・右
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_A_MAIN_NAME );
		return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN );

	case BOX2UI_ITEM_A_MAIN_PARTY:		// 33: 手持ちポケモン
//		Snd_SePlay( SE_BOX2_OPEN_PARTY_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_POKE_BTN, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_BUTTON );

	case BOX2UI_ITEM_A_MAIN_CHANGE:	// 34: 持ち物整理
//		Snd_SePlay( SE_BOX2_OPEN_PARTY_TRAY );
		return YStatusButtonAnmSet( syswk, BOX2MAIN_WINFRM_MV_BTN, BOX2SEQ_MAINSEQ_ITEMARRANGE_MOVE_BUTTON );

	case BOX2UI_ITEM_A_MAIN_MENU1:		// 36: バッグへ or もたせる
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEMARRANGE_BAG_BUTTON );

	case BOX2UI_ITEM_A_MAIN_CLOSE:		// 37: とじる
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_CLOSE_BTN, BOX2SEQ_MAINSEQ_ITEMARRANGE_CLOSE_BUTTON );

	case CURSORMOVE_NONE:				// 動作なし
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos == BOX2UI_ARRANGE_MAIN_NAME ){
				if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
//↑[GS_CONVERT_TAG]
//					Snd_SePlay( SE_BOX2_CHANGE_TRAY );
					BOX2MAIN_PokeSelectOff( syswk );
					return TrayLeftScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN );
				}
				if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
//↑[GS_CONVERT_TAG]
//					Snd_SePlay( SE_BOX2_CHANGE_TRAY );
					BOX2MAIN_PokeSelectOff( syswk );
					return TrayRightScrollSet( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN );
				}
			}
		}
		break;

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ITEM_A_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_ITEM_A_MAIN_MENU1 &&
					pos != BOX2UI_ITEM_A_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN_CURMOVE_RET );

	case BOX2UI_ITEM_A_MAIN_RETURN:	// 35: もどる
//		Snd_SePlay( SE_BOX2_DECIDE );
		BOX2MAIN_PokeInfoOff( syswk );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_BOXEND_INIT );

	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_BOXEND_CANCEL_INIT );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ITEM_A_MAIN_NAME ){
				BOX2MAIN_PokeInfoPut( syswk, pos );
			}else{
				if( pos != BOX2UI_ITEM_A_MAIN_MENU1 &&
					pos != BOX2UI_ITEM_A_MAIN_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ITEM_A_MAIN_MENU1, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_A_MAIN_MENU1 );
			return TrayItemGetKey( syswk, ret );
		}
		break;
	}

	return BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持たせたあとの処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeRetBagSet( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_VanishCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
	}

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
		BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	}else{
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeFreeMenuMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」メイン処理へ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持ち物をバッグに戻せるかをチェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：バッグに戻せないときのエラー処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeRetBagEnter( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
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

		BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_RET, BOX2BMPWIN_ID_MSG1 );
		BOX2BMP_ItemArrangeMenuStrPrint( syswk, 0 );
		syswk->app->get_item = ITEM_DUMMY_DATA;
		syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_SET;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_ENTER;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：バッグに戻すキャンセル処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeRetBagCancel( BOX2_SYS_WORK * syswk )
{
	const CURSORMOVE_DATA * pw;
	u16	pos;
	u8	x, y;

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		pos = syswk->get_pos;
		BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
		BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
	}else{
		pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	}
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );

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

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeFreeMenuMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：バッグへ戻す終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeRetBagEnd( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_END;
	}
	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
	}
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：タマゴは持てないメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持たせたメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：アイテムアイコン移動後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeIconMoveRet( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_ITEMMOVE_WORK * mvwk;

	mvwk = syswk->app->vfunk.work;

	if( mvwk->put_pos == syswk->get_pos || syswk->app->get_item == ITEM_DUMMY_DATA ){
		ItemMoveWorkFree( syswk );
		if( syswk->app->get_item != ITEM_DUMMY_DATA ){
//			Snd_SePlay( SE_BOX2_POKE_PUT );
			BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		}
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_A_MAIN_MENU1 );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
	}

//	Snd_SePlay( SE_BOX2_POKE_PUT );

	BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );

	// アウトライン消去
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	// アイテムアイコン縮小
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );

	return BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_POKEADD;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：アイテムをセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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
	item = BOX2MAIN_PokeParaGet( syswk, mvwk->put_pos, ID_PARA_item, NULL );
	BOX2MAIN_PokeParaPut( syswk, mvwk->put_pos, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, mvwk->put_pos );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, mvwk->put_pos,
			syswk->app->pokeicon_id[mvwk->put_pos] );
	}
	BOX2MAIN_PokeInfoPut( syswk, mvwk->put_pos );

	// 移動元にアイテムをセット
	syswk->app->get_item = item;
	BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, syswk->get_pos,
			syswk->app->pokeicon_id[syswk->get_pos] );
	}

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		syswk->get_pos = mvwk->put_pos;
		ItemMoveWorkFree( syswk );
		BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
		BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
		BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN );
	}

//	Snd_SePlay( SE_BOX2_POKE_CATCH );

	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemIconChange, BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_CHANGE );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：アイテムアイコン交換動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：アイテム交換終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeIconMoveChangeEnd( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_CHANGE_END;
	}

	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );

	BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」メイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeBoxPartyChgMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	// 手持ち
	ret = BOX2UI_HitCheckTrgPartyPokeRight();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return BoxPartyItemGet( syswk, ret );
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			CURSORMOVE_PosSet( syswk->app->cmwk, ret );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
				return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
			}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
				return YStatusButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
			}
			return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
		}
	}

	// ボックス
	ret = BOX2UI_HitTrayPokeTrg();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return BoxPartyItemGet( syswk, ret );
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			CURSORMOVE_PosSet( syswk->app->cmwk, ret );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
				return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
			}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
				return YStatusButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
			}
			return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
		}
	}

	// Ｙステータスボタン
	if( BOX2UI_YStatusButtonCheck( syswk ) == TRUE ){
		ret = CURSORMOVE_PosGet( syswk->app->cmwk );
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			syswk->get_pos = ret;
			syswk->y_status_flg = 1;
			return YStatusButtonAnmSet( syswk, BOX2MAIN_WINFRM_Y_ST_BTN, BOX2SEQ_MAINSEQ_STATUS_BUTTON );
		}
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
	}

	ret = BOX2UI_BoxArrangePokeMove( syswk );

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
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
			return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
		}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
			return YStatusButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
		}
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;

	case BOX2UI_ARRANGE_MOVE_RIGHT:	// 44: トレイ矢印（右）
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		BOX2MAIN_PokeSelectOff( syswk );
		BoxMoveTrayIconChgOnly( syswk, 1 );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
			return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
		}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
			return YStatusButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
		}
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;

	case BOX2UI_ARRANGE_MOVE_CHANGE:	// 45: ボックスをきりかえる
//		Snd_SePlay( SE_BOX2_CHANGE_TRAY );
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_BOXMV_BTN, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOX_MOVE_BUTTON );

	case CURSORMOVE_CURSOR_MOVE:	// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos < BOX2UI_ARRANGE_MOVE_RETURN ){
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
			return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN_CURMOVE_RET );
		}
		break;

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
	case CURSORMOVE_NONE:			// 動作なし
		break;

	case BOX2UI_ARRANGE_MOVE_RETURN:	// 36: やめる
//		Snd_SePlay( SE_BOX2_CLOSE_PARTY_TRAY );
		return PartyButtonAnmSet( syswk, 1, BOX2SEQ_MAINSEQ_BOX_MOVE_END_BUTTON );

	case CURSORMOVE_CANCEL:				// キャンセル
		if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == FALSE ){
//			Snd_SePlay( SE_BOX2_CLOSE_PARTY_TRAY );
			return PartyButtonAnmSet( syswk, 1, BOX2SEQ_MAINSEQ_BOX_MOVE_END_BUTTON );
		}else{
//			Snd_SePlay( SE_BOX2_CANCEL );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			return BoxMoveButtonFrmOut( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
		}
		break;

	default:
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
			BOX2MAIN_PokeInfoPut( syswk, ret );
			return BoxPartyItemGetKey( syswk, ret );
		}
	}

	return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeBoxPartyChgMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：上画面アイテムフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeBoxPartyIconMoveSubFrmOut( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_ICONMOVE_POKEADD;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持ち物交換
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeBoxPartyIconMovePokeAdd( BOX2_SYS_WORK * syswk )
{
	POKEMON_PASO_PARAM * ppp;
	BOX2MAIN_ITEMMOVE_WORK * mvwk;
	u16	item;

	mvwk = syswk->app->vfunk.work;

	if( BGWINFRM_MoveCheck( syswk->app->wfrm, BOX2MAIN_WINFRM_SUBDISP ) != 0 ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_ICONMOVE_POKEADD;
	}

	if( mvwk->put_pos == syswk->get_pos ){
		u32	set_pos = mvwk->set_pos;
		ItemMoveWorkFree( syswk );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		if( ITEM_CheckMail( syswk->app->get_item ) == TRUE && set_pos != syswk->get_pos ){
//			Snd_SePlay( SE_BOX2_WARNING );
			BOX2BMP_MailMoveErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
			syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_ERROR;
			return BOX2SEQ_MAINSEQ_TRGWAIT;
		}else{
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
		}
	}

	BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );

	// 移動先にアイテムをセット
	item = BOX2MAIN_PokeParaGet( syswk, mvwk->put_pos, ID_PARA_item, NULL );
	BOX2MAIN_PokeParaPut( syswk, mvwk->put_pos, ID_PARA_item, syswk->app->get_item );
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
	BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, syswk->get_pos,
			syswk->app->pokeicon_id[syswk->get_pos] );
	}

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		syswk->get_pos = mvwk->put_pos;
		ItemMoveWorkFree( syswk );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
	}

	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );
//	Snd_SePlay( SE_BOX2_POKE_CATCH );
	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeBoxPartyIconChgTouch, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_ICONMOVE_END );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持ち物交換終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeBoxPartyIconMoveEnd( BOX2_SYS_WORK * syswk )
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
		syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_ERROR;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}

	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：入れ替えエラーメッセージ表示後
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeBoxPartyChgError( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanishTrans( syswk->app, BOX2BMPWIN_ID_MSG1 );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：キーでアイテム入れ替え
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」キー取得メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeKeyGetMoveMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_KEYGET_MOVE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持ち物交換（キー操作）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeKeyGetChangeCkack( BOX2_SYS_WORK * syswk )
{
	POKEMON_PASO_PARAM * ppp;
	u16	item;

	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->app->poke_put_key, FALSE );

	if( syswk->app->poke_put_key == syswk->app->get_item_init_pos ){
		syswk->app->get_item = ITEM_DUMMY_DATA;
		BOX2MAIN_PokeInfoPut( syswk, syswk->app->poke_put_key );
		BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
		return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
	}

	// 持っているアイテムを取得
	item = BOX2MAIN_PokeParaGet( syswk, syswk->app->poke_put_key, ID_PARA_item, NULL );
	// アイテムを持たせる
	BOX2MAIN_PokeParaPut( syswk, syswk->app->poke_put_key, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->app->poke_put_key );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, syswk->app->poke_put_key,
			syswk->app->pokeicon_id[syswk->app->poke_put_key] );
	}
	BOX2MAIN_PokeInfoPut( syswk, syswk->app->poke_put_key );

	// 取得したアイテムを元のポケモンに持たせる
	syswk->app->get_item = item;
	BOX2MAIN_PokeParaPut( syswk, syswk->app->get_item_init_pos, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->app->get_item_init_pos );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, syswk->app->get_item_init_pos,
			syswk->app->pokeicon_id[syswk->app->get_item_init_pos] );
	}

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
		return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
	}

	syswk->app->poke_get_key = 1;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	BOX2OBJ_ItemIconPokePutHand( syswk->app, syswk->app->poke_put_key, BOX2MAIN_POKEMOVE_MODE_ALL );
	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );

	{
		u32 winID = BOX2BMP_PokeItemInfoWrite( syswk, syswk->app->get_item );
		BGWINFRM_BmpWinOn( syswk->app->wfrm, BOX2MAIN_WINFRM_SUBDISP, syswk->app->win[winID].win );
		BGWINFRM_BmpWinOn( syswk->app->wfrm, BOX2MAIN_WINFRM_SUBDISP, syswk->app->win[winID+1].win );
		BOX2OBJ_ItemIconChangeSub( syswk->app, syswk->app->get_item );
		BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeBoxPartyIconGetKey, BOX2SEQ_MAINSEQ_ITEMARRANGE_KEYGET_MOVE_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：持ち物整理・手持ちメイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	ret = BOX2UI_HitCheckTrgPartyPokeLeft();
	if( ret != GFL_UI_TP_HIT_NONE ){
//↑[GS_CONVERT_TAG]
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_POKE_CATCH );
			GFL_UI_TP_GetPointTrg( &syswk->app->tpx, &syswk->app->tpy );
//↑[GS_CONVERT_TAG]
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			return PartyItemGet( syswk, ret );
		}else{
			if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
				u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
				CURSORMOVE_PosSet( syswk->app->cmwk, pos );
				BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
				BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
				BOX2OBJ_PokeCursorVanish( syswk, FALSE );
				if( syswk->app->get_item != ITEM_DUMMY_DATA ){
					BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
				}
				return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeMenuClose, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN_CURMOVE_RET );
			}else{
				CURSORMOVE_PosSet( syswk->app->cmwk, ret - BOX2OBJ_POKEICON_TRAY_MAX );
				CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
				BOX2MAIN_PokeInfoOff( syswk );
				return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
			}
		}
	}

	ret = BOX2UI_BoxItemArrangePartyMoveMain( syswk );

	switch( ret ){

	case BOX2UI_ITEM_A_PARTY_CHANGE:	//「いれかえ」
//		Snd_SePlay( SE_BOX2_DECIDE );
		return PartyButtonAnmSet( syswk, 0, BOX2SEQ_MAINSEQ_ITEM_CHG_BUTTON );

	case BOX2UI_ITEM_A_PARTY_MENU1:	// バッグへ or もたせる
//		Snd_SePlay( SE_BOX2_DECIDE );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_MENU5, BOX2SEQ_MAINSEQ_ITEMARRANGE_BAG_BUTTON );

	case CURSORMOVE_CURSOR_MOVE:		// 移動
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos <= BOX2UI_ARRANGE_PARTY_POKE6 ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}else{
				if( pos != BOX2UI_ITEM_A_PARTY_MENU1 &&
					pos != BOX2UI_ITEM_A_PARTY_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN_CURMOVE_RET );

	case BOX2UI_ITEM_A_PARTY_RETURN:	//「もどる」
	case CURSORMOVE_CANCEL:				// キャンセル
//		Snd_SePlay( SE_BOX2_CLOSE_PARTY_TRAY );
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_PARTY_END_BUTTON );

	case BOX2UI_ITEM_A_PARTY_CLOSE:	//「とじる」
//		Snd_SePlay( SE_BOX2_CANCEL );
		{
			u8	pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, pos, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, pos );
		}
		return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_CLOSE_BTN, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_CLOSE_BUTTON );

	case CURSORMOVE_CURSOR_ON:			// カーソル表示
		{
			u8	pos = CURSORMOVE_PosGet( syswk->app->cmwk );
			if( pos <= BOX2UI_ARRANGE_PARTY_POKE6 ){
				BOX2MAIN_PokeInfoPut( syswk, pos+BOX2OBJ_POKEICON_TRAY_MAX );
			}else{
				if( pos != BOX2UI_ITEM_A_PARTY_MENU1 &&
					pos != BOX2UI_ITEM_A_PARTY_CLOSE ){
					BOX2MAIN_PokeInfoOff( syswk );
				}
			}
		}
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		break;

	case CURSORMOVE_NONE:				// 動作なし
		break;

	default:
		ret += BOX2OBJ_POKEICON_TRAY_MAX;
		if( BOX2MAIN_PokeParaGet( syswk, ret, ID_PARA_poke_exist, NULL ) != 0 ){
//			Snd_SePlay( SE_BOX2_DECIDE );
			BOX2MAIN_PokeInfoPut( syswk, ret );
			BOX2UI_CursorMoveVFuncWorkSet( syswk->app, BOX2UI_ITEM_A_PARTY_MENU1, CURSORMOVE_PosGet(syswk->app->cmwk) );
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_A_PARTY_MENU1 );
			return PartyItemGetKey( syswk, ret );
		}
	}

	return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちもをせいりする」手持ちポケモン選択メインへ戻る
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンのアイテム移動開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyIconMoveRet( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_ITEMMOVE_WORK * mvwk;

	mvwk = syswk->app->vfunk.work;

	if( mvwk->put_pos == syswk->get_pos || syswk->app->get_item == ITEM_DUMMY_DATA ){
		u32	set_pos = mvwk->set_pos;
		ItemMoveWorkFree( syswk );
		if( syswk->app->party_frm == 0 ){
			if( ITEM_CheckMail( syswk->app->get_item ) == TRUE && set_pos != syswk->get_pos ){
//				Snd_SePlay( SE_BOX2_WARNING );
				BGWINFRM_FrameOff( syswk->app->wfrm, BOX2MAIN_WINFRM_MENU5 );
				BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
				BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_MAIL_MOVE, BOX2BMPWIN_ID_MSG4 );
				syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_ERR;
				return BOX2SEQ_MAINSEQ_TRGWAIT;
			}
		}else{
			BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG4 );
			syswk->app->party_frm = 0;
		}
		if( syswk->app->get_item != ITEM_DUMMY_DATA ){
//			Snd_SePlay( SE_BOX2_POKE_PUT );
			BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );
		}
		CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_A_PARTY_MENU1 );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
	}

	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG4 );

//	Snd_SePlay( SE_BOX2_POKE_PUT );

	BOX2OBJ_PokeIconBlendSet( syswk->app, mvwk->put_pos, FALSE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );

	// アウトライン消去
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	// アイテムアイコン縮小
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );

	return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_POKEADD;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンのアイテム移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyIconMovePokeAdd( BOX2_SYS_WORK * syswk )
{
	POKEMON_PASO_PARAM * ppp;
	BOX2MAIN_ITEMMOVE_WORK * mvwk;
	u16	item;

	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_POKEADD;
	}

	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );

	mvwk = syswk->app->vfunk.work;

	// 移動先にアイテムをセット
	item = BOX2MAIN_PokeParaGet( syswk, mvwk->put_pos, ID_PARA_item, NULL );
	BOX2MAIN_PokeParaPut( syswk, mvwk->put_pos, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, mvwk->put_pos );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, mvwk->put_pos,
			syswk->app->pokeicon_id[mvwk->put_pos] );
	}
	BOX2MAIN_PokeInfoPut( syswk, mvwk->put_pos );

	// 移動元にアイテムをセット
	syswk->app->get_item = item;
	BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, syswk->app->get_item );
	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
	if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
		BOX2OBJ_PokeIconChange(
			syswk, syswk->tray, syswk->get_pos,
			syswk->app->pokeicon_id[syswk->get_pos] );
	}

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		if( syswk->app->party_frm == 0 ){
			syswk->get_pos = mvwk->put_pos;
			ItemMoveWorkFree( syswk );
			CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
			return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_POKEADD_END );
		}else{
			syswk->app->party_frm = 0;
			ItemMoveWorkFree( syswk );
			CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );
			BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
			BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrameMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_POKEADD_END );
		}
	}

//	Snd_SePlay( SE_BOX2_POKE_CATCH );

	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncItemIconPartyChange, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_CHANGE );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンのアイテム移動終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyIconMovePokeAddEnd( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンのアイテム交換
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyIconMoveChange( BOX2_SYS_WORK * syswk )
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

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		syswk->get_pos = mvwk->put_pos;
	}
	CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX );

	ItemMoveWorkFree( syswk );

	return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_CHANGE_END;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンのアイテム交換終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyIconMoveChangeEnd( BOX2_SYS_WORK * syswk )
{
	if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_CHANGE_END;
	}

	BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );

	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_POKEADD_END );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモンのアイテム移動エラー
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyIconMoveErr( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG4 );
	BGWINFRM_FrameOn( syswk->app->wfrm, BOX2MAIN_WINFRM_MENU5 );
	CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ITEM_A_PARTY_MENU1 );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス整理：タマゴにアイテムを持たせられないメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeItemSetEggError( BOX2_SYS_WORK * syswk )
{
//	Snd_SePlay( SE_BOX2_WARNING );
	BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_EGG, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス整理：メールを預かれないメッセージ表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeItemGetMailError( BOX2_SYS_WORK * syswk )
{
//	Snd_SePlay( SE_BOX2_WARNING );
	BOX2BMP_MailGetErrorPut( syswk, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}


//============================================================================================
//	設定関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * フェードイン設定
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	フェード後のシーケンス
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
 * フェードアウト設定
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	フェード後のシーケンス
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
 * ウェイト設定
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	ウェイト後のシーケンス
 * @param	wait	ウェイト
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
 * VBLANK関数セット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	func	VBLANK関数
 * @param	next	VBLANK関数終了後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int VFuncSet( BOX2_SYS_WORK * syswk, void * func, int next )
{
	syswk->next_seq = next;
	BOX2MAIN_VFuncSet( syswk->app, func );
	return BOX2SEQ_MAINSEQ_VFUNC;
}

//--------------------------------------------------------------------------------------------
/**
 * 設定済みのVBLANK関数開始リクエスト
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	VBLANK関数終了後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int VFuncReqSet( BOX2_SYS_WORK * syswk, int next )
{
	syswk->next_seq = next;
	BOX2MAIN_VFuncReqSet( syswk->app );
	return BOX2SEQ_MAINSEQ_VFUNC;
}

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	id		はい・いいえＩＤ
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int YesNoSet( BOX2_SYS_WORK * syswk, u32 id )
{
/*
	if( id == YESNO_ID_POKEFREE ){
		BOX2MAIN_YesNoWinSet( syswk, 1 );
	}else{
		BOX2MAIN_YesNoWinSet( syswk, 0 );
	}
	syswk->app->ynID = id;
*/
	syswk->app->ynID = id;
	BOX2MAIN_YesNoWinSet( syswk, 0 );
	return BOX2SEQ_MAINSEQ_YESNO;
}

//--------------------------------------------------------------------------------------------
/**
 * サブプロセスセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	type	サブプロセスタイプ
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
 * トレイ切り替え・左
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	切り替え後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayLeftScrollSet( BOX2_SYS_WORK * syswk, int next )
{
	if( syswk->tray == 0 ){
		syswk->tray = syswk->trayMax-1;
	}else{
		syswk->tray--;
	}

	BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
	BOX2MAIN_WallPaperSet(
		syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), BOX2MAIN_TRAY_SCROLL_L );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_L_ARROW, BOX2OBJ_ANM_L_ARROW_ON );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		if( syswk->dat->callMode == BOX_MODE_SEIRI || syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
			BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
		}
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncTrayScrollLeft, next );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイ切り替え・右
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	切り替え後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayRightScrollSet( BOX2_SYS_WORK * syswk, int next )
{
	if( syswk->tray == (syswk->trayMax-1) ){
		syswk->tray = 0;
	}else{
		syswk->tray++;
	}

	BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
	BOX2MAIN_WallPaperSet(
		syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), BOX2MAIN_TRAY_SCROLL_R );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_R_ARROW, BOX2OBJ_ANM_R_ARROW_ON );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		if( syswk->dat->callMode == BOX_MODE_SEIRI || syswk->dat->callMode == BOX_MODE_ITEM ){
			BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
			BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
		}
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncTrayScrollRight, next );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイのポケモンを取得
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayPokeGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_PokeCursorAdd( syswk );

	PokeMoveWorkAlloc( syswk );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == FALSE ){
		BOX2BGWFRM_TemochiButtonOutSet( syswk->app->wfrm );
		BOX2BGWFRM_IdouButtonOutSet( syswk->app->wfrm );
		BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	}
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );

	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_TRAY;
	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMove, BOX2SEQ_MAINSEQ_ARRANGE_TRAYPOKE_CHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイのポケモンを取得（手持ちに落とせるモード）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayPokeGetDrop( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_PokeCursorAdd( syswk );
	BOX2OBJ_PartyPokeIconFrmInSetRight( syswk );

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );

	BOX2BMP_PokePartyInMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );
	if( PokeParty_GetPokeCount( syswk->dat->pokeparty ) != TEMOTI_POKEMAX ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_TSURETEIKU );
	}else{
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_TSURETEIKU_ERR );
	}

	PokeMoveWorkAlloc( syswk );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == FALSE ){
		BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	}

	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
	return VFuncSet( syswk, BOX2MAIN_GetTrayPokeMoveDrop, BOX2SEQ_MAINSEQ_PARTYIN_POKECHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイのポケモンを取得（キー操作）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayPokeGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
	BOX2OBJ_PokeCursorAdd( syswk );

	BOX2BMP_PokePartyInMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncMenuOpenKey, BOX2SEQ_MAINSEQ_PARTYIN_MAIN_CURMOVE_RET );
}

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
static int TrayPokeGetKeyArrange( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
	BOX2OBJ_PokeCursorAdd( syswk );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
	BOX2BGWFRM_TemochiButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );

	BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncMenuOpenKeyArrange, BOX2SEQ_MAINSEQ_ARRANGE_MAIN_CURMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンを取得
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyPokeGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	POKEMON_PARAM * pp;

	syswk->get_pos = pos;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_PokeCursorAdd( syswk );

	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos-BOX2OBJ_POKEICON_TRAY_MAX );

	// 手持ち１匹
	if( BOX2MAIN_BattlePokeCheck( syswk, pos - BOX2OBJ_POKEICON_TRAY_MAX ) == FALSE &&
		BOXDAT_GetPokeExistCount2( syswk->dat->sv_box, syswk->tray ) == 0 ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_BATTLE_POKE_ERR );
	// メール
	}else if( ITEM_CheckMail( PP_Get(pp,ID_PARA_item,NULL) ) == TRUE ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_MAIL_ERR );
/*	カスタムボールはないハズ
	// カプセル
	}else if( PP_Get( pp, ID_PARA_cb_id, NULL ) != 0 ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_CAPSULE_ERR );
*/
	}else{
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_AZUKERU );
	}

	PokeMoveWorkAlloc( syswk );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMoveParty, BOX2SEQ_MAINSEQ_ARRANGE_PARTYPOKE_CHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンを取得（トレイに落とせるモード）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyPokeGetDrop( BOX2_SYS_WORK * syswk, u32 pos )
{
	POKEMON_PARAM * pp;
	u32	i;

	syswk->get_pos = pos;

	for( i=BOX2OBJ_POKEICON_TRAY_MAX; i<BOX2OBJ_POKEICON_MAX; i++ ){
		if( i == syswk->get_pos ){
			BOX2OBJ_PokeIconPriChg( syswk->app, i, BOX2OBJ_POKEICON_PRI_CHG_GET );
		}else{
			BOX2OBJ_PokeIconPriChg( syswk->app, i, BOX2OBJ_POKEICON_PRI_CHG_PUT );
		}
	}
	BOX2OBJ_PokeCursorAdd( syswk );

	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, TRUE );
	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, FALSE );

	BOX2BMP_PokePartyInMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );

	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos - BOX2OBJ_POKEICON_TRAY_MAX );

	// 手持ち１匹
	if( BOX2MAIN_BattlePokeCheck( syswk, pos - BOX2OBJ_POKEICON_TRAY_MAX ) == FALSE ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_BATTLE_POKE_ERR );
	// メール
	}else if( ITEM_CheckMail( PP_Get(pp,ID_PARA_item,NULL) ) == TRUE ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_MAIL_ERR );
/*	カスタムボールはないハズ
	// カプセル
	}else if( PP_Get( pp, ID_PARA_cb_id, NULL ) != 0 ){
		BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_CAPSULE_ERR );
*/
	}else{
		if( BOXDAT_GetEmptySpaceTray( syswk->dat->sv_box, syswk->tray ) == 0 ){
			BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_AZUKERU_ERR );
		}else{
			BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_AZUKERU );
		}
	}

	PokeMoveWorkAlloc( syswk );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == FALSE ){
		BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	}

	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_PARTY;
	return VFuncSet( syswk, BOX2MAIN_GetPartyPokeMoveDrop, BOX2SEQ_MAINSEQ_PARTYOUT_POKECHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンを取得（キー操作）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyPokeGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
	BOX2OBJ_PokeCursorAdd( syswk );

	BOX2BMP_PokePartyInMsgPut( syswk, syswk->get_pos, BOX2BMP_MSGID_PARTYIN_MENU, BOX2BMPWIN_ID_MSG3 );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncMenuOpenKey, BOX2SEQ_MAINSEQ_PARTYOUT_MAIN_CURMOVE_RET );
}

// 手持ちポケモンを取得（キー操作・ボックス整理）
static int PartyPokeGetKeyArrange( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
	BOX2OBJ_PokeCursorAdd( syswk );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );

	BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );

	return VFuncSet( syswk, BOX2MAIN_VFuncMenuOpenKeyArrange, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN_CURMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイ/手持ちポケモンを取得
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyPokeGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_PokeCursorAdd( syswk );

	if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_BoxMoveButtonOutSet( syswk->app->wfrm );
	}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
	}

	BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );

	BOX2BMP_ButtonPutYStatus( syswk );

	PokeMoveWorkAlloc( syswk );
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
	return VFuncSet( syswk, BOX2MAIN_VFuncGetPokeMoveBoxParty, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTYPOKE_CHG_END );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコン動作ワーク取得
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeMoveWorkAlloc( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	i;

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
 * ポケモンアイコン動作ワーク解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeMoveWorkFree( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	u32	i;

	work = syswk->app->vfunk.work;

/*
	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
		GFL_HEAP_FreeMemory( work->dat[i].pp );
	}
*/

	GFL_HEAP_FreeMemory( work );
//↑[GS_CONVERT_TAG]
}

//--------------------------------------------------------------------------------------------
/**
 * もちものメニュー
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PokeItemCheck( BOX2_SYS_WORK * syswk )
{
	u32 item;

	// タマゴ
	if( BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_tamago_flag, NULL ) != 0 ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeFreeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_ITEMSET_EGG_ERROR );
	}
	
	item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_item, NULL );

	if( item == ITEM_DUMMY_DATA ){
		syswk->cur_rcv_pos = BOX2UI_ARRANGE_MAIN_ITEM;
		return SubProcSet( syswk, SUB_PROC_TYPE_BAG );
	}else{
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		if( ITEM_CheckMail( item ) == TRUE ){
			BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
			return VFuncSet( syswk, BOX2MAIN_VFuncPokeFreeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_ITEMGET_MAIL_ERROR );
		}else{
			BOX2BGWFRM_PokeMenuOff( syswk->app->wfrm );
		}
	}
	BOX2BMP_ItemGetCheckMsgPut( syswk, item, BOX2BMPWIN_ID_MSG1 );
	return YesNoSet( syswk, YESNO_ID_ITEMGET );
}

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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
		BOX2BMP_BoxMoveNameNumWrite( syswk );
	}

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
	}
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );

	BOX2BGWFRM_MarkingFrameOutSet( syswk->app->wfrm );

	if( syswk->dat->callMode == BOX_MODE_SEIRI && syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		return VFuncSet( syswk, BOX2MAIN_VFuncMarkingFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_MARKING_RCV );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncMarkingFrameMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV );
}

//--------------------------------------------------------------------------------------------
/**
 * マーキング開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MarkingStartSet( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_MARKING_INIT );
}

//--------------------------------------------------------------------------------------------
/**
 * 逃がす開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PokeFreeStartSet( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeFreeMenuMove, BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_START );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス/手持ちボタンアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyButtonOutSet( BOX2_SYS_WORK * syswk )
{
	syswk->party_sel_flg = 1;

	BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	BOX2BGWFRM_TemochiButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ARRANGE_PARTYPOKE_FRMIN );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * 手持ちフレームセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PartyFrmSet( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFrameLoadArrange( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY );
	BOX2BMP_PartyCngButtonFrmPut( syswk->app );
	BOX2BMP_PartyChangeButtonPut( syswk->app );

	BOX2BGWFRM_BoxPartyButtonVanish( syswk->app->wfrm );
	BOX2BGWFRM_ButtonPutYameru( syswk );
	BOX2BGWFRM_PartyPokeFramePut( syswk->app->wfrm );
	BOX2OBJ_PartyPokeIconFrmSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちフレームセット（左）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PartyFrmSetRight( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PartyPokeFrameLoadArrange( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY );
	BOX2BMP_PartyCngButtonFrmPut( syswk->app );
	BOX2BMP_PartySelectButtonPut( syswk->app );

	BOX2BGWFRM_BoxPartyButtonVanish( syswk->app->wfrm );
	BOX2BGWFRM_PartyPokeFramePutRight( syswk->app->wfrm );
	BOX2OBJ_PartyPokeIconFrmSetRight( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちフレームセット（ポケモンをあずける）
 *
 * @param	syswk	ボックス画面システムワーク
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
 * 手持ちフレームを右へ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyFrmMoveRight( BOX2_SYS_WORK * syswk )
{
	syswk->get_pos = BOX2MAIN_GETPOS_NONE;
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_PartyPokeFrameRightMoveSet( syswk->app->wfrm );
	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	}
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrmRight, BOX2SEQ_MAINSEQ_ARRANGE_PARTYFRM_MOVE_R_END );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちフレームを左へ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyFrmMoveLeft( BOX2_SYS_WORK * syswk )
{
	syswk->get_pos = BOX2MAIN_GETPOS_NONE;
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	BOX2BGWFRM_PartyPokeFrameLeftMoveSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPartyFrmLeft, BOX2SEQ_MAINSEQ_ARRANGE_PARTYFRM_MOVE_L_END );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス/手持ちボタンアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveButtonOutSet( BOX2_SYS_WORK * syswk )
{
	syswk->box_mv_flg = 1;		// ボックス移動フラグ
	syswk->pokechg_mode = 0;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	BOX2BGWFRM_TemochiButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncUnderButtonMove, BOX2SEQ_MAINSEQ_ARRANGE_BOXMOVE_FRMIN );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス移動終了開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタンフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int YStatusButtonFrmOut( BOX2_SYS_WORK * syswk, int next )
{
	BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, next );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」フレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveMenuFrmOut( BOX2_SYS_WORK * syswk, u32 pos )
{
	int	next;

	syswk->box_mv_pos = pos;
	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameNumWrite( syswk );

	if( syswk->party_sel_flg == 0 ){
		next = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
	}else{
		next = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
	}
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveMenuOut, next );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタンフレームイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveButtonFrmIn( BOX2_SYS_WORK * syswk, u32 ret )
{
	int	next;
	u32	tmp;

	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		next = BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
	}else{
		next = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
	}

	// トレイ名と数表示
	tmp = BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + ret;
	if( tmp != syswk->box_mv_pos ){
		syswk->box_mv_pos = tmp;
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2BMP_BoxMoveNameNumWrite( syswk );
	}

	// 現在のトレイと同じ
	if( syswk->box_mv_pos == syswk->tray ){
		// ボタンが表示済みなら
		if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
			return BoxMoveButtonFrmOut( syswk, next );
		}
		if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, BOX2UI_ARRANGE_MOVE_TRAY1 + syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
			BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
			return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, next );
		}
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
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

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタンフレームアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveButtonFrmOut( BOX2_SYS_WORK * syswk, int next )
{
	BOX2BGWFRM_BoxMoveButtonOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveButtonYStatusButtonMove, next );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス移動・トレイ切り替えセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	mode	スクロールモード
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveTrayScrollSet( BOX2_SYS_WORK * syswk, u32 mode )
{
	void * func;
	u32	tmp;

	tmp = syswk->tray;
	syswk->tray = syswk->box_mv_pos;

	if( tmp > syswk->box_mv_pos ){
		BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
		BOX2MAIN_WallPaperSet(
			syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), BOX2MAIN_TRAY_SCROLL_L );
		func = BOX2MAIN_VFuncTrayScrollLeft;
	}else{
		BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
		BOX2MAIN_WallPaperSet(
			syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), BOX2MAIN_TRAY_SCROLL_R );
		func = BOX2MAIN_VFuncTrayScrollRight;
	}

	switch( mode ){
	case 0:		// 通常
		BOX2OBJ_BoxMoveCursorSet( syswk );
		if( syswk->party_sel_flg == 0 ){
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
		}else{
			syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN;
		}
		break;

	case 1:		// ボックステーマ変更
		BOX2OBJ_BoxMoveCursorSet( syswk );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
		break;

	case 2:		// 壁紙変更
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_WHITE_OUT;
		break;

	case 3:		// ポケモン移動
		BOX2OBJ_BoxMoveCursorSet( syswk );
		if( BOX2BGWFRM_BoxMoveButtonCheck(syswk->app->wfrm) == TRUE ){
			BOX2BGWFRM_BoxMoveButtonOutSet( syswk->app->wfrm );
		}
		syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTY_CHG_MAIN;
		break;

	case 4:		// 持ち物整理
		BOX2OBJ_BoxMoveCursorSet( syswk );
		if( BOX2BGWFRM_BoxMoveButtonCheck(syswk->app->wfrm) == TRUE ){
			BOX2BGWFRM_BoxMoveButtonOutSet( syswk->app->wfrm );
		}
		syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
		break;
	}

	return VFuncSet( syswk, func, syswk->next_seq );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス名表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	ret		選択位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxMoveNameChange( BOX2_SYS_WORK * syswk, u32 ret )
{
	u32	tmp;

	tmp = BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + ret;
	if( syswk->box_mv_pos == tmp ){
		return;
	}
	syswk->box_mv_pos = tmp;

	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameNumWrite( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス移動・トレイ切り替え矢印
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	mv		移動方向
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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
	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2OBJ_BoxMoveCursorSet( syswk );
	BOX2BMP_BoxMoveNameNumWrite( syswk );

	if( mv > 0 ){
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_BOXMV_RA, BOX2OBJ_ANM_R_ARROW_ON );
	}else{
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_BOXMV_LA, BOX2OBJ_ANM_L_ARROW_ON );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス移動フレームインセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	フレーム移動後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveFrmInSet( BOX2_SYS_WORK * syswk, int next )
{
	BOX2OBJ_BoxMoveObjInit( syswk );
	BOX2OBJ_BoxMoveCursorSet( syswk );
	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameNumWrite( syswk );

	BOX2BGWFRM_BoxMoveFrmInSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncBoxMoveFrmIn, next );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス移動フレームアウトセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	フレーム移動後のシーケンス
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
 * 「やめる」フレームインセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	フレーム移動後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveExitButtonInSet( BOX2_SYS_WORK * syswk, int next )
{
	BOX2BMP_ButtonPutYameru( syswk );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncExitButtonMove, next );
}

//--------------------------------------------------------------------------------------------
/**
 * 「やめる」or「もどる」フレームアウトセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	フレーム移動後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxMoveExitButtonOutSet( BOX2_SYS_WORK * syswk, int next )
{
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncExitButtonMove, next );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューアウトセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	フレーム移動後のシーケンス
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
 * ボックステーマ変更開始セット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	next	フレーム移動後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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
static int BoxThemaTrayIconSel( BOX2_SYS_WORK * syswk, u32 ret )
{
	const CURSORMOVE_DATA * pw;
	u8	pos;
	u8	x, y;

	syswk->box_mv_pos = BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + ret;
	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameNumWrite( syswk );

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
static int BoxThemaTrayIconChg( BOX2_SYS_WORK * syswk, s8 mv )
{
	BoxMoveTrayIconChgOnly( syswk, mv );
	CURSORMOVE_PosSet( syswk->app->cmwk, (syswk->box_mv_pos%BOX2OBJ_TRAYICON_MAX) );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_ARRANGE_BOXTHEMA_CHG_MAIN;
}

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

	if( mv > 0 ){
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_BOXMV_RA, BOX2OBJ_ANM_R_ARROW_ON );
	}else{
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_BOXMV_LA, BOX2OBJ_ANM_L_ARROW_ON );
	}

	// OBJ変更
	BOX2OBJ_WallPaperObjChange( syswk );
	// カーソル位置
	BOX2OBJ_WallPaperCursorSet( syswk );
	// 名前変更
	BOX2BMP_WallPaperNameWrite( syswk );

	CURSORMOVE_PosSet( syswk->app->cmwk, (syswk->app->wallpaper_pos%BOX2OBJ_WPICON_MAX) );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
}

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

//--------------------------------------------------------------------------------------------
/**
 * 壁紙変更
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * ポケモンを取得（ポケモンをつれていく/あずける）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyInPokeCheck( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	// 手持ちに空きがあるか
	if( PokeParty_GetPokeCount( syswk->dat->pokeparty ) != TEMOTI_POKEMAX ){
		BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
		BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
		return VFuncSet( syswk, BOX2MAIN_VFuncPokeMenuMove, BOX2SEQ_MAINSEQ_PARTYIN_PARTY_FRM_IN );
	}
	BOX2BGWFRM_PokeMenuOff( syswk->app->wfrm );
	BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_ERR, BOX2BMPWIN_ID_MSG1 );
	syswk->next_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEMENU_RCV;
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモン預けるメニュー開始
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyOutInit( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeFreeMenuMove, BOX2SEQ_MAINSEQ_PARTYOUT_START_CHECK );
}

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
	BOX2BMP_BoxMoveNameNumWrite( syswk );

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
static int PokeOutTrayIconChg( BOX2_SYS_WORK * syswk, s8 mv )
{
	BoxMoveTrayIconChgOnly( syswk, mv );
	CURSORMOVE_PosSet( syswk->app->cmwk, (syswk->box_mv_pos%BOX2OBJ_TRAYICON_MAX) );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
	return BOX2SEQ_MAINSEQ_PARTYOUT_BOXSELECT_MAIN;
}

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
static int PartyOutEnd( BOX2_SYS_WORK * syswk, BOOL flg )
{
	return BgButtonAnmSet( syswk, BOX2MAIN_WINFRM_RET_BTN, BOX2SEQ_MAINSEQ_PARTYOUT_BOXMENU_FRM_OUT );
}

//--------------------------------------------------------------------------------------------
/**
 * 預ける実行
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * 手カーソルでポケモン取得
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

	syswk->app->poke_get_key = 1;

	BOX2OBJ_PokeIconPriChg( syswk->app, syswk->get_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );

	if( syswk->quick_mode == 0 ){
		BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMoveGetKey, BOX2SEQ_MAINSEQ_KEYGET_POKEMOVE_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * 手カーソルでポケモン配置
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		配置位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PokeMovePutKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	if( pos == BOX2MAIN_GETPOS_NONE ){
		syswk->app->cancel_flg = TRUE;
	}else{
		syswk->app->cancel_flg = FALSE;
	}

	{	// ポケモンアイコンの位置を補正する
		s16	x, y;
		BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, &x, &y, CLSYS_DEFREND_MAIN );
		BOX2OBJ_SetPos(
			syswk->app, syswk->app->pokeicon_id[syswk->get_pos], x, y+4, CLSYS_DEFREND_MAIN );
	}

	// 現在のトレイor手持ちに配置
	if( pos < BOX2OBJ_POKEICON_MAX ){
		// 手持ち
		if( pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			u32	ppcnt = PokeParty_GetPokeCount( syswk->dat->pokeparty );
			if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				if( ( pos - BOX2OBJ_POKEICON_TRAY_MAX ) > ppcnt ){
					BOX2MAIN_PokeInfoOff( syswk );
					BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
				}else{
					BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				}
			}else{
				if( ( pos - BOX2OBJ_POKEICON_TRAY_MAX ) >= ppcnt ){
					BOX2MAIN_PokeInfoOff( syswk );
					BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
				}else{
					BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
				}
			}
		// 現在のトレイ
		}else{
			BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
		}
	// 別トレイ
	}else if( pos != BOX2MAIN_GETPOS_NONE ){
		syswk->box_mv_pos = BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + pos - BOX2UI_ARRANGE_MOVE_TRAY1;
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2BMP_BoxMoveNameNumWrite( syswk );
		if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			u32	party_pos;
			POKEMON_PARAM * pp;

			party_pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, party_pos );

			if( ITEM_CheckMail( PP_Get(pp,ID_PARA_item,NULL) ) == TRUE ){
				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
/*	カスタムボールはないハズ
			}else if( PP_Get( pp, ID_PARA_cb_id, NULL ) != 0 ){
				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
*/
			}else if( BOX2MAIN_BattlePokeCheck( syswk, party_pos ) == FALSE ){
				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
			}else if( syswk->box_mv_pos == syswk->tray ){
				BOX2MAIN_PokeInfoOff( syswk );
				BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
			}else if( BOXDAT_GetEmptySpaceTray( syswk->dat->sv_box, syswk->box_mv_pos ) == 0 ){
				BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
			}else{
				BOX2MAIN_PokeInfoOff( syswk );
				BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
			}
		}else if( syswk->box_mv_pos == syswk->tray ){
			BOX2MAIN_PokeInfoOff( syswk );
			BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
		}else if( BOXDAT_GetEmptySpaceTray( syswk->dat->sv_box, syswk->box_mv_pos ) == 0 ){
			BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
		}
	// キャンセル
	}else{
		u8	cur_pos = CURSORMOVE_PosGet( syswk->app->cmwk );
		if( cur_pos < BOX2UI_ARRANGE_MOVE_RETURN ){
			if( BOX2MAIN_PokeInfoPut( syswk, cur_pos ) == FALSE ){
				BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
			}
		}else{
			BOX2MAIN_PokeInfoOff( syswk );
			BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
		}
	}

	syswk->app->poke_put_key = pos;
	syswk->app->poke_get_key = 0;

	PokeMoveWorkAlloc( syswk );
	syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeMovePutKey, BOX2SEQ_MAINSEQ_ARRANGE_BOXPARTYPOKE_CHG_END_KEY );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイのアイテムを取得（キー操作）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayItemGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_item, NULL );

	BOX2BMP_ItemArrangeMenuStrPrint( syswk, syswk->app->get_item );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
	BOX2BGWFRM_TemochiButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );

		BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeGetKey, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN_CURMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイのアイテムを取得（タッチ）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int TrayItemGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	u16	tmp;

	tmp = syswk->app->get_item;

	syswk->get_pos = pos;

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_item, NULL );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		if( tmp != 0 || syswk->app->get_item != 0 ){
			BOX2BGWFRM_PokeMenuOff( syswk->app->wfrm );
		}else if( syswk->app->get_item != 0 ){
			BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		}
	}else{
		BOX2BGWFRM_TemochiButtonOutSet( syswk->app->wfrm );
		BOX2BGWFRM_IdouButtonOutSet( syswk->app->wfrm );
		BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	}
	BOX2BMP_ItemArrangeMenuStrPrint( syswk, syswk->app->get_item );

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );
		BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );
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

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeGetTouch, BOX2SEQ_MAINSEQ_ITEMARRANGE_ICONMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイ/手持ちアイテムを取得
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyItemGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_item, NULL );

	BOX2BMP_ButtonPutYStatus( syswk );

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
		CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
			return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
		}else if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
			BOX2BGWFRM_BoxMoveButtonOutSet( syswk->app->wfrm );
		}
		return VFuncSet( syswk, BOX2MAIN_VFuncYStatusButtonMove, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN );
	}

//	Snd_SePlay( SE_BOX2_POKE_CATCH );

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
	BOX2OBJ_ItemIconCursorAdd( syswk->app );

	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );

	BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );

	if( BOX2BGWFRM_BoxMoveButtonCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_BoxMoveButtonOutSet( syswk->app->wfrm );
	}else if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
	}

	{
		s16	x, y;

		BOX2OBJ_PokeIconDefaultPosGet( syswk->get_pos, &x, &y, BOX2MAIN_POKEMOVE_MODE_ALL );
		syswk->app->tpx = x + 8;
		syswk->app->tpy = y + 8;
	}

	ItemMoveWorkAlloc( syswk );

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeBoxPartyIconSetTouch, BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_ICONMOVE_SUBFRM_OUT );
}

//--------------------------------------------------------------------------------------------
/**
 * 手カーソルでトレイ/手持ちアイテムを取得
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyItemGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;
	syswk->app->get_item_init_pos = pos;

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_item, NULL );

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_CHG_MAIN;
	}

//	Snd_SePlay( SE_BOX2_POKE_CATCH );

	syswk->app->poke_get_key = 1;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	BOX2OBJ_ItemIconPokePutHand( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
	BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );

	BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );

	BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );

	if( BOX2BGWFRM_YStatusButtonCheck( syswk->app->wfrm ) == TRUE ){
		BOX2BGWFRM_YStatusButtonOutSet( syswk->app->wfrm );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeBoxPartyIconGetKey, BOX2SEQ_MAINSEQ_ITEMARRANGE_KEYGET_MOVE_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * 手カーソルでアイテムを配置・キャンセル処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyItemPutKeyCancel( BOX2_SYS_WORK * syswk )
{
	u32	pos = CURSORMOVE_PosGet( syswk->app->cmwk );

	if( pos < BOX2UI_ARRANGE_MOVE_RETURN ){
		if( BOX2MAIN_PokeInfoPut( syswk, pos ) == TRUE ){
			BOX2BGWFRM_YStatusButtonInSet( syswk->app->wfrm );
		}
	}else{
		BOX2MAIN_PokeInfoOff( syswk );
	}
	ItemMoveWorkAlloc( syswk );
	return VFuncSet(
			syswk,
			BOX2MAIN_VFuncItemArrangeBoxPartyIconPutKeyCancel,
			BOX2SEQ_MAINSEQ_ITEMARRANGE_BOXPARTY_ICONMOVE_END );
}

//--------------------------------------------------------------------------------------------
/**
 * 手カーソルでアイテムを配置
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		配置位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxPartyItemPutKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->app->poke_put_key = pos;
	syswk->app->poke_get_key = 0;

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

	BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeBoxPartyIconPutKey, BOX2SEQ_MAINSEQ_ITEMARRANGE_KEYGET_CHANGE_CHECK );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンのアイテムを取得（キー操作・持ち物整理）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyItemGetKey( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->get_pos = pos;

	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_item, NULL );

	BOX2BMP_ItemArrangeMenuStrPrint( syswk, syswk->app->get_item );

	BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );
		BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );
	}

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeGetKey, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN_CURMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンのアイテムを取得（タッチ）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		取得位置
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PartyItemGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	u16	tmp;

	syswk->get_pos = pos;

	tmp = syswk->app->get_item;
	syswk->app->get_item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_item, NULL );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == TRUE ){
		if( tmp != 0 || syswk->app->get_item != 0 ){
			BOX2BGWFRM_PokeMenuOff( syswk->app->wfrm );
		}else if( syswk->app->get_item != 0 ){
			BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
		}
	}else{
		BOX2BGWFRM_RetButtonOutSet( syswk->app->wfrm );
	}
	BOX2BMP_ItemArrangeMenuStrPrint( syswk, syswk->app->get_item );

	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		BOX2OBJ_ItemIconPokePut( syswk->app, syswk->get_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
		BOX2OBJ_ItemIconChange( syswk->app, syswk->app->get_item );
		BOX2OBJ_ItemIconCursorAdd( syswk->app );
		BOX2OBJ_PokeIconBlendSet( syswk->app, syswk->get_pos, TRUE );
		BOX2BGWFRM_SubDispItemFrmInSet( syswk->app->wfrm );
		if( ITEM_CheckMail( syswk->app->get_item ) == TRUE ){
			BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_MAIL_MOVE );
		}else{
			BOX2BMP_VBlankMsgSet( syswk, BOX2BMPWIN_ID_MSG4, BOX2BMP_MSGID_VBLANK_ITEM_MOVE );
		}
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

	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangePartyGetTouch, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_ICONMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * 持ち物整理・メニュー処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int ItemArrangeMenuStart( BOX2_SYS_WORK * syswk )
{
	int	next;

	if( syswk->app->get_item == ITEM_DUMMY_DATA ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		{
			POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
			if( PPP_Get( ppp, ID_PARA_tamago_flag, NULL ) == 0 ){
				syswk->cur_rcv_pos = BOX2UI_ITEM_A_MAIN_MENU1;
				return SubProcSet( syswk, SUB_PROC_TYPE_BAG );
			}
		}
		next = BOX2SEQ_MAINSEQ_ITEMARRANGE_GET_BAG_CHECK;
	}else{
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
		next = BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_CHECK;
	}

	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	return VFuncSet( syswk, BOX2MAIN_VFuncPokeFreeMenuMove, next );
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン動作ワーク取得
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ItemMoveWorkAlloc( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_ITEMMOVE_WORK * work;

	work = GFL_HEAP_AllocMemory( HEAPID_BOX_APP, sizeof(BOX2MAIN_ITEMMOVE_WORK) );
//↑[GS_CONVERT_TAG]

	syswk->app->vfunk.work = work;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン動作ワーク解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ItemMoveWorkFree( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
}


//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手カーソルで取得中のポケモンをキーで移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_KeyGetPokeMoveMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;

	// Ｙステータスボタン
	if( BOX2UI_YStatusButtonCheck( syswk ) == TRUE ){
//		Snd_SePlay( SE_BOX2_DECIDE );
		syswk->y_status_flg  = 1;
		syswk->y_status_hand = 1;
		syswk->y_status_pos  = CURSORMOVE_PosGet( syswk->app->cmwk );
		return YStatusButtonAnmSet( syswk, BOX2MAIN_WINFRM_Y_ST_BTN, BOX2SEQ_MAINSEQ_STATUS_BUTTON );
	}

	ret = BOX2UI_BoxArrangePokeMoveHand( syswk );

	if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == FALSE ){
		ret = CURSORMOVE_CANCEL;
	}

#ifdef	BTS_2299_TEST
	if( test_flg_2299 != 0 ){
		ret = CURSORMOVE_CANCEL;
	}
#endif	// BTS_2299_TEST

	switch( ret ){
	case CURSORMOVE_CURSOR_MOVE:	// 移動
//		Snd_SePlay( SE_BOX2_MOVE_CURSOR );
		return VFuncReqSet( syswk, BOX2SEQ_MAINSEQ_KEYGET_POKEMOVE_MAIN_CURMOVE_RET );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
	case CURSORMOVE_NONE:			// 動作なし
		break;

	case BOX2UI_ARRANGE_MOVE_RETURN:	// 36: やめる
	case CURSORMOVE_CANCEL:				// キャンセル
		return PokeMovePutKey( syswk, BOX2MAIN_GETPOS_NONE );

	case BOX2UI_ARRANGE_MOVE_TRAY1:	// 37: トレイアイコン
	case BOX2UI_ARRANGE_MOVE_TRAY2:	// 38: トレイアイコン
	case BOX2UI_ARRANGE_MOVE_TRAY3:	// 39: トレイアイコン
	case BOX2UI_ARRANGE_MOVE_TRAY4:	// 40: トレイアイコン
	case BOX2UI_ARRANGE_MOVE_TRAY5:	// 41: トレイアイコン
	case BOX2UI_ARRANGE_MOVE_TRAY6:	// 42: トレイアイコン
	default:
		return PokeMovePutKey( syswk, ret );
	}

	return BOX2SEQ_MAINSEQ_KEYGET_POKEMOVE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手取得中のポケモン移動後にキー操作に戻す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_KeyGetPokeMoveMainCurMoveRet( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->vfunk.work );
//↑[GS_CONVERT_TAG]
	return BOX2SEQ_MAINSEQ_KEYGET_POKEMOVE_MAIN;
}


//============================================================================================
//	はい・いいえ
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 道具を預かる：はい
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_ItemGetYes( BOX2_SYS_WORK * syswk )
{
	u32 item = BOX2MAIN_PokeParaGet( syswk, syswk->get_pos, ID_PARA_item, NULL );

	// アイテムチェック
	if( MYITEM_AddItem( syswk->dat->myitem, item, 1, HEAPID_BOX_APP ) == TRUE ){
		POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

		BOX2BMP_ItemGetMsgPut( syswk, item, BOX2BMPWIN_ID_MSG1 );
		item = 0;
		BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, item );
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
	return BOX2SEQ_MAINSEQ_TRGWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンを逃がす：はい
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_PokeFreeYes( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2MAIN_PokeFreeCreate( syswk );
	BOX2OBJ_PokeIconFreeStart( syswk->app->seqwk );

	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );

	return BOX2SEQ_MAINSEQ_ARRANGE_POKEFREE_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * バッグへ戻す：はい
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxItemArg_RetBagYes( BOX2_SYS_WORK * syswk )
{
	u16	item;

	if( MYITEM_AddItem( syswk->dat->myitem, syswk->app->get_item, 1, HEAPID_BOX_APP ) == FALSE ){
//		Snd_SePlay( SE_BOX2_WARNING );
		BOX2BMP_ItemArrangeMsgPut( syswk, BOX2BMP_MSGID_ITEM_A_MAX, BOX2BMPWIN_ID_MSG1 );
		syswk->next_seq = BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_CANCEL;
		return BOX2SEQ_MAINSEQ_TRGWAIT;
	}

	item = ITEM_DUMMY_DATA;
	BOX2MAIN_PokeParaPut( syswk, syswk->get_pos, ID_PARA_item, item );

	{
		POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );
		if( BOX2MAIN_PokeItemFormChange( ppp ) == TRUE ){
			BOX2OBJ_PokeIconChange(
				syswk, syswk->tray, syswk->get_pos,
				syswk->app->pokeicon_id[syswk->get_pos] );
		}
		BOX2MAIN_PokeInfoPut( syswk, syswk->get_pos );
	}

	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
	BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
	return BOX2SEQ_MAINSEQ_ITEMARRANGE_RET_BAG_ENTER;
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス終了：はい
 *
 * @param	syswk	ボックス画面システムワーク
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
 * ボックス終了：いいえ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BoxArg_BoxEndNo( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG1 );
	CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );

	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:		// あずける
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
		return BOX2SEQ_MAINSEQ_PARTYOUT_MAIN;

	case BOX_MODE_TURETEIKU:	// つれていく
		BOX2BMP_PokePartyInMsgPut( syswk, 0, BOX2BMP_MSGID_PARTYIN_INIT, BOX2BMPWIN_ID_MSG3 );
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		return BOX2SEQ_MAINSEQ_PARTYIN_MAIN;

	case BOX_MODE_SEIRI:		// せいり
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;

	case BOX_MODE_ITEM:			// どうぐせいり
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, FALSE );
		return BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN;
	}

	return BOX2SEQ_MAINSEQ_ARRANGE_MAIN;
}


//============================================================================================
//	ボタン関連
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 通常のＢＧボタンアニメセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	wfrmID	ＢＧウィンドウフレームＩＤ
 * @param	next	ボタンアニメ後のシーケンス
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
 * Ｙステータス用のＢＧボタンアニメセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	wfrmID	ＢＧウィンドウフレームＩＤ
 * @param	next	ボタンアニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームのＢＧボタンアニメセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	no		ボタン番号
 * @param	next	ボタンアニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MarkingButtonAnmSet( BOX2_SYS_WORK * syswk, u32 no, int next )
{
	syswk->app->bawk.btn_mode = BOX2MAIN_BTN_ANM_MODE_BG;
	syswk->app->bawk.btn_id   = BGWINFRM_BGFrameGet( syswk->app->wfrm, BOX2MAIN_WINFRM_MARK );
	syswk->app->bawk.btn_pal1 = 3;
	syswk->app->bawk.btn_pal2 = 2;
	syswk->app->bawk.btn_seq  = 0;
	syswk->app->bawk.btn_cnt  = 0;

	if( no == 0 ){
		syswk->app->bawk.btn_py = 16;
	}else{
		syswk->app->bawk.btn_py = 20;
	}
	syswk->app->bawk.btn_px = 22;
	syswk->app->bawk.btn_sx = 9;
	syswk->app->bawk.btn_sy = 4;

	syswk->next_seq = next;
	return BOX2SEQ_MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちフレームのＢＧボタンアニメセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	no		ボタン番号
 * @param	next	ボタンアニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * ボックス終了確認時のパッシブセット
 *
 * @param	syswk	ボックス画面システムワーク
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
	case BOX_MODE_SEIRI:		// せいり
	case BOX_MODE_ITEM:			// どうぐせいり
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックス終了「はい・いいえ」セット（ボタン）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
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
 * メインシーケンス：ボックス終了「はい・いいえ」セット（キャンセル）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxEndCancelInit( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2BMP_BoxEndMsgPut( syswk, BOX2BMPWIN_ID_MSG1, FALSE );
	BoxEndPassiveSet( syswk );
	return YesNoSet( syswk, YESNO_ID_BOX_END_CANCEL );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」手持ちポケモンボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	return BoxPartyButtonOutSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」ポケモン移動ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMoveButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	return BoxMoveButtonOutSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモン移動」メニューボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeMoveMenuButton( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );

	syswk->box_mv_flg = 1;		// ボックス移動フラグ
	syswk->pokechg_mode = 0;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );

	return PokeMenuOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_BOXMOVE_FRMIN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ようすをみる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_StatusButton( BOX2_SYS_WORK * syswk )
{
	return SubProcSet( syswk, SUB_PROC_TYPE_POKESTATUS );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちもの」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemButton( BOX2_SYS_WORK * syswk )
{
	return PokeItemCheck( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「マーキング」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MarkingButton( BOX2_SYS_WORK * syswk )
{
	return MarkingStartSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「マーキング」終了ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MarkingEndButton( BOX2_SYS_WORK * syswk )
{
	return MarkingScrollOutSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「にがす」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_FreeButton( BOX2_SYS_WORK * syswk )
{
	return PokeFreeStartSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」閉じるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuCloseKeyArrange, BOX2SEQ_MAINSEQ_ARRANGE_MAIN_CURMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモン「いれかえ」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyChgButton( BOX2_SYS_WORK * syswk )
{
	if( syswk->quick_get == 0 ){
		BOX2MAIN_PokeSelectOff( syswk );
	}else{
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	}
	return PartyFrmMoveRight( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモン閉じるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangePartyCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuCloseKeyArrange, BOX2SEQ_MAINSEQ_ARRANGE_PARTY_MAIN_CURMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：手持ちポケモン「やめる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyEndButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	return PartyFrmOutSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ボックスせいり」ボックス切り替えボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeBoxMoveButton( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	return BoxMoveTrayScrollSet( syswk, 3 );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ポケモン入れ替え終了ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxMoveEndButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	return BoxMoveEndStart( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ設定「ボックスをきりかえる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaTrayChgButton( BOX2_SYS_WORK * syswk )
{
	return BoxMoveTrayScrollSet( syswk, 1 );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ設定「かべがみ」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaWallPaperButton( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_SysWinVanish( syswk->app, BOX2BMPWIN_ID_MSG3 );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
	return BoxMoveFrmOutSet( syswk, BOX2SEQ_MAINSEQ_ARRANGE_WALLPAPER_FRM_IN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ設定「なまえ」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_BoxThemaNameButton( BOX2_SYS_WORK * syswk )
{
	return SubProcSet( syswk, SUB_PROC_TYPE_NAMEIN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ設定「かべがみをはりかえる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ArrangeWallPaperChgButton( BOX2_SYS_WORK * syswk )
{
	return BoxMoveTrayScrollSet( syswk, 2 );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボックステーマ設定壁紙「やめる」ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」連れて行くボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInButton( BOX2_SYS_WORK * syswk )
{
	return PartyInPokeCheck( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをつれていく」閉じるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyInCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuCloseKey, BOX2SEQ_MAINSEQ_PARTYIN_MENU_CLOSE_END );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」預けるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutButton( BOX2_SYS_WORK * syswk )
{
	return PartyOutInit( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンをあずける」閉じるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PartyOutCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_ButtonPutModoru( syswk );
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_SubDispWazaFrmOutSet( syswk->app );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	return VFuncSet( syswk, BOX2MAIN_VFuncMenuCloseKey, BOX2SEQ_MAINSEQ_PARTYOUT_MENU_CLOSE_END );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」手持ちポケモンボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	return BoxPartyButtonOutSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」ポケモン移動ボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeMoveButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
	return BoxMoveButtonOutSet( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」メニューボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeBagButton( BOX2_SYS_WORK * syswk )
{
	return ItemArrangeMenuStart( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」閉じるボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_TemochiButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_IdouButtonInSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
	}
	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeMenuClose, BOX2SEQ_MAINSEQ_ITEMARRANGE_MAIN_CURMOVE_RET );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」ボックス切り替えボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangeBoxMoveButton( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
	return BoxMoveTrayScrollSet( syswk, 4 );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」手持ちいれかえボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemChgButton( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeSelectOff( syswk );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
	return PartyFrmMoveRight( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「もちものせいり」ボックス切り替えボタン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ItemArrangePartyCloseButton( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_PokeMenuOutSet( syswk->app->wfrm );
	BOX2BGWFRM_RetButtonInSet( syswk->app->wfrm );
	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
	}
	return VFuncSet( syswk, BOX2MAIN_VFuncItemArrangeMenuClose, BOX2SEQ_MAINSEQ_ITEMARRANGE_PARTY_MAIN_CURMOVE_RET );
}
