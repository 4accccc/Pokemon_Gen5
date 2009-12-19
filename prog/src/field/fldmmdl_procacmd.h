//======================================================================
/**
 *
 * @file	fldmmdl_acmd.h
 * @brief	ÉtÉBÅ[ÉãÉhìÆçÏÉÇÉfÉãÅ@ÉAÉjÉÅÅ[ÉVÉáÉìÉRÉ}ÉìÉhìÆçÏä÷êîíËã`
 * @author	kagaya
 * @date	05.07.13
 *
 */
//======================================================================
#ifndef MMDL_PROCACMD_H_FILE
#define MMDL_PROCACMD_H_FILE

//======================================================================
//	extern
//======================================================================
//--------------------------------------------------------------
//	AC_DIR_U,D,L,R
//--------------------------------------------------------------
extern int (* const DATA_AC_DirU_Tbl[])( MMDL * );
extern int (* const DATA_AC_DirD_Tbl[])( MMDL * );
extern int (* const DATA_AC_DirL_Tbl[])( MMDL * );
extern int (* const DATA_AC_DirR_Tbl[])( MMDL * );

//--------------------------------------------------------------
//	AC_WALKån
//--------------------------------------------------------------
extern int (* const DATA_AC_WalkU_32F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkD_32F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkL_32F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkR_32F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkU_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkD_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkL_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkR_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkU_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkD_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkL_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkR_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkU_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkD_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkL_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkR_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkU_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkD_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkL_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkR_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkU_1F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkD_1F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkL_1F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkR_1F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashU_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashD_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashL_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashR_4F_Tbl[])( MMDL * );

//--------------------------------------------------------------
//	AC_STAY_WALKån
//--------------------------------------------------------------
extern int (* const DATA_AC_StayWalkU_32F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkD_32F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkL_32F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkR_32F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkU_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkD_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkL_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkR_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkU_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkD_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkL_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkR_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkU_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkD_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkL_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkR_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkU_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkD_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkL_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayWalkR_2F_Tbl[])( MMDL * );

//--------------------------------------------------------------
//	AC_JUMPån	
//--------------------------------------------------------------
extern int (* const DATA_AC_StayJumpU_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayJumpD_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayJumpL_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayJumpR_16F_Tbl[])( MMDL * );
 
extern int (* const DATA_AC_StayJumpU_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayJumpD_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayJumpL_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_StayJumpR_8F_Tbl[])( MMDL * );

extern int (* const DATA_AC_JumpU_1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpD_1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpL_1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpR_1G_8F_Tbl[])( MMDL * );

extern int (* const DATA_AC_JumpU_2G_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpD_2G_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpL_2G_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpR_2G_16F_Tbl[])( MMDL * );

extern int (* const DATA_AC_JumpHiL_1G_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpHiR_1G_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpHiL_3G_32F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpHiR_3G_32F_Tbl[])( MMDL * );

extern int (* const DATA_AC_JumpU_3G_24F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpD_3G_24F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpL_3G_24F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpR_3G_24F_Tbl[])( MMDL * );

//--------------------------------------------------------------
//	AC_WAITån
//--------------------------------------------------------------
extern int (* const DATA_AC_Wait_1F_Tbl[])( MMDL * );
extern int (* const DATA_AC_Wait_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_Wait_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_Wait_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_Wait_15F_Tbl[])( MMDL * );
extern int (* const DATA_AC_Wait_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_Wait_32F_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_WARPån
//--------------------------------------------------------------
extern int (* const DATA_AC_WarpUp_Tbl[])( MMDL * );
extern int (* const DATA_AC_WarpDown_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_VANISH_ONån
//--------------------------------------------------------------
extern int (* const DATA_AC_VanishON_Tbl[])( MMDL * );
extern int (* const DATA_AC_VanishOFF_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_DIR_PAUSE_ONån
//--------------------------------------------------------------
extern int (* const DATA_AC_DirPauseON_Tbl[])( MMDL * );
extern int (* const DATA_AC_DirPauseOFF_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_ANM_PAUSE_ONån
//--------------------------------------------------------------
extern int (* const DATA_AC_AnmPauseON_Tbl[])( MMDL * );
extern int (* const DATA_AC_AnmPauseOFF_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_MARK_GYOEån
//--------------------------------------------------------------
extern int (* const DATA_AC_MarkGyoe_Tbl[])( MMDL * );
extern int (* const DATA_AC_MarkSaisen_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_WALK_6Fån
//--------------------------------------------------------------
extern int (* const DATA_AC_WalkU6F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkD6F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkL6F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkR6F_Tbl[])( MMDL * );

extern int (* const DATA_AC_WalkU3F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkD3F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkL3F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkR3F_Tbl[])( MMDL * );

extern int (* const DATA_AC_WalkU7F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkD7F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkL7F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkR7F_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_PC_BOW
//--------------------------------------------------------------
extern int (* const DATA_AC_PcBow_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_HIDE_PULLOFF
//--------------------------------------------------------------
extern int (* const DATA_AC_HidePullOFF_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_HERO_BANZAI
//--------------------------------------------------------------
extern int (* const DATA_AC_HeroBanzai_Tbl[])( MMDL * );
extern int (* const DATA_AC_HeroBanzaiUke_Tbl[])( MMDL * );
extern int (* const DATA_AC_HeroItemGet_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_WALKGLån
//--------------------------------------------------------------
extern int (* const DATA_AC_WalkGLU8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGLD8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGLL8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGLR8F_Tbl[])( MMDL * );

extern int (* const DATA_AC_WalkGRU8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGRD8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGRL8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGRR8F_Tbl[])( MMDL * );

extern int (* const DATA_AC_WalkGUU8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUD8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUL8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUR8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUU4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUD4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUL4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUR4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUU2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUD2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUL2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_WalkGUR2F_Tbl[])( MMDL * );

extern int (* const DATA_AC_DashGLU4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashGLD4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashGLL4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashGLR4F_Tbl[])( MMDL * );

extern int (* const DATA_AC_DashGRU4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashGRD4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashGRL4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashGRR4F_Tbl[])( MMDL * );

extern int (* const DATA_AC_DashGUU4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashGUD4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashGUL4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_DashGUR4F_Tbl[])( MMDL * );

extern int (* const DATA_AC_SandWalkU_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_SandWalkD_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_SandWalkL_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_SandWalkR_16F_Tbl[])( MMDL * );

//--------------------------------------------------------------
///	AC_JUMPGLån
//--------------------------------------------------------------
extern int (* const DATA_AC_JumpGLU1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpGLD1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpGLL1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpGLR1G_8F_Tbl[])( MMDL * );

extern int (* const DATA_AC_JumpGRU1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpGRD1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpGRL1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpGRR1G_8F_Tbl[])( MMDL * );

extern int (* const DATA_AC_JumpGUU1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpGUD1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpGUL1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_JumpGUR1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_MarkGyoeTWait_Tbl[])( MMDL * );


//--------------------------------------------------------------
///	AC_RAILån
//--------------------------------------------------------------
//DUMMY
extern int (* const DATA_AC_RailDummy[])( MMDL * );
// DIRån
extern int (* const DATA_AC_RailDirU_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailDirD_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailDirL_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailDirR_Tbl[])( MMDL * );

// WALKån
extern int (* const DATA_AC_RailWalkU_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkD_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkL_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkR_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkU_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkD_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkL_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkR_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkU_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkD_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkL_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkR_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkU_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkD_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkL_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailWalkR_2F_Tbl[])( MMDL * );

// DASHån
extern int (* const DATA_AC_RailDashU_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailDashD_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailDashL_4F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailDashR_4F_Tbl[])( MMDL * );

// STAY WALKån
extern int (* const DATA_AC_RailStayWalkU_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkD_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkL_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkR_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkU_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkD_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkL_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkR_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkU_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkD_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkL_2F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailStayWalkR_2F_Tbl[])( MMDL * );

// JUMPån
extern int (* const DATA_AC_RailJumpU_1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailJumpD_1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailJumpL_1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailJumpR_1G_8F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailJumpU_2G_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailJumpD_2G_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailJumpL_2G_16F_Tbl[])( MMDL * );
extern int (* const DATA_AC_RailJumpR_2G_16F_Tbl[])( MMDL * );

#endif //MMDL_PROCACMD_H_FILE
