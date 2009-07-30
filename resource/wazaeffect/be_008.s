//===================================================
//
//	エフェクトシーケンス
//
//	コンバータから吐き出されたファイルです
//
//===================================================

	.text

#define	__ASM_NO_DEF_
	.include	c:/home/pokemon_wb/prog/src/battle/btlv/btlv_efftool.h
	.include	c:/home/pokemon_wb/prog/src/battle/btlv/btlv_effvm_def.h
	.include	c:/home/pokemon_wb/prog/include/system/mcss.h
	.include	c:/home/pokemon_wb/prog/include/sound/wb_sound_data.sadl
	.include	c:/home/pokemon_wb/prog/arc/particle/wazaeffect/spa_def.h

WE_008:
	.long	WE_008_00 - WE_008	//AA2BB
	.long	WE_008_00 - WE_008	//BB2AA
	.long	WE_008_00 - WE_008	//A2B
	.long	WE_008_00 - WE_008	//A2C
	.long	WE_008_00 - WE_008	//A2D
	.long	WE_008_00 - WE_008	//B2A
	.long	WE_008_00 - WE_008	//B2C
	.long	WE_008_00 - WE_008	//B2D
	.long	WE_008_00 - WE_008	//C2A
	.long	WE_008_00 - WE_008	//C2B
	.long	WE_008_00 - WE_008	//C2D
	.long	WE_008_00 - WE_008	//D2A
	.long	WE_008_00 - WE_008	//D2B
	.long	WE_008_00 - WE_008	//D2C

WE_008_00:
	POKEMON_SCALE	BTLEFF_TRAINER_POS_BB,	BTLEFF_POKEMON_SCALE_INTERPOLATION,	0x00000800,	0x00000800,	20,	0,	0
	CAMERA_MOVE	BTLEFF_CAMERA_MOVE_INTERPOLATION,	BTLEFF_CAMERA_POS_INIT,	20,	0,	18
	EFFECT_END_WAIT	BTLEFF_EFFENDWAIT_ALL
	SEQ_END	
