//============================================================================================
/**
 * @file	gf_standard.c
 * @brief	‚b•W€ŠÖ”
 * @author	GAME FREAK inc.
 * @date	2006.11.29
 */
//============================================================================================
#include "gflib.h"
#include "gf_standard.h"

//----------------------------------------------------------------------------
/**
 *	@brief	ƒƒ‚ƒŠ—Ìˆæ‚ğ”äŠr‚·‚é
 *	@param	s1	”äŠrƒƒ‚ƒŠ[1
 *	@param	s2	”äŠrƒƒ‚ƒŠ[2
 *	@param	n	”äŠrƒTƒCƒY
 *	@retval	‚O   “¯‚¶
 *	@retval	I‚O ˆÙ‚È‚é
 */
//-----------------------------------------------------------------------------
int GFL_STD_MemComp(const void *s1, const void *s2, const u32 size)
{
    const unsigned char  *p1 = (const unsigned char *)s1;
    const unsigned char  *p2 = (const unsigned char *)s2;
    u32 n = size;

    while (n-- > 0) {
        if (*p1 != *p2)
            return (*p1 - *p2);
        p1++;
        p2++;
    }
    return (0);
}

