//======================================================================
/**
 *
 * @file	ari_comm_def.c	
 * @brief	通信用定義郡
 * @author	ariizumi
 * @data	08.10.14
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "net/network_define.h"
#include "textprint.h"
#include "arc_def.h"

#include "ari_comm_func.h"

#ifndef ARI_COMM_DEF_H__
#define ARI_COMM_DEF_H__

//======================================================================
//	define
//======================================================================
#define FIELD_COMM_NAME_LENGTH (6)	    //終端文字込み
#define FC_ID			u16
#define FIELD_COMM_ID_MAX	( 0xFFFE )  //IDの最大値
#define FIELD_COMM_ID_INVALID	( 0xFFFF )  //IDの無効値
#define FIELD_COMM_SEARCH_PARENT_NUM (4)    //子機が親機を探して表示する数

#define IS_PARENT		(GFL_NET_IsParentMachine())
#define IS_CHILD		(!GFL_NET_IsParentMachine())	

//送受信関数用
enum FIELD_COMM_COMMAND_TYPE
{
    FC_CMD_FIRST_BEACON,    //接続直後に子→親へ送るビーコン
    FC_CMD_START_MODE,	    //フリー移動開始
    FC_CMD_PLAYER_DATA,	    //プレイヤー情報
};

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	FIELD_COMM_BEACON
//	ビーコンデータ
//======================================================================
typedef struct
{
    STRCODE name[FIELD_COMM_NAME_LENGTH];
    u16	    id;
}FIELD_COMM_BEACON;


#endif //ARI_COMM_DEF_H__
