//======================================================================
/**
 *
 * @file	fldmmdl_procdraw.h
 * @brief	フィールド動作モデル　描画関数定義
 * @author	kagaya
 * @data	05.07.13
 *
 */
//======================================================================
#ifndef FLDMMDL_PROCDRAW_H_FILE
#define FLDMMDL_PROCDRAW_H_FILE

//--------------------------------------------------------------
//	fldmmdl_draw_0.c
//--------------------------------------------------------------
extern const FLDMMDL_DRAW_PROC_LIST DATA_FLDMMDL_DRAWPROCLIST_Hero;
extern const FLDMMDL_DRAW_PROC_LIST DATA_FLDMMDL_DRAWPROCLIST_BlAct;

//--------------------------------------------------------------
//	fieldobj_draw.c
//--------------------------------------------------------------
extern void FLDMMDL_DrawNon_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawNon_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawNon_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawNon_Push( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawNon_Pop( FLDMMDL * fmmdl );

//--------------------------------------------------------------
//	fieldobj_draw_0.c
//--------------------------------------------------------------
#if 0
extern BLACT_WORK_PTR FLDMMDL_DrawBlAct00_BlActPtrGet( FLDMMDL * fmmdl );
#endif

extern void FLDMMDL_DrawBlAct00_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlAct00_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlAct00AnmNon_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlAct00AnmOneP_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlAct00_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlAct00_Push( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlAct00_Pop( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawBlActPcWoman_Draw( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawBlActKoiking_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActKoiking_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActKoiking_Push( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActKoiking_Pop( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActKoiking_Draw( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawBlActBird_Draw( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawBlActHero_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHero_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHeroAnmNon_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHero_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHero_Push( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHero_Pop( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawBlActHeroCycle_Draw( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawBlActHeroSwim_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHeroSp_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHeroWater_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHeroFishing_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHeroPoketch_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActHeroBanzai_Draw( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawModel00_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawModel00_Draw( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawModel00_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawModel00_Push( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawModel00_Pop( FLDMMDL * fmmdl );

//extern void FLDMMDL_DrawModel00_Board_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawModel00_SnowBall_Init( FLDMMDL * fmmdl );
//extern void FLDMMDL_DrawModel00_Book_Init( FLDMMDL * fmmdl );
//extern void FLDMMDL_DrawModel00_Door2_Init( FLDMMDL * fmmdl );
//extern void FLDMMDL_DrawModel00_Board_Pop( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawModel00_SnowBall_Pop( FLDMMDL * fmmdl );
//extern void FLDMMDL_DrawModel00_Book_Pop( FLDMMDL * fmmdl );
//extern void FLDMMDL_DrawModel00_Door2_Pop( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawSnowBallBreakSet( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawBlActOff_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActOff_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActOff_Push( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActOff_Pop( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActOff_Draw( FLDMMDL * fmmdl );
extern u16 FLDMMDL_DrawBlActOff_RotateXGet( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActOff_RotateXSet( FLDMMDL * fmmdl, u16 rx );
extern u16 FLDMMDL_DrawBlActOff_RotateYGet( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActOff_RotateYSet( FLDMMDL * fmmdl, u16 rx );
extern u16 FLDMMDL_DrawBlActOff_RotateZGet( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActOff_RotateZSet( FLDMMDL * fmmdl, u16 rx );

extern void FLDMMDL_DrawBlActBldAgunomu_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActBldAgunomu_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActBldAgunomu_Push( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActBldAgunomu_Pop( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActAgunomu_Draw( FLDMMDL * fmmdl );

extern void FLDMMDL_DrawBlActTw7SpPoke_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActTw7SpPoke_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActTw7SpPoke_Push( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActTw7SpPoke_Pop( FLDMMDL * fmmdl );
extern void FLDMMDL_DrawBlActTw7SpPoke_Draw( FLDMMDL * fmmdl );

#endif	//FLDMMDL_PROCDRAW_H_FILE
