#!/bin/sh
# Builds the AMX Mod X compiler (amxxpc) and copies the includes from the
# AMX Mod X sources, so the plugin can be built without the AMX Mod X
# downloads. amxxpc is a 32-bit program: needs gcc-multilib and g++-multilib.
#
#   tools/build-amxxpc.sh <version> <out-dir>
#
# <version> is 1.9 or 1.10; the matching git tag and commit are pinned below.
# Result: <out-dir>/amxxpc, <out-dir>/amxxpc32.so, <out-dir>/include/
set -eu

version=${1:?usage: build-amxxpc.sh <1.9|1.10> <out-dir>}
out=${2:?usage: build-amxxpc.sh <1.9|1.10> <out-dir>}

case "$version" in
	1.9)  tag=1.9.0.5303;  commit=b88b763a63bc7165f48e326c226eb05f9db0e54a ;;
	1.10) tag=1.10.0.5487; commit=8fc12670ea86470a0bfed4c35db5de73d3a39e8a ;;
	*) echo "unknown AMX Mod X version: $version" >&2; exit 1 ;;
esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

git -c advice.detachedHead=false clone -q --depth 1 --branch "$tag" https://github.com/alliedmodders/amxmodx.git "$work/src"
got=$(git -C "$work/src" rev-parse HEAD)
if [ "$got" != "$commit" ]; then
	echo "tag $tag points to $got, expected $commit" >&2
	exit 1
fi

src="$work/src"
cflags="-m32 -O2 -w"
# The old C sources (bundled zlib, Pawn compiler) predate C23 and the stricter
# defaults of GCC 14+: build them as gnu11 and keep implicit declarations legal.
ccflags="-std=gnu11 -Wno-error=implicit-function-declaration -Wno-error=int-conversion -Wno-error=incompatible-pointer-types"
mkdir -p "$out" "$work/zlib"

# compiler library (the Pawn compiler itself)
(cd "$src/compiler/libpc300" && gcc $cflags $ccflags -fPIC -shared \
	-DNO_MAIN -DPAWNC_DLL -DHAVE_STDINT_H -D_GNU_SOURCE -DENABLE_BINRELOC -DLINUX \
	-I. -I"$src/public" \
	sc1.c sc2.c sc3.c sc4.c sc5.c sc6.c sc7.c scvars.c scmemfil.c scstate.c \
	sclist.c sci18n.c pawncc.c libpawnc.c prefix.c memfile.c sp_symhash.c \
	-lm -lpthread -o "$out/amxxpc32.so")

# front end, writes the .amxx file
(cd "$work/zlib" && gcc $cflags $ccflags -c "$src"/third_party/zlib/*.c)
(cd "$src/compiler/amxxpc" && g++ $cflags -DAMX_ANSIONLY -DLINUX -D_LINUX \
	-I. -I"$src/public" -I"$src/third_party" \
	amx.cpp amxxpc.cpp Binary.cpp "$work"/zlib/*.o -ldl -o "$out/amxxpc")

rm -rf "$out/include"
cp -r "$src/plugins/include" "$out/include"
echo "built amxxpc $tag in $out"
