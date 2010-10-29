#include "common.h"

void PrintErrorMsg( u16 item, u16 err )
{
	switch( err ){
	case 0:			// 買値
		printf( "ERROR : item %d -買値\n\n", item );
		break;
	case 1:			// 装備効果
		printf( "ERROR : item %d -装備効果\n\n", item );
		break;
	case 2:			// 威力
		printf( "ERROR : item %d -威力\n\n", item );
		break;
	case 3:			// 重要	
		printf( "ERROR : item %d -重要\n\n", item );
		break;
	case 4:			// 便利ボタン
		printf( "ERROR : item %d -便利ボタン\n\n", item );
		break;
	case 5:			// 保存先
		printf( "ERROR : item %d -保存先\n\n", item );
		break;
	case 6:			// field機能
		printf( "ERROR : item %d -field機能\n\n", item );
		break;
	case 7:			// battle機能
		printf( "ERROR : item %d -battle機能\n\n", item );
		break;
	case 8:			// ワークタイプ
		printf( "ERROR : item %d -ワークタイプ\n\n", item );
		break;
	case 9:			// ワーク
		printf( "ERROR : item %d -ワーク\n\n", item );
		break;
	case 10:		// 能力ガード
		printf( "ERROR : item %d -能力ガード\n\n", item );
		break;
	case 11:		// クリティカル
		printf( "ERROR : item %d -クリティカル\n\n", item );
		break;
	case 12:		// 攻撃力
		printf( "ERROR : item %d -攻撃力\n\n", item );
		break;
	case 13:		// 防御力
		printf( "ERROR : item %d -防御力\n\n", item );
		break;
	case 14:		// 素早さ
		printf( "ERROR : item %d -素早さ\n\n", item );
		break;
	case 15:		// 命中率
		printf( "ERROR : item %d -命中率\n\n", item );
		break;
	case 16:		// 特殊攻撃
		printf( "ERROR : item %d -特殊攻撃\n\n", item );
		break;
	case 17:		// 特殊防御
		printf( "ERROR : item %d -特殊防御\n\n", item );
		break;
	case 18:		// レベルup
		printf( "ERROR : item %d -レベルup\n\n", item );
		break;
	case 19:		// pp回復flg
		printf( "ERROR : item %d -pp回復flg\n\n", item );
		break;
	case 20:		// pp回復値
		printf( "ERROR : item %d -pp回復値\n\n", item );
		break;
	case 21:		// pp値up
		printf( "ERROR : item %d -pp値up\n\n", item );
		break;
	case 22:		// HP努力値
		printf( "ERROR : item %d -HP努力値\n\n", item );
		break;
	case 23:		// 攻撃努力値
		printf( "ERROR : item %d -攻撃努力値\n\n", item );
		break;
	case 24:		// 防御努力値
		printf( "ERROR : item %d -防御努力値\n\n", item );
		break;
	case 25:		// 素早努力値
		printf( "ERROR : item %d -素早努力値\n\n", item );
		break;
	case 26:		// 特攻努力値
		printf( "ERROR : item %d -特攻努力値\n\n", item );
		break;
	case 27:		// 特防努力値
		printf( "ERROR : item %d -特防努力値\n\n", item );
		break;
	case 28:		// なつき度1
		printf( "ERROR : item %d -なつき度1\n\n", item );
		break;
	case 29:		// なつき度2
		printf( "ERROR : item %d -なつき度2\n\n", item );
		break;
	case 30:		// なつき度3
		printf( "ERROR : item %d -なつき度3\n\n", item );
		break;
	case 31:		// 眠り回復
		printf( "ERROR : item %d -眠り回復\n\n", item );
		break;
	case 32:		// 毒回復
		printf( "ERROR : item %d -毒回復\n\n", item );
		break;
	case 33:		// やけど回復
		printf( "ERROR : item %d -やけど回復\n\n", item );
		break;
	case 34:		// こおり回復
		printf( "ERROR : item %d -こおり回復\n\n", item );
		break;
	case 35:		// まひ回復
		printf( "ERROR : item %d -まひ回復\n\n", item );
		break;
	case 36:		// 混乱回復
		printf( "ERROR : item %d -混乱回復\n\n", item );
		break;		
	case 37:		// メロメロ回復
		printf( "ERROR : item %d -メロメロ回復\n\n", item );
		break;
	case 38:		// ＨＰ回復
		printf( "ERROR : item %d -ＨＰ回復\n\n", item );
		break;
	case 39:		// 瀕死回復
		printf( "ERROR : item %d -瀕死回復\n\n", item );
		break;
	case 40:		// 進化
		printf( "ERROR : item %d -進化\n\n", item );
		break;
	case 41:		// 戦闘ポケット
		printf( "ERROR : item %d -戦闘ポケット\n\n", item );
		break;
	case 42:		// なげつける威力
		printf( "ERROR : item %d -なげつける威力\n\n", item );
		break;
	case 43:		// しぜんのめぐみ威力
		printf( "ERROR : item %d -しぜんのめぐみ威力\n\n", item );
		break;
	case 44:		// しぜんのめぐみタイプ
		printf( "ERROR : item %d -しぜんのめぐみタイプ\n\n", item );
		break;
	case 45:		// なげつける効果
		printf( "ERROR : item %d -なげつける効果\n\n", item );
		break;
	case 46:		// ついばむ効果
		printf( "ERROR : item %d -ついばむ効果\n\n", item );
		break;
	case 47:		// 消費するか（戦闘）
		printf( "ERROR : item %d -Ｂ消費\n\n", item );
		break;
	case 48:		// 技マシン収録技
		printf( "ERROR : item %d -技マシン収録技\n\n", item );
		break;
	case 49:		// ソート番号
		printf( "ERROR : item %d -ソート番号\n\n", item );
		break;
	case 50:		// 努力値制御フラグ
		printf( "ERROR : item %d -努力値制御フラグ\n\n", item );
		break;
	case 51:		// 複合
		printf( "ERROR : item %d -複合\n\n", item );
		break;
	case 52:
	case 53:
	case 54:
	case 55:
	case 56:
	case 57:
	case 58:
	case 59:
	case 60:
	case 61:
	case 62:
	case 63:
	case 64:
	case 65:
	case 66:
	case 67:
	case 68:
	case 69:
		break;
	}
}
