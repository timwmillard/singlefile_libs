#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <AppKit/AppKit.h>
#elif __EMSCRIPTEN__
#else  // Linux
#include <gtk/gtk.h>
#endif

// Structure to hold file dialog results
typedef struct {
    char* path;
    int success;
} FileDialogResult;

// Free the result when done
void free_file_dialog_result(FileDialogResult* result) {
    if (result->path) {
        free(result->path);
        result->path = NULL;
    }
}

#ifdef _WIN32
// Windows implementation
FileDialogResult open_file_dialog(const char* title, const char* filter) {
    FileDialogResult result = {NULL, 0};
    OPENFILENAMEA ofn;
    char* filename = malloc(MAX_PATH);
    
    ZeroMemory(&ofn, sizeof(ofn));
    ZeroMemory(filename, MAX_PATH);
    
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter ? filter : "All Files\0*.*\0";
    ofn.lpstrTitle = title;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn)) {
        result.path = filename;
        result.success = 1;
    } else {
        free(filename);
    }
    
    return result;
}

#elif __APPLE__
// macOS implementation
FileDialogResult open_file_dialog(const char* title, const char* filter) {
    FileDialogResult result = {NULL, 0};
    
    @autoreleasepool {
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        panel.title = title ? [NSString stringWithUTF8String:title] : @"Open File";
        panel.canChooseFiles = YES;
        panel.canChooseDirectories = NO;
        panel.allowsMultipleSelection = NO;
        
        if ([panel runModal] == NSModalResponseOK) {
            NSURL *url = panel.URLs[0];
            const char* path = [[url path] UTF8String];
            result.path = strdup(path);
            result.success = 1;
        }
    }
    
    return result;
}

#elif __EMSCRIPTEN__
// Web implmentation
FileDialogResult open_file_dialog(const char* title, const char* filter) {
    FileDialogResult result = {NULL, 0};
    // TODO: web implementation
    return result;
}

#else
// GTK (Linux) implementation
FileDialogResult open_file_dialog(const char* title, const char* filter) {
    FileDialogResult result = {NULL, 0};

    // Initialize GTK if needed
    if (!gtk_init_check(NULL, NULL)) {
        return result;
    }

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        title ? title : "Open File",
        NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL
    );

    if (filter) {
        GtkFileFilter *file_filter = gtk_file_filter_new();
        gtk_file_filter_set_name(file_filter, filter);
        gtk_file_filter_add_pattern(file_filter, filter);
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), file_filter);
    }

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        result.path = strdup(filename);
        result.success = 1;
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
    while (gtk_events_pending()) gtk_main_iteration();

    return result;
}
#endif

