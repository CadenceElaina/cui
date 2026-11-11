GTK https://docs.gtk.org/gtk4/getting_started.html
a widget toolkit - each interface consists of widgets
GtkBuilder for complex user interfaces (dozens / hundreds of widgets)

event driven:
listens for button click / event --- passes to ---> application

## Run the gtk ui (replace ui.c , ui, ./ui with name of program)

gcc ui.c -o ui $(pkg-config --cflags --libs gtk4) && ./ui

int main(int argc, char \*\*argv)

These are command-line arguments, the words typed after the program name in the terminal. Not interactive input like scanf.

argc: argument count.
argv: argument vector, an array of strings. char \* is one string, so char \*\* is a pointer to a list of strings.

If you ran ./ui hello world:

    value

argc 3
argv[0] "./ui"
argv[1] "hello"
argv[2] "world"

Order of execution
main starts.
The app object is created.
activate is registered as the handler for the activate signal.
g_application_run starts the app, which fires activate.
activate builds and shows the window.
The main loop runs until you close the window.
g_application_run returns the exit status.
The app object is released.
main returns the status to the OS.

macro is a find and replace rule that runs before the code is compiled. its defined with #define:
#define PI 3.14159
#define SQUARE(x) ((x) \*(x))

double area = PI \* SQUARE(r); ---preprocessor---> double area = 3.14159 \* ((r) \_ (r));
similar to how #include gets the code and prints it before the code runs.

window open.
gtk_window_set_title(GTK_WINDOW(window), "Window"); // cast macro. C has no classes or inheritance. GTK mimics it with a trick: each "child" struct starts with its "parent" struct as its very first member.
simplified:
struct GtkWidget {
// widget stuff: size, visibility...
};

struct GtkWindow {
GtkWidget parent; // first member!
// window stuff: title, default size...
};

gtk*application_window_new returns a GtkWidget * (the most general type).
gtk*window_set_title only accepts a GtkWindow *, since titles are a window thing.
GTK_WINDOW(window) relabels the pointer so the types match.
Why use the macro instead of a plain cast

GTK_WINDOW(window) expands into a cast plus a runtime check. If the object isn’t really a window, GTK prints a warning in your terminal instead of silently doing something weird. For example, if you accidentally passed a button, you’d see something like:

invalid cast from 'GtkButton' to 'GtkWindow'

Scope, corrected slightly

A macro works from the line where it’s #defined downward, to the end of the file. It completely ignores braces, so defining it inside a function doesn’t confine it to that function:

c
void foo(void) {
#define LIMIT 10
}

void bar(void) {
int x = LIMIT; // works, because LIMIT was defined above this line
}

Above the #define, it doesn’t exist. And if the macro lives in a header, every file that #includes that header gets it. That’s how your ui.c can use GTK_WINDOW: it’s defined somewhere inside the files gtk/gtk.h pulls in.

The real differences
Macro Function Variable
Exists at runtime? No, just pasted text Yes, real compiled code Yes, real memory
Has a type? No Yes, args are checked Yes
Takes memory / has an address? No Yes Yes
Can change? No n/a Yes (unless const)
Respects braces/scope? No Yes Yes

When people use each
Constants: modern C usually prefers const int LIMIT = 10; or an enum, since those have types and show up in a debugger. Macros are still common for this, especially in older code and headers.
Small reusable logic: a regular function, or static inline if speed matters. You get type checking and no double-evaluation surprises.
Macros: for things functions can’t do, like GTK’s casting tricks, generating code, or #ifdef to compile different code on different systems.
