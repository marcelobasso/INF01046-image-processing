#include "core.h"

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

void open_image(GtkWidget *button, Program_instance *program_data) {
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
        if (program_data->current_image_path) {
            g_free(program_data->current_image_path);
        }

        // Get the selected filename
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        program_data->current_image_path = gtk_file_chooser_get_filename(chooser);

        // Set the image to the selected file
        gtk_image_set_from_file(GTK_IMAGE(program_data->original_image), program_data->current_image_path);
    }

    // resize window
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->original_image));
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(program_data->original_image));
    gtk_window_resize(GTK_WINDOW(window), gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
    gtk_widget_destroy(dialog);
}

void save_image(GtkWidget *button, Program_instance *program_data) {
    GtkWidget *dialog;
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    gchar *new_filename = g_strconcat(get_filename(program_data->current_image_path), "_edited.jpg", NULL);

    if (!program_data->current_image_path) {
        std::cerr << "No image loaded to save!" << std::endl;
        return;
    }

    if (!pixbuf) {
        std::cerr << "No image to save!" << std::endl;
        return;
    }

    dialog = gtk_file_chooser_dialog_new("Save Image",
        NULL,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        ("_Cancel"), GTK_RESPONSE_CANCEL,
        ("_Save"), GTK_RESPONSE_ACCEPT,
        NULL);


    // Select image
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), new_filename);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *save_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Save the pixbuf (image) in JPEG format
        if (!gdk_pixbuf_save(pixbuf, save_filename, "jpeg", NULL, "quality", "50", NULL)) {
            std::cerr << "Failed to save image!" << std::endl;
        } else {
            std::cout << "Image saved successfully as JPEG to " << save_filename << std::endl;
        }

        g_free(save_filename);
    }

    gtk_widget_destroy(dialog);
}

void update_pixbuf(GdkPixbuf *pixbuf, GtkWidget *current_image) {
    GdkPixbuf *updated_pixbuf = gdk_pixbuf_copy(pixbuf);
    gtk_image_set_from_pixbuf(GTK_IMAGE(current_image), updated_pixbuf);
    g_object_unref(updated_pixbuf);
}

void grayscale(GtkWidget *current_image, int &min, int &max) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(current_image));
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
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

    update_pixbuf(pixbuf, current_image);
}

void on_start_restart_clicked(GtkWidget *button, Program_instance *program_data) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->original_image));
    update_pixbuf(pixbuf, program_data->working_image);

    pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(program_data->working_image));
    gtk_window_resize(GTK_WINDOW(window), gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
}

void on_vertical_mirror_clicked(GtkWidget *button, Program_instance *program_data) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    unsigned char *pixels = gdk_pixbuf_get_pixels(pixbuf);

    // Temporary buffer to store a row
    unsigned char *temp_row = (unsigned char *) malloc(rowstride);

    for (int y = 0; y < height / 2; ++y) {
        unsigned char *top_row = pixels + y * rowstride;
        unsigned char *bottom_row = pixels + (height - y - 1) * rowstride;

        memcpy(temp_row, top_row, rowstride);
        memcpy(top_row, bottom_row, rowstride);
        memcpy(bottom_row, temp_row, rowstride);
    }

    // Free the temporary buffer
    free(temp_row);
    update_pixbuf(pixbuf, program_data->working_image);
}

void on_horizontal_mirror_clicked(GtkWidget *button, Program_instance *program_data) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    unsigned char *pixels = gdk_pixbuf_get_pixels(pixbuf);

    // Loop through each row
    for (int y = 0; y < height; ++y) {
        // Pointer to the first pixel of the current row
        unsigned char *row = pixels + y * rowstride;

        for (int x = 0; x < width / 2; ++x) {
            // Compute positions of the pixels to swap
            unsigned char *left_pixel = row + x * n_channels;  
            unsigned char *right_pixel = row + (width - 1 - x) * n_channels;  // Corresponding right pixel

            // Swap the color channels 
            for (int channel = 0; channel < n_channels; ++channel) {
                unsigned char temp = left_pixel[channel];
                left_pixel[channel] = right_pixel[channel];
                right_pixel[channel] = temp;
            }
        }
    }

    update_pixbuf(pixbuf, program_data->working_image);
}

void on_grayscale_clicked(GtkWidget *button, Program_instance *program_data) {
    int min, max;
    grayscale(program_data->working_image, min, max);
} 

void on_quantize_button_clicked(GtkWidget *button, Program_instance *program_data) {
    const int max_tones = atoi(gtk_entry_get_text(program_data->quantize_entry));
    GdkPixbuf *pixbuf;
    int width;
    int height;
    int rowstride;
    int n_channels;
    unsigned char *pixels;
    unsigned char *pixel;
    int luminance, min, max, bin_size, new_pixel_value;

    if (!max_tones) {
        std::cerr << "ERROR: Type the number of max tones for quantization" << std::endl;
        return;
    } else if (max_tones > 255) {
        std::cerr << "ERROR: The range accepted is [0..255]" << std::endl;
        return;
    }

    grayscale(program_data->working_image, min, max);
    pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    width = gdk_pixbuf_get_width(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);

    // if interval is smaller than n° of tones
    if (max_tones > max - min) {
        std::cout << "Nothing to be done" << std::endl;
        return;
    }

    bin_size = (int) (max - min + 1) / max_tones;

    for (int y = 0; y < height; ++y) {
        unsigned char *row = pixels + y * rowstride;

        for (int x = 0; x < width; ++x) {
            pixel = row + x * n_channels; 
            new_pixel_value = ((int) pixel[0] / bin_size) * bin_size + (int)(bin_size / 2);
            
            if (new_pixel_value > 254) new_pixel_value = 254;

            for (int channel = 0; channel < n_channels; channel++) {
                pixel[channel] = new_pixel_value;
            }
        }
    }

    update_pixbuf(pixbuf, program_data->working_image);
}

void calculate_histogram(GdkPixbuf *pixbuf, int *histogram) {
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    unsigned char *pixels = gdk_pixbuf_get_pixels(pixbuf);
    unsigned char *pixel;
    
    for (int i = 0; i < 256; ++i) {
        histogram[i] = 0;
    }

    for (int y = 0; y < height; ++y) {
        unsigned char *row = pixels + y * rowstride;
        for (int x = 0; x < width; ++x) {
            pixel = row + x * n_channels;
            int luminance = pixel[0];  // Assuming grayscale or processed grayscale
            histogram[luminance]++;
        }
    }
}

void on_histogram_clicked(GtkWidget *button, Program_instance *program_data) {
    GdkPixbuf *pixbuf;
    int histogram[256], min, max;

    if (!pixbuf) {
        std::cerr << "No image loaded!" << std::endl;
        return;
    }

    grayscale(program_data->working_image, min, max);
    pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    calculate_histogram(pixbuf, histogram);

    GtkWidget *histogram_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(histogram_window), "Image Histogram");
    gtk_window_set_default_size(GTK_WINDOW(histogram_window), 400, 300);

    for (int i = 0; i < 256; ++i) {
        if (histogram[i] > 0) {
            std::cout << "Tone " << i << ": " << histogram[i] << std::endl;
        }
    }

    gtk_widget_show_all(histogram_window);
}

// Function to create the control window 
void create_control_window(GtkImage *original_image, GtkImage *working_image, Program_instance *program_data) {
    GtkWidget *control_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(control_window), "Control Window");
    gtk_window_set_default_size(GTK_WINDOW(control_window), INITIAL_WIDTH, INITIAL_HEIGHT);
    gtk_window_set_position(GTK_WINDOW(control_window), GTK_WIN_POS_CENTER);

    // Create buttons
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(control_window), vbox);

    GtkWidget *open_button = gtk_button_new_with_label("Open Image");
    gtk_box_pack_start(GTK_BOX(vbox), open_button, TRUE, TRUE, 3);

    GtkWidget *save_button = gtk_button_new_with_label("Save Image");
    gtk_box_pack_start(GTK_BOX(vbox), save_button, TRUE, TRUE, 0);

    // Brings image to the editting window
    GtkWidget *start_restart = gtk_button_new_with_label("Start/Restart");
    gtk_box_pack_start(GTK_BOX(vbox), start_restart, TRUE, TRUE, 0);

    GtkWidget *vertical_mirror = gtk_button_new_with_label("Vertical mirror");
    gtk_box_pack_start(GTK_BOX(vbox), vertical_mirror, TRUE, TRUE, 0);

    GtkWidget *horizontal_mirror = gtk_button_new_with_label("Horizontal mirror");
    gtk_box_pack_start(GTK_BOX(vbox), horizontal_mirror, TRUE, TRUE, 0);

    GtkWidget *grayscale = gtk_button_new_with_label("Grayscale");
    gtk_box_pack_start(GTK_BOX(vbox), grayscale, TRUE, TRUE, 0);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    GtkWidget *quantize = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(quantize), "N° of tones");
    gtk_box_pack_start(GTK_BOX(hbox), quantize, TRUE, TRUE, 0);
    program_data->quantize_entry = GTK_ENTRY(quantize);

    GtkWidget *quantize_button = gtk_button_new_with_label("Quantize");
    gtk_box_pack_start(GTK_BOX(hbox), quantize_button, FALSE, FALSE, 0);

    GtkWidget *histogram = gtk_button_new_with_label("Histogram");
    gtk_box_pack_start(GTK_BOX(vbox), histogram, TRUE, TRUE, 0);

    // Connect the buttons to their respective functions
    g_signal_connect(open_button, "clicked", G_CALLBACK(open_image), program_data);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_image), program_data);
    g_signal_connect(start_restart, "clicked", G_CALLBACK(on_start_restart_clicked), program_data);
    g_signal_connect(vertical_mirror, "clicked", G_CALLBACK(on_vertical_mirror_clicked), program_data);
    g_signal_connect(horizontal_mirror, "clicked", G_CALLBACK(on_horizontal_mirror_clicked), program_data);
    g_signal_connect(grayscale, "clicked", G_CALLBACK(on_grayscale_clicked), program_data);
    g_signal_connect(quantize_button, "clicked", G_CALLBACK(on_quantize_button_clicked), program_data);
    g_signal_connect(histogram, "clicked", G_CALLBACK(on_histogram_clicked), program_data);

    // Connect the destroy signal to close the application
    g_signal_connect(control_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(control_window);
}