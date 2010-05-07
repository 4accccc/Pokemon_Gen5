//-------------------------------------------------------------------
// WifiConv.exe
//
// DPの時に作った地球儀座標取り込みツールを再度利用
//
// 2010.05.04:国リストのエクセル構造が変わったので引数「3」の時だけ変更
//
//
// AkitoMori GameFreak inc.
//-------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// バイナリにするときに使う構造体
typedef struct{
  short x;
  short y;
}POS_XY;

typedef struct{
  short type;
  short x;
  short y;
}POS_FLAG_XY;


//------------------------------------------------------------------
/**
 * $brief   XY形式で取り込む
 *
 * @param   fp    
 * @param   buf   
 * @param   str[][200]    
 * @param   posxy   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int GetTextInfo0( FILE *fp, char *buf, char str[][200], POS_XY *posxy)
{
  int count=0;
  int  no,  x, y;

  // ファイルが終わるまで
  while(fgets(buf,1000,fp)!=NULL){

    // 文字列から取り込み（文字・文字・数値・数値）
    if(sscanf(buf,"%[^,],%[^,],%x,%x" ,str[0],str[1],&x,&y)==EOF){
      printf("この行で入力エラーが起きました\n%s\n",buf);
      exit(1);
    }
    printf("%s, %s,  %x, %x, \n",str[0],str[1],x,y);

    // 構造体に格納
    posxy[count].x = x;
    posxy[count].y = y;

    // 取り込んだ回数を＋１
    count++;
    
  }
  return count;
}

//------------------------------------------------------------------
/**
 * $brief   タイプ＋XYで取り込む
 *
 * @param   fp    
 * @param   buf   
 * @param   str[][200]    
 * @param   posfxy    
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int GetTextInfo1( FILE *fp, char *buf, char str[][200], POS_FLAG_XY *posfxy)
{
  int count=0;
  int  no, x, y, d1,d2,flag;

  // ファイルが終わるまで
  while(fgets(buf,1000,fp)!=NULL){

    // 文字列から取り込み（文字・文字・文字・数値・数値→2個目の数値は10進で変換）
    if(sscanf(buf,"%[^,],%[^,],%[^,],%d,%x,%x,%d,%d" ,str[0],str[1],str[2],&d1,&x,&y,&d2,&flag)==EOF){
      printf("この行で入力エラーが起きました\n%s\n",buf);
      exit(1);
    }
    printf("%s, %s, %s, %x, %x, %d\n",str[0],str[1],str[2],x,y,flag);

    // 構造体に格納
    posfxy[count].type = flag;
    posfxy[count].x    = x;
    posfxy[count].y    = y;

    // 取り込んだ回数を＋１
    count++;
    
  }
  return count;
}


//==============================================================================
/**
 * $brief   メイン
 *
 * @param   argc    
 * @param   argv    
 *
 * @retval  int   
 */
//==============================================================================
int main(int argc, char **argv)
{
  char  buf[1000];    //上限は適当
  char  str[3][200];    //上限は適当
  POS_XY      posxy[10000];   //上限は適当
  POS_FLAG_XY posfxy[10000];    //上限は適当
  int  count;
  int convert_mode = 0;
  FILE *fp,*outfp;
  
  // オプションの数を数える
  if(argc!=4){
    printf("argc = %d\n",argc);
    printf("ファイル指定してください\n");
    printf("earthconv.exe <0,1> input output\n");
    exit(1);
  }
  
  if(argv[1][0]=='2' || argv[1][0]=='3'){
    convert_mode = argv[1][0]-'2';
  }else{
    printf("第１引数は「2」か「3」を指定してください\n出力データが２つ(XY)か３つ(TYPE,XY)かという意味です\n");
    exit(1);
  }
  
  // 入力ファイルオープン
  if((fp=fopen(argv[2],"r"))==NULL){
    printf("%sが読み込めません\n",argv[2]);
    exit(1);
  }

  // 出力ファイルオープン
  if((outfp=fopen(argv[3],"wb"))==NULL){
    printf("%sが作れません\n",argv[3]);
    exit(1);
  }


  // 2行読み捨てる
  fgets(buf,1000,fp);
  fgets(buf,1000,fp);
  

  // コンバート開始
  if(convert_mode==0){

    // XYのみ取り出すモード
    count = GetTextInfo0(fp, buf, str, posxy);

    printf("取り込んだ行数=%d 出力バイト数=%d\n", count, count*sizeof(POS_XY));

    // 出力
    fwrite((void*)&posxy, sizeof(POS_XY)*count, 1, outfp);

  }else{
    // TYPE・XYを取り出すモード
    count = GetTextInfo1(fp, buf, str, posfxy);

    printf("取り込んだ行数=%d 出力バイト数=%d\n", count, count*sizeof(POS_FLAG_XY));

    // 出力
    fwrite((void*)&posfxy, sizeof(POS_FLAG_XY)*count, 1, outfp);
  }


  
  return 0;
}
