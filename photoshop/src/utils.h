#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>   
#include <cstring>
#include <math.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <gtk/gtk.h>

#define R_WEIGHT 0.299
#define G_WEIGHT 0.587
#define B_WEIGHT 0.114

/// @brief Saves important image data such as width and height. Facilitates the passage of parameters.
typedef struct {
    int width, height, rowstride, n_channels, min = INT_MAX, max = 0, n_of_pixels;
    bool is_grayscale = FALSE;
    float histogram[256] = {0}, max_histogram = 0;
    unsigned char *pixels;
    GdkPixbuf *pixbuf;
} Image_data;

/// @brief Struct to keep program execution data saved. Facilitate the passage of shared informations.
typedef struct {
    char *current_image_path = NULL;
    GtkWidget *original_image, *working_image, *image_window, *work_image_window;
    GtkEntry *quantize_entry, *brightness_entry, *contrast_entry;
    GtkEntry *zoom_out_x_entry, *zoom_out_y_entry, *kernel_entry;
    GtkToggleButton *clampping;
    Image_data img_data;
} Program_instance;

/// @brief giving the file_path, slices edges and return only the filename
/// @param file_path 
/// @return gchar* filename
gchar* get_filename(gchar *file_path);

/// @brief asserts pixel in the interval [0..255]
/// @param (int) pixel 
/// @return int - pixel value corrected, if needed
int sanitize_pixel(int pixel);

/// @brief calculates the luminance of the pixel based on defined constansts
/// @param *pixel 
/// @return int - new pixel value 
int get_luminance(unsigned char *pixel);

/// @brief resize working_window after the image is resized (like in zoom or rotation)
/// @param *program_data 
void resize_working_window(Program_instance *program_data);

/// @brief updates working image data inside the program_data instance
/// @param *program_data 
void update_working_image_data(Program_instance *program_data);

/// @brief draws the histogram using GTK and cairo functions
/// @param widget 
/// @param cr 
/// @param img_data;
void draw_histogram(GtkWidget *widget, cairo_t *cr, Image_data *img_data);

/// @brief given raw pixel data, creates a new pixbuf instance
/// @param raw_data 
/// @param new_width 
/// @param new_height 
/// @return GdkPixbuf* - a pixbuf constructed from the raw data
GdkPixbuf *create_pixbuf_from_data(unsigned char *raw_data, int new_width, int new_height);