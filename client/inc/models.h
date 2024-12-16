#include "glib.h"

#ifndef CLIENT_MODELS
#define CLIENT_MODELS
typedef struct s_client_message{
    long long id;
    int  from_id;
    char* text;
    long long date;
} client_message;

typedef struct s_client_user{
    int id;
    char* login;
    void * picture;
} client_user;

typedef struct s_client_chat{
    int id;
    char* name;
    void * picture;
} client_chat;


extern GHashTable Chats;
#endif
