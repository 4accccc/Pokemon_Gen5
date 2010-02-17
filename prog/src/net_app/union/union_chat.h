//==============================================================================
/**
 * @file    union_chat.h
 * @brief   チャットのヘッダ
 * @author  matsuda
 * @date    2009.08.26(水)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void UnionChat_AddChat(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *bpc, const PMS_DATA *mine_pmsdata, UNION_CHAT_TYPE mine_chat_type);
extern UNION_CHAT_DATA * UnionChat_GetSpaceBuffer(UNION_CHAT_LOG *log);
extern UNION_CHAT_DATA * UnionChat_GetReadBuffer(UNION_CHAT_LOG *log, u32 log_no);
