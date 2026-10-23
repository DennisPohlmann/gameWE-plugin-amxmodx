---
name: release-security-gate
description: Full security pass over the gameWE AMX Mod X plugin before release-relevant commits are pushed to main (semantic-release then bumps, tags and releases). Use whenever a release is being prepared; ends with a go/no-go report and never tags or pushes on its own.
---

# Release security gate (gameWE AMX Mod X plugin)

Always run this gate before a release or tag, without asking first.

Run all steps over the whole repository, not only the last diff. Report a
table with the 20 points (ok / finding / n.a.) and a go/no-go.

Releases are made by semantic-release in CI: every push to `main` that
contains a `feat`, `fix` or breaking-change commit (Conventional Commits)
bumps the version, writes `CHANGELOG.md`, tags `vX.Y.Z` and publishes the
GitHub release (and, where there is one, the container image). So this gate
runs **before pushing release-relevant commits to `main`**, without asking
first. Never tag by hand, never edit the version files or `CHANGELOG.md` by
hand, and do not push to `main` yourself: report the gate result and the
version semantic-release will make (`feat` → minor, `fix` → patch,
`!`/`BREAKING CHANGE` → major), then ask Dennis for the go.

## Steps

1. **Clean tree and version:** `git status` is clean; `VERSION` and the top
   `CHANGELOG.md` entry agree with the latest tag
   (`git describe --tags --abbrev=0`; the plugin version comes from the tag
   or `VERSION`).
2. **Build:** `make compiler build AMXX=1.9` and
   `make clean compiler package AMXX=1.10`; no warnings; the package holds
   exactly the five `gamewe_*.amxx`.
3. **Secrets in the whole history:** prefer
   `gitleaks detect --source . --log-opts="--all"`; otherwise
   `git log -p --all | grep -nEi '(rcon_password|password|secret|token)\s*[:= ]'`.
4. **Command inventory:**
   `grep -n 'register_\(srvcmd\|clcmd\|concmd\)' addons/amxmodx/scripting/*.sma addons/amxmodx/scripting/include/*.inc`
   - every collector command is `register_srvcmd`, every client command is
   `say`/`say_team` or admin-gated.
5. **Injection review:** every `log_message(`, `client_print`,
   `show_hudmessage(`, `show_motd(`, `server_cmd(` with text from players or
   the collector; confirm `"%s"` and validation.
6. **Buffers:** `grep -n 'copy(\|formatex(\|format(\|read_args(\|read_argv(' ...`
   for changes since the last tag (`git diff $(git describe --tags --abbrev=0) -- addons`), sizes
   use `charsmax()` of the real buffer.
7. **Workflow:** `.github/workflows/build.yml` builds `main`, publishes
   only via semantic-release (no tag trigger), pins action versions, and
   `tools/build-amxxpc.sh` checks the commit of the AMX Mod X tag.
8. **Test on a server** if one is available: hostile player name, chat
   flood of `rank`, `gamewe_amx_browse <id> server.cfg`, userid of a player
   who left, a CS kill (exactly one kill line with positions).
9. **The 20 points:** go through the `security-check` skill's list for the
   whole plugin, with extra attention on changes since the last tag.
10. **Report:** go/no-go, findings with file:line and fix, the version semantic-release will make, then ask Dennis
    for the go to push to `main`.
