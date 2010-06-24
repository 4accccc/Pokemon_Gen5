//plist_sys.cÇ…íºê⁄includeÇ≥ÇπÇƒÇ¢ÇÈ

#define PLIST_DEBUG_GROUP_NUMBER (50)
#define PLIST_DEBUG_GROUP_NUMBER_REG (51)
#define PLIST_DEBUG_GROUP_NUMBER_POKE (52)
#define PLIST_DEBUG_GROUP_NUMBER_CHECK (53)
#define PLIST_DEBUG_GROUP_NUMBER_CHECK2 (54)

struct _PLIST_DEBUG_WORK
{
  REGULATION reg;
  u32        banPokeIdx;
  u32        banItemIdx;
  u32        pokeNo;
  
  u8         posArr[6];
  u32        checkArr[6];
  BOOL       ret;
  int        ret2;
  BOOL       isCheck;
};

static const BOOL PLD_UpdateU32( u32 *val , const u32 min , const u32 max )
{
  u16 value;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){ value = 10; }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ){ value = 100; }
  else{ value = 1; }

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    if( *val + value > max ){ *val = min; }
    else                    { *val += value; }
    return TRUE;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    if( *val < min+value ){ *val = max; }
    else                  { *val -= value; }
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------
static void PLD_U_RegLoPoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  u32 temp = reg->NUM_LO;
  if( PLD_UpdateU32( &temp , 1 , 6 ) )
  {
    reg->NUM_LO = temp;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegLoPoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  DEBUGWIN_ITEM_SetNameV( item , "PokeMinNum[%d]",reg->NUM_LO );
}

//----------------------------------------------------------------
static void PLD_U_RegHiPoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  u32 temp = reg->NUM_HI;
  if( PLD_UpdateU32( &temp , 1 , 6 ) )
  {
    reg->NUM_HI = temp;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegHiPoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  DEBUGWIN_ITEM_SetNameV( item , "PokeMaxNum[%d]",reg->NUM_HI );
}
//----------------------------------------------------------------
static void PLD_U_RegLevel( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  u32 temp = reg->LEVEL;
  if( PLD_UpdateU32( &temp , 0 , 100 ) )
  {
    reg->LEVEL = temp;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegLevel( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  DEBUGWIN_ITEM_SetNameV( item , "Level[%d]",reg->LEVEL );
}
//----------------------------------------------------------------
static void PLD_U_RegLevelType( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  u32 temp = reg->LEVEL_RANGE;
  if( PLD_UpdateU32( &temp , 0 , REGULATION_LEVEL_RANGE_MAX-1 ) )
  {
    reg->LEVEL_RANGE = temp;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegLevelType( void* userWork , DEBUGWIN_ITEM* item )
{
  static const char* typeName[REGULATION_LEVEL_RANGE_MAX] ={"none","over","under","over_set","same","under_set"};
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  DEBUGWIN_ITEM_SetNameV( item , "LvType[%s]",typeName[reg->LEVEL_RANGE] );
}
//----------------------------------------------------------------
static void PLD_U_RegSumLv( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  u32 temp = reg->LEVEL_TOTAL;
  if( PLD_UpdateU32( &temp , 0 , 600 ) )
  {
    reg->LEVEL_TOTAL = temp;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegSumLv( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  DEBUGWIN_ITEM_SetNameV( item , "SumLv[%d]",reg->LEVEL_TOTAL );
}
//----------------------------------------------------------------
static void PLD_U_RegSamePoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  u32 temp = reg->BOTH_POKE;
  if( PLD_UpdateU32( &temp , 0 , 1 ) )
  {
    reg->BOTH_POKE = temp;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegSamePoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  DEBUGWIN_ITEM_SetNameV( item , "SamePoke[%c]",(reg->BOTH_POKE?'o':'x') );
}
//----------------------------------------------------------------
static void PLD_U_RegSameItem( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  u32 temp = reg->BOTH_ITEM;
  if( PLD_UpdateU32( &temp , 0 , 1 ) )
  {
    reg->BOTH_ITEM = temp;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegSameItem( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  DEBUGWIN_ITEM_SetNameV( item , "SameItem[%c]",(reg->BOTH_ITEM?'o':'x') );
}
//----------------------------------------------------------------
static void PLD_U_RegBanPoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;

  if( PLD_UpdateU32( &work->debWork->banPokeIdx , 1 , MONSNO_TAMAGO-1 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    const u8 arrIdx = work->debWork->banPokeIdx/8;
    const u8 bitIdx = work->debWork->banPokeIdx%8;
    
    reg->VETO_POKE_BIT[arrIdx] ^= (1<<bitIdx);
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegBanPoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  const u8 arrIdx = work->debWork->banPokeIdx/8;
  const u8 bitIdx = work->debWork->banPokeIdx%8;
  const BOOL bit = (reg->VETO_POKE_BIT[arrIdx]&(1<<bitIdx));

  DEBUGWIN_ITEM_SetNameV( item , "BanPoke[%3d][%c]",work->debWork->banPokeIdx,(bit?'x':'o') );
}
//----------------------------------------------------------------
static void PLD_U_RegBanItem( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;

  if( PLD_UpdateU32( &work->debWork->banItemIdx , 1 , ITEM_DATA_MAX-1 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    const u8 arrIdx = work->debWork->banItemIdx/8;
    const u8 bitIdx = work->debWork->banItemIdx%8;
    
    reg->VETO_ITEM[arrIdx] ^= (1<<bitIdx);
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegBanItem( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  const u8 arrIdx = work->debWork->banItemIdx/8;
  const u8 bitIdx = work->debWork->banItemIdx%8;
  const BOOL bit = (reg->VETO_ITEM[arrIdx]&(1<<bitIdx));

  DEBUGWIN_ITEM_SetNameV( item , "BanItem[%3d][%c]",work->debWork->banItemIdx,(bit?'x':'o') );
}
//----------------------------------------------------------------
static void PLD_U_RegMustPoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  u32 temp = reg->MUST_POKE;
  if( PLD_UpdateU32( &temp , 0 , MONSNO_TAMAGO-1 ) )
  {
    reg->MUST_POKE = temp;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegMustPoke( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  DEBUGWIN_ITEM_SetNameV( item , "MustPoke[%d]",reg->MUST_POKE );
}
//----------------------------------------------------------------
static void PLD_U_RegMustForm( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  u32 temp = reg->MUST_POKE_FORM;
  if( PLD_UpdateU32( &temp , 0 , FORMNO_ANNOON_MAX-1 ) )
  {
    reg->MUST_POKE_FORM = temp;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_RegMustForm( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  REGULATION *reg = &work->debWork->reg;
  DEBUGWIN_ITEM_SetNameV( item , "MustForm[%d]",reg->MUST_POKE_FORM );
}
//================================================================
//----------------------------------------------------------------
static void PLD_U_PokePos( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( PLD_UpdateU32( &work->debWork->pokeNo , 0 , 5 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_PokePos( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "PokePos[%d]",work->debWork->pokeNo+1 );
}
//----------------------------------------------------------------
static void PLD_U_PokeMonsNo( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_monsno;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    return;
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    u32 temp = PP_Get( pp , paraId , NULL );
    if( PLD_UpdateU32( &temp , 1 , MONSNO_TAMAGO-1 ) )
    {
      PP_Put( pp , paraId , temp );
      DEBUGWIN_RefreshScreen();
    }
  }
}
static void PLD_D_PokeMonsNo( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_monsno;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    DEBUGWIN_ITEM_SetName( item , "--------" );
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    DEBUGWIN_ITEM_SetNameV( item , " MonsNo[%d]",PP_Get( pp , paraId , NULL ) );
  }
}
//----------------------------------------------------------------
static void PLD_U_PokeLv( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_level;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    return;
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    u32 temp = PP_Get( pp , paraId , NULL );
    if( PLD_UpdateU32( &temp , 1 , 100 ) )
    {
      POKETOOL_MakeLevelRevise( pp , temp );
      //PP_Put( pp , paraId , temp );
      DEBUGWIN_RefreshScreen();
    }
  }
}
static void PLD_D_PokeLv( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_level;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    DEBUGWIN_ITEM_SetName( item , "--------" );
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    DEBUGWIN_ITEM_SetNameV( item , " Level[%d]",PP_Get( pp , paraId , NULL ) );
  }
}
//----------------------------------------------------------------
static void PLD_U_PokeItem( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_item;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    return;
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    u32 temp = PP_Get( pp , paraId , NULL );
    if( PLD_UpdateU32( &temp , 0 , ITEM_DATA_MAX-1 ) )
    {
      PP_Put( pp , paraId , temp );
      DEBUGWIN_RefreshScreen();
    }
  }
}
static void PLD_D_PokeItem( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_item;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    DEBUGWIN_ITEM_SetName( item , "--------" );
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    DEBUGWIN_ITEM_SetNameV( item , " Item[%d]",PP_Get( pp , paraId , NULL ) );
  }
}
//----------------------------------------------------------------
static void PLD_U_PokeForm( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_form_no;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    return;
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    u32 temp = PP_Get( pp , paraId , NULL );
    if( PLD_UpdateU32( &temp , 0 , FORMNO_ANNOON_MAX-1 ) )
    {
      PP_Put( pp , paraId , temp );
      DEBUGWIN_RefreshScreen();
    }
  }
}
static void PLD_D_PokeForm( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_form_no;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    DEBUGWIN_ITEM_SetName( item , "--------" );
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    DEBUGWIN_ITEM_SetNameV( item , " Form[%d]",PP_Get( pp , paraId , NULL ) );
  }
}
//----------------------------------------------------------------
static void PLD_U_PokeEgg( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_tamago_flag;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    return;
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    u32 temp = PP_Get( pp , paraId , NULL );
    if( PLD_UpdateU32( &temp , 0 , 1 ) )
    {
      PP_Put( pp , paraId , temp );
      DEBUGWIN_RefreshScreen();
    }
  }
}
static void PLD_D_PokeEgg( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_tamago_flag;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    DEBUGWIN_ITEM_SetName( item , "--------" );
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    DEBUGWIN_ITEM_SetNameV( item , " Egg[%d]",PP_Get( pp , paraId , NULL ) );
  }
}
//----------------------------------------------------------------
static void PLD_U_PokeNowHp( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_hp;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    return;
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    u32 temp = PP_Get( pp , paraId , NULL );
    u32 max = PP_Get( pp , ID_PARA_hpmax , NULL );
    if( PLD_UpdateU32( &temp , 0 , max ) )
    {
      PP_Put( pp , paraId , temp );
      DEBUGWIN_RefreshScreen();
    }
  }
}
static void PLD_D_PokeNowHp( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_hp;
  PLIST_WORK *work = userWork;
  if( work->debWork->pokeNo >= PokeParty_GetPokeCount( work->plData->pp ) )
  {
    DEBUGWIN_ITEM_SetName( item , "--------" );
  }
  else
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->debWork->pokeNo );
    DEBUGWIN_ITEM_SetNameV( item , " NowHp[%d]",PP_Get( pp , paraId , NULL ) );
  }
}

//----------------------------------------------------------------
static void PLD_U_PokeReDraw( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  
  //ÉLÅ[ÇÃåÎìÆçÏñhÇÆÇΩÇﬂY
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
  {
    u8 i;
    for( i=0;i<PokeParty_GetPokeCount( work->plData->pp );i++ )
    {
      POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, i );
      PLIST_PLATE_ResetParam( work , work->plateWork[i] , pp , 0 );
    }
    DEBUGWIN_UpdateFrame();
  }
}
static void PLD_D_PokeReDraw( void* userWork , DEBUGWIN_ITEM* item )
{
  const int paraId = ID_PARA_tamago_flag;
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetName( item , "Refresh[Push Y]" );
}
//================================================================
//----------------------------------------------------------------
static void PLD_U_CheckRegCheck( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    work->debWork->isCheck = TRUE;
    DEBUGWIN_RefreshScreen();
    work->debWork->ret = PokeRegulationCheckPokeParty_Func( &work->debWork->reg , work->plData->pp , work->debWork->posArr );
    work->debWork->isCheck = FALSE;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_CheckRegCheck( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetName( item , "Check!" );
}

static void PLD_D_CheckResult( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( work->debWork->isCheck == TRUE )
  {
    DEBUGWIN_ITEM_SetNameV( item , "[wait...]");
  }
  else
  {
    DEBUGWIN_ITEM_SetNameV( item , "[%s]",(work->debWork->ret?"OK":"NG") );
  }
}

static void PLD_D_CheckArr1( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "1[%d]",work->debWork->posArr[0] );
}
static void PLD_D_CheckArr2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "2[%d]",work->debWork->posArr[1] );
}
static void PLD_D_CheckArr3( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "3[%d]",work->debWork->posArr[2] );
}
static void PLD_D_CheckArr4( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "4[%d]",work->debWork->posArr[3] );
}
static void PLD_D_CheckArr5( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "5[%d]",work->debWork->posArr[4] );
}
static void PLD_D_CheckArr6( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "6[%d]",work->debWork->posArr[5] );
}
//================================================================
//----------------------------------------------------------------
static void PLD_U_CheckRegCheck2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    u8 arr[6];
    u8 i;
    work->debWork->isCheck = TRUE;
    DEBUGWIN_RefreshScreen();
    for( i=0;i<6;i++ )
    {
      arr[i] = work->debWork->checkArr[i];
    }
    work->debWork->ret2 = PokeRegulationMatchFullPokeParty( &work->debWork->reg , work->plData->pp , arr );
    for( i=0;i<6;i++ )
    {
      work->debWork->checkArr[i] = arr[i];
    }
    work->debWork->isCheck = FALSE;
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_CheckRegCheck2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetName( item , "Check!" );
}

static void PLD_D_CheckResult2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( work->debWork->isCheck == TRUE )
  {
    DEBUGWIN_ITEM_SetNameV( item , "[wait...]");
  }
  else
  {
    static const char *strArr[8] = 
    {
      "OK","LV over","BothPoke","BothItem","NumError1","IllegalPoke","NumError2","NotMust",
    };
    DEBUGWIN_ITEM_SetNameV( item , "[%s]",strArr[work->debWork->ret2] );
  }
}

//----------------------------------------------------------------
static void PLD_U_CheckArr1_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( PLD_UpdateU32( &work->debWork->checkArr[0] , 0 , 6 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_CheckArr1_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "1[%d]",work->debWork->checkArr[0] );
}
//----------------------------------------------------------------
static void PLD_U_CheckArr2_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( PLD_UpdateU32( &work->debWork->checkArr[1] , 0 , 6 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_CheckArr2_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "2[%d]",work->debWork->checkArr[1] );
}
//----------------------------------------------------------------
static void PLD_U_CheckArr3_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( PLD_UpdateU32( &work->debWork->checkArr[2] , 0 , 6 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_CheckArr3_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "3[%d]",work->debWork->checkArr[2] );
}
//----------------------------------------------------------------
static void PLD_U_CheckArr4_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( PLD_UpdateU32( &work->debWork->checkArr[3] , 0 , 6 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_CheckArr4_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "4[%d]",work->debWork->checkArr[3] );
}
//----------------------------------------------------------------
static void PLD_U_CheckArr5_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( PLD_UpdateU32( &work->debWork->checkArr[4] , 0 , 6 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_CheckArr5_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "5[%d]",work->debWork->checkArr[4] );
}
//----------------------------------------------------------------
static void PLD_U_CheckArr6_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  if( PLD_UpdateU32( &work->debWork->checkArr[5] , 0 , 6 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void PLD_D_CheckArr6_2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "6[%d]",work->debWork->checkArr[5] );
}

//================================================================
static void PLIST_InitDebug( PLIST_WORK *work )
{
  DEBUGWIN_InitProc( PLIST_BG_MENU , work->fontHandle );
  DEBUGWIN_ChangeLetterColor( 0,0,0 );

  work->debWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PLIST_DEBUG_WORK) );
  work->debWork->banPokeIdx = 1;
  work->debWork->banItemIdx = 1;
  work->debWork->pokeNo = 0;
  work->debWork->ret = FALSE;
  work->debWork->posArr[0] = 0;
  work->debWork->posArr[1] = 0;
  work->debWork->posArr[2] = 0;
  work->debWork->posArr[3] = 0;
  work->debWork->posArr[4] = 0;
  work->debWork->posArr[5] = 0;
  work->debWork->checkArr[0] = 0;
  work->debWork->checkArr[1] = 0;
  work->debWork->checkArr[2] = 0;
  work->debWork->checkArr[3] = 0;
  work->debWork->checkArr[4] = 0;
  work->debWork->checkArr[5] = 0;
  PokeRegulation_LoadData( REG_RND_SINGLE , &work->debWork->reg );
  
  DEBUGWIN_AddGroupToTop( PLIST_DEBUG_GROUP_NUMBER , "PokeList" , work->heapId );
  DEBUGWIN_AddGroupToGroup( PLIST_DEBUG_GROUP_NUMBER_REG , "Regulation" , PLIST_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddGroupToGroup( PLIST_DEBUG_GROUP_NUMBER_POKE , "Poke" , PLIST_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddGroupToGroup( PLIST_DEBUG_GROUP_NUMBER_CHECK , "RegCheck" , PLIST_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddGroupToGroup( PLIST_DEBUG_GROUP_NUMBER_CHECK2 , "RegCheck2" , PLIST_DEBUG_GROUP_NUMBER , work->heapId );

  //Reg
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegLoPoke ,PLD_D_RegLoPoke , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegHiPoke ,PLD_D_RegHiPoke , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegSumLv  ,PLD_D_RegSumLv  , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegLevel  ,PLD_D_RegLevel  , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegLevelType ,PLD_D_RegLevelType  , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegSamePoke ,PLD_D_RegSamePoke , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegSameItem ,PLD_D_RegSameItem , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegBanPoke ,PLD_D_RegBanPoke , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegBanItem ,PLD_D_RegBanItem , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegMustPoke ,PLD_D_RegMustPoke , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_RegMustForm ,PLD_D_RegMustForm , (void*)work , PLIST_DEBUG_GROUP_NUMBER_REG , work->heapId );

  //Poke
  DEBUGWIN_AddItemToGroupEx( PLD_U_PokePos ,PLD_D_PokePos , (void*)work , PLIST_DEBUG_GROUP_NUMBER_POKE , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_PokeMonsNo ,PLD_D_PokeMonsNo , (void*)work , PLIST_DEBUG_GROUP_NUMBER_POKE , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_PokeLv ,PLD_D_PokeLv , (void*)work , PLIST_DEBUG_GROUP_NUMBER_POKE , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_PokeItem ,PLD_D_PokeItem , (void*)work , PLIST_DEBUG_GROUP_NUMBER_POKE , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_PokeForm ,PLD_D_PokeForm , (void*)work , PLIST_DEBUG_GROUP_NUMBER_POKE , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_PokeEgg ,PLD_D_PokeEgg , (void*)work , PLIST_DEBUG_GROUP_NUMBER_POKE , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_PokeNowHp ,PLD_D_PokeNowHp , (void*)work , PLIST_DEBUG_GROUP_NUMBER_POKE , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_PokeReDraw ,PLD_D_PokeReDraw , (void*)work , PLIST_DEBUG_GROUP_NUMBER_POKE , work->heapId );

  //Check
  DEBUGWIN_AddItemToGroupEx( PLD_U_CheckRegCheck ,PLD_D_CheckRegCheck , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK , work->heapId );
  DEBUGWIN_AddItemToGroupEx( NULL ,PLD_D_CheckResult , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK , work->heapId );
  DEBUGWIN_AddItemToGroupEx( NULL ,PLD_D_CheckArr1 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK , work->heapId );
  DEBUGWIN_AddItemToGroupEx( NULL ,PLD_D_CheckArr2 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK , work->heapId );
  DEBUGWIN_AddItemToGroupEx( NULL ,PLD_D_CheckArr3 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK , work->heapId );
  DEBUGWIN_AddItemToGroupEx( NULL ,PLD_D_CheckArr4 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK , work->heapId );
  DEBUGWIN_AddItemToGroupEx( NULL ,PLD_D_CheckArr5 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK , work->heapId );
  DEBUGWIN_AddItemToGroupEx( NULL ,PLD_D_CheckArr6 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK , work->heapId );

  //Check
  DEBUGWIN_AddItemToGroupEx( PLD_U_CheckRegCheck2 ,PLD_D_CheckRegCheck2 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK2 , work->heapId );
  DEBUGWIN_AddItemToGroupEx( NULL ,PLD_D_CheckResult2 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK2 , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_CheckArr1_2 ,PLD_D_CheckArr1_2 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK2 , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_CheckArr2_2 ,PLD_D_CheckArr2_2 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK2 , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_CheckArr3_2 ,PLD_D_CheckArr3_2 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK2 , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_CheckArr4_2 ,PLD_D_CheckArr4_2 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK2 , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_CheckArr5_2 ,PLD_D_CheckArr5_2 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK2 , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLD_U_CheckArr6_2 ,PLD_D_CheckArr6_2 , (void*)work , PLIST_DEBUG_GROUP_NUMBER_CHECK2 , work->heapId );
}

static void PLIST_TermDebug( PLIST_WORK *work )
{
  DEBUGWIN_RemoveGroup( PLIST_DEBUG_GROUP_NUMBER );
  DEBUGWIN_ExitProc();

  GFL_HEAP_FreeMemory( work->debWork );
}
