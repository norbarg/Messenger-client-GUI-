#ifndef CLIENT_H
#define CLIENT_H

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include "models.h"

typedef struct _Chat {
    int server_id;
    GtkTextView *message_entry;
    GtkWidget *messages_box;
    GtkWidget *scrolled_window;
    GtkWidget *chat_area;
    GtkWidget *message_entry_container;
    GtkWidget *toggle_button;
    gchar *chat_name; 
    gchar *contact_name; 
    gulong handler_id;

    gboolean is_editing;
    GtkWidget *editing_message_label;
    GtkWidget *editing_time_label;

    gboolean is_replying;
    gchar *reply_to_message_text;
    GtkWidget *reply_preview; 

    gboolean is_group;        
    GList *members;  
} Chat;

typedef struct {
    Chat *chat;
    GtkWidget *message_label;
    GtkWidget *time_label;
    GtkWidget *popover;
} EditData;

typedef struct {
    Chat *chat;
    GtkWidget *message_label;
    GtkWidget *popover;
} ReplyData;

    typedef struct {
        GtkWidget *username_entry;
        GtkWidget *password_entry;
        GtkWidget *profile_window;
    } ProfileData;

typedef struct {
    GtkWidget *members_entry;
    GtkWidget *window;
    Chat *chat;
} AddMembersData;

void general_client(void);
void initialize_client_buttons(GtkWidget *vbox);
void change_contact(GtkToggleButton *toggle_button, gpointer user_data);
bool create_contact_gui_elements(client_chat* CreateData);
void show_user_not_found_popup(GtkWindow *parent, const gchar *username);

#endif 
