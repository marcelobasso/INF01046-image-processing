#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <gtk/gtk.h>

#define R_WEIGHT 0.299
#define G_WEIGHT 0.587
#define B_WEIGHT 0.114
#define INITIAL_WIDTH 200
#define INITIAL_HEIGHT 100

typedef struct img_data {
    int width, height, rowstride, n_channels;
    unsigned char *pixels;
} Image_data;

typedef struct program_instance {
    char *current_image_path = NULL;
    GtkWidget *original_image, *working_image, *image_window, *work_image_window;
    GtkEntry *quantize_entry;
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
