//============================================================================
/**
 *
 *	@file		pmsi_search.h
 *	@brief
 *	@author		hosaka genya
 *	@data		2009.11.14
 *
 */
//============================================================================
#pragma once

typedef struct _PMS_INPUT_SEARCH PMS_INPUT_SEARCH;

//=============================================================================
/**
 *								íËêîíËã`
 */
//=============================================================================
enum
{ 
  PMS_INPUTWORD_MAX = 7,  ///< ì¸óÕÇ≈Ç´ÇÈç≈ëÂï∂éöêî
};

//=============================================================================
/**
 *								ç\ë¢ëÃíËã`
 */
//=============================================================================

//=============================================================================
/**
 *								EXTERNêÈåæ
 */
//=============================================================================
PMS_INPUT_SEARCH* PMSI_SEARCH_Create( const PMS_INPUT_WORK* mwk, const PMS_INPUT_DATA* dwk, HEAPID heap_id );
extern void PMSI_SEARCH_Delete( PMS_INPUT_SEARCH* wk );
extern void PMSI_SEARCH_AddWord( PMS_INPUT_SEARCH* wk, u16 word_code );
extern BOOL PMSI_SEARCH_DelWord( PMS_INPUT_SEARCH* wk );
extern u8 PMSI_SEARCH_GetCharNum( const PMS_INPUT_SEARCH* wk );
extern void PMSI_SEARCH_ClearWord( PMS_INPUT_SEARCH* wk );
extern void PMSI_SEARCH_GetInputWord( PMS_INPUT_SEARCH* wk, STRBUF* out_buf );
extern BOOL PMSI_SEARCH_Start( PMS_INPUT_SEARCH*wk );
extern u16 PMSI_SEARCH_GetResultCount( PMS_INPUT_SEARCH* wk );
extern void PMSI_SEARCH_GetResultString( PMS_INPUT_SEARCH* wk, u8 result_idx, STRBUF* dst_buf );
extern PMS_WORD PMSI_SEARCH_GetWordCode( PMS_INPUT_SEARCH* wk, u32 cur_pos );


