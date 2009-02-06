//======================================================================
/**
 *
 * @file	fldmmdl_procmove.h
 * @brief	フィールド動作モデル　動作関数定義
 * @author	kagaya
 * @data	05.07.13
 *
 */
//======================================================================
#ifndef FLDMMDL_PROCMOVE_H_FILE
#define FLDMMDL_PROCMOVE_H_FILE

//--------------------------------------------------------------
//	fieldobj_move_0.c
//--------------------------------------------------------------
extern void FLDMMDL_MoveDirRnd_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndUL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndUR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndDL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndDR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndUDL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndUDR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndULR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndDLR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndUD_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRndLR_Init( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveDirRnd_Move( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDirRnd_Delete( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveRnd_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRndV_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRndH_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRndHLim_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRndRect_Init( FLDMMDL * fmmdl );

extern void FLDMMDL_MvRnd_Move( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveDir_Move( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveUp_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveDown_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveLeft_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRight_Init( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveSpin_Move( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveSpinLeft_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveSpinRight_Init( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveRewar_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRewar_Move( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveRoute2_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRoute2_Move( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveRouteURLD_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteRLDU_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteDURL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteLDUR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteULRD_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteLRDU_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteDULR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteRDUL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteLUDR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteUDRL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteRLUD_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteDRLU_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteRUDL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteUDLR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteLRUD_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteDLRU_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRoute3_Move( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveRouteUL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteDR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteLD_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteRU_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteUR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteDL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteLU_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRouteRD_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveRoute4_Move( FLDMMDL * fmmdl );

//--------------------------------------------------------------
//	fieldobj_move_2.c
//--------------------------------------------------------------
extern void FLDMMDL_MovePair_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MovePair_Move( FLDMMDL * fmmdl );
extern void FLDMMDL_MovePair_Delete( FLDMMDL * fmmdl );

extern void FLDMMDL_MovePairTr_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MovePairTr_Move( FLDMMDL * fmmdl );
extern void FLDMMDL_MovePairTr_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_MovePairTr_Return( FLDMMDL * fmmdl );
extern FLDMMDL * FLDMMDL_MovePairSearch( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveHideSnow_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveHideSand_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveHideGround_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveHideKusa_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveHide_Move( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveHide_Delete( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveHide_Return( FLDMMDL * fmmdl );

extern void FLDMMDL_MoveCopyU_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveCopyD_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveCopyL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveCopyR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveCopyLGrassU_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveCopyLGrassD_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveCopyLGrassL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveCopyLGrassR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_MoveCopy_Move( FLDMMDL * fmmdl );

extern void FLDMMDL_AlongWallL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_AlongWallR_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_AlongWallLRL_Init( FLDMMDL * fmmdl );
extern void FLDMMDL_AlongWallLRR_Init( FLDMMDL * fmmdl );

extern void FLDMMDL_AlongWall_Move( FLDMMDL * fmmdl );

#endif //FLDMMDL_PROCMOVE_H_FILE
