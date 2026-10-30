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

The GitHub workflow `.github/workflows/build.yml` builds every push to `main` with
AMX Mod X 1.9 and 1.10 and keeps the package as a workflow artifact.

## Releases

Releases are made by [semantic-release](https://semantic-release.gitbook.io/)
(`.releaserc.json`) in the same workflow; nobody tags by hand. To make a
release, push commits written as [Conventional Commits](https://www.conventionalcommits.org/)
to `main`. semantic-release reads the commits since the last tag and decides
the next version:

| Commit | Release |
|---|---|
| `feat: ...` | minor (0.1.0 → 0.2.0) |
| `fix: ...` (also `fix(security): ...`), `perf: ...` | patch (0.1.0 → 0.1.1) |
| `feat!: ...`, `fix!: ...` or a `BREAKING CHANGE:` footer | major (0.1.0 → 1.0.0) |
| `docs`, `chore`, `ci`, `build`, `test`, `refactor`, `style` | no release |

For a release it writes `VERSION` and `CHANGELOG.md`, commits them as
`chore(release): vX.Y.Z [skip ci]`, tags `vX.Y.Z` and creates the GitHub
release. The workflow then builds that tag with AMX Mod X 1.10 and uploads the
`.tar.gz` (the five `.amxx` files) and its `.tar.gz.sha256` to the release.
Never edit `VERSION` or `CHANGELOG.md` by hand.

## Source layout

`addons/amxmodx/scripting/gamewe_<game>.sma` holds the game specific
code, `include/gamewe_core.inc` the commands, chat and menu shared by all games,
`include/gamewe_xstats.inc` the weapon statistics, `include/gamewe_lang.inc` the
plugin's messages in the 9 languages of `gamewe_lang`.
