# gameWE - AMX Mod X plugin: configuration

On the first start the plugin writes `addons/amxmodx/configs/plugins/gamewe.cfg`
with all cvars and their defaults. Change the values there; they are read on
every map change. New cvars of a later version are added to the file
automatically, but the comments of lines that are already there are not
rewritten: the allowed range (`Minimum`/`Maximum`) of a cvar only shows up
in a file written by a version that has it, so delete the file once after an
update to get the current descriptions and ranges. Values outside the range
are clamped.

## Cvars

| Cvar | Default | Meaning |
|------|---------|---------|
| `gamewe_message_prefix` | | Text put in front of every gameWE chat message, also in place of the `[gameWE]` of the collector's answers; up to 63 characters, [color tags](#color-tags) allowed, e.g. `"{lightgreen}[MyServer]{default}"`. Empty: the plugin's messages have no prefix and the collector's keep `[gameWE]`. |
| `gamewe_chat_cooldown` | `1.0` | Seconds between two logged `/` commands of one player, so nobody can flood the collector, `0`-`30`. `0` turns it off. Commands in between are hidden but not logged. |
| `gamewe_log_damage` | `1` | Counter-Strike: log an `attacked` line for every bullet hit (`0` off, `1` on). |
| `gamewe_damage_display` | `0` | Counter-Strike, Day of Defeat, Team Fortress Classic: default for players: damage summary on death (`0` off, `1` menu, `2` chat). Each player can override it in the settings menu. |
| `gamewe_damage_display_allow` | `1` | `0` switches the damage display off for everyone; players cannot enable it then. |
| `gamewe_plugin_version` | | Read only, the plugin version. |

## Per-player settings

The damage display is chosen by each player in the gameWE settings menu (see
[USAGE.md](USAGE.md#menu)): server default, off, menu or chat.
`gamewe_damage_display` is only the default for players who have not chosen
(or chose "server default"), so `0` means "off by default, players can switch
it on". To forbid it completely, set `gamewe_damage_display_allow 0`; the menu
then shows the entry as disabled. Natural Selection and Half-Life have no
damage data, so their settings menu has no damage entry.

Every player also chooses the collector's console events and global chat
events (`gamewe_display 0|1`, `gamewe_chat 0|1`, on by default; the
collector's `gamewe_console_events` / `gamewe_global_chat` decide whether
there are any), auto commands (`gamewe_auto`) and the language (`gamewe_lang`),
see [USAGE.md](USAGE.md#settings-the-plugin-keeps).

The choices are saved in the nVault `addons/amxmodx/data/vault/gamewe_settings.vault`
(all games, so every plugin needs the `nvault` module) under
`<SteamID>#damage_display` (value `0`/`1`/`2`; "server default" removes
the entry), `<SteamID>#console_events` and `#global_chat` (`0` when off; on
removes the entry), `<SteamID>#auto` (`<start>,<end>,<kill>`, each a command
of the allowed list or empty) and `<SteamID>#lang` (the gamewe_lang code). Entries of players who have not joined for 180 days are removed when
the plugin starts. Players with a LAN or pending id (`STEAM_ID_LAN`, ...) share
that id, so their choice is not saved and lasts until the next map. The message
prefix, the chat cooldown, damage logging and the blocked commands stay server
settings.

## Color tags

Counter-Strike shows three chat colors: the default color, the team color and
green. The prefix is green unless a tag changes it. Upper and lower case do not
matter.

| Tag | Counter-Strike |
|-----|----------------|
| `{default}` | default color |
| `{green}`, `{lightgreen}`, `{olive}`, `{lime}` | green |
| `{teamcolor}`, `{team}`, `{red}`, `{lightred}`, `{darkred}`, `{blue}`, `{lightblue}`, `{grey}`, `{gray}` | team color |
| `{yellow}`, `{orange}`, `{purple}` | removed |

The gameME tags `x01`, `x03` and `x04` work as well. Day of Defeat, Team
Fortress Classic, Natural Selection and Half-Life have no chat colors: the plugin
removes all tags there. The cvar has the same name as in the SourceMod plugin,
so one `gamewe_message_prefix` line works for both.

## Server commands


- `gamewe_block_commands_values "<command> <command> ..."` adds more chat commands that
  are silent with `/` (up to 32 per call), `gamewe_block_commands_values clear` removes
  all, including the defaults.
- `gamewe_message_prefix_clear` empties the prefix.
