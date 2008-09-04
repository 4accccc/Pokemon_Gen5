//=============================================================================
/**
 * @file	sample_net.h
 * @brief	GFL通信ライブラリー
 * @author	GAME FREAK Inc.
 * @date    2008.07.28
 */
//=============================================================================

#ifndef __FIELD_NET_H__
#define __FIELD_NET_H__


void FieldInitGameNet(void);
BOOL FieldConnectGameNet(void);
void FieldEndGameNet(void);
BOOL FieldExitGameNet(void);
void FieldSendGameNet( int comm, void* commWork );
NetID FieldGetNetID(void);


#endif //__FIELD_NET_H__

