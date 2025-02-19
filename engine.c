
#include <stdlib.h>

#include "debug.h"
#include "structures.h"

#include "clause.h"
#include "rule.h"
#include "database.h"
#include "engine.h"

///////////////////////////////////////////////////
// Private Functions
int Engine_compile(Engine* instance){
    DBG("Performing Engine compilation...\n");

    // get the number of total rules
    int numberOfCompiledRules = 0;
    for (int i=0; i<instance->numberOfDatabases; i++){
        numberOfCompiledRules += instance->databases[i]->numberOfRules;
    }

    // create space for all rules put together
    instance->compiledRules = malloc(sizeof(Rule*) * numberOfCompiledRules);
    instance->numberOfCompiledRules = numberOfCompiledRules;
    DBG("Detected %d total rules across all databases.\n", numberOfCompiledRules);

    // gather the rules together in the Engine object
    int ruleNumber = 0;
    for (int i=0; i<instance->numberOfDatabases; i++){
        for (int j=0; j<instance->databases[i]->numberOfRules; j++){
            instance->compiledRules[ruleNumber] = instance->databases[i]->rules[j];
            ruleNumber++;
        }
    }

    DBG("All rules gathered.\n");
    
    // create the Matcher for each clause of each rule
    DBG("Creating Matchers for each clause of each rule...\n");
    for (int i=0; i<instance->numberOfCompiledRules; i++){
        for (int j=0; j<instance->compiledRules[i]->numberOfClauses; j++){
            Clause_createMatcher(instance->compiledRules[i]->clauses[j]);
        }
    }

    // save the minimal and maximal metric for each rule
    DBG("Caching the minimal and maximal metrics for each rule...\n");
    for (int i=0; i<instance->numberOfCompiledRules; i++){
        Rule_cacheBestMetrics(instance->compiledRules[i]);
    }


    DBG("Engine compilation finished!\n");
    return 0;
}


///////////////////////////////////////////////////
// Public Functions

// initialize a new Engine
Engine* Engine_init(int numberOfDatabaseFiles, char** databaseFilenames){
    Engine* result = malloc(sizeof(Engine));

    // initialize the database files
    result->numberOfDatabases = numberOfDatabaseFiles;
    result->databases = malloc(sizeof(Database*) * numberOfDatabaseFiles);
    for (int i=0; i<numberOfDatabaseFiles; i++){
        result->databases[i] = Database_init(databaseFilenames[i]);
    }

    // compile the all of the rules
    Engine_compile(result);

    return result;
}


// execute an Engine on an array of tokens
// metric = index of the metric to minimize/maximize
// direction = positive or negative for whether to minimize or maximize
// return an array of strings. (last element is NULL)
char** Engine_execute(Engine* instance, char** tokens, int numberOfTokens, int metric, int direction, int* newLength){
    DBG("---------------------------------------------------\n");
    DBG("Executing Engine on an array of strings...\n");
    char** result = tokens;

    int initialLength = numberOfTokens;

    int substitutionsMade;
    int totalSubstitutions = 0;
    // do not stop until no substitutions were made on a pass
    int currentPass = 1;
    do {
        DBG("+++++++++++++++++++++++++\n");
        DBG("Current Pass: %d\n", currentPass);
        substitutionsMade = 0;
        // iterate through the array of rules in order
        for (int i=0; i<instance->numberOfCompiledRules; i++){
            int substitutions = 0;
            DBG("Executing rule %d/%d... ##############\n", i+1, instance->numberOfCompiledRules);
            result = Rule_execute(instance->compiledRules[i], result, numberOfTokens, metric, direction, &substitutions, &numberOfTokens, 0);
            substitutionsMade += substitutions;
            totalSubstitutions += substitutions;
        }
        currentPass++;
    } while (substitutionsMade != 0);

    *newLength = numberOfTokens;
    DBG("Engine execution finished! (%d total substitutions made)\n", totalSubstitutions);
    DBG("Number of tokens: %d -> %d\n", initialLength, numberOfTokens);
    return result;
}






