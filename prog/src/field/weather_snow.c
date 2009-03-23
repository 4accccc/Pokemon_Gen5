//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_snow.c
 *	@brief		天気	雪
 *	@author		tomoya takahashi
 *	@data		2009.03.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "arc/arc_def.h"
#include "arc/field_weather.naix"


#include "weather_snow.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
//	雪管理関数定数
//=====================================
#define	WEATHER_SNOW_TIMING_MIN		(14)				// 雪を出すタイミング最小
#define WEATHER_SNOW_TIMING_MAX		(24)				// 雪を出すタイミング最大
#define WEATHER_SNOW_TIMING_ADD		(5)					// タイミングを減らす数
#define	WEATHER_SNOW_ADD_TIMING		(1)					// １度のオブジェ登録数を増やすタイミング
#define	WEATHER_SNOW_ADD			(0)					// オブジェ登録数を足す値
#define WEATHER_SNOW_MAIN			(1)					// メインではこれだけ登録

/*== フェード無し開始の時 ==*/
#define WEATHER_SNOW_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_SNOW_NOFADE_OBJ_START_DUST_NUM	( 2 )			// 何個ずつずらすか
#define	WEATHER_SNOW_NOFADE_OBJ_START_DUST_MOVE	( 24 )			// ずらして動作させる値

/*== フォグ ==*/
#define	WEATHER_SNOW_FOG_TIMING		(1)							// に１回フォグテーブルを操作
#define	WEATHER_SNOW_FOG_TIMING_END	(2)							// に１回フォグテーブルを操作
#define WEATHER_SNOW_FOG_START		(16)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_SNOW_FOG_START_END	(32)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_SNOW_FOG_OFS		(0x300)

/*== 雪オブジェクト ==*/
#define	WEATHER_SNOW_ADD_TMG_X_BASE	(4)					// 雪のスピードを足すタイミング
#define	WEATHER_SNOW_ADD_TMG_X_RAN	(2)					// 雪のスピードを足すタイミング乱数値
#define	WEATHER_SNOW_ADD_TMG_Y_BASE	(1)					// 雪のスピードを足すタイミング
#define	WEATHER_SNOW_ADD_TMG_Y_RAN	(2)					// 雪のスピードを足すタイミング乱数値

#define	WEATHER_SNOW_START_X_BASE	(-32)						// ベースになるX開始座標
#define	WEATHER_SNOW_START_X_MAX	(414)						// X開始座標乱数の最大値
#define WEATHER_SNOW_OBJ_NUM		(4)					// オブジェ種類

#define WEATHER_SNOW_OBJ_AUTODEST_TIMING_MAX ( 60 )	// 自滅タイム　最大
#define WEATHER_SNOW_OBJ_AUTODEST_TIMING_MIN ( 4 )	// 自滅タイム　最小

/*== 雪グラフィック構成データ ==*/
#define WEATHER_SNOW_GRAPHIC_CELL	( 3 )
#define WEATHER_SNOW_GRAPHIC_BG		( WEATHER_GRAPHIC_NONE )



//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	普通の雪ワーク
//=====================================
typedef struct {
	s32 work[10];
} WEATHER_SNOW_WORK;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	通常の雪
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Init( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_FadeIn( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_NoFade( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Main( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_InitFadeOut( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_FadeOut( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Exit( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID ); 
static void WEATHER_SNOW_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_SNOW_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 



//-----------------------------------------------------------------------------
/**
 *			天気データ
 */
//-----------------------------------------------------------------------------
// 通常の雪
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SNOW = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	FALSE,		// BGを使用するか？
	NARC_field_weather_snow_NCGR,			// OAM CG
	NARC_field_weather_snow_NCLR,			// OAM PLTT
	NARC_field_weather_snow_NCER,			// OAM CELL
	NARC_field_weather_snow_NANR,			// OAM CELLANM
	0,			// BG CG
	0,			// BG PLTT
	0,			// BG SCRN

	// ワークサイズ
	sizeof(WEATHER_SNOW_WORK),

	// 管理関数
	WEATHER_SNOW_Init,		// 初期化
	WEATHER_SNOW_FadeIn,		// フェードイン
	WEATHER_SNOW_NoFade,		// フェードなし
	WEATHER_SNOW_Main,		// メイン処理
	WEATHER_SNOW_InitFadeOut,	// フェードアウト
	WEATHER_SNOW_FadeOut,		// フェードアウト
	WEATHER_SNOW_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_SNOW_OBJ_Move,
};









//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	普通の雪
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	初期化動作
 *
 *	@param	p_wk			ワーク
 *	@param	fog_cont		フォグが有効か？
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Init( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID )
{
	WEATHER_SNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	
	// 雪オブジェの登録フェード初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		1,
		WEATHER_SNOW_TIMING_MAX,
		WEATHER_SNOW_MAIN,
		WEATHER_SNOW_TIMING_MIN,
		-WEATHER_SNOW_TIMING_ADD,
		WEATHER_SNOW_ADD_TIMING,
		WEATHER_SNOW_ADD,
		WEATHER_SNOW_OBJ_Add );
	
	p_local_wk->work[1] = 0;							// 雪が１回下まで行ったかのフラグ
	
	// フォグの設定
	if( fog_cont ){
		WEATHER_TASK_FogFade_Init( p_wk,
				WEATHER_FOG_SLOPE_DEFAULT, 
				WEATHER_FOG_DEPTH_DEFAULT + WEATHER_SNOW_FOG_OFS, 
				GX_RGB(26,26,26),
				WEATHER_SNOW_FOG_TIMING );
	}
	p_local_wk->work[0] = WEATHER_SNOW_FOG_START;	// 同じくフォグ用
	
	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードイン処理
 *
 *	@param	p_wk		ワーク
 *	@param	fog_cont	フォグ管理フラグ
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_FadeIn( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID )
{
	BOOL result;
	BOOL fog_result;
	WEATHER_SNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワーク6が０になったらフォグを動かす
	}else{
		
		fog_result = WEATHER_TASK_FogFade_Main( p_wk );
		
		// タイミングが最小になったらメインへ
		if( fog_result && result ){		// フェードリザルトが完了ならばメインへ
			return WEATHER_TASK_FUNC_RESULT_FINISH;
		}
	}

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードなし初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_NoFade( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID )
{
	WEATHER_SNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_SNOW_MAIN,
		WEATHER_SNOW_TIMING_MIN,
		WEATHER_SNOW_MAIN,
		WEATHER_SNOW_TIMING_MIN,
		-WEATHER_SNOW_TIMING_ADD,
		WEATHER_SNOW_ADD_TIMING,
		WEATHER_SNOW_ADD,
		WEATHER_SNOW_OBJ_Add );
	
	p_local_wk->work[1] = 0;							// 雪が１回下まで行ったかのフラグ
	p_local_wk->work[2] = 0;							// 自動破棄フラグ
	
	// フォグの設定
	if(fog_cont){
		/*
		// データを設定
		sys_w->fogFade.Fog = sys_work->pWSysCont->fsys->fog_data;		// フォグ保存先代入
		weatherSysFogParamSet( sys_w->fogFade.Fog, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_SNOW_FOG_OFS, GX_RGB(26,26,26) );

		// テーブルデータを作成して反映
		weatherSysFogSet( &sys_w->fogFade );
		//*/
	}

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_SNOW_OBJ_Add, WEATHER_SNOW_NOFADE_OBJ_START_NUM, WEATHER_SNOW_NOFADE_OBJ_START_DUST_NUM, WEATHER_SNOW_NOFADE_OBJ_START_DUST_MOVE, heapID );


	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	雪メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Main( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID )
{
	WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID );

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト開始
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_InitFadeOut( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID )
{
	WEATHER_SNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	// obj
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_SNOW_TIMING_MAX,
			WEATHER_SNOW_TIMING_ADD,
			-1 );
	// fog
	if( fog_cont ){
//		weatherSysFogFadeInit( &sys_w->fogFade, WEATHER_SNOW_FOG_TIMING_END, FALSE );
	}
	p_local_wk->work[0] = WEATHER_SNOW_FOG_START_END;	// 同じくフォグ用

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_FadeOut( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID )
{
	WEATHER_SNOW_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行
	
	// フォグ操作
	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワークが０になったらフォグを動かす
	}else{
	
		if( fog_cont ){
			fog_result = WEATHER_TASK_FogFade_Main( p_wk );
		}else{
			fog_result = TRUE;
		}
	
		if( fog_result && result ){

			// オブジェ破棄フラグON
			p_local_wk->work[2] = 1;
			
			// 登録数が０になったら終了するかチェック
			// 自分の管理するあめが全て破棄されたら終了
			if( WEATHER_TASK_GetActiveObjNum( p_wk ) == 0 ){//*/
				
				return WEATHER_TASK_FUNC_RESULT_FINISH;
			}
		}
	}

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	破棄処理
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Exit( WEATHER_TASK* p_wk, BOOL fog_cont, u32 heapID )
{
	// FOG終了
	

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	雪オブジェ動作
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_SNOW_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
	GFL_CLACTPOS pos;
	GFL_CLWK* p_clwk;
	s32*	p_local_wk;
	s32*	p_flg;
	s32*	p_exitflg;

	// ユーザワーク取得
	p_local_wk = WEATHER_OBJ_WORK_GetWork( p_wk );

	// アクターワーク取得
	p_clwk = WEATHER_OBJ_WORK_GetClWk( p_wk );

	// 全破棄要請
	p_exitflg = (s32*)p_local_wk[7];
	switch( p_local_wk[8] ){
	case 0:
		// もし　破棄フラグが立っていたら自動破棄動さへ
		if( *p_exitflg ){
			p_local_wk[8] ++;
		}
		break;

	case 1:	// ｶｳﾝﾀが0になったら破棄
		p_local_wk[9] --;
		if( p_local_wk[9] < 0 ){
			WEATHER_TASK_DeleteObj( p_wk );
			return ;
		}
		break;
	}
	

	
	GFL_CLACT_WK_GetPos( p_clwk, &pos, CLSYS_DEFREND_MAIN );

	// 動かす
	// X座標移動
	if(((p_local_wk[5] & 0xffff) >= p_local_wk[2])){
		pos.x	+= p_local_wk[1];
		p_local_wk[4]++;
		p_local_wk[5] &= 0xffff0000;
		// スピードチェンジャー
		if(p_local_wk[4] < 10){
			p_local_wk[2]--;
		}else{
			p_local_wk[2]++;
		}
		if(p_local_wk[4] >= 20){	// これで左右にゆらゆら
			p_local_wk[4] = 0;
			p_local_wk[1]*=-1;
		}//*/
	}
	// Y座標移動
	if(((p_local_wk[5] >> 16) >= p_local_wk[3])){
		pos.y += 1;
		p_local_wk[5] &= 0x0000ffff;
	} 

	// 座標設定
	GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DEFREND_MAIN );
	
	// カウント設定
	p_local_wk[6] = (p_local_wk[6] + 1) % 100;
	p_local_wk[5]++;
	p_local_wk[5]+=0x10000;

	// 破棄ゾーンにいたら破棄  
	if(((pos.y < -284) && (pos.y > -296)) ||
		((pos.y > 212) && (pos.y < 232))){
		p_flg = (s32*)(p_local_wk[0]);
		*p_flg = 1;
		WEATHER_TASK_DeleteObj( p_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェクト登録処理
 *
 *	@param	p_wk	ワーク
 *	@param	num		数
 */
//-----------------------------------------------------------------------------
static void WEATHER_SNOW_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	int rand;	// 乱数
	WEATHER_OBJ_WORK* p_addobj;		// 登録オブジェ
	WEATHER_SNOW_WORK*	p_local_wk;	// システムワーク
	s32*	p_obj_wk;	// オブジェクトワーク
	int	frame;		// フレーム
	GFL_CLWK* p_clwk;
	GFL_CLACTPOS pos;	// 座標

	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	if(p_local_wk->work[1] == 1){
		num *= 2;
	}
	
	// num分オブジェクトを登録
	for(i=0;i<num;i++){

		p_addobj = WEATHER_TASK_CreateObj( p_wk, heapID );		// 登録
		if(p_addobj == NULL){			// 失敗したら終了
			break;
		}
		p_obj_wk	= WEATHER_OBJ_WORK_GetWork( p_addobj );
		p_clwk		= WEATHER_OBJ_WORK_GetClWk( p_addobj );
		
		frame = GFUser_GetPublicRand( WEATHER_SNOW_OBJ_NUM );
		GFL_CLACT_WK_SetAnmFrame( p_clwk, frame );
		
		p_obj_wk[4] = 10;	// 横の動く方向変更カウンタ	
		p_obj_wk[5] = 0;
		p_obj_wk[6] = 0;	// 輝きカウンタ
		p_obj_wk[8] = 0;							// シーケンス
		
		rand = GFUser_GetPublicRand(0);

		p_obj_wk[9] = WEATHER_SNOW_OBJ_AUTODEST_TIMING_MIN + (rand % WEATHER_SNOW_OBJ_AUTODEST_TIMING_MAX);		// 自滅ｶｳﾝﾀ
	
		// スピード
		if((rand % 2) == 0){
			p_obj_wk[1] = 1;
		}else{
			p_obj_wk[1] = -1;
		}
		p_obj_wk[2] = WEATHER_SNOW_ADD_TMG_X_BASE + (GFUser_GetPublicRand(WEATHER_SNOW_ADD_TMG_X_RAN));
		p_obj_wk[3] = WEATHER_SNOW_ADD_TMG_Y_BASE + (GFUser_GetPublicRand(WEATHER_SNOW_ADD_TMG_Y_RAN));

		p_obj_wk[0] = (s32)&p_local_wk->work[1];		// 下まで行ったらフラグを立てるポインタを保存
		p_obj_wk[7] = (s32)&p_local_wk->work[2];		// 破棄フラグポインタON
		
		// 座標を設定
		pos.x = ( WEATHER_SNOW_START_X_BASE + (GFUser_GetPublicRand(WEATHER_SNOW_START_X_MAX)) );
		
		if((p_local_wk->work[1] == 1) &&
			(i >= (num /2)) ){	// 半分出す
			pos.y = ( -40 - GFUser_GetPublicRand(20));
		}else{
			pos.y = ( -8 - GFUser_GetPublicRand(20));
		}
		GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DEFREND_MAIN );
	}

}


