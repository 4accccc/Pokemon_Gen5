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
//電気
extern VMCMD_RESULT EvCmdGymElec_Init( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_Recover( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_SetSpotLight( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_CallFadeoutEvent( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_CallShowupEvent( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGymElec_SetEffect( VMHANDLE *core, void *wk );

//以下SWAN未作成の箇所はコメントアウトしています。
//作成する際に、復活or削除してください。10/12/24 nagihashi

//ノーマル
//extern VMCMD_RESULT EvCmdGymNormal_Init( VMHANDLE *core, void *wk );
//extern VMCMD_RESULT EvCmdGymNormal_( VMHANDLE *core, void *wk );

//アンチ
//extern VMCMD_RESULT EvCmdGymAnti_Init( VMHANDLE *core, void *wk );
//extern VMCMD_RESULT EvCmdGymAnti_( VMHANDLE *core, void *wk );

//虫
extern VMCMD_RESULT EvCmdGymInsect_CocconOpen( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGymInsect_VacuumAnm( VMHANDLE * core, void *wk );

//地面
//extern VMCMD_RESULT EvCmdGymGround_( VMHANDLE *core, void *wk );

//飛行
extern VMCMD_RESULT EvCmdGymFly_Blow( VMHANDLE * core, void *wk );

//氷
//extern VMCMD_RESULT EvCmdGymIce_( VMHANDLE *core, void *wk );

//龍
//extern VMCMD_RESULT EvCmdGymDragon_Init( VMHANDLE *core, void *wk );
//extern VMCMD_RESULT EvCmdGymDragon_( VMHANDLE *core, void *wk );
