// Longer explanations of everything here: notes.md
// Build and run: gcc ui.c -o ui $(pkg-config --cflags --libs gtk4) && ./ui
#include <gtk/gtk.h> // pastes in GTK's declarations before compiling

// Callback: registered in main, called later by GLib when the app emits "activate".
// static = only visible inside this file.
static void
activate(GtkApplication *app,
         gpointer user_data) // gpointer = void *; receives the last argument of g_signal_connect
{
    GtkWidget *window;

    window = gtk_application_window_new(app);           // the app stays alive while this window is open
    gtk_window_set_title(GTK_WINDOW(window), "Window"); // GTK_WINDOW = checked cast from GtkWidget * to GtkWindow *
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    gtk_window_present(GTK_WINDOW(window)); // show it on screen
}

int main(int argc,
         char **argv) // command-line arguments: how many, and the list of strings
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS); // string is the application ID
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);             // on "activate", call activate(app, NULL)
    status = g_application_run(G_APPLICATION(app), argc, argv);                // emits startup then activate, runs the main loop, returns when the last window closes
    g_object_unref(app);                                                       // drop our reference; app is freed when the count hits zero

    return status; // exit code for the OS
}
