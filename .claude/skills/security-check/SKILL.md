---
name: security-check
description: Security checklist for every change in the gameWE AMX Mod X plugin (Pawn, log lines for the collector, RCON server commands). Use before finishing, committing or opening a PR for any .sma, .inc, build or workflow change.
---

# Security check for a change (gameWE AMX Mod X plugin)

Scope: the current diff (`git diff` plus staged and untracked files). Walk
the points below, only as deep as the diff reaches, and report the result.
Many points of the original web-app list do not apply to a game server
plugin; they are mapped to what does.

## Steps

1. Read the diff: `git status --short`, `git diff HEAD`.
2. List new or changed commands (`register_srvcmd`, `register_clcmd`,
   `register_concmd`), cvars, `log_message` formats, `show_motd`,
   `server_cmd` and `client_print*` calls.
3. For each point below that the diff touches, check it in the code.
4. Build: `make build` and, if the change touches natives or includes,
   `make build AMXX=1.9` (warnings are errors). If `amxxpc` is missing, run
   `make compiler` or say why it is missing.
5. Quick secret grep on the diff:
   `git diff HEAD | grep -nEi '(rcon_password|password|secret|token)\s*[:= ]'`.
6. Fix what you find, then report: touched points, findings with
   file:line, what you fixed, what is left.

## The checklist, adapted to the plugin

1. **Secrets / keys:** no RCON passwords, server IPs of real servers or
   tokens in code, configs or docs.
2. **Least privilege:** new commands for the collector are
   `register_srvcmd`; player-facing commands (`register_clcmd`) need an
   admin flag or must be harmless.
3. **IDOR:** a command with a userid acts only on that player after
   `gamewe_player_of_userid`; lists of ids (`1,2,3`) are each checked,
   deduplicated and bounded (`GAMEWE_MAX_RECIPIENTS`).
4. **Git secrets:** see step 5; build output (`build/`, `dist/`, `*.amxx`)
   stays ignored.
5. **Admin routes:** no admin-only action (swap, browse, menus for others,
   blocked-command changes) reachable from player chat or client console.
6. **User isolation:** private messages and menus go only to the target
   player; nothing about one player is sent to others unless it is public.
7. **Rate limits:** chat triggers that make the collector answer go through
   the `gamewe_chat_cooldown` check in `gamewe_client_say`.
8. **Storage:** the plugin writes no files except the AutoExecConfig
   `configs/plugins/gamewe.cfg` and the nVault `gamewe_settings` (player
   settings keyed by SteamID, pruned after 180 days, values range-checked
   on load); any new file uses a fixed name.
9. **Input validation:** collector arguments parsed with
   `gamewe_next_token`/`gamewe_rest_of` into sized buffers, numbers with
   `is_str_num` and range checks (time 1-120, flags 0/1), control
   characters dropped.
10. **Unauthenticated entry points:** chat (`say`, `say_team`) and menu
    keys are the only player input; chat is only compared with fixed
    triggers, menu keys only logged as numbers.
11. **Injection:** log injection, format injection and command injection.
    Player names in log lines only in the HL format; outside text is always
    an argument to `"%s"`; nothing from players or the collector reaches
    `server_cmd`, `client_cmd` or `show_motd` without validation.
12. **Sensitive logs:** no IP addresses or SteamIDs beyond what the HL log
    format already carries; no RCON output in logs.
13. **Field tampering:** values from players never decide points, ranks or
    teams in the log lines (the collector computes them).
14. **Uploads:** not applicable.
15. **Server logic:** the collector, not the plugin, is the source of truth
    for stats; the plugin only reports events. The kill-line replacement in
    `gamewe_cstrike.sma` must keep every kill logged exactly once.
16. **Trim responses:** messages shown to players contain only their own or
    public stats.
17. **Auth sessions:** not applicable (RCON auth is the server's).
18. **Dependencies:** a change to the AMX Mod X tags/commits in
    `tools/build-amxxpc.sh`, the minimum AMX Mod X version or
    `.github/workflows/build.yml` (pinned actions, Node 24 runtime,
    `ubuntu-26.04`) gets a note.
19. **Record access:** not applicable (no natives for other plugins).
20. **Attack your own change:** try one hostile input (name with `"`, `%s`,
    `^n`, 31 chars; userid of a disconnected player; `gamewe_amx_browse`
    with `server.cfg` or `javascript:`; `msay` with 2000 characters) and
    check the code path.
