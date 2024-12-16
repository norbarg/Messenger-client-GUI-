#include "main.h"
#include "gui_active_elements.h"
#include <libwebsockets.h>
#include <signal.h>
#include <json-c/json.h>
#include "client.h"
#include "authentication_gui.h"
#include "endpoints.h"
#include <X11/Xlib.h>

struct lws *global_wsi = NULL;
char* session = NULL;
GThread *ws_thread = NULL;

void lws_easy_write(struct lws *wsi, const char *data, int len) {

    if (!wsi || !data || len <= 0) {
        fprintf(stderr, "Invalid parameters to lws_easy_write.\n");
        return;
    }

    unsigned char *buffer = calloc(len + LWS_PRE, sizeof(char));
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed in lws_easy_write.\n");
        return;
    }

    memcpy(buffer + LWS_PRE, data, len);

    int n = lws_write(wsi, buffer + LWS_PRE, len, LWS_WRITE_TEXT);
    if (n < len) {
        fprintf(stderr, "lws_write error: only %d of %d bytes sent.\n", n, len);
    }
    free(buffer);
}


static int callback_client(struct lws *wsi, enum lws_callback_reasons reason,
                           void *user_data, void *in, size_t len) {       
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            g_print("Connected");
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:{
            char* receivedText = g_strdup((char *)in);
            g_print("%s",receivedText);

            struct json_object* message_obj = json_tokener_parse(receivedText);
            
            struct json_object* reasonString;

            if (!json_object_object_get_ex(message_obj, "reason", &reasonString)) return 0;
            
            const char* reason = json_object_get_string(reasonString);

            if(!strcmp(reason, "AUTH"))
            {
                auth_endpoint(message_obj);
            }
            
            if(message_obj!=0){
                json_object_put(message_obj);
            }


            free(receivedText);
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            break;
        }

        case LWS_CALLBACK_CLIENT_CLOSED:
            g_print("Connection closed.");
            interrupted = 1;
            break;

        default:
            break;
    }
    (void)user_data;
    (void)in;
    (void)len;
    (void)wsi;
    (void)reason;
    return 0;
}

static gpointer websocket_thread_func(gpointer data) {
    struct lws_context *context;
    struct lws_context_creation_info info;
    struct lws_client_connect_info ccinfo = {0};

    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = (const struct lws_protocols[]){
        {.name = "example-protocol",
        .callback = callback_client,
        .per_session_data_size = sizeof(long)},
        { NULL, NULL, 0, 0, 0, 0, 0} 
    };
    context = lws_create_context(&info);

    if (!context) {
        printf("Failed to create WebSocket context.");
        return NULL;
    }

    ccinfo.context = context;
    ccinfo.address = "localhost"; 
    ccinfo.port = 8080;                     // Use 443 for wss://, 80 for ws://
    ccinfo.path = "/";
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = "example-protocol";
    ccinfo.ssl_connection = 0; // Use 0 for ws://
    //ccinfo.ssl_connection = LCCSCF_USE_SSL; // Use 0 for ws://

    global_wsi = lws_client_connect_via_info(&ccinfo);
    g_print("%lld",(long long)global_wsi);
    if (!global_wsi) {
        printf("Failed to connect to WebSocket server.");
        lws_context_destroy(context);
        return NULL;
    }

    while (!interrupted) {
        lws_service(context, 50); 
    }

    lws_context_destroy(context);
    (void)data;
    return NULL;
}

int main(int argc, char **argv) {
    
    XInitThreads();
    gtk_init(&argc,&argv);
    ws_thread = g_thread_new("WebSocketThread", websocket_thread_func, NULL);
    authentication_window();
    g_thread_join(ws_thread);
    
    return 0;
}
