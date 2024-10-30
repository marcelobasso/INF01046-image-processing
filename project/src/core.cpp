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

// Defines interval of [0..255] to pixel accepted values
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

        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        program_data->current_image_path = gtk_file_chooser_get_filename(chooser);

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


    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), new_filename);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *save_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        if (!gdk_pixbuf_save(pixbuf, save_filename, "jpeg", NULL, "quality", "50", NULL)) {
            std::cerr << "Failed to save image!" << std::endl;
        } else {
            std::cout << "Image saved successfully as JPEG to " << save_filename << std::endl;
        }

        g_free(save_filename);
    }

    gtk_widget_destroy(dialog);
}

void update_pixbuf(GdkPixbuf *pixbuf, Program_instance *program_data) {
    GdkPixbuf *updated_pixbuf = gdk_pixbuf_copy(pixbuf);
    gtk_image_set_from_pixbuf(GTK_IMAGE(program_data->working_image), updated_pixbuf);
    
    program_data->img_data.pixbuf = updated_pixbuf;
    program_data->img_data.width = gdk_pixbuf_get_width(updated_pixbuf);
    program_data->img_data.height = gdk_pixbuf_get_height(updated_pixbuf);
    program_data->img_data.rowstride = gdk_pixbuf_get_rowstride(updated_pixbuf);
    program_data->img_data.n_channels = gdk_pixbuf_get_n_channels(updated_pixbuf);
    program_data->img_data.pixels = gdk_pixbuf_get_pixels(updated_pixbuf);

    g_object_unref(updated_pixbuf);
}

void grayscale(Program_instance *program_data) {
    Image_data *img_data = &program_data->img_data;
    unsigned char *pixel;
    int luminance;

    for (int y = 0; y < img_data->height; ++y) {
        unsigned char *row = img_data->pixels + y * img_data->rowstride;

        for (int x = 0; x < img_data->width; ++x) {
            pixel = row + x * img_data->n_channels; 
            luminance = pixel[0] * R_WEIGHT + pixel[1] * G_WEIGHT + pixel[2] * B_WEIGHT;
            if (luminance > img_data->max) img_data->max = luminance;
            if (luminance < img_data->min) img_data->min = luminance;

            for (int channel = 0; channel < img_data->n_channels; ++channel) {
                pixel[channel] = luminance;
            }
        }
    }

    update_pixbuf(img_data->pixbuf, program_data);
}

void on_grayscale_clicked(GtkWidget *button, Program_instance *program_data){
    grayscale(program_data);
}

void on_start_restart_clicked(GtkWidget *button, Program_instance *program_data) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->original_image));
    update_pixbuf(pixbuf, program_data);

    pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(program_data->working_image));
    gtk_window_resize(GTK_WINDOW(window), gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));

    program_data->img_data.pixbuf = pixbuf;
    program_data->img_data.width = gdk_pixbuf_get_width(pixbuf);
    program_data->img_data.height = gdk_pixbuf_get_height(pixbuf);
    program_data->img_data.rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    program_data->img_data.n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    program_data->img_data.pixels = gdk_pixbuf_get_pixels(pixbuf);
    program_data->img_data.min = INT_MAX;
    program_data->img_data.max = 0;
}

void on_vertical_mirror_clicked(GtkWidget *button, Program_instance *program_data) {
    Image_data img_data = program_data->img_data;
    unsigned char *temp_row = (unsigned char *) malloc(img_data.rowstride);
    
    for (int y = 0; y < img_data.height / 2; ++y) {
        unsigned char *top_row = img_data.pixels + y * img_data.rowstride;
        unsigned char *bottom_row = img_data.pixels + (img_data.height - y - 1) * img_data.rowstride;

        memcpy(temp_row, top_row, img_data.rowstride);
        memcpy(top_row, bottom_row, img_data.rowstride);
        memcpy(bottom_row, temp_row, img_data.rowstride);
    }

    free(temp_row);
    update_pixbuf(img_data.pixbuf, program_data);
}

void on_horizontal_mirror_clicked(GtkWidget *button, Program_instance *program_data) {
    Image_data img_data = program_data->img_data;

    // Loop through each row
    for (int y = 0; y < img_data.height; ++y) {
        unsigned char *row = img_data.pixels + y * img_data.rowstride;

        for (int x = 0; x < img_data.width / 2; ++x) {
            unsigned char *left_pixel = row + x * img_data.n_channels;  
            unsigned char *right_pixel = row + (img_data.width - 1 - x) * img_data.n_channels;  // Corresponding right pixel

            // Swap the color channels 
            for (int channel = 0; channel < img_data.n_channels; ++channel) {
                unsigned char temp = left_pixel[channel];
                left_pixel[channel] = right_pixel[channel];
                right_pixel[channel] = temp;
            }
        }
    }

    update_pixbuf(img_data.pixbuf, program_data);
}

void on_quantize_button_clicked(GtkWidget *button, Program_instance *program_data) {
    const int max_tones = atoi(gtk_entry_get_text(program_data->quantize_entry));
    grayscale(program_data);
    Image_data img_data = program_data->img_data;
    int luminance, bin_size, new_pixel_value;
    unsigned char *pixel;
    
    if (!max_tones) {
        std::cerr << "ERROR: Type the number of max tones for quantization" << std::endl;
        return;
    } else if (max_tones > 255) {
        std::cerr << "ERROR: The range accepted is [0..255]" << std::endl;
        return;
    }

    // if interval is smaller than n° of tones
    if (max_tones > img_data.max - img_data.min) {
        std::cout << "Nothing to be done" << std::endl;
        return;
    }

    bin_size = (int) (img_data.max - img_data.min + 1) / max_tones;
    std::cout << bin_size << " " << max_tones << " ";
    for (int y = 0; y < img_data.height; ++y) {
        unsigned char *row = img_data.pixels + y * img_data.rowstride;

        for (int x = 0; x < img_data.width; ++x) {
            pixel = row + x * img_data.n_channels; 
            new_pixel_value = ((int) pixel[0] / bin_size) * bin_size + (int)(bin_size / 2);
            
            if (new_pixel_value > 254) new_pixel_value = 254;

            for (int channel = 0; channel < img_data.n_channels; channel++) {
                pixel[channel] = new_pixel_value;
            }
        }
    }

    update_pixbuf(img_data.pixbuf, program_data);
}

void calculate_histogram(Program_instance *program_data) {
    Image_data *img_data = &program_data->img_data;
    int n_of_pixels = img_data->width * img_data->height;
    int index;
    unsigned char *row, *pixel;

    std::memset(img_data->histogram, 0, sizeof(img_data->histogram));
    img_data->max_histogram = 0;

    // accumulates pixels
    for (int y = 0; y < img_data->height; ++y) {
        row = img_data->pixels + y * img_data->rowstride;
        for (int x = 0; x < img_data->width; ++x) {
            pixel = row + x * img_data->n_channels;
            index = pixel[0];
            img_data->histogram[index] += 1; // Assuming grayscale or processed grayscale
        }
    }

    // calculate frequencies
    for (int i = 0; i < 256; i++) {
        img_data->histogram[i] /= n_of_pixels;
        if (img_data->histogram[i] > img_data->max_histogram){
            img_data->max_histogram = img_data->histogram[i];
        }
    }
}

static gboolean draw_histogram(GtkWidget *widget, cairo_t *cr, Image_data *img_data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    int array_size = 256;

    // Calculate max value in the array to scale y-axis
    float max_value = img_data->max_histogram;
    // if (max_value == 0) max_value = 1;  // Avoid division by zero

    // Dimensions and scaling factors
    float bar_width = width / array_size; // Width for each bar
    float y_scale = ((float) height - 64) / max_value;   // Height scale based on max frequency

    // Draw y-axis and x-axis
    cairo_set_source_rgb(cr, 255, 255, 255);
    cairo_set_line_width(cr, 2.0);

    // Y-axis
    cairo_move_to(cr, 39, 10);  // start of y-axis
    cairo_line_to(cr, 39, height - 30); // end of y-axis
    cairo_stroke(cr);

    // X-axis
    cairo_move_to(cr, 39, height - 30);
    cairo_line_to(cr, width - 10, height - 30);
    cairo_stroke(cr);

    // Draw histogram bars
    cairo_set_source_rgb(cr, 0.2, 0.5, 0.8);
    for (int i = 0; i < array_size; i++) {
        int bar_height = img_data->histogram[i] * y_scale;
        cairo_rectangle(cr, 40 + i * bar_width, height - 30 - bar_height, bar_width, bar_height);
        cairo_fill(cr);
    }

    // Draw x-axis labels (1 to 256)
    cairo_set_source_rgb(cr, 255, 255, 255);
    cairo_set_font_size(cr, 8);
    for (int i = 0; i < 256; i += 15) { // Interval for x-axis labels
        std::string label = std::to_string(i);
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

void close_window(GtkWidget *widget, gpointer data) {
    gtk_widget_destroy(widget);
}

void on_histogram_clicked(GtkWidget *button, Program_instance *program_data) {
    Image_data *img_data = &program_data->img_data;
    
    if (!img_data->pixbuf) {
        std::cerr << "No image loaded!" << std::endl;
        return;
    }
    grayscale(program_data);
    calculate_histogram(program_data);

    GtkWidget *histogram_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(histogram_window), "Image Histogram");
    gtk_window_set_default_size(GTK_WINDOW(histogram_window), 1000, 600);

    // Create a drawing area
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(histogram_window), drawing_area);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_histogram), img_data);
    g_signal_connect(histogram_window, "destroy", G_CALLBACK(close_window), NULL);

    gtk_widget_show_all(histogram_window);
}

void on_brightness_button_clicked(GtkWidget *button, Program_instance *program_data) {
    const int enhancement = atoi(gtk_entry_get_text(program_data->brightness_entry));
    Image_data img_data = program_data->img_data;
    int new_pixel_value;
    unsigned char *pixel;

    if (!enhancement) {
        std::cerr << "ERROR: Type a number to enhance brightness" << std::endl;
        return;
    } else if (enhancement > 255) {
        std::cerr << "ERROR: The range accepted is [0..255]" << std::endl;
        return;
    }

    for (int y = 0; y < img_data.height; ++y) {
        unsigned char *row = img_data.pixels + y * img_data.rowstride;
        for (int x = 0; x < img_data.width; ++x) {
            for (int channel = 0; channel < img_data.n_channels; channel++) {
                pixel = row + x * img_data.n_channels;
                new_pixel_value = pixel[channel] + enhancement;
                sanitize_pixel(new_pixel_value);
                pixel[channel] = new_pixel_value;
            }
        }
    }

    update_pixbuf(img_data.pixbuf, program_data);
}

void on_contrast_button_clicked(GtkWidget *button, Program_instance *program_data) {
    const gchar *value = gtk_entry_get_text(program_data->contrast_entry);
    const float enhancement = std::stof(value);
    Image_data img_data = program_data->img_data;
    int new_pixel_value;
    unsigned char *pixel;

    if (!enhancement) {
        std::cerr << "ERROR: Type a number to enhance contrast" << std::endl;
        return;
    } else if (enhancement > 255) {
        std::cerr << "ERROR: The range accepted is [0..255]" << std::endl;
        return;
    }

    for (int y = 0; y < img_data.height; ++y) {
        unsigned char *row = img_data.pixels + y * img_data.rowstride;
        for (int x = 0; x < img_data.width; ++x) {
            for (int channel = 0; channel < img_data.n_channels; channel++) {
                pixel = row + x * img_data.n_channels;
                new_pixel_value = pixel[channel] * enhancement;
                sanitize_pixel(new_pixel_value);
                pixel[channel] = new_pixel_value;
            }
        }
    }

    update_pixbuf(img_data.pixbuf, program_data);
}

void on_negative_button_clicked(GtkWidget *button, Program_instance *program_data) {
    Image_data img_data = program_data->img_data;
    unsigned char *pixel;

    for (int y = 0; y < img_data.height; ++y) {
        unsigned char *row = img_data.pixels + y * img_data.rowstride;
        for (int x = 0; x < img_data.width; ++x) {
            for (int channel = 0; channel < img_data.n_channels; channel++) {
                pixel = row + x * img_data.n_channels;
                pixel[channel] = 255 - pixel[channel];
            }
        }
    }

    update_pixbuf(img_data.pixbuf, program_data);
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
    gtk_widget_set_margin_start(vbox, 5);
    gtk_widget_set_margin_end(vbox, 5);
    
    // Label for the first section
    GtkWidget *label_operations = gtk_label_new("Basic Operations");
    gtk_box_pack_start(GTK_BOX(vbox), label_operations, FALSE, FALSE, 10);

    GtkWidget *open_button = gtk_button_new_with_label("Open Image");
    gtk_box_pack_start(GTK_BOX(vbox), open_button, TRUE, TRUE, 3);

    GtkWidget *save_button = gtk_button_new_with_label("Save Image");
    gtk_box_pack_start(GTK_BOX(vbox), save_button, TRUE, TRUE, 0);

    GtkWidget *start_restart = gtk_button_new_with_label("Start/Restart");
    gtk_box_pack_start(GTK_BOX(vbox), start_restart, TRUE, TRUE, 0);

    // Label for the first section
    GtkWidget *label_editting = gtk_label_new("Editting");
    gtk_box_pack_start(GTK_BOX(vbox), label_editting, FALSE, FALSE, 10);

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
    // GtkWidget *separator1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    // gtk_box_pack_start(GTK_BOX(controls_vbox), separator1, FALSE, TRUE, 5);

    // Label for the second section
    GtkWidget *label_adjustments = gtk_label_new("Adjustments");
    gtk_box_pack_start(GTK_BOX(controls_vbox), label_adjustments, FALSE, FALSE, 10);

    // Create a horizontal box for the quantize entry and its button
    GtkWidget *quantize_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(controls_vbox), quantize_hbox, TRUE, TRUE, 0);

    auto quantize = gtk_entry_new();
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