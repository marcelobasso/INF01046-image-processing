#include "core.h"

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    // centralizes shared information
    Program_instance program_data;

    // Create the main window to display the image
    program_data.image_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(program_data.image_window), "Original Image");
    gtk_window_set_default_size(GTK_WINDOW(program_data.image_window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(program_data.image_window), GTK_WIN_POS_CENTER);

    // Create second window for editting
    program_data.work_image_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(program_data.work_image_window), "Working Image");
    gtk_window_set_default_size(GTK_WINDOW(program_data.work_image_window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(program_data.work_image_window), GTK_WIN_POS_CENTER);

    // Create an image widget and add it to the image window
    program_data.original_image = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(program_data.image_window), program_data.original_image);
    program_data.working_image = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(program_data.work_image_window), program_data.working_image);

    // Connect the destroy signal to close the application
    g_signal_connect(program_data.image_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(program_data.work_image_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // show windows
    gtk_widget_show_all(program_data.image_window);
    gtk_widget_show_all(program_data.work_image_window);
    create_control_window(GTK_IMAGE(program_data.original_image), GTK_IMAGE(program_data.working_image), &program_data);

    gtk_main();
    return 0;
}
