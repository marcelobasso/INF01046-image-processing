#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <gtk/gtk.h>

// Declare the function to open an image
void open_image(GtkWidget *button, GtkImage *image);

// Declare the function to save an image
void save_image(GtkWidget *button, GtkImage *image);

// Declare the function to handle the tones button click
void on_quantize_button_clicked(GtkWidget *button, GtkEntry *entry);

void on_copy_clicked(GtkWidget *button, GtkImage *image);

// Variable to store the loaded image filename (for saving later)
char *current_image_filename = NULL;
GtkWidget *image, *cp_image;

// Function to create the control window with "Open Image", "Save Image" buttons, and "Tones" input field
void create_control_window(GtkImage *image, GtkImage *cp_image) {
    // Create a new window for controls
    GtkWidget *control_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(control_window), "Control Window");
    gtk_window_set_default_size(GTK_WINDOW(control_window), 200, 100);
    gtk_window_set_position(GTK_WINDOW(control_window), GTK_WIN_POS_CENTER);

    // Create a vertical box container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(control_window), vbox);

    // Open Image
    GtkWidget *open_button = gtk_button_new_with_label("Open Image");
    gtk_box_pack_start(GTK_BOX(vbox), open_button, TRUE, TRUE, 3);

    // save the image
    GtkWidget *save_button = gtk_button_new_with_label("Save Image");
    gtk_box_pack_start(GTK_BOX(vbox), save_button, TRUE, TRUE, 0);

    // Brings image to the editting window
    GtkWidget *copy = gtk_button_new_with_label("Copy Image");
    gtk_box_pack_start(GTK_BOX(vbox), copy, TRUE, TRUE, 0);

    // Mirror vertical
    GtkWidget *vertical_mirror = gtk_button_new_with_label("Vertical mirror");
    gtk_box_pack_start(GTK_BOX(vbox), vertical_mirror, TRUE, TRUE, 0);

    // Mirror horizontal
    GtkWidget *horizontal_mirror = gtk_button_new_with_label("Horizontal mirror");
    gtk_box_pack_start(GTK_BOX(vbox), horizontal_mirror, TRUE, TRUE, 0);

    GtkWidget *grayscale = gtk_button_new_with_label("Grayscale");
    gtk_box_pack_start(GTK_BOX(vbox), grayscale, TRUE, TRUE, 0);

    // Create a horizontal box container for tones input and button
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    // Create a text field (entry) for quantization
    GtkWidget *quantize = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(quantize), "N° of tones");
    gtk_box_pack_start(GTK_BOX(hbox), quantize, TRUE, TRUE, 0);

    // Create a button for submitting tones
    GtkWidget *quantize_button = gtk_button_new_with_label("Quantize");
    gtk_box_pack_start(GTK_BOX(hbox), quantize_button, FALSE, FALSE, 0);

    // Connect the buttons to their respective functions
    g_signal_connect(open_button, "clicked", G_CALLBACK(open_image), image);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_image), image);
    g_signal_connect(quantize_button, "clicked", G_CALLBACK(on_quantize_button_clicked), GTK_ENTRY(quantize));
    g_signal_connect(copy, "clicked", G_CALLBACK(on_copy_clicked), image);


    // Connect the destroy signal to close the application
    g_signal_connect(control_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets in the control window
    gtk_widget_show_all(control_window);
}

// Function to open an image using file chooser dialog
void open_image(GtkWidget *button, GtkImage *image) {
    GtkWidget *dialog;

    // Create a file chooser dialog
    dialog = gtk_file_chooser_dialog_new("Open Image",
        NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        ("_Cancel"), GTK_RESPONSE_CANCEL,
        ("_Open"), GTK_RESPONSE_ACCEPT,
        NULL);

    // Set filters for image files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pixbuf_formats(filter);  // Allows image file formats
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Check if the user selected a file
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        // Free the previously loaded filename if it exists
        if (current_image_filename) {
            g_free(current_image_filename);
        }

        // Get the selected filename
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        current_image_filename = gtk_file_chooser_get_filename(chooser);

        // Set the image to the selected file
        gtk_image_set_from_file(image, current_image_filename);
    }

    // Close the dialog after selection
    gtk_widget_destroy(dialog);
}

// Function to save the image as JPEG using a file chooser dialog
void save_image(GtkWidget *button, GtkImage *image) {
    // Check if there is an image to save
    if (!current_image_filename) {
        std::cerr << "No image loaded to save!" << std::endl;
        return;
    }

    // Get the pixbuf from the image widget
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(image);
    if (!pixbuf) {
        std::cerr << "No image to save!" << std::endl;
        return;
    }

    // Create a file chooser dialog for saving the file
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Save Image",
        NULL,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        ("_Cancel"), GTK_RESPONSE_CANCEL,
        ("_Save"), GTK_RESPONSE_ACCEPT,
        NULL);

    // Set a default filename for the save dialog with .jpg extension
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "untitled.jpg");

    // Run the dialog and check if the user selected a location
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *save_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Save the pixbuf (image) to the selected file in JPEG format
        if (!gdk_pixbuf_save(pixbuf, save_filename, "jpeg", NULL, "quality", "100", NULL)) {
            std::cerr << "Failed to save image!" << std::endl;
        } else {
            std::cout << "Image saved successfully as JPEG to " << save_filename << std::endl;
        }

        // Free the saved filename
        g_free(save_filename);
    }

    // Close the dialog after saving
    gtk_widget_destroy(dialog);
}

void on_copy_clicked(GtkWidget *button, GtkImage *image) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(image);
    gtk_image_set_from_pixbuf(GTK_IMAGE(cp_image), pixbuf);
}

// Function to handle the tones button click
void on_quantize_button_clicked(GtkWidget *button, GtkEntry *entry) {
    // Retrieve the text (tones) from the entry
    const char *tones_text = gtk_entry_get_text(entry);

    // Print the tones value
    std::cout << "Tones value entered: " << tones_text << std::endl;

    // You can implement further functionality to handle the tones value here
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window to display the image
    GtkWidget *image_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(image_window), "Original Image");
    gtk_window_set_default_size(GTK_WINDOW(image_window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(image_window), GTK_WIN_POS_CENTER);

    // Create second window for editting
    GtkWidget *work_image_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(work_image_window), "Work Image");
    gtk_window_set_default_size(GTK_WINDOW(work_image_window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(work_image_window), GTK_WIN_POS_CENTER);

    // Create an image widget and add it to the image window
    image = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(image_window), image);
    cp_image = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(work_image_window), cp_image);

    // Connect the destroy signal to close the application
    g_signal_connect(image_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(work_image_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);


    // Show the image window (this will show up without an image initially)
    gtk_widget_show_all(image_window);
    gtk_widget_show_all(work_image_window);

    // Create the control window 
    create_control_window(GTK_IMAGE(image), GTK_IMAGE(cp_image));

    // Start the GTK main loop
    gtk_main();

    return 0;
}
