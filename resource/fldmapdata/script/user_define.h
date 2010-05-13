//======================================================================
/**
 * @file  user_define.h
 * @author  GAMEFREAK inc.
 * @date  2009.11.11
 */
//======================================================================

#pragma once


//======================================================================
//  最初の3体識別用定義
//======================================================================
#define FIRST_POKETYPE_KUSA   ( 0 )	//草タイプを選んだ
#define FIRST_POKETYPE_HONOO	( 1 )	//炎タイプを選んだ
#define FIRST_POKETYPE_MIZU   ( 2 )	//水タイプを選んだ

//======================================================================
//  マップ書き換えの指定ID
//======================================================================
#define MAPREPLACE_ID_MUSEUM    MAPREPLACE_ID_01    /* 博物館 */
#define MAPREPLACE_ID_CHAMPROOM MAPREPLACE_ID_02    /* チャンピオン部屋 */
#define MAPREPLACE_ID_WFBC      MAPREPLACE_ID_03    /* WF/BC制御用 */
#define MAPREPLACE_ID_NCASTLE   MAPREPLACE_ID_04    /* Nの城の謁見の間 */
#define MAPREPLACE_ID_DUN08     MAPREPLACE_ID_05    /* りゅうらせんの塔 屋外 */
#define MAPREPLACE_ID_DUN04     MAPREPLACE_ID_06    /* 古代の城 */
//#define MAPREPLACE_ID_07  未使用
//#define MAPREPLACE_ID_08  未使用
//#define MAPREPLACE_ID_09  未使用
//#define MAPREPLACE_ID_10  未使用

//======================================================================
//  BMPメニューの表示位置用定義	※所持金表示もこの定義を使います
//======================================================================

//100220 ３月には右上表示へ。プログラム側で指定位置が右上基準になったら値を変更します。
#define MENU_X1   ( 31 )	//画面左上に表示する場合
#define MENU_Y1   ( 1 )	//画面左上に表示する場合
#define MENU_X2   ( 31 )	//所持金表示の下にメニューを表示する場合
#define MENU_Y2   ( 5 )	//所持金表示の下にメニューを表示する場合


//======================================================================
//	アイテムゲットポーズのタイミング用定義
//======================================================================
#define	ITEM_GET_FORM_TIMING		( 4 )

//======================================================================
//	ビッグスタジアム／リトルコートの種目定義名
//======================================================================
#define	SPORTS_EVENT_NONE			( 0 )	//初期化状態
#define	SPORTS_EVENT_BASEBALL		( 1 )
#define	SPORTS_EVENT_SOCCER			( 2 )
#define	SPORTS_EVENT_FOOTBALL		( 3 )
#define	SPORTS_EVENT_TENNIS			( 4 )
#define	SPORTS_EVENT_BASKETBALL		( 5 )

//======================================================================
//	カナワタウン：転車台に配置される車両の定義名
//======================================================================
#define	T05_TRAIN_NONE			( 0 )	//車両なし
#define	T05_TRAIN_SINGLE		( 1 )	//シングル
#define	T05_TRAIN_S_SINGLE		( 2 )	//スーパーシングル
#define	T05_TRAIN_DOUBLE		( 3 )	//ダブル
#define	T05_TRAIN_S_DOUBLE		( 4 )	//スーパーダブル
#define	T05_TRAIN_MULTI			( 5 )	//マルチ
#define	T05_TRAIN_S_MULTI		( 6 )	//スーパーマルチ
#define	T05_TRAIN_WIFI			( 7 )	//Ｗｉ－Ｆｉ
#define	T05_TRAIN_LOCAL			( 8 )	//カナワ行き
#define	T05_TRAIN_OLD			( 9 )	//レトロトレイン
#define	T05_TRAIN_NEW			( 10 )	//ニュートレイン

//======================================================================
//	ＢＧＭフェード速度用の定義名 ※60=1秒
//======================================================================
#define	BGM_FADE_VSHORT			( 6 )
#define	BGM_FADE_SHORT			( 30 )
#define	BGM_FADE_MIDDLE			( 60 )
#define	BGM_FADE_LONG			( 90 )
#define	BGM_FADE_VLONG			( 120 )

//======================================================================
//	電光掲示板のカメラ速度
//======================================================================
#define	ELBOARD_CAMERA_FRAME	( 36 )


