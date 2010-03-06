//=============================================================================================
/**
 * @file  btl_net.h
 * @brief ポケモンWB バトル  通信処理
 * @author  taya
 *
 * @date  2008.10.10  作成
 */
//=============================================================================================
#ifndef __BTL_NET_H__
#define __BTL_NET_H__

#include "poke_tool/pokeparty.h"
#include "battle/battle.h"

enum {
  BTL_NET_SERVER_VERSION = 100,
  BTL_NET_CONNECT_MACHINE_MAX = 4,  ///< 同時接続するマシン台数最大
};

typedef enum {

  BTL_NET_TIMING_NULL = 0,    ///< 無効値

  BTL_NET_TIMING_INITIALIZE,
  BTL_NET_TIMING_NOTIFY_PARTY_DATA,
  BTL_NET_TIMING_NOTIFY_PLAYER_DATA,
  BTL_NET_TIMING_SETUP_END,

  BTL_NET_TIMING_NOTIFY_AI_PARTY_1,
  BTL_NET_TIMING_NOTIFY_AI_PARTY_2,
  BTL_NET_TIMING_NOTIFY_AI_TRAINER_1,
  BTL_NET_TIMING_NOTIFY_AI_TRAINER_2,

}BtlNetTiming;


/**
 *  サーバーになったマシンが各マシンに対し、
 *  統一してもらうパラメータを送るのに使用する
 */
typedef struct {

  GFL_STD_RandContext  randomContext;
  u16                  debugFlagBit;
  u16                  LimitTimeGame;
  u16                  LimitTimeCommand;
  u8                   msgSpeed;
  u8                   fWazaEffectEnable : 4;
  u8                   fCommTag          : 4; ///< 通信タッグモード

}BTLNET_SERVER_NOTIFY_PARAM;

/**
 *  サーバマシンからAI用データ（パーティ、トレーナー）を送信するためのコンテナ
 */
typedef struct {

  u32     clientID :  8;
  u32     dataSize : 24;
  u8      data[0];

}BTLNET_AIDATA_CONTAINER;


extern void BTL_NET_InitSystem( GFL_NETHANDLE* netHandle, HEAPID heapID );
extern void BTL_NET_QuitSystem( void );
extern BOOL BTL_NET_CheckError( void );


extern BOOL BTL_NET_IsInitialized( void );
extern BOOL BTL_NET_DetermineServer( u8 clientID );

extern BOOL BTL_NET_IsServerDetermained( void );
extern BOOL BTL_NET_ImServer( void );
extern BOOL BTL_NET_NotifyServerParam( const BTLNET_SERVER_NOTIFY_PARAM* sendParam );
extern BOOL BTL_NET_IsServerParamReceived( BTLNET_SERVER_NOTIFY_PARAM* dst );
extern BOOL BTL_NET_StartNotifyPartyData( const POKEPARTY* party );
extern BOOL BTL_NET_IsCompleteNotifyPartyData( void );
extern const POKEPARTY* BTL_NET_GetPartyData( u8 clientID );
extern void BTL_NET_EndNotifyPartyData( void );

extern void BTL_NET_TimingSyncStart( u8 timingID );
extern BOOL BTL_NET_IsTimingSync( u8 timingID );

extern BOOL BTL_NET_SendToClient( u8 clientID, const void* adrs, u32 size );

extern BOOL BTL_NET_IsServerCmdReceived( void );
extern u32 BTL_NET_GetReceivedCmdData( const void** ppDest );

extern BOOL BTL_NET_ReturnToServer( const void* data, u32 size );
extern BOOL BTL_NET_CheckReturnFromClient( void );
extern u32 BTL_NET_GetRecvClientData( u8 clientID, const void** pptr );
extern void BTL_NET_ClearRecvData( void );

extern BOOL BTL_NET_StartNotifyPlayerData( const MYSTATUS* playerData );
extern BOOL BTL_NET_IsCompleteNotifyPlayerData( void );
extern void BTL_NET_EndNotifyPlayerData( void );
extern const MYSTATUS* BTL_NET_GetPlayerData( u8 clientID );


extern BOOL BTL_NET_StartNotify_AI_PartyData( const BTLNET_AIDATA_CONTAINER* container );
extern BOOL BTL_NET_IsRecved_AI_PartyData( u8 clientID );
extern BOOL BTL_NET_StartNotify_AI_TrainerData( const BSP_TRAINER_DATA* tr_data );
extern const BSP_TRAINER_DATA* BTL_NET_Get_AI_TrainerData( void );
extern void BTL_NET_Clear_AI_TrainerData( void );
extern BOOL BTL_NET_IsRecved_AI_TrainerData( void );


extern BTLNET_AIDATA_CONTAINER*  BTL_NET_AIDC_Create( u32 size, HEAPID heapID );
extern void BTL_NET_AIDC_Delete( BTLNET_AIDATA_CONTAINER* container );
extern void BTL_NET_AIDC_SetData( BTLNET_AIDATA_CONTAINER* container, const void* src, u8 clientID );


#endif
