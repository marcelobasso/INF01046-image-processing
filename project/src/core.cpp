#ifndef UTILS
#define UTILS
#include "utils.h"
#endif

#include "core.h"

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

    // only apply luminance once
    if (img_data->is_grayscale) return;

    for (int y = 0; y < img_data->height; ++y) {
        unsigned char *row = img_data->pixels + y * img_data->rowstride;

        for (int x = 0; x < img_data->width; ++x) {
            pixel = row + x * img_data->n_channels; 
            luminance = get_luminance(pixel);
            if (luminance > img_data->max) img_data->max = luminance;
            if (luminance < img_data->min) img_data->min = luminance;

            for (int channel = 0; channel < img_data->n_channels; ++channel) {
                pixel[channel] = luminance;
            }
        }
    }

    img_data->is_grayscale = TRUE;
    update_pixbuf(img_data->pixbuf, program_data);
}

void on_grayscale_clicked(GtkWidget *button, Program_instance *program_data){
    grayscale(program_data);
}

void on_start_restart_clicked(GtkWidget *button, Program_instance *program_data) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->original_image));
    update_pixbuf(pixbuf, program_data);
    resize_working_window(program_data);
    update_working_image_data(program_data);
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
    img_data->n_of_pixels = img_data->width * img_data->height;
    int index;
    unsigned char *row, *pixel;

    // accumulates pixels
    for (int y = 0; y < img_data->height; ++y) {
        row = img_data->pixels + y * img_data->rowstride;
        for (int x = 0; x < img_data->width; ++x) {
            pixel = row + x * img_data->n_channels;

            // if not grayscale, gets histogram_cum of the pixel luminance value
            index = img_data->is_grayscale ? pixel[0] : get_luminance(pixel);
            img_data->histogram[index] += 1;
        }
    }

    // calculate frequencies
    for (int i = 0; i < 256; i++) {
        img_data->histogram[i] /= img_data->n_of_pixels;
        if (img_data->histogram[i] > img_data->max_histogram){
            img_data->max_histogram = img_data->histogram[i];
        }
    }
}

void close_window(GtkWidget *widget, gpointer data) {
    gtk_widget_destroy(widget);
}

void on_histogram_clicked(GtkWidget *button, Program_instance *program_data) {
    Image_data *img_data = &program_data->img_data;
    std::memset(img_data->histogram, 0, sizeof(img_data->histogram));
    img_data->max_histogram = 0;

    if (!img_data->pixbuf) {
        std::cerr << "No image loaded!" << std::endl;
        return;
    }

    if (!img_data->is_grayscale) {
        grayscale(program_data);
    }
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
                pixel[channel] = sanitize_pixel(new_pixel_value);
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
                pixel[channel] = sanitize_pixel(new_pixel_value);
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

void on_equalization_clicked(GtkWidget *button, Program_instance *program_data) {
    Image_data *img_data = &program_data->img_data;
    std::memset(img_data->histogram, 0, sizeof(img_data->histogram));
    img_data->max_histogram = 0;
    float histogram_cum[256] = {0};
    unsigned char *row, *pixel;
    int new_pixel_value;

    if (!img_data->pixbuf) {
        std::cerr << "No image loaded!" << std::endl;
        return;
    }
    calculate_histogram(program_data);

    // computes the (renormalized) cumulative histogram
    histogram_cum[0] = 256 * img_data->histogram[0];
    for (int i = 1; i < 256; i++) {
        histogram_cum[i] = histogram_cum[i - 1] + 256 * img_data->histogram[i];
    }

    // use renormalized cumulative histogram as equalization function
    for (int y = 0; y < img_data->height; ++y) {
        row = img_data->pixels + y * img_data->rowstride;
        for (int x = 0; x < img_data->width; ++x) {
            pixel = row + x * img_data->n_channels;
            for (int c = 0; c < img_data->n_channels; ++c) {
                new_pixel_value = (int) histogram_cum[pixel[c]];
                pixel[c] = sanitize_pixel(new_pixel_value);
            }
        }
    }

    update_pixbuf(img_data->pixbuf, program_data);
}

void on_zoom_out_clicked(GtkWidget *button, Program_instance *program_data) {
    const gchar *x_str = gtk_entry_get_text(program_data->zoom_out_x_entry);
    const gchar *y_str = gtk_entry_get_text(program_data->zoom_out_y_entry);
    Image_data *img_data = &program_data->img_data;
    int x_rect = std::atoi(x_str), y_rect = std::atoi(y_str);
    int new_width = (img_data->width + x_rect - 1) / x_rect; 
    int new_height = (img_data->height + y_rect - 1) / y_rect;
    unsigned char *raw_data = new unsigned char[new_width * new_height * 3], *row, *pixel;
    int rect_sum = 0, index;

    if (!x_rect || !y_rect) {
        std::cerr << "ERROR: Type both the zoom out factors" << std::endl;
        return;
    }

    // for every square of given sizes
    for (int y = 0; y < img_data->height; y += y_rect) {
        for (int x = 0; x < img_data->width; x += x_rect) {
            // for every channel
            for (int c = 0; c < img_data->n_channels; c++) {
                rect_sum = 0;

                // for every pixel inside the giving square
                for (int inner_y = 0; inner_y < y_rect && (y + inner_y) < img_data->height; inner_y++) {
                    for (int inner_x = 0; inner_x < x_rect && (x + inner_x) < img_data->width; inner_x++) {
                        // gets the correct row inside the rectangle
                        row = img_data->pixels + (y + inner_y) * img_data->rowstride;
                        pixel = row + (x + inner_x) * img_data->n_channels;
                        // sums the pixel value in that channeç
                        rect_sum += pixel[c];
                    }
                }

                // adds the medium of the pixel values inside the rectangle to the
                // right index on the new image on the respective channel
                index = ((y / y_rect) * new_width + (x / x_rect)) * 3 + c;
                raw_data[index] = rect_sum / (x_rect * y_rect);
            }
        }
    }

    auto new_pixbuf = create_pixbuf_from_data(raw_data, new_width, new_height);
    update_pixbuf(new_pixbuf, program_data);
    resize_working_window(program_data);
    delete[] raw_data;
    g_object_unref(new_pixbuf);
}

void on_zoom_in_clicked(GtkWidget *button, Program_instance *program_data) {
    Image_data *img_data = &program_data->img_data;
    int new_width = img_data->width * 2 - 1;
    int new_height = img_data->height * 2 - 1;
    unsigned char *raw_data = new unsigned char[new_width * new_height * 3];
    unsigned char *pixel, *next_pixel, *row, *next_row, *diag_pixel;
    std::memset(raw_data, 0, new_width * new_height * 3);

    // Loop through each pixel in the original image and set the corresponding pixels in the zoomed image
    for (int y = 0; y < img_data->height; y++) {
        row = img_data->pixels + y * img_data->rowstride;
        for (int x = 0; x < img_data->width; x++) {
            pixel = row + x * img_data->n_channels;
            
            for (int c = 0; c < img_data->n_channels; c++) {
                int index = ((y * 2) * new_width + (x * 2)) * 3 + c;
                raw_data[index] = pixel[c];

                // horizontal middle if within bounds
                if (x < img_data->width - 1) {
                    next_pixel = row + (x + 1) * img_data->n_channels;
                    raw_data[index + 3] = (pixel[c] + next_pixel[c]) / 2;
                }

                // vertical middle if within bounds
                if (y < img_data->height - 1) {
                    next_row = img_data->pixels + (y + 1) * img_data->rowstride;
                    next_pixel = next_row + x * img_data->n_channels;
                    raw_data[index + new_width * 3] = (pixel[c] + next_pixel[c]) / 2;
                }

                // FIX: not calculating as described on the assignment!
                // diagonal pixel if within bounds
                if (x < img_data->width - 1 && y < img_data->height - 1) {
                    next_row = img_data->pixels + (y + 1) * img_data->rowstride;
                    diag_pixel = next_row + (x + 1) * img_data->n_channels;
                    raw_data[index + new_width * 3 + 3] = (pixel[c] + diag_pixel[c]) / 2;
                }
            }
        }
    }

    auto new_pixbuf = create_pixbuf_from_data(raw_data, new_width, new_height);
    update_pixbuf(new_pixbuf, program_data);
    resize_working_window(program_data);

    delete[] raw_data; // Clean up allocated memory
    g_object_unref(new_pixbuf);
}

void rotate_image(Program_instance *program_data, int rotation) {
    Image_data *img_data = &program_data->img_data;
    int new_width = img_data->height;
    int new_height = img_data->width;
    unsigned char *raw_data = new unsigned char[new_width * new_height * 3];
    unsigned char *pixel, *row;
    int x_dest, y_dest;
    std::memset(raw_data, 0, new_width * new_height * 3);

    // Loop through each pixel in the original image and set the corresponding pixels in the rotated image
    for (int y = 0; y < img_data->height; y++) {
        row = img_data->pixels + y * img_data->rowstride;
        for (int x = 0; x < img_data->width; x++) {
            pixel = row + x * img_data->n_channels;

            if (rotation == ROTATE_RIGHT) {
                x_dest = img_data->height - 1 - y;
                y_dest = x;
            } else {
                x_dest = y;
                y_dest = img_data->width - 1 - x;
            }

            int index = (y_dest * new_width + x_dest) * img_data->n_channels;
            for (int c = 0; c < img_data->n_channels; c++) {
                raw_data[index + c] = pixel[c];
            }
        }
    }

    auto new_pixbuf = create_pixbuf_from_data(raw_data, new_width, new_height);
    update_pixbuf(new_pixbuf, program_data);
    resize_working_window(program_data);

    delete[] raw_data; // Clean up allocated memory
    g_object_unref(new_pixbuf);
}

void on_rotate_r_clicked(GtkWidget *button, Program_instance *program_data) {
    rotate_image(program_data, ROTATE_RIGHT);
}

void on_rotate_l_clicked(GtkWidget *button, Program_instance *program_data) {
    rotate_image(program_data, ROTATE_LEFT);
}

// Function to set kernel text when a preset button is clicked
void set_kernel_text(GtkWidget *button, gpointer data) {
    const char* kernel_text = (const char*)data;
    GtkEntry *entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry"));
    gtk_entry_set_text(entry, kernel_text);
}

void extract_kernel_input(Program_instance *program_data, std::vector<std::vector<float>> &kernel) {
    const gchar *kernel_input = gtk_entry_get_text(program_data->kernel_entry);
    std::string kernel_str(kernel_input);

    kernel_str.erase(std::remove(kernel_str.begin(), kernel_str.end(), '['), kernel_str.end());
    kernel_str.erase(std::remove(kernel_str.begin(), kernel_str.end(), ']'), kernel_str.end());
    std::replace(kernel_str.begin(), kernel_str.end(), ',', ' ');

    std::istringstream stream(kernel_str);
    float value;
    int row = 0, col = 0;

    while (stream >> value) {
        kernel[row][col] = value;
        col++;
        if (col == 3) {
            col = 0;
            row++;
        }
    }
}

void copy_pixel_data(unsigned char* dest, unsigned char* src) {
    for (int c = 0; c < 3; c++) {
        dest[c] = src[c];
    }
}

void rotate_kernel_180(std::vector<std::vector<float>> &kernel) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            std::swap(kernel[i][j], kernel[2 - i][2 - j]);
        }
    }
}

// Function to handle the "Convolve" button click (for demonstration purposes)
void on_convolve_clicked(GtkWidget *button, Program_instance *program_data) {
    std::vector<std::vector<float>> kernel(3, std::vector<float>(3));
    Image_data *img_data = &program_data->img_data;
    extract_kernel_input(program_data, kernel);
    rotate_kernel_180(kernel);
    // creates image with borders extended
    int ext_height = img_data->height + 2;
    int ext_width = img_data->width + 2;
    unsigned char *raw_data_ext = new unsigned char[ext_height * ext_width * 3];
    // creates image for final convolution result
    unsigned char *raw_data_conv = new unsigned char[img_data->height * img_data->width * 3];
    unsigned char *pixel, *row;
    bool clammping = gtk_toggle_button_get_active(program_data->clampping) && (kernel[1][1] != 0.25 && kernel[1][1] != 8);
    int new_pixel[3];
    std::memset(raw_data_ext, 0, ext_height * ext_width * 3);
    std::memset(raw_data_conv, 0, img_data->height * img_data->width * 3);
    int index;

    // extends image borders
    // (hope u are not frightened by waling pointers)
    for (int y = 0; y < img_data->height; y++) {
        row = img_data->pixels + y * img_data->rowstride;
        for (int x = 0; x < img_data->width; x++) {
            pixel = row + x * img_data->n_channels;

            if (y == 0) {
                copy_pixel_data(raw_data_ext + (y * ext_width + (x + 1)) * 3, pixel);

                if (x == 0) //corner
                    copy_pixel_data(raw_data_ext + (y * ext_width + x) * 3, pixel);

                if (x == img_data->width - 1) // corner
                    copy_pixel_data(raw_data_ext + (y * ext_width + (x + 2)) * 3, pixel);
            }
            
            if (y == img_data->height - 1) {
                copy_pixel_data(raw_data_ext + ((y + 2)* ext_width + (x + 1)) * 3, pixel);

                if (x == 0) //corner
                    copy_pixel_data(raw_data_ext + ((y + 2) * ext_width + x) * 3, pixel);

                if (x == img_data->width - 1) // corner
                    copy_pixel_data(raw_data_ext + ((y + 2)* ext_width + (x + 2)) * 3, pixel);
            }

            if (x == 0)
                copy_pixel_data(raw_data_ext + ((y + 1) * ext_width + x) * 3, pixel);

            if (x == img_data->width - 1)
                copy_pixel_data(raw_data_ext + ((y + 1) * ext_width + (x + 2)) * 3, pixel);

            for (int c = 0; c < 3; c++)
                copy_pixel_data(raw_data_ext + ((y + 1) * ext_width + (x + 1)) * 3, pixel);
        }
    }
    auto pixbuf_ext = create_pixbuf_from_data(raw_data_ext, ext_width, ext_height);
    auto pixels_ext = gdk_pixbuf_get_pixels(pixbuf_ext);
    auto rowstride_ext = gdk_pixbuf_get_rowstride(pixbuf_ext);

    // Apply convolution on the image data
    for (int y = 1; y < img_data->height; y++) {
        for (int x = 1; x < img_data->width; x++) {
            std::fill(new_pixel, new_pixel + 3, 0);

            for (int inner_y = 0; inner_y < 3; inner_y++) {
                for (int inner_x = 0; inner_x < 3; inner_x++) {
                    row = pixels_ext + (y + inner_y - 1) * rowstride_ext;  // Adjust for offset
                    pixel = row + (x + inner_x - 1) * 3;

                    for (int c = 0; c < 3; c++) {
                        new_pixel[c] += kernel[inner_y][inner_x] * pixel[c];
                    }
                }
            }

            // Calculate the output position
            index = ((y - 1) * img_data->width + (x - 1)) * 3;
            for (int c = 0; c < 3; c++) {
                raw_data_conv[index + c] = sanitize_pixel(new_pixel[c] + (clammping ? 127 : 0));
            }
        }
    }

    auto pixbuf_conv = create_pixbuf_from_data(raw_data_conv, img_data->width, img_data->height);

    update_pixbuf(pixbuf_conv, program_data);
    resize_working_window(program_data);

    delete[] raw_data_ext; // Clean up allocated memory
    delete[] raw_data_conv;
    g_object_unref(pixbuf_ext);
    g_object_unref(pixbuf_conv);
}

// Function to open the "Convolution Setup" window
void on_convolution_clicked(GtkWidget *button, Program_instance *program_data) {
    const char *kernel_labels[] = {
        "Gaussian",
        "Laplacian",
        "High Pass",
        "Prewitt Hx",
        "Prewitt Hy",
        "Sobel Hx",
        "Sobel Hy"
    };

    const char *kernels[] = {
        "[0.0625 0.125 0.0625, 0.125 0.25 0.125, 0.0625 0.125 0.0625]",
        "[0 -1 0, -1 4 -1, 0 -1 0]",
        "[-1 -1 -1, -1 8 -1, -1 -1 -1]",
        "[-1 0 1, -1 0 1, -1 0 1]",
        "[-1 -1 -1, 0 0 0, 1 1 1]",
        "[-1 0 1, -2 0 2, -1 0 1]",
        "[-1 -2 -1, 0 0 0, 1 2 1]"
    };

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Convolution Setup");

    // Main vertical box layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_widget_set_margin_top(vbox, 15);
    gtk_widget_set_margin_bottom(vbox, 15);

    // Create the text entry and "Convolve" button horizontally
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *kernel_entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(kernel_entry), 40);
    gtk_entry_set_placeholder_text(GTK_ENTRY(kernel_entry), "[1 2 3, 4 5 6, 7 8 9]");
    gtk_box_pack_start(GTK_BOX(hbox), kernel_entry, TRUE, TRUE, 0);
    program_data->kernel_entry = GTK_ENTRY(kernel_entry);

    GtkWidget *convolve_button = gtk_button_new_with_label("Convolve");
    gtk_box_pack_start(GTK_BOX(hbox), convolve_button, FALSE, FALSE, 0);
    g_signal_connect(convolve_button, "clicked", G_CALLBACK(on_convolve_clicked), program_data);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *clampping = gtk_check_button_new_with_label("With clampping");
    gtk_box_pack_start(GTK_BOX(vbox), clampping, FALSE, FALSE, 10);
    program_data->clampping = GTK_TOGGLE_BUTTON(clampping);

    GtkWidget *label_kernels = gtk_label_new("Preset Kernels");
    gtk_label_set_xalign(GTK_LABEL(label_kernels), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), label_kernels, FALSE, FALSE, 10);
    gtk_widget_set_margin_top(label_kernels, 15);

    GtkWidget *preset_hbox;

    for (int i = 0; i < sizeof(kernel_labels) / sizeof(kernel_labels[0]); i++) {
        if (i % 4 == 0) {
            preset_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
            gtk_box_pack_start(GTK_BOX(vbox), preset_hbox, FALSE, FALSE, 0);
        }

        GtkWidget *preset_button = gtk_button_new_with_label(kernel_labels[i]);
        gtk_widget_set_size_request(preset_button, 100, -1);  // Set fixed width, height -1 keeps default
        g_object_set_data(G_OBJECT(preset_button), "entry", kernel_entry);
        g_signal_connect(preset_button, "clicked", G_CALLBACK(set_kernel_text), (gpointer) kernels[i]);
        gtk_box_pack_start(GTK_BOX(preset_hbox), preset_button, TRUE, TRUE, 0);
    }

    // Show all widgets
    gtk_widget_show_all(window);
}

void create_control_window(GtkImage *original_image, GtkImage *working_image, Program_instance *program_data) {
    GtkWidget *control_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(control_window), "Control Window");
    gtk_window_set_default_size(GTK_WINDOW(control_window), INITIAL_WIDTH, INITIAL_HEIGHT);
    gtk_window_set_position(GTK_WINDOW(control_window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(control_window), vbox);
    gtk_widget_set_margin_start(vbox, 5);
    gtk_widget_set_margin_end(vbox, 5);

    // ------------------------- BASIC OPERATIONS --------------------------------
    GtkWidget *label_operations = gtk_label_new("Basic Operations");
    gtk_box_pack_start(GTK_BOX(vbox), label_operations, FALSE, FALSE, 10);

    GtkWidget *open_button = gtk_button_new_with_label("Open Image");
    gtk_box_pack_start(GTK_BOX(vbox), open_button, TRUE, TRUE, 3);

    GtkWidget *save_button = gtk_button_new_with_label("Save Image");
    gtk_box_pack_start(GTK_BOX(vbox), save_button, TRUE, TRUE, 0);

    GtkWidget *start_restart = gtk_button_new_with_label("Start/Restart");
    gtk_box_pack_start(GTK_BOX(vbox), start_restart, TRUE, TRUE, 0);

    // ------------------------- EDITTING AND ADJUSTMENTS SECTION --------------------------------
    GtkWidget *hbox_edit_adjust = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_edit_adjust, TRUE, TRUE, 10);

    // Create a vertical box for Editing
    GtkWidget *vbox_editing = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox_edit_adjust), vbox_editing, TRUE, TRUE, 5);

    GtkWidget *label_editting = gtk_label_new("Editing");
    gtk_box_pack_start(GTK_BOX(vbox_editing), label_editting, FALSE, FALSE, 10);

    GtkWidget *vertical_mirror = gtk_button_new_with_label("Vertical mirror");
    gtk_box_pack_start(GTK_BOX(vbox_editing), vertical_mirror, TRUE, TRUE, 0);

    GtkWidget *horizontal_mirror = gtk_button_new_with_label("Horizontal mirror");
    gtk_box_pack_start(GTK_BOX(vbox_editing), horizontal_mirror, TRUE, TRUE, 0);

    GtkWidget *grayscale = gtk_button_new_with_label("Grayscale");
    gtk_box_pack_start(GTK_BOX(vbox_editing), grayscale, TRUE, TRUE, 0);

    GtkWidget *histogram = gtk_button_new_with_label("Show histogram");
    gtk_box_pack_start(GTK_BOX(vbox_editing), histogram, TRUE, TRUE, 0);

    GtkWidget *negative = gtk_button_new_with_label("Negative");
    gtk_box_pack_start(GTK_BOX(vbox_editing), negative, TRUE, TRUE, 0);

    GtkWidget *equalization = gtk_button_new_with_label("Equalization");
    gtk_box_pack_start(GTK_BOX(vbox_editing), equalization, TRUE, TRUE, 0);

    GtkWidget *rotate_r = gtk_button_new_with_label("Rotate 90° right");
    gtk_box_pack_start(GTK_BOX(vbox_editing), rotate_r, TRUE, TRUE, 0);

    GtkWidget *rotate_l = gtk_button_new_with_label("Rotate 90° left");
    gtk_box_pack_start(GTK_BOX(vbox_editing), rotate_l, TRUE, TRUE, 0);

    GtkWidget *convolution = gtk_button_new_with_label("Convolution");
    gtk_box_pack_start(GTK_BOX(vbox_editing), convolution, TRUE, TRUE, 0);

    // Create a vertical box for Adjustments
    GtkWidget *vbox_adjustments = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox_edit_adjust), vbox_adjustments, TRUE, FALSE, 5);

    GtkWidget *label_adjustments = gtk_label_new("Adjustments");
    gtk_box_pack_start(GTK_BOX(vbox_adjustments), label_adjustments, FALSE, FALSE, 10);

    // Set uniform width for entries and buttons
    const int uniform_width = 15;
    const int button_width = 100;

    // Quantize controls
    GtkWidget *quantize_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox_adjustments), quantize_hbox, FALSE, FALSE, 0);

    GtkWidget *quantize = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(quantize), "N° of tones");
    gtk_entry_set_width_chars(GTK_ENTRY(quantize), uniform_width);
    gtk_box_pack_start(GTK_BOX(quantize_hbox), quantize, FALSE, FALSE, 0);

    GtkWidget *quantize_button = gtk_button_new_with_label("Quantize");
    gtk_widget_set_size_request(quantize_button, button_width, -1);
    gtk_box_pack_start(GTK_BOX(quantize_hbox), quantize_button, FALSE, FALSE, 0);
    program_data->quantize_entry = GTK_ENTRY(quantize);

    // Brightness controls
    GtkWidget *brightness_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox_adjustments), brightness_hbox, FALSE, FALSE, 0);

    GtkWidget *brightness = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(brightness), "Brightness");
    gtk_entry_set_width_chars(GTK_ENTRY(brightness), uniform_width);
    gtk_box_pack_start(GTK_BOX(brightness_hbox), brightness, FALSE, FALSE, 0);
    program_data->brightness_entry = GTK_ENTRY(brightness);

    GtkWidget *brightness_button = gtk_button_new_with_label("Adjust");
    gtk_widget_set_size_request(brightness_button, button_width, -1);
    gtk_box_pack_start(GTK_BOX(brightness_hbox), brightness_button, FALSE, FALSE, 0);

    // Contrast controls
    GtkWidget *contrast_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox_adjustments), contrast_hbox, FALSE, FALSE, 0);

    GtkWidget *contrast = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(contrast), "Contrast");
    gtk_entry_set_width_chars(GTK_ENTRY(contrast), uniform_width);
    gtk_box_pack_start(GTK_BOX(contrast_hbox), contrast, FALSE, FALSE, 0);
    program_data->contrast_entry = GTK_ENTRY(contrast);

    GtkWidget *contrast_button = gtk_button_new_with_label("Adjust");
    gtk_widget_set_size_request(contrast_button, button_width, -1);
    gtk_box_pack_start(GTK_BOX(contrast_hbox), contrast_button, FALSE, FALSE, 0);

    // Zoom controls
    GtkWidget *label_zoom = gtk_label_new("Zoom");
    gtk_box_pack_start(GTK_BOX(vbox_adjustments), label_zoom, FALSE, FALSE, 10);

    GtkWidget *zoom_out_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox_adjustments), zoom_out_hbox, FALSE, FALSE, 0);

    GtkWidget *zoom_out_x = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(zoom_out_x), "x");
    gtk_entry_set_width_chars(GTK_ENTRY(zoom_out_x), (uniform_width / 2) - 1);  // Half-width for zoom out fields
    gtk_box_pack_start(GTK_BOX(zoom_out_hbox), zoom_out_x, FALSE, FALSE, 0);
    program_data->zoom_out_x_entry = GTK_ENTRY(zoom_out_x);

    GtkWidget *zoom_out_y = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(zoom_out_y), "y");
    gtk_entry_set_width_chars(GTK_ENTRY(zoom_out_y), (uniform_width / 2) - 1);  // Half-width for zoom out fields
    gtk_box_pack_start(GTK_BOX(zoom_out_hbox), zoom_out_y, FALSE, FALSE, 0);
    program_data->zoom_out_y_entry = GTK_ENTRY(zoom_out_y);

    GtkWidget *zoom_out_button = gtk_button_new_with_label("Zoom OUT");
    gtk_widget_set_size_request(zoom_out_button, button_width, -1);
    gtk_box_pack_start(GTK_BOX(zoom_out_hbox), zoom_out_button, FALSE, FALSE, 0);

    GtkWidget *zoom_in_button = gtk_button_new_with_label("Zoom in 2x2");
    gtk_widget_set_size_request(zoom_in_button, button_width, -1);
    gtk_box_pack_start(GTK_BOX(vbox_adjustments), zoom_in_button, FALSE, FALSE, 0);

    // Connect the buttons to their respective functions
    g_signal_connect(open_button, "clicked", G_CALLBACK(open_image), program_data);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_image), program_data);
    g_signal_connect(start_restart, "clicked", G_CALLBACK(on_start_restart_clicked), program_data);

    g_signal_connect(vertical_mirror, "clicked", G_CALLBACK(on_vertical_mirror_clicked), program_data);
    g_signal_connect(horizontal_mirror, "clicked", G_CALLBACK(on_horizontal_mirror_clicked), program_data);
    g_signal_connect(grayscale, "clicked", G_CALLBACK(on_grayscale_clicked), program_data);
    g_signal_connect(negative, "clicked", G_CALLBACK(on_negative_button_clicked), program_data);    
    g_signal_connect(histogram, "clicked", G_CALLBACK(on_histogram_clicked), program_data);
    g_signal_connect(equalization, "clicked", G_CALLBACK(on_equalization_clicked), program_data);
    g_signal_connect(rotate_r, "clicked", G_CALLBACK(on_rotate_r_clicked), program_data);
    g_signal_connect(rotate_l, "clicked", G_CALLBACK(on_rotate_l_clicked), program_data);
    g_signal_connect(convolution, "clicked", G_CALLBACK(on_convolution_clicked), program_data);

    g_signal_connect(quantize_button, "clicked", G_CALLBACK(on_quantize_button_clicked), program_data);
    g_signal_connect(brightness_button, "clicked", G_CALLBACK(on_brightness_button_clicked), program_data);
    g_signal_connect(contrast_button, "clicked", G_CALLBACK(on_contrast_button_clicked), program_data);    

    g_signal_connect(zoom_out_button, "clicked", G_CALLBACK(on_zoom_out_clicked), program_data);    
    g_signal_connect(zoom_in_button, "clicked", G_CALLBACK(on_zoom_in_clicked), program_data);
    g_signal_connect(control_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(control_window);
}


