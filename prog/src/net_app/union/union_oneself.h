//==============================================================================
/**
 * @file    union_oneself.h
 * @brief   ユニオン：自分動作のヘッダ
 * @author  matsuda
 * @date    2009.07.10(金)
 */
//==============================================================================
#pragma once

//==============================================================================
//  外部関数宣言
//==============================================================================
extern BOOL UnionOneself_ReqStatus(UNION_SYSTEM_PTR unisys, int req_status);
extern int UnionOneself_Update(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork);

