//======================================================================
/**
 * @file	plist_comm.h
 * @brief	ポケモンリスト通信(バトル同期処理
 * @author	ariizumi
 * @data	10/03/10
 *
 * モジュール名：PLIST_COMM
 */
//======================================================================
#pragma once

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define PLIST_COMM_TIMMIN_INIT_LIST (1) //開始時・コマンドの追加
#define PLIST_COMM_TIMMIN_EXIT_LIST (2) //終了時・コマンドの削除

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  PCFT_FINISH_SELECT,
}PLIST_COMM_FLG_TYPE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
FS_EXTERN_OVERLAY(pokelist_comm);

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern void PLIST_COMM_InitComm( PLIST_DATA *work );
extern void PLIST_COMM_ExitComm( PLIST_DATA *work );
extern void PLIST_COMM_UpdateComm( PLIST_DATA *work );

extern const BOOL PLIST_COMM_SendFlg( PLIST_DATA *work , const u8 flgType , const u16 flgValue );
extern void PLIST_COMM_SendTimming( PLIST_DATA *work , const u8 timmingNo );
extern const BOOL PLIST_COMM_CheckTimming( PLIST_DATA *work , const u8 timmingNo );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
