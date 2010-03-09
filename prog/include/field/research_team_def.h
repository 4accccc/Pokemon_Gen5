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
#define RESEARCH_MEMBER_RANK_C  (0) // ランクC
#define RESEARCH_MEMBER_RANK_B  (1) // ランクB
#define RESEARCH_MEMBER_RANK_A  (2) // ランクA
#define RESEARCH_MEMBER_RANK_S  (3) // ランクS
#define RESEARCH_MEMBER_RANK_SS (4) // ランクSS

// 調査依頼ID
#define RESEARCH_REQ_ID_WB_count             (0) //「ブラックと　ホワイト　おおいのは？」( 人数 )
#define RESEARCH_REQ_ID_WB_time              (1) //「ブラックと　ホワイト　おおいのは？」( 時間 )
#define RESEARCH_REQ_ID_FIRST_POKE_count     (2) //「えらんだ　さいしょのポケモンは？」( 人数 )
#define RESEARCH_REQ_ID_FIRST_POKE_time      (3) //「えらんだ　さいしょのポケモンは？」( 時間 )
#define RESEARCH_REQ_ID_PLAY_TIME_count      (4) //「いちばんおおい　プレイじかんは？」( 人数 )
#define RESEARCH_REQ_ID_PLAY_TIME_time       (5) //「いちばんおおい　プレイじかんは？」( 時間 )
#define RESEARCH_REQ_ID_SEX_count            (6) //「おとこのこと　おんなのこ　おおいのは？」( 人数 )
#define RESEARCH_REQ_ID_SEX_time             (7) //「おとこのこと　おんなのこ　おおいのは？」( 時間 )
#define RESEARCH_REQ_ID_JOB_count            (8) //「いちばんおおい　しごとは？」( 人数 )
#define RESEARCH_REQ_ID_JOB_time             (9) //「いちばんおおい　しごとは？」( 時間 )
#define RESEARCH_REQ_ID_HOBBY_count         (10) //「いちばんおおい　しゅみは？」( 人数 )
#define RESEARCH_REQ_ID_HOBBY_time          (11) //「いちばんおおい　しゅみは？」( 時間 )
#define RESEARCH_REQ_ID_BTL_OR_TRD_count    (12) //「たいせんと　こうかん　にんきがあるのは？」( 人数 )
#define RESEARCH_REQ_ID_BTL_OR_TRD_time     (13) //「たいせんと　こうかん　にんきがあるのは？」( 時間 )
#define RESEARCH_REQ_ID_FAVORITE_POKE_count (14) //「すきな　ポケモンは？」( 人数 )
#define RESEARCH_REQ_ID_FAVORITE_POKE_time  (15) //「すきな　ポケモンは？」( 時間 )
#define RESEARCH_REQ_ID_POKE_TYPE_count     (16) //「すきな　ポケモンのタイプは？」( 人数 )
#define RESEARCH_REQ_ID_POKE_TYPE_time      (17) //「すきな　ポケモンのタイプは？」( 時間 )
#define RESEARCH_REQ_ID_PLACE_count         (18) //「とかいと　いなか　すみたいのは？」( 人数 )
#define RESEARCH_REQ_ID_PLACE_time          (19) //「とかいと　いなか　すみたいのは？」( 時間 )
#define RESEARCH_REQ_ID_PRECIOUS_count      (20) //「だいじにしているものに　ついて」( 人数 )
#define RESEARCH_REQ_ID_PRECIOUS_time       (21) //「だいじにしているものに　ついて」( 時間 )
#define RESEARCH_REQ_ID_PARTNER_count       (22) //「ひとのみりょくに　ついて」( 人数 )
#define RESEARCH_REQ_ID_PARTNER_time        (23) //「ひとのみりょくに　ついて」( 時間 )
#define RESEARCH_REQ_ID_SEASON_count        (24) //「きせつに　ついて」( 人数 )
#define RESEARCH_REQ_ID_SEASON_time         (25) //「きせつに　ついて」( 時間 )
#define RESEARCH_REQ_ID_ART_count           (26) //「げいじゅつに　ついて」( 人数 )
#define RESEARCH_REQ_ID_ART_time            (27) //「げいじゅつに　ついて」( 時間 )
#define RESEARCH_REQ_ID_MUSIC_count         (28) //「いちばんすきな　おんがくは？」( 人数 )
#define RESEARCH_REQ_ID_MUSIC_time          (29) //「いちばんすきな　おんがくは？」( 時間 )
#define RESEARCH_REQ_ID_MOVIE_count         (30) //「えいぞうに　ついて」( 人数 )
#define RESEARCH_REQ_ID_MOVIE_time          (31) //「えいぞうに　ついて」( 時間 )
#define RESEARCH_REQ_ID_SCHOOL_count        (32) //「いちばんすきな　がっこうのじかんは？」( 人数 )
#define RESEARCH_REQ_ID_SCHOOL_time         (33) //「いちばんすきな　がっこうのじかんは？」( 時間 )
#define RESEARCH_REQ_ID_STUDY_count         (34) //「べんきょうに　ついて」( 人数 )
#define RESEARCH_REQ_ID_STUDY_time          (35) //「べんきょうに　ついて」( 時間 )
#define RESEARCH_REQ_ID_STORTS_count        (36) //「いちばんにんきのある　スポーツは？」( 人数 )
#define RESEARCH_REQ_ID_STORTS_time         (37) //「いちばんにんきのある　スポーツは？」( 時間 )
#define RESEARCH_REQ_ID_DAY_OFF_count       (38) //「きゅうじつに　ついて」( 人数 )
#define RESEARCH_REQ_ID_DAY_OFF_time        (39) //「きゅうじつに　ついて」( 時間 )
#define RESEARCH_REQ_ID_DREAM_count         (40) //「いちばんなりたいものは？」( 人数 )
#define RESEARCH_REQ_ID_DREAM_time          (41) //「いちばんなりたいものは？」( 時間 )
#define RESEARCH_REQ_ID_GYM_LEADER_count    (42) //「いちばんにんきのある　ジムリーダーは？」( 人数 )
#define RESEARCH_REQ_ID_GYM_LEADER_time     (43) //「いちばんにんきのある　ジムリーダーは？」( 時間 )
#define RESEARCH_REQ_ID_FUN_count           (44) //「ポケモンのおもしろいところは？」( 人数 )
#define RESEARCH_REQ_ID_FUN_time            (45) //「ポケモンのおもしろいところは？」( 時間 )
#define RESEARCH_REQ_ID_NUM                 (46) // 総数

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
