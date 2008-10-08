
#ifndef NPIMPORTERDEF_H
#define NPIMPORTERDEF_H


/*
	file data contents の表示モード
	0	表示しない
	1	表示する
*/
#define _NP_DATA_MASSAGE 0
#ifdef _NP_DATA_MASSAGE
	#define dfprintf( arg ) ( fprintf arg )
#else
	#define dfprintf( arg )
#endif	/* _NP_DATA_MASSAGE */

// ファイル名の最大長
#define DEFNP_BPH_FILE_NAME_LENGTH	256
// テクスチャの最大枚数
#define	NP_BPC_TEXTURE_MAX			32

#define NP_BPC_KEYFRAME_MAX			32

#endif //NPIMPORTERDEF_H

