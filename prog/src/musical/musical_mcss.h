
//============================================================================================
/**
 * @file	musical_mcss.h
 * @brief	ミュージカル特化マルチセルソフトウエアスプライト描画用関数
 * @author	ariizumi
 * @date	2008.02.27
 */
//============================================================================================

#include <gflib.h>

#include "system/gfl_use.h"
#include "musical/musical_system.h"

#ifndef	MUSICAL_MCSS_H_
#define	MUSICAL_MCSS_H_

//--------------------------------------------------------------------------
/**
 * 定数定義
 */
//--------------------------------------------------------------------------

#define	MUS_MCSS_MEPACHI_ON		( 1 )		//メパチON
#define	MUS_MCSS_MEPACHI_OFF	( 0 )		//メパチOFF
#define	MUS_MCSS_ANM_STOP_ON	( 1 )		//アニメストップON
#define	MUS_MCSS_ANM_STOP_OFF	( 0 )		//アニメストップOFF
#define	MUS_MCSS_VANISH_ON		( 1 )		//バニッシュON
#define	MUS_MCSS_VANISH_OFF		( 0 )		//バニッシュOFF

//--------------------------------------------------------------------------
/**
 * 構造体定義
 */
//--------------------------------------------------------------------------
typedef struct
{
	VecFx32	pos;	//マルチセルの位置
	VecFx32	ofs;	//差分(回転前
	VecFx32	scale;	//全体の拡縮
	VecFx32 rotOfs;	//本体の回転差分
	u16		rotZ;	//回転(セルの回転
	u16		itemRotZ;	//回転(装備箇所に設定された回転
	
}MUS_MCSS_CELL_DATA;

typedef void (*MusicalCellCallBack)( const u8 pltNo , MUS_MCSS_CELL_DATA *cellData , void* work );

typedef struct
{
	ARCID		arcID;
	ARCDATID	ncbr;
	ARCDATID	nclr;
	ARCDATID	ncer;
	ARCDATID	nanr;
	ARCDATID	nmcr;
	ARCDATID	nmar;
	ARCDATID	ncec;
}MUS_MCSS_ADD_WORK;
typedef struct _MUS_MCSS_NCEC		MUS_MCSS_NCEC;
typedef struct _MUS_MCSS_NCEC_MUS	MUS_MCSS_NCEC_MUS;
typedef struct _MUS_MCSS_NCEC_WORK	MUS_MCSS_NCEC_WORK;

typedef struct _MUS_MCSS_WORK		MUS_MCSS_WORK;
typedef struct _MUS_MCSS_SYS_WORK	MUS_MCSS_SYS_WORK;

extern	MUS_MCSS_SYS_WORK*	MUS_MCSS_Init( int max, HEAPID heapID );
extern	void			MUS_MCSS_Exit( MUS_MCSS_SYS_WORK *mcss_sys );
extern	void			MUS_MCSS_Main( MUS_MCSS_SYS_WORK *mcss_sys );
extern	void			MUS_MCSS_Draw( MUS_MCSS_SYS_WORK *mcss_sys , MusicalCellCallBack musCellCb );
extern	MUS_MCSS_WORK*	MUS_MCSS_Add( MUS_MCSS_SYS_WORK *mcss_sys, fx32	pos_x, fx32	pos_y, fx32	pos_z, MUS_MCSS_ADD_WORK *maw , void *work , const BOOL isVBlank );
extern	void			MUS_MCSS_Del( MUS_MCSS_SYS_WORK *mcss_sys, MUS_MCSS_WORK *mcss );
extern	void			MUS_MCSS_SetOrthoMode( MUS_MCSS_SYS_WORK *mcss_sys );
extern	void			MUS_MCSS_ResetOrthoMode( MUS_MCSS_SYS_WORK *mcss_sys );
extern	void			MUS_MCSS_SetTexAddres( MUS_MCSS_SYS_WORK *mcss_sys , u32 adr );
extern	void			MUS_MCSS_SetPltAddres( MUS_MCSS_SYS_WORK *mcss_sys , u32 adr );
extern	void			MUS_MCSS_GetPosition( MUS_MCSS_WORK *mcss, VecFx32 *pos );
extern	void			MUS_MCSS_SetPosition( MUS_MCSS_WORK *mcss, VecFx32 *pos );
extern	void			MUS_MCSS_GetScale( MUS_MCSS_WORK *mcss, VecFx32 *scale );
extern	void			MUS_MCSS_SetScale( MUS_MCSS_WORK *mcss, VecFx32 *scale );
extern	void			MUS_MCSS_GetRotation( MUS_MCSS_WORK *mcss, u16 *rot );
extern	void			MUS_MCSS_SetRotation( MUS_MCSS_WORK *mcss, u16 rot );
extern	void			MUS_MCSS_SetShadowScale( MUS_MCSS_WORK *mcss, VecFx32 *scale );
extern	void			MUS_MCSS_SetMepachiFlag( MUS_MCSS_WORK *mcss );
extern	void			MUS_MCSS_ResetMepachiFlag( MUS_MCSS_WORK *mcss );
extern	void			MUS_MCSS_SetAnmStopFlag( MUS_MCSS_WORK *mcss );
extern	void			MUS_MCSS_ResetAnmStopFlag( MUS_MCSS_WORK *mcss );
extern	void			MUS_MCSS_ChangeAnm( MUS_MCSS_WORK *mcss , const u8 anmIdx );
extern	int				MUS_MCSS_GetVanishFlag( MUS_MCSS_WORK *mcss );
extern	void			MUS_MCSS_SetVanishFlag( MUS_MCSS_WORK *mcss );
extern	void			MUS_MCSS_ResetVanishFlag( MUS_MCSS_WORK *mcss );

//MCSS_WORKでデータをコピーする(前後ろ切り替え時 コピー元mcss,コピー先mcss
extern	void			MUS_MCSS_CopyState( MUS_MCSS_WORK *srcMcss , MUS_MCSS_WORK *dstMcss );

#endif	//MUSICAL_MCSS_H_
