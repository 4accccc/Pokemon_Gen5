//============================================================================================
/**
 * @file	fssc_tower_sub.h
 * @bfief	フロンティアシステムスクリプトコマンドサブ：タワー
 * @author	Satoshi Nohara
 * @date	07.05.28
 */
//============================================================================================
#ifndef	__FSSC_TOWER_SUB_H__
#define	__FSSC_TOWER_SUB_H__

//#include "factory_tool.h"


//============================================================================================
//
//	extern宣言
//
//============================================================================================
#if 0
extern u16	TowerScrTools_GetMemberNum(u16 mode);
extern void TowerScrTools_SystemReset(void);
extern void TowerScrTools_ClearPlaySaveData(BTLTOWER_PLAYWORK* playSave);
extern BOOL TowerScrTools_IsSaveDataEnable(BTLTOWER_PLAYWORK* playSave);
extern u16	TowerScrTools_GetRenshouRecord(SAVEDATA* sv,u16 play_mode);
extern u16	TowerScr_SetWifiRank(BTOWER_SCRWORK* wk,SAVEDATA* sv,u8 mode);
extern void TowerScrTools_SetWifiUploadFlag(SAVEDATA* sv,u8 flag);
extern u16 TowerScrTools_GetWifiUploadFlag(SAVEDATA* sv);
extern u16 TowerScrTools_SetNGScore(SAVEDATA* savedata);
extern u16	TowerScrTools_IsPlayerDataEnable(SAVEDATA* sv);
#else
extern u16	FSTowerScrTools_GetMemberNum(u16 mode);
extern void FSTowerScrTools_SystemReset(void);
extern void FSTowerScrTools_ClearPlaySaveData(BTLTOWER_PLAYWORK* playSave);
extern BOOL FSTowerScrTools_IsSaveDataEnable(BTLTOWER_PLAYWORK* playSave);
extern u16	FSTowerScrTools_GetRenshouRecord(SAVEDATA* sv,u16 play_mode);
extern void FSTowerScrTools_SetWifiUploadFlag(SAVEDATA* sv,u8 flag);
extern u16 FSTowerScrTools_GetWifiUploadFlag(SAVEDATA* sv);
extern u16 FSTowerScrTools_SetNGScore(SAVEDATA* savedata);
extern u16	FSTowerScrTools_IsPlayerDataEnable(SAVEDATA* sv);
//頭にToolsがついていない
extern u16	FSTowerScr_SetWifiRank(BTOWER_SCRWORK* wk,SAVEDATA* sv,u8 mode);
extern u16	FSTowerScr_IncRound(BTOWER_SCRWORK* wk);
extern BOOL FSTowerScr_IsClear(BTOWER_SCRWORK* wk);
extern u16	FSTowerScr_GetPlayMode(BTOWER_SCRWORK* wk);
extern void FSTowerScr_ChoiceBtlPartner(BTOWER_SCRWORK* wk,SAVEDATA* sv);
extern u16 FSTowerScr_GetEnemyObj(BTOWER_SCRWORK* wk,u16 idx);
extern void FSTowerScr_LocalBattleCall(GMEVENT_CONTROL* event,BTOWER_SCRWORK* wk,BOOL* win_flag);
//extern void FSTowerScr_SetLoseScore(BTOWER_SCRWORK* wk,FIELDSYS_WORK* fsys);
extern void FSTowerScr_SetLoseScore(BTOWER_SCRWORK* wk,SAVEDATA* savedata);
extern void FSTowerScr_SetClearScore(BTOWER_SCRWORK* wk,SAVEDATA* savedata,FNOTE_DATA* fnote);
extern void FSTowerScr_WorkRelease(BTOWER_SCRWORK* wk);
extern u16 FSTowerScr_GetRenshouCount(BTOWER_SCRWORK* wk);
extern u16	FSTowerScr_GetNowRound(BTOWER_SCRWORK* wk);
extern void FSTowerScr_SaveRestPlayData(BTOWER_SCRWORK* wk);
extern void FSTowerScr_SetLeaderClearFlag(BTOWER_SCRWORK* wk,u16 mode);
#endif


#endif	//__FSSC_TOWER_SUB_H__


