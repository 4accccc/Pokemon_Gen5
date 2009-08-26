//============================================================================================
/**
 * @file	field_gimmick_def.h
 * @brief	マップ固有の仕掛けのIDを定義するファイル
 * @author	saito
 * @date	2006.02.09
 *
 */
//============================================================================================
#ifndef __FIELD_GIMMICK_DEF_H__
#define __FIELD_GIMMICK_DEF_H__

typedef enum{
	FLD_GIMMICK_NONE,			//0:なし
  FLD_GIMMICK_TEST,
/*  
	FLD_GIMMICK_GHOST_GYM,		//1:ゴーストジム
	FLD_GIMMICK_COMBAT_GYM,		//2:格闘ジム
	FLD_GIMMICK_ELEC_GYM_K,		//3:電気ジム	カントー
	FLD_GIMMICK_SKY_GYM,		//4:飛行ジム
	FLD_GIMMICK_INSECT_GYM,		//5:虫ジム
	FLD_GIMMICK_DRAGON_GYM,		//6:ドラゴンジム
	FLD_GIMMICK_POISON_GYM,		//7:毒ジム
	FLD_GIMMICK_ALMIGHTY_GYM,	//8:万能ジム
	FLD_GIMMICK_ARCEUS_GMK,		//9:シント遺跡ギミック
*/
  FLD_GIMMICK_MAX,			//ギミック最大数(ギミック無しを含めた数)

}FLD_GYMMICK_ID;

#endif	//__FIELD_GIMMICK_DEF_H__

