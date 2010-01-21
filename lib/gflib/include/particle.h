//==============================================================================
/**
 * @file	particle.h
 * @brief	パーティクル制御ヘッダ
 * @author	matsuda
 * @date	2005.07.11(月)
 */
//==============================================================================
#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "spl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MATSUDA_TEST

//==============================================================================
//	定数定義
//==============================================================================
#define	DEFAULT_PERSP_WAY	(8192)		//投射影の角度のデフォルト値（エディタの見た目に近い値）0x06c1

//--------------------------------------------------------------
//	Particle_ResourceSet関数のvram_free_mode
//--------------------------------------------------------------
///テクスチャ自動解放なし
#define PTC_AUTOTEX_NULL	(0)
///フレームテクスチャVRAMマネージャを使って解放
#define PTC_AUTOTEX_FRM		(1 << 0)
///リンクドリストテクスチャVRAMマネージャを使って解放
#define PTC_AUTOTEX_LNK		(1 << 1)

///パレット自動解放なし
#define PTC_AUTOPLTT_NULL	(0)
///フレームパレットVRAMマネージャを使って解放
#define PTC_AUTOPLTT_FRM	(1 << 2)
///リンクドリストパレットVRAMマネージャを使って解放
#define PTC_AUTOPLTT_LNK	(1 << 3)

//--------------------------------------------------------------
//	システムでSPLライブラリとやり取りする際に使用するグローバルワークの番号
//--------------------------------------------------------------
enum{
	PARTICLE_GLOBAL_0 = 0,
	PARTICLE_GLOBAL_1,
	PARTICLE_GLOBAL_2,
	//ボールカスタマイズが最大8個のspaを読み込むので増やした
	PARTICLE_GLOBAL_3,
	PARTICLE_GLOBAL_4,
	PARTICLE_GLOBAL_5,
	PARTICLE_GLOBAL_6,
	PARTICLE_GLOBAL_7,

	//ボールカスタマイズが2個同時に出るので2倍必要･･･。
	PARTICLE_GLOBAL_8,
	PARTICLE_GLOBAL_9,
	PARTICLE_GLOBAL_10,
	PARTICLE_GLOBAL_11,
	PARTICLE_GLOBAL_12,
	PARTICLE_GLOBAL_13,
	PARTICLE_GLOBAL_14,
	PARTICLE_GLOBAL_15,
	
	PARTICLE_GLOBAL_MAX,
};

//--------------------------------------------------------------
//	使用ヒープ領域
//--------------------------------------------------------------
///パーティクルシステムで使用するヒープ領域
#define PTC_SYS_HEAP		(HEAPID_BATTLE)	//(SYSTEM_W_HEAP_AREA)

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
///パーティクルシステム内でSPLライブラリが使用するヒープワークの標準サイズ
//#define PARTICLE_LIB_HEAP_SIZE		(0x4200)	///< EMIT_MAXを3から20に増やしたので、0x3800から変更
#define PARTICLE_LIB_HEAP_SIZE		(0x4800)	///< カスタムボール以外　元々0x4200
#define PARTICLE_LIB_HEAP_SIZE_CB	(0x4200)	///< カスタムボール 0x4200までいける0x41D8が限界


///< パーティクルで必須としてるメモリのサイズ
///< (0x419c)

//==============================================================================
//	型定義
//==============================================================================
///パーティクルシステムワークへのポインタ
typedef struct _PARTICLE_SYSTEM * GFL_PTC_PTR;


#ifdef PM_DEBUG

extern GFL_PTC_PTR gDebugPTC;

static inline GFL_PTC_PTR DEBUG_GFL_PTC_PTR_Get(void)
{
	return gDebugPTC;
}

#endif


///エミッタのポインタ型
typedef SPLEmitter * GFL_EMIT_PTR;

///テクスチャロード時にVRAMアドレスを返すためのコールバック関数の型
typedef u32 (*pTexCallback)(u32, BOOL);

///エミッタ生成時に呼ばれるコールバック関数の型
typedef void (*pEmitFunc)(GFL_EMIT_PTR);

///SPL_SetCallbackFuncに指定するコールバック関数の型
typedef void (*pEmitCBFunc)( GFL_EMIT_PTR, unsigned int );

#define PTC_NON_CREATE_EMITTER  (GFL_EMIT_PTR)( 0xffffffff )  //エミッタのCreateに失敗

//==============================================================================
//	外部関数宣言
//==============================================================================
extern	void			GFL_PTC_Init(HEAPID heapID);
extern	void			GFL_PTC_Main(void);
extern	GFL_PTC_PTR		GFL_PTC_Create(void *work, int work_size, int personal_camera, int heap_id);
extern	void			GFL_PTC_Delete(GFL_PTC_PTR psys);
extern	void			GFL_PTC_Exit(void);
extern	BOOL			GFL_PTC_CheckEnable(void);

extern	void			GFL_PTC_DeleteDebug(GFL_PTC_PTR psys);

//直接path指定はとりあえずなし
//extern	void			*Particle_ResourceLoad(int heap_id, const char *path);
extern	void			*GFL_PTC_LoadArcResource(int file_kind, int index, int heap_id);
extern	void			GFL_PTC_SetResource(GFL_PTC_PTR psys, void *resource, int tex_at_once, GFL_TCBSYS *tcbsys);
extern	void			GFL_PTC_SetResourceEx(GFL_PTC_PTR psys, void *resource, int tex_at_once, GFL_TCBSYS *tcbsys);
extern	GFL_EMIT_PTR	GFL_PTC_CreateEmitter(GFL_PTC_PTR psys, int res_no, const VecFx32 *init_pos);
extern	void			GFL_PTC_Draw(GFL_PTC_PTR psys);
extern	void			GFL_PTC_Calc(GFL_PTC_PTR psys);
extern	int				GFL_PTC_DrawAll(void);
extern	int				GFL_PTC_CalcAll(void);
extern	s32				GFL_PTC_GetEmitterNum(GFL_PTC_PTR psys);
extern	void			GFL_PTC_DeleteEmitterAll(GFL_PTC_PTR psys);
extern	void			GFL_PTC_DeleteEmitter(GFL_PTC_PTR psys, GFL_EMIT_PTR emit);
extern	GFL_EMIT_PTR	GFL_PTC_GetTempEmitterPtr(GFL_PTC_PTR psys);
extern	GFL_EMIT_PTR	GFL_PTC_CreateEmitterCallback(GFL_PTC_PTR psys, int res_no, pEmitFunc callback, void *temp_ptr);
extern	void			*GFL_PTC_GetHeapPtr(GFL_PTC_PTR psys);
extern	void			GFL_PTC_PersonalCameraCreate(GFL_PTC_PTR psys,GFL_G3D_PROJECTION *proj,u16 persp_way,VecFx32 *Eye,VecFx32 *Up,VecFx32 *At,int heap_id);
extern	void			GFL_PTC_PersonalCameraDelete(GFL_PTC_PTR psys);
extern	void			GFL_PTC_GetEye(GFL_PTC_PTR psys, VecFx32 *eye);
extern	void			GFL_PTC_SetEye(GFL_PTC_PTR psys, const VecFx32 *eye);
extern	void			GFL_PTC_GetVup(GFL_PTC_PTR psys, VecFx32 *up);
extern	void			GFL_PTC_SetVup(GFL_PTC_PTR psys, const VecFx32 *up);
extern	void			*GFL_PTC_GetTempPtr(void);
extern	void			GFL_PTC_GetDefaultEye(VecFx32 *eye);
extern	void			GFL_PTC_GetDefaultUp(VecFx32 *v_up);
extern	void			GFL_PTC_GetDefaultAt(VecFx32 *at);
extern	int				GFL_PTC_GetActionNum(void);
extern	GFL_G3D_CAMERA	*GFL_PTC_GetCameraPtr(GFL_PTC_PTR ptc);
extern	void			GFL_PTC_SetCameraType(GFL_PTC_PTR ptc, int camera_type);
extern	u8				GFL_PTC_GetCameraType(GFL_PTC_PTR ptc);
extern	void			GFL_PTC_SetLnkTexKey(NNSGfdTexKey tex_key);
extern	void			GFL_PTC_GetLnkTexKey(NNSGfdTexKey tex_key);
extern	void			GFL_PTC_SetPlttLnkTexKey(NNSGfdPlttKey pltt_key);

extern	void			GFL_PTC_GetEmitterVelocity(GFL_EMIT_PTR emit, VecFx32 *vel);
extern	void			GFL_PTC_SetEmitterVelocity(GFL_EMIT_PTR emit, const VecFx32 *vel);
extern	void			GFL_PTC_GetEmitterAxis(GFL_EMIT_PTR emit, VecFx16 *vec);
extern	fx32			GFL_PTC_GetEmitterRadius(GFL_EMIT_PTR emit);
extern  void      GFL_PTC_SetEmitterRadius(GFL_EMIT_PTR emit, fx32 radius);
extern  fx32      GFL_PTC_GetEmitterLength(GFL_EMIT_PTR emit);
extern  void      GFL_PTC_SetEmitterLength(GFL_EMIT_PTR emit, fx32 length);
extern	fx16			GFL_PTC_GetEmitterInitVelocityPos(GFL_EMIT_PTR emit);
extern	void			GFL_PTC_SetEmitterInitVelocityPos(GFL_EMIT_PTR emit, fx16 vel);
extern	fx16			GFL_PTC_GetEmitterInitVelocityAxis(GFL_EMIT_PTR emit);
extern	void			GFL_PTC_SetEmitterInitVelocityAxis(GFL_EMIT_PTR emit, fx16 vel);
extern	fx16			GFL_PTC_GetEmitterBaseScale(GFL_EMIT_PTR emit);
extern	u16			  GFL_PTC_GetEmitterParticleLife(GFL_EMIT_PTR emit);
extern	u16				GFL_PTC_GetEmitterEmissionInterval(GFL_EMIT_PTR emit);
extern	u16				GFL_PTC_GetEmitterBaseAlpha(GFL_EMIT_PTR emit);
extern	GXRgb			GFL_PTC_GetEmitterGlobalColor(GFL_EMIT_PTR emit);
extern	void			GFL_PTC_SetEmitterGenerationRatio(GFL_EMIT_PTR emit, fx32 gen_num);
extern	fx32			GFL_PTC_GetEmitterGenerationRatio(GFL_EMIT_PTR emit);
extern	void			GFL_PTC_GetEmitterPosition(GFL_EMIT_PTR emit, VecFx32 * p_pos);
extern	void			GFL_PTC_GetEmitterBasePosition(GFL_EMIT_PTR emit, VecFx32 * p_pos);

//エミッタの位置を設定する
extern	void			GFL_PTC_SetEmitterPosition(GFL_EMIT_PTR emit, const VecFx32 * p_pos);
//エミッタの方向を設定する
extern	void			GFL_PTC_SetEmitterAxis(GFL_EMIT_PTR emit, const VecFx16 * p_vec);
//エミッタのスケールを設定する
extern	void			GFL_PTC_SetEmitterBaseScale(GFL_EMIT_PTR emit, const fx16 p_scale);
//エミッタの寿命を設定する
extern	void			GFL_PTC_SetEmitterParticleLife(GFL_EMIT_PTR emit, const u16 ptcl_life);

// -----------------------------------------
//
//	Field param 操作関数
//
// -----------------------------------------
extern	void			GFL_PTC_SetEmitterGravityPos(GFL_EMIT_PTR emit, VecFx16 *mag);
extern	void			GFL_PTC_GetEmitterGravityPos(GFL_EMIT_PTR emit, VecFx16 *mag);
extern	void			GFL_PTC_SetEmitterRandomMag(GFL_EMIT_PTR emit, VecFx16* mag);
extern	void			GFL_PTC_GetEmitterRandomMag(GFL_EMIT_PTR emit, VecFx16* mag);
extern	void			GFL_PTC_SetEmitterRandomIntvl(GFL_EMIT_PTR emit, u16* mag);
extern	void			GFL_PTC_GetEmitterRandomIntvl(GFL_EMIT_PTR emit, u16* mag);
extern	void			GFL_PTC_SetEmitterMagnetPos(GFL_EMIT_PTR emit, VecFx32* p_pos);
extern	void			GFL_PTC_GetEmitterMagnetPos(GFL_EMIT_PTR emit, VecFx32* p_pos);
extern	void			GFL_PTC_SetEmitterMagnetMag(GFL_EMIT_PTR emit, fx16* mag);
extern	void			GFL_PTC_GetEmitterMagnetMag(GFL_EMIT_PTR emit, fx16* mag);
extern	void			GFL_PTC_SetEmitterSpinRad(GFL_EMIT_PTR emit, u16* rad);
extern	void			GFL_PTC_GetEmitterSpinRad(GFL_EMIT_PTR emit, u16* rad);
extern	void			GFL_PTC_SetEmitterSpinAxisType(GFL_EMIT_PTR emit, u16* axis_type);
extern	void			GFL_PTC_GetEmitterSpinAxisType(GFL_EMIT_PTR emit, u16* axis_type);
extern	void			GFL_PTC_SetEmitterSimpleCollisionYPos(GFL_EMIT_PTR emit, fx32* y);
extern	void			GFL_PTC_GetEmitterSimpleCollisionYPos(GFL_EMIT_PTR emit, fx32* y);
extern	void			GFL_PTC_SetEmitterSimpleCollisionCoeffBounce(GFL_EMIT_PTR emit, fx16* coeff_bounce);
extern	void			GFL_PTC_GetEmitterSimpleCollisionCoeffBounce(GFL_EMIT_PTR emit, fx16* coeff_bounce);
extern	void			GFL_PTC_SetEmitterSimpleCollisionEventType(GFL_EMIT_PTR emit, u16* ev_type);
extern	void			GFL_PTC_GetEmitterSimpleCollisionEventType(GFL_EMIT_PTR emit, u16* ev_type);
extern	void			GFL_PTC_SetEmitterSimpleCollisionGlobal(GFL_EMIT_PTR emit, u16* global);
extern	void			GFL_PTC_GetEmitterSimpleCollisionGlobal(GFL_EMIT_PTR emit, u16* global);
extern	void			GFL_PTC_SetEmitterConvergencePos(GFL_EMIT_PTR emit, VecFx32* p_pos);
extern	void			GFL_PTC_GetEmitterConvergencePos(GFL_EMIT_PTR emit, VecFx32* p_pos);
extern	void			GFL_PTC_SetEmitterConvergenceRatio(GFL_EMIT_PTR emit, fx16* ratio);
extern	void			GFL_PTC_GetEmitterConvergenceRatio(GFL_EMIT_PTR emit, fx16* ratio);

// -----------------------------------------
//	コールバック関連
// -----------------------------------------
extern	void			GFL_PTC_SetCallbackFunc( GFL_EMIT_PTR emit, pEmitCBFunc func );
extern	void			GFL_PTC_SetUserData( GFL_EMIT_PTR emit, void *work );
extern	void			*GFL_PTC_GetUserData( GFL_EMIT_PTR emit );

// -----------------------------------------
//	公開データアクセス
// -----------------------------------------
extern u16 GFL_PTC_GetResNum( const void* p_spa );

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif	//__PARTICLE_H__

