//======================================================================
/**
 *
 * @file	fldmmdl_procmove.h
 * @brief	フィールド動作モデル　動作関数定義
 * @author	kagaya
 * @date	05.07.13
 *
 */
//======================================================================
#ifndef MMDL_PROCMOVE_H_FILE
#define MMDL_PROCMOVE_H_FILE

//--------------------------------------------------------------
//	fieldobj_move_0.c
//--------------------------------------------------------------
extern void MMDL_MoveDirRnd_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndUL_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndUR_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndDL_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndDR_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndUDL_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndUDR_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndULR_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndDLR_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndUD_Init( MMDL * fmmdl );
extern void MMDL_MoveDirRndLR_Init( MMDL * fmmdl );

extern void MMDL_MoveDirRnd_Move( MMDL * fmmdl );
extern void MMDL_MoveDirRnd_Delete( MMDL * fmmdl );

extern void MMDL_MoveRnd_Init( MMDL * fmmdl );
extern void MMDL_MoveRndV_Init( MMDL * fmmdl );
extern void MMDL_MoveRndH_Init( MMDL * fmmdl );
extern void MMDL_MoveRndHLim_Init( MMDL * fmmdl );
extern void MMDL_MoveRndRect_Init( MMDL * fmmdl );

extern void MMDL_MvRnd_Move( MMDL * fmmdl );

extern void MMDL_MoveDir_Move( MMDL * fmmdl );

extern void MMDL_MoveUp_Init( MMDL * fmmdl );
extern void MMDL_MoveDown_Init( MMDL * fmmdl );
extern void MMDL_MoveLeft_Init( MMDL * fmmdl );
extern void MMDL_MoveRight_Init( MMDL * fmmdl );

extern void MMDL_MoveSpin_Move( MMDL * fmmdl );
extern void MMDL_MoveSpinLeft_Init( MMDL * fmmdl );
extern void MMDL_MoveSpinRight_Init( MMDL * fmmdl );

extern void MMDL_MoveRewar_Init( MMDL * fmmdl );
extern void MMDL_MoveRewar_Move( MMDL * fmmdl );

extern void MMDL_MoveRoute2_Init( MMDL * fmmdl );
extern void MMDL_MoveRoute2_Move( MMDL * fmmdl );

extern void MMDL_MoveRouteURLD_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteRLDU_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteDURL_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteLDUR_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteULRD_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteLRDU_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteDULR_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteRDUL_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteLUDR_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteUDRL_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteRLUD_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteDRLU_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteRUDL_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteUDLR_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteLRUD_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteDLRU_Init( MMDL * fmmdl );
extern void MMDL_MoveRoute3_Move( MMDL * fmmdl );

extern void MMDL_MoveRouteUL_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteDR_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteLD_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteRU_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteUR_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteDL_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteLU_Init( MMDL * fmmdl );
extern void MMDL_MoveRouteRD_Init( MMDL * fmmdl );
extern void MMDL_MoveRoute4_Move( MMDL * fmmdl );

//--------------------------------------------------------------
//	fieldobj_move_2.c
//--------------------------------------------------------------
extern void MMDL_MovePair_Init( MMDL * fmmdl );
extern void MMDL_MovePair_Move( MMDL * fmmdl );
extern void MMDL_MovePair_Delete( MMDL * fmmdl );

extern void MMDL_MovePairTr_Init( MMDL * fmmdl );
extern void MMDL_MovePairTr_Move( MMDL * fmmdl );
extern void MMDL_MovePairTr_Delete( MMDL * fmmdl );
extern void MMDL_MovePairTr_Return( MMDL * fmmdl );
extern MMDL * MMDL_MovePairSearch( MMDL * fmmdl );

extern void MMDL_MoveHideSnow_Init( MMDL * fmmdl );
extern void MMDL_MoveHideSand_Init( MMDL * fmmdl );
extern void MMDL_MoveHideGround_Init( MMDL * fmmdl );
extern void MMDL_MoveHideKusa_Init( MMDL * fmmdl );
extern void MMDL_MoveHide_Move( MMDL * fmmdl );
extern void MMDL_MoveHide_Delete( MMDL * fmmdl );
extern void MMDL_MoveHide_Return( MMDL * fmmdl );

extern void MMDL_MoveCopyU_Init( MMDL * fmmdl );
extern void MMDL_MoveCopyD_Init( MMDL * fmmdl );
extern void MMDL_MoveCopyL_Init( MMDL * fmmdl );
extern void MMDL_MoveCopyR_Init( MMDL * fmmdl );
extern void MMDL_MoveCopyLGrassU_Init( MMDL * fmmdl );
extern void MMDL_MoveCopyLGrassD_Init( MMDL * fmmdl );
extern void MMDL_MoveCopyLGrassL_Init( MMDL * fmmdl );
extern void MMDL_MoveCopyLGrassR_Init( MMDL * fmmdl );
extern void MMDL_MoveCopy_Move( MMDL * fmmdl );

extern void MMDL_AlongWallL_Init( MMDL * fmmdl );
extern void MMDL_AlongWallR_Init( MMDL * fmmdl );
extern void MMDL_AlongWallLRL_Init( MMDL * fmmdl );
extern void MMDL_AlongWallLRR_Init( MMDL * fmmdl );

extern void MMDL_AlongWall_Move( MMDL * fmmdl );


//--------------------------------------------------------------
//	fieldobj_railmove_0.c
//--------------------------------------------------------------
extern void MMDL_RailDmy_Init( MMDL * fmmdl );
extern void MMDL_RailDmy_Move( MMDL * fmmdl );
extern void MMDL_RailDmy_Delete( MMDL * fmmdl );
extern void MMDL_RailDmy_Return( MMDL * fmmdl );

// ランダム振り向き
extern void MMDL_RailDirRnd_ALL_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_UL_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_UR_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_DL_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_DR_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_UDL_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_UDR_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_ULR_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_DLR_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_UD_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_LR_Init( MMDL * fmmdl );
extern void MMDL_RailDirRnd_Move( MMDL * fmmdl );
extern void MMDL_RailDirRnd_Delete( MMDL * fmmdl );

// ランダム移動
extern void MMDL_RailRnd_ALL_Init( MMDL * fmmdl );
extern void MMDL_RailRnd_V_Init( MMDL * fmmdl );
extern void MMDL_RailRnd_H_Init( MMDL * fmmdl );
extern void MMDL_RailRnd_Move( MMDL * fmmdl );
extern void MMDL_RailRnd_Delete( MMDL * fmmdl );

// ルート移動
extern void MMDL_RailRoot2_Init( MMDL * fmmdl );
extern void MMDL_RailRoot2_Move( MMDL * fmmdl );
extern void MMDL_RailRoot2_Delete( MMDL * fmmdl );

#endif //MMDL_PROCMOVE_H_FILE
