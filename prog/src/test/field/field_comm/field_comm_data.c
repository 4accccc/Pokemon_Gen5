//======================================================================
/**
 * @file	field_comm_data.c
 * @brief	通信で共有するデータ(各キャラの座標とか)
 * @author	ariizumi
 * @data	08/11/13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "gamesystem/playerwork.h"
#include "test/ariizumi/ari_debug.h"
#include "field_comm_main.h"
#include "field_comm_data.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
//キャラ個人のデータ
typedef struct
{
	BOOL	isExist_;	//キャラが存在する
	BOOL	isValid_;	//データが有効
	F_COMM_CHARA_STATE	state_;
	F_COMM_TALK_STATE	talkState_;
	PLAYER_WORK plWork_;
}FIELD_COMM_CHARA_DATA;

//会話後の行動用のデータ管理
typedef struct
{
	F_COMM_USERDATA_TYPE type_;
	void*	selfData_;		//自身のデータ(送信バッファ
	void*	partnerData_;	//相手のデータ(受信バッファ
}FIELD_COMM_USERDATA;

typedef struct 
{
	HEAPID heapID_;
	u8	backupCommState_;	//FieldCommFuncのステートのバックアップ
	FIELD_COMM_CHARA_DATA selfData_;	//自身のデータ。
	FIELD_COMM_CHARA_DATA charaData_[FIELD_COMM_MEMBER_MAX];

	FIELD_COMM_USERDATA userData_;
}FIELD_COMM_DATA;

//あえて独立して確保させる
FIELD_COMM_DATA *commData = NULL;

//======================================================================
//	proto
//======================================================================
void	FIELD_COMM_DATA_InitSystem( HEAPID heapID );
void	FIELD_COMM_DATA_TermSystem(void);
BOOL	FIELD_COMM_DATA_IsExistSystem(void);

void	FIELD_COMM_DATA_SetFieldCommMode( const u8 mode );
const u8 FIELD_COMM_DATA_GetFieldCommMode(void);

void	FIELD_COMM_DATA_SetSelfData_PlayerWork( const PLAYER_WORK *plWork );
void	FIELD_COMM_DATA_SetSelfDataPos( const ZONEID zoneID , const VecFx32 *pos , const u16 *dir );
PLAYER_WORK* FIELD_COMM_DATA_GetSelfData_PlayerWork(void);

void	FIELD_COMM_DATA_SetCharaData_IsExist( const u8 idx , const BOOL );
const BOOL	FIELD_COMM_DATA_GetCharaData_IsExist( const u8 idx );
void	FIELD_COMM_DATA_SetCharaData_IsValid( const u8 idx , const BOOL );
const BOOL	FIELD_COMM_DATA_GetCharaData_IsValid( const u8 idx );
void	FIELD_COMM_DATA_SetTalkState( const u8 idx , const F_COMM_TALK_STATE state );
const	F_COMM_TALK_STATE FIELD_COMM_DATA_GetTalkState( const u8 idx );
const BOOL	FIELD_COMM_DATA_GetGridPos_AfterMove( const u8 idx , int *posX , int *posZ );

const F_COMM_CHARA_STATE FIELD_COMM_DATA_GetCharaData_State( const u8 idx );
void FIELD_COMM_DATA_SetCharaData_State( const u8 idx , const F_COMM_CHARA_STATE state );
PLAYER_WORK* FIELD_COMM_DATA_GetCharaData_PlayerWork( const u8 idx);

static void FIELD_COMM_DATA_InitOneCharaData( FIELD_COMM_CHARA_DATA *charaData );
static FIELD_COMM_CHARA_DATA* FIELD_COMM_DATA_GetCharaDataWork( const u8 idx );

//ユーザーデータ関連
void	FIELD_COMM_DATA_CreateUserData( const F_COMM_USERDATA_TYPE type );
void	FIELD_COMM_DATA_DeleteUserData(void);
const u32 FIELD_COMM_DATA_GetUserDataSize( const F_COMM_USERDATA_TYPE type );
const F_COMM_USERDATA_TYPE	FIELD_COMM_DATA_GetUserDataType_From_Action( const F_COMM_ACTION_LIST action );
void*	FIELD_COMM_DATA_GetSelfUserData( const F_COMM_USERDATA_TYPE type );
void*	FIELD_COMM_DATA_GetPartnerUserData( const F_COMM_USERDATA_TYPE type );

//--------------------------------------------------------------
//	通信用データ管理初期化
//--------------------------------------------------------------
void FIELD_COMM_DATA_InitSystem( HEAPID heapID )
{
	u8 i;
	//GF_ASSERT( commData == NULL );
	if( commData == NULL )
	{
		//最初はヒープの確保と全初期化
		commData = GFL_HEAP_AllocMemory( heapID , sizeof(FIELD_COMM_DATA) );
		commData->heapID_ = heapID;
		for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
		{
			FIELD_COMM_DATA_InitOneCharaData( &commData->charaData_[i] );
		}
		commData->userData_.type_ = FCUT_NO_INIT;
		commData->userData_.selfData_ = NULL;
		commData->userData_.partnerData_ = NULL;
	}
	else
	{
		//データがすでにあるときはexistだけ切っておく(COMM_ACTOR再生成のため
		for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
		{
			commData->charaData_[i].isExist_ = FALSE;
		}
	}
}

//--------------------------------------------------------------
//	通信用データ管理初期化
//--------------------------------------------------------------
void	FIELD_COMM_DATA_TermSystem( void )
{
	GF_ASSERT( commData != NULL );
	GFL_HEAP_FreeMemory( commData );
	commData = NULL;
}

//--------------------------------------------------------------
//	データが存在するか？
//--------------------------------------------------------------
BOOL	FIELD_COMM_DATA_IsExistSystem(void)
{
	return( commData != NULL );
}

//--------------------------------------------------------------
//	FieldCommFuncのステートのバックアップ
//--------------------------------------------------------------
void	FIELD_COMM_DATA_SetFieldCommMode( const u8 state )
{
	GF_ASSERT( commData != NULL );
	commData->backupCommState_ = state;
}
const u8 FIELD_COMM_DATA_GetFieldCommMode(void)
{
	GF_ASSERT( commData != NULL );
	return commData->backupCommState_;
}

//--------------------------------------------------------------
//	自分のデータセット
//--------------------------------------------------------------
void	FIELD_COMM_DATA_SetSelfData_Pos( const ZONEID *zoneID , const VecFx32 *pos , const u16 *dir )
{
	GF_ASSERT( commData != NULL );
	PLAYERWORK_setZoneID( &commData->selfData_.plWork_ , *zoneID );
	PLAYERWORK_setPosition( &commData->selfData_.plWork_ , pos );
	PLAYERWORK_setDirection( &commData->selfData_.plWork_ , *dir );
	commData->selfData_.isExist_ = TRUE;
	commData->selfData_.isValid_ = TRUE;	
}
//--------------------------------------------------------------
//	自分のデータ取得
//--------------------------------------------------------------
PLAYER_WORK*	FIELD_COMM_DATA_GetSelfData_PlayerWork( void )
{
	return FIELD_COMM_DATA_GetCharaData_PlayerWork( FCD_SELF_INDEX );
//	GF_ASSERT( commData != NULL );
//	return &commData->selfData_.plWork_;
}

//--------------------------------------------------------------
//	指定番号のフラグ取得・設定
//--------------------------------------------------------------
void	FIELD_COMM_DATA_SetCharaData_IsExist( const u8 idx , const BOOL flg)
{
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	pData->isExist_ = flg;
}
void	FIELD_COMM_DATA_SetCharaData_IsValid( const u8 idx , const BOOL flg)
{
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	pData->isValid_ = flg;
}
const BOOL	FIELD_COMM_DATA_GetCharaData_IsExist( const u8 idx )
{
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	return pData->isExist_;
}
const BOOL	FIELD_COMM_DATA_GetCharaData_IsValid( const u8 idx )
{
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	return pData->isValid_;
}

//--------------------------------------------------------------
//	会話系：会話ステートの取得・設定
//--------------------------------------------------------------
void	FIELD_COMM_DATA_SetTalkState( const u8 idx , const F_COMM_TALK_STATE state )
{
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	pData->talkState_ = state;
}
const	F_COMM_TALK_STATE FIELD_COMM_DATA_GetTalkState( const u8 idx )
{
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	return pData->talkState_;
}

//--------------------------------------------------------------
//	キャラの移動後(停止時は現座標)を調べる
//	@return 移動中であればTRUE
//--------------------------------------------------------------
const BOOL	FIELD_COMM_DATA_GetGridPos_AfterMove( const u8 idx , int *posX , int *posZ )
{
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	if( pData->isExist_ == FALSE )
	{
		//居ないっ
		*posX = -1;
		*posZ = -1;
		return FALSE;
	}
	{
		PLAYER_WORK	*plWork = &pData->plWork_;
		//FIXME:正しいグリッドサイズ取得する？
		const u8 gridSize = 16;
		const int gridX = F32_CONST( plWork->position.x );
		const int gridZ = F32_CONST( plWork->position.z );
		u8	modSize;	//あまりの量
		*posX = gridX/gridSize;
		*posZ = gridZ/gridSize;
		//どれだけ中心位置からずれてるか？ XZ両方ずれていることは無いので片方だけ取る
		switch( plWork->direction )
		{
		case COMMDIR_UP:
		case COMMDIR_DOWN:
			modSize = gridZ - (*posZ*gridSize);
			break;
		case COMMDIR_LEFT:
		case COMMDIR_RIGHT:
			modSize = gridX - (*posX*gridSize);
			break;
		}
		//移動してないのでOK
		if( modSize == 0 )
			return FALSE;
		switch( plWork->direction )
		{
		case COMMDIR_UP:
			//処理なし
			break;
		case COMMDIR_DOWN:
			*posZ += 1;
			break;
		case COMMDIR_LEFT:
			//処理なし
			break;
		case COMMDIR_RIGHT:
			*posX += 1;
			break;
		}
		return TRUE;
	}
}

//--------------------------------------------------------------
//	指定番号のステータス取得
//--------------------------------------------------------------
const F_COMM_CHARA_STATE FIELD_COMM_DATA_GetCharaData_State( const u8 idx )
{	
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	return pData->state_;
}
void FIELD_COMM_DATA_SetCharaData_State( const u8 idx , const F_COMM_CHARA_STATE state )
{
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	pData->state_ = state;
}

//--------------------------------------------------------------
//	指定番号のデータ取得
//--------------------------------------------------------------
PLAYER_WORK*	FIELD_COMM_DATA_GetCharaData_PlayerWork( const u8 idx )
{
	FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(idx);
	return &pData->plWork_;
}

//--------------------------------------------------------------
//	通信用データ管理初期化
//--------------------------------------------------------------
static void FIELD_COMM_DATA_InitOneCharaData( FIELD_COMM_CHARA_DATA *charaData )
{
	charaData->isExist_ = FALSE;
	charaData->isValid_ = FALSE;
	charaData->state_ = FCCS_NONE;
	charaData->talkState_ = FCTS_NONE;
	PLAYERWORK_init( &charaData->plWork_ );
}

//--------------------------------------------------------------
//	データ取得用　自キャラ・他キャラ管理
//--------------------------------------------------------------
static FIELD_COMM_CHARA_DATA* FIELD_COMM_DATA_GetCharaDataWork( const u8 idx )
{
	GF_ASSERT( commData != NULL );
	if( idx == FCD_SELF_INDEX )
		return &commData->selfData_;
	GF_ASSERT( idx < FIELD_COMM_MEMBER_MAX );
	return &commData->charaData_[idx];
}

//ユーザーデータ関連
void	FIELD_COMM_DATA_CreateUserData( const F_COMM_USERDATA_TYPE type )
{
	GF_ASSERT( commData != NULL );
	GF_ASSERT( commData->userData_.type_ == FCUT_NO_INIT );

	commData->userData_.type_ = type;
	commData->userData_.selfData_ = GFL_HEAP_AllocMemory( commData->heapID_ , FIELD_COMM_DATA_GetUserDataSize( type ));
	commData->userData_.partnerData_ = GFL_HEAP_AllocMemory( commData->heapID_ , FIELD_COMM_DATA_GetUserDataSize( type ));
}

void	FIELD_COMM_DATA_DeleteUserData(void)
{
	GF_ASSERT( commData != NULL );
	GF_ASSERT( commData->userData_.type_ < FCUT_MAX );
	
	GFL_HEAP_FreeMemory( commData->userData_.selfData_ );
	GFL_HEAP_FreeMemory( commData->userData_.partnerData_ );
	commData->userData_.type_ = FCUT_NO_INIT;
}

const u32 FIELD_COMM_DATA_GetUserDataSize( const F_COMM_USERDATA_TYPE type )
{
	static const u32 UserDataSizeTable[FCUT_MAX] =
	{
		0x100,
		0x400,
	};
	return UserDataSizeTable[type];
}
const F_COMM_USERDATA_TYPE	FIELD_COMM_DATA_GetUserDataType_From_Action( const F_COMM_ACTION_LIST action )
{
	static const F_COMM_USERDATA_TYPE UserDataTypeTable[FCAL_MAX] =
	{
		FCUT_TRAINERCARD,
		FCUT_BATTLE,
		FCUT_MAX
	};
	return UserDataTypeTable[action];
}

void*	FIELD_COMM_DATA_GetSelfUserData( const F_COMM_USERDATA_TYPE type )
{
	GF_ASSERT( commData != NULL );
	//受信バッファでタイプの指定ができないのでFCUT_MAXで処置。
	GF_ASSERT( commData->userData_.type_ == type || type == FCUT_MAX );
	return commData->userData_.selfData_;
}

void*	FIELD_COMM_DATA_GetPartnerUserData( const F_COMM_USERDATA_TYPE type )
{
	GF_ASSERT( commData != NULL );
	//受信バッファでタイプの指定ができないのでFCUT_MAXで処置。
	GF_ASSERT( commData->userData_.type_ == type || type == FCUT_MAX );
	return commData->userData_.partnerData_;
}


