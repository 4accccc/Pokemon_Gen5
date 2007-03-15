
//============================================================================================
/**
 * @file	yt_common.h
 * @brief	DS版ヨッシーのたまご通信ヘッダー
 * @date	2007.02.06
 */
//============================================================================================

#ifndef	__YT_NET_H__
#define	__YT_NET_H__


extern void YT_NET_SendAnmReq(int clactno,CLWK* p_wk,u16 anmseq,NET_PARAM* pNet);
extern void YT_NET_SendPosReq(int clactno,CLWK* p_wk,s16 x, s16 y,NET_PARAM* pNet);
extern BOOL YT_NET_Main(NET_PARAM* pNet);
extern BOOL YT_NET_IsParent(NET_PARAM* pNet);
extern void YT_NET_Init(GAME_PARAM* gp, BOOL bParent);


#endif	__YT_NET_H__


