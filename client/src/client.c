#include "../inc/client.h"
#include "authentication_gui.h"
#include "endpoints.h"
#include "models.h"
GtkWidget *main_window = NULL;
extern gchar *current_username;
GList *chat_list = NULL;

GtkWidget *center_label_global;
GtkCssProvider *provider_client = NULL;
static GtkWidget *feature_buttons_box = NULL;
static GtkWidget *overlay_global = NULL;
GtkWidget *side_panel_global = NULL;
GtkWidget *main_content_global = NULL;
GtkWidget *top_bar_global = NULL;
GtkWidget *chat_container_global = NULL;
GtkWidget *chat_name_label_global = NULL;

GtkWidget *menu_button_global = NULL;
Chat *active_chat_global = NULL;
GtkWidget *menu_popover_global = NULL;


void general_client(void){

    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "MASSANGER");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

    GtkWidget *window_client = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    //gtk_window_set_title(GTK_WINDOW(window_client), "MASSANGER");
    gtk_window_set_titlebar(GTK_WINDOW(window_client), header_bar);
    gtk_window_set_default_size(GTK_WINDOW(window_client), 1920, 1080);
    gtk_window_set_resizable(GTK_WINDOW(window_client), TRUE);

    main_window = window_client; 
  
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window_client), vbox);

    initialize_client_buttons(vbox);

    g_signal_connect(window_client, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window_client);
}

void apply_light_theme(void) {
    if (provider_client != NULL) {
        gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(),
                                                     GTK_STYLE_PROVIDER(provider_client));
        g_object_unref(provider_client);
        provider_client = NULL;
    }

    provider_client = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider_client, "style_client.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider_client),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void apply_dark_theme(void) {
    if (provider_client != NULL) {
        gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(),
                                                     GTK_STYLE_PROVIDER(provider_client));
        g_object_unref(provider_client);
        provider_client = NULL;
    }

    provider_client = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider_client, "style_dark_client.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider_client),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void on_change_theme_toggled(GtkToggleButton *toggle_button, gpointer user_data) {
    (void)user_data;

    gboolean active = gtk_toggle_button_get_active(toggle_button);

    if (active) {
        apply_dark_theme();
    } else {
        apply_light_theme();
    }
}

void on_reply_preview_close_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    Chat *chat = (Chat *)user_data;

    chat->is_replying = FALSE;
    if (chat->reply_to_message_text) {
        g_free(chat->reply_to_message_text);
        chat->reply_to_message_text = NULL;
    }

    if (chat->reply_preview) {
        gtk_widget_destroy(chat->reply_preview);
        chat->reply_preview = NULL;
    }
}

void on_reply_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    ReplyData *reply_data = (ReplyData *)user_data;
    Chat *chat = reply_data->chat;
    GtkWidget *message_label = reply_data->message_label;
    GtkWidget *popover = reply_data->popover;

    const gchar *original_message = gtk_label_get_text(GTK_LABEL(message_label));

    int quote_limit = 20; 

    gchar *truncated_message;
    if (g_utf8_strlen(original_message, -1) > quote_limit) {
        truncated_message = g_strndup(original_message, g_utf8_offset_to_pointer(original_message, quote_limit) - original_message);
        gchar *temp = truncated_message;
        truncated_message = g_strconcat(truncated_message, "...", NULL);
        g_free(temp);
    } else {
        truncated_message = g_strdup(original_message);
    }

    chat->is_replying = TRUE;
    chat->reply_to_message_text = truncated_message; 

    if (chat->reply_preview) {
        gtk_widget_destroy(chat->reply_preview);
    }

    chat->reply_preview = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *close_button = gtk_button_new_with_label("✖️");
    gtk_button_set_relief(GTK_BUTTON(close_button), GTK_RELIEF_NONE);
    GtkWidget *preview_label = gtk_label_new(truncated_message);
    gtk_label_set_line_wrap(GTK_LABEL(preview_label), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(preview_label), 30);

    gtk_box_pack_start(GTK_BOX(chat->reply_preview), close_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(chat->reply_preview), preview_label, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(chat->message_entry_container), chat->reply_preview, FALSE, FALSE, 0);
    gtk_widget_show_all(chat->reply_preview);

    g_signal_connect(close_button, "clicked", G_CALLBACK(on_reply_preview_close_clicked), chat);

    gtk_widget_hide(popover);

    gtk_widget_grab_focus(GTK_WIDGET(chat->message_entry));
}

void on_edit_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    EditData *edit_data = (EditData *)user_data;
    Chat *chat = edit_data->chat;
    GtkWidget *message_label = edit_data->message_label;
    GtkWidget *time_label = edit_data->time_label;
    GtkWidget *popover = edit_data->popover;

    const gchar *message_text = gtk_label_get_text(GTK_LABEL(message_label));

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(chat->message_entry);

    gtk_text_buffer_set_text(buffer, message_text, -1);

    chat->is_editing = TRUE;
    chat->editing_message_label = message_label;
    chat->editing_time_label = time_label;

    gtk_widget_hide(popover);

    gtk_widget_grab_focus(GTK_WIDGET(chat->message_entry));
}

void on_delete_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *message_box = GTK_WIDGET(user_data);
    GtkWidget *popover = gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(button)));

    gtk_widget_hide(popover);

    gtk_widget_destroy(message_box);
}

gboolean on_message_bubble_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    (void)widget;
    GtkPopover *popover = GTK_POPOVER(user_data);

    if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_SECONDARY) {
        if (gtk_widget_get_visible(GTK_WIDGET(popover))) {
            gtk_widget_hide(GTK_WIDGET(popover));
        } else {
            gtk_widget_show_all(GTK_WIDGET(popover)); 
            gtk_popover_popup(popover);
        }
        return TRUE; 
    }
    return FALSE; 
}

void get_message(Chat *chat, const gchar *message_text) {
    if (chat == NULL || chat->messages_box == NULL || chat->scrolled_window == NULL) {
        g_warning("One or more pointers in Chat structure are NULL");
        return;
    }

    GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(message_box, GTK_ALIGN_START); 
    gtk_widget_set_hexpand(message_box, FALSE);

    GtkWidget *message_label = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(message_label), message_text);
    gtk_label_set_line_wrap(GTK_LABEL(message_label), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(message_label), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_max_width_chars(GTK_LABEL(message_label), 50);
    gtk_widget_set_halign(message_label, GTK_ALIGN_START);

    GtkWidget *username_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(username_container, GTK_ALIGN_FILL);

    GtkWidget *username_label = gtk_label_new(chat->contact_name);
    gtk_widget_set_name(username_label, "username_label");
    gtk_widget_set_halign(username_label, GTK_ALIGN_START); 
    gtk_box_pack_start(GTK_BOX(username_container), username_label, FALSE, FALSE, 0);

    GtkWidget *message_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(message_container), username_container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(message_container), message_label, FALSE, FALSE, 0);

    GtkWidget *bubble = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(bubble, "incoming_message_bubble"); 
    gtk_widget_set_hexpand(bubble, FALSE);
    gtk_container_add(GTK_CONTAINER(bubble), message_container);

    GtkWidget *bubble_event_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(bubble_event_box), bubble);

    GtkWidget *popover = gtk_popover_new(bubble_event_box);
    gtk_widget_set_name(popover, "message_popover");

    GtkWidget *popover_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *reply_button = gtk_button_new_with_label("Reply");
    GtkWidget *delete_button = gtk_button_new_with_label("Delete");
    gtk_widget_set_name(reply_button, "reply_button");
    gtk_widget_set_name(delete_button, "delete_button");

    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_button_clicked), message_box);

    ReplyData *reply_data = g_new0(ReplyData, 1);
    reply_data->chat = chat;
    reply_data->message_label = message_label;
    reply_data->popover = popover;

    g_signal_connect(reply_button, "clicked", G_CALLBACK(on_reply_button_clicked), reply_data);

    gtk_box_pack_start(GTK_BOX(popover_box), reply_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), delete_button, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(popover), popover_box);

    gtk_widget_add_events(bubble_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(bubble_event_box), "button-press-event", G_CALLBACK(on_message_bubble_button_press), popover);

    time_t rawtime;
    struct tm *timeinfo;
    char buffer[10];
    time(&rawtime);
    timeinfo =localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);

    GtkWidget *time_label = gtk_label_new(buffer);
    gtk_widget_set_name(time_label, "time_label");
    gtk_widget_set_halign(time_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(bubble), time_label, FALSE, FALSE, 0);

    GtkWidget *avatar_image = gtk_image_new_from_icon_name("avatar-default-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_widget_set_size_request(avatar_image, 40, 40);
    gtk_widget_set_name(avatar_image, "avatar_image");

    GtkWidget *avatar_container = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(avatar_container), avatar_image);

    gtk_box_pack_start(GTK_BOX(message_box), avatar_container, FALSE, FALSE, 15);
    gtk_box_pack_start(GTK_BOX(message_box), bubble_event_box, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(chat->messages_box), message_box, FALSE, FALSE, 5);

    GtkTextBuffer *buffers = gtk_text_view_get_buffer(chat->message_entry);
    gtk_text_buffer_set_text(buffers, "", -1);

    gtk_widget_show_all(chat->chat_area);

    GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(chat->scrolled_window));
    gtk_adjustment_set_value(adjustment, gtk_adjustment_get_upper(adjustment));
}

void send_message(GtkButton *button, gpointer user_data) {
    (void)button;

    Chat *chat = (Chat *)user_data;

    if (chat == NULL || chat->message_entry == NULL || chat->messages_box == NULL || chat->scrolled_window == NULL) {
        g_warning("One or more pointers in Chat structure are NULL");
        return;
    }

    GtkTextBuffer *buffers = gtk_text_view_get_buffer(chat->message_entry);
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffers, &start);
    gtk_text_buffer_get_end_iter(buffers, &end);
    gchar *message_text = gtk_text_buffer_get_text(buffers, &start, &end, FALSE);

    if (g_strcmp0(message_text, "") == 0) {
        return; 
    }

    if (chat->is_editing && chat->editing_message_label != NULL && chat->editing_time_label != NULL) {

        gtk_label_set_text(GTK_LABEL(chat->editing_message_label), message_text);

        gchar buffer[20];
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);

        gchar *edited_time_text = g_strdup_printf("%s (edited)", buffer);
        gtk_label_set_text(GTK_LABEL(chat->editing_time_label), edited_time_text);
        g_free(edited_time_text);

        chat->is_editing = FALSE;
        chat->editing_message_label = NULL;
        chat->editing_time_label = NULL;

        GtkTextBuffer *buffer_edit = gtk_text_view_get_buffer(chat->message_entry);
        gtk_text_buffer_set_text(buffer_edit, "", -1);

        return;
    }

    //send_message_endpoint(message_text, chat->server_id);

    GtkWidget *message_content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    if (chat->is_replying && chat->reply_to_message_text != NULL) {
        const gchar *original_message = chat->reply_to_message_text;

        GtkWidget *quote_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_widget_set_name(quote_box, "quote_box");

        GtkWidget *quote_label = gtk_label_new(original_message);
        gtk_label_set_line_wrap(GTK_LABEL(quote_label), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(quote_label), 30);
        gtk_widget_set_halign(quote_label, GTK_ALIGN_START);

        gtk_box_pack_start(GTK_BOX(quote_box), quote_label, TRUE, TRUE, 0);

        gtk_box_pack_start(GTK_BOX(message_content_box), quote_box, FALSE, FALSE, 0);

        chat->is_replying = FALSE;
        g_free(chat->reply_to_message_text); 
        chat->reply_to_message_text = NULL;

        if (chat->reply_preview) {
            gtk_widget_destroy(chat->reply_preview);
            chat->reply_preview = NULL;
        }
    }

    // 
    if (message_text[0] == '$') {
        const gchar *incoming_message_text = message_text + 1;
        get_message(chat, incoming_message_text);

        gtk_text_buffer_set_text(buffers, "", -1);

        return;
    }

    GtkWidget *message_label = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(message_label), message_text);
    gtk_label_set_line_wrap(GTK_LABEL(message_label), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(message_label), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_max_width_chars(GTK_LABEL(message_label), 50); 
    gtk_widget_set_halign(message_label, GTK_ALIGN_START); 

    gtk_box_pack_start(GTK_BOX(message_content_box), message_label, FALSE, FALSE, 0);

    GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(message_box, GTK_ALIGN_END);
    gtk_widget_set_hexpand(message_box, FALSE);

    GtkWidget *username_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(username_container, GTK_ALIGN_FILL);

    GtkWidget *username_label = gtk_label_new(current_username);
    gtk_widget_set_name(username_label, "username_label");
    gtk_widget_set_halign(username_label, GTK_ALIGN_END);
    gtk_box_pack_end(GTK_BOX(username_container), username_label, FALSE, FALSE, 0);

    GtkWidget *message_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(message_container), username_container, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(message_container), message_content_box, FALSE, FALSE, 0);

    GtkWidget *bubble = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(bubble, "outgoing_message_bubble");
    gtk_widget_set_hexpand(bubble, FALSE);
    gtk_container_add(GTK_CONTAINER(bubble), message_container);

    GtkWidget *bubble_event_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(bubble_event_box), bubble);

    GtkWidget *popover = gtk_popover_new(bubble_event_box);
    gtk_widget_set_name(popover, "message_popover");

    GtkWidget *popover_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *edit_button = gtk_button_new_with_label("Edit");
    GtkWidget *reply_button = gtk_button_new_with_label("Reply");
    GtkWidget *delete_button = gtk_button_new_with_label("Delete");
    gtk_widget_set_name(edit_button, "edit_button");
    gtk_widget_set_name(reply_button, "reply_button");
    gtk_widget_set_name(delete_button, "delete_button");

    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_button_clicked), message_box);

    EditData *edit_data = g_new0(EditData, 1);
    edit_data->chat = chat;
    edit_data->message_label = message_label;
    edit_data->popover = popover;

    g_signal_connect(edit_button, "clicked", G_CALLBACK(on_edit_button_clicked), edit_data);

    ReplyData *reply_data = g_new0(ReplyData, 1);
    reply_data->chat = chat;
    reply_data->message_label = message_label;
    reply_data->popover = popover;

    g_signal_connect(reply_button, "clicked", G_CALLBACK(on_reply_button_clicked), reply_data);

    gtk_box_pack_start(GTK_BOX(popover_box), edit_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), reply_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), delete_button, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(popover), popover_box);

    gtk_widget_add_events(bubble_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(bubble_event_box), "button-press-event", G_CALLBACK(on_message_bubble_button_press), popover);

    time_t rawtime;
    struct tm *timeinfo;
    char buffer[10];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);


    GtkWidget *time_label = gtk_label_new(buffer);
    gtk_widget_set_name(time_label, "time_label");
    gtk_widget_set_halign(time_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(bubble), time_label, FALSE, FALSE, 0);

    edit_data->time_label = time_label;

    GtkWidget *avatar_image = gtk_image_new_from_icon_name("avatar-default-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_widget_set_size_request(avatar_image, 40, 40);
    gtk_widget_set_name(avatar_image, "avatar_image");

    GtkWidget *avatar_container = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(avatar_container), avatar_image);

    gtk_box_pack_end(GTK_BOX(message_box), avatar_container, FALSE, FALSE, 15);
    gtk_box_pack_end(GTK_BOX(message_box), bubble_event_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(chat->messages_box), message_box, FALSE, FALSE, 5);

    gtk_widget_show_all(chat->chat_area);

    GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(chat->scrolled_window));
    gtk_adjustment_set_value(adjustment, gtk_adjustment_get_upper(adjustment));

    gtk_text_buffer_set_text(buffers, "", -1);

    g_free(message_text);
}

static gboolean on_message_entry_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    (void)widget;
    Chat *chat = (Chat *)user_data;
    
    if ((event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) &&
        (event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_MOD1_MASK)) == 0) {

        send_message(NULL, chat);
        return TRUE; 
    }

    return FALSE; 
}

void creation_contact(GtkButton *button, gpointer user_data) {
    GtkWidget *contact_window = GTK_WIDGET(user_data);
    GtkWidget *name_entry = g_object_get_data(G_OBJECT(button), "name_entry");

    const gchar *name = gtk_entry_get_text(GTK_ENTRY(name_entry));

    if (g_strcmp0(name, "") != 0) {

        if (name[0] == '_') {
            show_user_not_found_popup(GTK_WINDOW(contact_window), name);
            return;
        }
    }

    client_chat* creation_data = malloc(sizeof(client_chat));
    creation_data->name = (char*)name;
    creation_data->id = -1;

    create_contact_gui_elements(creation_data);

    free(creation_data);

    gtk_widget_destroy(contact_window);
}

bool create_contact_gui_elements(client_chat* CreateData){
    GtkWidget *toggle_button = gtk_toggle_button_new();
    gtk_widget_set_name(toggle_button, "contact_button");
    gtk_button_set_relief(GTK_BUTTON(toggle_button), GTK_RELIEF_NONE);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    GtkWidget *avatar_icon = gtk_image_new_from_icon_name("avatar-default-symbolic", 5);
    GtkWidget *label = gtk_label_new(strdup(CreateData->name));
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(hbox), avatar_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(toggle_button), hbox);

    gtk_box_pack_start(GTK_BOX(side_panel_global), toggle_button, FALSE, FALSE, 0);
    gtk_widget_show_all(side_panel_global);

    Chat *new_chat = g_new0(Chat, 1);
    new_chat->toggle_button = toggle_button;
    new_chat->chat_name = g_strdup(CreateData->name);
    new_chat->contact_name = g_strdup(CreateData->name); 
    new_chat->server_id = CreateData->id;

    GtkWidget *chat_area_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(chat_area_box, "chat_area_box");
    gtk_widget_set_hexpand(chat_area_box, TRUE);
    gtk_widget_set_vexpand(chat_area_box, TRUE);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_vexpand(scrolled_window, TRUE);

    GtkWidget *messages_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(messages_box, "messages_box");
    gtk_widget_set_hexpand(messages_box, TRUE);
    gtk_widget_set_vexpand(messages_box, TRUE);
    gtk_widget_set_valign(messages_box, GTK_ALIGN_END);

    gtk_container_add(GTK_CONTAINER(scrolled_window), messages_box);
    gtk_box_pack_start(GTK_BOX(chat_area_box), scrolled_window, TRUE, TRUE, 0);

    new_chat->message_entry_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(new_chat->message_entry_container, "message_entry_container");
    gtk_widget_set_hexpand(new_chat->message_entry_container, TRUE);

    new_chat->message_entry = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_widget_set_name(GTK_WIDGET(new_chat->message_entry), "message_entry");

    gtk_text_view_set_wrap_mode(new_chat->message_entry, GTK_WRAP_WORD_CHAR);
    gtk_widget_set_vexpand(GTK_WIDGET(new_chat->message_entry), FALSE);
    gtk_widget_set_hexpand(GTK_WIDGET(new_chat->message_entry), TRUE);

    gtk_widget_set_size_request(GTK_WIDGET(new_chat->message_entry), -1, 50);

    GtkWidget *message_entry_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(message_entry_scrolled_window), 0);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(message_entry_scrolled_window), GTK_SHADOW_NONE);
    gtk_widget_set_name(GTK_WIDGET(message_entry_scrolled_window), "message_entry_scrolled_window");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(message_entry_scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(message_entry_scrolled_window, TRUE);
    gtk_widget_set_vexpand(message_entry_scrolled_window, FALSE);
    

   gtk_container_add(GTK_CONTAINER(message_entry_scrolled_window), GTK_WIDGET(new_chat->message_entry));

    gtk_box_pack_start(GTK_BOX(new_chat->message_entry_container), message_entry_scrolled_window, FALSE, TRUE, 5);

    GtkWidget *send_button = gtk_button_new();
    GtkWidget *send_icon = gtk_image_new_from_icon_name("mail-send-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(send_button), send_icon);
    gtk_button_set_relief(GTK_BUTTON(send_button), GTK_RELIEF_NONE);
    gtk_widget_set_name(send_button, "send_button");
    gtk_widget_set_size_request(send_button, 50, 50);
    gtk_widget_set_hexpand(send_button, FALSE);

    gtk_box_pack_start(GTK_BOX(new_chat->message_entry_container), send_button, FALSE, FALSE, 0);

    g_signal_connect(send_button, "clicked", G_CALLBACK(send_message), new_chat);

    g_signal_connect(G_OBJECT(new_chat->message_entry), "key-press-event", G_CALLBACK(on_message_entry_key_press), new_chat);

    gtk_box_pack_end(GTK_BOX(chat_area_box), new_chat->message_entry_container, FALSE, FALSE, 5);

    new_chat->chat_area = chat_area_box;
    new_chat->scrolled_window = scrolled_window;
    new_chat->messages_box = messages_box;

    gtk_widget_hide(new_chat->chat_area);

    gtk_box_pack_start(GTK_BOX(chat_container_global), new_chat->chat_area, TRUE, TRUE, 0);

    chat_list = g_list_append(chat_list, new_chat);

    new_chat->handler_id = g_signal_connect(toggle_button, "toggled", G_CALLBACK(change_contact), new_chat);
    return FALSE; 
}

void change_contact(GtkToggleButton *toggle_button, gpointer user_data) {
    Chat *chat = (Chat *)user_data;

    if (gtk_toggle_button_get_active(toggle_button)) {
        gtk_label_set_text(GTK_LABEL(chat_name_label_global), chat->chat_name);

        gtk_widget_show_all(chat->chat_area);

        gtk_widget_show(menu_button_global);

        active_chat_global = chat;

        GList *iter = chat_list;
        while (iter != NULL) {
            Chat *other_chat = (Chat *)iter->data;
            if (other_chat != chat) {
                g_signal_handler_block(other_chat->toggle_button, other_chat->handler_id);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(other_chat->toggle_button), FALSE);
                g_signal_handler_unblock(other_chat->toggle_button, other_chat->handler_id);
                gtk_widget_hide(other_chat->chat_area);
            }
            iter = iter->next;
        }
    } else {
        gtk_widget_hide(chat->chat_area);

        gboolean any_active = FALSE;
        GList *iter = chat_list;
        while (iter != NULL) {
            Chat *other_chat = (Chat *)iter->data;
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(other_chat->toggle_button))) {
                any_active = TRUE;
                break;
            }
            iter = iter->next;
        }

        if (!any_active) {
            gtk_label_set_text(GTK_LABEL(chat_name_label_global), "");
            gtk_widget_hide(menu_button_global);
            active_chat_global = NULL;
        }
    }
}


void creation_group(GtkButton *button, gpointer user_data) {
    GtkWidget *group_window = GTK_WIDGET(user_data);
    GtkWidget *title_entry = g_object_get_data(G_OBJECT(button), "title_entry");
    GtkWidget *members_entry = g_object_get_data(G_OBJECT(button), "members_entry"); 

    const gchar *title = gtk_entry_get_text(GTK_ENTRY(title_entry));
    const gchar *members_text = gtk_entry_get_text(GTK_ENTRY(members_entry)); 

    GtkWidget *toggle_button = gtk_toggle_button_new();
    gtk_widget_set_name(toggle_button, "group_button");
    gtk_button_set_relief(GTK_BUTTON(toggle_button), GTK_RELIEF_NONE);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    GtkWidget *avatar_icon = gtk_image_new_from_icon_name("emblem-people", 5);
    GtkWidget *label = gtk_label_new(title);
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(hbox), avatar_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(toggle_button), hbox);

    gtk_box_pack_start(GTK_BOX(side_panel_global), toggle_button, FALSE, FALSE, 0);
    gtk_widget_show_all(side_panel_global);

    Chat *new_chat = g_new0(Chat, 1);
    new_chat->toggle_button = toggle_button;
    new_chat->chat_name = g_strdup(title);
    new_chat->contact_name = g_strdup(title);

    new_chat->is_group = TRUE;

    if (members_text && g_strcmp0(members_text, "") != 0) {
        gchar **members_array = g_strsplit(members_text, ",", -1);
        for (int i = 0; members_array[i] != NULL; i++) {
            gchar *member = g_strstrip(members_array[i]);
            new_chat->members = g_list_append(new_chat->members, g_strdup(member));
        }
        g_strfreev(members_array);
    }

    GtkWidget *chat_area_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(chat_area_box, "chat_area_box");
    gtk_widget_set_hexpand(chat_area_box, TRUE);
    gtk_widget_set_vexpand(chat_area_box, TRUE);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_vexpand(scrolled_window, TRUE);

    GtkWidget *messages_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(messages_box, "messages_box");
    gtk_widget_set_hexpand(messages_box, TRUE);
    gtk_widget_set_vexpand(messages_box, TRUE);
    gtk_widget_set_valign(messages_box, GTK_ALIGN_END);

    gtk_container_add(GTK_CONTAINER(scrolled_window), messages_box);
    gtk_box_pack_start(GTK_BOX(chat_area_box), scrolled_window, TRUE, TRUE, 0);

    new_chat->message_entry_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(new_chat->message_entry_container, "message_entry_container");
    gtk_widget_set_hexpand(new_chat->message_entry_container, TRUE);

    new_chat->message_entry = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_widget_set_name(GTK_WIDGET(new_chat->message_entry), "message_entry");
    gtk_text_view_set_wrap_mode(new_chat->message_entry, GTK_WRAP_WORD_CHAR);
    gtk_widget_set_vexpand(GTK_WIDGET(new_chat->message_entry), FALSE);
    gtk_widget_set_hexpand(GTK_WIDGET(new_chat->message_entry), TRUE);

    gtk_widget_set_size_request(GTK_WIDGET(new_chat->message_entry), -1, 50);

    GtkWidget *message_entry_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(message_entry_scrolled_window), 0);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(message_entry_scrolled_window), GTK_SHADOW_NONE);
    gtk_widget_set_name(GTK_WIDGET(message_entry_scrolled_window), "message_entry_scrolled_window");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(message_entry_scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(message_entry_scrolled_window, TRUE);
    gtk_widget_set_vexpand(message_entry_scrolled_window, FALSE);

    gtk_container_add(GTK_CONTAINER(message_entry_scrolled_window), GTK_WIDGET(new_chat->message_entry));

    gtk_box_pack_start(GTK_BOX(new_chat->message_entry_container), message_entry_scrolled_window, FALSE, TRUE, 5);


    GtkWidget *send_button = gtk_button_new();
    GtkWidget *send_icon = gtk_image_new_from_icon_name("mail-send-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(send_button), send_icon);
    gtk_button_set_relief(GTK_BUTTON(send_button), GTK_RELIEF_NONE);
    gtk_widget_set_name(send_button, "send_button");
    gtk_widget_set_size_request(send_button, 50, 50);
    gtk_widget_set_hexpand(send_button, FALSE);

    //gtk_box_pack_start(GTK_BOX(new_chat->message_entry_container), new_chat->message_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(new_chat->message_entry_container), send_button, FALSE, FALSE, 0);

    g_signal_connect(send_button, "clicked", G_CALLBACK(send_message), new_chat);

    g_signal_connect(G_OBJECT(new_chat->message_entry), "key-press-event", G_CALLBACK(on_message_entry_key_press), new_chat);

    gtk_box_pack_end(GTK_BOX(chat_area_box), new_chat->message_entry_container, FALSE, FALSE, 5);

    new_chat->chat_area = chat_area_box;
    new_chat->scrolled_window = scrolled_window;
    new_chat->messages_box = messages_box;

    gtk_widget_hide(new_chat->chat_area);

    gtk_box_pack_start(GTK_BOX(chat_container_global), new_chat->chat_area, TRUE, TRUE, 0);

    chat_list = g_list_append(chat_list, new_chat);

    new_chat->handler_id = g_signal_connect(toggle_button, "toggled", G_CALLBACK(change_contact), new_chat);

    gtk_widget_destroy(group_window);
}

void create_group_box(void) {

    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Create Group");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

    GtkWidget *group_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_titlebar(GTK_WINDOW(group_window), header_bar);
    gtk_window_set_default_size(GTK_WINDOW(group_window), 300, 200);
    gtk_window_set_resizable(GTK_WINDOW(group_window), FALSE);
    gtk_widget_set_name(group_window, "group_window");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(group_window), vbox);

    GtkWidget *title_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(title_entry), "enter title...");
    gtk_widget_set_name(title_entry, "title_entry");
    gtk_box_pack_start(GTK_BOX(vbox), title_entry, FALSE, FALSE, 10);

    GtkWidget *members_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(members_entry), "add members (comma separated)...");
    gtk_widget_set_name(members_entry, "members_entry");
    gtk_box_pack_start(GTK_BOX(vbox), members_entry, FALSE, FALSE, 5);

    GtkWidget *enter_button = gtk_button_new_with_label("enter");
    gtk_widget_set_name(enter_button, "enter_button");
    gtk_box_pack_start(GTK_BOX(vbox), enter_button, FALSE, FALSE, 10);

    g_object_set_data(G_OBJECT(enter_button), "title_entry", title_entry);
    g_object_set_data(G_OBJECT(enter_button), "members_entry", members_entry);

    g_signal_connect(enter_button, "clicked", G_CALLBACK(creation_group), group_window);

    gtk_widget_show_all(group_window);
}

void create_contact_box(void) {

    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Create Chat");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

    GtkWidget *contact_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_titlebar(GTK_WINDOW(contact_window), header_bar);
    gtk_window_set_default_size(GTK_WINDOW(contact_window), 250, 100);
    gtk_window_set_resizable(GTK_WINDOW(contact_window), FALSE);
    gtk_widget_set_name(contact_window, "contact_window");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(contact_window), vbox); 

    GtkWidget *name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "enter name...");
    gtk_widget_set_name(name_entry, "name_entry");
    gtk_box_pack_start(GTK_BOX(vbox), name_entry, TRUE, TRUE, 10);

    GtkWidget *enter_button = gtk_button_new_with_label("enter");
    gtk_widget_set_name(enter_button, "enter_button");
    gtk_box_pack_start(GTK_BOX(vbox), enter_button, FALSE, FALSE, 10);

    g_object_set_data(G_OBJECT(enter_button), "name_entry", name_entry);
    g_signal_connect(enter_button, "clicked", G_CALLBACK(creation_contact), contact_window);

    gtk_widget_show_all(contact_window);
}

void hide_and_create_contact_box(void) {
     if (center_label_global != NULL) {
        gtk_widget_hide(center_label_global); 
        center_label_global = NULL;
    } 
    gtk_widget_hide(feature_buttons_box);
    create_contact_box();
}

void hide_and_create_group_box(void) {
     if (center_label_global != NULL) {
        gtk_widget_hide(center_label_global); 
        center_label_global = NULL; 
    } 
    gtk_widget_hide(feature_buttons_box);
    create_group_box();
}

void hide_center_label(void) {
    if (center_label_global != NULL) {
        gtk_widget_hide(center_label_global);
        center_label_global = NULL; 
    } 
}

void on_profile_confirm_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    ProfileData *data = (ProfileData *)user_data;

    const gchar *new_username = gtk_entry_get_text(GTK_ENTRY(data->username_entry));
    const gchar *new_password = gtk_entry_get_text(GTK_ENTRY(data->password_entry));

    if (g_strcmp0(new_username, "") != 0) {
        g_free(current_username);
        current_username = g_strdup(new_username);
    }

    if (g_strcmp0(new_password, "") != 0) {
        g_free(current_password);
        current_password = g_strdup(new_password);
    }

    gtk_widget_destroy(data->profile_window);

    g_free(data);
}

void open_profile_window(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;

    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Profile");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

    GtkWidget *profile_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_titlebar(GTK_WINDOW(profile_window), header_bar);
    gtk_window_set_default_size(GTK_WINDOW(profile_window), 300, 400);
    gtk_window_set_resizable(GTK_WINDOW(profile_window), FALSE);
    gtk_widget_set_name(profile_window, "profile_window");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(profile_window), vbox);

    GtkWidget *avatar_icon = gtk_image_new_from_icon_name("avatar-default-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_widget_set_name(avatar_icon, "prof_avatar_icon");
    gtk_widget_set_halign(avatar_icon, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), avatar_icon, FALSE, FALSE, 10);

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Change user name");
    gtk_entry_set_text(GTK_ENTRY(username_entry), current_username); 
    gtk_box_pack_start(GTK_BOX(vbox), username_entry, FALSE, FALSE, 10);
    gtk_widget_set_name(username_entry, "username_entry");

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Change password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE); 
    gtk_box_pack_start(GTK_BOX(vbox), password_entry, FALSE, FALSE, 10);
    gtk_widget_set_name(password_entry, "password_entry");

    GtkWidget *confirm_button = gtk_button_new_with_label("Confirm");
    gtk_widget_set_name(confirm_button, "confirm_button");
    gtk_box_pack_end(GTK_BOX(vbox), confirm_button, FALSE, FALSE, 10);

    

    ProfileData *data = g_new(ProfileData, 1);
    data->username_entry = username_entry;
    data->password_entry = password_entry;
    data->profile_window = profile_window;

    g_signal_connect(confirm_button, "clicked", G_CALLBACK(on_profile_confirm_clicked), data);

    gtk_widget_show_all(profile_window);
}

void on_delete_chat_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkWidget *popover = GTK_WIDGET(user_data);

    if (active_chat_global == NULL) {
        gtk_widget_hide(popover);
        return;
    }

    chat_list = g_list_remove(chat_list, active_chat_global);

    gtk_widget_destroy(active_chat_global->chat_area);
    gtk_widget_destroy(active_chat_global->toggle_button);

    g_free(active_chat_global->chat_name);
    g_free(active_chat_global->contact_name);
    g_free(active_chat_global);

    active_chat_global = NULL;

    gtk_widget_hide(menu_button_global);

    gtk_label_set_text(GTK_LABEL(chat_name_label_global), "");

    gtk_widget_hide(popover);
    menu_popover_global = NULL;
}

void on_add_members_confirm_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AddMembersData *data = (AddMembersData *)user_data;

    const gchar *members_text = gtk_entry_get_text(GTK_ENTRY(data->members_entry));

    if (members_text && g_strcmp0(members_text, "") != 0) {
        gchar **members_array = g_strsplit(members_text, ",", -1);
        for (int i = 0; members_array[i] != NULL; i++) {
            gchar *member = g_strstrip(members_array[i]); 
            data->chat->members = g_list_append(data->chat->members, g_strdup(member));
        }
        g_strfreev(members_array);
    }

    gtk_widget_destroy(data->window);

    g_free(data);
}

void on_add_members_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkWidget *popover = GTK_WIDGET(user_data);
    gtk_widget_hide(popover); 

    GtkWidget *add_members_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(add_members_window), "Add Members");
    gtk_window_set_default_size(GTK_WINDOW(add_members_window), 300, 100);
    gtk_window_set_resizable(GTK_WINDOW(add_members_window), FALSE);
    gtk_widget_set_name(add_members_window, "add_members_window");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(add_members_window), vbox);

    GtkWidget *members_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(members_entry), "Add members (comma separated)...");
    gtk_widget_set_name(members_entry, "members_entry");
    gtk_box_pack_start(GTK_BOX(vbox), members_entry, FALSE, FALSE, 5);

    GtkWidget *confirm_button = gtk_button_new_with_label("Confirm");
    gtk_widget_set_name(confirm_button, "confirm_button");
    gtk_box_pack_start(GTK_BOX(vbox), confirm_button, FALSE, FALSE, 5);

    AddMembersData *data = g_new0(AddMembersData, 1);
    data->members_entry = members_entry;
    data->window = add_members_window;
    data->chat = active_chat_global;

    g_signal_connect(confirm_button, "clicked", G_CALLBACK(on_add_members_confirm_clicked), data);

    gtk_widget_show_all(add_members_window);
}

void on_menu_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;

    if (active_chat_global == NULL) {
        return;
    }

    if (menu_popover_global != NULL) {
        gtk_widget_destroy(menu_popover_global);
        menu_popover_global = NULL;
    }

    GtkWidget *popover = gtk_popover_new(GTK_WIDGET(button));
    gtk_widget_set_name(popover, "menu_popover");

    menu_popover_global = popover;

    GtkWidget *popover_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *delete_chat_button = gtk_button_new_with_label("Delete chat");
    gtk_widget_set_name(delete_chat_button, "delete_chat_button");

    g_signal_connect(delete_chat_button, "clicked", G_CALLBACK(on_delete_chat_clicked), popover);

    gtk_box_pack_start(GTK_BOX(popover_box), delete_chat_button, FALSE, FALSE, 3);

    if (active_chat_global->is_group) {
        GtkWidget *add_members_button = gtk_button_new_with_label("Add Members");
        gtk_widget_set_name(add_members_button, "add_members_button");

        g_signal_connect(add_members_button, "clicked", G_CALLBACK(on_add_members_clicked), popover);

        gtk_box_pack_start(GTK_BOX(popover_box), add_members_button, FALSE, FALSE, 3);
    }

    gtk_container_add(GTK_CONTAINER(popover), popover_box);

    gtk_widget_show_all(popover);
    gtk_popover_popup(GTK_POPOVER(popover));
}

void show_user_not_found_popup(GtkWindow *parent, const gchar *username) {

    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Error");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

    GtkWidget *error_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(error_window, "auth_dialog");
    gtk_window_set_titlebar(GTK_WINDOW(error_window), header_bar);
    gtk_window_set_transient_for(GTK_WINDOW(error_window), parent);
    gtk_window_set_resizable(GTK_WINDOW(error_window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(error_window), 10);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(error_window), vbox);

    gchar *message = g_strdup_printf("User with nickname '%s' does not found.", username);
    GtkWidget *label = gtk_label_new(message);
    gtk_widget_set_name(label, "label");
    g_free(message);

    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    GtkWidget *close_button = gtk_button_new_with_label("close_button");
    gtk_widget_set_name(close_button, "close_button");
    gtk_box_pack_start(GTK_BOX(vbox), close_button, FALSE, FALSE, 0);

    g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_widget_destroy), error_window);

    gtk_widget_show_all(error_window);
}

void on_search_entry_activate(GtkEntry *entry, gpointer user_data) {
    (void)user_data; 

    const gchar *search_text = gtk_entry_get_text(entry);

    if (g_strcmp0(search_text, "") == 0) {
        return; 
    }

    if (search_text[0] == '_') {
        show_user_not_found_popup(GTK_WINDOW(main_window), search_text);
        gtk_entry_set_text(entry, "");
        return;
    }

    gboolean chat_found = FALSE;

    GList *iter = chat_list;
    while (iter != NULL) {
        Chat *chat = (Chat *)iter->data;

        if (g_ascii_strcasecmp(chat->chat_name, search_text) == 0) {
            chat_found = TRUE;

            g_signal_handler_block(chat->toggle_button, chat->handler_id);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chat->toggle_button), TRUE);
            g_signal_handler_unblock(chat->toggle_button, chat->handler_id);

            change_contact(GTK_TOGGLE_BUTTON(chat->toggle_button), chat);
        } else {
            g_signal_handler_block(chat->toggle_button, chat->handler_id);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chat->toggle_button), FALSE);
            g_signal_handler_unblock(chat->toggle_button, chat->handler_id);

            gtk_widget_hide(chat->chat_area);
        }
        iter = iter->next;
    }

     if (!chat_found) {
        g_print("Chat '%s' not found.\n", search_text);
    }

    gtk_entry_set_text(entry, "");
}

void create_feature_buttons_box(GtkToggleButton *features_button, gpointer side_panel_ptr) {
    (void)side_panel_ptr; 

    gboolean active = gtk_toggle_button_get_active(features_button);
    if (active) {
        gtk_widget_show_all(feature_buttons_box);
    } else {
        gtk_widget_hide(feature_buttons_box);
    }
}

void initialize_client_buttons(GtkWidget *vbox) {
    apply_light_theme(); 
    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(top_bar, "top_bar");
    top_bar_global = top_bar;

    GtkWidget *search_entry = gtk_entry_new();
    gtk_widget_set_name(search_entry, "search_entry");  
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search");
    gtk_widget_set_size_request(search_entry, 180, 30);
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(search_entry), GTK_ENTRY_ICON_PRIMARY, "system-search-symbolic");
    g_signal_connect(search_entry, "activate", G_CALLBACK(on_search_entry_activate), NULL);

    GtkWidget *menu_button = gtk_button_new();
    gtk_widget_set_name(menu_button, "menu_button");
    GtkWidget *menu_icon = gtk_image_new_from_icon_name("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(menu_button), menu_icon);
    gtk_button_set_relief(GTK_BUTTON(menu_button), GTK_RELIEF_NONE);
    gtk_widget_set_size_request(menu_button, 40, 40);
    gtk_widget_set_no_show_all(menu_button, TRUE);
    gtk_widget_hide(menu_button);
    menu_button_global = menu_button;
    g_signal_connect(menu_button, "clicked", G_CALLBACK(on_menu_button_clicked), NULL);

    GtkWidget *profile_button = gtk_button_new();
    gtk_widget_set_name(profile_button, "profile_button");
    GtkWidget *profile_icon = gtk_image_new_from_icon_name("avatar-default-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(profile_button), profile_icon);
    gtk_button_set_relief(GTK_BUTTON(profile_button), GTK_RELIEF_NONE);
    gtk_widget_set_size_request(profile_button, 40, 40);
    g_signal_connect(profile_button, "clicked", G_CALLBACK(open_profile_window), NULL);

    GtkWidget *chat_name_label = gtk_label_new("");
    gtk_widget_set_name(chat_name_label, "chat_name_label");
    chat_name_label_global = chat_name_label;

    gtk_box_pack_start(GTK_BOX(top_bar), search_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(top_bar), profile_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top_bar), chat_name_label, TRUE, TRUE, 5);
    gtk_box_pack_end(GTK_BOX(top_bar), menu_button, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), top_bar, FALSE, FALSE, 0);

    GtkWidget *side_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(side_panel, "side_panel");
    side_panel_global = side_panel;

    GtkWidget *side_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(side_scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(side_scrolled_window, 250, -1);
    gtk_widget_set_hexpand(side_scrolled_window, FALSE);
    gtk_widget_set_vexpand(side_scrolled_window, TRUE);
    gtk_container_add(GTK_CONTAINER(side_scrolled_window), side_panel);

    GtkWidget *overlay = gtk_overlay_new();
    overlay_global = overlay; 
    gtk_container_add(GTK_CONTAINER(overlay), side_scrolled_window);

    if (feature_buttons_box == NULL) {
        feature_buttons_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_name(feature_buttons_box, "feature_buttons_box");

        GtkWidget *change_theme_button = gtk_toggle_button_new_with_label("change theme");
        GtkWidget *create_group_button = gtk_button_new_with_label("create group");
        GtkWidget *create_chat_button = gtk_button_new_with_label("create chat");

        gtk_box_pack_start(GTK_BOX(feature_buttons_box), change_theme_button, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(feature_buttons_box), create_group_button, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(feature_buttons_box), create_chat_button, FALSE, FALSE, 5);

        gtk_overlay_add_overlay(GTK_OVERLAY(overlay), feature_buttons_box);
        gtk_widget_set_halign(feature_buttons_box, GTK_ALIGN_END);
        gtk_widget_set_valign(feature_buttons_box, GTK_ALIGN_END);

        g_signal_connect(create_chat_button, "clicked", G_CALLBACK(hide_and_create_contact_box), NULL);
        g_signal_connect(create_group_button, "clicked", G_CALLBACK(hide_and_create_group_box), NULL);
        g_signal_connect(change_theme_button, "toggled", G_CALLBACK(on_change_theme_toggled), NULL);
    }

    GtkWidget *features_button = gtk_toggle_button_new();
    gtk_widget_set_name(features_button, "features_button");
    GtkWidget *symbolic_plus = gtk_image_new_from_icon_name("list-add-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(features_button), symbolic_plus);
    gtk_button_set_relief(GTK_BUTTON(features_button), GTK_RELIEF_NONE);
    gtk_widget_set_size_request(features_button, 70, 70);
    g_signal_connect(features_button, "toggled", G_CALLBACK(create_feature_buttons_box), side_scrolled_window);
    g_signal_connect(features_button, "clicked", G_CALLBACK(hide_center_label), NULL);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), features_button);
    gtk_widget_set_halign(features_button, GTK_ALIGN_END);
    gtk_widget_set_valign(features_button, GTK_ALIGN_END);

    GtkWidget *main_content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    main_content_global = main_content;

    GtkWidget *chat_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    chat_container_global = chat_container;
    gtk_widget_set_name(chat_container, "chat_container");
    gtk_widget_set_hexpand(chat_container_global, TRUE);
    gtk_widget_set_vexpand(chat_container_global, TRUE);

    GtkWidget *center_label = gtk_label_new("Choose who to write to");
    gtk_widget_set_name(center_label, "center_label");
    gtk_widget_set_halign(center_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_label, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(chat_container), center_label, TRUE, TRUE, 0);
    center_label_global = center_label;

    gtk_box_pack_start(GTK_BOX(main_content), overlay, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_content), chat_container, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), main_content, TRUE, TRUE, 0);
}
