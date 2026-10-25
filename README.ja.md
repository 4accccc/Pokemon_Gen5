# ポケモン ブラック/ホワイト & ブラック2/ホワイト2 開発リポジトリ（SVN → Git）

[English](README.md) | [**日本語**](README.ja.md) | [简体中文](README.zh-CN.md)

本リポジトリは、Game Freak 社内 SVN リポジトリ `swan_mirror` の Git 変換版です。『ポケモン ブラック/ホワイト』（BW）と『ポケモン ブラック2/ホワイト2』（B2W2）の、エンジン開発から発売、海外版修正までの開発履歴を収録しています。

## 出典

- 元 SVN：`svn://swan.gamefreak.co.jp/swan`（Game Freak 社内サーバー）
- 本リポジトリの元：上記サーバーの `svnsync` ミラー `swan_mirror`
- 元リポジトリ UUID：`51b7bd8e-9221-0410-b601-b096e935bcf1`
- リビジョン範囲：r1 – r59,995
- 期間：2006-11-06 – 2012-10-09
- 作者：84 名（Git の author / committer は元の SVN ユーザー名。SVN はメールアドレスを保存しないため、すべて `ユーザー名@localhost`）
- 元のタグ：**0 個**。SVN の `tags/` は常に空で、ROM のバージョンは `wb_tags_ver.txt` / `swan_tags_ver.txt` に手動で記録されていました

## 変換方法

- ツール：svn2git 2.4.0 + git-svn（Git for Windows 2.52）。ローカルの `svnserve` に `svn://` で接続
- レイアウト：標準レイアウト。SVN `trunk` が Git の `trunk` ブランチ、`branches/*` → 同名のローカルブランチ
- `--no-metadata`：コミットメッセージに `git-svn-id` は含まれず、コミットと SVN リビジョンの対応は履歴に埋め込まれていません
- Git は空ディレクトリを追跡しないため、SVN の空ディレクトリ（`rom/` など）は保持されていません
- 大容量ファイル：履歴に存在する `tools/effecteditor/effecteditor.srl`（128MB）は `git lfs migrate import --above=100MB` で Git LFS に移行しました。この処理は該当ファイルを含むコミットを書き換えるため、移行前後でコミットハッシュが異なります
- 変換日：2026-10-25

## ブランチ

### trunk

- SVN の `trunk` に対応。つまり **ポケモン ブラック/ホワイト（BW）**
- SVN 由来のコミット 43,218 個。SVN 由来の tip は `21650c1c24`（2010-09-13）。現在の `trunk` HEAD はその上に載った README コミット
- 作業ツリーには SVN 由来のファイルが約 41,857 個（README を除く）
- 最初のコミットは r1023（2008-01-29、「トランク等のリポジトリ管理ディレクトリを追加」）。このリビジョンは空の `trunk/branches/tags` を作成しただけなので Git では空コミットになります。r1024/r1025 で `gflib` と `skel` が `trunk` に移動されました

### upper_version

- B2W2 の `branches/upper_version` に対応。社内では「upper version」/「Swan」と呼ばれていました
- コミット 58,519 個。HEAD は `c800aa8e78`（2012-10-09、「育て屋用メッセージファイル、sodateya_scr.mstxtを追加」）
- 作業ツリーには SVN 由来のファイルが約 55,034 個
- SVN で最後まで更新されていたブランチのため、最新のコード状態は `trunk` ではなく `upper_version` にあります

### pre-layout（**元のブランチではなく、本変換で追加**）

> **このブランチは元の SVN リポジトリには存在しません。初期履歴を保持するために変換時に構築したものです。**

- **r1 – r1022**、**1,022 コミット**を収録。現在の tip は `59c2265e2c`（2008-01-24）
- 内容は 2008 年のリポジトリ再編前の**ルート直下レイアウト**：`/gflib`、`/skel`、`/gf_library` など
- `trunk` と共通の祖先を持たない**孤立ブランチ**。最初のコミットは r1（2006-11-06、「test import」）
- 構築方法：
  - r1 – r1018 は git-svn が取得済みのルート履歴（`net@1018` の祖先）をそのまま使用
  - r1019 – r1022 は、対応する SVN リビジョンのツリー・作者・日付・メッセージを用いて `git commit-tree` で再構築
  - 末端のツリーは SVN r1022 のツリーと完全に一致（tree hash で検証済み）
- 必要な理由：r1023 でようやく `trunk/branches/tags` が作られ、r1024/r1025 で `gflib` と `skel` が `trunk` に移動しました。標準の svn2git は `trunk` のみを追跡するため `trunk` は r1023 から始まりますが、`pre-layout` が r1 – r1022 を補完します
- 注意：`trunk` とは別のルートです。`git log --all` では履歴が 2 本見えます。`trunk` の直接の祖先として扱わないでください

### 元の SVN ブランチ（17 個）

**13 個は SVN HEAD 時に存在**し、**4 個は削除済み**（`dungeon`、`net`、`cpptest`、`200912rom`）。すべてローカルブランチとして保持しています。

| Git ブランチ | 作成リビジョン | コピー元 | 用途 | HEAD 日付 | SVN 由来コミット数 |
|---|---|---|---|---|---|
| `trunk` | r1023 | — | SVN trunk、BW 本線 | 2010-09-13 | 43,218 |
| `dungeon` | r1026 | `/trunk:1025` | 3D ダンジョンゲームのプロトタイプ（2008、削除済み） | 2008-04-04 | 40 |
| `net` | r1028 | `/:1018` | DS 無線通信の実験（2008、削除済み） | 2008-03-10 | 1,024 |
| `cpptest` | r1070 | `/trunk:1055` | C → C++ 移行実験（2008、削除済み） | 2008-04-10 | 9 |
| `200912rom` | r16272 | `/trunk:16271` | マリオクラブ向け年末 ROM の一時ブランチ（当日削除） | 2009-12-24 | 15,202 |
| `rom20091222` | r16275 | `/trunk:16270` | 任天堂 QA（マリオクラブ）年末 ROM | 2010-01-29 | 15,416 |
| `playable` | r42492 | `/trunk:42491` | 店頭試遊台（kiosk demo） | 2010-06-30 | 41,288 |
| `playable_merge` | r43643 | `/trunk:43642` | 試遊版のマージ | 2010-07-07 | 42,285 |
| `bugfix` | r44550 | `/trunk:44549` | マスター前の不具合修正 | 2010-07-20 | 43,218 |
| `bugfix_100719` | r44607 | `/branches/bugfix:44606` | 0719 版を基準にした不具合修正 | 2010-07-20 | 43,166 |
| `fieldtest` | r44609 | `/trunk:44596` | フィールドテスト ROM | 2010-07-23 | 43,207 |
| `master_rom` | r44614 | `/trunk:44596` | 製品マスター ROM | 2010-07-23 | 43,206 |
| `master_2nd` | r44616 | `/trunk:44615` | 第二次生産分 | 2010-07-30 | 43,246 |
| `upper_version` | r44646 | `/branches/master_2nd:44645` | B2W2（Swan）本線 | 2012-10-09 | 58,519 |
| `localize_version` | r44796 | `/branches/upper_version:44795` | ローカライズ素材 | 2010-09-29 | 43,395 |
| `swan_comment` | r59915 | `/branches/upper_version:59914` | Swan コメント追加 | 2012-04-04 | 58,525 |
| `swan_bugfix` | r59916 | `/branches/upper_version:59915` | 海外版不具合修正 | 2012-07-05 | 58,548 |
| `fes_rom` | r59917 | `/branches/upper_version:59916` | フェスミッション配信 ROM | 2012-06-20 | 58,536 |
| `pre-layout` | — | 本変換で追加 | **元ブランチではない**。r1 – r1022 のルートレイアウト履歴 | 2008-01-24 | 1,022 |

> 「SVN 由来コミット数」は共通祖先を含むため、ブランチ間で単純に合計できません。`trunk` には README コミットが 1 個あるため、現在の HEAD は表の tip より 1 コミット先です。

### 追加のブラック/ホワイト開発ライン・ブランチ（3 個）

これらのブランチは、r44,799 で本線から分岐した並行するブラック/ホワイト開発ラインに由来します。固有なのは r44,800–r44,832 のみで、それ以前の履歴は `trunk` / `upper_version` と共通です。

| Git ブランチ | 作成リビジョン | ベース | 用途 | HEAD 日付 | コミット数（共通祖先を含む） |
|---|---|---|---|---|---|
| `wb-trunk` | r44,827 | `trunk` @ r44,799 | 追加のブラック/ホワイト本線（分岐後のバトル AI 修正） | 2012-02-20 | 43,220 |
| `wb-upper_version` | r44,800 | `upper_version` @ r44,799 | 並行ラインの upper_version（B2W2）修正 | 2010-12-08 | 43,420 |
| `wb-gpower` | r44,800 | `trunk` @ r44,799 | G-Power 配信 ROM ビルド | 2012-03-28 | 43,226 |

> これら 3 ブランチは**追加のブラック/ホワイト・ソースブランチ**であり、並行ラインの r44,800–r44,832 から再構築したものです。元の SVN ブランチ 17 個には含まれません。

## リポジトリ統計

- Git ブランチ数：**22**（`trunk` + 元の SVN ブランチ 17 個 + 追加のブラック/ホワイト・ブランチ 3 個 + `pre-layout`）
- 元の SVN ブランチ：17 個（現存 13 + 削除済み 4）
- 追加のブラック/ホワイト・ブランチ：3 個（r44,800–r44,832、固有コミット 33 個）
- 全ブランチのコミット和集合：**60,028**
- SVN リビジョン数：59,995（r1 – r59,995。追跡可能なファイル変更がないリビジョンが少数あるため、Git のコミット数はわずかに少ない）
- 並行ブラック/ホワイトライン：r44,800–r44,832（33 リビジョン、`wb-*` ブランチとして再構築）
- 作者：84 名
- タグ：0
- Git オブジェクト：739,029。pack サイズ約 2.46 GiB
- Git LFS：1 オブジェクト、約 128MB（`tools/effecteditor/effecteditor.srl`）
- `trunk` のファイル数：41,857
- `upper_version` のファイル数：55,034

## ディレクトリ構成（trunk）

```
prog/        ゲームコード（battle、field、net、savedata、music、waza_tool など）
resource/    ゲームリソース（約 180 の機能ディレクトリ、26,979 ファイル）
lib/         NitroSDK / TwlSDK / GFLIB / AMProtect のヘッダーとライブラリ
tools/       開発ツール（多数の Windows 実行ファイル）
data/        データテーブル
wii/         Wii C++ Stationery テンプレート（Wii ゲームではない）
rom/         SVN では空ディレクトリ。Git では保持されない
```

## ビルド

- 2010 年代の NDS/DSi ツールチェーンが必要：CodeWarrior for DSi 1.2（mwccarm / mwldarm）、NitroSDK / TwlSDK 5.3 patch5、Cygwin、Ruby 1.8.7、および makerom などの補助ツール
- ビルド設定は `prog/spec/`（`version_b`、`version_w`、`version_br`、`version_wr` など）
- `trunk` は BW、`upper_version` は B2W2。spec のファイル名は同じため、`git worktree` で別々にチェックアウトしてビルドすることを推奨
- 本リポジトリには**完全なライセンス済みツールチェーンは含まれません**（コンパイラや makerom などは別途用意する必要があります）
- リポジトリの作業ツリー内で直接ビルドしないでください。未追跡の `obj/bin/narc/naix` や `prog/spec/version` の変更が大量に発生し、作業ツリーが汚れます

## Git LFS

- 履歴中の 128MB の `tools/effecteditor/effecteditor.srl` は Git LFS に保存されています
- 移行コマンド：`git lfs migrate import --above=100MB`。59,995 コミットを書き換えたため、移行前後でコミットハッシュが異なります
- クローン後は `git lfs install` と `git lfs pull` を実行しないと、作業ツリーにはポインタファイルしか存在しません
- `.gitattributes` のルール：`/tools/effecteditor/effecteditor.srl filter=lfs diff=lfs merge=lfs -text`

## 既知の制限

- 元の SVN にタグはなく、本リポジトリにもタグはありません
- 空ディレクトリは保持されません（Git の制限）
- コミットメッセージに `git-svn-id` がないため、対応する SVN リビジョンを直接確認できません。ブランチ作成リビジョンは上表のとおりで、`pre-layout` は r1 – r1022 を対象とします
- ソースには Game Freak 社員の会社メールアドレスが履歴コメントとして含まれています（削除していません）
- テキストの多くは Shift-JIS です。バイナリ資産（`.narc`、`.imd`、`.sdat` など）が多数あります。100MB 超のファイルのみ Git LFS を使用し、50–100MB のファイルは通常の Git オブジェクトのままです（GitHub は警告を出しますが受け付けます）
- `wb-*` の 3 ブランチは、r44,799 で分岐した並行ブラック/ホワイトラインから再構築したものです。コミットメタデータは保持していますが、分岐後のハッシュは本線と異なります
- ソースには NetAgent の AMProtect/DSProtect 反海賊版ミドルウェアとその呼び出しコードが含まれています（歴史的保存目的）

## 著作権

すべての内容の著作権は Game Freak / Nintendo および各権利者に帰属します。本リポジトリは歴史的保存・研究・preservation のみを目的として提供され、使用・複製・配布の許諾を与えるものではありません。
