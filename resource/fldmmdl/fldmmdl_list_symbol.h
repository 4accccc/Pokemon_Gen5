//動作モデル管理表で使用するシンボル郡
//define定義が可能です。数値は10進もしくは16進表記が有効です。
//"//"以降の文字列をコメントと判断します。

//DRAWTYPE 表示タイプで指定するシンボルです。
#define DRAWTYPE_NON 0 //表示無し
#define DRAWTYPE_BLACT 1 //ビルボードアクター
#define DRAWTYPE_MDL 2 //ポリゴンモデル

//PROC 処理関数で指定するシンボルです。
#define PROC_NON 0 //無し
#define PROC_HERO 1 //自機専用
#define PROC_BLACT 2 //汎用ビルボードアクター

//FOOTMARK 足跡種類で指定するシンボルです。
#define FOOTMARK_NON 0 //足跡無し
#define FOOTMARK_HUMAN 1 //二足の足跡
#define FOOTMARK_CYCLE 2 //自転車足跡

//MDLSIZE モデルサイズで指定するシンボルです。
#define MDLSIZE_32x32 0 //32x32

//TEXSIZE テクスチャサイズで指定するシンボルです。
#define TEXSIZE_32x1024 0 //32x1024

//ANMID アニメIDで指定するシンボルです。
#define ANMID_HERO 0 //自機専用
#define ANMID_BLACT 1 //汎用ビルボードアクター用
