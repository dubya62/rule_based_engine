
#include <string.h>
#include <stdlib.h>
#include <float.h>

#include "debug.h"
#include "structures.h"

#include "clause.h"
#include "rule.h"

///////////////////////////////////////////
// Private Functions
int Rule_parse(Rule* instance, char* ruleString){
    DBG("Parsing rule: %s\n", ruleString);
    int n = strlen(ruleString);

    int numberOfClauses = 0;
    Clause** clauses = NULL;
    // parse the rule string into an array of clauses (separated by eq not in ")
    int clauseStart = 0;
    int quotes = 0;
    int backslashes = 0;
    for (int i=0; i<n; i++){
        switch(ruleString[i]){
            case '\\':
                backslashes++;
                break;
            case '=':
                // if not in quotes, parse everything up to this equal sign as a clause
                if (!quotes){
                    ruleString[i] = '\0';
                    numberOfClauses++;
                    clauses = realloc(clauses, sizeof(Clause*) * numberOfClauses);
                    clauses[numberOfClauses-1] = Clause_init(ruleString+clauseStart);
                    clauseStart = i + 1;
                }
                break;
            case '"':
                if (backslashes % 2 == 0){
                    quotes ^= 1;
                }
                break;
        }
        if (ruleString[i] != '\\'){
            backslashes = 0;
        }
    }
    if (clauseStart < n){
        numberOfClauses++;
        clauses = realloc(clauses, sizeof(Clause*) * numberOfClauses);
        clauses[numberOfClauses-1] = Clause_init(ruleString+clauseStart);
    }

    instance->numberOfClauses = numberOfClauses;
    instance->clauses = clauses;

    return 0;
}

// parse a rule string into a Rule object


///////////////////////////////////////////
// Public Functions

// initialize a new Rule
Rule* Rule_init(char* ruleString){
    Rule* result = (Rule*) malloc(sizeof(Rule));

    DBG("Parsing Rule...\n");
    Rule_parse(result, ruleString);

    DBG("Rule fully initialized (%d clauses)!\n", result->numberOfClauses)

    return result;
}

// cache the best metrics for this rule
int Rule_cacheBestMetrics(Rule* instance){
    // get the most number of metrics that any one clause has
    instance->numberOfMetrics = 0;
    for (int i=0; i<instance->numberOfClauses; i++){
        if (instance->clauses[i]->numberOfMetrics > instance->numberOfMetrics){
            instance->numberOfMetrics = instance->clauses[i]->numberOfMetrics;
        }
    }

    if (instance->numberOfMetrics == 0 || instance->numberOfClauses == 0){
        instance->minimalMetric = NULL;
        instance->maximalMetric = NULL;
        return 0;
    }

    instance->minimalMetric = (int*) malloc(sizeof(int) * instance->numberOfMetrics);
    instance->maximalMetric = (int*) malloc(sizeof(int) * instance->numberOfMetrics);

    for (int i=0; i<instance->numberOfMetrics; i++){
        float minimum = FLT_MAX;
        float maximum = FLT_MIN;

        instance->minimalMetric[i] = instance->numberOfClauses-1;
        instance->maximalMetric[i] = instance->numberOfClauses-1;

        for (int j=0; j<instance->numberOfClauses; j++){
            // make sure the metric exists for this clause
            if (instance->clauses[j]->numberOfMetrics > i && instance->clauses[j]->metrics[i] != -1.0){
                float currentMetric = instance->clauses[j]->metrics[i];
                DBG("currentMetric: %f\n", currentMetric);
                if (currentMetric > maximum){
                    maximum = currentMetric;
                    instance->maximalMetric[i] = j;
                } 
                if (currentMetric < minimum){
                    minimum = currentMetric;
                    instance->minimalMetric[i] = j;
                }
            }
        }
    }

    DBG("Best metrics for rule:\n");
    for (int i=0; i<instance->numberOfMetrics; i++){
        DBG("\tMetric %d: min = %d\tmax = %d\n", i, instance->minimalMetric[i], instance->maximalMetric[i]);
    }

    return 0;
}


char** Rule_execute(Rule* instance, char** tokens, int numberOfTokens, int metric, int direction, int* substitutions, int* newNumberOfTokens){
    char** result = tokens;

    if (metric >= instance->numberOfMetrics){
        return result;
    }

    if (direction == 0){
        return result;
    }

    DBG("Attempting to match tokens against each clause...\n");
    // try to match the instance against each clause until a match is found
    for (int i=0; i<instance->numberOfClauses; i++){
    
        // need to get the offset, variable bindings, length
        MatchResult* matchResult = Clause_match(instance->clauses[i], tokens, numberOfTokens);
        if (matchResult == NULL){
            continue;
        }

        // find the best replacement clause

        // substitute the best clause for the current one if the metric is better

    }

    // once a match is found, replace the matched tokens with the optimal tokens for this rule
    
    return result;
}

