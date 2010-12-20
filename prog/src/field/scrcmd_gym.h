//======================================================================
/**
 * @file  scrcmd_gym.h
 * @brief  スクリプトコマンド：ジム関連
 * @author  Saito
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdGymElec_Init( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_Recover( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_SetSpotLight( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_CallFadeoutEvent( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_CallShowupEvent( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_SetFanEffect( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGymNormal_Init( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymNormal_Unrock( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymNormal_CheckUnrock( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymNormal_MoveWall( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGymAnti_Init( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymAnti_SwOn( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymAnti_SwOff( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymAnti_OpenDoor( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGymInsect_Init( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymInsect_SwOn( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymInsect_PoleOn( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymInsect_TrTrapOn( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGymGround_MvLift( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymGround_ExitLift( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymGroundEnt_ExitLift( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymGround_SetEnterPos( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymGroundEnt_SetEnterPos( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGymIce_SwAnm( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymIce_WallAnm( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymIce_ChgRail( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGymDragon_Init( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymDragon_CallGmk( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymDragon_JumpDown( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymDragon_MoveArm( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymDragon_MoveHead( VMHANDLE *core, void *wk );
