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

## Menu

`gamewe` or `gamewe_menu` (also `gameme`, `gameme_menu`) opens the gameWE menu. The
menu entries send the matching chat command for the player, for example `/rank`.
Counter-Strike and Day of Defeat get the full menu, the other games a short one
(rank, next, top10, settings).

`settings` or `gamewe_settings` in chat, or `amx_gamewe_settings` in the console, opens the
settings menu. The plugin hides `settings` and `gamewe_settings` and does not log them, so
the collector does not answer them as well. This is the only exception to the rule above. The menu has "Hide me in the ranking"
(sends `/gamewe_hideranking 1`) and "Show me in the ranking" (`/gamewe_hideranking 0`).

In Counter-Strike, Day of Defeat and Team Fortress Classic the settings menu also
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
