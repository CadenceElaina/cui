#include <gtk/gtk.h> // preprocessor literally pastes the contents of that header file into the code before compiling. Header only contains declarations (fn names, types, macros)

static void // void the fn returns nothing, static on a function, means that it is only visible inside this .c file, activate is the fn name- its a callback meaning its never called by myself. GTK calls it with g_signal_connect.
activate(GtkApplication *app,
         gpointer user_data) // GtkApplication *app = a pointer to the application object GTK passes into this function. gpointer = GLib's name for void *, a pointer to anything. Whatever is passed as the last argument of g_signal_connect shows up here.
{
    GtkWidget *window; // a pointer holding the address of a window object GTK creates in memory.

    window = gtk_application_window_new(app);           // creates the window tying it to app - the app stays alive as long as it has the window open.
    gtk_window_set_title(GTK_WINDOW(window), "Window"); // cast macro. C has no classes or inheritance. GTK mimics it with a trick: each "child" struct starts with its "parent" struct as its very first member.
    // typedef struct ParentStruct{...} parent_t;
    // typedef struct ChildStruct { struct parent_t;? ...}
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    gtk_window_present(GTK_WINDOW(window)); // shows the window on screen
}

int main(int argc,
         char **argv) // command line arguments, words typed after the program name in the terminal - not interactive like scanf. argc = argument count, argv = argument vector, an array of strings whereas char * is one string (array of characters). So char ** is a pointer to a list of strings
{
    GtkApplication *app; // pointer to the app created
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS); // app is C variable name. String is application ID
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);             // wiring. When app emits the activate signal call the activate function and pass in NULL as user_data. G_CALLBACK casts the function to a generic function-pointer type.
    status = g_application_run(G_APPLICATION(app), argc, argv);                // Where the program lives. 1. starts up the app and emits activate triggering the fn and opening the window. 2. enters the main loop, waiting for events. 3. blocks here until the last window closes.. Returns an exit code typically 0. Howd the app end?
    g_object_unref(app);                                                       // Reference counting in GObjects. tracks number of owners. unref subtracts one. When count reaches zero the object frees itself. app still holds the old address afterward - now pointing to free memory.

    return status; // number handed back to OS as the exit code.
}

// FIX BELOW: gcc $(pkg-config --cflags gtk4) -o ui ui.c $(pkg-config --libs gtk4)
/*
 #include <gtk/gtk.h> makes gcc look for gtk/gtk.h in its default include path (the
  list of folders it searches for headers, mainly /usr/include). Fedora puts GTK4 in
  /usr/include/gtk-4.0/gtk/gtk.h, one folder deeper than gcc looks. GTK also depends on GLib, Pango,
  Cairo and others, and each of those has its own include folder and its own library that has to be
  linked.
*/