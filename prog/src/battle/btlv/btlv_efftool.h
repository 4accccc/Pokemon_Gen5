
//============================================================================================
/**
 * @file	btlv_efftool.h
 * @brief	戦闘エフェクトツール
 * @author	soga
 * @date	2008.11.14
 */
//============================================================================================

#pragma once

//タスクの種類
#define	EFFTOOL_CALCTYPE_DIRECT		    	( 0 )	//直接値を代入
#define	EFFTOOL_CALCTYPE_INTERPOLATION	( 1 )	//指定値までを補間しながら計算
#define	EFFTOOL_CALCTYPE_ROUNDTRIP	  	( 2 )	//指定した区間を往復計算（スタート地点とゴール地点の往復）
#define	EFFTOOL_CALCTYPE_ROUNDTRIP_LONG	( 3 )	//指定した区間を往復計算（スタート地点を基準に＋－方向の往復）

#define BTLV_EFFTOOL_POS2BIT_XOR  ( 0xffffffff )  //ビット反転する時にXORマスクする値

#ifndef __ASM_NO_DEF_

#include "btlv_mcss.h"

typedef struct
{
	int				move_type;
	VecFx32		start_value;
	VecFx32		end_value;
	VecFx32		vector;
	int				vec_time;
	int				vec_time_tmp;
	int				wait;
	int				wait_tmp;
	int				count;
}EFFTOOL_MOVE_WORK;

typedef struct
{ 
  GFL_G3D_RES** g3DRES;             //パレットフェード  対象3Dモデルリソースへのポインタ
  void**        pData_dst;          //パレットフェード  転送用ワーク
	u8            pal_fade_flag;      //パレットフェード  起動フラグ
  u8            pal_fade_count;     //パレットフェード  対象リソース数
	u8            pal_fade_start_evy; //パレットフェード　START_EVY値
	u8            pal_fade_end_evy;		//パレットフェード　END_EVY値
	u8            pal_fade_wait;			//パレットフェード　wait値
	u8            pal_fade_wait_tmp;	//パレットフェード　wait_tmp値
	u16           pal_fade_rgb;				//パレットフェード　end_evy時のrgb値
}EFFTOOL_PAL_FADE_WORK;

extern  void	BTLV_EFFTOOL_CalcMove( fx32 start, fx32 end, fx32 *out, fx32 frame );
extern	void	BTLV_EFFTOOL_CalcMoveVector( VecFx32 *start, VecFx32 *end, VecFx32 *out, fx32 flame );
extern	void	BTLV_EFFTOOL_CheckMove( fx32 *now_pos, fx32 *vec, fx32 *move_pos, BOOL *ret );
extern  void	BTLV_EFFTOOL_CheckVecMove( VecFx32 *now_pos, VecFx32 *vec, VecFx32 *move_pos, BOOL *ret );
extern	BOOL	BTLV_EFFTOOL_CalcParam( EFFTOOL_MOVE_WORK *emw, VecFx32 *now_param );
extern	u32		BTLV_EFFTOOL_Pos2Bit( BtlvMcssPos no );
extern  void  BTLV_EFFTOOL_CalcPaletteFade( EFFTOOL_PAL_FADE_WORK* epfw );

static	inline	u32	BTLV_EFFTOOL_No2Bit( u32 no )
{	
	GF_ASSERT( no < 32 );
	
	return ( 1 <<  ( no ) );
}

#endif __ASM_NO_DEF_
