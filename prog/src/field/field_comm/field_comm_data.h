//======================================================================
/**
 * @file	field_comm_data.h
 * @brief	通信で共有するデータ(各キャラの座標とか)
 * @author	ariizumi
 * @data	08/11/13
 */
//======================================================================

#ifndef FIELD_COMM_DATA_H__
#define FIELD_COMM_DATA_H__

#include "gamesystem/playerwork.h"
#include "field_comm_def.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
//方向定義
//field_actor.cのACTWALK_に合わせる事
typedef enum
{
	
//	COMMDIR_UP		= 0x0000,
//	COMMDIR_LEFT	= 0x4000,
//	COMMDIR_DOWN	= 0x8000,
//	COMMDIR_RIGHT	= 0xc000,
	COMMDIR_UP		= 0,
	COMMDIR_DOWN	= 1,
	COMMDIR_LEFT	= 2,
	COMMDIR_RIGHT	= 3,
}FIELD_COMM_ACTER_DIR;

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================
extern void FIELD_COMM_DATA_InitSystem( HEAPID heapID );
extern void	FIELD_COMM_DATA_TermSystem();
extern BOOL	FIELD_COMM_DATA_IsExistSystem(void);
extern void	FIELD_COMM_DATA_SetFieldCommMode( const u8 mode );
extern const u8 FIELD_COMM_DATA_GetFieldCommMode(void);

extern void	FIELD_COMM_DATA_SetSelfData_Pos( const ZONEID *zoneID , const VecFx32 *pos , const u16 *dir );
extern PLAYER_WORK*	FIELD_COMM_DATA_GetSelfData_PlayerWork(void);

extern void	FIELD_COMM_DATA_SetCharaData_IsExist( const u8 idx , const BOOL );
extern void	FIELD_COMM_DATA_SetCharaData_IsValid( const u8 idx , const BOOL );
extern const BOOL	FIELD_COMM_DATA_GetCharaData_IsExist( const u8 idx );
extern const BOOL	FIELD_COMM_DATA_GetCharaData_IsValid( const u8 idx );
extern void	FIELD_COMM_DATA_SetTalkState( const u8 idx , const F_COMM_TALK_STATE state );
extern const F_COMM_TALK_STATE FIELD_COMM_DATA_GetTalkState( const u8 idx );
extern const BOOL	FIELD_COMM_DATA_GetGridPos_AfterMove( const u8 idx , int *posX , int *posZ );

extern const F_COMM_CHARA_STATE FIELD_COMM_DATA_GetCharaData_State( const u8 idx );
extern void FIELD_COMM_DATA_SetCharaData_State( const u8 idx , const F_COMM_CHARA_STATE state );
extern PLAYER_WORK*	FIELD_COMM_DATA_GetCharaData_PlayerWork( const u8 idx );

//ユーザーデータ関連
extern void	FIELD_COMM_DATA_CreateUserData( const F_COMM_USERDATA_TYPE type );
extern void	FIELD_COMM_DATA_DeleteUserData(void);
extern const u32 FIELD_COMM_DATA_GetUserDataSize( const F_COMM_USERDATA_TYPE type );
extern const F_COMM_USERDATA_TYPE	FIELD_COMM_DATA_GetUserDataType_From_Action( const F_COMM_ACTION_LIST action );
extern void*	FIELD_COMM_DATA_GetSelfUserData( const F_COMM_USERDATA_TYPE type );
extern void*	FIELD_COMM_DATA_GetPartnerUserData( const F_COMM_USERDATA_TYPE type );
extern F_COMM_USERDATA_TYPE FIELD_COMM_DATA_GetUserDataType( void );




#endif //FIELD_COMM_DATA_H__

