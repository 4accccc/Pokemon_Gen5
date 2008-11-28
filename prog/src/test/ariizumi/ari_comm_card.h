//======================================================================
/**
 * @file	ari_comm_trade.h
 * @brief	フィールド通信テスト用パート
 * @author	ariizumi
 * @data	08/11/25
 */
//======================================================================

#ifndef ARI_COMM_CARD_H__
#define ARI_COMM_CARD_H__

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
typedef enum
{
	CMI_ID_TITLE,		//ID
	CMI_ID_USER,
	CMI_NAME_TITLE,		//名前
	CMI_NAME_USER,
	CMI_MONEY_TITLE,	//所持金
	CMI_MONEY_USER,
	CMI_BOOK_TITLE,		//図鑑匹数
	CMI_BOOK_USER,
	CMI_SCORE_TITLE,	//スコア
	CMI_SCORE_USER,	
	CMI_TIME_TITLE,		//プレイ時間
	CMI_TIME_USER,
	CMI_DAY_TITLE,		//プレイ開始日
	CMI_DAY_USER_YEAR,		//年
	CMI_DAY_USER_MONTH,		//月
	CMI_DAY_USER_DAY,		//日
	CMI_MAX,
}CARD_MESSAGE_INDEX;


//======================================================================
//	typedef struct
//======================================================================
typedef struct _ARI_COMM_CARD_WORK ARI_COMM_CARD_WORK;

//======================================================================
//	proto
//======================================================================
extern ARI_COMM_CARD_WORK* ARI_COMM_CARD_Init( HEAPID heapID );
extern const BOOL ARI_COMM_CARD_Loop( ARI_COMM_CARD_WORK *work );
extern void ARI_COMM_CARD_Term( ARI_COMM_CARD_WORK *work );

extern void ARI_COMM_CARD_GetCardString( char* str , const u8 idx , ARI_COMM_CARD_WORK *work );


#endif //ARI_COMM_CARD_H__
