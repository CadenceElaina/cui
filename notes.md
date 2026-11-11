# GTK4 notes

Created: 2026-11-11 11:42
Last modified: 2026-11-11 11:48

## Build and run

```sh
gcc ui.c -o ui $(pkg-config --cflags --libs gtk4) && ./ui
```

Swap `ui.c` and `ui` for another program's file and output names.

Why `pkg-config` is needed: `#include <gtk/gtk.h>` makes gcc search its default include path (the list of folders it checks for headers, mainly `/usr/include`). Fedora puts GTK4 at `/usr/include/gtk-4.0/gtk/gtk.h`, one folder deeper than gcc looks. GTK also depends on GLib, Pango, Cairo and others, and each has its own include folder and its own library that has to be linked. `pkg-config` prints all of those flags for you:

- `--cflags`: `-I` flags, extra folders to search for headers (used while compiling)
- `--libs`: `-l` flags, the libraries to link (used while linking)

The library flags go after `ui.c`. The linker resolves symbols (function and variable names) left to right: it only pulls from a library the symbols that earlier files have already asked for. With the libraries first, nothing has asked yet, so the build can fail with `undefined reference to 'gtk_...'`.

## What GTK is

GTK ([getting started guide](https://docs.gtk.org/gtk4/getting_started.html)) is a widget toolkit: every interface is built out of widgets (windows, buttons, labels). For complex interfaces with dozens or hundreds of widgets, GtkBuilder loads the layout from a file instead of creating each widget in code.

It is event driven: the main loop waits for events (a click, a key press, a window closing) and passes each one to the function the application registered for it.

```
user clicks a button --> main loop --> handler function in the app
```

A "hold" is anything that keeps the app running. Each open window counts as one, and code can add more with `g_application_hold`. `g_application_run` returns once there are none left, which for ui.c means once the last window closes.

## Order of execution in ui.c

```
main starts
  -> gtk_application_new     creates the app object
  -> g_signal_connect        registers activate() as the handler for the "activate" signal
  -> g_application_run       starts the app
       -> "startup" signal   one-time setup (ui.c has no handler for it)
       -> "activate" signal  GLib calls activate(), which builds and shows the window
       -> main loop          waits for events until nothing holds the app open
     <- returns exit status
  -> g_object_unref          releases the app object
  -> return status           exit code handed to the OS
```

## #include

The preprocessor (the step that runs before compiling) pastes the entire header file into the source at that line. A header only holds declarations: function names, types, macros. The compiled GTK code itself lives in the libraries that `--libs` links in.

## static functions and callbacks

`static` on a function means it is only visible inside its own `.c` file.

`activate` is a callback: a function you never call yourself. `g_signal_connect` only registers it, handing GLib a function pointer (the function's address). GLib's signal system calls it later, when the app emits the "activate" signal inside `g_application_run`. Handing over a function now so someone else can call it later is the core of event-driven programming. `G_CALLBACK(activate)` casts it to GTK's generic function-pointer type, so `g_signal_connect` can accept handlers with any signature.

`gpointer` is GLib's name for `void *`, a pointer to anything. Whatever is passed as the last argument of `g_signal_connect` arrives in the callback as `user_data`. ui.c passes `NULL`, so `user_data` is unused.

## Command-line arguments: argc and argv

`int main(int argc, char **argv)` receives the words typed after the program name in the terminal. This is not interactive input like `scanf`.

- `argc`: argument count.
- `argv`: argument vector, an array of strings. A `char *` is a pointer to the first character of a string, which is why it behaves like an array of characters. A `char **` points to the first of a list of those pointers.
- `argv[0]` is the program's own name, as typed to launch it.
- `argv[argc]` is always `NULL`, so a loop can stop at the `NULL` instead of counting.

Running `./ui hello world`:

| expression | value |
|---|---|
| `argc` | `3` |
| `argv[0]` | `"./ui"` |
| `argv[1]` | `"hello"` |
| `argv[2]` | `"world"` |
| `argv[3]` | `NULL` |

ui.c passes these to `g_application_run`, and GTK treats extra words as files to open. With `G_APPLICATION_DEFAULT_FLAGS` the app can't open files, so `./ui hello world` prints `This application can not open files.` and exits with status 1 without opening a window.

## Cast macros and fake inheritance (GTK_WINDOW)

C has no classes or inheritance. GTK mimics it with a trick: each "child" struct starts with its "parent" struct as its very first member (the struct itself, not a pointer to it).

```c
typedef struct { int x; } Parent;
typedef struct {
    Parent parent_instance;  // first member, not a pointer
    int y;
} Child;
```

Because `parent_instance` sits at offset 0 (zero bytes from the start of `Child`), a `Child *` holds the same address as a pointer to its `Parent`, so casting between them is safe.

The GTK version, simplified:

```c
struct GtkWidget {
    // widget stuff: size, visibility...
};

struct GtkWindow {
    GtkWidget parent; // first member
    // window stuff: title, default size...
};
```

- `gtk_application_window_new` returns a `GtkWidget *` (the most general type).
- `gtk_window_set_title` only accepts a `GtkWindow *`, since titles are a window thing.
- `GTK_WINDOW(window)` relabels the pointer so the types match.

`G_APPLICATION(app)` in `main` is the same kind of macro, going from `GtkApplication *` to its parent type `GApplication *`.

### Why the macro instead of a plain cast

A plain C cast, `(GtkWindow *)window`, compiles no matter what `window` really is, and silently does the wrong thing if it isn't a window. `GTK_WINDOW(window)` expands into a cast plus a runtime check. If the object isn't really a window, GTK prints a warning in the terminal instead of silently doing something weird. Accidentally passing a button gives something like:

```
invalid cast from 'GtkButton' to 'GtkWindow'
```

## Reference counting (g_object_unref)

GTK objects track how many owners they have (the reference count). `g_object_unref` subtracts one, and when the count reaches zero the object frees itself. After that, `app` still holds the old address, now pointing at freed memory (a dangling pointer), so it must not be used again.

`g_clear_object(&app)` does the unref and sets `app` to `NULL` in one step, so a later mistaken use hits `NULL` instead of freed memory. ui.c doesn't need it, because `main` returns on the next line and `app` is never touched again.

## Macros

A macro is a find-and-replace rule that the preprocessor applies before the code is compiled. It is defined with `#define`:

```c
#define PI 3.14159
#define SQUARE(x) ((x) * (x))

double area = PI * SQUARE(r);
// after the preprocessor:
double area = 3.14159 * ((r) * (r));
```

This is the same step as `#include`, which pastes a file's contents in before compiling.

### Scope

A macro works from the line where it is `#define`d down to the end of the file. It ignores braces completely, so defining it inside a function does not confine it to that function:

```c
void foo(void) {
#define LIMIT 10
}

void bar(void) {
    int x = LIMIT; // works, because LIMIT was defined above this line
}
```

Above the `#define`, it doesn't exist. If the macro lives in a header, every file that `#include`s that header gets it. That is how ui.c can use `GTK_WINDOW`: it is defined somewhere inside the files `gtk/gtk.h` pulls in.

### Macro vs function vs variable

| | Macro | Function | Variable |
|---|---|---|---|
| Exists at runtime? | No, just pasted text | Yes, real compiled code | Yes, real memory |
| Has a type? | No | Yes, arguments are checked | Yes |
| Takes memory / has an address? | No | Yes | Yes |
| Can change? | No | n/a | Yes (unless `const`) |
| Respects braces and scope? | No | Yes | Yes |

### When people use each

- Constants: modern C usually prefers `const int LIMIT = 10;` or an `enum`, since those have types and show up in a debugger. Macros are still common for this, especially in older code and headers.
- Small reusable logic: a regular function, or `static inline` if speed matters (`inline` asks the compiler to paste the function body at each call site). You get type checking and no double evaluation. Double evaluation is the macro trap where an argument runs twice: `SQUARE(i++)` becomes `((i++) * (i++))`, which changes `i` twice in one expression. C calls that undefined behavior (the standard makes no promise about the result).
- Macros: for things functions can't do, like GTK's casting checks, generating code, or `#ifdef` to compile different code on different systems.
