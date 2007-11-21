//=============================================================================================
/**
 * @file	sound.c
 * @brief	サウンドルーチン
 * @author	Hisashi Sogabe
 * @date	2007/04/17
 */
//=============================================================================================
#include "gflib.h"

//=============================================================================================
//	型宣言
//=============================================================================================

//=============================================================================================
//	定数宣言
//=============================================================================================
#define	SOUND_HEAP_SIZE		(0x40000)
#define	STOP_FADE_FRAME		(1)				//GFL_SOUND_StopBgmでの曲がとまるまでのFADE_WAIT
#define	SOUND_SEQARC_NOLOAD	(0xffffffff)
#define	SOUND_HANDLE_MAX	(8)

//=============================================================================================
//	ワーク
//=============================================================================================
static	NNSSndArc			arc;
//static	u8					sndHeap[SOUND_HEAP_SIZE];	//アプリごとに静的に確保してもらうように変更（サイズを可変にするため）
static	NNSSndHeapHandle	heap;
static	NNSSndHandle		sndHandle[SOUND_HANDLE_MAX];
static	int					GFL_SOUND_seqarc_no=SOUND_SEQARC_NOLOAD;
static	int					cur_bgm_no;

//=============================================================================================
//=============================================================================================
//	設定関数
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * サウンド初期化
 *
 * @param	sndHeap		サウンドヒープへのポインタ（静的に確保することを推奨）
 * @param	sndHeapSize	サウンドヒープのサイズ
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_Init( u8 *sndHeap,int sndHeapSize )
{
	int	i;

	NNS_SndInit();
	heap=NNS_SndHeapCreate(sndHeap,sndHeapSize);

	for(i=0;i<SOUND_HANDLE_MAX;i++){
		NNS_SndHandleInit(&sndHandle[i]);
	}

	arc.file_open=FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドメイン処理
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_Main( void )
{
	NNS_SndMain();
}

//--------------------------------------------------------------------------------------------
/**
 * サウンド終了（呼ばれることはないだろうが、一応作成）
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_Exit( void )
{
	NNS_SndHeapDestroy( heap );
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドデータロード（アーカイブデータ）
 *
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_LoadArchiveData( int arcID, int datID )
{
	BOOL	result;

	if(arc.file_open==TRUE){
		FS_CloseFile(&arc.file);
	}

	GFL_ARC_OpenFileTopPosWrite(arcID,datID,&arc.file);

    arc.file_open = TRUE;
    
    /* セットアップ */
    result = NNS_SndArcSetup( &arc, heap, FALSE );

    GF_ASSERT( result );
    if ( ! result ) return;

	NNS_SndArcSetCurrent(&arc);

	NNS_SndArcPlayerSetup(heap);
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドデータロード（シーケンスデータ）
 *
 * @param	seq_no	シーケンスデータナンバー
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_LoadSequenceData( int seq_no )
{
	NNS_SndArcLoadSeq(seq_no,heap);
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドデータロード（シーケンスアーカイブデータ）
 *
 * @param	seqarc_no	シーケンスアーカイブデータナンバー
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_LoadSequenceArchiveData( int seqarc_no )
{
	NNS_SndArcLoadSeqArc(seqarc_no,heap);
	GFL_SOUND_seqarc_no=seqarc_no;
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドデータロード（バンクデータ）
 *
 * @param	bank_no	バンクデータナンバー
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_LoadBankData( int bank_no )
{
	NNS_SndArcLoadBank(bank_no,heap);
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドデータロード（グループデータ）
 *
 * @param	group_no	グループデータナンバー
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_LoadGroupData( int group_no )
{
    const NNSSndArcGroupInfo* info;
    const NNSSndArcGroupItem* item;
    int itemNo;

	NNS_SndArcLoadGroup(group_no,heap);
    info = NNS_SndArcGetGroupInfo( group_no );

    GF_ASSERT ( info!=NULL );
    
    for( itemNo = 0; itemNo < info->count ; itemNo++ )
    {
        item = & info->item[ itemNo ];
        
        switch( item->type ) {
        case NNS_SNDARC_SNDTYPE_SEQARC:
			GFL_SOUND_seqarc_no=item->index;
            break;
        default:
            GF_ASSERT(1);
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------
/**
 * シーケンスナンバーからプレイヤーナンバーを取得
 *
 * @param	no	シーケンスナンバー
 *
 * @retval	プレイヤーナンバー or 0xff=取得失敗
 */
//--------------------------------------------------------------------------------------------
u8	GFL_SOUND_GetPlayerNo( u16 no )
{
	const NNSSndSeqParam* param;
	
	if( no == 0 ){
		//GF_ASSERT( (0) && "シーケンスナンバーが不正なのでプレイヤーナンバー取得失敗！" );
		return 0xff;	//エラー
	}

	//サウンドアーカイブ中の各シーケンスに対する、シーケンスパラメータ構造体を取得
	param = NNS_SndArcGetSeqParam( no );
	//OS_Printf( "プレイヤーナンバー = %d\n", param->playerNo );

	if( param == NULL ){
		//GF_ASSERT( (0) && "シーケンスナンバーが不正なのでプレイヤーナンバー取得失敗！" );
		OS_Printf( "シーケンスナンバーが不正なのでプレイヤーナンバー取得失敗！\n" );
		return 0xff;	//エラー
	}

	GF_ASSERT(param->playerNo<SOUND_HANDLE_MAX);

	return param->playerNo;
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドデータ再生（BGM）
 *
 * @param	bgm_no	BGMナンバー
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_PlayBGM( int bgm_no )
{
	u8	player_no;

	player_no=GFL_SOUND_GetPlayerNo(bgm_no);
	GF_ASSERT(player_no!=0xff);

	NNS_SndArcPlayerStartSeq(&sndHandle[player_no],bgm_no);
	cur_bgm_no=bgm_no;
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドデータStop（BGM）
 *
 * 今鳴っているBGMを止める
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_StopBGM( void )
{
	u8	player_no;

	player_no=GFL_SOUND_GetPlayerNo(cur_bgm_no);
	GF_ASSERT(player_no!=0xff);

	NNS_SndPlayerStopSeq(&sndHandle[player_no],STOP_FADE_FRAME);
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドデータ再生（SE）
 *
 * @param	se_no	SEナンバー
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_PlaySE( int se_no )
{
	u8	player_no;

	player_no=GFL_SOUND_GetPlayerNo(se_no);
	GF_ASSERT(player_no!=0xff);

	NNS_SndArcPlayerStartSeq(&sndHandle[player_no],se_no);
}

//--------------------------------------------------------------
/**
 * @brief	player_noを指定して再生中のシーケンスを止める
 *
 * @param	player_no	playerナンバー
 */
//--------------------------------------------------------------
void	GFL_SOUND_StopPlayerNo(u8 player_no)
{
	NNS_SndPlayerStopSeqByPlayerNo( player_no, 0 );
}

//--------------------------------------------------------------
/**
 * @brief	player_noを指定して再生中のシーケンスの数を返す
 *
 * @param	player_no	playerナンバー
 *
 * @retval	"再生中のシーケンスの数"
 */
//--------------------------------------------------------------
int		GFL_SOUND_CheckPlaySeq( u8 player_no )
{
	//上限はチェックしていないので注意！
	if( player_no < 0 ){
		GF_ASSERT( (0) && "引数が不正です！" );
	}

	return NNS_SndPlayerCountPlayingSeqByPlayerNo( player_no );
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドヒープ階層ロード
 *
 * @param	level	ロードする階層レベル
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_LoadHeapState(int level)
{
	NNS_SndHeapLoadState(heap,level);
}

//--------------------------------------------------------------------------------------------
/**
 * サウンドヒープ階層セーブ
 *
 * @retval	保存した階層レベル（-1でエラー）
 */
//--------------------------------------------------------------------------------------------
int	GFL_SOUND_SaveHeapState(void)
{
	return NNS_SndHeapSaveState(heap);
}

//--------------------------------------------------------------------------------------------
/**
 * プレイヤーナンバーを指定して一時停止
 *
 * @param	playerNo	プレイヤー番号
 * @param	flg			一時停止フラグ ( TRUE = PAUSE )
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_SOUND_PausePlayerNo( int playerNo, BOOL flg )
{
	NNS_SndPlayerPauseByPlayerNo( playerNo, flg );
}

//--------------------------------------------------------------------------------------------
/**
 * 音程を変更
 *
 * @param	playerNo	プレイヤー番号
 * @param	trbitmask	トラックビットマスク
 * @param	pitch		音程変化の値（-32768～32767）
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_SOUND_SetPitch( int playerNo, u16 trbitmask, int pitch )
{
	NNS_SndPlayerSetTrackPitch( &sndHandle[playerNo], trbitmask, pitch );
}

//--------------------------------------------------------------------------------------------
/**
 * 音程を変更
 *
 * @param	playerNo	プレイヤー番号
 * @param	trbitmask	トラックビットマスク
 * @param	pan			定位変化の値（-128～127）
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_SOUND_SetPan( int playerNo, u16 trbitmask, int pan )
{
	NNS_SndPlayerSetTrackPan( &sndHandle[playerNo], trbitmask, pan );
}

//SEQARCを用いての再生（現状は未サポート）
#if 0
//--------------------------------------------------------------------------------------------
/**
 * サウンドデータ再生（SE）
 *
 * @param	se_no	SEナンバー
 */
//--------------------------------------------------------------------------------------------
void	GFL_SOUND_PlaySE( int se_no )
{
	GF_ASSERT(GFL_SOUND_seqarc_no!=SOUND_SEQARC_NOLOAD);
	NNS_SndArcPlayerStartSeqArc(&seHandle,GFL_SOUND_seqarc_no,se_no);
}
#endif

