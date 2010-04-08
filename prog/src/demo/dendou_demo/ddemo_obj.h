//============================================================================================
/**
 * @file		ddemo_obj.h
 * @brief		殿堂入りデモ画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.06
 *
 *	モジュール名：DDEMOOBJ
 */
//============================================================================================
#pragma	once



extern void DDEMOOBJ_Init( DDEMOMAIN_WORK * wk );
extern void DDEMOOBJ_Exit( DDEMOMAIN_WORK * wk );
extern void DDEMOOBJ_AnmMain( DDEMOMAIN_WORK * wk );

extern void DDEMOOBJ_SetPos( DDEMOMAIN_WORK * wk, u32 id, s16 x, s16 y );
extern void DDEMOOBJ_GetPos( DDEMOMAIN_WORK * wk, u32 id, s16 * x, s16 * y );
extern void DDEMOOBJ_Move( DDEMOMAIN_WORK * wk, u32 id, s16 mx, s16 my );
extern void DDEMOOBJ_SetVanish( DDEMOMAIN_WORK * wk, u32 id, BOOL flg );
extern void DDEMOOBJ_SetAutoAnm( DDEMOMAIN_WORK * wk, u32 id, u32 anm );
extern BOOL DDEMOOBJ_CheckAnm( DDEMOMAIN_WORK * wk, u32 id );

extern void DDEMOOBJ_AddPoke( DDEMOMAIN_WORK * wk );

extern void DDEMOOBJ_PrintPokeInfo( DDEMOMAIN_WORK * wk );
extern void DDEMOOBJ_MoveFontOamPos( DDEMOMAIN_WORK * wk );
extern void BOX2OBJ_FontOamVanish( DDEMOMAIN_WORK * wk, u32 idx, BOOL flg );

extern void DDEMOOBJ_Init2ndScene( DDEMOMAIN_WORK * wk );
