//=============================================================================================
/**
 * @file	btl_net.h
 * @brief	ポケモンWB バトル  通信処理
 * @author	taya
 *
 * @date	2008.10.10	作成
 */
//=============================================================================================
#ifndef __BTL_NET_H__
#define __BTL_NET_H__



extern void BTL_NET_InitSystem( GFL_NETHANDLE* netHandle, HEAPID heapID );
extern void BTL_NET_QuitSystem( void );



typedef struct {

	u8  serverVersion;		///< サーババージョン
	u8  commPos;			///< 対戦部屋立ち位置（マルチ等で利用）
	u8  netID      : 4;		///< ネットID
	u8  pokeCount  : 4;		///< 参加ポケモン数
	u8  pokeData[1];		///< ポケモンデータ部

}BTL_NET_INITIALIZE_DATA;

typedef struct {

	struct {
		u8 netID;
		u8 clientID;
	}PairID[ BTL_CLIENT_MAX ];

}BTL_NET_INITIALIZE_RET_DATA;





typedef enum {

	BTL_NETCMD_SUPPLY_POKEDATA = 0,

}BtlNetCommand;





extern void BTL_NET_StartCommand( BtlNetCommand cmd );
extern BOOL BTL_NET_WaitCommand( void );


#endif
