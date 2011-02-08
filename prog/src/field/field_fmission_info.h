//===============================================================
/*
 *  @file   field_fmission_info.h
 *  @brief  フェスミッション　フィールド情報表示システム
 *  @author Miyuki Iwasawa
 *  @date   11.02.08
 */
//===============================================================

#pragma once

typedef struct _FIELD_FMISSION_INFO FIELD_FMISSION_INFO;


extern FIELD_FMISSION_INFO* FIELD_FMISSION_INFO_Create( GAMESYS_WORK* gsys, HEAPID heapID );
extern void FIELD_FMISSION_INFO_Delete( FIELD_FMISSION_INFO* wk );

extern void FIELD_FMISSION_INFO_Process( FIELD_FMISSION_INFO* wk );
extern void FIELD_FMISSION_INFO_Draw( FIELD_FMISSION_INFO* wk );


