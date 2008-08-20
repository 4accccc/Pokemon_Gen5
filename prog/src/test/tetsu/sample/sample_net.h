//=============================================================================
/**
 * @file	sample_net.h
 * @brief	GFL通信ライブラリー
 * @author	GAME FREAK Inc.
 * @date    2008.07.28
 */
//=============================================================================

#ifndef __SAMPLE_NET_H__
#define __SAMPLE_NET_H__


void InitSampleGameNet(void);
BOOL ConnectSampleGameNet(void);
void EndSampleGameNet(void);
BOOL ExitSampleGameNet(void);
void SendSampleGameNet( int comm, void* commWork );
NetID GetSampleNetID(void);


#endif //__SAMPLE_NET_H__

