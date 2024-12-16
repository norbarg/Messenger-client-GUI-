#include <gtk/gtk.h>
#include <stdbool.h>
#include "authentication_gui.h"
#include "client.h"

extern char* session;
extern struct lws *global_wsi;
extern GThread *ws_thread;
#ifndef MAIN_H
#define MAIN_H 

static int interrupted = 0;

void lws_easy_write(struct lws* to, const char* data, int len);
#endif
