#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <gtk/gtk.h>

#define R_WEIGHT 0.299
#define G_WEIGHT 0.587
#define B_WEIGHT 0.114


void open_image(GtkWidget *button, GtkImage *image);

void save_image(GtkWidget *button, GtkWidget *current_image);

void on_quantize_button_clicked(GtkWidget *button, GtkEntry *entry);

void on_start_restart_clicked(GtkWidget *button, GtkImage *image);

void on_vertical_mirror_clicked(GtkWidget *button, GtkImage *image);

void on_horizontal_mirror_clicked(GtkWidget *button, GtkImage *image);

void on_grayscale_clicked(GtkWidget *button, GtkWidget *current_image);

// Variable to store the loaded image filename (for saving later)
char *current_image_path = NULL;
GtkWidget *original_image, *cp_image;

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
    GtkWidget *start_restart = gtk_button_new_with_label("Start/Restart");
    gtk_box_pack_start(GTK_BOX(vbox), start_restart, TRUE, TRUE, 0);

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

    // Create a button for submitting n of tones
    GtkWidget *quantize_button = gtk_button_new_with_label("Quantize");
    gtk_box_pack_start(GTK_BOX(hbox), quantize_button, FALSE, FALSE, 0);

    // Connect the buttons to their respective functions
    g_signal_connect(open_button, "clicked", G_CALLBACK(open_image), image);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_image), cp_image);
    g_signal_connect(start_restart, "clicked", G_CALLBACK(on_start_restart_clicked), image);
    g_signal_connect(vertical_mirror, "clicked", G_CALLBACK(on_vertical_mirror_clicked), image);
    g_signal_connect(horizontal_mirror, "clicked", G_CALLBACK(on_horizontal_mirror_clicked), image);
    g_signal_connect(grayscale, "clicked", G_CALLBACK(on_grayscale_clicked), cp_image);
    g_signal_connect(quantize_button, "clicked", G_CALLBACK(on_quantize_button_clicked), GTK_ENTRY(quantize));

    // Connect the destroy signal to close the application
    g_signal_connect(control_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(control_window);
}

void open_image(GtkWidget *button, GtkImage *image) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Open Image",
        NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        ("_Cancel"), GTK_RESPONSE_CANCEL,
        ("_Open"), GTK_RESPONSE_ACCEPT,
        NULL);

    // Set filters for image files
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "../assets/test_images/");
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pixbuf_formats(filter);  // Allows image file formats
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Check if the user selected a file
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        if (current_image_path) {
            g_free(current_image_path);
        }

        // Get the selected filename
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        current_image_path = gtk_file_chooser_get_filename(chooser);

        // Set the image to the selected file
        gtk_image_set_from_file(GTK_IMAGE(original_image), current_image_path);
    }

    gtk_widget_destroy(dialog);
}

gchar* get_filename(gchar *file_path) {
    gchar *filename = NULL;
    gchar **split_array;
    const gchar *delimiter_path = "/";
    const gchar *delimiter_name = ".";
    int length = 0;

    // gets the name of the file with extension
    split_array = g_strsplit(file_path, delimiter_path, -1);
    length = g_strv_length(split_array);
    filename = g_strdup(split_array[length - 1]);

    // removes extension
    split_array = g_strsplit(filename, delimiter_name, -1);
    length = g_strv_length(split_array);
    filename = g_strdup(split_array[0]); 

    g_strfreev(split_array);

    return filename;
}

// Function to save the image as JPEG using a file chooser dialog
void save_image(GtkWidget *button, GtkWidget *current_image) {
    GtkWidget *dialog;
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(current_image));
    gchar *new_filename = g_strconcat(get_filename(current_image_path), "_edited.jpg", NULL);

    // Check if there is an image to save
    if (!current_image_path) {
        std::cerr << "No image loaded to save!" << std::endl;
        return;
    }

    // Get the pixbuf from the image widget
    if (!pixbuf) {
        std::cerr << "No image to save!" << std::endl;
        return;
    }

    // Create a file chooser dialog for saving the file
    dialog = gtk_file_chooser_dialog_new("Save Image",
        NULL,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        ("_Cancel"), GTK_RESPONSE_CANCEL,
        ("_Save"), GTK_RESPONSE_ACCEPT,
        NULL);


    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), new_filename);

    // Select image
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *save_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Save the pixbuf (image) in JPEG format
        if (!gdk_pixbuf_save(pixbuf, save_filename, "jpeg", NULL, "quality", "100", NULL)) {
            std::cerr << "Failed to save image!" << std::endl;
        } else {
            std::cout << "Image saved successfully as JPEG to " << save_filename << std::endl;
        }

        g_free(save_filename);
    }

    gtk_widget_destroy(dialog);
}

void update_pixbuf(GdkPixbuf *pixbuf, GtkWidget *current_image) {
    // Update the image with the flipped pixels
    GdkPixbuf *updated_pixbuf = gdk_pixbuf_copy(pixbuf);
    gtk_image_set_from_pixbuf(GTK_IMAGE(current_image), updated_pixbuf);
    g_object_unref(updated_pixbuf);
}

void grayscale(GtkWidget *current_image, int &min, int &max) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(current_image));
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    // n of pixels in a row
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    unsigned char *pixels = gdk_pixbuf_get_pixels(pixbuf);
    unsigned char *pixel;
    int luminance;

    // for calculating the range of the interval
    min = INT_MAX; 
    max = 0;

    for (int y = 0; y < height; ++y) {
        // Pointer to the first pixel of the current row
        unsigned char *row = pixels + y * rowstride;

        for (int x = 0; x < width; ++x) {
            pixel = row + x * n_channels; 
            luminance = pixel[0] * R_WEIGHT + pixel[1] * G_WEIGHT + pixel[2] * B_WEIGHT;
            if (luminance > max) max = luminance;
            if (luminance < min) min = luminance;

            for (int channel = 0; channel < n_channels; ++channel) {
                pixel[channel] = luminance;
            }
        }
    }

    update_pixbuf(pixbuf, cp_image);
}

void on_start_restart_clicked(GtkWidget *button, GtkImage *image) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(original_image));

    update_pixbuf(pixbuf, cp_image);
}

void on_vertical_mirror_clicked(GtkWidget *button, GtkImage *image) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(cp_image));
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    // number of bytes per row in the image data
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    // Temporary buffer to store a row
    guchar *temp_row = (guchar *) malloc(rowstride);

    for (int y = 0; y < height / 2; ++y) {
        guchar *top_row = pixels + y * rowstride;
        guchar *bottom_row = pixels + (height - y - 1) * rowstride;

        memcpy(temp_row, top_row, rowstride);
        memcpy(top_row, bottom_row, rowstride);
        memcpy(bottom_row, temp_row, rowstride);
    }

    // Free the temporary buffer
    free(temp_row);
    update_pixbuf(pixbuf, cp_image);
}

void on_horizontal_mirror_clicked(GtkWidget *button, GtkImage *image) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(cp_image));
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    unsigned char *pixels = gdk_pixbuf_get_pixels(pixbuf);

    std::cout << "number of channels: " << n_channels << std::endl;

    // Loop through each row
    for (int y = 0; y < height; ++y) {
        // Pointer to the first pixel of the current row
        unsigned char *row = pixels + y * rowstride;

        // Loop through the first half of the row
        for (int x = 0; x < width / 2; ++x) {
            // Compute positions of the pixels to swap
            unsigned char *left_pixel = row + x * n_channels;  // Left pixel
            unsigned char *right_pixel = row + (width - 1 - x) * n_channels;  // Corresponding right pixel

            // Swap the color channels (RGB or RGBA) of the left and right pixels
            for (int channel = 0; channel < n_channels; ++channel) {
                unsigned char temp = left_pixel[channel];
                left_pixel[channel] = right_pixel[channel];
                right_pixel[channel] = temp;
            }
        }
    }

    update_pixbuf(pixbuf, cp_image);
}

void on_grayscale_clicked(GtkWidget *button, GtkWidget *current_image) {
    int min, max;
    grayscale(current_image, min, max);
} 

void on_quantize_button_clicked(GtkWidget *button, GtkEntry *entry) {
    const int max_tones = atoi(gtk_entry_get_text(entry));
    GdkPixbuf *pixbuf;
    int width;
    int height;
    int rowstride;
    int n_channels;
    unsigned char *pixels;
    unsigned char *pixel;
    int luminance, min, max, bin_size, new_pixel_value;

    
    // validate input
    if (!max_tones) {
        std::cerr << "ERROR: Type the number of max tones for quantization" << std::endl;
        return;
    } else if (max_tones > 255) {
        std::cerr << "ERROR: The range accepted is [0..255]" << std::endl;
        return;
    }

    grayscale(cp_image, min, max);
    pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(cp_image));
    width = gdk_pixbuf_get_width(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);

    std::cout << "Max tones: " << max_tones << std::endl;
    std::cout << "Image tones interval: " << max - min << std::endl;

    // if interval is smaller than n° of tones
    if (max_tones > max - min) {
        std::cout << "Nothing to be done" << std::endl;
        return;
    }

    bin_size = (int) (max - min) / max_tones;
    std::cout << "Bin size: " << bin_size << std::endl;


    for (int y = 0; y < height; ++y) {
        // Pointer to the first pixel of the current row
        unsigned char *row = pixels + y * rowstride;

        for (int x = 0; x < width; ++x) {
            pixel = row + x * n_channels; 
            new_pixel_value = (int) ((int)pixel[0] / bin_size) * bin_size + (int)(bin_size / 2);

            for (int channel = 0; channel < n_channels; channel++) {
                pixel[channel] = new_pixel_value;
            }
        }
    }

    update_pixbuf(pixbuf, cp_image);
}

int main(int argc, char *argv[]) {
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
    original_image = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(image_window), original_image);
    cp_image = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(work_image_window), cp_image);

    // Connect the destroy signal to close the application
    g_signal_connect(image_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(work_image_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // show windows
    gtk_widget_show_all(image_window);
    gtk_widget_show_all(work_image_window);
    create_control_window(GTK_IMAGE(original_image), GTK_IMAGE(cp_image));

    gtk_main();

    return 0;
}
