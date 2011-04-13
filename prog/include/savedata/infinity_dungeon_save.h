//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		infinity_dungeon_save.h
//   @brief   無限ダンジョンセーブデータ
//   @author	Masayuki Onoue
//   @date		2011.02.24
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
//
//					定数宣言
//
//=============================================================================

//=============================================================================
//  typedef
//=============================================================================
typedef struct _INFINITYDUNGEON_SAVE INFINITYDUNGEON_SAVE;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern int INFINITYDUNGEON_SAVE_GetWorkSize(void);
extern void INFINITYDUNGEON_SAVE_Init(INFINITYDUNGEON_SAVE *p_save);
extern INFINITYDUNGEON_SAVE* SaveData_GetINFINITYDUNGEON( SAVE_CONTROL_WORK * p_save );
extern u16 INFINITY_DUNGEON_SAVE_GetTrNo_ByObjId( u16 obj_id );
extern u8 INFINITY_DUNGEON_SAVE_GetHintLevel( const INFINITYDUNGEON_SAVE *p_save );

// ダンジョン情報（取得系）
extern u8 INFINITYDUNGEON_SAVE_GetDungeonStatus( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetExDungeonStatus( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetExMissionCount( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetMementoFlag( const INFINITYDUNGEON_SAVE *p_save );

// MAP情報（取得系）
extern BOOL INFINITYDUNGEON_SAVE_GetTrainerFlag( const INFINITYDUNGEON_SAVE *p_save ,int floor_id ,int trainer_id_pos );
extern u32 INFINITYDUNGEON_SAVE_GetRandomSeed( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetMapStatus( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetRoomID( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetPartnerID( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetFloorID( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetRoomID( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetPartnerID( const INFINITYDUNGEON_SAVE *p_save );
extern u8 INFINITYDUNGEON_SAVE_GetDungeonAreaID( const INFINITYDUNGEON_SAVE *p_save );

// ダンジョン情報（設定系）
extern void INFINITYDUNGEON_SAVE_SetDungeonStatus( INFINITYDUNGEON_SAVE *p_save ,u8 dungeon_status);
extern void INFINITYDUNGEON_SAVE_SetExDungeonStatus( INFINITYDUNGEON_SAVE *p_save ,u8 ex_dungeon_status);
extern void INFINITYDUNGEON_SAVE_AddExMissionCount( INFINITYDUNGEON_SAVE *p_save );
extern void INFINITYDUNGEON_SAVE_SetMementoFlag( INFINITYDUNGEON_SAVE *p_save ,u8 memento_status);

// MAP情報（設定系）
extern void INFINITYDUNGEON_SAVE_SetRandomSeed( INFINITYDUNGEON_SAVE *p_save ,u32 rand_seed);
extern void INFINITYDUNGEON_SAVE_SetTrainerFlag( INFINITYDUNGEON_SAVE *p_save ,int floor_id ,int trainer_id_pos , BOOL status);
extern void INFINITYDUNGEON_SAVE_SetMapStatus( INFINITYDUNGEON_SAVE *p_save ,u8 map_status);
extern void INFINITYDUNGEON_SAVE_SetFloorID( INFINITYDUNGEON_SAVE *p_save ,u8 floor_id);
extern void INFINITYDUNGEON_SAVE_SetRoomID( INFINITYDUNGEON_SAVE *p_save ,u8 room_id);
extern void INFINITYDUNGEON_SAVE_SetPartnerID( INFINITYDUNGEON_SAVE *p_save ,u8 partner_id);





