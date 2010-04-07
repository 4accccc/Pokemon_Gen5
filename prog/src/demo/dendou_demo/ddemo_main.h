//============================================================================================
/**
 * @file		ddemo_main.h
 * @brief		殿堂入りデモ画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.06
 *
 *	モジュール名：DDEMOMAIN
 */
//============================================================================================
#pragma	once

#include "system/main.h"
#include "demo/dendou_demo.h"


// 後方確保用ヒープＩＤ
#define	HEAPID_DENDOU_DEMO_L		( GFL_HEAP_LOWID(HEAPID_DENDOU_DEMO) )


typedef struct {
	DENDOUDEMO_PARAM * dat;

	GFL_TCB * vtask;		// TCB ( VBLANK )

	GFL_G3D_UTIL * g3d_util;
	GFL_G3D_SCENE * g3d_scene;
	GFL_G3D_CAMERA * g3d_camera[2];
	GFL_G3D_LIGHTSET * g3d_light;
//	u32	g3d_obj_id;
//	u32	box_anm;

	GFL_PTC_PTR	ptc;											// パーティクル
	u8	ptcWork[PARTICLE_LIB_HEAP_SIZE];	// パーティクルで使用するワーク

	int	mainSeq;
	int	nextSeq;
}DDEMOMAIN_WORK;

typedef int (*pDDEMOMAIN_FUNC)(DDEMOMAIN_WORK*);




extern void DDEMOMAIN_InitVBlank( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_ExitVBlank( DDEMOMAIN_WORK * wk );

extern void DDEMOMAIN_InitVram(void);
extern const GFL_DISP_VRAM * DDEMOMAIN_GetVramBankData(void);

extern void DDEMOMAIN_InitBg(void);
extern void DDEMOMAIN_ExitBg(void);

extern void DDEMOMAIN_SetBlendAlpha(void);

extern void DDEMOMAIN_Init3D( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_Exit3D( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_Main3D( DDEMOMAIN_WORK * wk );

extern void DDEMOMAIN_InitParticle( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_ExitParticle( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_SetEffectParticle( DDEMOMAIN_WORK * wk, u32 id );
