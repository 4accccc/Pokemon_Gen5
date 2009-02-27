
//============================================================================================
/**
 * @file	mcss_def.h
 * @brief	マルチセルソフトウエアスプライト描画用関数
 * @author	soga
 * @date	2008.11.17
 */
//============================================================================================

//--------------------------------------------------------------------------
/**
 * 定数定義
 */
//--------------------------------------------------------------------------

#define SCREEN_WIDTH		(256)			// 画面幅
#define SCREEN_HEIGHT		(192)			// 画面高さ

#define SIZE_OF_RES_POOL	(1000)			// リソースプールのサイズ

#define	DEFAULT_Z_OFFSET	(-0x100)		//NNS_G2dSetRendererSpriteZoffsetに渡すセル描画毎に更新するZ値

#define	DOT_LENGTH			(FX32_HALF >> 5)	//セルデータ1ドットに対するポリゴンの長さの基準

//--------------------------------------------------------------------------
/**
 * 構造体定義
 */
//--------------------------------------------------------------------------
struct _MCSS_NCEC
{

	fx32	pos_x;		//セル描画X座標
	fx32	pos_y;		//セル描画Y座標
	fx32	size_x;		//セルサイズX
	fx32	size_y;		//セルサイズY
	fx32	tex_s;		//テクスチャs値
	fx32	tex_t;		//テクスチャt値

	//以下、メパチ用キャラ
	fx32	mepachi_pos_x;		//セル描画X座標
	fx32	mepachi_pos_y;		//セル描画Y座標
	fx32	mepachi_size_x;		//セルサイズX
	fx32	mepachi_size_y;		//セルサイズY
	fx32	mepachi_tex_s;		//テクスチャs値
	fx32	mepachi_tex_t;		//テクスチャt値
};

struct _MCSS_NCEC_WORK
{
	u32			cells;		//セル枚数
	MCSS_NCEC	ncec[1];	//セル情報（可変なので、1個分だけ確保）
};

struct _MCSS_WORK
{
	void						*mcss_ncer_buf;			//ファイルデータ読み込みバッファ
	NNSG2dCellDataBank			*mcss_ncer;				//ファイルデータ読み込みバッファからncerデータを抽出したデータ
	void						*mcss_nmcr_buf;			//ファイルデータ読み込みバッファ
	NNSG2dMultiCellDataBank		*mcss_nmcr;				//ファイルデータ読み込みバッファからnmcrデータを抽出したデータ
	void						*mcss_nanr_buf;			//ファイルデータ読み込みバッファ
	NNSG2dCellAnimBankData		*mcss_nanr;				//ファイルデータ読み込みバッファからnanrデータを抽出したデータ
	void						*mcss_nmar_buf;			//ファイルデータ読み込みバッファ
	NNSG2dMultiCellAnimBankData	*mcss_nmar;				//ファイルデータ読み込みバッファからnmarデータを抽出したデータ
	MCSS_NCEC_WORK				*mcss_ncec;				//1枚の板ポリで表示するための情報が格納された独自フォーマットデータ
	NNSG2dMultiCellAnimation	mcss_mcanim;			//マルチセルアニメーションの実体
	void						*mcss_mcanim_buf;		//マルチセルアニメーションの実体の内部で使用するワーク領域
	NNSG2dImageProxy			mcss_image_proxy;		//テクスチャプロキシ
	NNSG2dImagePaletteProxy		mcss_palette_proxy;		//パレットプロキシ
	VecFx32						pos;					//マルチセルのポジション
	VecFx32						scale;					//マルチセルのスケール
	VecFx32						shadow_scale;			//影のスケール
	u32							mepachi_flag	:1;		//メパチフラグ
	u32							anm_stop_flag	:1;		//アニメストップフラグ
	u32							vanish_flag		:1;		//バニッシュフラグ
	u32											:29;
	int							index;					//登録INDEX
	int							heapID;					//使用するヒープID
};

struct _MCSS_SYS_WORK
{
	int						mcss_max;			//登録数MAX
	MCSS_WORK				**mcss;				//登録マルチセル構造体
#ifdef USE_RENDER
	NNSG2dRendererInstance	mcss_render;		//描画用 Render　
	NNSG2dRenderSurface		mcss_surface;		//メイン画面 Surface
#endif //USE_RENDER
	u32						mcss_ortho_mode	:1;	//正射影描画モードフラグ
	u32										:31;
	int						texAdrs;			//テクスチャ転送開始アドレス
	int						palAdrs;			//テクスチャパレット転送開始アドレス
	int						heapID;				//使用するヒープID
};

