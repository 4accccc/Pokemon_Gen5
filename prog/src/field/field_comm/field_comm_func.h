//======================================================================
/**
 * @file  field_comm_func.c
 * @brief フィールド通信　通信機能部
 * @author  ariizumi
 * @data  08/11/13
 */
//======================================================================

#ifndef FIELD_COMM_FUNC_H__
#define FIELD_COMM_FUNC_H__

#include "field_comm_def.h"
//======================================================================
//  define
//======================================================================

//======================================================================
//  enum
//======================================================================
//通信状態
typedef enum
{
  FIELD_COMM_MODE_NONE, //通信なし
  FIELD_COMM_MODE_WAIT,   //侵入待ち
  FIELD_COMM_MODE_SEARCH,   //侵入先探し中
  FIELD_COMM_MODE_CONNECT,  //侵入後
  FIELD_COMM_MODE_TRY_CONNECT,  //子機となり親へ接続を試みる
  FIELD_COMM_MODE_CONNECTING, //子機接続
}FIELD_COMM_MODE;

typedef enum
{
  FCRT_PROFILE, //キャラの基本情報
}F_COMM_REQUEST_TYPE;

//======================================================================
//  typedef struct
//======================================================================
///GameCommSys_Bootする時に引き渡すparent_work
typedef struct{
  GAMESYS_WORK *gsys;
  GAME_COMM_SYS_PTR game_comm;
  u8 parent_macAddress[6];
  u8 my_invasion;             ///<TRUE:侵入開始するユーザーである
  u8 padding;
}FIELD_INVALID_PARENT_WORK;


//======================================================================
//  proto
//======================================================================

extern FIELD_COMM_FUNC* FIELD_COMM_FUNC_InitSystem( HEAPID heapID );
extern void FIELD_COMM_FUNC_DataReset( FIELD_COMM_FUNC *commFunc );
extern void FIELD_COMM_FUNC_TermSystem( FIELD_COMM_FUNC *commFunc );
extern void * FIELD_COMM_FUNC_InitCommSystem( int *seq, void *pwk );
extern BOOL  FIELD_COMM_FUNC_InitCommSystemWait( int *seq, void *pwk, void *pWork );
extern BOOL  FIELD_COMM_FUNC_TermCommSystem( int *seq, void *pwk, void *pWork );
extern BOOL  FIELD_COMM_FUNC_TermCommSystemWait( int *seq, void *pwk, void *pWork );
extern void  FIELD_COMM_FUNC_UpdateSystem( int *seq, void *pwk, void *pWork );

extern void FIELD_COMM_FUNC_StartCommWait( FIELD_COMM_FUNC *commFunc );
extern void FIELD_COMM_FUNC_StartCommSearch( FIELD_COMM_FUNC *commFunc );
extern void FIELD_COMM_FUNC_StartCommChangeover( FIELD_COMM_FUNC *commFunc );
extern void FIELD_COMM_FUNC_InitCommData_StartTalk( FIELD_COMM_FUNC *commFunc );
//送信用関数
extern const BOOL  FIELD_COMM_FUNC_Send_SelfData( FIELD_COMM_FUNC *commFunc, FIELD_COMM_DATA *commData, int palace_area, int mission_no );
extern const BOOL FIELD_COMM_FUNC_Send_RequestData( const u8 charaIdx , const F_COMM_REQUEST_TYPE reqType , FIELD_COMM_FUNC *commFunc );
extern const BOOL  FIELD_COMM_FUNC_Send_SelfProfile( const int sendNetID , COMM_FIELD_SYS_PTR commField );
extern const BOOL FIELD_COMM_FUNC_Send_CommonFlg( FIELD_COMM_FUNC *commFunc, const F_COMM_COMMON_FLG flg , const u16 val , const u16 val2, const u8 sendID );
extern const BOOL  FIELD_COMM_FUNC_Send_UserData( FIELD_COMM_DATA *commData, F_COMM_USERDATA_TYPE type , const u8 sendID );
//同期用コマンド
extern void FIELD_COMM_FUNC_Send_SyncCommand( const F_COMM_SYNC_TYPE type , FIELD_COMM_FUNC *commFunc );
extern const BOOL FIELD_COMM_FUNC_CheckSyncFinish( const F_COMM_SYNC_TYPE type , const u8 checkBit , FIELD_COMM_FUNC *commFunc );
extern const u8 FIELD_COMM_FUNC_GetBit_TalkMember( FIELD_COMM_FUNC *commFunc );
//各種チェック関数
extern const BOOL FIELD_COMM_FUNC_IsFinishInitCommSystem( FIELD_COMM_FUNC *commFunc );
extern const BOOL FIELD_COMM_FUNC_IsFinishTermCommSystem( FIELD_COMM_FUNC *commFunc );
extern const FIELD_COMM_MODE FIELD_COMM_FUNC_GetCommMode( FIELD_COMM_FUNC *commFunc );
extern const int  FIELD_COMM_FUNC_GetMemberNum( void );
extern const int  FIELD_COMM_FUNC_GetSelfIndex( FIELD_COMM_FUNC *commFunc );
extern const BOOL FIELD_COMM_FUNC_IsReserveTalk( FIELD_COMM_FUNC *commFunc );
extern void FIELD_COMM_FUNC_ResetReserveTalk( FIELD_COMM_FUNC *commFunc );
extern void FIELD_COMM_FUNC_GetTalkParterData_ID( u8 *ID , FIELD_COMM_FUNC *commFunc );
extern void FIELD_COMM_FUNC_GetTalkParterData_Pos( u16 *posX , u16 *posZ , FIELD_COMM_FUNC *commFunc );
extern const F_COMM_ACTION_LIST FIELD_COMM_FUNC_GetSelectAction( FIELD_COMM_FUNC *commFunc );
extern const BOOL FIELD_COMM_FUNC_IsActionReturn( FIELD_COMM_FUNC *commFunc );
extern const BOOL FIELD_COMM_FUNC_GetActionReturn( FIELD_COMM_FUNC *commFunc );
extern void FIELD_COMM_FUNC_ResetGetUserData( FIELD_COMM_FUNC *commFunc );
extern const BOOL FIELD_COMM_FUNC_IsGetUserData( FIELD_COMM_FUNC *commFunc );


#endif //FIELD_COMM_FUNC_H__

