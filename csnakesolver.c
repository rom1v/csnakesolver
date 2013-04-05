/*
 * csnakesolver v0.1, 19th october 2011
 *
 * changelog:
 *   0.1
 *     - initial version
 *
 * Solver for generalized snake-cube:
 * http://en.wikipedia.org/wiki/Snake_cube
 * http://fr.wikipedia.org/wiki/Cube_serpent
 * 
 * By Romain Vimont (®om)
 *   rom@rom1v.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define EXEMPLE_R // change with EXEMPLE_M or EXEMPLE_L

#ifdef EXEMPLE_R
#define SNAKE_STRUCTURE {2, 1, 1, 2, 1, 2, 1, 1, 2, 2, 1, 1, 1, 2, 2, 2, 2}
#define STRUCTURE_VECTOR_COUNT 17
#define VOLUME_DIMENSIONS {3, 3, 3}
#define DIMENSIONS_COUNT 3
#endif

#ifdef EXEMPLE_M
#define SNAKE_STRUCTURE {2, 1, 2, 1, 1, 3, 1, 2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 2, 3, 1, 1, 1, 3, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1}
#define STRUCTURE_VECTOR_COUNT 46
#define VOLUME_DIMENSIONS {4, 4, 4}
#define DIMENSIONS_COUNT 3
#endif

#ifdef EXEMPLE_L
#define SNAKE_STRUCTURE {1, 1, 2, 1, 1, 3, 1, 2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, 2, 2, 1, 1, 1, 1, 1, 2, 3, 1, 1, 1, 3, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3}
#define STRUCTURE_VECTOR_COUNT 47
#define VOLUME_DIMENSIONS {4, 4, 4}
#define DIMENSIONS_COUNT 3
#endif

#define VARIABLES {'x', 'y', 'z', 't'}
#define VARIABLES_COUNT 4

int structure[] = SNAKE_STRUCTURE;
int dimensions[] = VOLUME_DIMENSIONS;
int variables[] = VARIABLES;

int structure_length; // sum of SNAKE_STRUCTURE
int volume_size; // product of dimensions

typedef struct vector {
    int position;
    int value;
} vector_s;

typedef struct volume_helper {
    bool * flags; // length = product of all VOLUME_DIMENSIONS items
    vector_s path[STRUCTURE_VECTOR_COUNT];
    int path_length;
    int init_cursor[DIMENSIONS_COUNT];
    int cursor[DIMENSIONS_COUNT];
} volume_helper_s;

typedef struct symmetry_helper {
    int eq_classes_path[DIMENSIONS_COUNT * STRUCTURE_VECTOR_COUNT + 2];
    int * eq_classes;
} symmetry_helper_s;


vector_s new_vector(char position, char value);
void symmetry_helper_init(symmetry_helper_s * symmetry_helper);
void volume_helper_init(volume_helper_s * volume_helper);
void init();

char * vector_to_string(vector_s vector);
char * get_variable(char position);
char * get_canonical_number(char number);

char * cursor_to_string(int cursor[]);

void volume_helper_set_cursor(int cursor[]);
bool * volume_helper_get_flag_pointer(int cursor[]);
bool volume_helper_get_flag(int cursor[]);
void volume_helper_set_flag(int cursor[], bool value);
bool volume_helper_can_move(vector_s vector);
void volume_helper_move(vector_s vector);
void volume_helper_back();
void volume_helper_append_vector_in_path(vector_s vector) ;
vector_s volume_helper_pop_vector_in_path();

void symmetry_helper_init_eq_classes_from_dimensions();
bool symmetry_helper_inc_cursor(int cursor[]);
bool symmetry_helper_inc_cursor_rec(int cursor[], int index);
void symmetry_helper_set_cursor(int cursor[]);
void symmetry_helper_move(vector_s vector);
void symmetry_helper_back();
bool symmetry_helper_must_explore(int i);
bool eq_cmp(int p1, int p2, int dim);

bool solve();
bool solve_rec(int init_cursor[], int step);
bool solution(int * init_cursor, vector_s * path);

vector_s new_vector(char position, char value) {
    vector_s vector;
    vector.position = position;
    vector.value = value;
    return vector;
};

symmetry_helper_s symmetry_helper;
volume_helper_s volume_helper;

void symmetry_helper_init(symmetry_helper_s * symmetry_helper) {
    symmetry_helper->eq_classes = symmetry_helper->eq_classes_path;
}

void volume_helper_init(volume_helper_s * volume_helper) {
    volume_helper->flags = (bool *) malloc(volume_size * sizeof(bool));
    volume_helper->path_length = 0;
}

void init() {
    int i;
    volume_size = 1;
    for (i = 0; i < DIMENSIONS_COUNT; i++) {
        volume_size *= dimensions[i];
    }
    structure_length = 0;
    for (i = 0; i < STRUCTURE_VECTOR_COUNT; i++) {
        structure_length += structure[i];
    }
    symmetry_helper_init(&symmetry_helper);
    volume_helper_init(&volume_helper);
}

char * vector_to_string(vector_s vector) {
    char * string = (char *) malloc(5 * sizeof(char));
    char * variable = get_variable(vector.position);
    char * canonical_number = get_canonical_number(vector.value);
    sprintf(string, "%s%s", canonical_number, variable);
    free(variable);
    free(canonical_number);
    return string;
}

char * get_variable(char position) {
    char * variable = (char *) malloc(3 * sizeof(char));
    if (position < VARIABLES_COUNT) {
        sprintf(variable, "%c", variables[position]);
    } else {
        sprintf(variable, "k%i", position - VARIABLES_COUNT);
    }
    return variable;
}

char * get_canonical_number(char number) {
    char * canonical_number = (char *) malloc(3 * sizeof(char));
    if (number == (char) 1) {
        sprintf(canonical_number, "");
    } else if (number == (char) -1) {
        sprintf(canonical_number, "-");
    } else {
        sprintf(canonical_number, "%i", number);
    }
    return canonical_number;
}

char * cursor_to_string(int cursor[]) {
    char * result = (char *) malloc(255 * sizeof(char));
    char * s = result;
    int i;
    s += sprintf(s, "(");
    for (i = 0; i < DIMENSIONS_COUNT; i++) {
        if (i != 0) {
            s += sprintf(s, ", ");
        }
        s += sprintf(s, "%i", cursor[i]);
    }
    s += sprintf(s, ")");
    return result;
}



void volume_helper_set_cursor(int cursor[]) {
    volume_helper_set_flag(volume_helper.init_cursor, false);
    memcpy(volume_helper.init_cursor, cursor, DIMENSIONS_COUNT * sizeof(int));
    memcpy(volume_helper.cursor, cursor, DIMENSIONS_COUNT * sizeof(int));
    volume_helper_set_flag(cursor, true);
}

bool * volume_helper_get_flag_pointer(int cursor[]) {
    bool * p_flag = volume_helper.flags;
    int product = 1;
    int i;
    for (i = DIMENSIONS_COUNT - 1; i >= 0; i--) {
        p_flag += cursor[i] * product;
        product *= dimensions[i];
    }
    return p_flag;
}

bool volume_helper_get_flag(int cursor[]) {
    return * volume_helper_get_flag_pointer(cursor);
}

void volume_helper_set_flag(int cursor[], bool value) {
    * volume_helper_get_flag_pointer(cursor) = value;
}

bool volume_helper_can_move(vector_s vector) {
    int cursor_position_value = volume_helper.cursor[vector.position];
    int new_value = cursor_position_value + vector.value;
    int future_cursor[DIMENSIONS_COUNT];
    int sign, i, abs_value;
    if (new_value < 0 || new_value >= dimensions[vector.position]) {
        return false;
    }
    memcpy(future_cursor, volume_helper.cursor, DIMENSIONS_COUNT * sizeof(int));
    if (vector.value < 0) {
        sign = -1;
    } else {
        sign = 1;
    }
    abs_value = sign * vector.value;
    for (i = 0; i < abs_value; i++) {
        future_cursor[vector.position] += sign;
        if (volume_helper_get_flag(future_cursor)) {
            return false;        
        }
    }
    return true;
}

void volume_helper_move(vector_s vector) {
    int sign, i, abs_value;
    volume_helper_append_vector_in_path(vector);
    if (vector.value < 0) {
        sign = -1;
    } else {
        sign = 1;
    }
    abs_value = sign * vector.value;
    for (i = 0; i < abs_value; i++) {
        volume_helper.cursor[vector.position] += sign;
        volume_helper_set_flag(volume_helper.cursor, true);
    }
}

void volume_helper_back() {
    int sign, i, abs_value;
    vector_s vector = volume_helper_pop_vector_in_path();
    if (vector.value < 0) {
        sign = -1;
    } else {
        sign = 1;
    }
    abs_value = sign * vector.value;
    for (i = 0; i < abs_value; i++) {
        volume_helper_set_flag(volume_helper.cursor, false);
        volume_helper.cursor[vector.position] += -sign;
    }
}

void volume_helper_append_vector_in_path(vector_s vector) {
    vector_s * current_vector = volume_helper.path + volume_helper.path_length;
    memcpy(current_vector, &vector, sizeof(vector_s));
    volume_helper.path_length ++;
}

vector_s volume_helper_pop_vector_in_path() {
    volume_helper.path_length --;
    vector_s * current_vector = volume_helper.path + volume_helper.path_length;
    vector_s vector;
    memcpy(&vector, current_vector, sizeof(vector));
    return vector;
}

void symmetry_helper_init_eq_classes_from_dimensions() {
    // eq_classes from dimensions is the first item of eq_classes_path
    int * eq_classes = symmetry_helper.eq_classes_path; 
    int i, j;
    int value;
    bool found;
    for (i = 1; i < DIMENSIONS_COUNT; i++) {
        value = dimensions[i];
        j = 0;
        found = false;
        while (j < i && !found) {
            if (dimensions[j] == value) {
                eq_classes[i] = j;
                found = true;
            } else {
                j++;
            }
        }
        if (!found) {
            eq_classes[j] = j;
        }
    }
    symmetry_helper.eq_classes = eq_classes;
}

bool symmetry_helper_inc_cursor(int cursor[]) {
    return symmetry_helper_inc_cursor_rec(cursor, DIMENSIONS_COUNT - 1);
}

bool symmetry_helper_inc_cursor_rec(int cursor[], int index) {
    int * eq_classes = symmetry_helper.eq_classes_path;
    int value = cursor[index];
    int i;
    if (value < (dimensions[index] - 1) / 2) {
        // the last coordinate can be incremented
        cursor[index] ++;
        return true;
    }
    // we must increment recursively the previous coordinate
    if (index == 0) {
        // there is no more coordinate to increment
        return false;
    }
    i = index - 1;
    if (!symmetry_helper_inc_cursor_rec(cursor, i)) {
        return false;
    }
    while (i >= 0 && eq_classes[i] != eq_classes[index]) {
        i--;
    }
    if (i >= 0) {
        // coordinate value must at least equals the previous coordinates
        // in the same equivalence class
        cursor[index] = cursor[i];
    } else {
        cursor[index] = 0;
    }
    return true;
}

void symmetry_helper_set_cursor(int cursor[]) {
    int * eq_classes_path = symmetry_helper.eq_classes_path;
    int * cursor_eq_classes = symmetry_helper.eq_classes_path + DIMENSIONS_COUNT;
    int i, j, old_class;
    
    symmetry_helper.eq_classes = cursor_eq_classes;
    // copy the eq_classes computed from the dimensions into the next segment
    memcpy(cursor_eq_classes, eq_classes_path, DIMENSIONS_COUNT * sizeof(int));
    for (i = 0; i < DIMENSIONS_COUNT; i++) {
        if (cursor_eq_classes[i] != i && !eq_cmp(cursor_eq_classes[i], cursor[i], dimensions[i])) {
            old_class = cursor_eq_classes[i];
            cursor_eq_classes[i] = i;
            for (j = i + 1; j < DIMENSIONS_COUNT; j++) {
                if (cursor_eq_classes[j] == old_class) {
                    cursor_eq_classes[j] = i;
                }
            }
        }
    }
}

void symmetry_helper_move(vector_s vector) {
    int position = vector.position;
    int * previous_eq_classes = symmetry_helper.eq_classes;
    int * new_eq_classes = previous_eq_classes + DIMENSIONS_COUNT;
    
    int new_eq_class = -1;
    int i;
    memcpy(new_eq_classes, previous_eq_classes, DIMENSIONS_COUNT * sizeof(int));
    for (i = position + 1; i < DIMENSIONS_COUNT; i++) {
        if (new_eq_classes[i] == position) {
            if (new_eq_class == -1) {
                new_eq_class = i;
            }
            new_eq_classes[i] = new_eq_class;
        }
    }
    symmetry_helper.eq_classes = new_eq_classes;
}

void symmetry_helper_back() {
    symmetry_helper.eq_classes -= DIMENSIONS_COUNT;
}

bool symmetry_helper_must_explore(int i) {
    return symmetry_helper.eq_classes[i] == i;
}

bool eq_cmp(int p1, int p2, int dim) {
    return p1 == p2 || p1 + p2 + 1 == dim;
}

bool solve() {
    int cursor[DIMENSIONS_COUNT] = {}; // init with zeros
    int i;
    if (structure_length + 1 != volume_size) {
        fprintf(stderr,
                "Structure has not the right length (%i instead of %i)\n",
                structure_length, volume_size);
        return false;
    }
    
    do {
        volume_helper_set_cursor(cursor);
        symmetry_helper_set_cursor(cursor);
        if (!solve_rec(cursor, 0)) {
            return false;
        }
    } while (symmetry_helper_inc_cursor(cursor));
    
    // explored all possible solutions
    return true;
}

bool solve_rec(int init_cursor[], int step) {
    int previous_position;
    int norm = structure[step];
    int vector_value;
    int i, k;
    vector_s possible_vector;
    if (step == STRUCTURE_VECTOR_COUNT) {
        if (!solution(volume_helper.init_cursor, volume_helper.path)) {
            // stop searching for new solutions
            return false;
        }   
    } else {
        if (volume_helper.path_length == 0) {
            previous_position = -1;
        } else {
            previous_position = volume_helper.path[volume_helper.path_length -1].position;
        }
        for (i = 0; i < DIMENSIONS_COUNT; i++) {
            if (i != previous_position && symmetry_helper_must_explore(i)) {
                for (k = 0; k < 2; k++) {
                    vector_value = k == 0 ? norm : -norm;
                    possible_vector = new_vector(i, vector_value);
                    if (volume_helper_can_move(possible_vector)) {
                        volume_helper_move(possible_vector);
                        symmetry_helper_move(possible_vector);
                        if (!solve_rec(init_cursor, step + 1)) {
                            return false;
                        }
                        volume_helper_back();
                        symmetry_helper_back();
                    }
                }
            }
        }
    }
    return true;
}

bool solution(int * init_cursor, vector_s * path) {
    int i;
    char * vector_string;
    vector_s vector;
    printf("(%s, [", cursor_to_string(init_cursor));
    for (i = 0; i < STRUCTURE_VECTOR_COUNT; i++) {
        if (i != 0) {
            printf(", ");
        }
        vector = path[i];
        vector_string = vector_to_string(vector);
        printf("%s", vector_string);
        free(vector_string);
    }
    printf("])\n");
    // stop after the first solution
    return false;
}

int main(void) {
    init();
    solve();
    exit(0);
}
