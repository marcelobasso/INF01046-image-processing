#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>   
#include <cstring>
#include <math.h>
#include <vector>
#include <algorithm>

#ifndef UTILS
#define UTILS
#include "utils.h"

#endif

#define INITIAL_WIDTH 200
#define INITIAL_HEIGHT 100
#define ROTATE_RIGHT 0
#define ROTATE_LEFT 1

#define GAUSSIAN 0
#define LAPLACIAN 1
#define HIGH_PASS 2
#define PREWITT_HX 3
#define PREWITT_HY 4
#define SOBEL_HX 5
#define SOBRE_HY 6

void create_control_window(GtkImage *original_image, GtkImage *working_image, Program_instance *program_data);
