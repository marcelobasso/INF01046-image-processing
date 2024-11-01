#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>   
#include <cstring>
#include <math.h>
#include <vector>
#include <algorithm>
#include <gtk/gtk.h>

#define R_WEIGHT 0.299
#define G_WEIGHT 0.587
#define B_WEIGHT 0.114
#define INITIAL_WIDTH 200
#define INITIAL_HEIGHT 100

typedef struct {
    int width, height, rowstride, n_channels, min = INT_MAX, max = 0, n_of_pixels;
    bool is_grayscale = FALSE;
    float histogram[256] = {0}, max_histogram = 0;
    unsigned char *pixels;
    GdkPixbuf *pixbuf;
} Image_data;

typedef struct {
    char *current_image_path = NULL;
    GtkWidget *original_image, *working_image, *image_window, *work_image_window;
    GtkEntry *quantize_entry, *brightness_entry, *contrast_entry;
    GtkEntry *zoom_out_x_entry, *zoom_out_y_entry;
    Image_data img_data;
} Program_instance;


void open_image(GtkWidget *button, Program_instance *program_data);
void save_image(GtkWidget *button, Program_instance *program_data);
void on_quantize_button_clicked(GtkWidget *button, Program_instance *program_data);
void on_start_restart_clicked(GtkWidget *button, Program_instance *program_data);
void on_vertical_mirror_clicked(GtkWidget *button, Program_instance *program_data);
void on_horizontal_mirror_clicked(GtkWidget *button, Program_instance *program_data);
void on_grayscale_clicked(GtkWidget *button, Program_instance *program_data);
void on_histogram_clicked(GtkWidget *button, Program_instance *program_data);

void create_control_window(GtkImage *original_image, GtkImage *working_image, Program_instance *program_data);
