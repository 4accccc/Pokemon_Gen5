#pragma once
//このファイルはtownmap_data_convによって自動生成されています

//データ総数
#define TOWNMAP_DATA_MAX	(61)

//データエラー値
#define TOWNMAP_DATA_ERROR	(65535)

//場所のタイプ
#define TOWNMAP_PLACETYPE_TOWN	(0)
#define TOWNMAP_PLACETYPE_ROAD	(1)
#define TOWNMAP_PLACETYPE_DUNGEON	(2)
#define TOWNMAP_PLACETYPE_TOWN_SMALL	(3)
#define TOWNMAP_PLACETYPE_HIDE	(4)
#define TOWNMAP_PLACETYPE_HIDE_TOWN_S	(5)
#define TOWNMAP_ZONESEARCH_NOW_ONLY	(0)
#define TOWNMAP_ZONESEARCH_NOW_AND_ESCAPE	(1)
#define TOWNMAP_PLACETYPE_MAX	(2)

//ユーザー指定フラグ
//到着フラグや隠しマップフラグが以下の値ならばプログラムで個別制御を行わなければならない
#define TOWNMAP_USER_FLAG_LIBERTY_GARDEN_TICKET	(61440)
#define TOWNMAP_USER_FLAG_OVERSEAS_TRADE	(61441)
