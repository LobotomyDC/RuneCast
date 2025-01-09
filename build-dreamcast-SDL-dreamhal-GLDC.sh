source /opt/toolchains/dc/kos/environ.sh

make -f Makefile.dreamcast OLDSDL=1 clean && make -f Makefile.dreamcast OLDSDL=1

# Below is optional to build .cdi. Delete "-N" if you want padding, for whatever reason that may be

#./mkdcdisc -e '[ELF LOCATION]' -d '[ASSET DIRECTORY]' -o RuneScape.cdi -N
