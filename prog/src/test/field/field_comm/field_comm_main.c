//======================================================================
/**
 * @file	field_comm_main.c
 * @brief	フィールド通信の処理の外と中をつなぐ関数とか群
 * @author	ariizumi
 * @data	08/11/11
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "gamesystem/gamesystem.h"
#include "field_comm_main.h"
#include "field_comm_menu.h"
#include "field_comm_func.h"
#include "field_comm_data.h"

#include "msg/msg_d_field.h"

//======================================================================
//	define
//======================================================================
#define BGPLANE_YESNO_WINDOW (GFL_BG_FRAME1_M)	//使用フレーム(DEBUG_BGFRAME_MENU 
#define BGPLANE_MSG_WINDOW (GFL_BG_FRAME2_M)	//使用フレーム(DEBUG_BGFRAME_MENU 

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
struct _FIELD_COMM_MAIN
{
	u8	menuSeq_;
	HEAPID	heapID_;
	FIELD_COMM_MENU *commMenu_;
	FIELD_COMM_FUNC *commFunc_;
};

//======================================================================
//	proto
//======================================================================

FIELD_COMM_MAIN* FieldCommMain_InitSystem( int heapID );
void	FieldCommMain_TermSystem( FIELD_COMM_MAIN *commSys );
void	FieldCommMain_UpdateCommSystem( FIELD_MAIN_WORK *fieldWork , 
				GAMESYS_WORK *gameSys , PC_ACTCONT *pcActor , FIELD_COMM_MAIN *commSys );

//接続開始用メニュー処理
//開始時
void	FieldCommMain_InitStartCommMenu( FIELD_COMM_MAIN *commSys );
void	FieldCommMain_TermStartCommMenu( FIELD_COMM_MAIN *commSys );
const BOOL	FieldCommMain_LoopStartCommMenu( FIELD_COMM_MAIN *commSys );
//橋の時
void	FieldCommMain_InitStartInvasionMenu( FIELD_COMM_MAIN *commSys );
void	FieldCommMain_TermStartInvasionMenu( FIELD_COMM_MAIN *commSys );
const BOOL	FieldCommMain_LoopStartInvasionMenu( FIELD_COMM_MAIN *commSys );

//--------------------------------------------------------------
// フィールド通信システム初期化
//--------------------------------------------------------------
FIELD_COMM_MAIN* FieldCommMain_InitSystem( int heapID )
{
	FIELD_COMM_MAIN *commSys;
	commSys = GFL_HEAP_AllocMemory( heapID , sizeof(FIELD_COMM_MAIN) );
	commSys->heapID_ = heapID;
	commSys->commFunc_ = FieldCommFunc_InitSystem( heapID );
	
	FieldCommData_InitSystem( heapID );
	return commSys;
}

//--------------------------------------------------------------
// フィールド通信システム開放
//--------------------------------------------------------------
void FieldCommMain_TermSystem( FIELD_COMM_MAIN *commSys )
{
	FieldCommData_TermSystem();
	FieldCommFunc_TermSystem( commSys->commFunc_ );
	GFL_HEAP_FreeMemory( commSys );
}

//--------------------------------------------------------------
// フィールド通信システム更新
//	@param	FIELD_MAIN_WORK フィールドワーク
//	@param	GAMESYS_WORK	ゲームシステムワーク(PLAYER_WORK取得用
//	@param	PC_ACTCONT		プレイヤーアクター(プレイヤー数値取得用
//	@param	FIELD_COMM_MAIN	通信ワーク
//	自分のキャラの数値を取得して通信用に保存
//	他キャラの情報を取得し、通信から設定
//--------------------------------------------------------------
void	FieldCommMain_UpdateCommSystem( FIELD_MAIN_WORK *fieldWork , 
				GAMESYS_WORK *gameSys , PC_ACTCONT *pcActor , FIELD_COMM_MAIN *commSys )
{
	if( FieldCommFunc_IsFinishInitCommSystem( commSys->commFunc_ ) == TRUE )
	{
		ZONEID zoneID;
		VecFx32 pos;
		u16 dir;
		u8 i;
		PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gameSys );
		//自キャラ座標を更新
		zoneID = PLAYERWORK_getZoneID( plWork );
		GetPlayerActTrans( pcActor , &pos );
		//GetPlayerActDirection( pcActor , &dir );
		dir = FieldMainGrid_GetPlayerDir( fieldWork );
		FieldCommData_SetSelfData_Pos( &zoneID , &pos , &dir );

		FieldCommFunc_Send_SelfData( commSys->commFunc_ );

		//届いたデータのチェック
		for( i=0;i<FIELD_COMM_CHARA_MAX;i++ )
		{
			if( i != FieldCommFunc_GetSelfIndex(commSys->commFunc_) &&
				FieldCommData_GetCharaData_IsValid(i) == TRUE )
			{
				//有効なデータが入っている
				GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
				PLAYER_WORK *setPlWork = GAMEDATA_GetPlayerWork( gameData , i+1 );	//0には自分が入っているから
				PLAYER_WORK *charaWork = FieldCommData_GetCharaData_PlayerWork(i);
				GFL_STD_MemCopy( (void*)charaWork , (void*)setPlWork , sizeof(PLAYER_WORK) );
				if( FieldCommData_GetCharaData_IsExist(i) == FALSE )
				{
					//未初期化なキャラなので、初期化する
					FieldMain_AddCommActor( fieldWork , setPlWork );
					FieldCommData_SetCharaData_IsExist(i,TRUE);
				}
				
				FieldCommData_SetCharaData_IsValid(i,FALSE);
			}
		}
	}
}

//--------------------------------------------------------------
// 通信開始メニュー初期化
//--------------------------------------------------------------
void	FieldCommMain_InitStartCommMenu( FIELD_COMM_MAIN *commSys )
{
	commSys->commMenu_ = FieldCommMenu_InitCommMenu( commSys->heapID_ );
	FieldCommMenu_OpenMessageWindow( BGPLANE_MSG_WINDOW , commSys->commMenu_ );
	FieldCommMenu_OpenYesNoMenu( BGPLANE_YESNO_WINDOW , commSys->commMenu_ );
	FieldCommMenu_SetMessage( DEBUG_FIELD_C_STR00 , commSys->commMenu_ );
	commSys->menuSeq_ = 0;
}

//--------------------------------------------------------------
//	通信開始メニュー 開放 
//--------------------------------------------------------------
void	FieldCommMain_TermStartCommMenu( FIELD_COMM_MAIN *commSys )
{
	FieldCommMenu_CloseMessageWindow( commSys->commMenu_ );
	FieldCommMenu_TermCommMenu( commSys->commMenu_ );
}

//--------------------------------------------------------------
// 通信開始メニュー更新
//--------------------------------------------------------------
const BOOL	FieldCommMain_LoopStartCommMenu( FIELD_COMM_MAIN *commSys )
{
	FieldCommMenu_UpdateMessageWindow( commSys->commMenu_ );
	switch( commSys->menuSeq_ )
	{
	case 0:
		{
			const u8 ret = FieldCommMenu_UpdateYesNoMenu( commSys->commMenu_ );
			if( ret == YNR_YES ){
				FieldCommMenu_CloseYesNoMenu( commSys->commMenu_ );
				commSys->menuSeq_++;
			}
			else if( ret == YNR_NO ){
				FieldCommMenu_CloseYesNoMenu( commSys->commMenu_ );
				return (TRUE);
			}
		}
		break;
	case 1:
		//未初期化のときだけ初期化する
		if( FieldCommFunc_IsFinishInitCommSystem( commSys->commFunc_ ) == FALSE ){
			FieldCommFunc_InitCommSystem( commSys->commFunc_ );
		}
		commSys->menuSeq_++;
		break;
	case 2:
		if( FieldCommFunc_IsFinishInitCommSystem( commSys->commFunc_ ) == TRUE ){
			FieldCommFunc_StartCommChangeover( commSys->commFunc_ );
			commSys->menuSeq_++;
			return (TRUE);
		}
		break;
	}
	return (FALSE);
}

//--------------------------------------------------------------
// 侵入開始メニュー 初期化
//--------------------------------------------------------------
void	FieldCommMain_InitStartInvasionMenu( FIELD_COMM_MAIN *commSys )
{
	commSys->commMenu_ = FieldCommMenu_InitCommMenu( commSys->heapID_ );
	FieldCommMenu_OpenMessageWindow( BGPLANE_MSG_WINDOW , commSys->commMenu_ );
	FieldCommMenu_OpenYesNoMenu( BGPLANE_YESNO_WINDOW , commSys->commMenu_ );
	FieldCommMenu_SetMessage( DEBUG_FIELD_C_STR01 , commSys->commMenu_ );
	commSys->menuSeq_ = 0;
}

//--------------------------------------------------------------
// 侵入開始メニュー開放
//--------------------------------------------------------------
void	FieldCommMain_TermStartInvasionMenu( FIELD_COMM_MAIN *commSys )
{
	FieldCommMenu_CloseMessageWindow( commSys->commMenu_ );
	FieldCommMenu_TermCommMenu( commSys->commMenu_ );
}

//--------------------------------------------------------------
// 侵入開始メニュー更新
//--------------------------------------------------------------
const BOOL	FieldCommMain_LoopStartInvasionMenu( FIELD_COMM_MAIN *commSys )
{
	FieldCommMenu_UpdateMessageWindow( commSys->commMenu_ );
	switch( commSys->menuSeq_ )
	{
	case 0:
		{
			const u8 ret = FieldCommMenu_UpdateYesNoMenu( commSys->commMenu_ );
			if( ret == YNR_YES ){
				FieldCommMenu_CloseYesNoMenu( commSys->commMenu_ );
				commSys->menuSeq_++;
			}
			else if( ret == YNR_NO ){
				FieldCommMenu_CloseYesNoMenu( commSys->commMenu_ );
				return (TRUE);
			}
		}
		break;
	case 1:
		//未初期化のときだけ初期化する
		if( FieldCommFunc_IsFinishInitCommSystem( commSys->commFunc_ ) == FALSE ){
			FieldCommFunc_InitCommSystem( commSys->commFunc_ );
		}
		commSys->menuSeq_++;
		break;
	case 2:
		if( FieldCommFunc_IsFinishInitCommSystem( commSys->commFunc_ ) == TRUE ){
			FieldCommFunc_StartCommChangeover( commSys->commFunc_ );
			commSys->menuSeq_++;
			return (TRUE);
		}
		break;
	}
	return (FALSE);
}



