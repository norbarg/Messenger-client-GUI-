#include "main.h"
#include "authentication_gui.h"
#include "gui_active_elements.h"
#include <libwebsockets.h>
#include "client.h"
#include "gui_active_elements.h"
#include <json-c/json.h>

gchar *current_username = NULL;
gchar *current_password = NULL;
LoginData* loginData = NULL;

void authentication_window(void) {

    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Authentication");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(window,"window");
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE); 

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    initialize_auth_buttons(vbox, window);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);

    gtk_main();
}

static GtkCssProvider *provider;

bool go_to_main_client(GtkWindow *window){
    gtk_window_close(window);
    gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider));
    general_client();
    gtk_main();
    return false;
}
bool show_login_error(void){
    gtk_label_set_text(GTK_LABEL(loginData->error_label), "Incorrect username or password.");
    gtk_widget_show(loginData->error_label);
    return false;
}

void apply_css(void) {
    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

void initialize_auth_buttons(GtkWidget *vbox, GtkWidget *window) {
    apply_css();
    GtkWidget *toggle_buttons_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *input_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *action_buttons_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    gtk_box_pack_start(GTK_BOX(vbox), toggle_buttons_container, FALSE, FALSE, 10);

    GtkWidget *registration_button = gtk_toggle_button_new_with_label("Register");
    GtkWidget *login_button = gtk_toggle_button_new_with_label("Login");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(registration_button), TRUE);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(hbox), registration_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), login_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(toggle_buttons_container), hbox, FALSE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(vbox), input_container, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), action_buttons_container, FALSE, FALSE, 10);

    gtk_widget_show(input_container);
    gtk_widget_hide(action_buttons_container);

    g_object_set_data(G_OBJECT(registration_button), "input_container", input_container);
    g_object_set_data(G_OBJECT(registration_button), "action_buttons_container", action_buttons_container);
    g_object_set_data(G_OBJECT(registration_button), "login_button", login_button);
    g_object_set_data(G_OBJECT(registration_button), "window", window);

    g_object_set_data(G_OBJECT(login_button), "input_container", input_container);
    g_object_set_data(G_OBJECT(login_button), "action_buttons_container", action_buttons_container);
    g_object_set_data(G_OBJECT(login_button), "registration_button", registration_button);
    g_object_set_data(G_OBJECT(login_button), "window", window);

    g_signal_connect(registration_button, "toggled", G_CALLBACK(on_register_button_toggled), window);
    g_signal_connect(login_button, "toggled", G_CALLBACK(on_login_button_toggled), window);

    on_register(input_container, window);
}


void on_register_button_toggled(GtkToggleButton *toggle_button, GtkWidget *window) {
    if (gtk_toggle_button_get_active(toggle_button)) {
        GtkWidget *input_container = g_object_get_data(G_OBJECT(toggle_button), "input_container");
        GtkWidget *action_buttons_container = g_object_get_data(G_OBJECT(toggle_button), "action_buttons_container");
        GtkWidget *login_button = g_object_get_data(G_OBJECT(toggle_button), "login_button");

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(login_button), FALSE);
        gtk_widget_show(input_container);
        gtk_widget_hide(action_buttons_container);

        on_register(input_container, window);
    }
}

void on_login_button_toggled(GtkToggleButton *toggle_button, GtkWidget *window) {
    if (gtk_toggle_button_get_active(toggle_button)) {
        GtkWidget *input_container = g_object_get_data(G_OBJECT(toggle_button), "input_container");
        GtkWidget *action_buttons_container = g_object_get_data(G_OBJECT(toggle_button), "action_buttons_container");
        GtkWidget *registration_button = g_object_get_data(G_OBJECT(toggle_button), "registration_button");

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(registration_button), FALSE);
        gtk_widget_hide(input_container);
        gtk_widget_show(action_buttons_container);

        on_login(action_buttons_container, window);
    }
}

void on_register(GtkWidget *input_container, GtkWidget *window) {
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(input_container));
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Username");

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    GtkWidget *confirm_password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(confirm_password_entry), "Confirm Password");
    gtk_entry_set_visibility(GTK_ENTRY(confirm_password_entry), FALSE);

    GtkWidget *error_label = gtk_label_new("");
    gtk_widget_set_name(error_label, "error_label");
    gtk_widget_set_halign(error_label, GTK_ALIGN_CENTER);
    gtk_widget_set_no_show_all(error_label, TRUE);



    GtkWidget *confirm_button = gtk_button_new_with_label("Register");
    GtkWidget *exit_button = gtk_button_new_with_label("Exit");

    gtk_box_pack_start(GTK_BOX(input_container), username_entry, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(input_container), password_entry, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(input_container), confirm_password_entry, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(input_container), error_label, FALSE, FALSE, 10); 

    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(action_box), exit_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(action_box), confirm_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(input_container), action_box, FALSE, FALSE, 10);

    RegisterData *reg_data = g_new(RegisterData, 1);
    reg_data->username_entry = username_entry;
    reg_data->password_entry = password_entry;
    reg_data->confirm_password_entry = confirm_password_entry;
    reg_data->error_label = error_label; 
    reg_data->window = window;

    g_signal_connect(exit_button, "clicked", gtk_main_quit, NULL);
    g_signal_connect(confirm_button, "clicked", G_CALLBACK(on_register_button_clicked), reg_data);

    gtk_widget_show_all(input_container);
}

void on_register_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    RegisterData *data = (RegisterData *)user_data;

    const gchar *username = gtk_entry_get_text(GTK_ENTRY(data->username_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(data->password_entry));
    const gchar *confirm_password = gtk_entry_get_text(GTK_ENTRY(data->confirm_password_entry));

    if (g_strcmp0(password, confirm_password) != 0) {
        gtk_label_set_text(GTK_LABEL(data->error_label), "Passwords do not match!");
        gtk_widget_show(data->error_label);
    } 
    if (strlen(username) == 0) {
        gtk_label_set_text(GTK_LABEL(data->error_label), "Enter username");
        gtk_widget_show(data->error_label);
    } else {
        gtk_widget_hide(data->error_label);

        current_username = g_strdup(username);
        current_password = g_strdup(password);

        g_free(data);

        go_to_main_client(GTK_WINDOW(data->window));
    }
}

void on_login(GtkWidget *action_buttons_container, GtkWidget *window) {

    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(action_buttons_container));
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Username");

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    GtkWidget *error_label = gtk_label_new("");
    gtk_widget_set_name(error_label, "error_label");
    gtk_widget_set_halign(error_label, GTK_ALIGN_CENTER);
    gtk_widget_set_no_show_all(error_label, TRUE);


    GtkWidget *log_in_button = gtk_button_new_with_label("Log in");
    GtkWidget *exit_button = gtk_button_new_with_label("Exit");

    gtk_box_pack_start(GTK_BOX(action_buttons_container), username_entry, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(action_buttons_container), password_entry, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(action_buttons_container), error_label, FALSE, FALSE, 10); 

    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(action_box), exit_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(action_box), log_in_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(action_buttons_container), action_box, FALSE, FALSE, 10);

    LoginData *login_data = g_new(LoginData, 1);
    login_data->username_entry = username_entry;
    login_data->password_entry = password_entry;
    login_data->error_label = error_label; 
    login_data->window = window;


    g_signal_connect(exit_button, "clicked", gtk_main_quit, NULL);
    g_signal_connect(log_in_button, "clicked", G_CALLBACK(on_login_button_clicked), login_data);

    gtk_widget_show_all(action_buttons_container);
}

void on_login_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    LoginData *data = (LoginData *)user_data;

    const gchar *username = gtk_entry_get_text(GTK_ENTRY(data->username_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(data->password_entry));
    loginData = data;

    current_username = g_strdup(username);
    current_password = g_strdup(password);
    struct json_object* messaje_obj = json_object_new_object();
    json_object_object_add(messaje_obj, "reason", json_object_new_string("AUTH"));
    json_object_object_add(messaje_obj, "login", json_object_new_string(current_username));
    json_object_object_add(messaje_obj, "password", json_object_new_string(current_password));
    const char* message_json = json_object_to_json_string(messaje_obj);
    
    char* message_json_dup = strdup(message_json);
    g_print("%lld",(long long)global_wsi);
    lws_easy_write(global_wsi, message_json_dup, strlen(message_json_dup));

    if(message_json_dup!=0)
        free(message_json_dup);

    json_object_put(messaje_obj);




    
}
