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
#define FOOTMARK_HUMAN 1 //二足の足
#define FOOTMARK_CYCLE 2 //自転車足跡

//MDLSIZE モデルサイズで指定するシンボルです。
#define MDLSIZE_32x32 0 //32x32

//TEXSIZE テクスチャサイズで指定するシンボルです。
#define TEXSIZE_8x8 0 //8x8
#define TEXSIZE_16x8 1 //16x8
#define TEXSIZE_16x16 2 //16x16
#define TEXSIZE_16x32 3 //16x32
#define TEXSIZE_32x16 4 //32x16
#define TEXSIZE_32x32 5 //32x32
#define TEXSIZE_32x64 6 //32x64
#define TEXSIZE_64x32 7 //64x32
#define TEXSIZE_64x64 8 //64x64
#define TEXSIZE_64x128 9  //64x128
#define TEXSIZE_128x64 10 //128x64
#define TEXSIZE_128x128 11 //128x128
#define TEXSIZE_128x256 12 //128x256
#define TEXSIZE_256x128 13 //256x128
#define TEXSIZE_256x256 14 //256x256
#define TEXSIZE_256x512 15 //256x512
#define TEXSIZE_512x256 16 //512x256
#define TEXSIZE_512x512 17 //512x512
#define TEXSIZE_512x1024 18 //512x1024
#define TEXSIZE_1024x512 19 //1024x512
#define TEXSIZE_1024x1024 20 //1024x1024
#define TEXSIZE_32x512 21 //32x512
#define TEXSIZE_32x1024 22 //32x1024

//ANMID アニメIDで指定するシンボルです。
#define ANMID_NON 0 //アニメ無し
#define ANMID_HERO 1 //自機専用
#define ANMID_BLACT 2 //汎用ビルボードアクター用
