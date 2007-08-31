//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		clact.h
 *	@brief		セルアクターシステム
 *	@author		tomoya takahashi
 *	@data		2006.11.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __CLACT_H__
#define __CLACT_H__


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	CLSYS描画面タイプ
//	レンダラーのサーフェース設定や、
//	Vramアドレス指定に使用します。
//=====================================
typedef enum {
	CLSYS_DRAW_MAIN,// メイン画面
	CLSYS_DRAW_SUB,	// サブ画面
	CLSYS_DRAW_MAX,	// タイプ最大数
} CLSYS_DRAW_TYPE;


//-------------------------------------
///	NNSG2dRendererAffineTypeOverwiteModeこの型名は長いので変更
//	セルアトリビュートのアフィン変換フラグを上書きするフラグ
//=====================================
typedef NNSG2dRendererAffineTypeOverwiteMode	CLSYS_AFFINETYPE;
enum{
	CLSYS_AFFINETYPE_NONE	= NNS_G2D_RND_AFFINE_OVERWRITE_NONE,  // 上書きしない
	CLSYS_AFFINETYPE_NORMAL = NNS_G2D_RND_AFFINE_OVERWRITE_NORMAL,// 通常のアフィン変換方式に設定
	CLSYS_AFFINETYPE_DOUBLE = NNS_G2D_RND_AFFINE_OVERWRITE_DOUBLE,// 倍角アフィン変換方式に設定
	CLSYS_AFFINETYPE_NUM
};

//-------------------------------------
///	NNSG2dAnimationPlayMode	この型名は長いので変更
//	セルアニメーションのアニメ方式を上書指定するフラグ
//=====================================
typedef NNSG2dAnimationPlayMode		CLSYS_ANM_PLAYMODE;
enum{
	CLSYS_ANMPM_INVALID		= NNS_G2D_ANIMATIONPLAYMODE_INVALID,		// 無効
	CLSYS_ANMPM_FORWARD		= NNS_G2D_ANIMATIONPLAYMODE_FORWARD,        // ワンタイム再生(順方向)
	CLSYS_ANMPM_FORWARD_L	= NNS_G2D_ANIMATIONPLAYMODE_FORWARD_LOOP,   // リピート再生(順方向ループ)
	CLSYS_ANMPM_REVERSE		= NNS_G2D_ANIMATIONPLAYMODE_REVERSE,        // 往復再生(リバース（順＋逆方向）
	CLSYS_ANMPM_REVERSE_L	= NNS_G2D_ANIMATIONPLAYMODE_REVERSE_LOOP,   // 往復再生リピート（リバース（順＋逆順方向） ループ）
	CLSYS_ANMPM_MAX
};


//-------------------------------------
///	フリップタイプ
//	GFL_CLWKへのフリップ設定に使用します。
//=====================================
typedef enum {
	CLWK_FLIP_V,		// Vフリップ状態
	CLWK_FLIP_H,		// Hフリップ状態
	CLWK_FLIP_MAX
} CLWK_FLIP_TYPE;

//-------------------------------------
///	座標タイプ
//	座標の個別設定を行うときに使用します。
//=====================================
typedef enum{
	CLSYS_MAT_X,			// X座標
	CLSYS_MAT_Y,			// Y座標
	CLSYS_MAT_MAX
} CLSYS_MAT_TYPE;

//-------------------------------------
///	デフォルト設定レンダラー内サーフェース指定定数
//=====================================
typedef enum{
	CLSYS_DEFREND_MAIN,
	CLSYS_DEFREND_SUB,
	CLSYS_DEFREND_NUM
} CLSYS_DEFREND_TYPE;


//-------------------------------------
///	設定サーフェース　絶対座標指定
//	GFL_CLWKの座標設定関数で絶対座標を設定するときに使用します。
//=====================================
#define CLWK_SETSF_NONE	(0xffff)

//-------------------------------------
///	Vramアドレス取得失敗
//	GFL_CLACT_WK_GetPlttAddr
//	GFL_CLACT_WK_GetCharAddr
//	の戻り値
//=====================================
#define CLWK_VRAM_ADDR_NONE	( 0xffffffff )


//-------------------------------------
///	ユーザー拡張アトリビュート取得
//	データがないときの戻り値
//	GFL_CLACT_WK_GetPlttAddr
//	GFL_CLACT_WK_GetCharAddr
//	の戻り値
//=====================================
#define CLWK_USERATTR_NONE	(0)	// ユーザー拡張アトリビュートなし



//-----------------------------------------------------------------------------
/**
 *					外部非公開システム構造体
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	ユーザー定義レンダラー
//=====================================
typedef struct _CLSYS_REND	GFL_CLSYS_REND;

//-------------------------------------
///	セルアクターユニット
//=====================================
typedef struct _CLUNIT		GFL_CLUNIT;

//-------------------------------------
///	セルアクターワーク
//=====================================
typedef struct _CLWK		GFL_CLWK;





//-----------------------------------------------------------------------------
/**
 *					初期化データ構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	CLSYS初期化データ
//=====================================
typedef struct {
	s16 surface_main_left;	// メイン　サーフェースの左上座標
	s16 surface_main_top;	// メイン　サーフェースの左上座標
	s16 surface_sub_left;	// サブ　サーフェースの左上座標
	s16 surface_sub_top;	// サブ　サーフェースの左上座標
	u8	oamst_main;			// メイン画面OAM管理開始位置
	u8	oamnum_main;		// メイン画面OAM管理数
	u8	oamst_sub;			// サブ画面OAM管理開始位置
	u8	oamnum_sub;			// サブ画面OAM管理数
	u8	tr_cell;			// セルVram転送管理数
} GFL_CLSYS_INIT;


//-------------------------------------
///	独自レンダラー作成用
/// サーフェースデータ構造体
//=====================================
typedef struct {
	s16	lefttop_x;			// サーフェース左上ｘ座標
	s16	lefttop_y;			// サーフェース左上ｙ座標
	s16	width;				// サーフェース幅
	s16	height;				// サーフェース高さ
	CLSYS_DRAW_TYPE	type;	// サーフェースタイプ(CLSYS_DRAW_TYPE)
} GFL_REND_SURFACE_INIT;

//-------------------------------------
///	GFL_CLWK初期化リソースデータ
//	それぞれのアニメ方法のデータを設定する関数も用意されています。
//	・セルアニメ			GFL_CLACT_WK_SetCellResData(...)
//	・Vram転送セルアニメ	GFL_CLACT_WK_SetTrCellResData(...)
//	・マルチセルアニメ		GFL_CLACT_WK_SetMCellResData(...)
//=====================================
typedef struct {
	const NNSG2dImageProxy*			cp_img;		// イメージプロクシ
	const NNSG2dImagePaletteProxy*	cp_pltt;	// パレットプロクシ
	NNSG2dCellDataBank*				p_cell;		// セルデータ
    const NNSG2dCellAnimBankData*   cp_canm;	// セルアニメーション

	// 以下は必要なときだけ値を入れる
	const NNSG2dMultiCellDataBank*  cp_mcell;	// マルチセルデータ		（無いときNULL）
    const NNSG2dMultiCellAnimBankData* cp_mcanm;// マルチセルアニメーション（無いときNULL）
	const NNSG2dCharacterData*		cp_char;	// Vram転送セルアニメ以外はNULL
} GFL_CLWK_RES;


//-------------------------------------
///	GFL_CLWK初期化基本データ
//=====================================
typedef struct {
	s16	pos_x;				// ｘ座標
	s16 pos_y;				// ｙ座標
	u16 anmseq;				// アニメーションシーケンス
	u8	softpri;			// ソフト優先順位	0>0xff
	u8	bgpri;				// BG優先順位
} GFL_CLWK_DATA;

//-------------------------------------
///	GFL_CLWK初期化アフィン変換データ
//=====================================
typedef struct {
	GFL_CLWK_DATA clwkdata;		// 基本データ
	
	s16	affinepos_x;		// アフィンｘ座標
	s16 affinepos_y;		// アフィンｙ座標
	fx32 scale_x;			// 拡大ｘ値
	fx32 scale_y;			// 拡大ｙ値
	u16	rotation;			// 回転角度(0～0xffff 0xffffが360度)
	u16	affine_type;		// 上書きアフィンタイプ（CLSYS_AFFINETYPE）
} GFL_CLWK_AFFINEDATA;


//-------------------------------------
///	GFL_CLWK　座標構造体	
//=====================================
typedef struct {
	s16 x;
	s16 y;
} GFL_CLACTPOS;

//-------------------------------------
///	GFL_CLWK　スケール構造体
//=====================================
typedef struct {
	fx32 x;
	fx32 y;
} GFL_CLSCALE;





//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	CLSYS関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム　初期化
 *
 *	@param	cp_data		初期化データ
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_Init( const GFL_CLSYS_INIT* cp_data, HEAPID heapID );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム	破棄
 *
 *	@param	none
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_Exit( void );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム　メイン処理
 *
 *	*全セルアクターユニットの描画が終わった後に呼ぶ必要があります。
 *	*メインループの最後に呼ぶようにしておくと確実だと思います。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_Main( void );

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム　Vブランク処理
 *
 * 長いですが、すみません。呼んでください↓
 *
 * [セルアクターシステム　Vブランク期間内での転送処理　説明]
 *	＞用途に合わせて２つのタイプを選べる
 *　　・転送処理方法には２つのタイプがあります。
 *		1.	GFL_CLACT_VBlankFunc
 *		2.	GFL_CLACT_VBlankFuncTransOnly ＋　GFL_CLACT_ClearOamBuff
 *	　・1　GFL_CLACT_VBlankFunc
 *		OAMデータ転送後、バッファをクリーンします。
 *	　　処理落ちなどで、１ループ中に２回割り込みが入ると何も表示されなく
 *	　　なってしまいますので、割り込み内で使用する事が出来ません。
 *
 *	  ・2　GFL_CLACT_VBlankFuncTransOnly ＋　GFL_CLACT_ClearOamBuff
 *		GFL_CLACT_VBlankFuncTransOnlyはOAMバッファのデータの転送のみ行いますので、
 *		この関数をVBlank割り込み内で呼んでください。
 *		CLACT_UNITの描画前にGFL_CLACT_ClearOamBuffを呼ぶことでOAMバッファをクリアする
 *		事が出来ます。
 *
 *	　＞GFL_CLACT_ClearOamBuffについて
 *		・セルアクターシステム内にはoammanclearフラグというものを持っています。
 *		　これは、OamBuffをクリアしてよいかを判別するフラグです。
 *		　フラグが立っている状態のときのみGFL_CLACT_ClearOamBuffを
 *		　実行することが出来ます。
 *		　『oammanclearフラグの動作』
 *			GFL_CLACT_VBlankFuncTransOnlyが呼ばれると立ちます。
 *			GFL_CLACT_ClearOamBuffが呼ばれると落ちます。
 *		　これはCLACT_UNITが自由にGFL_CLACT_ClearOamBuffを呼べるようにするため
 *		　行っています。
 *		・GFL_CLACT_VBlankFuncの関数ではoammanclearフラグが立たないので、
 *		　GFL_CLACT_ClearOamBuffは機能しません。
 *			
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_VBlankFunc( void );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターOAMバッファのクリーン処理
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_ClearOamBuff( void );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム	Vブランク処理	転送のみ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_VBlankFuncTransOnly( void );

//-------------------------------------
///	USER定義レンダラー関係
// 自分独自のサーフェース設定をした
// レンダラーを使用したいとき、この関数郡で作成して、
// セルアクターユニットに関連付けることが出来ます。
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムを作成する
 *
 *	@param	cp_data			ユーザー定義サーフェースデータ
 *	@param	data_num		データ数
 *	@param	heapID			ヒープID
 *
 *	@return	作成したレンダラーシステム
 */
//-----------------------------------------------------------------------------
extern GFL_CLSYS_REND* GFL_CLACT_USERREND_Create( const GFL_REND_SURFACE_INIT* cp_data, u32 data_num, HEAPID heapID );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムを破棄する
 *
 *	@param	p_rend			ユーザー定義サーフェースデータ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_Delete( GFL_CLSYS_REND* p_rend );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムにサーフェース左上座標を設定する
 *
 *	@param	p_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	cp_pos			設定する座標
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_SetSurfacePos( GFL_CLSYS_REND* p_rend, u32 idx, const GFL_CLACTPOS* cp_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムからサーフェース左上座標を取得する
 *
 *	@param	cp_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	p_pos			座標格納先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_GetSurfacePos( const GFL_CLSYS_REND* cp_rend, u32 idx, GFL_CLACTPOS* p_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムにサーフェースサイズを設定する
 *
 *	@param	p_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	cp_size			サイズ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_SetSurfaceSize( GFL_CLSYS_REND* p_rend, u32 idx, const GFL_CLACTPOS* cp_size );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムからサーフェースサイズを取得する
 *
 *	@param	cp_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	p_size			サイズ取得先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_GetSurfaceSize( const GFL_CLSYS_REND* cp_rend, u32 idx, GFL_CLACTPOS* p_size );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムにサーフェースタイプを設定する
 *
 *	@param	p_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	type			サーフェースタイプ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_SetSurfaceType( GFL_CLSYS_REND* p_rend, u32 idx, CLSYS_DRAW_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムからサーフェースタイプを取得する
 *
 *	@param	cp_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *
 *	@return	サーフェースタイプ	（CLSYS_DRAW_TYPE）
 */
//-----------------------------------------------------------------------------
extern CLSYS_DRAW_TYPE GFL_CLACT_USERREND_GetSurfaceType( const GFL_CLSYS_REND* cp_rend, u32 idx );

//-------------------------------------
///	GFL_CLUNIT関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットを生成
 *
 *	@param	wknum		ワーク数
 *	@param	heapID		ヒープID
 *
 *	@return	ユニットポインタ
 */
//-----------------------------------------------------------------------------
extern GFL_CLUNIT* GFL_CLACT_UNIT_Create( u16 wknum, HEAPID heapID );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットを破棄
 *
 *	@param	p_unit			セルアクターユニット
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_UNIT_Delete( GFL_CLUNIT* p_unit );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニット　描画処理
 *
 *	@param	p_unit			セルアクターユニット
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_UNIT_Draw( GFL_CLUNIT* p_unit );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットに描画フラグを設定
 *	
 *	@param	p_unit		セルアクターユニット
 *	@param	on_off		描画ON-OFF
 *
 *	TRUE	表示
 *	FALSE	非表示
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_UNIT_SetDrawFlag( GFL_CLUNIT* p_unit, BOOL on_off );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットの描画フラグを取得
 *
 *	@param	cp_unit		セルアクターユニット
 *
 *	@retval	TRUE	表示
 *	@retval	FALSE	非表示
 */
//-----------------------------------------------------------------------------
extern BOOL GFL_CLACT_UNIT_GetDrawFlag( const GFL_CLUNIT* cp_unit );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットにユーザー独自のレンダラーシステムを設定
 *
 *	@param	p_unit		セルアクターユニット
 *	@param	p_rend		ユーザー独自のレンダラー
 *
 *	この関数で、好きな設定をしたレンダラーを使用することが出来るようになります。
 *	＞ユーザー独自のレンダラーを作成する
 *		GFL_CLACT_USERREND_～関数で作成できます。
 *
 *  ＞注意事項
 *	　　座標設定するときに渡すsetsfの値は、ユーザー独自レンダラー
 *	　　に登録したサーフェースのインデックス番号となります。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_UNIT_SetUserRend( GFL_CLUNIT* p_unit, GFL_CLSYS_REND* p_rend );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットにデフォルトレンダラーシステムを設定
 *
 *	@param	p_unit		セルアクターユニット
 *
 *	この関数で、独自レンダラーシステムからデフォルトレンダラーシステムを
 *	使用するように戻すことが出来ます。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_UNIT_SetDefaultRend( GFL_CLUNIT* p_unit );

//-------------------------------------
///	GFL_CLWK関係
//
/*	【setsfの説明】
 *		GFL_CLUNITの使用するレンダラーシステムを変更していないときは
 *		CLSYS_DEFREND_TYPEの値を指定する
 *		・CLSYS_DEFREND_MAIN指定時	pos_x/yがメイン画面左上座標からの相対座標になる。
 *		・CLSYS_DEFREND_SUB指定時	pos_x/yがサブ画面左上座標からの相対座標になる。
 *		
 *		独自のレンダラーシステム(GFL_CLSYS_REND)をGFL_CLUNITに設定しているときは、
 *		サーフェースの要素数を指定することで、
 *		指定されたサーフェース左上座標からの相対座標になる。
 *
 *		通常/独自レンダラーシステム共通で、
 *		CLWK_SETSF_NONEを指定すると絶対座標設定になる
 */
//=====================================
// 初期化リソースデータ設定関数
//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメ用リソースデータ作成
 *
 *	@param	p_res			リソースデータ格納先
 *	@param	cp_img			イメージプロクシ
 *	@param	cp_pltt			パレットプロクシ
 *	@param	p_cell			セルデータバンク
 *	@param	cp_canm			セルアニメーションデータバンク
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetCellResData( GFL_CLWK_RES* p_res, const NNSG2dImageProxy* cp_img, const NNSG2dImagePaletteProxy* cp_pltt, NNSG2dCellDataBank* p_cell, const NNSG2dCellAnimBankData* cp_canm );
//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーション用リソースデータ作成
 *
 *	@param	p_res		リソースデータ格納先
 *	@param	cp_img		イメージプロクシ
 *	@param	cp_pltt     パレットプロクシ
 *	@param	p_cell      セルデータバンク
 *	@param	cp_canm     セルアニメーションデータバンク
 *	@param	cp_char     キャラクタデータ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetTrCellResData( GFL_CLWK_RES* p_res, const NNSG2dImageProxy* cp_img, const NNSG2dImagePaletteProxy* cp_pltt, NNSG2dCellDataBank* p_cell, const NNSG2dCellAnimBankData* cp_canm, const NNSG2dCharacterData* cp_char );
//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーション　リソースデータ作成
 *
 *	@param	p_res			リソースデータ格納先
 *	@param	cp_img			イメージプロクシ
 *	@param	cp_pltt         パレットプロクシ
 *	@param	p_cell          セルデータバンク
 *	@param	cp_canm         セルアニメーションデータバンク
 *	@param	cp_mcell		マルチセルデータバンク
 *	@param	cp_mcanm		マルチセルアニメーションデータバンク
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetMCellResData( GFL_CLWK_RES* p_res, const NNSG2dImageProxy* cp_img, const NNSG2dImagePaletteProxy* cp_pltt, NNSG2dCellDataBank* p_cell, const NNSG2dCellAnimBankData* cp_canm, const NNSG2dMultiCellDataBank* cp_mcell, const NNSG2dMultiCellAnimBankData* cp_mcanm );

// 登録破棄関係
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターの登録
 *
 *	@param	p_unit			セルアクターユニット
 *	@param	cp_data			セルアクターデータ
 *	@param	cp_res			セルアクターリソース
 *	@param	setsf			設定サーフェース
 *	@param	heapID			ヒープID
 *
 *	@return	登録したセルアクターワーク
 *
 *	【setsfの説明】
 *		GFL_CLUNITの使用するレンダラーシステムを変更していないときは
 *		CLSYS_DEFREND_TYPEの値を指定する
 *		・CLSYS_DEFREND_MAIN指定時	pos_x/yがメイン画面左上座標からの相対座標になる。
 *		・CLSYS_DEFREND_SUB指定時	pos_x/yがサブ画面左上座標からの相対座標になる。
 *		
 *		独自のレンダラーシステムをGFL_CLUNITに設定しているときは、
 *		サーフェースの要素数を指定することで、
 *		指定されたサーフェース左上座標からの相対座標になる。
 *
 *		通常/独自レンダラーシステム共通で、
 *		CLWK_SETSF_NONEを指定すると絶対座標設定になる
 */
//-----------------------------------------------------------------------------
extern GFL_CLWK* GFL_CLACT_WK_Add( GFL_CLUNIT* p_unit, const GFL_CLWK_DATA* cp_data, const GFL_CLWK_RES* cp_res, u16 setsf, HEAPID heapID );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターの登録	アフィン変換バージョン
 *
 *	@param	p_unit			セルアクターユニット
 *	@param	cp_data			セルアクターデータ　アフィンバージョン
 *	@param	cp_res			セルアクターリソース
 *	@param	setsf			サーフェースインデックス
 *	@param	heapID			ヒープID
 *
 *	@return	登録したセルアクターワーク
 */
//-----------------------------------------------------------------------------
extern GFL_CLWK* GFL_CLACT_WK_AddAffine( GFL_CLUNIT* p_unit, const GFL_CLWK_AFFINEDATA* cp_data, const GFL_CLWK_RES* cp_res, u16 setsf, HEAPID heapID );
//----------------------------------------------------------------------------
/**
 *	@brief	破棄処理
 *
 *	@param	p_wk	破棄するセルアクターワーク
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_Remove( GFL_CLWK* p_wk );

// ワークに対する操作
//----------------------------------------------------------------------------
/**
 *	@brief	表示フラグ設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	on_off			表示ON-OFF
 *	TRUE	表示
 *	FALSE	非表示
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetDrawFlag( GFL_CLWK* p_wk, BOOL on_off );
//----------------------------------------------------------------------------
/**
 *	@brief	表示フラグ取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@retval	TRUE	表示
 *	@retval	FALSE	非表示
 */
//-----------------------------------------------------------------------------
extern BOOL GFL_CLACT_WK_GetDrawFlag( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_pos			座標構造体
 *	@param	setsf			サーフェースインデックス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetPos( GFL_CLWK* p_wk, const GFL_CLACTPOS* cp_pos, u16 setsf );
//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_pos			座標格納先
 *	@param	setsf			サーフェースインデックス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetPos( const GFL_CLWK* cp_wk, GFL_CLACTPOS* p_pos, u16 setsf );
//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標個別設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	pos			設定値
 *	@param	setsf		設定サーフェース
 *	@param	type		座標タイプ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetTypePos( GFL_CLWK* p_wk, s16 pos, u16 setsf, CLSYS_MAT_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標個別取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	setsf		設定サーフェース
 *	@param	type		座標タイプ
 *
 *	@return	座標タイプのサーフェース内相対座標
 */
//-----------------------------------------------------------------------------
extern s16 GFL_CLACT_WK_GetTypePos( const GFL_CLWK* cp_wk, u16 setsf, CLSYS_MAT_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標設定
 *
 *	@param	p_wk			セルアクターワーク	
 *	@param	cp_pos			座標
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetWldPos( GFL_CLWK* p_wk, const GFL_CLACTPOS* cp_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_pos			座標格納先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetWldPos( const GFL_CLWK* cp_wk, GFL_CLACTPOS* p_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標個別設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	pos				設定値
 *	@param	type			座標タイプ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetWldTypePos( GFL_CLWK* p_wk, s16 pos, CLSYS_MAT_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標個別取得
 *	
 *	@param	cp_wk			セルアクターワーク
 *	@param	type			座標タイプ
 *
 *	@return	座標タイプの絶対座標
 */
//-----------------------------------------------------------------------------
extern s16 GFL_CLACT_WK_GetWldTypePos( const GFL_CLWK* cp_wk, CLSYS_MAT_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	アフィンパラメータ設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	affine		アフィンパラメータ
 *	affine
 *		CLSYS_AFFINETYPE_NONE	上書きしない
 *		CLSYS_AFFINETYPE_NORMAL	通常のアフィン変換方式に設定
 *		CLSYS_AFFINETYPE_DOUBLE	倍角アフィン変換方式に設定
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAffineParam( GFL_CLWK* p_wk, CLSYS_AFFINETYPE affine );
//----------------------------------------------------------------------------
/**
 *	@brief	アフィンパラメータ取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	CLSYS_AFFINETYPE_NONE	上書きしない
 *	@retval	CLSYS_AFFINETYPE_NORMAL	通常のアフィン変換方式に設定
 *	@retval	CLSYS_AFFINETYPE_DOUBLE	倍角アフィン変換方式に設定
 */
//-----------------------------------------------------------------------------
extern CLSYS_AFFINETYPE GFL_CLACTWkGetAffineParam( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標一括設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	cp_pos		アフィン変換相対座標
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAffinePos( GFL_CLWK* p_wk, const GFL_CLACTPOS* cp_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標一括取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	p_pos		アフィン変換相対座標取得先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetAffinePos( const GFL_CLWK* cp_wk, GFL_CLACTPOS* p_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標個別設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	pos			設定値
 *	@param	type		座標タイプ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetTypeAffinePos( GFL_CLWK* p_wk, s16 pos, CLSYS_MAT_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標個別取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	type		座標タイプ
 *
 *	@return	座標タイプのアフィン変換相対座標
 */
//-----------------------------------------------------------------------------
extern s16 GFL_CLACT_WK_GetTypeAffinePos( const GFL_CLWK* cp_wk, CLSYS_MAT_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	拡大縮小値一括設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_sca			拡大縮小値
 *
 *	*AffineParamに
 *		CLSYS_AFFINETYPE_NORMAL		通常
 *		CLSYS_AFFINETYPE_DOUBLE		倍角
 *	が設定されていないと描画に反映されません。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetScale( GFL_CLWK* p_wk, const GFL_CLSCALE* cp_sca );
//----------------------------------------------------------------------------
/**
 *	@brief	拡大縮小値一括取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_sca			拡大縮小値取得先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetScale( const GFL_CLWK* cp_wk, GFL_CLSCALE* p_sca );
//----------------------------------------------------------------------------
/**
 *	@brief	拡大縮小値個別設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	scale			設定値
 *	@param	type			座標タイプ
 *
 *	*AffineParamに
 *		CLSYS_AFFINETYPE_NORMAL		通常
 *		CLSYS_AFFINETYPE_DOUBLE		倍角
 *	が設定されていないと描画に反映されません。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetTypeScale( GFL_CLWK* p_wk, fx32 scale, CLSYS_MAT_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	拡大縮小値個別取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	type			座標タイプ
 *
 *	@return	座標タイプの拡大縮小値
 */
//-----------------------------------------------------------------------------
extern fx32 GFL_CLACT_WK_GetTypeScale( const GFL_CLWK* cp_wk, CLSYS_MAT_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	回転角度設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	rotation		回転角度(0～0xffff 0xffffが360度)
 *
 *	*AffineParamに
 *		CLSYS_AFFINETYPE_NORMAL		通常
 *		CLSYS_AFFINETYPE_DOUBLE		倍角
 *	が設定されていないと描画に反映されません。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetRotation( GFL_CLWK* p_wk, u16 rotation );
//----------------------------------------------------------------------------
/**
 *	@brief	回転角度を取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@return	回転角度(0～0xffff 0xffffが360度)
 */
//-----------------------------------------------------------------------------
extern u16  GFL_CLACT_WK_GetRotation( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief		フリップ設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	flip_type		設定フリップタイプ
 *	@param	on_off			ONOFFフラグ	TRUE:On	FALSE:Off
 *
 *	*AffineParamがNNS_G2D_RND_AFFINE_OVERWRITE_NONEになっていないと反映されません
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetFlip( GFL_CLWK* p_wk, CLWK_FLIP_TYPE flip_type, BOOL on_off );
//----------------------------------------------------------------------------
/**
 *	@brief	フリップ設定を取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	flip_type		フリップタイプ
 *
 *	@retval	TRUE	フリップ設定
 *	@retval	FALSE	フリップ未設定
 */
//-----------------------------------------------------------------------------
extern BOOL GFL_CLACT_WK_GetFlip( const GFL_CLWK* cp_wk, CLWK_FLIP_TYPE flip_type );
//----------------------------------------------------------------------------
/**
 *	@brief		オブジェモード設定
 *	
 *	@param	p_wk		セルアクターワーク
 *	@param	mode		オブジェモード
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetObjMode( GFL_CLWK* p_wk, GXOamMode mode );
//----------------------------------------------------------------------------
/**
 *	@brief	オブジェクトモード取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	GX_OAM_MODE_NORMAL		ノーマルOBJ
 *	@retval	GX_OAM_MODE_XLU			半透明OBJ 
 *	@retval	GX_OAM_MODE_OBJWND		OBJウィンドウ
 *	@retval	GX_OAM_MODE_BITMAPOBJ	ビットマップOBJ
 */
//-----------------------------------------------------------------------------
extern GXOamMode GFL_CLACT_WK_GetObjMode( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	モザイクフラグ設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	on_off		モザイクON－OFF	TRUE:On	FALSE:Off
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetMosaic( GFL_CLWK* p_wk, BOOL on_off );
//----------------------------------------------------------------------------
/**
 *	@brief	モザイクフラグ取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	TRUE	モザイク設定
 *	@retval	FALSE	モザイク未設定
 */
//-----------------------------------------------------------------------------
extern BOOL GFL_CLACT_WK_GetMosaic( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	パレットオフセット設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	pal_offs	パレットオフセット
 *
 *	パレットプロクシアドレス+pal_offs+OamAttr.c_paramの
 *	パレット番号を使用するようになります。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetPlttOffs( GFL_CLWK* p_wk, u8 pal_offs );
//----------------------------------------------------------------------------
/**
 *	@brief	パレットオフセット取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	pal_offs	パレットオフセット
 */
//-----------------------------------------------------------------------------
extern u8 GFL_CLACT_WK_GetPlttOffs( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	パレットプロクシのアドレス取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	type		描画タイプ
 *
 *	@retval	CLWK_VRAM_ADDR_NONE以外	パレットVramアドレス
 *	@retval	CLWK_VRAM_ADDR_NONE		読み込まれていない
 */
//-----------------------------------------------------------------------------
extern u32 GFL_CLACT_WK_GetPlttAddr( const GFL_CLWK* cp_wk, CLSYS_DRAW_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	新しいパレットプロクシを設定する
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_pltt			設定するパレットプロクシ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetPlttProxy( GFL_CLWK* p_wk, const NNSG2dImagePaletteProxy* cp_pltt );
//----------------------------------------------------------------------------
/**
 *	@brief	パレットプロクシデータの取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_pltt			パレットプロクシ格納先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetPlttProxy( const GFL_CLWK* cp_wk,  NNSG2dImagePaletteProxy* p_pltt );
//----------------------------------------------------------------------------
/**
 *	@brief	キャラクタデータVramアドレスを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	type		描画タイプ
 *
 *	@retval	CLWK_VRAM_ADDR_NONE以外	キャラクタVramアドレス
 *	@retval	CLWK_VRAM_ADDR_NONE		読み込まれていない
 */
//-----------------------------------------------------------------------------
extern u32 GFL_CLACT_WK_GetCharAddr( const GFL_CLWK* cp_wk, CLSYS_DRAW_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	イメージプロクシを設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_img			設定イメージプロクシ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetImgProxy( GFL_CLWK* p_wk, const NNSG2dImageProxy* cp_img );
//----------------------------------------------------------------------------
/**
 *	@brief	イメージプロクシを取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_img			イメージプロクシ格納先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetImgProxy( const GFL_CLWK* cp_wk,  NNSG2dImageProxy* p_img );
//----------------------------------------------------------------------------
/**
 *	@brief	ソフト優先順位の設定
 *	
 *	@param	p_wk			セルアクターワーク
 *	@param	pri				ソフト優先順位	0>0xff
 *	
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetSoftPri( GFL_CLWK* p_wk, u8 pri );
//----------------------------------------------------------------------------
/**
 *	@brief	ソフト優先順位の取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@return	ソフト優先順位
 */
//-----------------------------------------------------------------------------
extern u8 GFL_CLACT_WK_GetSoftPri( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	BG優先順位の設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	pri				BG優先順位
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetBgPri( GFL_CLWK* p_wk, u8 pri );
//----------------------------------------------------------------------------
/**
 *	@brief	BG優先順位を取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	BG優先順位
 */
//-----------------------------------------------------------------------------
extern u8 GFL_CLACT_WK_GetBgPri( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを変更
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	anmseq		アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmSeq( GFL_CLWK* p_wk, u16 anmseq );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
extern u16 GFL_CLACT_WK_GetAnmSeq( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスの数を取得
 *	
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションシーケンスの数
 */
//-----------------------------------------------------------------------------
extern u16 GFL_CLACT_WK_GetAnmSeqNum( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスが変わっていたら変更する
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	anmseq		アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmSeqDiff( GFL_CLWK* p_wk, u16 anmseq );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを変更する　
 *			その際フレーム時間などのリセットを行いません。　MultiCell非対応
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	anmseq		アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmSeqNoReset( GFL_CLWK* p_wk, u16 anmseq );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレームを設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	idx			フレーム数
 *	NitroCharacterのアニメーションシーケンス内のコマ番号を指定してください
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmFrame( GFL_CLWK* p_wk, u16 idx );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレームを進める
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	speed		進めるアニメーションスピード
 *
 *	*逆再生も可能です
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_AddAnmFrame( GFL_CLWK* p_wk, fx32 speed );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレーム数を取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションフレーム数
 */
//-----------------------------------------------------------------------------
extern u16 GFL_CLACT_WK_GetAnmFrame( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションフラグ設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	on_off		オートアニメーションOn-Off	TRUE:On	FALSE:Off
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAutoAnmFlag( GFL_CLWK* p_wk, BOOL on_off );
//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションフラグを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	TRUE	オートアニメON
 *	@retval	FALSE	オートアニメOFF
 */
//-----------------------------------------------------------------------------
extern BOOL GFL_CLACT_WK_GetAutoAnmFlag( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションスピード設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	speed		アニメーションスピード		
 *
 *	*逆再生も可能です
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAutoAnmSpeed( GFL_CLWK* p_wk, fx32 speed );
//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションスピードを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	オートアニメーションスピード
 */
//-----------------------------------------------------------------------------
extern fx32 GFL_CLACT_WK_GetAutoAnmSpeed( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション開始
 *
 *	@param	p_wk		セルアクターワーク
 *	GFL_CLACT_WK_StopAnmでとめたアニメーションを開始させます。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_StartAnm( GFL_CLWK* p_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション停止
 *
 *	@param	p_wk		セルアクターワーク
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_StopAnm( GFL_CLWK* p_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターワーク	アニメーション動作チェック
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	TRUE	再生中
 *	@retval	FALSE	停止中
 */
//-----------------------------------------------------------------------------
extern BOOL GFL_CLACT_WK_CheckAnmActive( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションリスタート
 *
 *	@param	p_wk	セルアクターワーク
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_ResetAnm( GFL_CLWK* p_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	上書きアニメーション動作モード設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	playmode	動作モード
 *
 *	playmode
 *	CLSYS_ANMPM_INVALID		無効（これを設定するとNitroCharacterで設定したのアニメ方式になります）
 *	CLSYS_ANMPM_FORWARD		ワンタイム再生(順方向)
 *	CLSYS_ANMPM_FORWARD_L	リピート再生(順方向ループ)
 *	CLSYS_ANMPM_REVERSE		往復再生(リバース（順＋逆方向）
 *	CLSYS_ANMPM_REVERSE_L	往復再生リピート（リバース（順＋逆順方向） ループ）
 *		
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmMode( GFL_CLWK* p_wk, CLSYS_ANM_PLAYMODE playmode );
//----------------------------------------------------------------------------
/**
 *	@brief	上書きアニメーション方式を取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	CLSYS_ANMPM_INVALID		無効(NitroCharacterで設定したアニメ方式でアニメしています)
 *	@retval	CLSYS_ANMPM_FORWARD		ワンタイム再生(順方向)
 *	@retval	CLSYS_ANMPM_FORWARD_L	リピート再生(順方向ループ)
 *	@retval	CLSYS_ANMPM_REVERSE		往復再生(リバース（順＋逆方向）
 *	@retval	CLSYS_ANMPM_REVERSE_L	往復再生リピート（リバース（順＋逆順方向） ループ）
 */
//-----------------------------------------------------------------------------
extern CLSYS_ANM_PLAYMODE GFL_CLACT_WK_GetAnmMode( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	seq			シーケンスナンバー
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
extern u32 GFL_CLACT_WK_GetUserAttrAnmSeq( const GFL_CLWK* cp_wk, u32 seq );
//----------------------------------------------------------------------------
/**
 *	@brief	今のアニメーションシーケンスのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
extern u32 GFL_CLACT_WK_GetUserAttrAnmSeqNow( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレームのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	seq			シーケンスナンバー
 *	@param	frame		フレームナンバー
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
extern u32 GFL_CLACT_WK_GetUserAttrAnmFrame( const GFL_CLWK* cp_wk, u32 seq, u32 frame );
//----------------------------------------------------------------------------
/**
 *	@brief	今のアニメーションフレームのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
extern u32 GFL_CLACT_WK_GetUserAttrAnmFrameNow( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	セルのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	cellidx		セルインデックス
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
extern u32 GFL_CLACT_WK_GetUserAttrCell( const GFL_CLWK* cp_wk, u32 cellidx );

#endif		// __CLACT_H__

