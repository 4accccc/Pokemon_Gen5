//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _GFL_BBDACT_SYS		GFL_BBDACT_SYS;
typedef void		(GFL_BBDACT_FUNC)( GFL_BBDACT_SYS*, int actID, void* work );

typedef struct {
	u32		arcID;
	u32		datID;
	u8		texFmt;		//GFL_BBD_TEXFMT
	u8		texSiz;		//GFL_BBD_TEXSIZ
	u8		celSizX;
	u8		celSizY;
}GFL_BBDACT_RESDATA;

typedef const GFL_BBDACT_RESDATA*	GFL_BBDACT_RES_SETTBL;
typedef u16		GFL_BBDACT_RESUNIT_ID;

typedef struct {
	u16					resID;
	fx16				sizX;		// 0 < sizX < FX16_ONE*8-1
	fx16				sizY;		// 0 < sizY < FX16_ONE*8-1
	u8					alpha;
	BOOL				drawEnable; 
	GFL_BBD_LIGHTMASK	lightMask;
	VecFx32				trans;
	GFL_BBDACT_FUNC*	func;	//動作関数
	void*				work;
}GFL_BBDACT_ACTDATA;

typedef const GFL_BBDACT_ACTDATA*	GFL_BBDACT_ACT_SETTBL;
typedef u16		GFL_BBDACT_ACTUNIT_ID;

#define GFL_BBDACT_ANMFLIP_ON	(1)
#define GFL_BBDACT_ANMFLIP_OFF	(0)

#define GFL_BBDACT_ANMCOM_END	(0x3fff)
#define GFL_BBDACT_ANMCOM_CHG	(0x3ffe)
#define GFL_BBDACT_ANMCOM_LOOP	(0x3ffd)
#define GFL_BBDACT_ANMCOM_JMP	(0x3ffc)

//ループコマンド用定義
#define GFL_BBDACT_ANMLOOPPARAM( lpCnt, frmIdx )	{ ((lpCnt&0xff)<< 8)|(frmIdx&0xff) }

typedef union {
	struct {
		u16			command:14;			//コマンド
		u16			dummy0:1;			//フリップＳ
		u16			dummy1:1;			//フリップＴ
		u16			param;				//パラメータ
	} comData;

	struct {
		u16			celIdx:14;			//セルナンバー
		u16			flipS:1;			//フリップＳ
		u16			flipT:1;			//フリップＴ
		u16			wait;				//描画ウェイト
	} anmData;
}GFL_BBDACT_ANM;

typedef const GFL_BBDACT_ANM**	GFL_BBDACT_ANMTBL;
//------------------------------------------------------------------
/**
 *
 * @brief	セットアップ
 *
 */
//------------------------------------------------------------------
//	ビルボードアクトシステム作成 (return bbdActSys)
extern GFL_BBDACT_SYS*	GFL_BBDACT_CreateSys
							( const u16 bbdResMax, const u16 bbdActMax, HEAPID heapID );
//	ビルボードアクトメイン
extern void	GFL_BBDACT_Main( GFL_BBDACT_SYS* bbdActSys );
//	ビルボードアクト描画
extern void	GFL_BBDACT_Draw
		( GFL_BBDACT_SYS* bbdActSys, GFL_G3D_CAMERA* g3Dcamera, GFL_G3D_LIGHTSET* g3Dlightset );
//	ビルボードアクトシステム破棄
extern void	GFL_BBDACT_DeleteSys( GFL_BBDACT_SYS* bbdActSys );
//	ビルボードシステムポインタ取得
extern GFL_BBD_SYS*	GFL_BBDACT_GetBBDSystem( GFL_BBDACT_SYS* bbdActSys );

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクトリソースのセットアップと破棄
 */
//------------------------------------------------------------------
//セットアップ
extern GFL_BBDACT_RESUNIT_ID GFL_BBDACT_AddResourceUnit
			( GFL_BBDACT_SYS* bbdActSys, GFL_BBDACT_RES_SETTBL resTbl, u32 resCount );
//破棄
extern void GFL_BBDACT_RemoveResourceUnit
			( GFL_BBDACT_SYS* bbdActSys, GFL_BBDACT_RESUNIT_ID idx, u32 resCount );

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクトのセットアップと破棄
 */
//------------------------------------------------------------------
//セットアップ
extern GFL_BBDACT_ACTUNIT_ID GFL_BBDACT_AddAct( GFL_BBDACT_SYS* bbdActSys, 
				GFL_BBDACT_RESUNIT_ID idx, GFL_BBDACT_ACT_SETTBL actTbl, u32 actCount );
//破棄
extern void GFL_BBDACT_RemoveAct
				( GFL_BBDACT_SYS* bbdActSys, GFL_BBDACT_ACTUNIT_ID idx, u32 actCount ); 

//------------------------------------------------------------------
//	ビルボードアクト描画フラグ設定
extern void	GFL_BBDACT_SetDrawEnable( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, BOOL drawEnable );
//	ビルボードアクトライト反映フラグ設定
extern void	GFL_BBDACT_SetLightEnable
				( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, BOOL lightEnable, u8 lightNum );
//	ビルボードアクトアニメ設定
extern void	GFL_BBDACT_SetAnimeEnable( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, BOOL animeEnable );
extern void	GFL_BBDACT_SetAnimeTable( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, 
									GFL_BBDACT_ANMTBL animeTable, u16 animeTableCount );
extern void	GFL_BBDACT_SetAnimeIdx( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeIdx );
extern void	GFL_BBDACT_SetAnimeIdxContinue( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeIdx );
extern void	GFL_BBDACT_SetAnimeFrmIdx( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeFrmIdx );

inline void GFL_BBDACT_SetAnimeIdxOn( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeIdx )
{
	GFL_BBDACT_SetAnimeIdx( bbdActSys, actIdx, animeIdx );
	GFL_BBDACT_SetAnimeEnable( bbdActSys, actIdx, TRUE );
}
inline void GFL_BBDACT_SetFrmIdxOn( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeFrmIdx )
{
	GFL_BBDACT_SetAnimeFrmIdx( bbdActSys, actIdx, animeFrmIdx );
	GFL_BBDACT_SetAnimeEnable( bbdActSys, actIdx, TRUE );
}
//	動作関数設定
extern void	GFL_BBDACT_SetFunc( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, GFL_BBDACT_FUNC* func );

