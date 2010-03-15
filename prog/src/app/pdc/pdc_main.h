//============================================================================
/**
 *
 *  @file   pdc_main.h
 *  @brief  ポケモンドリームキャッチ
 *  @author soga
 *  @data   2010.03.12
 *
 */
//============================================================================
#pragma once

#include "app/pdc.h"

typedef struct _PDC_MAIN_WORK PDC_MAIN_WORK;

extern  PDC_MAIN_WORK*  PDC_MAIN_Init( PDC_SETUP_PARAM* psp, HEAPID heapID );
extern  PDC_RESULT      PDC_MAIN_Main( PDC_MAIN_WORK* pmw );
extern  void            PDC_MAIN_Exit( PDC_MAIN_WORK* pmw );
