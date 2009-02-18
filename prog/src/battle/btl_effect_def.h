//======================================================================
/**
 * @file	btl_effect_def.h
 * @brief	戦闘エフェクト
 * @author	HisashiSogabe
 * @date	2009.02.12
 */
//======================================================================

#ifndef __BTL_EFFECT_DEF_H_
#define __BTL_EFFECT_DEF_H_

#define	BTL_EFFVM_STACK_SIZE	( 16 )	///<使用するスタックのサイズ
#define	BTL_EFFVM_REG_SIZE		( 8 )	///<使用するレジスタの数

//カメラ移動タイプ
#define	BTL_CAMERA_MOVE_INIT			( 0 )		//初期位置
#define	BTL_CAMERA_MOVE_DIRECT			( 1 )		//ダイレクト
#define	BTL_CAMERA_MOVE_INTERPOLATION	( 2 )		//追従

#endif //__BTL_EFFECT_DEF_H_
