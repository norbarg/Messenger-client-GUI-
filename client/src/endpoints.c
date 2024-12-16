#include "main.h"
#include "libwebsockets.h"
#include "endpoints.h"
#include <json-c/json.h>
#include "models.h"




void auth_endpoint(struct json_object* message_obj){
    bool success = true;
    struct json_object* succesString;
    if(json_object_object_get_ex(message_obj, "result", &succesString)){
        const char* successValue = json_object_get_string(succesString);
        success = strcmp(successValue, "success") == 0;
    }
    if (success) { 
        struct json_object* sessionString;
        if(json_object_object_get_ex(message_obj,"session",&sessionString)){
            session = strdup(json_object_get_string(sessionString));
        }
        gdk_threads_add_idle((GSourceFunc)go_to_main_client, GTK_WINDOW(loginData->window));


        struct json_object* chatsObjArr;
        json_object_object_get_ex(message_obj, "chats", &chatsObjArr);
        int count = json_object_array_length(chatsObjArr);
        for(int i=0;i<count;i++){
            struct json_object* chatObj=json_object_array_get_idx(chatsObjArr, i);
            if(!chatObj)continue;;
            struct json_object* nameString;
            struct json_object* idObject;
            if(!json_object_object_get_ex(chatObj, "name", &nameString))continue;
            if(!json_object_object_get_ex(chatObj, "id", &idObject))continue;
            char* name = strdup(json_object_get_string(nameString));
            int id = json_object_get_int(idObject);
            client_chat* createData = malloc(sizeof(client_chat));
            createData->name = name;
            createData->id = id;
            gdk_threads_add_idle((GSourceFunc)create_contact_gui_elements, createData);
        }
        fetch_chats();
        g_free(loginData);

        
    } else {
        gdk_threads_add_idle((GSourceFunc)show_login_error, NULL);
    }
}

void fetch_chats(void){

    struct json_object* message_obj = json_object_new_object();

    json_object_object_add(message_obj, "reason", json_object_new_string("FETCH_CHATS"));
    json_object_object_add(message_obj, "session", json_object_new_string(session));

    
    const char* json = json_object_to_json_string(message_obj);
    g_print("%lld", (long long)global_wsi);
    lws_easy_write(global_wsi, json, strlen(json));

    json_object_put(message_obj);
}

void send_message_endpoint(char* text, int chat_id){
    struct json_object* message_obj = json_object_new_object();

    json_object_object_add(message_obj, "reason", json_object_new_string("SEND_MESSAGE"));
    json_object_object_add(message_obj, "text", json_object_new_string(text));
    json_object_object_add(message_obj, "chat_id", json_object_new_int(chat_id));
    json_object_object_add(message_obj, "session", json_object_new_string(session));

    const char* json = json_object_to_json_string(message_obj);

    lws_easy_write(global_wsi, json, strlen(json));

    json_object_put(message_obj);
}


void incoming_chats_enpoint(struct json_object* message_obj){
    bool success = true;
    struct json_object* succesString;
    if(json_object_object_get_ex(message_obj, "result", &succesString)){
        const char* successValue = json_object_get_string(succesString);
        success = strcmp(successValue, "success") == 0;
    }
    if (success) { 
        go_to_main_client(GTK_WINDOW(loginData->window));
        g_free(loginData);
    } else {
        gtk_label_set_text(GTK_LABEL(loginData->error_label), "Incorrect username or password.");
        gtk_widget_show(loginData->error_label);
    }
}


