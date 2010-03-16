/////////////////////////////////////////////////////////////////////
/**
 * @brief  すれ違い調査隊 定数定義
 * @file   research_team_def.h
 * @author obata
 * @date   2010.03.08
 */
/////////////////////////////////////////////////////////////////////
#pragma once

// 1つの調査依頼で調べる質問の最大数
#define MAX_QNUM_PER_RESEARCH_REQ (3)

// 調査隊ランク
#define RESEARCH_MEMBER_RANK_C  (0)
#define RESEARCH_MEMBER_RANK_B  (1)
#define RESEARCH_MEMBER_RANK_A  (2)
#define RESEARCH_MEMBER_RANK_S  (3)
#define RESEARCH_MEMBER_RANK_SS (4)

///調査隊ランク
#define RESEARCH_TEAM_RANK_0 (0)
#define RESEARCH_TEAM_RANK_1 (1)
#define RESEARCH_TEAM_RANK_2 (2)
#define RESEARCH_TEAM_RANK_3 (3)
#define RESEARCH_TEAM_RANK_4 (4)
#define RESEARCH_TEAM_RANK_5 (5)
#define RESEARCH_TEAM_RANK_MAX (6)

// 調査タイプ
#define RESEARCH_REQ_TYPE_COUNT (0) // 人数調査
#define RESEARCH_REQ_TYPE_TIME  (1) // 時間調査
#define RESEARCH_REQ_TYPE_NUM   (2) // 総数

// 調査依頼ID
#define RESEARCH_REQ_ID_NONE                 (0) // ダミー ( 依頼を受けていない )
#define RESEARCH_REQ_ID_WB_count             (1) //「ブラックと　ホワイト　おおいのは？」( 人数 )
#define RESEARCH_REQ_ID_WB_time              (2) //「ブラックと　ホワイト　おおいのは？」( 時間 )
#define RESEARCH_REQ_ID_FIRST_POKE_count     (3) //「えらんだ　さいしょのポケモンは？」( 人数 )
#define RESEARCH_REQ_ID_FIRST_POKE_time      (4) //「えらんだ　さいしょのポケモンは？」( 時間 )
#define RESEARCH_REQ_ID_PLAY_TIME_count      (5) //「いちばんおおい　プレイじかんは？」( 人数 )
#define RESEARCH_REQ_ID_PLAY_TIME_time       (6) //「いちばんおおい　プレイじかんは？」( 時間 )
#define RESEARCH_REQ_ID_SEX_count            (7) //「おとこのこと　おんなのこ　おおいのは？」( 人数 )
#define RESEARCH_REQ_ID_SEX_time             (8) //「おとこのこと　おんなのこ　おおいのは？」( 時間 )
#define RESEARCH_REQ_ID_JOB_count            (9) //「いちばんおおい　しごとは？」( 人数 )
#define RESEARCH_REQ_ID_JOB_time            (10) //「いちばんおおい　しごとは？」( 時間 )
#define RESEARCH_REQ_ID_HOBBY_count         (11) //「いちばんおおい　しゅみは？」( 人数 )
#define RESEARCH_REQ_ID_HOBBY_time          (12) //「いちばんおおい　しゅみは？」( 時間 )
#define RESEARCH_REQ_ID_BTL_OR_TRD_count    (13) //「たいせんと　こうかん　にんきがあるのは？」( 人数 )
#define RESEARCH_REQ_ID_BTL_OR_TRD_time     (14) //「たいせんと　こうかん　にんきがあるのは？」( 時間 )
#define RESEARCH_REQ_ID_FAVORITE_POKE_count (15) //「すきな　ポケモンは？」( 人数 )
#define RESEARCH_REQ_ID_FAVORITE_POKE_time  (16) //「すきな　ポケモンは？」( 時間 )
#define RESEARCH_REQ_ID_POKE_TYPE_count     (17) //「すきな　ポケモンのタイプは？」( 人数 )
#define RESEARCH_REQ_ID_POKE_TYPE_time      (18) //「すきな　ポケモンのタイプは？」( 時間 )
#define RESEARCH_REQ_ID_PLACE_count         (19) //「とかいと　いなか　すみたいのは？」( 人数 )
#define RESEARCH_REQ_ID_PLACE_time          (20) //「とかいと　いなか　すみたいのは？」( 時間 )
#define RESEARCH_REQ_ID_PRECIOUS_count      (21) //「だいじにしているものに　ついて」( 人数 )
#define RESEARCH_REQ_ID_PRECIOUS_time       (22) //「だいじにしているものに　ついて」( 時間 )
#define RESEARCH_REQ_ID_PARTNER_count       (23) //「ひとのみりょくに　ついて」( 人数 )
#define RESEARCH_REQ_ID_PARTNER_time        (24) //「ひとのみりょくに　ついて」( 時間 )
#define RESEARCH_REQ_ID_SEASON_count        (25) //「きせつに　ついて」( 人数 )
#define RESEARCH_REQ_ID_SEASON_time         (26) //「きせつに　ついて」( 時間 )
#define RESEARCH_REQ_ID_ART_count           (27) //「げいじゅつに　ついて」( 人数 )
#define RESEARCH_REQ_ID_ART_time            (28) //「げいじゅつに　ついて」( 時間 )
#define RESEARCH_REQ_ID_MUSIC_count         (29) //「いちばんすきな　おんがくは？」( 人数 )
#define RESEARCH_REQ_ID_MUSIC_time          (30) //「いちばんすきな　おんがくは？」( 時間 )
#define RESEARCH_REQ_ID_MOVIE_count         (31) //「えいぞうに　ついて」( 人数 )
#define RESEARCH_REQ_ID_MOVIE_time          (32) //「えいぞうに　ついて」( 時間 )
#define RESEARCH_REQ_ID_SCHOOL_count        (33) //「いちばんすきな　がっこうのじかんは？」( 人数 )
#define RESEARCH_REQ_ID_SCHOOL_time         (34) //「いちばんすきな　がっこうのじかんは？」( 時間 )
#define RESEARCH_REQ_ID_STUDY_count         (35) //「べんきょうに　ついて」( 人数 )
#define RESEARCH_REQ_ID_STUDY_time          (36) //「べんきょうに　ついて」( 時間 )
#define RESEARCH_REQ_ID_STORTS_count        (37) //「いちばんにんきのある　スポーツは？」( 人数 )
#define RESEARCH_REQ_ID_STORTS_time         (38) //「いちばんにんきのある　スポーツは？」( 時間 )
#define RESEARCH_REQ_ID_DAY_OFF_count       (39) //「きゅうじつに　ついて」( 人数 )
#define RESEARCH_REQ_ID_DAY_OFF_time        (40) //「きゅうじつに　ついて」( 時間 )
#define RESEARCH_REQ_ID_DREAM_count         (41) //「いちばんなりたいものは？」( 人数 )
#define RESEARCH_REQ_ID_DREAM_time          (42) //「いちばんなりたいものは？」( 時間 )
#define RESEARCH_REQ_ID_GYM_LEADER_count    (43) //「いちばんにんきのある　ジムリーダーは？」( 人数 )
#define RESEARCH_REQ_ID_GYM_LEADER_time     (44) //「いちばんにんきのある　ジムリーダーは？」( 時間 )
#define RESEARCH_REQ_ID_FUN_count           (45) //「ポケモンのおもしろいところは？」( 人数 )
#define RESEARCH_REQ_ID_FUN_time            (46) //「ポケモンのおもしろいところは？」( 時間 )
#define RESEARCH_REQ_ID_NUM                 (47) // 総数
#define RESEARCH_REQ_ID_MAX                 (RESEARCH_REQ_ID_NUM-1) // 最大値

// アンケートID
#define RESEARCH_QUESTIONNAIRE_ID_YOU           (0) // どんなひとが　いるか
#define RESEARCH_QUESTIONNAIRE_ID_FAVARITE_POKE (1) // すきなポケモン
#define RESEARCH_QUESTIONNAIRE_ID_WISH          (2) // みんなの　りそう
#define RESEARCH_QUESTIONNAIRE_ID_PARTNER       (3) // すきなひと
#define RESEARCH_QUESTIONNAIRE_ID_TASTE         (4) // みんなの　このみ
#define RESEARCH_QUESTIONNAIRE_ID_HOBBY         (5) // みんなの　しゅみ
#define RESEARCH_QUESTIONNAIRE_ID_SCHOOL        (6) // がっこうせいかつ
#define RESEARCH_QUESTIONNAIRE_ID_PLAY          (7) // どんな　あそびがすきか
#define RESEARCH_QUESTIONNAIRE_ID_POKEMON       (8) // ポケモン
#define RESEARCH_QUESTIONNAIRE_ID_DUMMY         (9) // ダミー
