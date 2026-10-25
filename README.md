# Pokémon Black/White & Black 2/White 2 Development Repository (SVN → Git)

[**English**](README.md) | [日本語](README.ja.md) | [简体中文](README.zh-CN.md)

This repository is a Git conversion of Game Freak's internal SVN repository `swan_mirror`. It contains the development history of *Pokémon Black/White* (BW) and *Pokémon Black 2/White 2* (B2W2), from engine R&D through release and overseas bug fixes.

## Source

- Original SVN: `svn://swan.gamefreak.co.jp/swan` (Game Freak internal server)
- This repository is based on the `svnsync` mirror `swan_mirror`
- Original repository UUID: `51b7bd8e-9221-0410-b601-b096e935bcf1`
- Revision range: r1 – r59,995
- Date range: 2006-11-06 – 2012-10-09
- Authors: 84 (Git author and committer names are the original SVN usernames; SVN does not store email addresses, so all emails are `username@localhost`)
- Original tags: **0**. The SVN `tags/` directory was always empty; ROM versions were recorded manually in `wb_tags_ver.txt` / `swan_tags_ver.txt`

## Conversion

- Tools: svn2git 2.4.0 + git-svn (Git for Windows 2.52), reading through a local `svnserve` over `svn://`
- Layout: standard layout; SVN `trunk` is the Git `trunk` branch; `branches/*` → local branches with the same names
- `--no-metadata`: commit messages do **not** contain `git-svn-id`, so the mapping between commits and SVN revisions is not embedded in the history
- Git does not track empty directories, so SVN empty directories (e.g. `rom/`) are not preserved
- Large files: `tools/effecteditor/effecteditor.srl` (128 MB, present in history) was migrated to Git LFS with `git lfs migrate import --above=100MB`; this rewrote the commits containing it, so commit hashes differ from the pre-migration repository
- Conversion date: 2026-10-25

## Branches

### trunk

- Corresponds to SVN `trunk`, i.e. **Pokémon Black/White (BW)**
- 43,218 SVN-derived commits; SVN-derived tip `21650c1c24` (2010-09-13); the current `trunk` HEAD is the README commit on top of it
- About 41,857 files of SVN-derived content in the working tree (plus the README files)
- The first commit is r1023 (2008-01-29, "トランク等のリポジトリ管理ディレクトリを追加"). That revision only created the empty `trunk/branches/tags` directories, so it is an empty commit in Git; r1024/r1025 then moved `gflib` and `skel` into `trunk`

### upper_version

- Corresponds to `branches/upper_version` for B2W2, internally called "upper version" / "Swan"
- 58,519 commits; HEAD `c800aa8e78` (2012-10-09, "育て屋用メッセージファイル、sodateya_scr.mstxtを追加" — added a message file for the Day Care)
- About 55,034 files of SVN-derived content in the working tree
- This was the last branch still updated in SVN, so the newest code state is on `upper_version`, not `trunk`

### pre-layout (**not an original branch; added by this conversion**)

> **This branch does not exist in the original SVN repository. It was constructed during the conversion to preserve the early history.**

- Contains **r1 – r1022**, **1,022 commits**; current tip `59c2265e2c` (2008-01-24)
- Content is the **root-level layout** before the 2008 repository reorganization: `/gflib`, `/skel`, `/gf_library`, etc.
- It is an **orphan branch** with no common ancestor with `trunk`; its first commit is r1 (2006-11-06, "test import")
- Construction:
  - r1 – r1018 comes from the root history already fetched by git-svn (the ancestry of `net@1018`)
  - r1019 – r1022 were reconstructed with `git commit-tree` using the tree, author, date and message of the corresponding SVN revisions
  - The final tree is identical to the SVN r1022 tree (tree hash verified)
- Why it exists: r1023 only created `trunk/branches/tags`, and r1024/r1025 moved `gflib` and `skel` into `trunk`. Standard svn2git tracks only `trunk`, so `trunk` starts at r1023; `pre-layout` fills in r1 – r1022
- Note: it is a separate root from `trunk`; `git log --all` shows two history lines. Do not treat it as a direct ancestor of `trunk`

### Original SVN branches (17)

**13 branches still existed at SVN HEAD**; **4 were deleted** (`dungeon`, `net`, `cpptest`, `200912rom`). All are preserved as local branches.

| Git branch | Created at | Copied from | Purpose | HEAD date | SVN-derived commits |
|---|---|---|---|---|---|
| `trunk` | r1023 | — | SVN trunk, BW mainline | 2010-09-13 | 43,218 |
| `dungeon` | r1026 | `/trunk:1025` | 3D dungeon game prototype (2008, deleted) | 2008-04-04 | 40 |
| `net` | r1028 | `/:1018` | DS wireless communication experiment (2008, deleted) | 2008-03-10 | 1,024 |
| `cpptest` | r1070 | `/trunk:1055` | C → C++ migration experiment (2008, deleted) | 2008-04-10 | 9 |
| `200912rom` | r16272 | `/trunk:16271` | Temporary Mario Club year-end ROM branch (deleted the same day) | 2009-12-24 | 15,202 |
| `rom20091222` | r16275 | `/trunk:16270` | Nintendo QA (Mario Club) year-end ROM | 2010-01-29 | 15,416 |
| `playable` | r42492 | `/trunk:42491` | Retail kiosk demo | 2010-06-30 | 41,288 |
| `playable_merge` | r43643 | `/trunk:43642` | Demo version merge | 2010-07-07 | 42,285 |
| `bugfix` | r44550 | `/trunk:44549` | Bug fixes before the master ROM | 2010-07-20 | 43,218 |
| `bugfix_100719` | r44607 | `/branches/bugfix:44606` | Bug fixes based on the 0719 build | 2010-07-20 | 43,166 |
| `fieldtest` | r44609 | `/trunk:44596` | Field test ROM | 2010-07-23 | 43,207 |
| `master_rom` | r44614 | `/trunk:44596` | Production master ROM | 2010-07-23 | 43,206 |
| `master_2nd` | r44616 | `/trunk:44615` | Second production batch | 2010-07-30 | 43,246 |
| `upper_version` | r44646 | `/branches/master_2nd:44645` | B2W2 (Swan) mainline | 2012-10-09 | 58,519 |
| `localize_version` | r44796 | `/branches/upper_version:44795` | Localization assets | 2010-09-29 | 43,395 |
| `swan_comment` | r59915 | `/branches/upper_version:59914` | Swan comment additions | 2012-04-04 | 58,525 |
| `swan_bugfix` | r59916 | `/branches/upper_version:59915` | Overseas bug fixes | 2012-07-05 | 58,548 |
| `fes_rom` | r59917 | `/branches/upper_version:59916` | Funfest Mission distribution ROM | 2012-06-20 | 58,536 |
| `pre-layout` | — | Added by this conversion | **Not an original branch**; r1 – r1022 root-layout history | 2008-01-24 | 1,022 |

> "SVN-derived commits" includes shared ancestors, so the numbers cannot be added directly. `trunk` also has one README commit, so its current HEAD is one commit ahead of the tip shown above.

## Repository statistics

- Git branches: **19** (`trunk` + 17 original SVN branches + `pre-layout`)
- Original SVN branches: 17 (13 alive + 4 deleted)
- Union of commits across all branches: **59,994**
- SVN revisions: 59,995 (r1 – r59,995; a few revisions had no trackable file changes, so the Git commit count is slightly lower)
- Authors: 84
- Tags: 0
- Git objects: 738,649; pack size about 2.46 GiB
- Git LFS: 1 object, about 128 MB (`tools/effecteditor/effecteditor.srl`)
- `trunk` files: 41,857
- `upper_version` files: 55,034

## Directory layout (trunk)

```
prog/        Game code (battle, field, net, savedata, music, waza_tool, etc.)
resource/    Game resources (about 180 feature directories, 26,979 files)
lib/         NitroSDK / TwlSDK / GFLIB / AMProtect headers and libraries
tools/       Development tools (many Windows executables)
data/        Data tables
wii/         Wii C++ Stationery template (not a Wii game)
rom/         Empty in SVN; not preserved by Git
```

## Building

- Requires a 2010-era NDS/DSi toolchain: CodeWarrior for DSi 1.2 (mwccarm / mwldarm), NitroSDK / TwlSDK 5.3 patch5, Cygwin, Ruby 1.8.7, and helper tools such as makerom
- Build configuration is in `prog/spec/` (`version_b`, `version_w`, `version_br`, `version_wr`, etc.)
- `trunk` is BW and `upper_version` is B2W2; both use the same spec file names, so use separate `git worktree` checkouts for builds
- The repository does **not** include a complete licensed toolchain (the compiler, makerom, etc. must be supplied separately)
- Do not build inside the repository working tree: it creates many untracked `obj/bin/narc/naix` files and modifies `prog/spec/version`, polluting the worktree

## Git LFS

- The 128 MB `tools/effecteditor/effecteditor.srl` in history is stored in Git LFS
- Migration command: `git lfs migrate import --above=100MB`; it rewrote 59,995 commits, so commit hashes differ before and after migration
- After cloning, run `git lfs install` and `git lfs pull` to fetch the file; otherwise the working tree contains only a pointer
- `.gitattributes` rule: `/tools/effecteditor/effecteditor.srl filter=lfs diff=lfs merge=lfs -text`

## Known limitations

- The original SVN has no tags, and this repository has no tags either
- Empty directories are not preserved (Git limitation)
- Commit messages do not contain `git-svn-id`, so the corresponding SVN revision is not directly visible; branch creation revisions are listed above, and `pre-layout` covers r1 – r1022
- The source contains Game Freak employees' corporate email addresses (historical comments, not removed)
- Much of the text is Shift-JIS encoded; there are many binary assets (`.narc`, `.imd`, `.sdat`, etc.); only files over 100 MB use Git LFS, while 50–100 MB files remain normal Git objects (GitHub warns about them but accepts them)
- The source contains the NetAgent AMProtect/DSProtect anti-piracy middleware and its call sites, preserved for historical archiving

## Copyright

All content is copyright Game Freak / Nintendo and their respective rights holders. This repository is provided for historical archiving, research and preservation only; no license to use, copy or distribute is granted.
