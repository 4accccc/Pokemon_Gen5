
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

typedef struct
{

	u16		char_no;	//セルで使用するキャラクタNo
	u8		size_x;		//セルサイズX
	u8		size_y;		//セルサイズY
	s32		pos_x;		//セル描画X座標
	s32		pos_y;		//セル描画Y座標

	//以下、メパチ用キャラ
	u16		mepachi_char_no;	//セルで使用するキャラクタNo
	u8		mepachi_size_x;		//セルサイズX
	u8		mepachi_size_y;		//セルサイズY
	s32		mepachi_pos_x;		//セル描画X座標
	s32		mepachi_pos_y;		//セル描画Y座標

}MCSS_NCEC;

typedef struct
{
	u32			cells;		//セル枚数
	MCSS_NCEC	ncec[1];	//セル情報（可変なので、1個分だけ確保）
}MCSS_NCEC_WORK;

typedef struct
{
	void						*mcss_ncer_buf;		//ファイルデータ読み込みバッファ
	NNSG2dCellDataBank			*mcss_ncer;			//ファイルデータ読み込みバッファからncerデータを抽出したデータ
	void						*mcss_nmcr_buf;		//ファイルデータ読み込みバッファ
	NNSG2dMultiCellDataBank		*mcss_nmcr;			//ファイルデータ読み込みバッファからnmcrデータを抽出したデータ
	void						*mcss_nanr_buf;		//ファイルデータ読み込みバッファ
	NNSG2dCellAnimBankData		*mcss_nanr;			//ファイルデータ読み込みバッファからnanrデータを抽出したデータ
	void						*mcss_nmar_buf;		//ファイルデータ読み込みバッファ
	NNSG2dMultiCellAnimBankData	*mcss_nmar;			//ファイルデータ読み込みバッファからnmarデータを抽出したデータ
	MCSS_NCEC_WORK				*mcss_ncec;			//1枚の板ポリで表示するための情報が格納された独自フォーマットデータ
	NNSG2dMultiCellAnimation	mcss_mcanim;		//マルチセルアニメーションの実体
	void						*mcss_mcanim_buf;	//マルチセルアニメーションの実体の内部で使用するワーク領域
	NNSG2dImageProxy			mcss_image_proxy;	//テクスチャプロキシ
	NNSG2dImagePaletteProxy		mcss_palette_proxy;	//パレットプロキシ
	VecFx32						pos;				//マルチセルのポジション
	fx32						scale_x;
	fx32						scale_y;
	int							mepachi_flag;		//メパチフラグ（暫定）
	int							index;				//登録INDEX
	int							heapID;				//使用するヒープID
}MCSS_WORK;

typedef struct
{
	int						max;				//登録数MAX
	MCSS_WORK				**mcss;				//登録マルチセル構造体
#ifdef USE_RENDER
	NNSG2dRendererInstance	mcss_render;		//描画用 Render　
	NNSG2dRenderSurface		mcss_surface;		//メイン画面 Surface
#endif //USE_RENDER
	GFL_G3D_CAMERA			*camera;			//カメラへのポインタ
	int						texAdrs;			//テクスチャ転送開始アドレス
	int						palAdrs;			//テクスチャパレット転送開始アドレス
	int						heapID;				//使用するヒープID
}MCSS_SYS_WORK;

extern	MCSS_SYS_WORK*	MCSS_Init( int max, GFL_G3D_CAMERA *camera, HEAPID heapID );
extern	void	MCSS_Exit( MCSS_SYS_WORK *mcss_sys );
extern	void	MCSS_Main( MCSS_SYS_WORK *mcss_sys );
extern	void	MCSS_Draw( MCSS_SYS_WORK *mcss_sys );
extern	MCSS_WORK*	MCSS_Add( MCSS_SYS_WORK *mcss_sys,
					  fx32			pos_x,
					  fx32			pos_y,
					  fx32			pos_z,
					  ARCID			arcID,
					  ARCDATID		ncbr,
					  ARCDATID		nclr,
					  ARCDATID		ncer,
					  ARCDATID		nanr,
					  ARCDATID		nmcr,
					  ARCDATID		nmar,
					  ARCDATID		ncec);
extern	void	MCSS_Del( MCSS_SYS_WORK *mcss_sys, MCSS_WORK *mcss );
extern	void	MCSS_SetScaleX( MCSS_WORK *mcss, fx32 scale_x );
extern	void	MCSS_SetScaleY( MCSS_WORK *mcss, fx32 scale_y );

