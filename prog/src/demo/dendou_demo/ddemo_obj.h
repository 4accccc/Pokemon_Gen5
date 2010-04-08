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

extern void DDEMOOBJ_AddPoke( DDEMOMAIN_WORK * wk );

extern void DDEMOOBJ_PrintPokeInfo( DDEMOMAIN_WORK * wk );
extern void DDEMOOBJ_MoveFontOamPos( DDEMOMAIN_WORK * wk );
