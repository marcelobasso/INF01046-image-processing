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

void create_control_window(GtkImage *original_image, GtkImage *working_image, Program_instance *program_data);
