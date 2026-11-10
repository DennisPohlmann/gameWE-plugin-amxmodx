# gameWE - AMX Mod X plugin: development

Building and releasing the plugin. The code layout and the rules for changes
are in [CLAUDE.md](../CLAUDE.md).

## Build

You need `amxxpc` from AMX Mod X 1.9 or newer. The easiest way is to build it from
the AMX Mod X sources (needs `git`, `gcc-multilib` and `g++-multilib`, because
`amxxpc` is a 32-bit program):

```bash
make compiler             # build/amxx-1.10/amxxpc and its includes
make build                # build/stage/addons/amxmodx/plugins/gamewe_*.amxx
make package              # dist/gamewe-amxmodx-<version>.tar.gz and .tar.gz.sha256
make compiler build AMXX=1.9   # check the build with AMX Mod X 1.9
```

To use an installed compiler instead: `make build AMXXPC=/path/to/amxxpc
AMXX_INCLUDE=/path/to/scripting/include` (`amxxpc32.so` must be next to `amxxpc`).
The version comes from the git tag (`v1.2.3`) or the `VERSION` file. Warnings are
treated as errors.

The GitHub workflow `.github/workflows/build.yml` builds every push (any
branch) and every pull request with AMX Mod X 1.9 and 1.10 and keeps the
package as a workflow artifact for 7 days; nothing is published without a
release run.

## Releases

Releases are made by [semantic-release](https://semantic-release.gitbook.io/)
(`.releaserc.json`) in the same workflow; nobody tags by hand. Pushing to
`main` does not release: a release is started by hand on `main`, in GitHub
under Actions → build → Run workflow with `release` ticked, or with
`gh workflow run build.yml --ref main -f release=true`. Run the `release-security-gate` skill
first (see `CLAUDE.md`). The run builds and tests everything again; only
when that passed does semantic-release read the
[Conventional Commits](https://www.conventionalcommits.org/) since the last
tag and decide the next version:

| Commit | Release |
|---|---|
| `feat: ...` | minor (0.1.0 → 0.2.0) |
| `fix: ...` (also `fix(security): ...`), `perf: ...` | patch (0.1.0 → 0.1.1) |
| `feat!: ...`, `fix!: ...` or a `BREAKING CHANGE:` footer | major (0.1.0 → 1.0.0) |
| `docs`, `chore`, `ci`, `build`, `test`, `refactor`, `style` | no release |

A run with `release` ticked that makes no release (no such commit since the
last tag, or `main` moved on while the run was checking) fails at the step
`stop if nothing was released`; start it again once there is something to
release.

For a release it writes `VERSION` and `CHANGELOG.md`, commits them as
`chore(release): vX.Y.Z [skip ci]`, tags `vX.Y.Z` and creates the GitHub
release as a draft. The workflow then builds that tag with AMX Mod X 1.10,
uploads the `.tar.gz` (the five `.amxx` files) and its `.tar.gz.sha256` to
the release and makes it public. The release stays a draft until all files are in it, so it never shows up
incomplete; if a job fails, re-run the failed jobs of that run.
Never edit `VERSION` or `CHANGELOG.md` by hand.

## Source layout

`addons/amxmodx/scripting/gamewe_<game>.sma` holds the game specific
code, `include/gamewe_core.inc` the commands, chat and menu shared by all games,
`include/gamewe_xstats.inc` the weapon statistics, `include/gamewe_lang.inc` the
plugin's messages in the 9 languages of `gamewe_lang`.
