#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <gtk/gtk.h>
#include <stdlib.h>


#define LOGIN_LABEL "Login"
#define REGISTRATION_LABEL "Registration"
#define REGISTER_LABEL "Register"
#define USERNAME_PLACEHOLDER "User name"
#define PASSWORD_PLACEHOLDER "Password"
#define CONFIRM_PASSWORD_PLACEHOLDER "Confirm password"
#define EXIT_BUTTON_LABEL "Exit"
#define LOG_IN_BUTTON_LABEL "Log in"

#define USAGE_MESSAGE "usage: ./uchat [ip] [port]\n"

extern gchar *current_username;
extern gchar *current_password;

typedef struct {
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *confirm_password_entry;
    GtkWidget *error_label; 
    GtkWidget *window;
} RegisterData;

typedef struct {
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *error_label; 
    GtkWidget *window;
} LoginData;


void authentication_window(void);
void initialize_auth_buttons(GtkWidget *vbox, GtkWidget *window);
void apply_css(void);

bool go_to_main_client(GtkWindow *window);
bool show_login_error(void);

void on_login(GtkWidget *container3, GtkWidget *window);
void on_register(GtkWidget *container2, GtkWidget *window);
void on_login_button_toggled(GtkToggleButton *toggle_button, GtkWidget *window);
void on_register_button_toggled(GtkToggleButton *toggle_button,  GtkWidget *window);
void on_register_button_clicked(GtkButton *button, gpointer user_data);
void on_login_button_clicked(GtkButton *button, gpointer user_data);

#endif 
