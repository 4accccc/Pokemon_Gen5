//======================================================================
/**
 *
 * @file	dlplay_comm_func.c	
 * @brief	DL通信 親子共通の通信関係
 * @author	ariizumi
 * @data	08/10/21
 */
//======================================================================

#ifndef DLPALY_COMM_FUNC__
#define DLPALY_COMM_FUNC__

#include "dlplay_func.h"

#define TEST_DATA_LINE (16)
#define TEST_DATA_NUM  (256)

#define SELECT_BOX_INVALID (0xFF)

//ボックス1個分のPPPデータサイズ
#define LARGEPACKET_POKE_SIZE (30*0x88)
typedef struct _DLPLAY_COMM_DATA DLPLAY_COMM_DATA;
//送信用データ
typedef struct
{
	u8 pokeData_[LARGEPACKET_POKE_SIZE];
}DLPLAY_LARGE_PACKET;

//送信用 BOXインデックス
#define BOX_TRAY_NUM (18)
#define BOX_MONS_NUM (30)
#define BOX_TRAY_NAME_LEN (20)
#define BOX_MONS_NAME_LEN (10+1)
typedef struct
{
	u16 name_[BOX_MONS_NAME_LEN];
	u16 pokeNo_:9;
	u16	lv_:7;
	u8	sex_:2;
	u8	form_:5;
	u8	isEgg_:1;	//この段階では不正な卵かどうかは判断しない
	u8	rare_:1;	//レアかどうか？
	u8	padding_:7;
}DLPLAY_MONS_INDEX;
typedef struct
{
	u16	boxName_[BOX_TRAY_NUM][BOX_TRAY_NAME_LEN];
	DLPLAY_MONS_INDEX pokeData_[BOX_TRAY_NUM][BOX_MONS_NUM];
}DLPLAY_BOX_INDEX;

//本体に刺さっているカードの種類
typedef enum {
	CARD_TYPE_DP,		//ダイアモンド＆パール
	CARD_TYPE_PT,		//プラチナ
	CARD_TYPE_GS,		//ゴールド＆シルバー
	CARD_TYPE_INVALID,
}DLPLAY_CARD_TYPE;

//汎用フラグ送信用フラグ
enum DLPLAY_COMMAND_FLG_TYPE
{
	DC_FLG_FINISH_SAVE1_PARENT,	//親機第1セーブ完了
	DC_FLG_FINISH_SAVE1_CHILD,	//子機第1セーブ完了
	DC_FLG_PERMIT_LASTBIT_SAVE,	//最終ビットセーブ開始
	DC_FLG_FINISH_SAVE_ALL,		//全セーブ完了
	DC_FLG_POST_BOX_DATA,		//親→子ボックスデータ受信確認
};

extern DLPLAY_COMM_DATA* DLPlayComm_InitData( u32 heapID );
extern void	DLPlayComm_TermData( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_InitSystem( DLPLAY_COMM_DATA *d_comm);
extern void	DLPlayComm_TermSystem( DLPLAY_COMM_DATA *d_comm );
#if DLPLAY_FUNC_USE_PRINT
extern void	DLPlayComm_SetMsgSys( DLPLAY_MSG_SYS *msgSys , DLPLAY_COMM_DATA *d_comm );
#endif
extern void	DLPlayComm_InitParent( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_InitChild( DLPLAY_COMM_DATA *d_comm , u8 *macAddress );
extern BOOL	DLPlayComm_IsFinish_Negotiation( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_IsFinish_InitSystem( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_IsFinish_TermSystem( DLPLAY_COMM_DATA *d_comm );
extern BOOL DLPlayComm_IsConnect( DLPLAY_COMM_DATA *d_comm );
extern BOOL DLPlayComm_IsStartPostIndex( DLPLAY_COMM_DATA *d_comm );
extern BOOL DLPlayComm_IsPostIndex( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_IsPost_SendIndex( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_IsPostData( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_IsPost_SendData( DLPLAY_COMM_DATA *d_comm );
extern const u8 DLPlayComm_GetSelectBoxNumber( DLPLAY_COMM_DATA *d_comm );
extern const u8 DLPlayComm_GetPostErrorState( DLPLAY_COMM_DATA *d_comm );
extern const u16	DLPlayComm_IsFinishSaveFlg( u8 flg , DLPLAY_COMM_DATA *d_comm );

extern DLPLAY_LARGE_PACKET*	DLPlayComm_GetLargePacketBuff( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_Send_LargeData( DLPLAY_COMM_DATA *d_comm );
extern DLPLAY_BOX_INDEX* DLPlayComm_GetBoxIndexBuff( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_Send_BoxIndex( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_Send_ConnectSign( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_Send_BoxNumber( u8 idx , DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_Send_ErrorState( u8 type , DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_Send_CommonFlg( u8 flg , u16 value , DLPLAY_COMM_DATA *d_comm );

#endif // DLPALY_COMM_FUNC__

