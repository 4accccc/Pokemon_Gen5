//============================================================================================
/**
 * @file		dpc_obj.h
 * @brief		殿堂入り確認画面 ＯＢＪ関連
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DPCOBJ
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

#define	DPCOBJ_POKEMAX1_SPACE_RAD		( 0 )
#define	DPCOBJ_POKEMAX2_SPACE_RAD		( 180 )
#define	DPCOBJ_POKEMAX3_SPACE_RAD		( 120 )
#define	DPCOBJ_POKEMAX4_SPACE_RAD		( 90 )
#define	DPCOBJ_POKEMAX5_SPACE_RAD		( 72 )
#define	DPCOBJ_POKEMAX6_SPACE_RAD		( 60 )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================


extern void DPCOBJ_Init( DPCMAIN_WORK * wk );
extern void DPCOBJ_Exit( DPCMAIN_WORK * wk );
extern void DPCOBJ_AnmMain( DPCMAIN_WORK * wk );
extern void DPCOBJ_SetVanish( DPCMAIN_WORK * wk, u32 id, BOOL flg );
extern void DPCOBJ_SetAutoAnm( DPCMAIN_WORK * wk, u32 id, u32 anm );
extern BOOL DPCOBJ_CheckAnm( DPCMAIN_WORK * wk, u32 id );
extern u32 DPCOBJ_GetDefaultPoke( DPCMAIN_WORK * wk );
extern void DPCOBJ_SetPos( DPCMAIN_WORK * wk, u32 id, s16 x, s16 y );
extern void DPCOBJ_GetPos( DPCMAIN_WORK * wk, u32 id, s16 * x, s16 * y );

extern void DPCOBJ_AddPoke( DPCMAIN_WORK * wk );
extern void DPCOBJ_SetPokePos( DPCMAIN_WORK * wk, u32 id, u32 rad );
extern void DPCOBJ_ChangePokePriority( DPCMAIN_WORK * wk );

extern void DPCOBJ_InitFadeEvy( DPCMAIN_WORK * wk, BOOL flg );

extern void DPCOBJ_FontOamVanish( DPCMAIN_WORK * wk, BOOL flg );
