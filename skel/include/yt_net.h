
//============================================================================================
/**
 * @file	yt_common.h
 * @brief	DS版ヨッシーのたまご通信ヘッダー
 * @date	2007.02.06
 */
//============================================================================================

#ifndef	__YT_NET_H__
#define	__YT_NET_H__


extern void YT_NET_DeleteReq(int clactno,NET_PARAM* pNet);
extern void YT_NET_SendPlayerAnmReq(int player_no,int anm_no,int pat_no,u16 line_no,int rot, NET_PARAM* pNet);
extern void YT_NET_SendAnmReq(int clactno,u16 anmseq,NET_PARAM* pNet);
extern void YT_NET_SendPosReq(int clactno,s16 x, s16 y,NET_PARAM* pNet);
extern BOOL YT_NET_Main(NET_PARAM* pNet);
extern BOOL YT_NET_IsParent(NET_PARAM* pNet);
extern void YT_NET_Init(GAME_PARAM* gp, BOOL bParent);
extern void YT_NetSendYossyBirthAnime(u8 posx,u8 posy,u8 count,NET_PARAM* pNet);
extern void YT_NET_SendAnmCreate(int clactno,u16 type,NET_PARAM* pNet);
extern void YT_NET_SendPlayerScreenMake(int player_no, int old_line_no, int new_line_no,NET_PARAM* pNet);
extern void YT_NET_SendPlayerRotateScreenMake(u8 player_no, u8 line_no, u8 flag,NET_PARAM* pNet);


#endif	__YT_NET_H__


