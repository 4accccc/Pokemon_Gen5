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
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
#include "message.naix"
#else                    //DL子機時処理

#ifdef MB_TYPE_POKE_SHIFTER
#include "message_dl.naix"
#else
#include "message_dl_movie.naix"
#endif

#endif //MULTI_BOOT_MAKE

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

//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	GlobalMsg_PokeName = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monsname_dat, heapID );
#else                    //DL子機時処理
#ifdef MB_TYPE_POKE_SHIFTER
	GlobalMsg_PokeName = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_dl_monsname_dat, heapID );
#else
	GlobalMsg_PokeName = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_dl_movie_monsname_dat, heapID );
#endif
#endif //MULTI_BOOT_MAKE
}


