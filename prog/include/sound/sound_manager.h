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
// 階層プレーヤー初期化
extern void	SOUNDMAN_InitHierarchyPlayer( u16	hierarchyPlayerNo );

// 階層プレーヤー状態取得
extern u32				SOUNDMAN_GetHierarchyPlayerSoundIdx( void );
extern u32				SOUNDMAN_GetHierarchyPlayerPlayerNoIdx( void );
extern NNSSndHandle*	SOUNDMAN_GetHierarchyPlayerSndHandle( void );
extern int				SOUNDMAN_GetHierarchyPlayerSoundHeapLv( void );
extern u32				SOUNDMAN_GetHierarchyPlayerSoundIdxByPlayerID( int playerID );
// 階層プレーヤー状態設定
extern void				SOUNDMAN_UpdateHierarchyPlayerSoundHeapLv( void );

// 階層プレーヤー状態復元　※現在再生中のサウンドデータが読み込まれた状態
extern void	SOUNDMAN_RecoverHierarchyPlayerState( void );

// 階層プレーヤーサウンド操作
extern BOOL	SOUNDMAN_LoadHierarchyPlayer( u32 soundIdx );
extern void	SOUNDMAN_UnloadHierarchyPlayer( void );
extern BOOL	SOUNDMAN_PushHierarchyPlayer( void );
extern BOOL	SOUNDMAN_PopHierarchyPlayer( void );

// 階層プレーヤーサウンド操作(スレッド再生用)
extern void	SOUNDMAN_LoadHierarchyPlayer_forThread_heapsvSB( void );
extern BOOL	SOUNDMAN_LoadHierarchyPlayer_forThread_end( u32 soundIdx );

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
