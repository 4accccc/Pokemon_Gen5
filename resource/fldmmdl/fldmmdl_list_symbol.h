//動作モデル管理表で使用するシンボル郡
//define定義が可能です。数値は10進もしくは16進表記が有効です。
//行間"//"以降の文字列をコメントと判断します。

//DRAWTYPE 表示タイプで指定するシンボルです。
#define DRAWTYPE_NON 0 //表示無し
#define DRAWTYPE_BLACT 1 //ビルボードアクター
#define DRAWTYPE_MDL 2 //ポリゴンモデル

//PROC 処理関数で指定するシンボルです。
#define PROC_NON 0 //無し
#define PROC_HERO 1 //自機専用
#define PROC_BLACT 2 //汎用ビルボードアクター
#define PROC_CYCLEHERO 3 //自転車自機専用
#define PROC_SWIMHERO 4 //波乗り自機専用
#define PROC_BLACT_ALANM16 5 //汎用ビルボードアクター常に16フレーム移動アニメ
#define PROC_HERO_ITEMGET 6 //自機アイテムゲット
#define PROC_HERO_PCAZUKE 7 //自機PC預け
#define PROC_BLACT_ONELOOP 8 //一パターンループアニメ
#define PROC_PCWOMAN 9 //PC姉
#define PROC_POKE 10 //連れ歩きポケモン
#define PROC_POKE_FLY 11 //連れ歩きポケモン 浮遊演出
#define PROC_MODEL 12 //ポリゴンモデル
#define PROC_FISHINGHERO 13 //釣り自機専用
#define PROC_YUREHERO 14 //尾瀬揺れ自機
#define PROC_BLACT_ALANM32 15 //汎用ビルボードアクター常に32フレーム移動アニメ

//FOOTMARK 足跡種類で指定するシンボルです。
#define FOOTMARK_NON 0 //足跡無し
#define FOOTMARK_HUMAN 1 //二足の足
#define FOOTMARK_CYCLE 2 //自転車足跡

//MDLSIZE モデルサイズで指定するシンボルです。
#define MDLSIZE_32x32 0 //32x32
#define MDLSIZE_16x16 1 //16x16
#define MDLSIZE_64x64 2 //64x64

//TEXSIZE テクスチャサイズで指定するシンボルです。
#define TEXSIZE_8x8 0
#define TEXSIZE_8x16 1
#define TEXSIZE_8x32 2
#define TEXSIZE_8x64 3
#define TEXSIZE_8x128 4
#define TEXSIZE_8x256 5
#define TEXSIZE_8x512 6
#define TEXSIZE_8x1024 7
#define TEXSIZE_16x8 8
#define TEXSIZE_16x16 9
#define TEXSIZE_16x32 10
#define TEXSIZE_16x64 11
#define TEXSIZE_16x128 12
#define TEXSIZE_16x256 13
#define TEXSIZE_16x512 14
#define TEXSIZE_16x1024 15
#define TEXSIZE_32x8 16
#define TEXSIZE_32x16 17
#define TEXSIZE_32x32 18
#define TEXSIZE_32x64 19
#define TEXSIZE_32x128 20
#define TEXSIZE_32x256 21
#define TEXSIZE_32x512 22
#define TEXSIZE_32x1024 23
#define TEXSIZE_64x8 24 
#define TEXSIZE_64x16 25
#define TEXSIZE_64x32 26
#define TEXSIZE_64x64 27
#define TEXSIZE_64x128 28
#define TEXSIZE_64x256 29
#define TEXSIZE_64x512 30
#define TEXSIZE_64x1024 31
#define TEXSIZE_128x8 32
#define TEXSIZE_128x16 33
#define TEXSIZE_128x32 34
#define TEXSIZE_128x64 35
#define TEXSIZE_128x128 36
#define TEXSIZE_128x256 37
#define TEXSIZE_128x512 38
#define TEXSIZE_128x1024 39
#define TEXSIZE_256x8 40
#define TEXSIZE_256x16 41
#define TEXSIZE_256x32 42
#define TEXSIZE_256x64 43
#define TEXSIZE_256x128 44
#define TEXSIZE_256x256 45
#define TEXSIZE_256x512 46
#define TEXSIZE_256x1024 47
#define TEXSIZE_512x8 48
#define TEXSIZE_512x16 49
#define TEXSIZE_512x32 50
#define TEXSIZE_512x64 51
#define TEXSIZE_512x128 52
#define TEXSIZE_512x256 53
#define TEXSIZE_512x512 54
#define TEXSIZE_512x1024 55
#define TEXSIZE_1024x8 56
#define TEXSIZE_1024x16 57
#define TEXSIZE_1024x32 58
#define TEXSIZE_1024x64 59
#define TEXSIZE_1024x128 60
#define TEXSIZE_1024x256 61
#define TEXSIZE_1024x512 62
#define TEXSIZE_1024x1024 63

//ANMID アニメIDで指定するシンボルです。
#define ANMID_NON 0 //アニメ無し
#define ANMID_HERO 1 //自機専用
#define ANMID_BLACT 2 //汎用ビルボードアクター用 旧
#define ANMID_CYCLEHERO_OLD 3 //自転車自機専用 旧
#define ANMID_SWIMHERO 4 //波乗り自機専用
#define ANMID_BLACT_FLIP 5 //人物汎用 反転使用
#define ANMID_BLACT_NONFLIP 6 //人物汎用 反転未使用
#define ANMID_CYCLEHERO 7 //自転車自機専用
#define ANMID_ONECELL 8 //一パターン絵専用
#define ANMID_ITEMGET 9 //自機アイテムゲット
#define ANMID_PCAZUKE 10 //自機ポケセン預け
#define ANMID_SAVE 11 //自機レポート
#define ANMID_PCWOMAN 12 //PC姉
#define ANMID_POKE_FLIP 13 //ポケモン　反転使用
#define ANMID_POKE_NONFLIP 14 //ポケモン　反転未使用
#define ANMID_FISHINGHERO 15 //自機釣り
#define ANMID_CUTINHERO 16 //カットイン自機
#define ANMID_YUREHERO 17 //尾瀬揺れ自機

//SEX 性別で指定するシンボルです。
#define SEX_MALE 0
#define SEX_FEMALE 1
#define SEX_NON 2
