# gameWE - AMX Mod X plugin: interface

How the plugin talks to the [gameWE collector](https://github.com/DennisPohlmann/gameWE-collector):
the log lines it writes and the server commands the collector sends over RCON.
The collector's side is described in its
[INTERFACE.md](https://github.com/DennisPohlmann/gameWE-collector/blob/main/docs/INTERFACE.md).

## Log lines

The plugin writes standard HL log lines (`"Name<userid><authid><team>"`):

```
"Name<2><STEAM_0:0:123><CT>" triggered "weaponstats" (weapon "ak47") (shots "14") (hits "6") (kills "1") (headshots "1") (tks "0") (damage "412") (deaths "0")
"Name<2><STEAM_0:0:123><CT>" triggered "weaponstats2" (weapon "ak47") (head "1") (chest "3") (stomach "1") (leftarm "0") (rightarm "1") (leftleg "0") (rightleg "0")
"Name<2><STEAM_0:0:123><CT>" killed "Other<3><STEAM_0:1:456><TERRORIST>" with "ak47" (attacker_position "-1022 380 -60") (victim_position "-830 512 -60")
"Name<2><STEAM_0:0:123><CT>" triggered "headshot"
"Name<2><STEAM_0:0:123><CT>" attacked "Other<3><STEAM_0:1:456><TERRORIST>" with "ak47" (damage "36") (damage_armor "4") (health "64") (armor "96") (hitgroup "chest")
"Name<2><STEAM_0:0:123><CT>" say "/rank"
"Name<2><STEAM_0:0:123><CT>" selected "3"
"Name<2><STEAM_0:0:123><CT>" requested "hideranking" (value "0")
```

- `weaponstats` lines are written when a player dies, disconnects or the round ends
  (Counter-Strike, Day of Defeat), each life only once. `weaponstats2` follows when the
  weapon hit something. The weapon name is the one of the kill lines.
- `requested "hideranking"` asks the collector for the player's ranking state when
  the settings menu opens and the state is not known yet; the collector answers
  with `gamewe_amx_hidestate`.
- Counter-Strike: the plugin replaces the kill and suicide lines of the game by the
  same lines with positions, so each kill is still logged once.

## Server commands

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
| `gamewe_amx_hidestate <userid> <hideranking>` | The player's ranking state for the settings menu: `0` shown, `1` hidden by an admin, `2` banned, `3` hidden by the player. Sent by the collector after `!hideranking` / `!settings` and for `requested "hideranking"`. |

The collector starts every chat line of `psay` with `[gameWE]`. When
`gamewe_message_prefix` is set, the plugin puts the prefix in its place, so the
collector's answers show the server's prefix like the plugin's own messages.
