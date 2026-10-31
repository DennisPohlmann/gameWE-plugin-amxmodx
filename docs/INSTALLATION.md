# Installing gameWE - AMX Mod X plugin

This guide installs the gameWE plugin on a Half-Life 1 (GoldSrc) game server, step by
step. The plugin only writes log lines and answers the collector, so the
gameWE collector has to receive the
server log as well.

1. [Requirements](#requirements)
2. [Which plugin for which game](#which-plugin-for-which-game)
3. [Install from the tar file](#install-from-the-tar-file)
4. [Install on a game server in a container](#install-on-a-game-server-in-a-container)
5. [Send the log to the collector](#send-the-log-to-the-collector)
6. [Check the installation](#check-the-installation)
7. [Configuration file](#configuration-file)
8. [Update](#update)
9. [Uninstall](#uninstall)
10. [Coming from the gameME plugin](#coming-from-the-gameme-plugin)
11. [Troubleshooting](#troubleshooting)

## Requirements

- A HLDS game server with [Metamod](http://metamod.org/) or Metamod-P/Metamod-R and
  [AMX Mod X](https://www.amxmodx.org/) **1.9 or newer** (1.10 recommended) installed and
  running. `amxx version` in the server console shows the version.
- The AMX Mod X modules of your game (see the table below). They ship with AMX Mod X.
- A running gameWE collector that the game server can reach over UDP.
- The release file `gamewe-amxmodx-<version>.tar.gz` and its
  `gamewe-amxmodx-<version>.tar.gz.sha256`.

## Which plugin for which game

The tar file holds one plugin per game. Load **only the plugin of your game**: the others
need modules of other games and fail to load.

| Game | Game folder | Plugin | Modules in `modules.ini` |
|------|-------------|--------|--------------------------|
| Counter-Strike 1.6 | `cstrike/` | `gamewe_cstrike.amxx` | `cstrike`, `csx`, `fakemeta`, `hamsandwich`, `nvault` |
| Counter-Strike: Condition Zero | `czero/` | `gamewe_cstrike.amxx` | `cstrike`, `csx`, `fakemeta`, `hamsandwich`, `nvault` |
| Day of Defeat | `dod/` | `gamewe_dod.amxx` | `dodx`, `dodfun`, `hamsandwich`, `nvault` |
| Half-Life Deathmatch | `valve/` | `gamewe_hlmp.amxx` | `nvault` |

AMX Mod X loads most modules on its own when a plugin needs them. If one is commented
out with `;` in `addons/amxmodx/configs/modules.ini`, remove the `;`.

## Install from the tar file

The examples use Counter-Strike 1.6 in `/srv/hlds/cstrike`. Use your own game folder
from the table above.

1. **Check the download.** In the folder with both files:

   ```bash
   sha256sum -c gamewe-amxmodx-0.1.0.tar.gz.sha256
   ```

   It must print `gamewe-amxmodx-0.1.0.tar.gz: OK`.

2. **Extract into the game folder**, the folder that contains `addons/`:

   ```bash
   tar -xzf gamewe-amxmodx-0.1.0.tar.gz -C /srv/hlds/cstrike
   ```

   The archive only holds `addons/amxmodx/plugins/gamewe_*.amxx`. Nothing else on the
   server is overwritten.

   On Windows, extract the archive with 7-Zip or `tar -xzf` in PowerShell and copy the
   `addons` folder into the game folder, for example `C:\hlds\cstrike\`.

3. **Enable the plugin.** Add one line at the end of
   `addons/amxmodx/configs/plugins.ini`:

   ```
   gamewe_cstrike.amxx
   ```

   If `gameme_cstrike.amxx` is listed there, remove that line (see
   [Coming from the gameME plugin](#coming-from-the-gameme-plugin)).

4. **Turn on logging.** In `server.cfg` (in the game folder):

   ```
   log on
   ```

   and the collector address, see [Send the log to the collector](#send-the-log-to-the-collector).

5. **Restart the server** or change the map, then [check the installation](#check-the-installation).

The server folders after the install:

```
cstrike/
├── server.cfg                         log on, logaddress_add
└── addons/amxmodx/
    ├── plugins/gamewe_cstrike.amxx    from the tar file
    ├── configs/plugins.ini            line "gamewe_cstrike.amxx"
    ├── configs/modules.ini            modules of the game enabled
    └── configs/plugins/gamewe.cfg     written by the plugin on the first start
```

## Install on a game server in a container

The plugin runs inside the game server, so it is not a container of its own. You add the
`.amxx` file to the game folder of the server container. There are two ways.

### Bind mount (no own image)

Most HLDS images keep the game folder in a volume or let you mount single files. Put the
plugin and your configs on the host and mount them into the container:

```bash
mkdir -p /opt/hlds/amxx
tar -xzf gamewe-amxmodx-0.1.0.tar.gz -C /opt/hlds/amxx
```

```yaml
# compose.yaml (part of the game server service)
services:
  cstrike:
    image: <your hlds image>
    volumes:
      - /opt/hlds/amxx/addons/amxmodx/plugins/gamewe_cstrike.amxx:/home/steam/hlds/cstrike/addons/amxmodx/plugins/gamewe_cstrike.amxx:ro
      - /opt/hlds/config/plugins.ini:/home/steam/hlds/cstrike/addons/amxmodx/configs/plugins.ini:ro
      - /opt/hlds/config/server.cfg:/home/steam/hlds/cstrike/server.cfg:ro
```

The path inside the container (`/home/steam/hlds/` here) depends on the image; look it up
in the documentation of the image you use. `plugins.ini` and `server.cfg` on the host need
the same lines as in [Install from the tar file](#install-from-the-tar-file).

Do not mount `addons/amxmodx/configs/plugins/` read-only: the plugin writes `gamewe.cfg`
there on the first start. Mount it read-write, or let it live in the volume of the image.

### Own image

If you build your own game server image, copy the plugin in at build time:

```dockerfile
FROM <your hlds image with AMX Mod X>
ADD gamewe-amxmodx-0.1.0.tar.gz /home/steam/hlds/cstrike/
RUN echo "gamewe_cstrike.amxx" >> /home/steam/hlds/cstrike/addons/amxmodx/configs/plugins.ini
```

`ADD` extracts the tar file. Check that `plugins.ini` does not list the plugin twice.

### Network in a container

The game server sends its log over UDP to the collector. `logaddress_add` only takes an
IP address, not a host or service name. With the collector in the same compose project,
give the collector a fixed IP in the compose network and use that IP. Otherwise use the
IP of the host the collector runs on. The collector needs its UDP log port published if
it runs on another host.

## Send the log to the collector

Add to `server.cfg`, with the IP and UDP port of your collector:

```
log on
logaddress_add 203.0.113.5 41337
```

GoldSrc (HLDS) takes IP and port separated by a space; the `ip:port` form of Source
servers does not work here. 41337 is the collector's default UDP port for game server
logs (`listen.udp.port` in the collector config). With `logaddress_add` you can send
the log to more than one address.
`logaddress_del` removes an address, `logaddress_delall` all.

The collector also sends commands back to the server over RCON, so `rcon_password` must
be set in `server.cfg` and the same password stored for this server in the collector.

## Check the installation

In the server console (or with `rcon`):

```
amxx plugins
```

lists `gameWE` with the status `running`. If it says `bad load` or `error`, see
[Troubleshooting](#troubleshooting).

```
gamewe_plugin_version
```

prints the installed version.

In game, `/rank` must not show up in chat but get an answer from the collector. `!rank`
shows up in chat and is answered as well. `/gamewe` opens the gameWE menu.

## Configuration file

On the first start the plugin writes `addons/amxmodx/configs/plugins/gamewe.cfg` with all
cvars and their defaults. Change the values there; they are read on every map change.
All cvars and server commands are in [CONFIG.md](CONFIG.md), the chat commands in
[USAGE.md](USAGE.md).

## Update

1. Check and extract the new tar file the same way as in
   [Install from the tar file](#install-from-the-tar-file). It replaces the `.amxx` files.
2. Change the map or restart the server.

`plugins.ini` and `gamewe.cfg` stay as they are. New cvars are added to `gamewe.cfg`
automatically. Read the [CHANGELOG](../CHANGELOG.md) before an update.

## Uninstall

1. Remove the `gamewe_<game>.amxx` line from `addons/amxmodx/configs/plugins.ini`.
2. Delete `addons/amxmodx/plugins/gamewe_*.amxx`,
   `addons/amxmodx/configs/plugins/gamewe.cfg` and the players' saved settings
   `addons/amxmodx/data/vault/gamewe_settings.vault`.
3. Change the map or restart the server.

## Coming from the gameME plugin

1. Remove `gameme_<game>.amxx` from `plugins.ini` and delete it from `plugins/`. Do not run
   both plugins: both answer the `gameme_amx_*` commands and log the same events twice.
2. Install the gameWE plugin as described above.
3. Old `gameme_*` cvars are not read any more. Set the matching `gamewe_*` cvars in
   `configs/plugins/gamewe.cfg`.

The collector keeps working with the `gameme_amx_*` command names, the gameWE plugin
accepts them. Log lines are the same as before, with two changes: weapon names in
`weaponstats` are the log names of the kill lines (Day of Defeat used display names
before), and `attacked` lines carry the real damage instead of `0`. `gameme_webpage`
is gone.

## Troubleshooting

| Problem | Cause and fix |
|---------|---------------|
| `amxx plugins` shows `bad load` | A module is missing. Enable the modules of the game in `modules.ini`, and check that you loaded the plugin of your game. `amxx modules` lists the loaded modules. |
| `Plugin uses an unknown function` in the AMX Mod X log | AMX Mod X is older than 1.9. Update it. |
| No answer to `/rank` | Is `log on` set and does `logaddress_add` point to the collector? Does the collector have the right RCON password? The collector log shows whether lines arrive. |
| `/rank` shows up in chat | Another plugin handles `say` before gameWE. Move `gamewe_<game>.amxx` further up in `plugins.ini`. |
| Commands are answered twice | The gameME plugin is still loaded. Remove it. |
| `gamewe.cfg` is not written | `configs/plugins/` is not writable, for example a read-only mount in a container. |

Errors of AMX Mod X plugins are in `addons/amxmodx/logs/error_<date>.log`.
