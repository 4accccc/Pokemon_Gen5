/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_TRANSPORT_H_
#define DWC_TRANSPORT_H_


#ifdef  __cplusplus
extern "C"
{
#endif

//#define DWC_RELIABLE_IS_PRIVATE ///< reliable送受信機能を外部に公開しない

#define DWC_TRANSPORT_MTU_SIZE                      1500    ///< Reliable/Unreliable送信で送信できる最大IPパケットサイズ
#define DWC_TRANSPORT_RELIABLE_ALL_HEADER_SIZE      35      ///< Reliable送信で付加されるヘッダサイズ
// IPヘッダ(20byte) + UDPヘッダ(8byte) + GameSpyのヘッダ(7byte)

#define DWC_TRANSPORT_UNRELIABLE_ALL_HEADER_SIZE    38      ///< Unreliable送信で付加されるヘッダサイズ
// IPヘッダ(20byte) + UDPヘッダ(8byte) + DWCのヘッダ(8byte) + GameSpyのヘッダ(2byte)

#define	DWC_TRANSPORT_SEND_RELIABLE_MAX         (DWC_TRANSPORT_MTU_SIZE-DWC_TRANSPORT_RELIABLE_ALL_HEADER_SIZE)     ///< Reliable送信で一度に送信できる最大データサイズ。
#define	DWC_TRANSPORT_SEND_UNRELIABLE_MAX       (DWC_TRANSPORT_MTU_SIZE-DWC_TRANSPORT_UNRELIABLE_ALL_HEADER_SIZE)   ///< Unreliable送信で一度に送信できる最大データサイズ。

#define DWC_TRANSPORT_SEND_MAX                  DWC_TRANSPORT_SEND_RELIABLE_MAX                                     ///< Reliable送信で一度に送信できる最大データサイズ。(互換性のために残されています。DWC_TRANSPORT_SEND_RELIABLE_MAXを使用してください。
    /** -----------------------------------------------------------------
      関数のtypedef
      -------------------------------------------------------------------*/

    /**
     * データの送信が完了したときに呼び出されるコールバック関数です。
     * 
     * DWC_SendReliable / DWC_SendReliableBitmap / DWC_SendUnreliable /
     * DWC_SendUnreliableBitmap 関数によるデータの送信が完了した時に呼び出されます
     *
     * AIDのビットマップによる送信を行った場合は、1つのAIDに対する送信が完了する度
     * に、コールバックが呼び出されます。
     *
     * 本コールバック関数はDWC_SetUserSendCallback関数によって設定することができま
     * す。
     *
     * Param:    size    送信したデータのサイズ
     * Param:    aid     送信先のAID
     * Param:    param   コールバック用パラメータ
     *
     */
    typedef void	(*DWCUserSendCallback)( int size, u8 aid, void* param );

    /**
     * データの受信があったときに呼び出されるコールバック関数です。
     * 
     * DWC_SendReliable / DWC_SendReliableBitmap / DWC_SendUnreliable /
     * DWC_SendUnreliableBitmap 関数によって送られてきたデータを受信した時に呼び出
     * されます。
     *
     * 本コールバック関数はDWC_SetUserRecvCallback関数によって設定することができま
     * す。
     *
     * Param:    aid     送信元のAID
     * Param:    buffer  受信したデータへのポインタ
     * Param:    size    受信したデータのサイズ
     * Param:    param   コールバック用パラメータ
     *
     */
    typedef void	(*DWCUserRecvCallback)( u8 aid, u8* buffer, int size, void* param );

    /**
     * 一定期間データの受信がないときに呼び出されるコールバック関数です。
     * 
     * DWC_SetRecvTimeoutTime関数で設定した期間、特定のホストからデータを受信しなか
     * った時に、呼び出されます。
     *
     * 本コールバック関数はDWC_SetUserRecvTimeoutCallback関数によって設定することが
     * できます。
     *
     * Param:    aid 送信元のAID
     * Param:    param コールバック用パラメータ
     *
     * See also:   DWC_SetUserRecvTimeoutCallback
     * See also:   DWC_SetRecvTimeoutTime
     *
     */
    typedef void	(*DWCUserRecvTimeoutCallback)( u8 aid, void* param );

    /**
     * DWC_Ping関数での測定結果を通知するコールバック関数です。
     * 
     * DWC_Ping関数での測定結果を通知するコールバック関数です。
     * 
     * 本コールバック関数はDWC_SetUserPingCallback関数によって設定することができま
     * す。
     *
     * Param:   latency Pingに要した往復時間（ミリ秒）
     * Param:   aid     Ping測定を行った相手のAID
     * Param:   param   コールバック用パラメータ
     *
     */
    typedef void	(*DWCUserPingCallback)( int latency, u8 aid, void* param );


    /* -----------------------------------------------------------------
      外部関数
      -------------------------------------------------------------------*/
#ifndef DWC_RELIABLE_IS_PRIVATE
    BOOL	DWC_SendReliable				( u8 aid, const void* buffer, int size );		// Relible送信
    u32		DWC_SendReliableBitmap			( u32 bitmap, const void* buffer, int size );	// Bitmap指定Relible送信
#endif
    BOOL	DWC_SendUnreliable				( u8 aid, const void* buffer, int size );				// Unreliable送信
    u32		DWC_SendUnreliableBitmap		( u32 bitmap, const void* buffer, int size );			// Bitmap指定Unreliable送信
    BOOL	DWC_Ping						( u8 aid );												// Ping値測定
    BOOL	DWC_SetRecvBuffer				( u8 aid, void* recvBuffer, int size );					// 受信バッファを設定
    BOOL	DWC_SetUserSendCallback			( DWCUserSendCallback callback, void* param );			// 送信コールバック
    BOOL	DWC_SetUserRecvCallback			( DWCUserRecvCallback callback, void* param );			// 受信コールバック
    BOOL	DWC_SetUserRecvTimeoutCallback	( DWCUserRecvTimeoutCallback callback, void* param );	// 受信タイムアウトコールバック
    BOOL	DWC_SetRecvTimeoutTime			( u8 aid, u32 time );									// 受信タイムアウト時間（単位ミリ秒）を設定
    BOOL	DWC_SetUserPingCallback			( DWCUserPingCallback callback, void* param );			// Ping測定完了コールバック
#ifndef DWC_RELIABLE_IS_PRIVATE
    BOOL	DWC_SetSendSplitMax				( u16 sendSplitMax );							// 送信分割最大サイズを設定
    BOOL	DWC_IsSendableReliable			( u8 aid );										// Reliable送信可能判定関数
#endif
    BOOL    DWC_SetConnectionKeepAliveTime	( u32 time );									// ホストのタイムアウト時間の設定
    u32     DWC_GetConnectionKeepAliveTime	( void );										// ホストのタイムアウト時間の取得

#ifndef	RVLDWC_FINAL

    BOOL	DWC_SetSendDelay				( u16 delay, u8 aid );							// 送信側に遅延を設定
    BOOL	DWC_SetRecvDelay				( u16 delay, u8 aid );							// 受信側に遅延を設定
    BOOL	DWC_SetSendDrop					( u8 drop, u8 aid );							// 送信側パケットロス率を設定
    BOOL	DWC_SetRecvDrop					( u8 drop, u8 aid );							// 受信側パケットロス率を設定

#else

#define	DWC_SetSendDelay( delay,  aid )	( (BOOL) 0 )
#define	DWC_SetRecvDelay( delay,  aid )	( (BOOL) 0 )
#define	DWC_SetSendDrop( drop, aid )	( (BOOL) 0 )
#define	DWC_SetRecvDrop( drop, aid )	( (BOOL) 0 )

#endif	// RVLDWC_FINAL


#ifdef  __cplusplus
}		/* extern "C" */
#endif


#endif // DWC_TRANSPORT_H_
