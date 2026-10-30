# gameWE - AMX Mod X plugin: usage

What players can type in game and what the gameWE menu offers. The answers
come from the [gameWE collector](https://github.com/DennisPohlmann/gameWE-collector);
the plugin hides the commands from chat, logs them and shows the replies.

## Chat commands

Players type these in chat with `/` or `!` in front. The collector answers both, the
only difference is who sees the command (there is no option for this):

- `/rank` is **silent**: the plugin hides it from chat and logs it itself as a `say` line,
  for example `"Name<2><STEAM_0:0:123><CT>" say "/rank"`.
- `!rank` (and a plain `rank`) stays **visible** in chat, the game logs it as usual.

`rank`, `skill`, `points`, `place`, `session`, `sdata`, `session_data`, `kpd`,
`kdratio`, `kdeath`, `next`, `load`, `status`, `servers`, `top5`, `top10`, `top20`,
`maps`, `map_stats`, `clans`, `cheaters`, `statsme`, `weapons`, `weapon`, `action`,
`actions`, `accuracy`, `targets`, `target`, `kills`, `kill`, `player_kills`, `cmd`,
`cmds`, `commands`, `help`, `gstats`, `global_stats`, `hideranking [1|0|on|off|hide|show]`,
`gamewe_hideranking [1|0|on|off|hide|show]`, `gamewe_atb 0|1`, `gamewe_reset [0|1]`.
The `gameme_*` spellings are silent with `/` too.

The collector answers gameME's command list (its USAGE.md lists them): the
stats in chat or a menu (`!rank`, `!kpd`, `!session`, `!next`, `!top5`,
`!top10`, `!top20`, and `!place`, `!kdeath`, `!session_data` to everybody),
web pages (`!statsme`, `!status`, `!servers`, `!load`, `!clans`, `!cheaters`,
`!maps`, `!weapons`, `!accuracy`, `!targets`, `!kills`, `!actions`; opened in
the MOTD window on Counter-Strike, sent as a chat line in the other games,
whose MOTD shows no web pages), `!hideranking`, `!settings`, `!gamewe_reset`
(if the server allows it) and `!help`.

### Settings the plugin keeps

These are answered by the plugin itself, in the player's language, and saved
per SteamID ([CONFIG.md](CONFIG.md#per-player-settings)); the collector does
not see them as commands. With `/` they are hidden from chat, with `!` or
without a prefix they stay visible. The `gameme_*` spellings work as well. A
player can use them at most once per `gamewe_chat_cooldown`.

| Command | Effect |
|---------|--------|
| `gamewe_display 0\|1` | console events off or on (default on): the collector's points of your kills and actions in the console, when the collector has `gamewe_console_events` on |
| `gamewe_chat 0\|1` | global chat events off or on (default on): the collector's announcements of player and team actions with points, when the collector has `gamewe_global_chat` on |
| `gamewe_auto start\|end\|kill <command>` | a command the plugin sends for you at round start (`start`), round end (`end`, both as the game logs `Round_Start` / `Round_End`; games without rounds have none) or when you die (`kill`, as gameME), as if you typed `/<command>`. Only `rank`, `skill`, `points`, `kpd`, `kdratio`, `session`, `sdata`, `next`, `top5`, `top10`, `top20`, `statsme`; `gamewe_auto start clear` removes one, `gamewe_auto clear` all, `gamewe_auto` alone lists them. The chat cooldown applies |
| `gamewe_lang en\|de\|ru\|es\|fr\|pt\|nl\|dk\|pl` | the language of the plugin's messages and settings menu entries and of the collector's answers. The plugin logs `setup "gamewe_lang" (value "de")` for the collector, also when the player joins. Without a choice the plugin uses the game's language (setinfo `lang`) and the collector English. The main menu and the damage display stay English |

`!hideranking` or `/gamewe_hideranking` hides the player in the ranking (`1`/`on`/`hide`),
shows them again (`0`/`off`/`show`) or toggles without an argument. The collector does this
and confirms it with a private message (collector 0.1.0 or newer).

## Menu

`gamewe` or `gamewe_menu` (also `gameme`, `gameme_menu`) opens the gameWE menu. The
menu entries send the matching chat command for the player, for example `/rank`.
Counter-Strike and Day of Defeat get the full menu, the other games a short one
(rank, next, top10, settings).

`settings` or `gamewe_settings` in chat, or `amx_gamewe_settings` in the console, opens the
settings menu. The plugin hides `settings` and `gamewe_settings` and does not log them, so
the collector does not answer them as well. This is the only exception to the rule above. The menu has one ranking entry that fits the player's current state: "Hide me in
the ranking" (sends `/gamewe_hideranking 1`) when the player is shown, "Show me in
the ranking" (`/gamewe_hideranking 0`) when the player hid themselves, and a
disabled "Hidden from the ranking" when an admin or a ban hid the player. The
collector answers in chat and with the new state, and the menu opens again with
the switched entry. The plugin has no database: it gets the state from the
collector (`gamewe_amx_hidestate`, asked for with `requested "hideranking"` when
the menu opens the first time). As long as no state has arrived (a collector
without this command), the entry is "Hide or show me in the ranking" and switches
(`/gamewe_hideranking`).

The settings menu also has "Console events" and "Global chat" with the
player's current value (on, off; `gamewe_display`, `gamewe_chat` above);
selecting one switches it.

In Counter-Strike and Day of Defeat the settings menu also
has "Damage display" with the player's current value: server default, off, menu or
chat. Selecting it switches to the next value and shows the menu again;
"server default" follows `gamewe_damage_display` (the value is shown in
brackets), and the entry is disabled when the server sets
`gamewe_damage_display_allow 0`. The choice is saved per SteamID (see
[CONFIG.md](CONFIG.md#per-player-settings)).

The damage display shows the damage of the current life, per opponent (damage
and hits): "Damage given" (the opponents the player hit, `(killed)` for a kill)
and "Damage taken" (who hit the player, `(killer)` for the killer). With "menu"
it is shown as a menu on death and, in Counter-Strike and Day of Defeat, also at
round end for the players still alive; with "chat" it is shown in two chat lines
on death.
