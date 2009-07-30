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

WE_007:
	.long	WE_007_00 - WE_007	//AA2BB
	.long	WE_007_00 - WE_007	//BB2AA
	.long	WE_007_00 - WE_007	//A2B
	.long	WE_007_00 - WE_007	//A2C
	.long	WE_007_00 - WE_007	//A2D
	.long	WE_007_00 - WE_007	//B2A
	.long	WE_007_00 - WE_007	//B2C
	.long	WE_007_00 - WE_007	//B2D
	.long	WE_007_00 - WE_007	//C2A
	.long	WE_007_00 - WE_007	//C2B
	.long	WE_007_00 - WE_007	//C2D
	.long	WE_007_00 - WE_007	//D2A
	.long	WE_007_00 - WE_007	//D2B
	.long	WE_007_00 - WE_007	//D2C

WE_007_00:
	POKEMON_PAL_FADE	BTLEFF_TRAINER_POS_BB,	12,	12,	0,	0x00000000,	0x00000000,	0x00000000
	POKEMON_SCALE	BTLEFF_POKEMON_POS_BB,	BTLEFF_POKEMON_SCALE_DIRECT,	0x00000000,	0x00000000,	0,	0,	0
	POKEMON_MOVE	BTLEFF_POKEMON_POS_BB,	BTLEFF_POKEMON_MOVE_INTERPOLATION,	0x00000000,	0x00004000,	1,	0,	0
	CAMERA_MOVE_COODINATE	BTLEFF_CAMERA_MOVE_DIRECT,	0x000004cd,	0x00007800,	0xffff1000,	0x000004cd,	0x00003800,	0xfffe7000,	0,	0,	0
	CAMERA_MOVE_ANGLE	BTLEFF_CAMERA_MOVE_DIRECT,	0,	-8192,	0,	0,	0
	WAIT	20
	POKEMON_PAL_FADE	BTLEFF_TRAINER_POS_BB,	12,	0,	1,	0x00000000,	0x00000000,	0x00000000
	CAMERA_MOVE_ANGLE	BTLEFF_CAMERA_MOVE_INTERPOLATION,	0,	8192,	20,	0,	0
	EFFECT_END_WAIT	BTLEFF_EFFENDWAIT_CAMERA
	CAMERA_MOVE_COODINATE	BTLEFF_CAMERA_MOVE_INTERPOLATION_RELATIVITY,	0x00000000,	0xffffd000,	0x00012000,	0xfffff000,	0x00000000,	0x00012000,	40,	0,	0
	EFFECT_END_WAIT	BTLEFF_EFFENDWAIT_ALL
	CAMERA_PROJECTION	BTLEFF_CAMERA_PROJECTION_ORTHO
	SEQ_END	
