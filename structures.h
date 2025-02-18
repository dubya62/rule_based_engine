#ifndef STRUCTURES_H
#define STRUCTURES_H

// A Clause holds an array of Tokens and their metrics
typedef struct Clause{
    char** tokens; 
    float* metrics; // metric value of -1 is equivalent to empty
} Clause;

// A Rule holds many equivalent clauses
typedef struct Rule{
    Clause** clauses;
} Rule;

// A Database holds an array of Rules
typedef struct Database{
    Rule** rules;
} Database;

// An Engine holds an array of databases
typedef struct Engine{
    Database** databases;
} Engine;

#endif
