
//============================================================================================
/**
 * @file	gf_mcs.h
 * @brief	MCS関連関数
 * @author	soga
 * @date	2009.03.06
 */
//============================================================================================

#ifndef __GF_MCS_H_
#define __GF_MCS_H_

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================

// Windowsアプリケーションとの識別で使用するチャンネル値です
enum{
	MCS_CHANNEL0 = 0,
	MCS_CHANNEL1,
	MCS_CHANNEL2,
	MCS_CHANNEL3,
	MCS_CHANNEL4,
	MCS_CHANNEL5,
	MCS_CHANNEL6,
	MCS_CHANNEL7,
	MCS_CHANNEL8,
	MCS_CHANNEL9,
	MCS_CHANNEL10,
	MCS_CHANNEL11,
	MCS_CHANNEL12,
	MCS_CHANNEL13,
	MCS_CHANNEL14,
	MCS_CHANNEL15,

	MCS_CHANNEL_END,
};

#define	MCS_CH_MAX		( MCS_CHANNEL_END )
#define	MCS_SEND_SIZE	( 0x1800 )

extern	BOOL	MCS_Init( HEAPID heapID );
extern	BOOL	MCS_Open( void );
extern	void	MCS_Close( void );
extern	void	MCS_Exit( void );
extern	void	MCS_Main( void );
extern	u32		MCS_Read( void *buf, int size );
extern	BOOL	MCS_Write( int ch, const void *buf, int size );
extern	u32		MCS_CheckRead( void );
extern	BOOL	MCS_CheckEnable( void );

#endif __GF_MCS_H_
