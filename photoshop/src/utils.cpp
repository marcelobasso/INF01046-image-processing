#include "utils.h"

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

int sanitize_pixel(int pixel) {
    if (pixel > 255) pixel = 255;
    if (pixel < 0) pixel = 0;

    return pixel;
}

int get_luminance(unsigned char *pixel) {
    return pixel[0] * R_WEIGHT + pixel[1] * G_WEIGHT + pixel[2] * B_WEIGHT;
}

void resize_working_window(Program_instance *program_data) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(program_data->working_image));
    int new_width, new_height;
    new_width = gdk_pixbuf_get_width(pixbuf);
    new_height = gdk_pixbuf_get_height(pixbuf);
    gtk_window_resize(GTK_WINDOW(window), new_width, new_height);
    gtk_window_set_default_size(GTK_WINDOW(window), new_width, new_height);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
}

void update_working_image_data(Program_instance *program_data) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(program_data->working_image));
    program_data->img_data.pixbuf = pixbuf;
    program_data->img_data.width = gdk_pixbuf_get_width(pixbuf);
    program_data->img_data.height = gdk_pixbuf_get_height(pixbuf);
    program_data->img_data.rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    program_data->img_data.n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    program_data->img_data.pixels = gdk_pixbuf_get_pixels(pixbuf);
    program_data->img_data.is_grayscale = FALSE;
    program_data->img_data.min = INT_MAX;
    program_data->img_data.max = 0;
}

void draw_histogram(GtkWidget *widget, cairo_t *cr, Image_data *img_data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    int array_size = 256;

    // Calculate max value in the array to scale y-axis
    float max_value = img_data->max_histogram;

    // Dimensions and scaling factors
    float bar_width = width / array_size; // Width for each bar
    float y_scale = ((float) height - 64) / max_value;   // Height scale based on max frequency

    // Draw y-axis and x-axis
    cairo_set_source_rgb(cr, 255, 255, 255);
    cairo_set_line_width(cr, 2.0);

    // Y-axis
    cairo_move_to(cr, 70, 30);  // start of y-axis
    cairo_line_to(cr, 70, height - 30); // end of y-axis
    cairo_stroke(cr);

    // X-axis
    cairo_move_to(cr, 70, height - 30);
    cairo_line_to(cr, width - 10, height - 30);
    cairo_stroke(cr);

    // Draw histogram bars
    cairo_set_source_rgb(cr, 0.2, 0.5, 0.8);
    for (int i = 0; i < array_size; i++) {
        int bar_height = img_data->histogram[i] * y_scale;
        cairo_rectangle(cr, 70 + i * bar_width, height - 30 - bar_height, bar_width, bar_height);
        cairo_fill(cr);
    }

    // Draw x-axis labels (1 to 256)
    cairo_set_source_rgb(cr, 255, 255, 255);
    cairo_set_font_size(cr, 10);
    for (int i = 0; i < 256; i += 15) { // Interval for x-axis labels
        std::string label = std::to_string(i);
        cairo_move_to(cr, 70 + i * bar_width, height - 15);
        cairo_show_text(cr, label.c_str());
    }

    // Draw y-axis labels (proportions)
    int y_intervals = 3;
    for (int i = 0; i <= y_intervals; i++) {
        float proportion = (max_value * i) / y_intervals;
        std::string label = std::to_string(proportion).substr(0, 7);
        cairo_move_to(cr, 10, height - 30 - i * (height - 64) / y_intervals);
        cairo_show_text(cr, label.c_str());
    }
}

GdkPixbuf *create_pixbuf_from_data(unsigned char *raw_data, int new_width, int new_height) {
    return gdk_pixbuf_new_from_data(
        raw_data,               // pixels data
        GDK_COLORSPACE_RGB,     // color system
        FALSE,                  // if has alpha (4 channels)
        8,                      // n of bits per pixel information
        new_width,  
        new_height,
        new_width * 3,
        NULL,       
        NULL
    );
}