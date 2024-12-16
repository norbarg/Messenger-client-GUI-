BINARYNAME =uchat

SQL_INCLUDE_DIR = ${libq_include_dir}
SQL_LIB_DIR = ${libq_lib_dir}

WEBSOCKETS_INCLUDE_DIR = ${websockets_include_dir}
WEBSOCKETS_LIB_DIR = ${websockets_dir_lib}

OPENSSL_INCLUDE_DIR = ${openssl_include_dir}
OPENSSL_LIB_DIR = ${openssl_lib_dir}

UV_INCLUDE_DIR = ${uv_include_dir}
UV_LIB_DIR = ${uv_lib_dir}

JSON_C_INCLUDE_DIR = ${json_c_include_dir}
JSON_C_LIB_DIR = ${json_c_lib_dir}

CC = clang
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -Wpedantic -MMD -MP \
         -I${SQL_INCLUDE_DIR} ${WEBSOCKETS_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${UV_INCLUDE_DIR} ${JSON_C_INCLUDE_DIR}
LDFLAGS = -L$(SQL_LIB_DIR) ${WEBSOCKETS_LIB_DIR} -lwebsockets ${OPENSSL_LIB_DIR} -lssl -lcrypto ${UV_LIB_DIR} -luv -pthread ${JSON_C_LIB_DIR} -ljson-c -lX11

SRCDIR = src
INCDIR = inc
OBJDIR = obj
GTK_LIBS = -rdynamic `pkg-config --cflags --libs gtk+-3.0`
GTK_INC = `pkg-config --cflags gtk+-3.0`

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
DEPENDS = $(OBJECTS:.o=.d)



INCLUDES = -I$(INCDIR)

.PHONY: all clean uninstall reinstall

all: $(BINARYNAME)

$(BINARYNAME):$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(GTK_LIBS) -I $(LDFLAGS) -o $@ 

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(GTK_INC) -c $< -o $@ -I $(INCDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

-include $(DEPENDS)

clean:
	rm -rf $(OBJDIR)


uninstall: clean
	rm -f $(BINARYNAME)


reinstall: uninstall all