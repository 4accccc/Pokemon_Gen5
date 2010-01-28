/////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  プレイヤーボリュームのフェード管理
 * @file   player_volume_fader.h
 * @author obata
 * @date   2010.01.27
 */
/////////////////////////////////////////////////////////////////////////////////////
#pragma once


// システムの不完全型宣言
typedef struct _PLAYER_VOLUME_FADER PLAYER_VOLUME_FADER;


// システム生成/破棄
extern PLAYER_VOLUME_FADER* PLAYER_VOLUME_FADER_Create( HEAPID heapID, u8 playerNo );
extern void                 PLAYER_VOLUME_FADER_Delete( PLAYER_VOLUME_FADER* fader );

// システム動作
extern void PLAYER_VOLUME_FADER_Main( PLAYER_VOLUME_FADER* fader );

// ボリューム変更
extern void PLAYER_VOLUME_FADER_SetVolume( PLAYER_VOLUME_FADER* fader, u8 volume, u16 frames );
