//============================================================================================
/**
 * @file	mcs_resident.c
 * @brief	MCS：常駐監視
 * @author	
 * @date	
 */
//============================================================================================
#ifdef PM_DEBUG

#include "gflib.h"
#include "arc_def.h"

#include "sound/pm_sndsys.h"
#include "debug/gf_mcs.h"

#define GFL_MCS_RESIDENT_ID	(9876543)

static int count = 0;
//============================================================================================
/**
 *
 * @brief	システム起動 & 終了
 *
 */
//============================================================================================
void	GFL_MCS_Resident(void)
{
	GFL_MCS_Write(GFL_MCS_RESIDENT_ID, &count, 4);
	count++;
}

#endif

