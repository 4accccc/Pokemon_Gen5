#pragma once

static const STRCODE sc_pdwacc_gamesync_id_buff[]={
  65,	//A0
  66,	//B1
  67,	//C2
  68,	//D3
  69,	//E4
  70,	//F4
  71,	//G5
  72,	//H6
  //73,	//I
  74,	//J7
  75,	//K8
  76,	//L9
  77,	//M10
  78,	//N11
  //79,	//O
  80,	//P12
  81,	//Q13
  82,	//R14
  83,	//S15
  84,	//T16
  85,	//U17
  86,	//V18
  87,	//W19
  88,	//X20
  89,	//Y21
  90,	//Z22
  //48,	//0
  //49,	//1
  50,	//2 23
  51,	//3 24
  52,	//4 25
  53,	//5 26
  54,	//6 27
  55,	//7 28
  56,	//8 29
  57,	//9 30
};


//------------------------------------------------------------------------------
/**
 * @brief   アクセスコード表示
 * @retval  none
 */
//------------------------------------------------------------------------------
inline void PDWACC_MESSAGE_GetPassWord(u32 profileID, STRBUF* pStrbuf)
{
  int i;

  u64 moji;
  u16 word;
  STRCODE disp[13*sizeof(STRCODE)];
  s32 id = profileID;
  u16 crc = GFL_STD_CrcCalc( &id, 4 );
  u64 code = id + crc * 0x100000000;
  OS_TPrintf("id=%x crc=%x code=%x\n",id,crc,code);

  moji = code;
  GFL_STD_MemFill(disp,0xff,sizeof(disp));
  for(i = 0; i < 10 ; i++){
    word = moji & 0x1f;
    moji = moji >> 5;
    disp[i]=sc_pdwacc_gamesync_id_buff[word];
  }
  GFL_STR_SetStringCode(pStrbuf,disp);
}

//------------------------------------------------------------------------------
/**
 * @brief   コードから数値に変換
 * @retval  none
 */
//------------------------------------------------------------------------------
inline u32 PDWACC_MESSAGE_ConvertID( const STRBUF *cp_strbuf )
{
  int i;
  int j;
  u64 code = 0;
  u32 id;
  u32 crc;

  const STRCODE *cp_strcode = GFL_STR_GetStringCodePointer(cp_strbuf);


  GF_ASSERT( GFL_STR_GetBufferLength(cp_strbuf) == 10 );

  for( i = 0; i < 10 ; i++ ){
    u64 word  = 0xFFFFFFFF;
    for( j = 0; j < NELEMS(sc_pdwacc_gamesync_id_buff); j++ )
    {
      if( cp_strcode[i] == sc_pdwacc_gamesync_id_buff[j] )
      {
        word = j;
        break;
      }
    }
    GF_ASSERT( word != 0xFFFFFFFF );
    code += word << (i*5);
    OS_TPrintf( "idx%d value%d\n", word, word << (i*5) );
  }

  id = code%0x100000000;
  crc = code/0x100000000;

  OS_TPrintf("id=%d crc=%d code=%d\n",id,crc,code);

  GF_ASSERT( crc == GFL_STD_CrcCalc( &id, 4 ) );
  return id;
}
