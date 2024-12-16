#include "main.h"
#include <json-c/json.h>
#include "gui_active_elements.h"

void auth_endpoint(struct json_object* message_obj);

void fetch_chats(void);

void send_message_endpoint(char* text, int chat_id);

