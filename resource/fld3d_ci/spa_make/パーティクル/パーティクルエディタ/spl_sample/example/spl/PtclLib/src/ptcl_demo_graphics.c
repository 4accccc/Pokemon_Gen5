/*---------------------------------------------
  Utility program codes for particle demo .
  system.
 
 $Log: ptcl_demo_graphics.c,v $
 Revision 1.3  2005/02/18 04:15:16  okane_shinji
 VRAM¥…Õ˝Ω§¿µ

 Revision 1.2  2004/12/27 04:32:20  okane_shinji
 (none)

 Revision 1.1  2004/09/15 01:23:09  okane_shinji
 èâä˙ìoò^

 
 *---------------------------------------------*/

#include<nnsys.h>
#include"ptcl_demo.h"

/*-----------------------------------------------------------------
  Name       : PtclDemo_DrawAxis(void)
  Description: ç¿ïWé≤Çï`âÊÇµÇ‹Ç∑ÅB
  Returns    : Ç»Çµ
 *-----------------------------------------------------------------*/
void PtclDemo_DrawAxis(void)
{
    G3_Scale( FX32_ONE * 10, FX32_ONE * 10, FX32_ONE * 10 );

    G3_Begin(GX_BEGIN_TRIANGLES);
        G3_Color( GX_RGB( 31, 0, 0 ) );
        G3_Vtx10( 0, 0, 0 );
        G3_Vtx10( 0, 0, 0 );
        G3_Vtx10( FX16_ONE, 0, 0 );
    G3_End() ;
    G3_Begin(GX_BEGIN_TRIANGLES);
        G3_Color( GX_RGB( 0, 31, 0 ) );
        G3_Vtx10( 0, 0, 0 );
        G3_Vtx10( 0, 0, 0 );
        G3_Vtx10( 0, FX16_ONE, 0 );
    G3_End() ;
    G3_Begin(GX_BEGIN_TRIANGLES);
        G3_Color( GX_RGB( 0, 0, 31 ) );
        G3_Vtx10( 0, 0, 0 );
        G3_Vtx10( 0, 0, 0 );
        G3_Vtx10( 0, 0, FX16_ONE );
    G3_End() ;
}

/*-----------------------------------------------------------------
  Name       : PtclDemo_InitScreen(void)
  Description: ï`âÊÉXÉeÅ[ÉgÇèâä˙âªÇµÇ‹Ç∑ÅB
  Returns    : Ç»Çµ
 *-----------------------------------------------------------------*/
void PtclDemo_InitScreen(void)
{
//  G3X_Init();                                            // initialize the 3D graphics states
    G3X_InitMtxStack();                                    // initialize the matrix stack
    GX_SetBankForTex(GX_VRAM_TEX_012_ABC);               // VRAM-ABCD for texture images
    GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0123_E);          // VRAM-E for texture palettes
    GX_SetBankForBG(GX_VRAM_BG_128_D);                      // VRAM-G for BGs
    GX_SetBankForOBJ(GX_VRAM_OBJ_32_FG);                    // VRAM-F for OBJs
    
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,               // graphics mode
                       GX_BGMODE_0,                        // BGMODE is 4
                       GX_BG0_AS_3D);                      // BG #0 is for 3D

    GX_SetVisiblePlane( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_OBJ );
    
    GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);
    
    GX_SetBGCharOffset(GX_BGCHAROFFSET_0x00000);
    GX_SetBGScrOffset ( GX_BGSCROFFSET_0x00000);

    G2_SetBG1Control(
        GX_BG_SCRSIZE_TEXT_256x256,
        GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x3800,
        GX_BG_CHARBASE_0x00000,
        GX_BG_EXTPLTT_01);

    G2_SetBG0Priority(1);
    G2_SetBG1Priority(0);

    G3X_SetShading(GX_SHADING_TOON);                       // shading mode is toon
    G3X_AntiAlias(TRUE);                                   // enable antialias(without additional computing costs)
    G2_BlendNone();                                        // no 2D alpha blending or brightness change

    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

    G3X_SetClearColor( GX_RGB( 0, 0, 0), 0, 0x7fff, 63, FALSE );

    // ÉâÉCÉgÇOÇÃê›íË
    G3_LightVector(GX_LIGHTID_0, 0, 0, -FX32_ONE + 1);
    G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));

    G3X_AlphaTest(FALSE, 0);                   // ÉAÉãÉtÉ@ÉeÉXÉgÅ@Å@ÉIÉt
    G3X_AlphaBlend(TRUE);                      // ÉAÉãÉtÉ@ÉuÉåÉìÉhÅ@ÉIÉì

    G3_ViewPort(0, 0, 255, 191);

    G3_Perspective( FX32_SIN30, FX32_COS30, FX32_ONE * 4 / 3, FX32_ONE * 1, FX32_ONE * 5000, NULL );

    GX_DispOn();
    GXS_DispOn();
}

/*-----------------------------------------------------------------
  Name       : PtclDemo_Lookat( fx32 fromX, fx32 fromY, fx32 fromZ, fx32 atX, fx32 atY, fx32 atZ, fx32 upX, fx32 upY, fx32 upZ, MtxFx43 * mtx )
  Description: ÉrÉÖÅ[ïœä∑ÇÇ®Ç±Ç»Ç¢Ç‹Ç∑ÅB
  Returns    : Ç»Çµ
 *-----------------------------------------------------------------*/
void PtclDemo_Lookat( fx32 fromX, fx32 fromY, fx32 fromZ, fx32 atX, fx32 atY, fx32 atZ, fx32 upX, fx32 upY, fx32 upZ, MtxFx43 * mtx )
{
    VecFx32 Eye ;
    VecFx32 at  ;       
    VecFx32 vUp ;    // Up

    Eye.x = fromX ;
    Eye.y = fromY ;     
    Eye.z = fromZ ;
    at.x  = atX ;
    at.y  = atY ;     
    at.z  = atZ ;
    vUp.x = upX ;
    vUp.y = upY ;     
    vUp.z = upZ ;

    if( mtx != NULL ){
        G3_LookAt(&Eye, &vUp, &at, mtx);
    }else{
        G3_LookAt(&Eye, &vUp, &at, NULL);
    }

}

/*-----------------------------------------------------------------
  Name       : PtclDemo_LoadNCLR
  Description: ÉpÉåÉbÉgÉfÅ[É^Çì«Ç›çûÇ›Ç‹Ç∑
  Returns    : Ç»Çµ
 *-----------------------------------------------------------------*/
void* PtclDemo_LoadNCLR( NNSG2dPaletteData** ppPltData, const char* pFname )
{
    void* pFile = PtclDemo_LoadFile( pFname ) ;

    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedPaletteData( pFile, ppPltData ) )
        {
            NNS_G2dPrintPaletteData( *ppPltData );
            return pFile;
        }

        OS_Free( pFile );
    }
    return NULL ;
}

/*-----------------------------------------------------------------
  Name       : PtclDemo_LoadNCGR
  Description: ÉLÉÉÉâÉNÉ^ÉfÅ[É^Çì«Ç›çûÇ›Ç‹Ç∑
  Returns    : Ç»Çµ
 *-----------------------------------------------------------------*/
void* PtclDemo_LoadNCGR( NNSG2dCharacterData** ppCharData, const char* pFname )
{
    void* pFile = PtclDemo_LoadFile( pFname ) ;

    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedCharacterData( pFile, ppCharData ) )
        {
            NNS_G2dPrintCharacterData( *ppCharData );
            return pFile;
        }

        OS_Free( pFile );
    }
    return NULL ;
}

/*-----------------------------------------------------------------
  Name       : PtclDemo_LoadNCER
  Description: ÉZÉãÉfÅ[É^Çì«Ç›çûÇ›Ç‹Ç∑
  Returns    : Ç»Çµ
 *-----------------------------------------------------------------*/
void* PtclDemo_LoadNCER( NNSG2dCellDataBank** ppCellBank, const char* pFname )
{
    void* pFile = PtclDemo_LoadFile( pFname ) ;

    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedCellBank( pFile, ppCellBank ) )
        {
            NNS_G2dPrintCellBank( *ppCellBank );
            return pFile;
        }

        OS_Free( pFile );
    }

    return NULL ;
}

/*-----------------------------------------------------------------
  Name       : PtclDemo_LoadNSCR
  Description: ÉXÉNÉäÅ[ÉìÉfÅ[É^Çì«Ç›çûÇ›Ç‹Ç∑
  Returns    : Ç»Çµ
 *-----------------------------------------------------------------*/
void* PtclDemo_LoadNSCR( NNSG2dScreenData** ppScrData, const char* pFname )
{
    void* pFile = PtclDemo_LoadFile( pFname ) ;

    if( pFile != NULL )
    {
        if( NNS_G2dGetUnpackedScreenData( pFile, ppScrData ) )
        {
            NNS_G2dPrintScreenData( *ppScrData );
            return pFile;
        }

        OS_Free( pFile );
    }
    return NULL ;
}




// end of file