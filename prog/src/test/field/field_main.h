#include "field_g3d_mapper.h"

typedef struct {
	//横ブロック数, 縦ブロック数, ブロック１辺の幅, グラフィックアーカイブＩＤ, 実マップデータ
	FLD_G3D_MAPPER_RESIST	mapperData;
	VecFx32				startPos;	//開始位置
	const char * mapname;			//マップ名
}SCENE_DATA;

extern const SCENE_DATA	resistMapTbl[];
extern const unsigned int resistMapTblCount;
