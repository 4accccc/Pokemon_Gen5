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
#define FIELD_COMM_NAME_LENGTH (6)

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
