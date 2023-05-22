#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "integer_stack.h"

int yyerror(const char *s);

int ismodifiable = 0;                // 0: normal variable, 1: constant
int blockCurrentIdentifier = 1;      // default the main file (functions must be defined in block 0)
int blockCounter = 1;                // count the number of total blocks defined and stored
int variableCounter = 0;             // count the number of total variables stored
int typeIdentifier = -1;             // -1: uninitialized, 0: int, 1: float, 2: char, 3: string, 4: boolean, 5: enum
int varOrFunctionVarIndetifier = -1; // -1: uninitialized, 0: variable, 1: constant variable, 2: function type, 3: enum type enum x {dsdasdasdsadsad asd asdasd}, 4: constant, 5: unresolvable
int registerCounter = 0;
int tempCounter = 0;

// 0: int, 1: float, 2: char, 3: string, 4: boolean, 5: enum  // for declaring function
int functionArgumentTypesArray[MAX_SIZE];
int functionArgumentTypesCounter;

// for calling function
int callArgumentTypesArray[MAX_SIZE];
int callArgumentTypesCounter;

// for enums
char *enumTypeValuesArray[MAX_SIZE];
int enumTypeValuesounter;

int labelCounter = 0;  // LABEL 0
int labelCounter2 = 0; // ENDIFLABEL0

int openForLoopCounter = 0;  // OPENFORLOOPLABEL0
int closeForLoopCounter = 0; // CLOSEFORLOOPLABEL0

int openWhileLoopCounter = 0;  // OPENWHILELOOPLABEL0
int closeWhileLoopCounter = 0; // CLOSEWHILELOOPLABEL0

int openRepeatUntilCounter = 0; // OPENREPEATUNTILLABEL0

int caseLabelCounter = 0;        // CASELABEL0
int closeSwitchLabelCounter = 0; // CLOSESWITCHLABEL0
int defaultCaseCounter = 0;      // DEFAULTCASE0

int endLabelCounter = 0; // ENDLABEL0

FILE *outputFile; // for writing to a file
const char *quadFileName = "quadruples.txt";

FILE *outputFile2; // for writing to a symbol table
const char *symFileName = "symboltable.txt";

// for int and float operations to prevent duplicates
int uniqueIdArray[10000];
// counter for the array for the for loops that will be reset after
int uniqueIdArrayCounter = 0;
// the unique id that will be assigned to each new variable
int uniqueIdCounter = 0;

typedef struct
{
    // -1: uninitialized, 0: variable, 1: constant variable, 2: function type, 3: enum type enum x {dsdasdasdsadsad asd asdasd}, 4: constant, 5: unresolvable
    int varOrFunctionVar;
    // -1: uninitialized, 0: int, 1: float, 2: char, 3: string, 4: boolean, 5: enum
    int variableType;

    char *variableId; // id of the variable

    int intValue;      // DEFAULT: -INFINITY
    double floatValue; // DEFAULT: -INFINITY
    char charValue;    // DEFAULT: '\0'
    char *stringValue; // DEFAULT: NULL
    int boolValue;     // DEFAULT: -1
    char *enumValue;   // DEFAULT: NULL
    int variableValue;

    // for enum type
    char *emumInitialization[100]; // DEFAULT: NULL
    int enumInitializationCounter; // DEFAULT: -1

    // if the variable could not be resolved in compile time
    char *functionVariableId;

    // if it is function type to store function argument variable types
    int functionArgumentTypesArray[100];
    int functionArgumentTypesCounter;

    // the line number of the variable declaration
    int declaredLineNo; // -1

    // block number
    int blockParent; // -1

    // talsem el float wel int
    int uniqueId;

    char *enumParent;

} VariableEntry;

VariableEntry SymbolTableVariables[MAX_SIZE];

typedef struct
{
    // -1: uninitialized, 0: normal blocks 1: function blocks, 3: if-else block, 4: switch block,
    //  5: while block, 6: repeat untill block, 7: for block
    int blockType;
    int blockLevel;       // -1: uninitialized
    int parentBlockLevel; // -1: uninitialized
} BlockEntry;

BlockEntry BlockArray[MAX_SIZE];

typedef struct
{
    char *comparisonName;
    int comparisonResult;
} ComparisonEntry;

void printStack()
{
    push(&blockStack, 10);
    push(&blockStack, 20);
    push(&blockStack, 30);

    displayStack(&blockStack); // Stack elements: 30 20 10

    printf("Top element: %d\n", peek(&blockStack)); // Top element: 30

    printf("Popped element: %d\n", pop(&blockStack)); // Popped element: 30

    displayStack(&blockStack); // Stack elements: 20 10
}

// ====================================== LOGIC ========================================

// === Block Logic ====
int addBlock(int blockType, int currentBlockLevel)
{
    BlockEntry tempBlock;
    tempBlock.blockType = blockType;
    tempBlock.blockLevel = blockCounter;
    tempBlock.parentBlockLevel = currentBlockLevel;

    BlockArray[blockCounter] = tempBlock;
    int returnBlockCounter = blockCounter;
    blockCounter++;

    return blockCounter;
}

// === REST ===

int checkIfUniqueIdExists(int uniqueId)
{
    for (int i = 0; i < uniqueIdArrayCounter; i++)
    {
        if (uniqueIdArray[i] == uniqueId)
        {
            return 1;
        }
    }
    return 0;
}

void addUniqueIdtoUniqueIdArray(int uniqueId)
{
    uniqueIdArray[uniqueIdArrayCounter] = uniqueId;
    uniqueIdArrayCounter++;
}

char *combineNumberWithString(int number, const char *prefix)
{
    // Determine the length of the resulting string
    int length = snprintf(NULL, 0, "%s%d", prefix, number);

    // Allocate memory for the string
    char *result = (char *)malloc((length + 1) * sizeof(char));

    // Combine the prefix string and number into the resulting string
    snprintf(result, length + 1, "%s%d", prefix, number);

    return result;
}

void incrementRegister()
{
    if (registerCounter == 6)
    {
        registerCounter = 0;
    }
    else
    {
        registerCounter++;
    }
}

void incrementTemp()
{
    if (tempCounter == 6)
    {
        tempCounter = 0;
    }
    else
    {
        tempCounter++;
    }
}

VariableEntry createEmptyVariableEntry()
{
    VariableEntry emptyVariable = {
        .varOrFunctionVar = -1,
        .variableType = -1,
        .variableId = NULL,
        .intValue = INT_MIN,
        .floatValue = -INFINITY,
        .charValue = '\0',
        .stringValue = NULL,
        .boolValue = -1,
        .enumValue = NULL,
        .enumInitializationCounter = -1,
        .functionArgumentTypesCounter = -1,
        .declaredLineNo = -1,
        .blockParent = -1,
        .uniqueId = uniqueIdCounter,
        .enumParent = NULL};

    uniqueIdArrayCounter++;
    return emptyVariable;
}

int getVariableId(char *variable)
{
    for (int i = 0; i < variableCounter; i++)
    {
        if (SymbolTableVariables[i].variableId != NULL && strcmp(SymbolTableVariables[i].variableId, variable) == 0)
        {
            return i;
        }
    }

    return -1;
}

int checkvarOrFunctionVarValue(char *variable)
{
    if (variable == NULL)
    {
        perror("NULL INPUT AT checkvarOrFunctionVarValue()\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < variableCounter; i++)
    {
        if (SymbolTableVariables[i].variableId != NULL && strcmp(SymbolTableVariables[i].variableId, variable) == 0)
        {
            return SymbolTableVariables[i].varOrFunctionVar;
        }
    }

    return -1;
}

// check if the variable name is already taken by functions as the functions in our
//    language are global and can only be declared global (block level 0)
int findFunction(char *variable)
{
    for (int i = 0; i < variableCounter; i++)
    {
        if (variable != NULL && SymbolTableVariables[i].variableId != NULL && strcmp(SymbolTableVariables[i].variableId, variable) == 0 && SymbolTableVariables[i].varOrFunctionVar == 2)
        {
            return i;
        }
    }
    return -1;
}

// find the variable in the symbol table and return it, else return an empty variable entry
VariableEntry findVariable(char *variable, int currentBlock)
{
    for (int jj = blockStack.top; jj >= 0; jj--)
    {
        for (int i = 0; i < variableCounter; i++)
        {
            if (strcmp(SymbolTableVariables[i].variableId, variable) == 0 && SymbolTableVariables[i].blockParent == blockStack.array[jj])
            {
                return SymbolTableVariables[i];
            }
        }
    }
    // create a struct with empty values
    VariableEntry emptyVariable = createEmptyVariableEntry();
    return emptyVariable;
}

int checkVariableTypeValue(char *variable, int currentBlock)
{
    VariableEntry tempVariable = findVariable(variable, currentBlock);
    if (tempVariable.variableId != NULL)
    {
        return tempVariable.variableType;
    }

    return -1;
}

// int x;
// int y;
// int x;
// int x = 5;
// typeOfOperation: 0: create new variable, 1: create and assign, 2: assign value, 3: create new function, 4: create new enum type, 5: create and assign enum value, 6: assign enum type)

// insert variable in the symbol table
void insertVariable(int typeOfOperation, char *variable, int variableOrFunction, int variableType, VariableEntry entry, int declaredLineNo, int currentBlock)
{
    // check if the variable exists
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    if (typeOfOperation == 0 || typeOfOperation == 1) // create new variable or create and assign new value
    {
        //  printf("here2: 213213123\n"); /// !!!!!!!!!
        // variable already exists at this block level or is the name of a function
        if (tempVariable.variableId != NULL || findFunction(tempVariable.variableId) != -1)
        {
            // throw error
            fprintf(stderr, "Error at %d: redeclaration of variable %s\n", declaredLineNo, variable);
            exit(EXIT_FAILURE);
        }
        else
        {

            // declare the variable

            // check if the variable is unresolvable //variableType == 0 && // for int function
            if (entry.varOrFunctionVar == 2 || entry.varOrFunctionVar == 5)
            {
                // printf("here fccc \n");
                tempVariable.varOrFunctionVar = 5;
                tempVariable.functionVariableId = strdup(entry.functionVariableId);
            }
            else if (ismodifiable == 1) // constant variable
            {
                tempVariable.varOrFunctionVar = 1;
            }
            else
            {
                tempVariable.varOrFunctionVar = 0;
            }

            tempVariable.variableType = variableType;
            tempVariable.variableId = strdup(variable);
            tempVariable.intValue = entry.intValue;
            tempVariable.floatValue = entry.floatValue;
            tempVariable.charValue = entry.charValue;
            tempVariable.stringValue = entry.stringValue;
            tempVariable.boolValue = entry.boolValue;
            tempVariable.enumValue = entry.enumValue;
            tempVariable.enumInitializationCounter = -1;
            tempVariable.declaredLineNo = declaredLineNo;
            tempVariable.blockParent = currentBlock;

            SymbolTableVariables[variableCounter] = tempVariable;
            variableCounter++;
        }
    }
    else if (typeOfOperation == 3) // create a new function
    {
        // variable already exists at this block level or is the name of a function
        if (tempVariable.variableId != NULL || findFunction(tempVariable.variableId) != -1)
        {
            // throw error
            fprintf(stderr, "Error at %d: redeclaration of variable %s\n", declaredLineNo, variable);
            exit(EXIT_FAILURE);
        }
        // functions must be created in level 0
        if (currentBlock != 0)
        {
            fprintf(stderr, "error at %d: Functions must be created in the global scope\n", declaredLineNo);
            exit(EXIT_FAILURE);
        }

        tempVariable.variableType = variableType;
        tempVariable.variableId = strdup(variable);
        tempVariable.intValue = entry.intValue;
        tempVariable.floatValue = entry.floatValue;
        tempVariable.charValue = entry.charValue;
        tempVariable.stringValue = entry.stringValue;
        tempVariable.boolValue = entry.boolValue;
        tempVariable.enumValue = entry.enumValue;
        tempVariable.enumInitializationCounter = -1;
        tempVariable.declaredLineNo = declaredLineNo;
        tempVariable.blockParent = currentBlock;

        // fill the function arguments
        tempVariable.varOrFunctionVar = 2; // function type
        tempVariable.functionVariableId = strdup(variable);

        tempVariable.functionArgumentTypesCounter = 0;
        for (int i = 0; i < functionArgumentTypesCounter; i++)
        {
            tempVariable.functionArgumentTypesArray[tempVariable.functionArgumentTypesCounter] = functionArgumentTypesArray[i];
            tempVariable.functionArgumentTypesCounter++;
        }

        SymbolTableVariables[variableCounter] = tempVariable;
        variableCounter++;
    }
    else if (typeOfOperation == 4) // create new enum type
    {
        // variable already exists at this block level or is the name of a function
        if (tempVariable.variableId != NULL || findFunction(tempVariable.variableId) != -1)
        {
            // throw error
            fprintf(stderr, "Error at %d: redeclaration of variable %s\n", declaredLineNo, variable);
            exit(EXIT_FAILURE);
        }
        // ENUM types must be created in level 0
        if (currentBlock != 0)
        {
            fprintf(stderr, "error at %d: ENUM types must be created in the global scope\n", declaredLineNo);
            exit(EXIT_FAILURE);
        }

        tempVariable.variableType = variableType;
        tempVariable.variableId = strdup(variable);
        tempVariable.intValue = entry.intValue;
        tempVariable.floatValue = entry.floatValue;
        tempVariable.charValue = entry.charValue;
        tempVariable.stringValue = entry.stringValue;
        tempVariable.boolValue = entry.boolValue;
        tempVariable.enumValue = entry.enumValue;
        tempVariable.declaredLineNo = declaredLineNo;
        tempVariable.blockParent = currentBlock;
        tempVariable.varOrFunctionVar = -1;

        tempVariable.enumInitializationCounter = 0;
        tempVariable.varOrFunctionVar = 3; // enum type

        for (int i = 0; i < enumTypeValuesounter; i++)
        {
            tempVariable.emumInitialization[tempVariable.enumInitializationCounter] = strdup(enumTypeValuesArray[i]);
            tempVariable.enumInitializationCounter++;
        }

        SymbolTableVariables[variableCounter] = tempVariable;
        variableCounter++;
    }
}

// validate variable for assignments (x=5)
// invalid -1
// valid type variable (1)
// function return (2)
int validateVariable(char *variable, int variableType, char *typeName, int currentBlock, int lineno)
{
    // check if there is a function with the variable name
    int variableFunctionId = findFunction(variable);

    if (variableFunctionId != -1)
    {
        // check if the function is an same type function
        if (SymbolTableVariables[variableFunctionId].variableType == variableType)
        {
            return 2; // function returned (unresolvable)
        }
        else
        {
            fprintf(stderr, "Error at %d: Invalid assignment. Expected an %s value.\n", lineno, typeName);
            exit(EXIT_FAILURE);
        }
    }

    // find the variable
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    // throw error if the variable is not found or is not type
    if (tempVariable.variableId == NULL)
    {
        fprintf(stderr, "Error at %d: \"%s\" undeclared (or not in scope)\n", lineno, variable);
        exit(EXIT_FAILURE);
        return -1;
    }
    else if (tempVariable.variableType != variableType)
    {
        fprintf(stderr, "Error at %d: Invalid assignment. Expected an %s value\n", lineno, typeName);
        exit(EXIT_FAILURE);
        return -1;
    }

    return 1;
}

int getVariableType(char *variable, int currentBlock, int lineno)
{
    // find the variable
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    // throw error if the variable is not found or is not type
    if (tempVariable.variableId == NULL)
    {
        fprintf(stderr, "Error at %d: \"%s\" undeclared (or not in scope)\n", lineno, variable);
        exit(EXIT_FAILURE);
        return -1;
    }

    return tempVariable.variableType;
}

// === INTEGER ===
VariableEntry getIntVarValue(char *variable, int currentBlock, int lineno)
{
    // validate variable as integer
    int varValidation = validateVariable(variable, 0, "integer", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getIntVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // find the variable
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    // check if it is a function return
    if (varValidation == 2)
    {
        return tempVariable;
    }

    if (tempVariable.varOrFunctionVar == 5)
    {
        return tempVariable;
    }

    // check if variable is uninitialized
    if (tempVariable.intValue == INT_MIN)
    {
        fprintf(stderr, "Error at %d: Attempted to use uninitialized int variable \"%s\"\n", lineno, tempVariable.variableId);
        exit(EXIT_FAILURE);
        return createEmptyVariableEntry();
    }

    // return the integer value
    return tempVariable;
}

void assignIntVariableValue(char *variable, VariableEntry entry, int currentBlock, int lineno)
{
    // validate variable as integer
    int varValidation = validateVariable(variable, 0, "integer", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getIntVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // throw error if it is a function
    if (varValidation == 2)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a function\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // get the variable index and assign to it the value
    int variableIndex = getVariableId(variable);

    // check if the variable is constant
    if (SymbolTableVariables[variableIndex].varOrFunctionVar == 1)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a constant variable\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // make it normal again
    SymbolTableVariables[variableIndex].varOrFunctionVar = 0;
    // make the variable unresolvable if it is INT_MAX (function)
    if (entry.varOrFunctionVar == 2)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.variableId);
    }

    // y = x and x is unresolvable
    else if (entry.varOrFunctionVar == 5)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.functionVariableId);
        SymbolTableVariables[variableIndex].intValue = 0;
    }

    else
        // assign the value
        SymbolTableVariables[variableIndex].intValue = entry.intValue;
}

// === FLOAT ===
VariableEntry getFloatVarValue(char *variable, int currentBlock, int lineno)
{
    // validate variable as integer
    int varValidation = validateVariable(variable, 1, "float", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getFloatVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // find the variable
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    // check if it is a function return
    if (varValidation == 2)
    {
        return tempVariable;
    }

    if (tempVariable.varOrFunctionVar == 5)
    {
        return tempVariable;
    }

    // check if variable is uninitialized
    if (tempVariable.floatValue == -INFINITY)
    {
        fprintf(stderr, "Error at %d: Attempted to use uninitialized float variable \"%s\"\n", lineno, tempVariable.variableId);
        exit(EXIT_FAILURE);
        return createEmptyVariableEntry();
    }

    // return the integer value
    return tempVariable;
}

void assignFloatVariableValue(char *variable, VariableEntry entry, int currentBlock, int lineno)
{
    // validate variable as float
    int varValidation = validateVariable(variable, 1, "float", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getFloatVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // throw error if it is a function
    if (varValidation == 2)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a function\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // get the variable index and assign to it the value
    int variableIndex = getVariableId(variable);

    // check if the variable is constant
    if (SymbolTableVariables[variableIndex].varOrFunctionVar == 1)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a constant variable\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // make it normal again
    SymbolTableVariables[variableIndex].varOrFunctionVar = 0;
    // make the variable unresolvable if it is INT_MAX (function)
    if (entry.varOrFunctionVar == 2)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.variableId);
    }

    // y = x and x is unresolvable
    else if (entry.varOrFunctionVar == 5)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.functionVariableId);
        SymbolTableVariables[variableIndex].floatValue = 0.0;
    }

    else
        // assign the value
        SymbolTableVariables[variableIndex].floatValue = entry.floatValue;
}

// === Char ===
VariableEntry getCharVarValue(char *variable, int currentBlock, int lineno)
{
    // validate variable as integer
    int varValidation = validateVariable(variable, 2, "char", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getCharVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // find the variable
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    // check if it is a function return
    if (varValidation == 2)
    {
        return tempVariable;
    }

    if (tempVariable.varOrFunctionVar == 5)
    {
        return tempVariable;
    }

    // check if variable is uninitialized
    if (tempVariable.charValue == '\0')
    {
        fprintf(stderr, "Error at %d: Attempted to use uninitialized char variable \"%s\"\n", lineno, tempVariable.variableId);
        exit(EXIT_FAILURE);
        return createEmptyVariableEntry();
    }

    // return the char value
    return tempVariable;
}

void assignCharVariableValue(char *variable, VariableEntry entry, int currentBlock, int lineno)
{
    // validate variable as float
    int varValidation = validateVariable(variable, 2, "char", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getCharVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // throw error if it is a function
    if (varValidation == 2)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a function\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // get the variable index and assign to it the value
    int variableIndex = getVariableId(variable);

    // check if the variable is constant
    if (SymbolTableVariables[variableIndex].varOrFunctionVar == 1)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a constant variable\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // make it normal again
    SymbolTableVariables[variableIndex].varOrFunctionVar = 0;
    // make the variable unresolvable if it is INT_MAX (function)
    if (entry.varOrFunctionVar == 2)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.variableId);
        SymbolTableVariables[variableIndex].charValue = '0';
    }

    // y = x and x is unresolvable
    else if (entry.varOrFunctionVar == 5)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.functionVariableId);
        SymbolTableVariables[variableIndex].charValue = '0';
    }

    else // assign the value
        SymbolTableVariables[variableIndex].charValue = entry.charValue;
}

// === String ===
VariableEntry getStringVarValue(char *variable, int currentBlock, int lineno)
{
    // validate variable as integer
    int varValidation = validateVariable(variable, 3, "string", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getCharVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // find the variable
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    // check if it is a function return
    if (varValidation == 2)
    {
        return tempVariable;
    }

    if (tempVariable.varOrFunctionVar == 5)
    {
        return tempVariable;
    }

    // check if variable is uninitialized
    if (tempVariable.stringValue == NULL)
    {
        fprintf(stderr, "Error at %d: Attempted to use uninitialized string variable \"%s\"\n", lineno, tempVariable.variableId);
        exit(EXIT_FAILURE);
        return createEmptyVariableEntry();
    }

    // return the string value
    return tempVariable;
}

void assignStringVariableValue(char *variable, VariableEntry entry, int currentBlock, int lineno)
{

    // validate variable as float
    int varValidation = validateVariable(variable, 3, "string", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getStringVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // throw error if it is a function
    if (varValidation == 2)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a function\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // get the variable index and assign to it the value
    int variableIndex = getVariableId(variable);

    // check if the variable is constant
    if (SymbolTableVariables[variableIndex].varOrFunctionVar == 1)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a constant variable\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // make it normal again
    SymbolTableVariables[variableIndex].varOrFunctionVar = 0;
    // make the variable unresolvable if it is INT_MAX (function)
    if (entry.varOrFunctionVar == 2)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.variableId);
        SymbolTableVariables[variableIndex].stringValue = strdup("unresolvablee");
    }
    // y = x and x is unresolvable
    else if (entry.varOrFunctionVar == 5)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.functionVariableId);
        SymbolTableVariables[variableIndex].stringValue = strdup("unresolvablee");
    }

    // printf("seggg %s\n", entry.stringValue); ///// !!!!!!!!!! //////
    // assign the value
    else
        SymbolTableVariables[variableIndex].stringValue = strdup(entry.stringValue);
}

// === Bool ===
VariableEntry getBoolVarValue(char *variable, int currentBlock, int lineno)
{
    // validate variable as bool
    int varValidation = validateVariable(variable, 4, "bool", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getBoolVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // find the variable
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    // check if it is a function return
    if (varValidation == 2)
    {
        return tempVariable;
    }

    if (tempVariable.varOrFunctionVar == 5)
    {
        return tempVariable;
    }

    // check if variable is uninitialized
    if (tempVariable.boolValue == -1)
    {
        fprintf(stderr, "Error at %d: Attempted to use uninitialized bool variable \"%s\"\n", lineno, tempVariable.variableId);
        exit(EXIT_FAILURE);
        return createEmptyVariableEntry();
    }

    // return the integer value
    return tempVariable;
}

void assignBoolVariableValue(char *variable, VariableEntry entry, int currentBlock, int lineno)
{
    // validate variable as integer
    int varValidation = validateVariable(variable, 4, "bool", currentBlock, lineno);

    // throw error if somehow the validation returned after error
    if (varValidation == -1)
    {
        perror("Validation error at getBoolVarValue()\n");
        exit(EXIT_FAILURE);
    }

    // throw error if it is a function
    if (varValidation == 2)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a function\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // get the variable index and assign to it the value
    int variableIndex = getVariableId(variable);

    // check if the variable is constant
    if (SymbolTableVariables[variableIndex].varOrFunctionVar == 1)
    {
        fprintf(stderr, "Error at %d: \"%s\" is a constant variable\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // make it normal again
    SymbolTableVariables[variableIndex].varOrFunctionVar = 0;
    // make the variable unresolvable if it is INT_MAX (function)
    if (entry.varOrFunctionVar == 2)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.variableId);
    }

    // y = x and x is unresolvable
    else if (entry.varOrFunctionVar == 5)
    {
        SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
        SymbolTableVariables[variableIndex].functionVariableId = strdup(entry.functionVariableId);
        SymbolTableVariables[variableIndex].boolValue = 0;
    }

    else
        // assign the value
        SymbolTableVariables[variableIndex].boolValue = entry.boolValue;
}

// === COMPARISON === //
VariableEntry checkVarValue(char *variable, int variableType, int currentBlock, int lineno)
{
    if (variableType == 0) // int
    {
        return getIntVarValue(variable, currentBlock, lineno);
    }
    else if (variableType == 1) // float
    {
        return getFloatVarValue(variable, currentBlock, lineno);
    }
    else if (variableType == 2) // char
    {
        return getCharVarValue(variable, currentBlock, lineno);
    }
    else if (variableType == 3) // string
    {
        return getStringVarValue(variable, currentBlock, lineno);
    }
    else if (variableType == 4) // bool
    {
        return getBoolVarValue(variable, currentBlock, lineno);
    }

    return createEmptyVariableEntry();
}

int performIntComparison(int intValue1, int intValue2, int op)
{
    int comparisonResult = -1;
    switch (op)
    {
    case 0: // EQ
        if (intValue1 == intValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;

        break;
    case 1: // NEQ
        if (intValue1 != intValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 2: // GT

        if (intValue1 > intValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 3: // GE
        if (intValue1 >= intValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 4: // LT
        if (intValue1 < intValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 5: // LE
        if (intValue1 <= intValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;

        break;
    }

    return comparisonResult;
}

int performFloatComparison(float floatValue1, float floatValue2, int op)
{
    int comparisonResult = -1;
    switch (op)
    {
    case 0: // EQ
        if (floatValue1 == floatValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;

        break;
    case 1: // NEQ
        if (floatValue1 != floatValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 2: // GT

        if (floatValue1 > floatValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 3: // GE
        if (floatValue1 >= floatValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 4: // LT
        if (floatValue1 < floatValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 5: // LE
        if (floatValue1 <= floatValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;

        break;
    }

    return comparisonResult;
}

int performCharComparison(char charValue1, char charValue2, int op)
{
    int comparisonResult = -1;
    switch (op)
    {
    case 0: // EQ
        if (charValue1 == charValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;

        break;
    case 1: // NEQ
        if (charValue1 != charValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 2: // GT

        if (charValue1 > charValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 3: // GE
        if (charValue1 >= charValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 4: // LT
        if (charValue1 < charValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 5: // LE
        if (charValue1 <= charValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;

        break;
    }

    return comparisonResult;
}

int performStringComparison(char *stringValue1, char *stringValue2, int op, int lineno)
{
    int comparisonResult = -1;
    switch (op)
    {
    case 0: // EQ
        if (strcmp(stringValue1, stringValue2) == 0)
            comparisonResult = 1;
        else
            comparisonResult = 0;

        break;
    case 1: // NEQ
        if (strcmp(stringValue1, stringValue2) != 0)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 2: // GT
    case 3: // GE
    case 4: // LT
    case 5: // LE
        fprintf(stderr, "Error at %d: Non-equality string comparison\n", lineno);
        exit(EXIT_FAILURE);
        break;
    }

    return comparisonResult;
}

int performBoolComparison(int boolValue1, int boolValue2, int op)
{
    int comparisonResult = -1;
    switch (op)
    {
    case 0: // EQ
        if (boolValue1 == boolValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;

        break;
    case 1: // NEQ
        if (boolValue1 != boolValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 2: // GT

        if (boolValue1 > boolValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 3: // GE
        if (boolValue1 >= boolValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 4: // LT
        if (boolValue1 < boolValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;
        break;
    case 5: // LE
        if (boolValue1 <= boolValue2)
            comparisonResult = 1;
        else
            comparisonResult = 0;

        break;
    }

    return comparisonResult;
}

int comparisonLogic(VariableEntry operand1, VariableEntry operand2, int op, int currentBlock, int lineno)
{
    int comparisonResult = -1;
    // variables whose condition result can be determined (not waiting for function call)
    if ((operand1.varOrFunctionVar == 0 || operand1.varOrFunctionVar == 1 || operand1.varOrFunctionVar == 4) && (operand2.varOrFunctionVar == 0 || operand2.varOrFunctionVar == 1 || operand2.varOrFunctionVar == 4))
    {
        // check for uninitalized variables
        // checkVarValue($1, operand1.variableType, currentBlock, lineno);
        // checkVarValue($3, operand1.variableType, currentBlock, lineno);

        switch (operand1.variableType)
        {
        case 0: // int
            comparisonResult = performIntComparison(operand1.intValue, operand2.intValue, op);
            break;
        case 1: // float
            comparisonResult = performFloatComparison(operand1.floatValue, operand2.floatValue, op);
            break;
        case 2: // char
            comparisonResult = performCharComparison(operand1.charValue, operand2.charValue, op);
            break;
        case 3: // string
            comparisonResult = performStringComparison(operand1.stringValue, operand2.stringValue, op, lineno);
            break;
        case 4: // bool
            comparisonResult = performBoolComparison(operand1.boolValue, operand2.boolValue, op);
            break;
        }
        return comparisonResult;
    }

    return -1;
}

int singleComparisonLogic(VariableEntry operand1)
{
    int comparisonResult = -1;
    // variables whose condition result can be determined (not waiting for function call)
    if ((operand1.varOrFunctionVar == 0 || operand1.varOrFunctionVar == 1 || operand1.varOrFunctionVar == 4))
    {
        // check for uninitalized variables
        // checkVarValue($1, operand1.variableType, currentBlock, lineno);
        // checkVarValue($3, operand1.variableType, currentBlock, lineno);

        switch (operand1.variableType)
        {
        case 0: // int
            comparisonResult = operand1.intValue != 0 ? 1 : 0;
            break;
        case 1: // float
            comparisonResult = operand1.floatValue != 0.0 ? 1 : 0;
            break;
        case 2: // char
            comparisonResult = 1;
            break;
        case 3: // string
            comparisonResult = 1;
            break;
        case 4: // bool
            comparisonResult = operand1.boolValue == 1 ? 1 : 0;
            break;
        }
        return comparisonResult;
    }

    return -1;
}

// === FUNCTION CALLS ===
char *convertNumbertoTypeName(int type)
{
    switch (type)
    {
    case 0:
        return "integer";
    case 1:
        return "float";
    case 2:
        return "char";
    case 3:
        return "string";
    case 4:
        return "bool";
    case 5:
        return "enum";
    default:
        return "unknown";
    }
}

VariableEntry getValueForAssignmentStatementFunctionCall(char *variable, int variableType, int currentBlock, int lineno)
{
    char *typeName = convertNumbertoTypeName(variableType);
    // validate variable as integer
    int varValidation = validateVariable(variable, variableType, typeName, currentBlock, lineno);

    // find the variable
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    // check if it is a function return
    if (varValidation == 2)
    {
        return tempVariable;
    }

    if (tempVariable.varOrFunctionVar == 5)
    {
        return tempVariable;
    }

    // return the string value
    return tempVariable;
}

// === ENUM ===

VariableEntry validateEnumType(char *enumType, int lineno, int currentBlock)
{
    VariableEntry tempVariable = findVariable(enumType, currentBlock);

    if (tempVariable.variableId == NULL)
    {
        // throw error
        fprintf(stderr, "Error at %d: ENUM type %s is not found\n", lineno, enumType);
        exit(EXIT_FAILURE);
    }

    // if the variable is not enum type
    if (tempVariable.varOrFunctionVar != 3)
    {
        // throw error
        fprintf(stderr, "Error at %d: %s is not an ENUM type\n", lineno, enumType);
        exit(EXIT_FAILURE);
    }

    return tempVariable;
}

int checkIfEnumAssignmentValid(char *enumType, char *enumValue, int lineno, int currentBlock)
{
    VariableEntry tempVariable = validateEnumType(enumType, lineno, currentBlock);
    for (int i = 0; i < tempVariable.enumInitializationCounter; i++)
    {
        if (strcmp(tempVariable.emumInitialization[i], enumValue) == 0)
        {
            return 1;
        }
    }
    fprintf(stderr, "Error at %d: '%s' is not a valid '%s' ENUM type value\n", lineno, enumValue, enumType);
    exit(EXIT_FAILURE);
    return 0;
}

VariableEntry getEnumVarValue(char *variable, int currentBlock, int lineno, int wantToCheck)
{
    // check if there is a function with the variable name
    int variableFunctionId = findFunction(variable);

    if (variableFunctionId != -1)
    {
        fprintf(stderr, "Error at %d: %s is not an enum variable\n", lineno, variable);
        exit(EXIT_FAILURE);
    }

    // find the variable
    VariableEntry tempVariable = findVariable(variable, currentBlock);

    if (tempVariable.variableId == NULL)
    {
        fprintf(stderr, "Error at %d: \"%s\" undeclared (or not in scope)\n", lineno, variable);
        exit(EXIT_FAILURE);
    }
    else if (tempVariable.variableType != 5)
    {
        fprintf(stderr, "Error at %d: Invalid assignment. Expected an ENUM value\n", lineno);
        exit(EXIT_FAILURE);
    }

    // find the variable
    tempVariable = findVariable(variable, currentBlock);

    // check if variable is uninitialized
    if (tempVariable.enumValue == NULL && wantToCheck == 1)
    {
        fprintf(stderr, "Error at %d: Attempted to use uninitialized ENUM type variable \"%s\"\n", lineno, tempVariable.variableId);
        exit(EXIT_FAILURE);
        return createEmptyVariableEntry();
    }

    // return the integer value
    return tempVariable;
}

// operations 0: create 1: create and assign 2: assign
void createAndAssignNewEnum(int operation, char *enumType, char *enumVariable, char *enumValue, int lineno, int currentBlock)
{
    validateEnumType(enumType, lineno, currentBlock);
    VariableEntry entry = findVariable(enumVariable, currentBlock);

    if (operation == 0 || operation == 1)
    {
        if (entry.variableId != NULL || findFunction(entry.variableId) != -1)
        {
            // throw error
            fprintf(stderr, "Error at %d: redeclaration of variable %s\n", lineno, enumVariable);
            exit(EXIT_FAILURE);
        }

        if (ismodifiable == 1) // constant variable
        {
            entry.varOrFunctionVar = 1;
        }
        else
        {
            entry.varOrFunctionVar = 0;
        }

        entry.variableType = 5; // enum
        entry.variableId = strdup(enumVariable);
        entry.enumParent = strdup(enumType);
        if (operation == 1)
        {
            checkIfEnumAssignmentValid(enumType, enumValue, lineno, currentBlock);
            entry.enumValue = enumValue;
        }
        entry.declaredLineNo = lineno;
        entry.blockParent = currentBlock;

        SymbolTableVariables[variableCounter] = entry;
        variableCounter++;
    }
    else if (operation == 2)
    {
        // check if there is a function with the variable name
        int variableFunctionId = findFunction(enumVariable);

        if (variableFunctionId != -1)
        {
            fprintf(stderr, "Error at %d: %s is not an enum variable\n", lineno, enumVariable);
            exit(EXIT_FAILURE);
        }

        // find the variable
        VariableEntry tempVariable = findVariable(enumVariable, currentBlock);

        if (tempVariable.variableId == NULL)
        {
            fprintf(stderr, "Error at %d: \"%s\" undeclared (or not in scope)\n", lineno, enumVariable);
            exit(EXIT_FAILURE);
        }
        else if (tempVariable.variableType != 5)
        {
            fprintf(stderr, "Error at %d: Invalid assignment. Expected an ENUM value\n", lineno);
            exit(EXIT_FAILURE);
        }

        // find the variable
        tempVariable = findVariable(enumVariable, currentBlock);

        // get the variable index and assign to it the value
        int variableIndex = getVariableId(enumVariable);

        // check if the variable is constant
        if (SymbolTableVariables[variableIndex].varOrFunctionVar == 1)
        {
            fprintf(stderr, "Error at %d: \"%s\" is a constant variable\n", lineno, enumVariable);
            exit(EXIT_FAILURE);
        }

        // validate the value inserted
        checkIfEnumAssignmentValid(SymbolTableVariables[variableIndex].enumParent, enumValue, lineno, currentBlock);

        // assign the value
        SymbolTableVariables[variableIndex].enumValue = strdup(enumValue);
    }
}

// ===================================== QUADRUPLES ======================================
void initializeQuadrupleFile()
{
    if (outputFile == NULL)
    {
        outputFile = fopen(quadFileName, "w"); // Open the file in append mode
        if (outputFile == NULL)
        {
            fprintf(stderr, "Error opening file for writing.\n");
        }
    }
}

void writeLineToQuadrupleFile(const char *line)
{
    if (outputFile == NULL)
    {
        initializeQuadrupleFile();
    }
    if (outputFile != NULL)
    {
        fprintf(outputFile, "%s\n", line);
        fflush(outputFile); // Flush the changes to the file
    }
}

void closeQuadrupleFile()
{
    if (outputFile != NULL)
    {
        fclose(outputFile);
    }
}

void generateInitializeVariable(char *variableId)
{
    char *registerName = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "MOV %s, ", registerName);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, ", registerName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "MOV %s, %s", variableId, registerName);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", variableId, registerName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void generateCallFunction(char *variableId)
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "call %s", variableId);
    line = (char *)malloc(len + 1);
    sprintf(line, "call %s", variableId);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void generateAssignVariableValue(char *variableId, char *valueAsString, int varOrFunctionVar)
{
    if (varOrFunctionVar == 5)
    {
        // printf("the variable at generateAssignVariableValue could not be resolverd\n");
        generateCallFunction(SymbolTableVariables[getVariableId(variableId)].functionVariableId);
        return;
    }

    char *registerName = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "MOV %s, %s", registerName, valueAsString);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", registerName, valueAsString);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "MOV %s, %s", variableId, registerName);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", variableId, registerName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    free(registerName);
}

// MOV R0, value
void generateMovRLine(int uniqueId, char *valueAsString, char *registerName)
{
    // if (checkIfUniqueIdExists(uniqueId) == 1)
    // {
    //     return;
    // }
    // else {
    //     addUniqueIdtoUniqueIdArray(uniqueId);
    // }
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "MOV %s, %s", registerName, valueAsString);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", registerName, valueAsString);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void generateOpLine(const char *operationType, const char *operand1, const char *operand2, const char *operand3)
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s %s, %s, %s", operationType, operand1, operand2, operand3);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s %s, %s, %s", operationType, operand1, operand2, operand3);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

// === INTEGER ===
// for MOV R0, value , convert value to string
char *returnIntValueToPrintableValue(int varOrFunctionVar, int intValue, char *variableId)
{
    if (varOrFunctionVar == 4) // if it is constant
    {
        // Calculate the length of the number
        int length = snprintf(NULL, 0, "%d", intValue);

        // Allocate memory for the array of characters
        char *array = (char *)malloc((length + 1) * sizeof(char));

        // Convert the number to a string
        char *numberString = (char *)malloc((length + 1) * sizeof(char));
        sprintf(numberString, "%d", intValue);

        // Copy the characters to the array
        strcpy(array, numberString);

        // Free the allocated memory for the number string
        free(numberString);

        return array;
    }

    // if it is a variable
    return variableId;
}

void intFactorToFile(VariableEntry entry)
{

    // return the function itself if one of the operands are functions
    if (entry.varOrFunctionVar == 2 || entry.varOrFunctionVar == 5)
    {
        generateCallFunction(entry.functionVariableId);
        return;
    }

    // printf("intvalue=%d, variableid=%s\n", entry.intValue, entry.variableId);
    char *registerName = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    // get the value to be in the line MOV R0, Value
    char *finalValue = returnIntValueToPrintableValue(entry.varOrFunctionVar, entry.intValue, entry.variableId);

    // generate the line in file
    generateMovRLine(entry.uniqueId, finalValue, registerName);

    // Free the registerName
    free(registerName);
}

VariableEntry performIntOperation(VariableEntry operand1, VariableEntry operand2, char op, int lineno)
{
    // return the function itself if one of the operands are functions
    if (operand1.varOrFunctionVar == 2 || operand1.varOrFunctionVar == 5)
    {
        generateCallFunction(operand1.functionVariableId);
        return operand1;
    }
    if (operand2.varOrFunctionVar == 2 || operand2.varOrFunctionVar == 5)
    {
        generateCallFunction(operand2.functionVariableId);
        return operand2;
    }

    // get regesters for the 2 operands
    char *registerNameOperand1 = combineNumberWithString(registerCounter, "R");
    incrementRegister();
    char *registerNameOperand2 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    VariableEntry outputEntry = createEmptyVariableEntry();
    outputEntry.varOrFunctionVar = 0; // variable
    outputEntry.variableType = 0;     // int

    int finalvalue = 0;
    char *finalOperand = NULL;

    switch (op)
    {
    case '+':
        finalvalue = operand1.intValue + operand2.intValue;
        finalOperand = strdup("ADD");
        break;
    case '-':
        finalvalue = operand1.intValue - operand2.intValue;
        finalOperand = strdup("SUB");
        break;
    case '*':
        finalvalue = operand1.intValue * operand2.intValue;
        finalOperand = strdup("MUL");
        break;
    case '/':
        if (operand2.intValue == 0)
        {
            fprintf(stderr, "Error at %d: division by zero\n", lineno);
            exit(EXIT_FAILURE);
        }
        finalvalue = operand1.intValue / operand2.intValue;
        finalOperand = strdup("DIV");
        break;
    }

    // get the values that will be used in printing for the 2 operands
    char *finalValueOperand1 = returnIntValueToPrintableValue(operand1.varOrFunctionVar, operand1.intValue, operand1.variableId);
    char *finalValueOperand2 = returnIntValueToPrintableValue(operand2.varOrFunctionVar, operand2.intValue, operand2.variableId);

    // print MOV lines for the 2 operands
    generateMovRLine(operand1.uniqueId, finalValueOperand1, registerNameOperand1);
    generateMovRLine(operand2.uniqueId, finalValueOperand2, registerNameOperand2);

    // rename the operation to temp line
    char *tempName = combineNumberWithString(tempCounter, "Temp");
    incrementTemp();

    // write to file the operation line
    generateOpLine(finalOperand, tempName, registerNameOperand1, registerNameOperand2);

    // create a new variableEntry and return it
    VariableEntry finalVariable = createEmptyVariableEntry();
    finalVariable.varOrFunctionVar = 0; // variable
    finalVariable.variableType = 0;     // int
    finalVariable.variableId = strdup(tempName);
    finalVariable.intValue = finalvalue;

    free(registerNameOperand1);
    free(registerNameOperand2);
    free(tempName);

    return finalVariable;
}

// === Float ===
char *returnFloatValueToPrintableValue(int varOrFunctionVar, float floatValue, char *variableId)
{
    if (varOrFunctionVar == 4) // if it is constant
    {
        // Calculate the length of the number
        int length = snprintf(NULL, 0, "%f", floatValue);

        // Allocate memory for the array of characters
        char *array = (char *)malloc((length + 1) * sizeof(char));

        // Convert the number to a string
        char *numberString = (char *)malloc((length + 1) * sizeof(char));
        sprintf(numberString, "%f", floatValue);

        // Copy the characters to the array
        strcpy(array, numberString);

        // Free the allocated memory for the number string
        free(numberString);

        return array;
    }

    // if it is a variable
    return variableId;
}

void floatFactorToFile(VariableEntry entry)
{

    // return the function itself if one of the operands are functions
    if (entry.varOrFunctionVar == 2 || entry.varOrFunctionVar == 5)
    {
        generateCallFunction(entry.functionVariableId);
        return;
    }

    char *registerName = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    // get the value to be in the line MOV R0, Value
    char *finalValue = returnFloatValueToPrintableValue(entry.varOrFunctionVar, entry.floatValue, entry.variableId);

    // generate the line in file
    generateMovRLine(entry.uniqueId, finalValue, registerName);

    // Free the registerName
    free(registerName);
}

VariableEntry performFloatOperation(VariableEntry operand1, VariableEntry operand2, char op, int lineno)
{
    // return the function itself if one of the operands are functions
    if (operand1.varOrFunctionVar == 2 || operand1.varOrFunctionVar == 5)
    {
        generateCallFunction(operand1.functionVariableId);
        return operand1;
    }
    if (operand2.varOrFunctionVar == 2 || operand2.varOrFunctionVar == 5)
    {
        generateCallFunction(operand2.functionVariableId);
        return operand2;
    }

    // get regesters for the 2 operands
    char *registerNameOperand1 = combineNumberWithString(registerCounter, "R");
    incrementRegister();
    char *registerNameOperand2 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    VariableEntry outputEntry = createEmptyVariableEntry();
    outputEntry.varOrFunctionVar = 0; // variable
    outputEntry.variableType = 1;     // float

    float finalvalue = 0.0;
    char *finalOperand = NULL;

    switch (op)
    {
    case '+':
        finalvalue = operand1.floatValue + operand2.floatValue;
        finalOperand = strdup("ADD");
        break;
    case '-':
        finalvalue = operand1.floatValue - operand2.floatValue;
        finalOperand = strdup("SUB");
        break;
    case '*':
        finalvalue = operand1.floatValue * operand2.floatValue;
        finalOperand = strdup("MUL");
        break;
    case '/':
        if (operand2.floatValue == 0)
        {
            fprintf(stderr, "Error at %d: division by zero\n", lineno);
            exit(EXIT_FAILURE);
        }
        finalvalue = operand1.floatValue / operand2.floatValue;
        finalOperand = strdup("DIV");
        break;
    }

    // get the values that will be used in printing for the 2 operands
    char *finalValueOperand1 = returnFloatValueToPrintableValue(operand1.varOrFunctionVar, operand1.floatValue, operand1.variableId);
    char *finalValueOperand2 = returnFloatValueToPrintableValue(operand2.varOrFunctionVar, operand2.floatValue, operand2.variableId);

    // print MOV lines for the 2 operands
    generateMovRLine(operand1.uniqueId, finalValueOperand1, registerNameOperand1);
    generateMovRLine(operand2.uniqueId, finalValueOperand2, registerNameOperand2);

    // rename the operation to temp line
    char *tempName = combineNumberWithString(tempCounter, "Temp");
    incrementTemp();

    // write to file the operation line
    generateOpLine(finalOperand, tempName, registerNameOperand1, registerNameOperand2);

    // create a new variableEntry and return it
    VariableEntry finalVariable = createEmptyVariableEntry();
    finalVariable.varOrFunctionVar = 0; // variable
    finalVariable.variableType = 1;     // float
    finalVariable.variableId = strdup(tempName);
    finalVariable.floatValue = finalvalue;

    free(registerNameOperand1);
    free(registerNameOperand2);
    free(tempName);

    return finalVariable;
}

// === Char ===
char *returnCharValueToPrintableValue(int varOrFunctionVar, char charValue, char *variableId)
{
    if (varOrFunctionVar == 4) // if it is a constant
    {
        // Allocate memory for the array of characters (including null terminator and quotes)
        char *array = (char *)malloc(4 * sizeof(char));

        // Assign the character value to the array with quotes
        sprintf(array, "'%c'", charValue);

        return array;
    }

    // If it is a variable, return the variable ID
    return variableId;
}

// === String ===
char *returnStringValueToPrintableValue(int varOrFunctionVar, char *stringValue, char *variableId)
{
    if (varOrFunctionVar == 4) // if it is a constant
    {
        // Calculate the length of the string value
        int length = strlen(stringValue);

        // Allocate memory for the array of characters (including null terminator)
        char *array = (char *)malloc((length + 1) * sizeof(char));

        // Copy the string value to the array
        strcpy(array, stringValue);

        return array;
    }

    // If it is a variable, return the variable ID
    return variableId;
}

// === Bool ===
char *returnBoolValueToPrintableValue(int varOrFunctionVar, int boolValue, char *variableId)
{
    if (varOrFunctionVar == 4) // if it is a constant
    {
        // Allocate memory for the array of characters
        char *array = NULL;

        if (boolValue == 1)
        {
            array = strdup("true");
        }
        else if (boolValue == 0)
        {
            array = strdup("false");
        }

        return array;
    }

    // If it is a variable, return the variable ID
    return variableId;
}

// === COMPARISON ===
char *getComparisonString(int comparisonSymbol)
{
    switch (comparisonSymbol)
    {
    case 0:
        return "CMPEQ";
    case 1:
        return "CMPNE";
    case 2:
        return "CMPGT";
    case 3:
        return "CMPGE";
    case 4:
        return "CMPLT";
    case 5:
        return "CMPLE";
    default:
        return NULL;
    }
}

ComparisonEntry generatePriorityComparison(char *valueAsString1, char *valueAsString2, char *opString, int comparisonResult)
{
    ComparisonEntry tempComparisonEntry;
    if (comparisonResult == -1)
    {
        tempComparisonEntry.comparisonResult = -1;
        tempComparisonEntry.comparisonName = NULL;
        return tempComparisonEntry;
    }

    char *registerName1 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    char *registerName2 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "MOV %s, %s", registerName1, valueAsString1);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", registerName1, valueAsString1);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "MOV %s, %s", registerName2, valueAsString2);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", registerName2, valueAsString2);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    char *tempName = combineNumberWithString(tempCounter, "Temp");
    incrementTemp();

    len = snprintf(NULL, 0, "%s %s, %s, %s", opString, tempName, registerName1, registerName2);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s %s, %s, %s", opString, tempName, registerName1, registerName2);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    free(registerName1);
    free(registerName2);

    tempComparisonEntry.comparisonName = strdup(tempName);
    tempComparisonEntry.comparisonResult = comparisonResult;

    return tempComparisonEntry;
}

char *returnValueToPrintableValue(VariableEntry entry)
{
    switch (entry.variableType)
    {
    case 0:
        return returnIntValueToPrintableValue(entry.varOrFunctionVar, entry.intValue, entry.variableId);
        break;
    case 1:
        return returnFloatValueToPrintableValue(entry.varOrFunctionVar, entry.floatValue, entry.variableId);
        break;
    case 2:
        return returnCharValueToPrintableValue(entry.varOrFunctionVar, entry.charValue, entry.variableId);
        break;
    case 3:
        return returnStringValueToPrintableValue(entry.varOrFunctionVar, entry.stringValue, entry.variableId);
        break;
    case 4:
        return returnBoolValueToPrintableValue(entry.varOrFunctionVar, entry.boolValue, entry.variableId);
        break;
    }

    return NULL;
}

ComparisonEntry generateSingleCondtion(VariableEntry entry, int comparisonResult)
{

    ComparisonEntry tempComparisonEntry;
    tempComparisonEntry.comparisonResult = -1;
    tempComparisonEntry.comparisonName = NULL;
    if (comparisonResult == -1)
    {
        return tempComparisonEntry;
    }

    char *valueAsString = returnValueToPrintableValue(entry);

    char *registerName1 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    char *registerName2 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    if (entry.variableType == 0) // int
    {
        int len = -1;
        char *line = NULL;

        len = snprintf(NULL, 0, "MOV %s, %s", registerName1, valueAsString);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, %s", registerName1, valueAsString);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        len = snprintf(NULL, 0, "MOV %s, 0", registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, 0", registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        char *tempName = combineNumberWithString(tempCounter, "Temp");
        incrementTemp();

        len = snprintf(NULL, 0, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        tempComparisonEntry.comparisonResult = comparisonResult;
        tempComparisonEntry.comparisonName = strdup(tempName);
        return tempComparisonEntry;
    }
    if (entry.variableType == 1) // float
    {
        int len = -1;
        char *line = NULL;

        len = snprintf(NULL, 0, "MOV %s, %s", registerName1, valueAsString);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, %s", registerName1, valueAsString);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        len = snprintf(NULL, 0, "MOV %s, 0.0", registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, 0.0", registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        char *tempName = combineNumberWithString(tempCounter, "Temp");
        incrementTemp();

        len = snprintf(NULL, 0, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        tempComparisonEntry.comparisonResult = comparisonResult;
        tempComparisonEntry.comparisonName = strdup(tempName);
        return tempComparisonEntry;
    }
    if (entry.variableType == 4) // bool
    {
        int len = -1;
        char *line = NULL;

        len = snprintf(NULL, 0, "MOV %s, %s", registerName1, valueAsString);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, %s", registerName1, valueAsString);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        len = snprintf(NULL, 0, "MOV %s, false", registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, false", registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        char *tempName = combineNumberWithString(tempCounter, "Temp");
        incrementTemp();

        len = snprintf(NULL, 0, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        tempComparisonEntry.comparisonResult = comparisonResult;
        tempComparisonEntry.comparisonName = strdup(tempName);
        return tempComparisonEntry;
    }
    if (entry.variableType == 2) // char
    {
        int len = -1;
        char *line = NULL;

        len = snprintf(NULL, 0, "MOV %s, 1", registerName1);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, 1", registerName1);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        len = snprintf(NULL, 0, "MOV %s, 0", registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, 0", registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        char *tempName = combineNumberWithString(tempCounter, "Temp");
        incrementTemp();

        len = snprintf(NULL, 0, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        tempComparisonEntry.comparisonResult = comparisonResult;
        tempComparisonEntry.comparisonName = strdup(tempName);
        return tempComparisonEntry;
    }
    if (entry.variableType == 3) // string
    {
        int len = -1;
        char *line = NULL;

        len = snprintf(NULL, 0, "MOV %s, 1", registerName1);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, 1", registerName1);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        len = snprintf(NULL, 0, "MOV %s, 0", registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "MOV %s, 0", registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        char *tempName = combineNumberWithString(tempCounter, "Temp");
        incrementTemp();

        len = snprintf(NULL, 0, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);
        line = (char *)malloc(len + 1);
        sprintf(line, "CMPNEQ %s %s, %s", tempName, registerName1, registerName2);

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        tempComparisonEntry.comparisonResult = comparisonResult;
        tempComparisonEntry.comparisonName = strdup(tempName);
        return tempComparisonEntry;
    }

    return tempComparisonEntry;
}

ComparisonEntry generateNotCondition(ComparisonEntry condition1)
{
    ComparisonEntry tempComparisonEntry;
    tempComparisonEntry.comparisonResult = -1;
    tempComparisonEntry.comparisonName = NULL;
    if (condition1.comparisonResult == -1)
    {
        int len = -1;
        char *line = NULL;

        len = snprintf(NULL, 0, "call function");
        line = (char *)malloc(len + 1);
        sprintf(line, "call function");

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        return tempComparisonEntry;
    }
    char *registerName1 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "MOV %s, %s", registerName1, condition1.comparisonName);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", registerName1, condition1.comparisonName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    char *tempName = combineNumberWithString(tempCounter, "Temp");
    incrementTemp();

    len = snprintf(NULL, 0, "NOT %s, %s", tempName, registerName1);
    line = (char *)malloc(len + 1);
    sprintf(line, "NOT %s, %s", tempName, registerName1);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    // not the value
    tempComparisonEntry.comparisonResult = condition1.comparisonResult == 0 ? 1 : 0;
    // printf("%d\n", tempComparisonEntry.comparisonResult);
    tempComparisonEntry.comparisonName = strdup(tempName);
    return tempComparisonEntry;
}
// compType = 0: or, 1: and
ComparisonEntry generateOrAndCondition(ComparisonEntry condition1, ComparisonEntry condition2, int compType)
{
    ComparisonEntry tempComparisonEntry;
    tempComparisonEntry.comparisonResult = -1;
    tempComparisonEntry.comparisonName = NULL;
    if (condition1.comparisonResult == -1 || condition2.comparisonResult == -1)
    {
        int len = -1;
        char *line = NULL;

        len = snprintf(NULL, 0, "call function");
        line = (char *)malloc(len + 1);
        sprintf(line, "call function");

        if (line != NULL)
        {
            writeLineToQuadrupleFile(line);
            free(line);
        }

        return tempComparisonEntry;
    }

    char *registerName1 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    char *registerName2 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "MOV %s, %s", registerName1, condition1.comparisonName);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", registerName1, condition1.comparisonName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "MOV %s, %s", registerName2, condition2.comparisonName);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", registerName2, condition2.comparisonName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    char *tempName = combineNumberWithString(tempCounter, "Temp");
    incrementTemp();

    char *opString = compType == 0 ? strdup("OR") : strdup("AND");

    len = snprintf(NULL, 0, "%s %s, %s, %s", opString, tempName, registerName1, registerName2);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s %s, %s, %s", opString, tempName, registerName1, registerName2);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
    if (compType == 0) // or
    {
        tempComparisonEntry.comparisonResult = (condition1.comparisonResult == 1 || condition2.comparisonResult == 1) ? 1 : 0;
    }
    else if (compType == 1) // and
    {
        tempComparisonEntry.comparisonResult = (condition1.comparisonResult == 1 && condition2.comparisonResult == 1) ? 1 : 0;
    }
    // printf("%d\n", tempComparisonEntry.comparisonResult);
    tempComparisonEntry.comparisonName = strdup(tempName);
    return tempComparisonEntry;
}

// === IF CONDITION ===
void startIf()
{
    char *labelName = combineNumberWithString(labelCounter, "JF LABEL");
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s", labelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s", labelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "OPEN IF");
    line = (char *)malloc(len + 1);
    sprintf(line, "OPEN IF");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void endIf()
{
    char *labelName = combineNumberWithString(labelCounter, "LABEL");
    char *endIfLabelName = combineNumberWithString(labelCounter2, "ENDIFLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "CLOSE IF");
    line = (char *)malloc(len + 1);
    sprintf(line, "CLOSE IF");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "JMP %s", endIfLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "JMP %s", endIfLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "%s:", labelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", labelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    labelCounter++;
}

void openElse()
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "OPEN ELSE");
    line = (char *)malloc(len + 1);
    sprintf(line, "OPEN ELSE");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void closeElse()
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "CLOSE ELSE");
    line = (char *)malloc(len + 1);
    sprintf(line, "CLOSE ELSE");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void finishIfstatement()
{
    char *endIfLabelName = combineNumberWithString(labelCounter2, "ENDIFLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s:", endIfLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", endIfLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    labelCounter2++;
}

// === FOR LOOP ===
void forLoopSemiColon1()
{
    char *openForLoopLabelName = combineNumberWithString(openForLoopCounter, "OPENFORLOOPLABEL");
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s:", openForLoopLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", openForLoopLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void forLoopSemiColon2()
{
    char *closeForLoopLabelName = combineNumberWithString(closeForLoopCounter, "JF CLOSEFORLOOPLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s", closeForLoopLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s", closeForLoopLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "OPEN FOR LOOP");
    line = (char *)malloc(len + 1);
    sprintf(line, "OPEN FOR LOOP");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void finishForLoopStatement()
{
    char *openForLoopLabelName = combineNumberWithString(openForLoopCounter, "OPENFORLOOPLABEL");
    char *closeForLoopLabelName = combineNumberWithString(closeForLoopCounter, "CLOSEFORLOOPLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "JMP %s", openForLoopLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "JMP %s", openForLoopLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "CLOSE FOR LOOP");
    line = (char *)malloc(len + 1);
    sprintf(line, "CLOSE FOR LOOP");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "%s:", closeForLoopLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", closeForLoopLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    openForLoopCounter++;
    closeForLoopCounter++;
}

// === WHILE LOOP ===
void startWhile()
{
    char *openWhileLoopLabelName = combineNumberWithString(openWhileLoopCounter, "OPENWHILELOOPLABEL");
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s:", openWhileLoopLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", openWhileLoopLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void whileLoopAfterCondition()
{
    char *closeWhileLoopLabelName = combineNumberWithString(closeWhileLoopCounter, "JF CLOSEWHILELOOPLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s", closeWhileLoopLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s", closeWhileLoopLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "OPEN WHILE");
    line = (char *)malloc(len + 1);
    sprintf(line, "OPEN WHILE");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void finishWhileLoopStatement()
{
    char *openWhileLoopLabelName = combineNumberWithString(openWhileLoopCounter, "OPENWHILELOOPLABEL");
    char *closeWhileLoopLabelName = combineNumberWithString(closeWhileLoopCounter, "CLOSEWHILELOOPLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "JMP %s", openWhileLoopLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "JMP %s", openWhileLoopLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "CLOSE WHILE");
    line = (char *)malloc(len + 1);
    sprintf(line, "CLOSE WHILE");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "%s:", closeWhileLoopLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", closeWhileLoopLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    openWhileLoopCounter;
    closeWhileLoopCounter;
}

// === REPEAT UNTIL ===
void startRepeatUntil()
{
    char *openRepeatUntilLabelName = combineNumberWithString(openRepeatUntilCounter, "OPENREPEATUNTILLABEL");
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s:", openRepeatUntilLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", openRepeatUntilLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void finishRepeatUntilStatement()
{
    char *openRepeatUntilLabelName = combineNumberWithString(openRepeatUntilCounter, "JT OPENREPEATUNTILLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s", openRepeatUntilLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s", openRepeatUntilLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    openRepeatUntilCounter++;
}

// === SWITCH STATEMENT ===
void startSwitchStatement()
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "OPEN SWITCH");
    line = (char *)malloc(len + 1);
    sprintf(line, "OPEN SWITCH");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void switchCaseStart(char *variableId, char *valueAsString)
{
    char *caseLabelName = combineNumberWithString(caseLabelCounter, "CASELABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "CASE %s:", valueAsString);
    line = (char *)malloc(len + 1);
    sprintf(line, "CASE %s:", valueAsString);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    char *registerName1 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    char *registerName2 = combineNumberWithString(registerCounter, "R");
    incrementRegister();

    len = snprintf(NULL, 0, "MOV %s, %s", registerName1, variableId);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", registerName1, variableId);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "MOV %s, %s", registerName2, valueAsString);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", registerName2, valueAsString);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    char *tempName = combineNumberWithString(tempCounter, "Temp");
    incrementTemp();

    len = snprintf(NULL, 0, "CMPEQ %s, %s, %s", tempName, registerName1, registerName2);
    line = (char *)malloc(len + 1);
    sprintf(line, "CMPEQ %s, %s, %s", tempName, registerName1, registerName2);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "JF %s", caseLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "JF %s", caseLabelName);
    // caseLabelCounter++;

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void switchCaseEnd()
{
    char *closeSwitchLabelName = combineNumberWithString(closeSwitchLabelCounter, "CLOSESWITCHLABEL");
    char *caseLabelName = combineNumberWithString(caseLabelCounter, "CASELABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "JMP %s", closeSwitchLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "JMP %s", closeSwitchLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "%s:", caseLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", caseLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
    caseLabelCounter++;
}

void switchDefaultCase()
{
    char *defaultCaseLabelName = combineNumberWithString(defaultCaseCounter, "DEFAULTCASE");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s:", defaultCaseLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", defaultCaseLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
    defaultCaseCounter++;
}

void finishSwitchStatement()
{
    char *closeSwitchLabelName = combineNumberWithString(closeSwitchLabelCounter, "CLOSESWITCHLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "CLOSE SWITCH");
    line = (char *)malloc(len + 1);
    sprintf(line, "CLOSE SWITCH");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "%s:", closeSwitchLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", closeSwitchLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    closeSwitchLabelCounter++;
}

// === Function ===
void startFunction(char *variableId)
{
    char *endLabelLabelName = combineNumberWithString(endLabelCounter, "ENDLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "JMP %s", endLabelLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "JMP %s", endLabelLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    len = snprintf(NULL, 0, "%s:", variableId);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", variableId);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void generatReturnLine()
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "RET");
    line = (char *)malloc(len + 1);
    sprintf(line, "RET");

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void generateReturnValue(char *inputString)
{
    char *tempName = combineNumberWithString(tempCounter, "Temp");
    incrementTemp();

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "MOV %s, %s", tempName, inputString);
    line = (char *)malloc(len + 1);
    sprintf(line, "MOV %s, %s", tempName, inputString);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void finishFunctionDecleration()
{
    generatReturnLine();

    char *endLabelLabelName = combineNumberWithString(endLabelCounter, "ENDLABEL");

    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "%s:", endLabelLabelName);
    line = (char *)malloc(len + 1);
    sprintf(line, "%s:", endLabelLabelName);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }

    endLabelCounter++;
}

// === ENUM TYPE ===
void startEnumType(char *variableId)
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "CRT %s", variableId);
    line = (char *)malloc(len + 1);
    sprintf(line, "CRT %s", variableId);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void addEnumValues(char *variableId, char *inputEnumValue)
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "ADV %s, %s", variableId, inputEnumValue);
    line = (char *)malloc(len + 1);
    sprintf(line, "ADV %s, %s", variableId, inputEnumValue);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

void endEnumType(char *variableId)
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "ECR %s", variableId);
    line = (char *)malloc(len + 1);
    sprintf(line, "ECR %s", variableId);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

// === ENUM VARIABLE ===
void generateEnumVariable(char *enumType, char *enumVar)
{
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "CRV %s, %s", enumType, enumVar);
    line = (char *)malloc(len + 1);
    sprintf(line, "CRV %s, %s", enumType, enumVar);

    if (line != NULL)
    {
        writeLineToQuadrupleFile(line);
        free(line);
    }
}

char *returnEnumValueToPrintableValue(int varOrFunctionVar, char *enumValue, char *variableId)
{
    if (varOrFunctionVar == 4) // if it is a constant
    {
        return enumValue;
    }

    // If it is a variable, return the variable ID
    return variableId;
}

//  ========================================= SYMBOL TABLE ======================================

void initializeSymFile()
{
    if (outputFile2 == NULL)
    {
        outputFile2 = fopen(symFileName, "a"); // Open the file in append mode
        if (outputFile2 == NULL)
        {
            fprintf(stderr, "Error opening file for writing.\n");
        }
    }
}

void writeLineToSymFile(const char *line)
{
    if (outputFile2 == NULL)
    {
        initializeSymFile();
    }
    if (outputFile2 != NULL)
    {
        fprintf(outputFile2, "%s\n", line);
        fflush(outputFile2); // Flush the changes to the file
    }
}

void closeSymFile()
{
    if (outputFile2 != NULL)
    {
        fclose(outputFile2);
    }
}

void buildSymbolTable()
{
    // type the first line
    int len = -1;
    char *line = NULL;

    len = snprintf(NULL, 0, "variable_type, type, variable_Id, value, line_declared");
    line = (char *)malloc(len + 1);
    sprintf(line, "variable_type, type, variable_Id, value, line_declared");

    if (line != NULL)
    {
        writeLineToSymFile(line);
        free(line);
    }

    for (int i = 0; i < variableCounter; i++)
    {
        if (SymbolTableVariables[i].variableId != NULL)
        {
            if (SymbolTableVariables[i].varOrFunctionVar == 0) // normal
            {
                if (SymbolTableVariables[i].variableType == 0) // int
                {
                    len = snprintf(NULL, 0, "variable, int, %s, %d, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].intValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "variable, int, %s, %d, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].intValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 1) // float
                {
                    len = snprintf(NULL, 0, "variable, float, %s, %f, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].floatValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "variable, float, %s, %f, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].floatValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 2) // char
                {
                    len = snprintf(NULL, 0, "variable, char, %s, %c, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].charValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "variable, char, %s, %c, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].charValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 3) // string
                {
                    len = snprintf(NULL, 0, "variable, string, %s, %s, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].stringValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "variable, string, %s, %s, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].stringValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 4) // bool
                {
                    len = snprintf(NULL, 0, "variable, bool, %s, %d, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].boolValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "variable, bool, %s, %d, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].boolValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 5) // enum
                {
                    len = snprintf(NULL, 0, "variable, enum, %s, %s, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].enumValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "variable, enum, %s, %s, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].enumValue, SymbolTableVariables[i].declaredLineNo);
                }

                if (line != NULL)
                {
                    writeLineToSymFile(line);
                    free(line);
                }
            }
            else if (SymbolTableVariables[i].varOrFunctionVar == 1) // constant variable
            {
                if (SymbolTableVariables[i].variableType == 0) // int
                {
                    len = snprintf(NULL, 0, "const variable, int, %s, %d, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].intValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "const variable, int, %s, %d, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].intValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 1) // float
                {
                    len = snprintf(NULL, 0, "const variable, float, %s, %f, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].floatValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "const variable, float, %s, %f, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].floatValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 2) // char
                {
                    len = snprintf(NULL, 0, "const variable, char, %s, %c, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].charValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "const variable, char, %s, %c, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].charValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 3) // string
                {
                    len = snprintf(NULL, 0, "const variable, string, %s, %s, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].stringValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "const variable, string, %s, %s, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].stringValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 4) // bool
                {
                    len = snprintf(NULL, 0, "const variable, bool, %s, %d, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].boolValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "const variable, bool, %s, %d, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].boolValue, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 5) // enum
                {
                    len = snprintf(NULL, 0, "const variable, enum, %s, %s, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].enumValue, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "const variable, enum, %s, %s, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].enumValue, SymbolTableVariables[i].declaredLineNo);
                }
                if (line != NULL)
                {
                    writeLineToSymFile(line);
                    free(line);
                }
            }
            else if (SymbolTableVariables[i].varOrFunctionVar == 2) // function
            {
                if (SymbolTableVariables[i].variableType == 0) // int
                {
                    len = snprintf(NULL, 0, "function variable, int, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "function variable, int, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 1) // float
                {
                    len = snprintf(NULL, 0, "function variable, float, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "function variable, float, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 2) // char
                {
                    len = snprintf(NULL, 0, "function variable, char, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "function variable, char, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 3) // string
                {
                    len = snprintf(NULL, 0, "function variable, string, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "function variable, string, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 4) // bool
                {
                    len = snprintf(NULL, 0, "function variable, bool, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "function variable, bool, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 5) // enum
                {
                    len = snprintf(NULL, 0, "function variable, VOID, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "function variable, VOID, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                if (line != NULL)
                {
                    writeLineToSymFile(line);
                    free(line);
                }
            }
            else if (SymbolTableVariables[i].varOrFunctionVar == 5) // function
            {
                if (SymbolTableVariables[i].variableType == 0) // int
                {
                    len = snprintf(NULL, 0, "unresolvable variable, int, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "unresolvable variable, int, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 1) // float
                {
                    len = snprintf(NULL, 0, "unresolvable variable, float, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "unresolvable variable, float, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 2) // char
                {
                    len = snprintf(NULL, 0, "unresolvable variable, char, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "unresolvable variable, char, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 3) // string
                {
                    len = snprintf(NULL, 0, "unresolvable variable, string, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "unresolvable variable, string, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 4) // bool
                {
                    len = snprintf(NULL, 0, "unresolvable variable, bool, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "unresolvable variable, bool, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                else if (SymbolTableVariables[i].variableType == 5) // enum
                {
                    len = snprintf(NULL, 0, "unresolvable variable, enum, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                    line = (char *)malloc(len + 1);
                    sprintf(line, "unresolvable variable, enum, %s, NA, %d", SymbolTableVariables[i].variableId, SymbolTableVariables[i].declaredLineNo);
                }
                if (line != NULL)
                {
                    writeLineToSymFile(line);
                    free(line);
                }
            }
        }
    }
    len = snprintf(NULL, 0, "============================================");
    line = (char *)malloc(len + 1);
    sprintf(line, "============================================");

    if (line != NULL)
    {
        writeLineToSymFile(line);
        free(line);
    }
}