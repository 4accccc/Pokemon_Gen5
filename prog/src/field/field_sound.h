///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief フィールドのサウンド制御 ( WB特化機能 )
 * @file   field_sound.h
 * @author obata
 * @date   2010.01.15
 */
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gflib.h"
#include "gamesystem/gamesystem.h"  // for GAMESYS_WORK
#include "gamesystem/gamedata_def.h"  // for GAMEDATA
#include "gamesystem/game_event.h"  // for GMEVENT
#include "field_sound_proc.h"


//=================================================================================
// ■定数
//================================================================================= 
// フェード フレーム数
#define FSND_FADE_NONE   (0)   // 無
#define FSND_FADE_SHORT  (9)   // 短 
#define FSND_FADE_NORMAL (30)  // 中
#define FSND_FADE_LONG   (45)  // 長


//=================================================================================
// ■BGMの操作
//=================================================================================
extern GMEVENT* EVENT_FSND_ForcePlayBGM( GAMESYS_WORK* gameSystem, u16 soundIdx );
extern GMEVENT* EVENT_FSND_FadeInBGM( GAMESYS_WORK* gameSystem, u16 fadeInFrame );
extern GMEVENT* EVENT_FSND_FadeOutBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame );
extern GMEVENT* EVENT_FSND_PushBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame );
extern GMEVENT* EVENT_FSND_PopBGM( GAMESYS_WORK* gameSystem, 
                                   u16 fadeOutFrame, u16 fadeInFrame );
extern GMEVENT* EVENT_FSND_AllPopBGM( GAMESYS_WORK* gameSystem, u16 fadeInFrame );
extern GMEVENT* EVENT_FSND_PushPlayNextBGM( GAMESYS_WORK* gameSystem, u32 soundIdx, 
                                            u16 fadeOutFrame, u16 fadeInFrame );
extern GMEVENT* EVENT_FSND_PushPlayJingleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx );
extern GMEVENT* EVENT_FSND_ChangeBGM( GAMESYS_WORK* gameSystem, u32 soundIdx, 
                                      u16 fadeOutFrame, u16 fadeInFrame );

// ベースBGMを退避し, 戦闘曲を再生する
extern GMEVENT* EVENT_FSND_PushPlayBattleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx );

// フェード操作の完了待ち
extern GMEVENT* EVENT_FSND_WaitBGMFade( GAMESYS_WORK* gameSystem );


//=================================================================================
// ■プレイヤーの操作により発生するBGMの変更
//=================================================================================
// ゾーン チェンジ
extern void FSND_ChangeBGM_byZoneChange( FIELD_SOUND* fieldSound, GAMEDATA* gameData, 
                                         u16 prevZoneID, u16 nextZoneID ); 

// マップ チェンジ
extern void FSND_StandByNextMapBGM( FIELD_SOUND* fieldSound, GAMEDATA* gameData, 
                                    u16 prevZoneID, u16 nextZoneID );

extern void FSND_PlayStartBGM( FIELD_SOUND* fieldSound );

// 自機移動フォーム変更
extern void FSND_ChangeBGM_byPlayerFormChange( FIELD_SOUND* fieldSound, 
                                               GAMEDATA* gameData, u16 zoneID ); 


//=================================================================================
// ■BGM No.の取得
//=================================================================================
// 指定したフィールド上で再生すべき BGM No.
extern u32 FSND_GetFieldBGM( GAMEDATA* gdata, u32 zoneID ); 

// ゾーンチェンジ時に再生すべき BGM No.
extern u32 FSND_GetZoneChangeBGM( GAMEDATA* gdata, u32 prevZoneID, u32 nextZoneID );

// マップチェンジ時に再生すべき BGM No.
extern u32 FSND_GetMapChangeBGM( GAMEDATA* gdata, u32 prevZoneID, u32 nextZoneID );

// トレーナー視線 BGM No.
extern u32 FSND_GetTrainerEyeBGM( u32 trType ); 
