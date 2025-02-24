
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


// TODO: make this replace the variables and such
char** createReplacementString(MatchResult* matchResult, Clause* matchedClause, char** tokens, int numberOfTokens, Clause* bestClause, int* resultLength){

    DBG("Creating replacement String\n");
    DBG("numberOfVariables = %d\n", matchResult->numberOfVariables);
    for (int i=0; i<matchResult->numberOfVariables; i++){
        DBG("\tvariable binding length = %d\n", matchResult->variableBindingLengths[i]);
        DBG("\t\t");
        for (int j=0; j<matchResult->variableBindingLengths[i]; j++){
            DBG("%s, ", matchResult->variableBindings[i][j]);
        }
        DBG("\n");
    }

    // check the variable bindings to see how large each is
    int replacementLength = 0;
    for (int i=0; i<bestClause->numberOfTokens; i++){
        int variableAccess = bestClause->matcher->variableAccesses[i];
        if (variableAccess != -1){
            // TODO: get the length of the variable binding
            if (matchResult->numberOfVariables > variableAccess){
                if (matchResult->variableBindingLengths[variableAccess] != -1){
                    replacementLength += matchResult->variableBindingLengths[variableAccess];
                }
            }
        } else {
            replacementLength++;
        }
    }

    DBG("Replacment's length = %d\n", replacementLength);



    char** replacement = (char**) malloc(sizeof(char*) * replacementLength);
    *resultLength = replacementLength;

    DBG("Creating replacement string...\n");
    int replacementIndex = 0;
    for (int i=0; i<bestClause->numberOfTokens; i++){
        // replace with variable value
        int variableAccess = bestClause->matcher->variableAccesses[i];
        DBG("%d: variableAccess = %d\n", i, variableAccess);
        if (variableAccess != -1){
            DBG("This is bound to a variable...\n");
            if (matchResult->numberOfVariables > variableAccess){
                DBG("Length of binding: %d\n", matchResult->variableBindingLengths[variableAccess]);
                if (matchResult->variableBindingLengths[variableAccess] > 0){
                    for (int j=0; j < matchResult->variableBindingLengths[variableAccess]; j++){
                        replacement[replacementIndex] = matchResult->variableBindings[variableAccess][j];
                        replacementIndex++;
                    }
                }
            }
        } else {
            DBG("Not bound to variable...\n");
            replacement[replacementIndex] = bestClause->tokens[i];
            replacementIndex++;
        }
    }

    DBG("Finished creating replacement string.\n");




    return replacement;
}


char** Rule_execute(Rule* instance, char** tokens, int numberOfTokens, int metric, int direction, int* substitutions, int* newNumberOfTokens, int startOffset, int startingClause){
    char** result = tokens;

    if (metric >= instance->numberOfMetrics){
        return result;
    }

    if (direction == 0){
        return result;
    }

    if (startOffset >= numberOfTokens){
        DBG("Reached end of tokens for this rule...\n");
        return result;
    }

    DBG("Attempting to match tokens against each clause...\n");
    // try to match the instance against each clause until a match is found
    
    for (int i=startingClause; i<instance->numberOfClauses; i++){
        DBG("Attempting to match with clause %d\n", i);
    
        // need to get the offset, variable bindings, length
        MatchResult* matchResult = Clause_match(instance->clauses[i], tokens, numberOfTokens, startOffset);
        if (matchResult == NULL){
            continue;
        }
        DBG("Found a matching clause. Finding the best replacement...\n");
        DBG("MatchResult information:\n");
        DBG("\toffset = %d\n\tlength = %d\n", matchResult->offset, matchResult->length);

        // find the best replacement clause
        int bestClause;
        if (direction < 0){
            bestClause = instance->minimalMetric[metric];
        } else {
            bestClause = instance->maximalMetric[metric];
        }

        if (bestClause == i){
            DBG("Already at the best clause... No substitution needed.\n");
            *newNumberOfTokens = numberOfTokens;
            return Rule_execute(instance, result, numberOfTokens, metric, direction, substitutions, newNumberOfTokens, matchResult->offset + matchResult->length, i+1);
        }

        // make sure the best metric is actually better
        if (direction < 0){
            if (instance->clauses[i]->numberOfMetrics > i && instance->clauses[bestClause]->metrics[metric] > instance->clauses[i]->metrics[metric]){
                DBG("Already at best clause... No substitution needed.\n");
                *newNumberOfTokens = numberOfTokens;
                return Rule_execute(instance, result, numberOfTokens, metric, direction, substitutions, newNumberOfTokens, matchResult->offset + matchResult->length, i+1);
            }
        } else {
            if (instance->clauses[i]->numberOfMetrics > i && instance->clauses[bestClause]->metrics[metric] < instance->clauses[i]->metrics[metric]){
                DBG("Already at best clause... No substitution needed.\n");
                *newNumberOfTokens = numberOfTokens;
                return Rule_execute(instance, result, numberOfTokens, metric, direction, substitutions, newNumberOfTokens, matchResult->offset + matchResult->length, i+1);
            }
        }

        // substitute the best clause for the current one if the metric is better
        DBG("Substitution needed...\n");

        Clause* bestClauseData = instance->clauses[bestClause];

        // create the replacement string
        int replacementLength;
        char** replacementString = createReplacementString(matchResult, instance->clauses[i], tokens, numberOfTokens, bestClauseData, &replacementLength);

        int newLength = numberOfTokens - matchResult->length + replacementLength;
        *newNumberOfTokens = newLength;
        DBG("Number of tokens: %d -> %d\n", numberOfTokens, newLength);

        char** substituted = malloc(sizeof(char*) * newLength);
        *substitutions += 1;

        int currentSpot = 0;
        for (int j=0; j<matchResult->offset; j++){
            substituted[currentSpot] = tokens[j];
            currentSpot++;
        }
        for (int j=0; j<replacementLength; j++){
            substituted[currentSpot] = replacementString[j];
            currentSpot++;
        }
        for (int j=matchResult->offset + matchResult->length; j<numberOfTokens; j++){
            substituted[currentSpot] = tokens[j];
            currentSpot++;
        }

        DBG("New tokens:\n\t");
        for (int j=0; j<newLength; j++){
            DBG("%s, ", substituted[j]);
        }
        DBG("\n");


        return Rule_execute(instance, substituted, newLength, metric, direction, substitutions, newNumberOfTokens, matchResult->offset + matchResult->length, 0);
    }
    
    *newNumberOfTokens = numberOfTokens;
    return result;
}

