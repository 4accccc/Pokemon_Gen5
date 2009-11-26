//============================================================================================
/**
 * @file		box2_bgwfrm.h
 * @brief		ボックス画面 BGウィンドウフレーム関連
 * @author	Hiroyuki Nakamura
 * @date		09.10.07
 *
 *	モジュール名：BOX2BGWFRM
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

// 手持ちポケモンフレームデータ
#define	BOX2BGWFRM_PARTYPOKE_LX	( 2 )
//#define	WINFRM_PARTYPOKE_RX	( 21 )
#define	BOX2BGWFRM_PARTYPOKE_PY	( 6 )
//#define	WINFRM_PARTYPOKE_INIT_PY	( 24 )
//#define	WINFRM_PARTYPOKE_RET_PX		( 24 )
//#define	WINFRM_PARTYPOKE_RET_PY		( 15 )
#define	BOX2BGWFRM_PARTYPOKE_SX	( 11 )		// 手持ちポケモンフレームＸサイズ
#define	BOX2BGWFRM_PARTYPOKE_SY	( 15 )		// 手持ちポケモンフレームＹサイズ


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧフレームワーク作成（全体）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BGWFRM_Init( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧフレームワーク解放
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BGWFRM_Exit( BOX2_APP_WORK * appwk );


//--------------------------------------------------------------------------------------------
/**
 * @brief		「てもちポケモン」ボタン配置
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BGWFRM_TemochiButtonOn( BOX2_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンいどう」ボタン配置
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BGWFRM_BoxListButtonOn( BOX2_APP_WORK * appwk );



extern void BOX2BGWFRM_SubDispWazaFrmOutPosSet( BGWINFRM_WORK * wk );

extern BOOL BOX2BGWFRM_PokeMenuMoveMain( BGWINFRM_WORK * wk );

extern BOOL BOX2BGWFRM_PartyPokeFrameMove( BOX2_SYS_WORK * syswk );

extern void BOX2BGWFRM_PartyPokeFrameOnlyInSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_PartyPokeFrameOnlyOutSet( BGWINFRM_WORK * wk );

extern BOOL BOX2BGWFRM_AzukeruPartyPokeFrameMove( BOX2_SYS_WORK * syswk );

extern BOOL BOX2BGWFRM_CheckPartyPokeFrameRight( BGWINFRM_WORK * wk );

extern BOOL BOX2BGWFRM_BoxMoveButtonCheck( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_BoxMoveButtonOutSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_PokeMenuOutSet( BGWINFRM_WORK * wk );

extern BOOL BOX2BGWFRM_PokeMenuPutCheck( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_SubDispItemFrmOutSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_BoxMoveButtonInSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_BoxMoveFrmOutSet( BGWINFRM_WORK * wk );
extern BOOL BOX2BGWFRM_CheckBoxMoveFrm( BGWINFRM_WORK * wk );



extern void BOX2BGWFRM_MarkingFrameOutSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_PartyPokeFrameInSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_PartyPokeFrameOutSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_PartyPokeFrameRightMoveSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_PokeMenuInSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_PokeMenuOff( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_SubDispItemFrmInSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_AzukeruPartyPokeFrameLeftMoveSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_AzukeruPartyPokeFrameRightMoveSet( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_BoxMoveMenuInSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_BoxMoveMenuOutSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_PartyPokeFrameInitPutRight( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_BoxMoveFrmInSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_BoxThemaMenuOutSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_SubDispItemFrmOutPosSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_BoxThemaMenuInSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_SubDispWazaFrmInSet( BOX2_APP_WORK * appwk );
extern void BOX2BGWFRM_PartyPokeFrameLeftMoveSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_SubDispWazaFrmOutSet( BOX2_APP_WORK * appwk );
extern BOOL BOX2BGWFRM_YStatusButtonCheck( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_YStatusButtonInSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_YStatusButtonOutSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_ArrangeUnderButtonDel( BOX2_SYS_WORK * syswk );
extern void BOX2BGWFRM_BoxMoveFrmPut( BOX2_SYS_WORK * syswk );
extern void BOX2BGWFRM_BoxThemaFrmPut( BOX2_SYS_WORK * syswk );
extern void BOX2BGWFRM_MarkingFrameInSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_MarkingFramePut( BOX2_SYS_WORK * syswk );
extern void BOX2BGWFRM_BoxListButtonOff( BOX2_APP_WORK * appwk );
extern void BOX2BGWFRM_TemochiButtonOff( BOX2_APP_WORK * appwk );
extern void BOX2BGWFRM_MarkingSwitch( BOX2_SYS_WORK * syswk, u32 pos );
extern void BOX2BGWFRM_PartyPokeFrameInitPutLeft( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_PartyPokeFrameOff( BOX2_SYS_WORK * syswk );
extern void BOX2BGWFRM_PartyPokeFramePut( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_PartyPokeFramePutRight( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_PokeMenuOpenPosSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_SubDispItemFrmInPosSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_SubDispWazaFrmInPosSet( BOX2_APP_WORK * appwk );
extern void BOX2BGWFRM_WallPaperFrameInSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_WallPaperFrameOutSet( BGWINFRM_WORK * wk );
extern void BOX2BGWFRM_YStatusButtonPut( BGWINFRM_WORK * wk );

extern void BOX2BGWFRM_SubDispWinFrmMove( BOX2_SYS_WORK * syswk );

extern void BOX2BGWFRM_PutTouchBar( BGWINFRM_WORK * wk );
