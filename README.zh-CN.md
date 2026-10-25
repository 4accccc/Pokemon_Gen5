# Pokémon Black/White & Black 2/White 2 开发仓库（SVN → Git）

[English](README.md) | [日本語](README.ja.md) | [**简体中文**](README.zh-CN.md)

本仓库是 Game Freak 内部 SVN 仓库 `swan_mirror` 的 Git 转换版本，包含《Pokémon Black/White》（BW）与《Pokémon Black 2/White 2》（B2W2）从引擎研发到发售、再到海外版修复的完整开发历史。

## 来源

- 原始 SVN：`svn://swan.gamefreak.co.jp/swan`（Game Freak 内部服务器）
- 本仓库来源：上述服务器的 `svnsync` 镜像 `swan_mirror`
- 原仓库 UUID：`51b7bd8e-9221-0410-b601-b096e935bcf1`
- 修订范围：r1 – r59,995
- 时间范围：2006-11-06 – 2012-10-09
- 提交作者：84 位（Git 作者名与提交名均为原 SVN 用户名；SVN 不保存邮箱，因此统一为 `用户名@localhost`）
- 原始 tags：**0 个**。SVN 的 `tags/` 目录始终为空，ROM 版本通过 `wb_tags_ver.txt` / `swan_tags_ver.txt` 手工记录

## 转换方式

- 工具：svn2git 2.4.0 + git-svn（Git for Windows 2.52），通过本地 `svnserve` 的 `svn://` 协议读取
- 布局：标准布局，SVN `trunk` 对应 Git 的 `trunk` 分支，`branches/*` → 同名本地分支
- 使用 `--no-metadata`：提交信息中**没有** `git-svn-id` 后缀，因此提交与 SVN revision 的对应关系未写入提交信息
- Git 不保存空目录，SVN 中的空目录（如 `rom/`）在转换后不保留
- 大文件：`tools/effecteditor/effecteditor.srl`（128MB，存在于历史中）已通过 `git lfs migrate import --above=100MB` 迁移到 Git LFS；该迁移重写了包含它的提交，因此 commit hash 与迁移前不同
- 转换完成日期：2026-10-25

## 分支说明

### trunk

- 对应 SVN `trunk`，即 **Pokémon Black/White（BW）**
- 43,218 个 SVN 派生提交；SVN 派生 tip `21650c1c24`（2010-09-13）；当前 trunk HEAD 是在此之上的 README 提交
- 工作区约 41,857 个 SVN 派生文件（不含 README）
- 第一个提交是 r1023（2008-01-29，"トランク等のリポジトリ管理ディレクトリを追加"）。该版本只创建了 `trunk/branches/tags` 三个空目录，因此在 Git 中是一个空提交；r1024/r1025 才把 `gflib`、`skel` 移入 trunk

### upper_version

- 对应 B2W2 的 `branches/upper_version`，内部称 "upper version"/"Swan"
- 58,519 个提交；HEAD `c800aa8e78`（2012-10-09，"育て屋用メッセージファイル、sodateya_scr.mstxtを追加"）
- 工作区约 55,034 个 SVN 派生文件
- 这是 SVN 中最后仍在更新的分支，因此仓库最新的代码状态在 `upper_version`，而不是 `trunk`

### pre-layout（**非原始分支，由本次转换额外构建**）

> **该分支不存在于原始 SVN 仓库中，是转换时为了保留前置历史而人工构建的。**

- 包含 **r1 – r1022**，共 **1,022 个提交**；当前 tip `59c2265e2c`（2008-01-24）
- 内容是 2008 年仓库布局变更前的**根目录结构**：`/gflib`、`/skel`、`/gf_library` 等
- 是一个**孤儿分支**，与 `trunk` 没有共同祖先；`git log` 首提交为 r1（2006-11-06，"test import"）
- 构建方式：
  - r1 – r1018 直接取自 git-svn 已抓取的根历史（`net@1018` 的祖先链）
  - r1019 – r1022 按 SVN 对应版本的树、作者、日期、提交信息，用 `git commit-tree` 重建
  - 末端树与 SVN r1022 的树完全一致（已校验 tree hash）
- 为什么需要它：r1023 才创建 `trunk/branches/tags`，r1024/r1025 才把 `gflib`、`skel` 移入 trunk。标准 svn2git 只跟踪 `trunk`，因此 `trunk` 从 r1023 开始；`pre-layout` 用于补齐 r1 – r1022 这段早期历史
- 使用注意：它与 `trunk` 是两条独立的根，`git log --all` 会看到两条历史线；不要把它当作 `trunk` 的直接祖先

### 原始 SVN 分支（17 个）

其中 **13 个在 SVN HEAD 时仍存在**，**4 个已被删除**（`dungeon`、`net`、`cpptest`、`200912rom`）。Git 中全部保留为本地分支。

| Git 分支 | 创建版本 | 复制来源 | 用途 | HEAD 日期 | SVN 派生提交数 |
|---|---|---|---|---|---|
| `trunk` | r1023 | — | SVN trunk，BW 主线 | 2010-09-13 | 43,218 |
| `dungeon` | r1026 | `/trunk:1025` | 3D 地牢游戏原型（2008，已删除） | 2008-04-04 | 40 |
| `net` | r1028 | `/:1018` | DS 无线通信实验（2008，已删除） | 2008-03-10 | 1,024 |
| `cpptest` | r1070 | `/trunk:1055` | C → C++ 迁移实验（2008，已删除） | 2008-04-10 | 9 |
| `200912rom` | r16272 | `/trunk:16271` | Mario Club 年末 ROM 临时分支（当天删除） | 2009-12-24 | 15,202 |
| `rom20091222` | r16275 | `/trunk:16270` | 任天堂 QA（Mario Club）年末 ROM | 2010-01-29 | 15,416 |
| `playable` | r42492 | `/trunk:42491` | 店铺试玩台（kiosk demo） | 2010-06-30 | 41,288 |
| `playable_merge` | r43643 | `/trunk:43642` | 试玩版本合并 | 2010-07-07 | 42,285 |
| `bugfix` | r44550 | `/trunk:44549` | 母盘前缺陷修复 | 2010-07-20 | 43,218 |
| `bugfix_100719` | r44607 | `/branches/bugfix:44606` | 以 0719 为基准的缺陷修复 | 2010-07-20 | 43,166 |
| `fieldtest` | r44609 | `/trunk:44596` | 现场测试 ROM | 2010-07-23 | 43,207 |
| `master_rom` | r44614 | `/trunk:44596` | 制品母盘（master ROM） | 2010-07-23 | 43,206 |
| `master_2nd` | r44616 | `/trunk:44615` | 第二批次生产 | 2010-07-30 | 43,246 |
| `upper_version` | r44646 | `/branches/master_2nd:44645` | B2W2（Swan）主线 | 2012-10-09 | 58,519 |
| `localize_version` | r44796 | `/branches/upper_version:44795` | 本地化素材 | 2010-09-29 | 43,395 |
| `swan_comment` | r59915 | `/branches/upper_version:59914` | Swan 注释追加 | 2012-04-04 | 58,525 |
| `swan_bugfix` | r59916 | `/branches/upper_version:59915` | 海外版不具合修复 | 2012-07-05 | 58,548 |
| `fes_rom` | r59917 | `/branches/upper_version:59916` | フェスミッション（Funfest Mission）配信 ROM | 2012-06-20 | 58,536 |
| `pre-layout` | — | 本次转换构建 | **非原始分支**，r1 – r1022 根布局历史 | 2008-01-24 | 1,022 |

> “SVN 派生提交数”包含共享祖先，因此分支之间的数字不可直接相加；`trunk` 另有 1 个 README 提交，当前 HEAD 比表中 tip 多 1 个提交。

## 仓库统计

- Git 分支总数：**19**（`trunk` + 17 个原始 SVN 分支 + `pre-layout`）
- 原始 SVN 分支：17 个（13 个存活 + 4 个已删除）
- 所有分支的提交并集：**59,994**
- SVN 修订总数：59,995（r1 – r59,995；少数修订没有可跟踪的文件变化，因此 Git 提交数略少）
- 作者：84
- Tag：0
- Git 对象：738,649；pack 体积约 2.46 GiB
- Git LFS：1 个对象，约 128MB（`tools/effecteditor/effecteditor.srl`）
- `trunk` 文件数：41,857
- `upper_version` 文件数：55,034

## 目录结构（以 trunk 为例）

```
prog/        游戏代码（battle、field、net、savedata、music、waza_tool 等）
resource/    游戏资源（约 180 个功能目录，26,979 个文件）
lib/         NitroSDK / TwlSDK / GFLIB / AMProtect 等库与头文件
tools/       开发工具（大量 Windows 可执行工具）
data/        数据表
wii/         Wii C++ Stationery 模板（不是 Wii 游戏）
rom/         SVN 中为空目录，Git 不保留
```

## 构建

- 需要 2010 年代 NDS/DSi 工具链：CodeWarrior for DSi 1.2（mwccarm / mwldarm）、NitroSDK / TwlSDK 5.3 patch5、Cygwin、Ruby 1.8.7，以及 makerom 等辅助工具
- 构建配置在 `prog/spec/`（`version_b` / `version_w` / `version_br` / `version_wr` 等）
- `trunk` 对应 BW，`upper_version` 对应 B2W2；两套 spec 文件名相同，建议用 `git worktree` 分别检出后编译
- 仓库本身**不包含完整的授权编译工具链**（编译器、makerom 等需自行准备）
- 不要在仓库工作区内直接编译：会产生大量未跟踪的 `obj/bin/narc/naix` 与 `prog/spec/version` 改动，破坏工作区纯净

## Git LFS

- 仓库历史中 128MB 的 `tools/effecteditor/effecteditor.srl` 已迁移到 Git LFS
- 迁移命令：`git lfs migrate import --above=100MB`；该操作重写了 59,995 个提交，因此迁移前后的 commit hash 不同
- 克隆后需要 `git lfs install` 并 `git lfs pull` 才能取回该文件；否则工作区里只是指针文件
- `.gitattributes` 规则：`/tools/effecteditor/effecteditor.srl filter=lfs diff=lfs merge=lfs -text`

## 已知限制

- 原始 SVN 没有 tag，本仓库也没有 tag
- 空目录不保留（Git 限制）
- 提交信息不含 `git-svn-id`，无法直接从提交看出对应 SVN revision；分支创建版本见上表，`pre-layout` 的范围为 r1 – r1022
- 源码中包含 Game Freak 员工的公司邮箱（历史注释，未清除）
- 大量文本为 Shift-JIS 编码；大量二进制资源（`.narc`、`.imd`、`.sdat` 等）；仅超过 100MB 的文件使用 Git LFS，50–100MB 文件仍是普通 Git 对象（GitHub 会警告但不阻止）
- 源码中包含反盗版中间件（NetAgent AMProtect/DSProtect）及其调用代码，仅为历史存档

## 版权

仓库内容版权归 Game Freak / Nintendo 及相关权利人所有。本仓库仅用于历史存档、研究与 preservation，不授予任何使用、复制或分发许可。
