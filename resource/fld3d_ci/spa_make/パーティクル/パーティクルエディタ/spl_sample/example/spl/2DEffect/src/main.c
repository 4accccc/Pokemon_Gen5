/*-------------------------------------------
 * SPL library ƒTƒ“ƒvƒ‹ "2DEffect"
 *
 * SPLƒGƒtƒFƒNƒg‚ğ2D OBJ‚ğ‚à‚¿‚¢‚Ä•`‰æ‚·‚éƒTƒ“ƒvƒ‹‚Å‚·
 *
 * (c)2004 Nintendo
 * 
 * 
 * $Log: main.c,v $
 * Revision 1.3  2005/02/18 04:15:16  okane_shinji
 * VRAM´ÉÍı½¤Àµ
 *
 * Revision 1.2  2004/12/28 02:42:35  okane_shinji
 * (none)
 *
 * Revision 1.1  2004/12/28 02:20:06  okane_shinji
 * ‰”Å“o˜^
 *
 * Revision 1.5  2004/12/27 04:33:50  okane_shinji
 * (none)
 *
 * Revision 1.4  2004/12/27 04:31:37  okane_shinji
 * ‰“o˜^
 *
 * 
 *-------------------------------------------*/


#include<nnsys.h>

#include"ptcl_demo.h"
#include"spl.h"

// ƒp[ƒeƒBƒNƒ‹ŠÖŒW
static SPLManager *spl_manager = NULL  ;
static SPLEmitter *spl_emitter[4] ;
static VecFx32     init_pos    = {0,0,0} ;
void              *resPtcl ;

// 2DŠÖŒW
static NNSG2dCellDataBank      *pCellBank; // ƒZƒ‹ƒoƒ“ƒN‚Ö‚Ìƒ|ƒCƒ“ƒ^
static NNSG2dOamManagerInstance sObjOamManager;  // OBJ o—Í—p OAM ƒ}ƒl[ƒWƒƒ
static NNSG2dImageProxy         sImageProxy;    // Cell —pƒLƒƒƒ‰ƒNƒ^/ƒeƒNƒXƒ`ƒƒƒvƒƒLƒV
static NNSG2dImagePaletteProxy  sPaletteProxy;  // Cell —pƒpƒŒƒbƒgƒvƒƒLƒV

#define NUM_OF_OAM                  128                     // OAM ƒ}ƒl[ƒWƒƒ‚ÉŠ„‚è“–‚Ä‚é OAM ‚Ì”
#define NUM_OF_AFFINE               ( NUM_OF_OAM / 4 )        // OAM ƒ}ƒl[ƒWƒƒ‚ÉŠ„‚è“–‚Ä‚é Affine ƒpƒ‰ƒ[ƒ^‚Ì”

// Œ»İ•\¦’†‚ÌƒGƒ~ƒbƒ^
static u16 suCurrentEmitter = 0 ;

// ƒXƒP[ƒ‹
static fx32 sScale[16] =
{
    FX_F32_TO_FX32( 1.0f ),
    FX_F32_TO_FX32( 1.1f ),
    FX_F32_TO_FX32( 1.3f ),
    FX_F32_TO_FX32( 1.6f ),
    FX_F32_TO_FX32( 2.0f ),
    FX_F32_TO_FX32( 2.5f ),
    FX_F32_TO_FX32( 3.1f ),
    FX_F32_TO_FX32( 3.8f ),
    FX_F32_TO_FX32( 4.6f ),
    FX_F32_TO_FX32( 5.5f ),
    FX_F32_TO_FX32( 6.0f ),
    FX_F32_TO_FX32( 7.1f ),
    FX_F32_TO_FX32( 8.2f ),
    FX_F32_TO_FX32( 9.5f ),
    FX_F32_TO_FX32( 10.9f ),
    FX_F32_TO_FX32( 13.4f ),

} ;


/*---------------------------------------------------
 * ‰Šú‰»
 *-------------------------------------------------*/
static void init()
{
    NNSG2dCharacterData *pCharData;
    NNSG2dPaletteData   *pPlttData;
    u16 i ;

    void *pBuf ;

    // Šeí‰Šú‰»
    PtclDemo_InitSystem() ;
    PtclDemo_InitVRAM() ;
    PtclDemo_InitMemory() ;
    PtclDemo_InitScreen() ;

    // VRAMƒ}ƒl[ƒWƒƒ‚ğƒZƒbƒg
    NNS_GfdInitFrmTexVramManager(3, TRUE);
    NNS_GfdInitFrmPlttVramManager(0x8000, TRUE);

    // G2D‚ğ‰Šú‰»
    NNS_G2dInitOamManagerModule() ;
    NNS_G2dInitImageProxy( &sImageProxy );
    NNS_G2dInitImagePaletteProxy( &sPaletteProxy );
    (void)NNS_G2dGetNewOamManagerInstance( &sObjOamManager, 0, NUM_OF_OAM -1 , 0, NUM_OF_AFFINE - 1, NNS_G2D_OAMTYPE_MAIN ) ;

    //////////////////////////////////////////////////
    // CELL‚ğ“Ç‚İ‚İ

    // ƒZƒ‹ƒf[ƒ^“Ç‚İ‚İ
    (void)PtclDemo_LoadNCER( &pCellBank, "data/Cell_sample.NCER" );

    // 2Dƒf[ƒ^“Ç‚İ‚İ
    pBuf = PtclDemo_LoadNCGR( &pCharData, "data/Character_sample.NCGR" );
    NNS_G2dLoadImage2DMapping( pCharData, 0, NNS_G2D_VRAM_TYPE_2DMAIN, &sImageProxy );
    OS_Free( pBuf ); // VRAM‚ÉƒRƒs[‚µ‚½‚Ì‚Å‰ğ•ú
    
    // ƒpƒŒƒbƒgƒf[ƒ^“Ç‚İ‚İ
    pBuf = PtclDemo_LoadNCLR( &pPlttData, "data/Color_sample.NCLR" );
    NNS_G2dLoadPalette( pPlttData, 0, NNS_G2D_VRAM_TYPE_2DMAIN, &sPaletteProxy );
    OS_Free( pBuf ); // VRAM‚ÉƒRƒs[‚µ‚½‚Ì‚Å‰ğ•ú


    //////////////////////////////////////////////////
    // SPL‰Šú‰»

    // ƒp[ƒeƒBƒNƒ‹ƒ}ƒl[ƒWƒƒ‚ğ‰Šú‰»
    spl_manager = SPL_Init( OS_AllocFromMain, 4, 200, 0, 1, 30 );

    // ƒp[ƒeƒBƒNƒ‹ƒŠƒ\[ƒX‚ğ“Ç‚İ‚İ
    resPtcl = PtclDemo_LoadFile( "data/output.spa" ) ;
    SPL_Load( spl_manager, resPtcl );

    // ƒeƒNƒXƒ`ƒƒ‚ğ“Ç‚İ‚İ
    (void)SPL_LoadTexByVRAMManager( spl_manager ) ;
    (void)SPL_LoadTexPlttByVRAMManager( spl_manager ) ;

    // ƒGƒ~ƒbƒ^‚ğì¬
    spl_emitter[0] = SPL_Create( spl_manager, 0, &init_pos );
    spl_emitter[1] = SPL_Create( spl_manager, 1, &init_pos );
    spl_emitter[2] = SPL_Create( spl_manager, 2, &init_pos );
    spl_emitter[3] = SPL_Create( spl_manager, 3, &init_pos );
   

    // ƒAƒtƒBƒ“ƒpƒ‰ƒ[ƒ^‚ğ“o˜^‚·‚é
    for( i = 0 ; i < 16 ; ++i )
    {
        MtxFx22 affine ;
        MTX_Identity22( &affine );
        MTX_Scale22( &affine, sScale[i], sScale[i] );
        NNS_G2dSetOamManagerAffine( NNS_G2D_OAMTYPE_MAIN, &affine, i );
    }

    
}

/*---------------------------------------------------
 * •`‰æ
 *-------------------------------------------------*/
static void draw()
{

    unsigned short uUsedOAM = 0 ;
    const NNSG2dCellData* pCell;                      // •\¦‚·‚é Cell
    
    // ƒeƒ“ƒ|ƒ‰ƒŠ‚ÈOAMƒoƒbƒtƒ@
    static GXOamAttr    temp[NUM_OF_OAM] ;
    int i = 0 ;

    // æ“ª‚ÌƒGƒ~ƒbƒ^‚ğæ“¾B
    SPLParticle* ptcl = SPL_GetHeadParticle( spl_emitter[suCurrentEmitter] ) ;

    if( ptcl == NULL )
    {
        return ;
    }

    // ƒAƒNƒeƒBƒu‚Èƒp[ƒeƒBƒNƒ‹‚·‚×‚Ä‚É‘Î‚µ‚ÄB
    while(1)
    {
        VecFx32 ptcl_pos ;
        NNSG2dFVec2 pos ;
        MtxFx22 affine ;
        u16 affineIdx = 0 ;
        u16 ptcl_age, ptcl_life ;
        
        // ƒp[ƒeƒBƒNƒ‹‚ÌˆÊ’u‚ğæ“¾‚·‚é
        SPL_GetParticlePos( ptcl, &ptcl_pos ) ;
     
        // ƒp[ƒeƒBƒNƒ‹‚Ìõ–½‚Æ”N—î‚ğæ“¾‚·‚é
        ptcl_life = SPL_GetParticleLife( ptcl ) ;
        ptcl_age  = SPL_GetParticleAge( ptcl ) ;
        
        // õ–½‚Æ”N—î‚©‚çŒ»İ‚ÌƒXƒP[ƒ‹ƒCƒ“ƒfƒbƒNƒX‚ğŒvZ‚·‚éB
        affineIdx = (u16)( ( FX_Mul( FX_Div( FX32_ONE * 15, ptcl_life * FX32_ONE ), ptcl_age * FX32_ONE ) ) >> FX32_SHIFT ) ;

        // ƒ}ƒgƒŠƒNƒXì¬
        MTX_Identity22( &affine );
        MTX_Scale22( &affine, FX_Div( FX32_ONE, sScale[affineIdx] ), FX_Div( FX32_ONE, sScale[affineIdx] ) );

        // ˆÚ“®—Ê‚ÌƒXƒP[ƒŠƒ“ƒO‚ÆA‰æ–Ê’†S‚Ö‚ÌˆÚ“®•â³B
        pos.x = ( ptcl_pos.x << 2 ) + 128 * FX32_ONE;
        pos.y = ( ( -ptcl_pos.y) << 2 ) + 96  * FX32_ONE ;

        // ƒZƒ‹‚ğæ“¾
        pCell = NNS_G2dGetCellDataByIdx( pCellBank, 0 );

        // Oam‚É‘‚«o‚·
        uUsedOAM += NNS_G2dMakeCellToOams( &temp[i], (u16)( NUM_OF_OAM-uUsedOAM ), pCell, &affine, &pos, affineIdx, FALSE ) ;
                                
        // Ÿ‚Ìƒp[ƒeƒBƒNƒ‹‚ğ’²‚×‚éB
        ptcl = SPL_GetNextParticle( ptcl ) ;
        
        ++i ;

        if( ptcl == NULL )
        {
            break ;
        }

    }

    (void)NNS_G2dEntryOamManagerOam( &sObjOamManager, temp, uUsedOAM );
    
    // 2D•`‰æ‚Ì‚Æ‚«‚ÍSPL_Draw‚Í•s—v‚Å‚·B
    //SPL_Draw( spl_manager, &camera );

}

/*---------------------------------------------------
 * ŒvZ
 *-------------------------------------------------*/
static void calc()
{
    SPL_Calc( spl_manager ) ;
    
    // Aƒ{ƒ^ƒ“‚ğ‰Ÿ‚·‚Æ•\¦‚·‚éƒGƒ~ƒbƒ^‚ğØ‚è‘Ö‚¦‚é
    if( PtclDemo_KeyOneshot( PAD_BUTTON_A ) )
    {
        ++suCurrentEmitter ;
        
        if( suCurrentEmitter > 3 )
        {
            suCurrentEmitter = 0 ;
        }
    }
    
    PtclDemo_UpdateInput() ;
}

/*---------------------------------------------------
 * ƒƒCƒ“
 *-------------------------------------------------*/
void NitroMain()
{
    init() ;

    while(1)
    {
        draw();

        calc() ;

        SVC_WaitVBlankIntr(); 
        NNS_G2dApplyOamManagerToHW( &sObjOamManager );
        NNS_G2dResetOamManagerBuffer(  &sObjOamManager );
        
    }
}



// end of file