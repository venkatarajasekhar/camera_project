#
# OpenSSL/crypto/blowfish/Makefile
#

DIR=	bf
TOP=	../..
CC=	cc
CPP=	$(CC) -E
INCLUDES=
CFLAG=-g
INSTALL_PREFIX=
OPENSSLDIR=     /usr/local/ssl
INSTALLTOP=/usr/local/ssl
MAKEDEPPROG=	makedepend
MAKEDEPEND=	$(TOP)/util/domd $(TOP) -MD $(MAKEDEPPROG)
MAKEFILE=	Makefile
AR=		ar r

BF_ENC=		bf_enc.o
# or use
#DES_ENC=	bx86-elf.o

CFLAGS= $(INCLUDES) $(CFLAG)
ASFLAGS= $(INCLUDES) $(ASFLAG)

GENERAL=Makefile
TEST=bftest.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=bf_skey.c bf_ecb.c bf_enc.c bf_cfb64.c bf_ofb64.c 
LIBOBJ=bf_skey.o bf_ecb.o $(BF_ENC) bf_cfb64.o bf_ofb64.o

SRC= $(LIBSRC)

EXHEADER= blowfish.h
HEADER=	bf_pi.h bf_locl.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

lib:	$(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

# elf
asm/bx86-elf.s: asm/bf-586.pl ../perlasm/x86asm.pl ../perlasm/cbc.pl
	(cd asm; $(PERL) bf-586.pl elf $(CFLAGS) $(PROCESSOR) > bx86-elf.s)

# a.out
asm/bx86-out.o: asm/bx86unix.cpp
	$(CPP) -DOUT asm/bx86unix.cpp | as -o asm/bx86-out.o

# bsdi
asm/bx86bsdi.o: asm/bx86unix.cpp
	$(CPP) -DBSDI asm/bx86unix.cpp | sed 's/ :/:/' | as -o asm/bx86bsdi.o

asm/bx86unix.cpp: asm/bf-586.pl ../perlasm/x86asm.pl ../perlasm/cbc.pl
	(cd asm; $(PERL) bf-586.pl cpp $(PROCESSOR) >bx86unix.cpp)

files:
	$(PERL) $(TOP)/util/files.pl Makefile >> $(TOP)/MINFO

links:
	@$(PERL) $(TOP)/util/mklink.pl ../../include/openssl $(EXHEADER)
	@$(PERL) $(TOP)/util/mklink.pl ../../test $(TEST)
	@$(PERL) $(TOP)/util/mklink.pl ../../apps $(APPS)

install: installs

installs:
	@headerlist="$(EXHEADER)"; for i in $$headerlist ; \
	do  \
	(cp $$i $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i; \
	chmod 644 $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i ); \
	done;

tags:
	ctags $(SRC)

tests:

lint:
	lint -DLINT $(INCLUDES) $(SRC)>fluff

depend:
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(PROGS) $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f asm/bx86unix.cpp asm/*-elf.* *.o asm/*.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.
