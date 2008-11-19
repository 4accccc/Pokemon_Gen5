//============================================================================================
/**
 * @file	global_msg.c
 * @brief	グローバルメッセージデータ
 * @author	taya
 * @date	2008.11.19
 */
//============================================================================================
#include <assert.h>
#include <heapsys.h>

#include "print/global_msg.h"

#include "arc_def.h"
#include "message.naix"


//--------------------------------------------------------------
/**
 *	グローバルメッセージデータ
 */
//--------------------------------------------------------------
const GFL_MSGDATA* GlobalMsg_PokeName = NULL;


//=============================================================================================
/**
 * グローバルメッセージデータハンドラを生成（プログラム起動後に１度だけ呼ぶ）
 *
 * @param   heapID		生成用ヒープID
 */
//=============================================================================================
void GLOBALMSG_Init( HEAPID heapID )
{
	GF_ASSERT(GlobalMsg_PokeName == NULL);

	GlobalMsg_PokeName = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monsname_dat, heapID );
}


