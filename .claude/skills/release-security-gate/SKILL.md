---
name: release-security-gate
description: Full security pass over the gameWE AMX Mod X plugin before a release is started (build workflow by hand with release=true; semantic-release then bumps, tags and releases). Use whenever a release is being prepared; ends with a go/no-go report and never tags, pushes or starts a release on its own.
---

# Release security gate (gameWE AMX Mod X plugin)

Always run this gate before a release or tag, without asking first.

Run all steps over the whole repository, not only the last diff. Report a
table with the 20 points (ok / finding / n.a.) and a go/no-go.

Releases are made by semantic-release in CI, only when the build workflow is
started by hand on `main` with `release` ticked (`gh workflow run build.yml
--ref main -f release=true`). With a `feat`, `fix` or breaking-change commit
(Conventional Commits) since the last tag it bumps the version, writes
`CHANGELOG.md`, tags `vX.Y.Z` and publishes the GitHub release (and, where
there is one, the container image). Pushing to `main` does not release any
more. So this gate runs **before every release run**, without asking first.
Never tag by hand, never edit the version files or `CHANGELOG.md` by hand,
never start the release run and do not push to `main` yourself: report the
gate result and the version semantic-release will make (`feat` → minor, `fix`
→ patch, `!`/`BREAKING CHANGE` → major), then ask Dennis for the go.

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
7. **Workflow:** `.github/workflows/build.yml` builds every push and pull
   request without publishing, publishes only via semantic-release in a
   manual run (`workflow_dispatch` with `release`, guarded to `main`; no tag
   trigger; draft until the package is in it), pins action versions, and
   `tools/build-amxxpc.sh` checks the commit of the AMX Mod X tag.
8. **Test on a server** if one is available: hostile player name, chat
   flood of `rank`, `gamewe_amx_browse <id> server.cfg`, userid of a player
   who left, a CS kill (exactly one kill line with positions).
9. **The 20 points:** go through the `security-check` skill's list for the
   whole plugin, with extra attention on changes since the last tag.
10. **Security and privacy audit:** run the `security-privacy-audit`
   skill over the whole repository, all branches and tags since the last
   release, the release commit messages and the `CHANGELOG.md` entry that
   semantic-release will write. Any `BLOCKED` item is a no-go.
11. **Report:** go/no-go, the `security-privacy-audit` summary, findings with file:line and fix, the version semantic-release will make, then ask Dennis
    for the go to start the release run.
