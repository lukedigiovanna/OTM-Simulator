#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

struct state {
    char* name;
    bool accepting;
};

enum direction {
    LEFT, RIGHT, HOLD
};

struct transition {
    // input
    char* curr_state_name;
    char input_symbol;
    char tape_symbol;
    // output
    char write_symbol;
    char* destination_state;
    enum direction direction;
};

struct tape {
    char* tape;
    int capacity;
    int curr_pos;
};

static int nstates;
static struct state* states;
static int ntransitions;
static struct transition* transition_table;
static struct tape tape;

static struct state* find_state(const char* name) {
    for (int i = 0; i < nstates; i++) {
        if (!strcmp(name, states[i].name)) {
            return states + i;
        }
    }
    return NULL;
}

static void init_tape(struct tape* tape) {
    tape->capacity = 10;
    tape->tape = malloc(tape->capacity);
    // initialize all to blanks.
    for (int i = 0; i < tape->capacity; i++) {
        tape->tape[i] = '_';
    }
}

static void free_tape(struct tape* tape) {
    free(tape->tape);
}

static char current_tape_symbol(struct tape* tape) {
    return tape->tape[tape->curr_pos];
}

static void move_tape(struct tape* tape, enum direction direction) {
    if (direction == LEFT) {
        if (tape->curr_pos == 0) {
            // then we need to make space on the tape
            // double the size of the tape.
            char* new_tape = malloc(tape->capacity * 2);
            // set the first half to blanks
            for (int i = 0; i < tape->capacity; i++) {
                new_tape[i] = '_';
            }
            // copy contents from second half
            for (int i = tape->capacity; i < tape->capacity * 2; i++) {
                new_tape[i] = tape->tape[i - tape->capacity];
            }
            free(tape->tape); // free the old allocated char array.
            tape->curr_pos = tape->capacity;
            tape->tape = new_tape;
            tape->capacity *= 2;
        }
        tape->curr_pos--;
    }
    else if (direction == RIGHT) {
        if (tape->curr_pos == tape->capacity - 1) {
            char* new_tape = malloc(tape->capacity * 2);
            for (int i = 0; i < tape->capacity; i++) {
                new_tape[i] = tape->tape[i];
            }
            for (int i = tape->capacity; i < tape->capacity * 2; i++) {
                new_tape[i] = '_';
            }
            free(tape->tape);
            tape->tape = new_tape;
            tape->capacity *= 2;
        }
        tape->curr_pos++;
    }
    // if direction is HOLD there is nothing to do.
}

static struct transition* find_transition(const char* curr_state, char input_symbol, char tape_symbol) {
    for (int i = 0; i < ntransitions; i++) {
        struct transition* trans = &transition_table[i];
        if (!strcmp(curr_state, trans->curr_state_name) && input_symbol == trans->input_symbol && tape_symbol == trans->tape_symbol) {
            return trans;
        }
    }

    return NULL;
}

// prints the state and the tape
static void print_state(struct state* state, struct tape* tape) {
    printf("State: %s and is %d\n", state->name, state->accepting);
    for (int i = 0; i < tape->capacity; i++) {
        printf("%c", tape->tape[i]);
        if (i == tape->curr_pos) {
            printf("<");
        }
        else {
            printf(" ");
        }
    }
    printf("\n");
}

// returns the state we end up in
static struct state* iterate_otm(struct state* curr_state, char input_symbol, struct tape* tape) {
    // find the corresponding transition
    char curr_symbol = current_tape_symbol(tape);
    struct transition* trans = find_transition(curr_state->name, input_symbol, curr_symbol);
    if (trans == NULL) {
        return NULL;
    }
    // now manipulate the OTM
    tape->tape[tape->curr_pos] = trans->write_symbol;
    move_tape(tape, trans->direction);
    
    return find_state(trans->destination_state);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./otm <config file> <input string>\n");
        return -1;
    }
    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Could not open config file.\n");
        return 0;
    }

    // expected format
    char* buffer = NULL;
    size_t len;
    getline(&buffer, &len, file);
    nstates = atoi(buffer);
    states = calloc(nstates, sizeof(struct state));
    for (int i = 0; i < nstates; i++) {
        getline(&buffer, &len, file);
        char name[20];
        char acc[20];
        sscanf(buffer, "%s %s", name, acc);
        states[i].name = calloc(strlen(name) + 1, 1);
        strcpy(states[i].name, name);
        if (!strcmp(acc, "accept")) {
            states[i].accepting = true;
        }
        else {
            states[i].accepting = false;
        }
    }
    getline(&buffer, &len, file);
    ntransitions = atoi(buffer);
    transition_table = calloc(ntransitions, sizeof(struct transition));
    for (int i = 0; i < ntransitions; i++) {
        getline(&buffer, &len, file);
        char cur[20], dest[20];
        char input, read, write, dir;
        sscanf(buffer, "%s %c %c %s %c %c", cur, &input, &read, dest, &write, &dir);
        transition_table[i].curr_state_name = calloc(strlen(cur) + 1, 1);
        transition_table[i].destination_state = calloc(strlen(dest) + 1, 1);
        strcpy(transition_table[i].curr_state_name, cur);
        strcpy(transition_table[i].destination_state, dest);
        transition_table[i].input_symbol = input;
        transition_table[i].tape_symbol = read;
        transition_table[i].write_symbol = write;
        if (dir == 'R') transition_table[i].direction = RIGHT;
        else if (dir == 'L') transition_table[i].direction = LEFT;
        else transition_table[i].direction = HOLD;
    }

    getline(&buffer, &len, file);
    char* start_state = buffer;

    fclose(file);

    init_tape(&tape);

    struct state* curr_state = find_state(start_state);
    print_state(curr_state, &tape);
    char* input = argv[2];
    for (int i = 0; i < strlen(input); i++) {
        curr_state = iterate_otm(curr_state, input[i], &tape);
        print_state(curr_state, &tape);
    }

    free_tape(&tape);
    free(states);

    return 0;
}