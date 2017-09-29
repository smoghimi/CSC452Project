srcdir = .
prefix = /home/yetter
exec_prefix =   ${prefix}

AC_FLAGS =              -DHAVE_CONFIG_H
INSTALL =               /usr/bin/install -c
INSTALL_PROGRAM =       ${INSTALL}
INSTALL_DATA =          ${INSTALL} -m 644
CC =            gcc
LD =            @LD@

SRC_DIR =       .
LIB_DIR =       ${exec_prefix}/lib
INC_DIR =       ${prefix}/include
BIN_DIR =       ${exec_prefix}/bin
CFLAGS =        -g -O2
