//======================================================================
/**
 * @file  enc_adpcm.c
 * @brief ADPCM変換
 * @author  ariizumi
 * @data  09/11/11
 *
 * モジュール名：ENC_ADPCM
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "enc_adpcm.h"

//======================================================================
//  define
//======================================================================
#pragma mark [> define

//======================================================================
//  enum
//======================================================================
#pragma mark [> enum


//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
typedef struct {
    s16 prevSample;  /* Previous output value */
    s8  prevIndex;    /* Index into stepsize table */
}ADPCMstate ;
//======================================================================
//  proto
//======================================================================
#pragma mark [> proto
static void adpcm_coder( s16 *indata, s8 *outdata, int len, ADPCMstate *state);
static void adpcm_decoder( s8 *indata, s16 *outdata, int len,ADPCMstate *state);

static char ADPCMEncorder( short sample, ADPCMstate* state );
static s16 ADPCMDecoder( int code, ADPCMstate* state );

static ADPCMstate adpcmState;

void ENC_ADPCM_ResetParam( void )
{
  adpcmState.prevSample = 0;
  adpcmState.prevIndex = 0;
}

//--------------------------------------------------------------
//  エンコードメイン
//--------------------------------------------------------------
const u32 ENC_ADPCM_EncodeData( void* src , const u32 srcSize , void* dst )
{
  int i;
  s16 *srcData = src;
  s8 *dstData = dst;
//  adpcm_state state = {0,0};
  
/*
  for( i=0;i<srcSize/2;i++ )
  {
    OS_TFPrintf(2,"%d\n",srcData[i]);
  }
  OS_TFPrintf(2,"------------------------------------------\n");
*/
  adpcm_coder( srcData , dstData , srcSize/2 , &adpcmState );
/*
  for( i=0;i<(srcSize+3)/4;i++ )
  {
    OS_TFPrintf(2,"%d\n",dstData[i]);
  }
  OS_TFPrintf(2,"------------------------------------------\n");
*/  
  return (srcSize+3)/4;
}

//--------------------------------------------------------------
//  デコードメイン
//--------------------------------------------------------------
const u32 ENC_ADPCM_DecodeData( void* src , const u32 srcSize , void* dst )
{
  int i;
  s8 *srcData = src;
  s16 *dstData = dst;
//  adpcm_state state = {0,0};
/*
  for( i=0;i<srcSize;i++ )
  {
    OS_TFPrintf(3,"%d\n",srcData[i]);
  }
  OS_TFPrintf(3,"------------------------------------------\n");
*/
  adpcm_decoder(  srcData , dstData , srcSize*2 , &adpcmState );
/*
  for( i=0;i<srcSize*2;i++ )
  {
    OS_TFPrintf(3,"%d\n",dstData[i]);
  }
  OS_TFPrintf(3,"------------------------------------------\n");
*/
  return srcSize*4;
}

static void adpcm_coder( s16 *indata, s8 *outdata, int len, ADPCMstate *state)
{
  int i;
  int outIdx = 0;
  u8 tempData = 0;
  u8 isUpper = FALSE;
  for( i=0;i<len;i++ )
  {
    const u8 retCode = ADPCMEncorder( indata[i] , state );

    if( isUpper == FALSE )
    {
      tempData = retCode;
      isUpper = TRUE;
    }
    else
    {
      tempData += retCode<<4;
      outdata[outIdx] = tempData;
      
      isUpper = FALSE;
      tempData = 0;
      outIdx++;
    }
  }

  if( isUpper == TRUE )
  {
    outdata[outIdx] = tempData;
  }
}

static void adpcm_decoder( s8 *indata, s16 *outdata, int len,ADPCMstate *state)
{
  int i;
  for( i=0;i<len/2;i++ )
  {
    u8 lowerData = indata[i]&0x0f;
    u8 upperData = (indata[i]&0xf0)>>4;

    outdata[i*2] = ADPCMDecoder( lowerData , state );
    outdata[i*2+1] = ADPCMDecoder( upperData , state );
  }
}



#define INDEX_NUM 89

static const signed char sIndexTable[16] = {
  -1, -1, -1, -1, 2, 4, 6, 8,
  -1, -1, -1, -1, 2, 4, 6, 8,
};

static const int sStepSizeTable[ INDEX_NUM ] = {
  7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
  19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
  50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
  130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
  337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
  876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
  2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
  5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


static char ADPCMEncorder( short sample, ADPCMstate* state )
{
  int code;
  int diff;
  int diff2;
  int step;
  int pred;
  int index;

  pred = (int)(state->prevSample);
  index = state->prevIndex;
  step = sStepSizeTable[index];

  diff = sample - pred;
  if ( diff >= 0 ) {
    code = 0;
  }
  else {
    code = 8;
    diff = -diff;
  }

  diff2 = step >> 3;
  if ( diff >= step ) {
    code |= 4;
    diff -= step;
    diff2 += step;
  }
  step >>= 1;
  if ( diff >= step ) {
    code |= 2;
    diff -= step;
    diff2 += step;
  }
  step >>= 1;
  if ( diff >= step ) {
    code |= 1;
    diff2 += step;
  }
  
  if ( code & 8 ) 
    pred -= diff2;
  else
    pred += diff2;
  
  if ( pred > 32767 )
    pred = 32767;
  else if ( pred < -32767 )
    pred = -32767;

  index += sIndexTable[code];
  
  if ( index < 0 )
    index = 0;
  if ( index > 88 )
    index = 88;

  state->prevSample = (short)pred;
  state->prevIndex = index;

  return ( (char)(code & 0x0f) );
}

static s16 ADPCMDecoder( int code, ADPCMstate* state )
{
  int step;
  int sample;
  int index;
  int d;
    
  sample = state->prevSample;
  index  = state->prevIndex;
    
  step = sStepSizeTable[ index ];
    
  d = step >> 3;
  if ( code & 4 ) d += step;
    if ( code & 2 ) d += step >> 1;
  if ( code & 1 ) d += step >> 2;
    
  if ( code & 8 ) {
        sample -= d;
        if ( sample < -32768 ) sample = -32768;
    }
  else {
        sample += d;
        if ( sample > 32767 ) sample = 32767;
    }
    
  index += sIndexTable[ code ];
    
  if ( index < 0 ) index = 0;
  else if ( index > INDEX_NUM-1 ) index = INDEX_NUM-1;
    
  state->prevSample = (s16)sample;
  state->prevIndex = (u8)index;
    
  return (s16)sample;
}
