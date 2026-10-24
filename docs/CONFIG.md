# gameWE - AMX Mod X plugin: configuration

On the first start the plugin writes `addons/amxmodx/configs/plugins/gamewe.cfg`
with all cvars and their defaults. Change the values there; they are read on
every map change. New cvars of a later version are added to the file
automatically.

## Cvars

| Cvar | Default | Meaning |
|------|---------|---------|
| `gamewe_message_prefix` | | Text put in front of every gameWE chat message. |
| `gamewe_chat_cooldown` | `1.0` | Seconds between two logged `/` commands of one player, so nobody can flood the collector. `0` turns it off. Commands in between are hidden but not logged. |
| `gamewe_log_damage` | `1` | Counter-Strike: log an `attacked` line for every bullet hit. |
| `gamewe_plugin_version` | | Read only, the plugin version. |

## Server commands


- `gamewe_block_commands_values "<command> <command> ..."` adds more chat commands that
  are silent with `/` (up to 32 per call), `gamewe_block_commands_values clear` removes
  all, including the defaults.
- `gamewe_message_prefix_clear` empties the prefix.
