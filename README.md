# gameWE - AMX Mod X plugin

> **Fork:** This project is a fork of the gameME AMX Mod X plugin
> ([gamemedev/plugin-amxmodx](https://github.com/gamemedev/plugin-amxmodx)), version 2.4, by TTS Oetzel & Goerz GmbH.

The gameWE AMX Mod X plugin runs on Half-Life 1 (GoldSrc) game servers and works
together with the gameWE collector. It is the HL1 counterpart of the
[gameWE SourceMod plugin](https://github.com/DennisPohlmann/gameWE-plugin-sourcemod). It

- writes extra statistics to the server log: per-weapon accuracy and hit groups
  (`weaponstats`, `weaponstats2`), kill positions, headshots and bullet hits
  (Counter-Strike),
- offers the server commands the collector uses to answer players in game
  (`gamewe_amx_psay`, `gamewe_amx_msay`, ...),
- hides the stats chat commands (`rank`, `/top10`, ...) from chat, but still
  logs them for the collector,
- shows the gameWE menu (`/gamewe`).

The log format and the command interface stay compatible with gameME.

## Contents

1. [Supported games](#supported-games)
2. [Install](#install) (details in [INSTALLATION.md](INSTALLATION.md))
3. [Configuration](#configuration)
4. [Chat commands](#chat-commands)
5. [Collector interface](#collector-interface)
6. [Migrating from the gameME plugin](#migrating-from-the-gameme-plugin)
7. [Building](#building)
8. [License](#license)

## Supported games

There is one plugin per game:

| Game | Plugin | AMX Mod X modules |
|------|--------|-------------------|
| Counter-Strike 1.6, Condition Zero | `gamewe_cstrike.amxx` | cstrike, csx, fakemeta, hamsandwich |
| Day of Defeat | `gamewe_dod.amxx` | dodx, dodfun, hamsandwich |
| Team Fortress Classic | `gamewe_tfc.amxx` | tfcx, hamsandwich |
| Natural Selection | `gamewe_ns.amxx` | - |
| Half-Life Deathmatch | `gamewe_hlmp.amxx` | - |

What each plugin logs in addition to the game's own log:

| | cstrike | dod | tfc | ns | hlmp |
|---|:-:|:-:|:-:|:-:|:-:|
| `weaponstats`, `weaponstats2` | yes | yes | yes | | |
| kill and suicide positions | yes | | | | |
| `headshot` action | yes | | | | |
| `attacked` lines (bullet hits) | yes | | | | |
| hidden chat commands, menu | yes | yes | yes | yes | yes |
| team swap | yes | yes | | | |
| colored chat | yes | | | | |

## Install

Requirements: [AMX Mod X](https://www.amxmodx.org/) **1.9 or newer** (1.10 recommended)
and the modules listed above.

In short: extract `gamewe-amxmodx-<version>.tar.gz` into the game folder (the folder with
`addons/`), add only the plugin of your game to `addons/amxmodx/configs/plugins.ini`,
set `log on` and `logaddress_add <collector ip> <port>` in `server.cfg` and change the map.

**[INSTALLATION.md](INSTALLATION.md)** has every step: checking the download, the game
folders of each game, game servers in a container (bind mount or own image), the
collector connection, checking, updating, uninstalling and troubleshooting.

## Configuration

| Cvar | Default | Meaning |
|------|---------|---------|
| `gamewe_message_prefix` | | Text put in front of every gameWE chat message. |
| `gamewe_chat_cooldown` | `1.0` | Seconds between two logged `/` commands of one player, so nobody can flood the collector. `0` turns it off. Commands in between are hidden but not logged. |
| `gamewe_log_damage` | `1` | Counter-Strike: log an `attacked` line for every bullet hit. |
| `gamewe_plugin_version` | | Read only, the plugin version. |

Server commands:

- `gamewe_block_commands_values "<command> <command> ..."` adds more chat commands that
  are silent with `/` (up to 32 per call), `gamewe_block_commands_values clear` removes
  all, including the defaults.
- `gamewe_message_prefix_clear` empties the prefix.

## Chat commands

Players type these in chat with `/` or `!` in front. The collector answers both, the
only difference is who sees the command (there is no option for this):

- `/rank` is **silent**: the plugin hides it from chat and logs it itself as a `say` line,
  for example `"Name<2><STEAM_0:0:123><CT>" say "/rank"`.
- `!rank` (and a plain `rank`) stays **visible** in chat, the game logs it as usual.

`rank`, `skill`, `points`, `place`, `session`, `sdata`, `kpd`, `kdratio`, `kdeath`,
`next`, `load`, `status`, `top5`, `top10`, `top20`, `maps`, `map_stats`, `clans`,
`cheaters`, `statsme`, `weapons`, `weapon`, `action`, `actions`, `accuracy`,
`targets`, `target`, `kills`, `kill`, `player_kills`, `cmds`, `commands`, `help`,
`gstats`, `global_stats`, `hideranking [1|0|on|off|hide|show]`,
`gamewe_hideranking [1|0|on|off|hide|show]`, and the old gameME settings commands
`gamewe_display 0|1`, `gamewe_chat 0|1`, `gamewe_atb 0|1`, `gamewe_reset`. The `gameme_*`
spellings are silent with `/` too.

`!hideranking` or `/gamewe_hideranking` hides the player in the ranking (`1`/`on`/`hide`),
shows them again (`0`/`off`/`show`) or toggles without an argument. The collector does this
and confirms it with a private message (collector 0.1.0 or newer).

`gamewe` or `gamewe_menu` (also `gameme`, `gameme_menu`) opens the gameWE menu. The
menu entries send the matching chat command for the player, for example `/rank`.
Counter-Strike and Day of Defeat get the full menu, the other games a short one
(rank, next, top10, settings).

`settings` or `gamewe_settings` in chat, or `amx_gamewe_settings` in the console, opens the
settings menu. The plugin hides `settings` and `gamewe_settings` and does not log them, so
the collector does not answer them as well. This is the only exception to the rule above. The menu has "Hide me in the ranking"
(sends `/gamewe_hideranking 1`) and "Show me in the ranking" (`/gamewe_hideranking 0`).

## Collector interface

### Log lines

The plugin writes standard HL log lines (`"Name<userid><authid><team>"`):

```
"Name<2><STEAM_0:0:123><CT>" triggered "weaponstats" (weapon "ak47") (shots "14") (hits "6") (kills "1") (headshots "1") (tks "0") (damage "412") (deaths "0")
"Name<2><STEAM_0:0:123><CT>" triggered "weaponstats2" (weapon "ak47") (head "1") (chest "3") (stomach "1") (leftarm "0") (rightarm "1") (leftleg "0") (rightleg "0")
"Name<2><STEAM_0:0:123><CT>" killed "Other<3><STEAM_0:1:456><TERRORIST>" with "ak47" (attacker_position "-1022 380 -60") (victim_position "-830 512 -60")
"Name<2><STEAM_0:0:123><CT>" triggered "headshot"
"Name<2><STEAM_0:0:123><CT>" attacked "Other<3><STEAM_0:1:456><TERRORIST>" with "ak47" (damage "36") (damage_armor "4") (health "64") (armor "96") (hitgroup "chest")
"Name<2><STEAM_0:0:123><CT>" say "/rank"
"Name<2><STEAM_0:0:123><CT>" selected "3"
```

- `weaponstats` lines are written when a player dies, disconnects or the round ends
  (Counter-Strike, Day of Defeat), each life only once. `weaponstats2` follows when the
  weapon hit something. The weapon name is the one of the kill lines.
- Counter-Strike: the plugin replaces the kill and suicide lines of the game by the
  same lines with positions, so each kill is still logged once.

### Server commands

The collector sends these over RCON. They are server commands, players cannot
use them. Every command exists in three spellings that do the same:
`gamewe_amx_<name>` (recommended), `gameme_amx_<name>` (gameME) and `gamewe_<name>`
(the spelling of the SourceMod plugin).

Players are addressed by their **userid**, the number in the first `<..>` of a log
line (`2` above), with or without `#` in front. Lists are separated by commas
(up to 32). Arguments may be quoted or not; the message is the rest of the line.

| Command | Does |
|---------|------|
| `gamewe_amx_psay <userid[,userid...]> [colored] "<message>"` | Private chat message. `colored 1` (Counter-Strike): the tags `x01` (default), `x03` (team color) and `x04` (green) become colors, the first player name gets its team color, other names green. |
| `gamewe_amx_psay2 <userid[,userid...]> [colored] "<message>"` | Private chat message, all green on Counter-Strike. |
| `gamewe_amx_say "<message>"` | Chat message to everybody. |
| `gamewe_amx_csay "<message>"` | Center message to everybody. |
| `gamewe_amx_hint <userid[,userid...]> "<message>"` | HUD message at the bottom. |
| `gamewe_amx_msay <time> <userid> [handler] "<menu text>"` | Menu for `time` seconds (1-120, otherwise 15). Lines are separated by `\n` or `^n`. With `handler 1` the choice is logged as `selected "<1-9>"` or `selected "cancel"`. |
| `gamewe_amx_browse <userid> "<url>"` | Opens a website in the MOTD window. Only `http://` and `https://` URLs. |
| `gamewe_amx_swap <userid>` | Moves a player to the other team (Counter-Strike, Day of Defeat) and logs `joined team`. |

## Migrating from the gameME plugin

- Remove `gameme_<game>.amxx` from `plugins.ini` and add `gamewe_<game>.amxx`. Do not
  run both plugins: both handle the `gameme_amx_*` commands and log the same events.
- The cvars are renamed from `gameme_*` to `gamewe_*` and live in
  `configs/plugins/gamewe.cfg`. `gameme_webpage` is gone.
- The server commands keep working with the `gameme_amx_` prefix.
- Log lines are the same as before, with two changes: weapon names in `weaponstats`
  are the log names of the kill lines (Day of Defeat used display names before), and
  `attacked` lines carry the real damage instead of `0`.

## Building

You need `amxxpc` from AMX Mod X 1.9 or newer. The easiest way is to build it from
the AMX Mod X sources (needs `git`, `gcc-multilib` and `g++-multilib`, because
`amxxpc` is a 32-bit program):

```bash
make compiler             # build/amxx-1.10/amxxpc and its includes
make build                # build/stage/addons/amxmodx/plugins/gamewe_*.amxx
make package              # dist/gamewe-amxmodx-<version>.tar.gz and .sha256
make compiler build AMXX=1.9   # check the build with AMX Mod X 1.9
```

To use an installed compiler instead: `make build AMXXPC=/path/to/amxxpc
AMXX_INCLUDE=/path/to/scripting/include` (`amxxpc32.so` must be next to `amxxpc`).
The version comes from the git tag (`v1.2.3`) or the `VERSION` file. Warnings are
treated as errors.

The GitHub workflow `.github/workflows/build.yml` builds every push to `main` with
AMX Mod X 1.9 and 1.10 and keeps the package as a workflow artifact.

### Releases

Releases are made by [semantic-release](https://semantic-release.gitbook.io/)
(`.releaserc.json`) in the same workflow; nobody tags by hand. To make a
release, push commits written as [Conventional Commits](https://www.conventionalcommits.org/)
to `main`. semantic-release reads the commits since the last tag and decides
the next version:

| Commit | Release |
|---|---|
| `feat: ...` | minor (0.1.0 → 0.2.0) |
| `fix: ...` (also `fix(security): ...`), `perf: ...` | patch (0.1.0 → 0.1.1) |
| `feat!: ...`, `fix!: ...` or a `BREAKING CHANGE:` footer | major (0.1.0 → 1.0.0) |
| `docs`, `chore`, `ci`, `build`, `test`, `refactor`, `style` | no release |

For a release it writes `VERSION` and `CHANGELOG.md`, commits them as
`chore(release): vX.Y.Z [skip ci]`, tags `vX.Y.Z` and creates the GitHub
release. The workflow then builds that tag with AMX Mod X 1.10 and uploads the
`.tar.gz` (the five `.amxx` files) and its `.sha256` to the release. Never edit
`VERSION` or `CHANGELOG.md` by hand.

Source layout: `addons/amxmodx/scripting/gamewe_<game>.sma` holds the game specific
code, `include/gamewe_core.inc` the commands, chat and menu shared by all games,
`include/gamewe_xstats.inc` the weapon statistics.

## License

gameWE is free software under the **GNU General Public License version 3 or later**,
see [LICENSE](LICENSE).

It is based on the gameME plugin for AMX Mod X, Copyright (C) 2007-2017
TTS Oetzel & Goerz GmbH, published under the GPL version 2 or later. The gameME
changelog up to 2.4 is kept in [docs/gameme-CHANGELOG.txt](docs/gameme-CHANGELOG.txt).
