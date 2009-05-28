//============================================================================================
/**
 * @file	sound_manager.h
 * @brief	サウンド管理
 * @author	
 * @date	
 */
//============================================================================================
#ifndef __SND_MANAGER_H__
#define __SND_MANAGER_H__

//============================================================================================
/**
 *
 *
 * @brief	サウンド管理システム初期化
 *				以下の関数を使用する前に必ず呼び出す必要がある
 *
 */
//============================================================================================
extern void	SOUNDMAN_Init(NNSSndHeapHandle* pHeapHandle);





//============================================================================================
/**
 * @brief	階層構造プレーヤー関数
 */
//============================================================================================
// 階層プレーヤー設定構造体
typedef struct {
	u16	hierarchyPlayerSize;
	u16	hierarchyPlayerStartNo;
	u16	playerPlayableChannel;
}SOUNDMAN_HIERARCHY_PLAYER_DATA;

// スレッド使用引数構造体
typedef struct {
	u32	soundIdx;
	u8	volume;
	u8	dummy[3];
}SOUNDMAN_HIERARCHY_PLAYTHREAD_ARG;

//------------------------------------------------------------------
// 階層プレーヤー初期化
extern void	SOUNDMAN_InitHierarchyPlayer( const SOUNDMAN_HIERARCHY_PLAYER_DATA* playerData );

// 階層プレーヤー状態取得
extern u32				SOUNDMAN_GetHierarchyPlayerSoundIdx( void );
extern u32				SOUNDMAN_GetHierarchyPlayerPlayerNoIdx( void );
extern NNSSndHandle*	SOUNDMAN_GetHierarchyPlayerSndHandle( void );
extern int				SOUNDMAN_GetHierarchyPlayerSoundHeapLv( void );
// 階層プレーヤー状態設定
extern void				SOUNDMAN_UpdateHierarchyPlayerSoundHeapLv( void );

// 階層プレーヤー状態復元　※現在再生中のサウンドデータが読み込まれた状態
extern void	SOUNDMAN_RecoverHierarchyPlayerState( void );

// 階層プレーヤーサウンド操作
extern BOOL	SOUNDMAN_PlayHierarchyPlayer( u32 soundIdx );
extern void	SOUNDMAN_StopHierarchyPlayer( void );
extern void	SOUNDMAN_PauseHierarchyPlayer( BOOL flag );
extern BOOL	SOUNDMAN_PushHierarchyPlayer( void );
extern BOOL	SOUNDMAN_PopHierarchyPlayer( void );

// 階層プレーヤーサウンド操作(スレッド再生用)
extern void	SOUNDMAN_PlayHierarchyPlayer_forThread_heapsv( void );
extern BOOL	SOUNDMAN_PlayHierarchyPlayer_forThread_play( u32 soundIdx );

//============================================================================================
/**
 * @brief	サウンドプリセット関数
 */
//============================================================================================
// サウンドプリセット用ハンドル
typedef struct _SOUNDMAN_PRESET_HANDLE	SOUNDMAN_PRESET_HANDLE;

// プリセットおよびリリース
extern SOUNDMAN_PRESET_HANDLE*	SOUNDMAN_PresetSoundTbl( const u32* soundIdxTbl, u32 tblNum );
extern SOUNDMAN_PRESET_HANDLE*	SOUNDMAN_PresetGroup( u32 groupIdx );
extern void						SOUNDMAN_ReleasePresetData( SOUNDMAN_PRESET_HANDLE* handle );

#endif
