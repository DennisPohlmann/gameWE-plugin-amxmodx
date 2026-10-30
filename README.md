# gameWE - AMX Mod X plugin

> **Fork:** This project is a fork of the gameME AMX Mod X plugin
> ([gamemedev/plugin-amxmodx](https://github.com/gamemedev/plugin-amxmodx)), version 2.4, by TTS Oetzel & Goerz GmbH.

gameWE is a statistics system for Source and GoldSrc game servers, in the
style of gameME / HLstatsX. The **AMX Mod X plugin** runs on Half-Life 1
(GoldSrc) game servers and works together with the
[gameWE collector](https://github.com/DennisPohlmann/gameWE-collector). It is
the GoldSrc counterpart of the
[gameWE SourceMod plugin](https://github.com/DennisPohlmann/gameWE-plugin-sourcemod). It

- writes extra statistics to the server log: per-weapon accuracy and hit groups
  (`weaponstats`, `weaponstats2`), kill positions, headshots and bullet hits
  (Counter-Strike),
- offers the server commands the collector uses to answer players in game
  (`gamewe_amx_psay`, `gamewe_amx_msay`, ...),
- hides the stats chat commands (`rank`, `/top10`, ...) from chat, but still
  logs them for the collector,
- shows the gameWE menu (`/gamewe`) and, in Counter-Strike, Day of Defeat and
  Team Fortress Classic, a damage summary on death; every player chooses the
  damage display in the gameWE settings menu, `gamewe.cfg` only sets the default,
- keeps the players' own settings of gameME's commands: console and chat
  events (`gamewe_display`, `gamewe_chat`), auto commands at round start,
  round end and death (`gamewe_auto`) and the language (`gamewe_lang`, 9
  languages, also for the collector's answers).

The log format and the command interface stay compatible with gameME.

Current version: see `VERSION` and [CHANGELOG.md](CHANGELOG.md).

## Requirements

- a HLDS game server with Metamod (or Metamod-P/Metamod-R) and
  [AMX Mod X](https://www.amxmodx.org/) **1.9 or newer** (1.10 recommended)
- the AMX Mod X modules of your game (see below), which ship with AMX Mod X
- the gameWE collector 0.1.0 or newer, with the server registered in its
  `hlstats_Servers`

## Supported games

There is one plugin per game:

| Game | Plugin | AMX Mod X modules |
|------|--------|-------------------|
| Counter-Strike 1.6, Condition Zero | `gamewe_cstrike.amxx` | cstrike, csx, fakemeta, hamsandwich, nvault |
| Day of Defeat | `gamewe_dod.amxx` | dodx, dodfun, hamsandwich, nvault |
| Team Fortress Classic | `gamewe_tfc.amxx` | tfcx, hamsandwich, nvault |
| Natural Selection | `gamewe_ns.amxx` | nvault |
| Half-Life Deathmatch | `gamewe_hlmp.amxx` | nvault |

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

## Quick start

1. Extract `gamewe-amxmodx-<version>.tar.gz` from the releases into the game
   folder (the folder with `addons/`):

   ```bash
   tar -xzf gamewe-amxmodx-0.1.0.tar.gz -C /srv/hlds/cstrike
   ```

2. Add only the plugin of your game to `addons/amxmodx/configs/plugins.ini`,
   e.g. `gamewe_cstrike.amxx`, and remove a `gameme_<game>.amxx` line.
3. Add to `server.cfg`:

   ```
   log on
   logaddress_add 203.0.113.5 41337     // the collector's IP and UDP port
   rcon_password "change-me"            // the same as in the collector's hlstats_Servers
   ```

4. Change the map; `amxx plugins` shows `gameWE` as `running`.

Every step in detail, also for game servers in a container:
[docs/INSTALLATION.md](docs/INSTALLATION.md).

## Documentation

| File | Content |
|------|---------|
| [docs/INSTALLATION.md](docs/INSTALLATION.md) | installation step by step: plugin per game, containers, collector connection, check, update, removal, coming from gameME, troubleshooting |
| [docs/CONFIG.md](docs/CONFIG.md) | the cvars in `configs/plugins/gamewe.cfg` and the server commands for settings |
| [docs/USAGE.md](docs/USAGE.md) | chat commands and the gameWE menu |
| [docs/INTERFACE.md](docs/INTERFACE.md) | log lines and the collector's server commands |
| [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) | building, source layout, CI and releases |
| [CHANGELOG.md](CHANGELOG.md) | changes per version |
| [docs/gameme-CHANGELOG.txt](docs/gameme-CHANGELOG.txt) | the changelog of the original gameME plugin up to 2.4 |

## gameWE components

| Repository | Part |
|------------|------|
| [gameWE-collector](https://github.com/DennisPohlmann/gameWE-collector) | receives the game server logs and fills the database |
| [gameWE-web](https://github.com/DennisPohlmann/gameWE-web) | statistics website and admin area |
| [gameWE-plugin-sourcemod](https://github.com/DennisPohlmann/gameWE-plugin-sourcemod) | game server plugin for Source games |
| [gameWE-plugin-amxmodx](https://github.com/DennisPohlmann/gameWE-plugin-amxmodx) | game server plugin for GoldSrc games (this repository) |

## Building

```bash
make compiler   # build/amxx-1.10/amxxpc and its includes
make package    # dist/gamewe-amxmodx-<version>.tar.gz and .tar.gz.sha256
```

Requirements, the source layout and the release process:
[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md).

## License

gameWE is free software under the **GNU General Public License version 3 or later**,
see [LICENSE](LICENSE).

It is based on the gameME plugin for AMX Mod X, Copyright (C) 2007-2017
TTS Oetzel & Goerz GmbH, published under the GPL version 2 or later. The gameME
changelog up to 2.4 is kept in [docs/gameme-CHANGELOG.txt](docs/gameme-CHANGELOG.txt).
