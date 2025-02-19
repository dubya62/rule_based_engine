#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef struct MatchResult{
    int offset;
    int length;

    // TODO: handle variable bindings
    int numberOfVariables;
    int** variableBindingLengths; // number of Variables length
    char*** variableBindings; // number of Variables length of variableBindings length
} MatchResult;

typedef struct Matcher{
    int* minRepetitions; // minimum number of repetitions for token at this index
    int* maxRepetitions; // maximum number of repetitions for token at this index
    int* variableAccesses; // which variables are accessed for this token (-1 = none)
    int* internalVariables; // which variables are internal (-1 = not)

    int* numberOfMatchingTokens; // 0 = Any, otherwise the lengths of matchingTokens
    char*** matchingTokens; // NULL = Any, otherwise is an array of strings that match
} Matcher;

// A Clause holds an array of Tokens and their metrics
typedef struct Clause{
    int numberOfTokens;
    char** tokens; 

    int numberOfMetrics;
    float* metrics; // metric value of -1 is equivalent to empty

    Matcher* matcher;
} Clause;

// A Rule holds many equivalent clauses
// Rules must be compiled to be able to execute them
typedef struct Rule{
    int numberOfClauses;
    Clause** clauses;

    int numberOfMetrics;
    int* minimalMetric; // for each metric, the clause index of the minimal representation
    int* maximalMetric; // for each metric, the clause index of the maximal representation
} Rule;

// A Database holds an array of Rules
typedef struct Database{
    int numberOfRules;
    Rule** rules;
} Database;


// An Engine holds an array of databases and an array of CompiledRules
typedef struct Engine{
    int internalVariable; // keeps track of the next internal variable
    int numberOfDatabases;
    Database** databases;

    int numberOfCompiledRules;
    Rule** compiledRules; // rules that are ready to execute
} Engine;

#endif
