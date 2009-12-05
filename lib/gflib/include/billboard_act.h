//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		billboard_act.h
 *	@brief		ビルボートアクターシステム
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __BILLBOARD_ACT_H__
#define __BILLBOARD_ACT_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
/**
 *				定数
 */
//=============================================================================
//-------------------------------------
///	ビルボードアクターリソースタイプ
//=====================================
typedef enum {
	GFL_BBDACT_RESTYPE_DEFAULT = 0,	//リソースデータをVRAM転送後も存続
	GFL_BBDACT_RESTYPE_DATACUT,		//リソースデータをVRAM転送後に破棄
	GFL_BBDACT_RESTYPE_TRANSSRC,	//VRAM転送アニメ用リソース
}GFL_BBDACT_RESTYPE;

//-------------------------------------
///	リソース転送の種類
//	（転送用コールバックにて渡ってきます）
//=====================================
typedef enum {
	GFL_BBDACT_TRANSTYPE_DATA = 0,	//リソースの転送種類がデータ
	GFL_BBDACT_TRANSTYPE_PLTT,		//リソースの転送種類がパレット
}GFL_BBDACT_TRANSTYPE;

//=============================================================================
/**
 *				システム定義
 */
//=============================================================================
//-------------------------------------
///	ビルボードアクターシステム不完全型
//=====================================
typedef struct _GFL_BBDACT_SYS		GFL_BBDACT_SYS;


//-------------------------------------
///	動作用コールバック関数
//=====================================
typedef void	(GFL_BBDACT_FUNC)( GFL_BBDACT_SYS*, int actID, void* work );

//-------------------------------------
///	リソース転送用コールバック関数
//  GFL_BBDACT_TRANSTYPEが引数として渡ってきますので
//  リソース別の読み込みをしたい場合は指定してください。
//=====================================
typedef void	(GFL_BBDACT_TRANSFUNC)( GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz );

//=============================================================================
/**
 *				リソース用定義
 */
//=============================================================================

//-------------------------------------
///	リソース設定用情報
//	VRAM転送アニメ用に
//	リソースはテーブルで渡すようになっています。
//=====================================
typedef struct {
	u32					arcID;
	u32					datID;
	u8					texFmt;		//GFL_BBD_TEXFMT
	u8					texSiz;		//GFL_BBD_TEXSIZ
	u8					celSizX;
	u8					celSizY;
	GFL_BBDACT_RESTYPE	dataCut;
}GFL_BBDACT_RESDATA;
typedef const GFL_BBDACT_RESDATA*	GFL_BBDACT_RES_SETTBL;

//-------------------------------------
///	G3DRES版リソース設定用情報
//	VRAM転送アニメ用に
//	リソースはテーブルで渡すようになっています。
//=====================================
typedef struct {
	GFL_G3D_RES			*g3dres;		
	u8					texFmt;		//GFL_BBD_TEXFMT
	u8					texSiz;		//GFL_BBD_TEXSIZ
	u8					celSizX;
	u8					celSizY;
	GFL_BBDACT_RESTYPE	dataCut;
}GFL_BBDACT_G3DRESDATA;
typedef const GFL_BBDACT_G3DRESDATA*	GFL_BBDACT_G3DRES_SETTBL;

//-------------------------------------
///	リソース設定番号（１テーブル１番号です）
//=====================================
typedef u16		GFL_BBDACT_RESUNIT_ID;

//=============================================================================
/**
 *				アクター用定義
 */
//=============================================================================

//-------------------------------------
///	アクター設定用情報
//	アクターも一気に作成することができるように
//	テーブルで渡すようになっています。
//=====================================
typedef struct {
	u16					resID;		//テーブルの中の何番目を使うか
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

//-------------------------------------
///	アクター設定番号（１テーブル１番号です）
//=====================================
typedef u16		GFL_BBDACT_ACTUNIT_ID;


//=============================================================================
/**
 *				アニメーション用定義
 */
//=============================================================================
#define GFL_BBDACT_ANMFLIP_ON	(1)			//反転させる
#define GFL_BBDACT_ANMFLIP_OFF	(0)			//反転させない
//-------------------------------------
///	アニメ用コマンド
//=====================================
#define GFL_BBDACT_ANMCOM_END	(0x3fff)	//アニメ終了コマンド
#define GFL_BBDACT_ANMCOM_CHG	(0x3ffe)	//アニメシーケンス変更コマンド
#define GFL_BBDACT_ANMCOM_LOOP	(0x3ffd)	//アニメフレームループ用コマンド
#define GFL_BBDACT_ANMCOM_JMP	(0x3ffc)	//アニメフレーム変更用コマンド

//-------------------------------------
///	ループ用設定マクロ
//=====================================
#define GFL_BBDACT_ANMLOOPPARAM( lpCnt, frmIdx )	{ ((lpCnt&0xff)<< 8)|(frmIdx&0xff) }

//-------------------------------------
///	アニメーション設定構造体
//	アニメーション設定構造体はアニメ切り替え用とコマンド用を
//	共通のテーブルにできるようにunionになっています。
//=====================================
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


//=============================================================================
/**
 *								関数宣言
 */
//=============================================================================
//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
//	ビルボードアクトシステム作成 (return bbdActSys)
extern GFL_BBDACT_SYS*	GFL_BBDACT_CreateSys
	( const u16 bbdResMax, const u16 bbdActMax, GFL_BBDACT_TRANSFUNC transFunc, HEAPID heapID );
//	ビルボードアクトメイン
extern void	GFL_BBDACT_Main( GFL_BBDACT_SYS* bbdActSys );
//	ビルボードアクト描画
extern void	GFL_BBDACT_Draw
		( GFL_BBDACT_SYS* bbdActSys, GFL_G3D_CAMERA* g3Dcamera, GFL_G3D_LIGHTSET* g3Dlightset );
//	ビルボードアクトシステム破棄
extern void	GFL_BBDACT_DeleteSys( GFL_BBDACT_SYS* bbdActSys );
//	ビルボードシステムポインタ取得
extern GFL_BBD_SYS*	GFL_BBDACT_GetBBDSystem( GFL_BBDACT_SYS* bbdActSys );
extern const GFL_BBD_SYS* GFL_BBDACT_GetBBDSystemConst( const GFL_BBDACT_SYS* bbdActSys );

//------------------------------------------------------------------
/**
 * @brief	ビルボードアクトリソースのセットアップと破棄
 */
//------------------------------------------------------------------
//セットアップ
extern GFL_BBDACT_RESUNIT_ID GFL_BBDACT_AddResourceUnit
			( GFL_BBDACT_SYS* bbdActSys, GFL_BBDACT_RES_SETTBL resTbl, u32 resCount );

extern GFL_BBDACT_RESUNIT_ID GFL_BBDACT_AddResourceG3DResUnit
			( GFL_BBDACT_SYS* bbdActSys, GFL_BBDACT_G3DRES_SETTBL resTbl, u32 resCount );
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
/**
 * @brief	ビルボードアクトの設定、取得関数
 */
//------------------------------------------------------------------
//	ビルボードアクト描画フラグ設定
extern void	GFL_BBDACT_SetDrawEnable( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, BOOL drawEnable );
extern BOOL GFL_BBDACT_GetDrawEnable( const GFL_BBDACT_SYS* bbdActSys, u16 actIdx );
//	ビルボードアクトライト反映フラグ設定
extern void	GFL_BBDACT_SetLightEnable
				( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, BOOL lightEnable, u8 lightNum );
extern BOOL GFL_BBDACT_GetLightEnable
				( const GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u8 lightNum );
//	ビルボードアクトアニメ設定
extern void	GFL_BBDACT_SetAnimeEnable( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, BOOL animeEnable );
extern BOOL GFL_BBDACT_GetAnimeEnable( const GFL_BBDACT_SYS* bbdActSys, u16 actIdx );
extern void	GFL_BBDACT_SetAnimeTable( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, 
									GFL_BBDACT_ANMTBL animeTable, u16 animeTableCount );
//	アニメインデックス設定・取得
extern void	GFL_BBDACT_SetAnimeIdx( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeIdx );
extern u16 GFL_BBDACT_GetAnimeIdx( const GFL_BBDACT_SYS* bbdActSys, u16 actIdx );
extern void	GFL_BBDACT_SetAnimeIdxContinue( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeIdx );
//	アニメフレームインデックス設定・取得
extern void	GFL_BBDACT_SetAnimeFrmIdx( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeFrmIdx );
extern u16 GFL_BBDACT_GetAnimeFrmIdx( const GFL_BBDACT_SYS* bbdActSys, u16 actIdx );
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
extern BOOL	GFL_BBDACT_GetAnimeLastCommand( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16* comm );

//	動作関数設定
extern void	GFL_BBDACT_SetFunc( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, GFL_BBDACT_FUNC* func );
//	転送リソース関連付け（既にリソースとして読み込まれている場合に使用）
extern void	GFL_BBDACT_BindActTexRes( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 resIdx );
//	転送リソースロード＆設定（リソースとして読み込まれていない場合に使用）
extern void	GFL_BBDACT_BindActTexResLoad
			( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u32 arcID, u32 datID );

//	ビルボードオブジェクト配列ＩＮＤＥＸ取得
extern int	GFL_BBDACT_GetBBDActIdxResIdx( GFL_BBDACT_SYS* bbdActSys, u16 actIdx );
//	リソースＩＤＸ取得
extern u16	GFL_BBDACT_GetResIdx( GFL_BBDACT_SYS* bbdActSys, u16 actIdx );
//	アクターのリソース変更
extern void GFL_BBDACT_SetResIdx( GFL_BBDACT_SYS* bbdActSys, u16 actIdx, GFL_BBDACT_RESUNIT_ID resUnitID, u16 resIdx );
 
#ifdef __cplusplus
} /* extern "C" */
#endif


#endif		// __BILLBOARD_ACT_H__
