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
	PLAYER_WORK plWork_;
}FIELD_COMM_CHARA_DATA;

typedef struct 
{
	FIELD_COMM_CHARA_DATA selfData_;	//自身のデータ。
	FIELD_COMM_CHARA_DATA charaData_[FIELD_COMM_CHARA_MAX];
}FIELD_COMM_DATA;

//あえて独立して確保させる
FIELD_COMM_DATA *commData = NULL;

//======================================================================
//	proto
//======================================================================
void	FieldCommData_InitSystem( HEAPID heapID );
void	FieldCommData_TermSystem();

void	FieldCommData_SetSelfData_PlayerWork( const PLAYER_WORK *plWork );
void	FieldCommData_SetSelfDataPos( const ZONEID zoneID , const VecFx32 *pos , const u16 *dir );
PLAYER_WORK* FieldCommData_GetSelfData_PlayerWork(void);

void	FieldCommData_SetCharaData_IsExist( const idx , const BOOL );
void	FieldCommData_SetCharaData_IsValid( const idx , const BOOL );
const BOOL	FieldCommData_GetCharaData_IsExist( const idx );
const BOOL	FieldCommData_GetCharaData_IsValid( const idx );
PLAYER_WORK* FieldCommData_GetCharaData_PlayerWork( const idx);

static void FieldCommData_InitOneCharaData( FIELD_COMM_CHARA_DATA *charaData );

//--------------------------------------------------------------
//	通信用データ管理初期化
//--------------------------------------------------------------
void FieldCommData_InitSystem( HEAPID heapID )
{
	u8 i;
	GF_ASSERT( commData == NULL );
	commData = GFL_HEAP_AllocMemory( heapID , sizeof(FIELD_COMM_DATA) );
	for( i=0;i<FIELD_COMM_CHARA_MAX;i++ )
	{
		FieldCommData_InitOneCharaData( &commData->charaData_[i] );
	}
}

//--------------------------------------------------------------
//	通信用データ管理初期化
//--------------------------------------------------------------
void	FieldCommData_TermSystem( )
{
	GF_ASSERT( commData != NULL );
	GFL_HEAP_FreeMemory( commData );
	commData = NULL;
}

//--------------------------------------------------------------
//	自分のデータセット
//--------------------------------------------------------------
void	FieldCommData_SetSelfData_Pos( const ZONEID *zoneID , const VecFx32 *pos , const u16 *dir )
{
	GF_ASSERT( commData != NULL );
	PLAYERWORK_setZoneID( &commData->selfData_.plWork_ , *zoneID );
	PLAYERWORK_setPosition( &commData->selfData_.plWork_ , pos );
	PLAYERWORK_setDirection( &commData->selfData_.plWork_ , *dir );
}
//--------------------------------------------------------------
//	自分のデータ取得
//--------------------------------------------------------------
PLAYER_WORK*	FieldCommData_GetSelfData_PlayerWork( void )
{
	GF_ASSERT( commData != NULL );
	return &commData->selfData_.plWork_;
}

//--------------------------------------------------------------
//	指定番号のフラグ取得・設定
//--------------------------------------------------------------
void	FieldCommData_SetCharaData_IsExist( const idx , const BOOL flg)
{
	commData->charaData_[idx].isExist_ = flg;
}
void	FieldCommData_SetCharaData_IsValid( const idx , const BOOL flg)
{
	commData->charaData_[idx].isValid_ = flg;
}
const BOOL	FieldCommData_GetCharaData_IsExist( const idx )
{
	return commData->charaData_[idx].isExist_;
}
const BOOL	FieldCommData_GetCharaData_IsValid( const idx )
{
	return commData->charaData_[idx].isValid_;
}

//--------------------------------------------------------------
//	指定番号のデータ取得
//--------------------------------------------------------------
PLAYER_WORK*	FieldCommData_GetCharaData_PlayerWork( const idx )
{
	GF_ASSERT( commData != NULL );
	return &commData->charaData_[idx].plWork_;
}

//--------------------------------------------------------------
//	通信用データ管理初期化
//--------------------------------------------------------------
static void FieldCommData_InitOneCharaData( FIELD_COMM_CHARA_DATA *charaData )
{
	charaData->isExist_ = FALSE;
	charaData->isValid_ = FALSE;
	PLAYERWORK_init( &charaData->plWork_ );
}

