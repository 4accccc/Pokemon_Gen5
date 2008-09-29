#include "field_g3d_mapper.h"
#include "field_player.h"			//PC_ACT_FUNC

typedef struct _DEPEND_FUNCTIONS DEPEND_FUNCTIONS;

typedef struct {
	//横ブロック数, 縦ブロック数, ブロック１辺の幅, グラフィックアーカイブＩＤ, 実マップデータ
	FLD_G3D_MAPPER_RESIST	mapperData;
	VecFx32				startPos;	//開始位置
	const DEPEND_FUNCTIONS * dep_funcs;
}SCENE_DATA;

extern const DEPEND_FUNCTIONS FieldBaseFunctions;
extern const DEPEND_FUNCTIONS FieldNoGridFunctions;
extern const DEPEND_FUNCTIONS FieldGridFunctions;
extern const SCENE_DATA	resistMapTbl[];
extern const unsigned int resistMapTblCount;
