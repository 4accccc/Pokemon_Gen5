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
#include "system/bmp_oam.h"
#include "system/mcss.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "sound/sound_manager.h"
#include "demo/dendou_demo.h"

#include "ddemo_obj_def.h"


// 後方確保用ヒープＩＤ
#define	HEAPID_DENDOU_DEMO_L		( GFL_HEAP_LOWID(HEAPID_DENDOU_DEMO) )

// ＯＡＭフォントワーク
typedef struct {
	BMPOAM_ACT_PTR oam;
	GFL_BMP_DATA * bmp;
}DDEMO_FONTOAM;

typedef struct {
	DENDOUDEMO_PARAM * dat;

	GFL_TCB * vtask;		// TCB ( VBLANK )

	u32	scene;

	SOUNDMAN_PRESET_HANDLE * sndHandle;

	GFL_G3D_UTIL * g3d_util;
	GFL_G3D_SCENE * g3d_scene;
//	GFL_G3D_CAMERA * g3d_camera[2];
//	GFL_G3D_LIGHTSET * g3d_light;
//	u32	g3d_obj_id;
//	u32	box_anm;
//	BOOL	double3dFlag;
	MCSS_SYS_WORK * mcss;
	MCSS_WORK * mcssWork;
	GFL_G3D_CAMERA * mcssCamera;
	BOOL	mcssAnmEndFlg;

	GFL_PTC_PTR	ptc;											// パーティクル
	GFL_PTC_PTR	ptcName;									// パーティクル
	u8	ptcWork[PARTICLE_LIB_HEAP_SIZE];	// パーティクルで使用するワーク
	u8	ptcNameWork[PARTICLE_LIB_HEAP_SIZE];	// パーティクルで使用するワーク
	fx32	ptcCamera[5];

	GFL_FONT * font;						// 通常フォント
	GFL_FONT * nfnt;						// 数字フォント
	GFL_MSGDATA * mman;					// メッセージデータマネージャ
	WORDSET * wset;							// 単語セット
	STRBUF * exp;								// メッセージ展開領域
//	PRINT_QUE * que;						// プリントキュー

	// OBJ
	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[DDEMOOBJ_ID_MAX];
	u32	chrRes[DDEMOOBJ_CHRRES_MAX];
	u32	palRes[DDEMOOBJ_PALRES_MAX];
	u32	celRes[DDEMOOBJ_CELRES_MAX];
	GFL_CLSYS_REND * clrend;

	// ＯＡＭフォント
	BMPOAM_SYS_PTR	fntoam;
	DDEMO_FONTOAM	fobj[DDEMOOBJ_FOAM_MAX];

	u16	monsno;
	u16	type;
	u8	form;

	u8	pokePos;
	u8	pokeMax;

	u8	rndCnt;
	s16	rndDx;
	s16	rndDy;
	s16	rndMx[8];
	s16	rndMy[8];

	u32	voicePlayer;

	int	wait;

	int	mainSeq;
	int	subSeq;
	int	nextSeq;
}DDEMOMAIN_WORK;

typedef int (*pDDEMOMAIN_FUNC)(DDEMOMAIN_WORK*);




extern void DDEMOMAIN_InitScene1VBlank( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_InitScene2VBlank( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_ExitVBlank( DDEMOMAIN_WORK * wk );

extern void DDEMOMAIN_InitVram( u32 scene );
extern const GFL_DISP_VRAM * DDEMOMAIN_GetVramBankData( u32 scene );

extern void DDEMOMAIN_InitBg(void);
extern void DDEMOMAIN_ExitBg(void);
extern void DDEMOMAIN_InitBgMode(void);

extern void DDEMOMAIN_InitScene2BgFrame(void);
extern void DDEMOMAIN_ExitScene2BgFrame(void);
extern void DDEMOMAIN_LoadScene2BgGraphic(void);



extern void DDEMOMAIN_SetBlendAlpha(void);

extern void DDEMOMAIN_InitMsg( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_ExitMsg( DDEMOMAIN_WORK * wk );

extern void DDEMOMAIN_InitSound( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_ExitSound( DDEMOMAIN_WORK * wk );


extern void DDEMOMAIN_GetPokeMax( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_GetPokeData( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_LoadPokeVoice( DDEMOMAIN_WORK * wk );


extern void DDEMOMAIN_Init3D( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_Exit3D( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_Main3D( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_MainDouble3D( DDEMOMAIN_WORK * wk );

extern void DDEMOMAIN_InitDouble3D(void);
extern void DDEMOMAIN_ExitDouble3D(void);

extern void DDEMOMAIN_InitParticle(void);
extern void DDEMOMAIN_ExitParticle(void);
extern void DDEMOMAIN_CreateTypeParticle( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_DeleteTypeParticle( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_SetTypeParticle( DDEMOMAIN_WORK * wk, u32 id );
extern void DDEMOMAIN_CreateNameParticle( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_DeleteNameParticle( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_SetNameParticle( DDEMOMAIN_WORK * wk, u32 id );

extern void DDEMOMAIN_InitMcss( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_ExitMcss( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_AddMcss( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_DelMcss( DDEMOMAIN_WORK * wk );
extern void DDEMOMAIN_MoveMcss( DDEMOMAIN_WORK * wk, s16 mv );
extern void DDEMOMAIN_SetMcssCallBack( DDEMOMAIN_WORK * wk );
