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

void sanitize_pixel(int &pixel) {
    if (pixel > 255) pixel = 255;
    if (pixel < 0) pixel = 0;
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
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "assets/test_images/");
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

static gboolean draw_histogram(GtkWidget *widget, cairo_t *cr, int *histogram) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    int array_size = 256;

    // Calculate max value in the array to scale y-axis
    int max_value = *std::max_element(histogram, histogram + array_size);
    if (max_value == 0) max_value = 1;  // Avoid division by zero

    // Dimensions and scaling factors
    float bar_width = (float) width / array_size; // Width for each bar
    float y_scale = static_cast<float>(height - 40) / max_value;   // Height scale based on max frequency

    // Draw y-axis and x-axis
    cairo_set_source_rgb(cr, 255, 255, 255);
    cairo_set_line_width(cr, 4.0);

    // Y-axis
    cairo_move_to(cr, 40, 10);  // start of y-axis
    cairo_line_to(cr, 40, height - 30); // end of y-axis
    cairo_stroke(cr);

    // X-axis
    cairo_move_to(cr, 40, height - 30);
    cairo_line_to(cr, width - 10, height - 30);
    cairo_stroke(cr);

    // Draw histogram bars
    cairo_set_source_rgb(cr, 0.2, 0.5, 0.8);
    for (int i = 0; i < array_size; i++) {
        int bar_height = histogram[i] * y_scale;
        cairo_rectangle(cr, 40 + i * bar_width, height - 30 - bar_height, bar_width, bar_height);
        cairo_fill(cr);
    }

    // Draw x-axis labels (1 to 256)
    cairo_set_source_rgb(cr, 255, 255, 255);
    cairo_set_font_size(cr, 8);
    for (int i = 0; i <= 256; i += 64) { // Interval for x-axis labels
        std::string label = std::to_string(i + 1);
        cairo_move_to(cr, 40 + i * bar_width, height - 15);
        cairo_show_text(cr, label.c_str());
    }

    // Draw y-axis labels (proportions)
    int y_intervals = 5;
    for (int i = 0; i <= y_intervals; i++) {
        int proportion = (max_value * i) / y_intervals;
        std::string label = std::to_string(proportion);
        cairo_move_to(cr, 10, height - 30 - i * (height - 40) / y_intervals);
        cairo_show_text(cr, label.c_str());
    }

    return FALSE;
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
    gtk_window_set_default_size(GTK_WINDOW(histogram_window), 800, 600);

    for (int i = 0; i < 256; ++i) {
        if (histogram[i] > 0) {
            std::cout << "Tone " << i << ": " << histogram[i] << std::endl;
        }
    }

    // Create a drawing area
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(histogram_window), drawing_area);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_histogram), histogram);
    g_signal_connect(histogram_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(histogram_window);
}

void on_brightness_button_clicked(GtkWidget *button, Program_instance *program_data) {
    const int enhancement = atoi(gtk_entry_get_text(program_data->brightness_entry));
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    unsigned char *pixels = gdk_pixbuf_get_pixels(pixbuf);
    unsigned char *pixel;
    int luminance, min, max, new_pixel_value;

    if (!enhancement) {
        std::cerr << "ERROR: Type a number to enhance brightness" << std::endl;
        return;
    } else if (enhancement > 255) {
        std::cerr << "ERROR: The range accepted is [0..255]" << std::endl;
        return;
    }

    for (int y = 0; y < height; ++y) {
        unsigned char *row = pixels + y * rowstride;
        for (int x = 0; x < width; ++x) {
            for (int channel = 0; channel < n_channels; channel++) {
                pixel = row + x * n_channels;
                new_pixel_value = pixel[channel] + enhancement;
                sanitize_pixel(new_pixel_value);
                pixel[channel] = new_pixel_value;
            }
        }
    }

    update_pixbuf(pixbuf, program_data->working_image);
}

void on_contrast_button_clicked(GtkWidget *button, Program_instance *program_data) {
    const gchar *value = gtk_entry_get_text(program_data->contrast_entry);
    const float enhancement = std::stof(value);
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    unsigned char *pixels = gdk_pixbuf_get_pixels(pixbuf);
    unsigned char *pixel;
    int luminance, min, max, new_pixel_value;

    if (!enhancement) {
        std::cerr << "ERROR: Type a number to enhance contrast" << std::endl;
        return;
    } else if (enhancement > 255) {
        std::cerr << "ERROR: The range accepted is [0..255]" << std::endl;
        return;
    }

    for (int y = 0; y < height; ++y) {
        unsigned char *row = pixels + y * rowstride;
        for (int x = 0; x < width; ++x) {
            for (int channel = 0; channel < n_channels; channel++) {
                pixel = row + x * n_channels;
                new_pixel_value = (int) pixel[channel] * enhancement;
                sanitize_pixel(new_pixel_value);
                pixel[channel] = new_pixel_value;
            }
        }
    }

    update_pixbuf(pixbuf, program_data->working_image);
}

void on_negative_button_clicked(GtkWidget *button, Program_instance *program_data) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    unsigned char *pixels = gdk_pixbuf_get_pixels(pixbuf);
    unsigned char *pixel;
    int luminance, min, max, new_pixel_value;

    for (int y = 0; y < height; ++y) {
        unsigned char *row = pixels + y * rowstride;
        for (int x = 0; x < width; ++x) {
            for (int channel = 0; channel < n_channels; channel++) {
                pixel = row + x * n_channels;
                new_pixel_value = 255 - pixel[channel];
                pixel[channel] = new_pixel_value;
            }
        }
    }

    update_pixbuf(pixbuf, program_data->working_image);
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
    
    // Label for the first section
    GtkWidget *label1 = gtk_label_new("Basic Operations");
    gtk_box_pack_start(GTK_BOX(vbox), label1, FALSE, FALSE, 10);

    GtkWidget *open_button = gtk_button_new_with_label("Open Image");
    gtk_box_pack_start(GTK_BOX(vbox), open_button, TRUE, TRUE, 3);

    GtkWidget *save_button = gtk_button_new_with_label("Save Image");
    gtk_box_pack_start(GTK_BOX(vbox), save_button, TRUE, TRUE, 0);

    GtkWidget *start_restart = gtk_button_new_with_label("Start/Restart");
    gtk_box_pack_start(GTK_BOX(vbox), start_restart, TRUE, TRUE, 0);

    GtkWidget *vertical_mirror = gtk_button_new_with_label("Vertical mirror");
    gtk_box_pack_start(GTK_BOX(vbox), vertical_mirror, TRUE, TRUE, 0);

    GtkWidget *horizontal_mirror = gtk_button_new_with_label("Horizontal mirror");
    gtk_box_pack_start(GTK_BOX(vbox), horizontal_mirror, TRUE, TRUE, 0);

    GtkWidget *grayscale = gtk_button_new_with_label("Grayscale");
    gtk_box_pack_start(GTK_BOX(vbox), grayscale, TRUE, TRUE, 0);

    GtkWidget *histogram = gtk_button_new_with_label("Show histogram");
    gtk_box_pack_start(GTK_BOX(vbox), histogram, TRUE, TRUE, 0);

    GtkWidget *negative = gtk_button_new_with_label("Negative");
    gtk_box_pack_start(GTK_BOX(vbox), negative, TRUE, TRUE, 0);


    // Create a vertical box to hold the entries and their buttons
    GtkWidget *controls_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), controls_vbox, TRUE, TRUE, 5);

    // ---------------------------------------------------------------------------
    // Separator for visual division
    GtkWidget *separator1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(controls_vbox), separator1, FALSE, TRUE, 5);

    // Label for the second section
    GtkWidget *label2 = gtk_label_new("Adjustments");
    gtk_box_pack_start(GTK_BOX(controls_vbox), label2, FALSE, FALSE, 10);

    // Create a horizontal box for the quantize entry and its button
    GtkWidget *quantize_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(controls_vbox), quantize_hbox, TRUE, TRUE, 0);

    GtkWidget *quantize = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(quantize), "N° of tones");
    gtk_entry_set_width_chars(GTK_ENTRY(quantize), 15);
    gtk_widget_set_halign(quantize, GTK_ALIGN_START);  // Align left to prevent expanding
    gtk_box_pack_start(GTK_BOX(quantize_hbox), quantize, FALSE, FALSE, 0);
    program_data->quantize_entry = GTK_ENTRY(quantize);

    GtkWidget *quantize_button = gtk_button_new_with_label("Quantize");
    gtk_box_pack_start(GTK_BOX(quantize_hbox), quantize_button, TRUE, TRUE, 0);

    // Create a horizontal box for the brightness entry and its button
    GtkWidget *brightness_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(controls_vbox), brightness_hbox, TRUE, TRUE, 0);

    GtkWidget *brightness = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(brightness), "Brightness");
    gtk_entry_set_width_chars(GTK_ENTRY(brightness), 15);
    gtk_widget_set_halign(brightness, GTK_ALIGN_START);  // Align left to prevent expanding
    gtk_box_pack_start(GTK_BOX(brightness_hbox), brightness, FALSE, FALSE, 0);
    program_data->brightness_entry = GTK_ENTRY(brightness);

    GtkWidget *brightness_button = gtk_button_new_with_label("Adjust");
    gtk_box_pack_start(GTK_BOX(brightness_hbox), brightness_button, TRUE, TRUE, 0);

    // Contrast
    GtkWidget *contrast_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(controls_vbox), contrast_hbox, TRUE, TRUE, 0);

    GtkWidget *contrast = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(contrast), "Contrast");
    gtk_entry_set_width_chars(GTK_ENTRY(contrast), 15);
    gtk_widget_set_halign(contrast, GTK_ALIGN_START);  // Align left to prevent expanding
    gtk_box_pack_start(GTK_BOX(contrast_hbox), contrast, FALSE, FALSE, 0);
    program_data->contrast_entry = GTK_ENTRY(contrast);

    GtkWidget *contrast_button = gtk_button_new_with_label("Adjust");
    gtk_box_pack_start(GTK_BOX(contrast_hbox), contrast_button, TRUE, TRUE, 0);

    // Connect the buttons to their respective functions
    g_signal_connect(open_button, "clicked", G_CALLBACK(open_image), program_data);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_image), program_data);
    g_signal_connect(start_restart, "clicked", G_CALLBACK(on_start_restart_clicked), program_data);
    g_signal_connect(vertical_mirror, "clicked", G_CALLBACK(on_vertical_mirror_clicked), program_data);
    g_signal_connect(horizontal_mirror, "clicked", G_CALLBACK(on_horizontal_mirror_clicked), program_data);
    g_signal_connect(grayscale, "clicked", G_CALLBACK(on_grayscale_clicked), program_data);
    g_signal_connect(negative, "clicked", G_CALLBACK(on_negative_button_clicked), program_data);    
    g_signal_connect(histogram, "clicked", G_CALLBACK(on_histogram_clicked), program_data);

    g_signal_connect(quantize_button, "clicked", G_CALLBACK(on_quantize_button_clicked), program_data);
    g_signal_connect(brightness_button, "clicked", G_CALLBACK(on_brightness_button_clicked), program_data);
    g_signal_connect(contrast_button, "clicked", G_CALLBACK(on_contrast_button_clicked), program_data);    


    g_signal_connect(control_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(control_window);
}