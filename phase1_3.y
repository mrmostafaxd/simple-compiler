%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "functions.h"
    #include "y.tab.h"
    #include "lex.yy.c"
    
    void checkBrackets();
    void closeProgram();
    int yylex(void);
    extern int yylineno;
    int yyerror(const char* s);
    int main(void);
    int bracketCount = 0;

    // for switch statement
    int globalSwitchVariableType = -1;
    char* globalSwitchVariableName = NULL;

    // for declaring functions
    int globalFunctionVariableType = -1;
    int theFunctionReturnedSomething = 1;
    char* globalFunctionVariableName = NULL;

    // for calling functions
    char* globalFunctionCallVariableName = NULL;

    // for enums
    char* globalEnumTypeVariableName = NULL;


    // tazbet //
    ////////////
    
    // for repeating "int MOV R1, 8" as a result of print in intFacter
    int intMultipleOperationUsed = 0;
%}

%union {
    char* idValue;
    int intValue;
    double floatValue;
    char* stringValue;
    char charValue;
    char* enumValue;
    VariableEntry dataItem;
    int boolValue;

    ComparisonEntry comparisonItem;
};

%token <intValue> INT_NUM 
%token <idValue> VARIABLE
%token <floatValue> FLOAT_NUM
%token <charValue> CHAR_VALUE
%token <stringValue> STRING_VALUE
%token <enumValue> ENUM_IDENTIFIER
%token  <boolValue> TRUE FALSE

%token INT FLOAT CHAR BOOL STRING CONST FUNC RETURN ENUM SYM END VOID
%token IF ELIF SWITCH CASE BREAK DEFAULT FOR WHILE REPEAT UNTIL
%token INC DEC

%nonassoc ELSE IFX
%nonassoc LOWER_THAN_ELSE

%type <dataItem> intExpr intPriorityExpr intFactor
%type <dataItem> floatExpr floatPriorityExpr floatFactor
%type <dataItem> funcCallStartStatement funcCallStatement
%type <idValue> intVariableStatement floatVariableStatement charVariableStatement stringVariableStatement boolVariableStatement specialFuncType enumStartStatement
%type <boolValue> boolFactor
%type <intValue> prioritySymbol
%type <comparisonItem> priorityCondtion condition

%left '+' '-'
%left '*' '/'
%left <stringValue> GT GE LT LE EQ NEQ OR AND NOT

%start statement

%%

/* program                     :   program statement
                            |
                            ; */

statement                   :   varStatement ';' 
                            |   constStatement ';'
                            |   assignStatement ';'
                            |   intExpr ';' {uniqueIdArrayCounter = 0;}
                            |   floatExpr ';'
                            /* |   conditionalStatement  */
                            |   ifStatement
                            |   enumStatement ';'
                            |   whileStatement
                            |   repeatStatement ';'
                            |   forStatement
                            |   switchStatement
                            |   funcStatement
                            |   funcCallStatement ';'
                            |   END ';' {printf("exit\n");exit(EXIT_SUCCESS);}
                            |   SYM ';' {buildSymbolTable();}
                            |   statement varStatement ';'
                            |   statement constStatement ';'
                            |   statement assignStatement ';'
                            |   statement intExpr ';' {uniqueIdArrayCounter = 0;}
                            |   statement floatExpr ';'
                            /* |   statement conditionalStatement  */
                            |   statement ifStatement
                            |   statement enumStatement ';'
                            |   statement whileStatement
                            |   statement repeatStatement ';'
                            |   statement forStatement
                            |   statement switchStatement
                            |   statement funcStatement
                            |   statement funcCallStatement ';'
                            |   statement END ';' {closeProgram();}
                            |   statement SYM ';' {buildSymbolTable();};
                            |   statement openBlock statement closeBlock {;}
                            |   openBlock statement closeBlock statement {;}
                            ;   

constStatement              :   CONST {ismodifiable = 1;} defStatement {printf("constant decleration \n");}
                            ;

varStatement                :   decStatement
                            |   funcDefStatement
                            |   defStatement  
                            ;   

decStatement                :   intVariableStatement {

                                                        VariableEntry entry = createEmptyVariableEntry();
                                                        // insertVariable(0, $1, 0, 0, INT_MIN, -INFINITY, '\0', NULL, -1, NULL, NULL, yylineno, peek(&blockStack));
                                                        insertVariable(0, $1, 0, 0, entry, yylineno, peek(&blockStack));
                                                        printf("int variable decleration %d\n", yylineno); 
                                                    }
                            |   floatVariableStatement  {
                                                            VariableEntry entry = createEmptyVariableEntry();
                                                            insertVariable(0, $1, 0, 1, entry, yylineno, peek(&blockStack));
                                                            printf("float variable decleration \n");
                                                        }
                            |   charVariableStatement   {
                                                            VariableEntry entry = createEmptyVariableEntry();
                                                            insertVariable(0, $1, 0, 2, entry, yylineno, peek(&blockStack));
                                                            printf("char variable decleration \n");
                                                        }
                            |   stringVariableStatement {
                                                            VariableEntry entry = createEmptyVariableEntry();
                                                            insertVariable(0, $1, 0, 3, entry, yylineno, peek(&blockStack));
                                                            printf("string variable decleration \n");
                                                        }
                            |   boolVariableStatement   {
                                                            VariableEntry entry = createEmptyVariableEntry();
                                                            insertVariable(0, $1, 0, 4, entry, yylineno, peek(&blockStack));
                                                            printf("bool variable decleration \n");
                                                        }
                            |   ENUM VARIABLE VARIABLE  {
                                                            createAndAssignNewEnum(0, $2, $3, NULL, yylineno, peek(&blockStack));
                                                            generateEnumVariable($2, $3);
                                                        }
                            ;

funcDefStatement            :   intVariableStatement '=' funcCallStatement   {
                                                                                // get the function
                                                                                VariableEntry entry = $3;

                                                                                // check if it is not integer
                                                                                if (entry.variableType != 0)
                                                                                {
                                                                                    fprintf(stderr, "Error at %d: Invalid assignment. Expected an integer value\n", yylineno);
                                                                                    exit(EXIT_FAILURE);
                                                                                }
                                                                                
                                                                                insertVariable(0, $1, 0, 0, entry, yylineno, peek(&blockStack));
                                                                                generateCallFunction(entry.variableId);
                                                                            }
                            |   floatVariableStatement '=' funcCallStatement    {
                                                                                    // get the function
                                                                                    VariableEntry entry = $3;

                                                                                    // check if it is not float
                                                                                    if (entry.variableType != 1)
                                                                                    {
                                                                                        fprintf(stderr, "Error at %d: Invalid assignment. Expected a float value\n", yylineno);
                                                                                        exit(EXIT_FAILURE);
                                                                                    }
                                                                                    
                                                                                    insertVariable(0, $1, 0, 1, entry, yylineno, peek(&blockStack));
                                                                                    generateCallFunction(entry.variableId);
                                                                                }
                            |   charVariableStatement '=' funcCallStatement     {
                                                                                    // get the function
                                                                                    VariableEntry entry = $3;

                                                                                    // check if it is not char
                                                                                    if (entry.variableType != 2)
                                                                                    {
                                                                                        fprintf(stderr, "Error at %d: Invalid assignment. Expected a char value\n", yylineno);
                                                                                        exit(EXIT_FAILURE);
                                                                                    }
                                                                                    
                                                                                    insertVariable(0, $1, 0, 2, entry, yylineno, peek(&blockStack));
                                                                                    generateCallFunction(entry.variableId);
                                                                                }
                            |   stringVariableStatement '=' funcCallStatement   {
                                                                                    // get the function
                                                                                    VariableEntry entry = $3;

                                                                                    // check if it is not string
                                                                                    if (entry.variableType != 3)
                                                                                    {
                                                                                        fprintf(stderr, "Error at %d: Invalid assignment. Expected a string value\n", yylineno);
                                                                                        exit(EXIT_FAILURE);
                                                                                    }
                                                                                    
                                                                                    // printf("hereeeeee\n");
                                                                                    insertVariable(0, $1, 0, 3, entry, yylineno, peek(&blockStack));
                                                                                    generateCallFunction(entry.variableId);
                                                                                }
                            |   boolVariableStatement '=' funcCallStatement {
                                                                                // get the function
                                                                                VariableEntry entry = $3;

                                                                                // check if it is not bool
                                                                                if (entry.variableType != 4)
                                                                                {
                                                                                    fprintf(stderr, "Error at %d: Invalid assignment. Expected a bool value\n", yylineno);
                                                                                    exit(EXIT_FAILURE);
                                                                                }
                                                                                
                                                                                insertVariable(0, $1, 0, 4, entry, yylineno, peek(&blockStack));
                                                                                generateCallFunction(entry.variableId);
                                                                            }
                            ;

defStatement                :   intVariableStatement '=' intExpr {
                                                                    VariableEntry entry = $3;
                                                                    insertVariable(0, $1, 0, 0, entry, yylineno, peek(&blockStack));
                                                                    char* finalName = returnIntValueToPrintableValue(entry.varOrFunctionVar, entry.intValue, entry.variableId);
                                                                    generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);
                                                                    uniqueIdArrayCounter = 0;
                                                                    ismodifiable = 0;
                                                                }
                            |   floatVariableStatement '=' floatExpr    {
                                                                            VariableEntry entry = $3;
                                                                            insertVariable(0, $1, 0, 1, entry, yylineno, peek(&blockStack));
                                                                            char* finalName = returnFloatValueToPrintableValue(entry.varOrFunctionVar, entry.floatValue, entry.variableId);
                                                                            generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);
                                                                            uniqueIdArrayCounter = 0;
                                                                            ismodifiable = 0;
                                                                        }
                            |   charVariableStatement '=' CHAR_VALUE    {
                                                                            // create char_value                                                            
                                                                            VariableEntry entry = createEmptyVariableEntry();
                                                                            entry.varOrFunctionVar = 4;
                                                                            entry.variableType = 2;
                                                                            entry.charValue = $3;
                                                                            varOrFunctionVarIndetifier = 4; 
                                                                            typeIdentifier = 2;

                                                                            // insert dummy variable value
                                                                            insertVariable(0, $1, 0, 2, entry, yylineno, peek(&blockStack));
                                                                            char* finalName = returnCharValueToPrintableValue(entry.varOrFunctionVar, entry.charValue, entry.variableId);
                                                                            generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);
                                                                            ismodifiable = 0;
                                                                        }
                            |   charVariableStatement '=' VARIABLE  {
                                                                        // get char variable
                                                                        // printf("here 1\n");
                                                                        VariableEntry entry = getCharVarValue($3, peek(&blockStack), yylineno);
                                                                        varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($1);
                                                                        typeIdentifier = checkVariableTypeValue($1, peek(&blockStack));
                                                                        
                                                                        // insert dummy variable value
                                                                        insertVariable(0, $1, 0, 2, entry, yylineno, peek(&blockStack));
                                                                        char* finalName = returnCharValueToPrintableValue(entry.varOrFunctionVar, entry.charValue, entry.variableId);
                                                                        generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);
                                                                        ismodifiable = 0;

                                                                        printf("char variable decleration and definition \n");
                                                                    }
                            |   stringVariableStatement '=' STRING_VALUE    {
                                                                                // create string_value                                                            
                                                                                VariableEntry entry = createEmptyVariableEntry();
                                                                                entry.varOrFunctionVar = 4;
                                                                                entry.variableType = 3;
                                                                                entry.stringValue = $3;
                                                                                varOrFunctionVarIndetifier = 4; 
                                                                                typeIdentifier = 3;

                                                                                // insert dummy variable value
                                                                                insertVariable(0, $1, 0, 3, entry, yylineno, peek(&blockStack));
                                                                                char* finalName = returnStringValueToPrintableValue(entry.varOrFunctionVar, entry.stringValue, entry.variableId);
                                                                                generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);                                                                                
                                                                                ismodifiable = 0;

                                                                                printf("string variable decleration and definition \n");
                                                                            }
                            |   stringVariableStatement '=' VARIABLE        {
                                                                                // get string variable
                                                                                VariableEntry entry = getStringVarValue($3, peek(&blockStack), yylineno);
                                                                                varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($1);
                                                                                typeIdentifier = checkVariableTypeValue($1, peek(&blockStack));

                                                                                // insert dummy variable value
                                                                                insertVariable(0, $1, 0, 3, entry, yylineno, peek(&blockStack));
                                                                                char* finalName = returnStringValueToPrintableValue(entry.varOrFunctionVar, entry.stringValue, entry.variableId);
                                                                                generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);                                                                                
                                                                                ismodifiable = 0;

                                                                                printf("string variable decleration and definition \n");
                                                                            }
                            |   boolVariableStatement '=' boolFactor    {
                                                                            // create bool value
                                                                            VariableEntry entry = createEmptyVariableEntry();
                                                                            entry.varOrFunctionVar = 4;
                                                                            entry.variableType = 4;
                                                                            entry.boolValue = $3;
                                                                            varOrFunctionVarIndetifier = 4; 
                                                                            typeIdentifier = 4;

                                                                            // insert dummy variable value
                                                                            insertVariable(0, $1, 0, 4, entry, yylineno, peek(&blockStack));
                                                                            char* finalName = returnBoolValueToPrintableValue(entry.varOrFunctionVar, entry.boolValue, entry.variableId);
                                                                            generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);                                                                                
                                                                            ismodifiable = 0;
                                                                            printf("bool variable decleration and definition\n");
                                                                        }
                            |   boolVariableStatement '=' VARIABLE      {
                                                                            // get bool variable
                                                                            VariableEntry entry = getBoolVarValue($3, peek(&blockStack), yylineno);
                                                                            varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($1);
                                                                            typeIdentifier = checkVariableTypeValue($1, peek(&blockStack));


                                                                            // insert dummy variable value
                                                                            insertVariable(0, $1, 0, 4, entry, yylineno, peek(&blockStack));
                                                                            char* finalName = returnBoolValueToPrintableValue(entry.varOrFunctionVar, entry.boolValue, entry.variableId);
                                                                            generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);                                                                                
                                                                            ismodifiable = 0;
                                                                            printf("bool variable decleration and definition\n");
                                                                        }
                            |   ENUM VARIABLE VARIABLE '=' ENUM_IDENTIFIER  {
                                                                                createAndAssignNewEnum(1, $2, $3, $5, yylineno, peek(&blockStack));
                                                                                generateEnumVariable($2, $3);
                                                                                char* finalName = returnEnumValueToPrintableValue(4, $5, NULL);
                                                                                generateAssignVariableValue($3, finalName, 0);

                                                                                ismodifiable = 0;
                                                                            }
                            |   ENUM VARIABLE VARIABLE '=' VARIABLE {
                                                                        // get enum value
                                                                        VariableEntry entry = getEnumVarValue($5, peek(&blockStack), yylineno, 1);
                                                                        if (strcmp($2, entry.enumParent) != 0)
                                                                        {
                                                                            fprintf(stderr, "Error at %d: Incompatible ENUM types\n", yylineno);
                                                                            exit(EXIT_FAILURE);
                                                                        }

                                                                        createAndAssignNewEnum(1, $2, $3, entry.enumValue, yylineno, peek(&blockStack));


                                                                        generateEnumVariable($2, $3);
                                                                        char* finalName = returnEnumValueToPrintableValue(0, $5, $5);
                                                                        generateAssignVariableValue($3, finalName, entry.varOrFunctionVar);
                                                                        ismodifiable = 0;
                                                                    }
                            ;

intVariableStatement         :   INT VARIABLE    {$$ = $2; generateInitializeVariable($2);}
                            ;

floatVariableStatement      :   FLOAT VARIABLE  {$$ = $2; generateInitializeVariable($2);}
                            ;

charVariableStatement       :   CHAR VARIABLE   {$$ = $2; generateInitializeVariable($2);}
                            ;

stringVariableStatement     :   STRING VARIABLE  {$$ = $2; generateInitializeVariable($2);}
                            ;

boolVariableStatement       :   BOOL VARIABLE   {$$ = $2; generateInitializeVariable($2);}
                            ;

assignStatement             :   VARIABLE '=' funcCallStatement  {
                                                                    // get the function
                                                                    VariableEntry entry2 = $3;

                                                                    // if void throw an error
                                                                    if (entry2.variableType == 5)
                                                                    {
                                                                        fprintf(stderr, "Error at %d: invalid conversion from void'\n", yylineno);
                                                                        exit(EXIT_FAILURE);
                                                                    }

                                                                    // get the variable
                                                                    VariableEntry entry1 = getValueForAssignmentStatementFunctionCall($1, entry2.variableType, peek(&blockStack), yylineno);
                                                                    // printf("hello wwewewew\n");

                                                                    // get its index and convert it to unresolvable
                                                                    int variableIndex = getVariableId(entry1.variableId);
                                                                    if (entry2.variableType == 3)
                                                                    {
                                                                        SymbolTableVariables[variableIndex].stringValue = strdup("aviodableee3");

                                                                    }
                                                                    SymbolTableVariables[variableIndex].varOrFunctionVar = 5;
                                                                    SymbolTableVariables[variableIndex].functionVariableId = strdup(entry2.variableId);
                                                                    generateCallFunction(entry2.variableId);


                                                                }
                            |   VARIABLE '=' VARIABLE   {
                                                            int currentType = getVariableType($3, peek(&blockStack), yylineno);
                                                            if (currentType == 0) // int
                                                            {
                                                                // get right variable
                                                                VariableEntry entry = getIntVarValue($3, peek(&blockStack), yylineno);
                                                                varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($3);
                                                                typeIdentifier = checkVariableTypeValue($3, peek(&blockStack));

                                                                // apply on left variable
                                                                assignIntVariableValue($1, entry, peek(&blockStack), yylineno);

                                                                int variableIndex = getVariableId($1);
                                                                if (SymbolTableVariables[variableIndex].varOrFunctionVar == 5)
                                                                {
                                                                    generateCallFunction(entry.functionVariableId);
                                                                }
                                                                else
                                                                {
                                                                    char* finalName = returnIntValueToPrintableValue(entry.varOrFunctionVar, entry.intValue, entry.variableId);
                                                                    generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);

                                                                }
                                                                printf("int variable assignment\n");
                                                                uniqueIdArrayCounter = 0;

                                                            }
                                                            else if (currentType == 1) // float
                                                            {
                                                                // get right variable
                                                                VariableEntry entry = getFloatVarValue($3, peek(&blockStack), yylineno);
                                                                varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($3);
                                                                typeIdentifier = checkVariableTypeValue($3, peek(&blockStack));

                                                                // apply on left variable
                                                                assignFloatVariableValue($1, entry, peek(&blockStack), yylineno);

                                                                int variableIndex = getVariableId($1);
                                                                if (SymbolTableVariables[variableIndex].varOrFunctionVar == 5)
                                                                {
                                                                    generateCallFunction(entry.functionVariableId);
                                                                }
                                                                else
                                                                {
                                                                    char* finalName = returnIntValueToPrintableValue(entry.varOrFunctionVar, entry.intValue, entry.variableId);
                                                                    generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);

                                                                }
                                                                printf("float variable assignment\n");
                                                                uniqueIdArrayCounter = 0;
                                                            }
                                                            else if (currentType == 2) // char
                                                            {
                                                                // get char variable
                                                                VariableEntry entry = getCharVarValue($3, peek(&blockStack), yylineno);
                                                                varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($1);
                                                                typeIdentifier = checkVariableTypeValue($1, peek(&blockStack));

                                                                // apply on left variable
                                                                assignCharVariableValue($1, entry, peek(&blockStack), yylineno);

                                                                int variableIndex = getVariableId($1);
                                                                if (SymbolTableVariables[variableIndex].varOrFunctionVar == 5)
                                                                {
                                                                    generateCallFunction(entry.functionVariableId);
                                                                }
                                                                else
                                                                {
                                                                    char* finalName = returnCharValueToPrintableValue(entry.varOrFunctionVar, entry.charValue, entry.variableId);
                                                                    generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);

                                                                }
                                                                printf("char variable assignment\n");
                                                                uniqueIdArrayCounter = 0;
                                                            }
                                                            else if (currentType == 3) // string
                                                            {
                                                                // get string variable
                                                                VariableEntry entry = getStringVarValue($3, peek(&blockStack), yylineno);
                                                                varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($1);
                                                                typeIdentifier = checkVariableTypeValue($1, peek(&blockStack));

                                                            
                                                                // printf("ahaaa %s\n", entry.stringValue);// !!!!!!!!!!!! NULL !!!!!!!!!!

                                                                // apply on left value
                                                                assignStringVariableValue($1, entry, peek(&blockStack), yylineno);

                                                                int variableIndex = getVariableId($1);
                                                                if (SymbolTableVariables[variableIndex].varOrFunctionVar == 5)
                                                                {
                                                                    generateCallFunction(entry.functionVariableId);
                                                                }
                                                                else
                                                                {
                                                                    char* finalName = returnStringValueToPrintableValue(entry.varOrFunctionVar, entry.stringValue, entry.variableId);
                                                                    generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);

                                                                }
                                                                printf("string variable assignment\n");

                                                            }
                                                            else if (currentType == 4) // bool
                                                            {
                                                                // get bool variable
                                                                VariableEntry entry = getBoolVarValue($3, peek(&blockStack), yylineno);
                                                                varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($1);
                                                                typeIdentifier = checkVariableTypeValue($1, peek(&blockStack));

                                                                // apply on left value
                                                                assignBoolVariableValue($1, entry, peek(&blockStack), yylineno);

                                                                int variableIndex = getVariableId($1);
                                                                if (SymbolTableVariables[variableIndex].varOrFunctionVar == 5)
                                                                {
                                                                    generateCallFunction(entry.functionVariableId);
                                                                }
                                                                else
                                                                {
                                                                    char* finalName = returnBoolValueToPrintableValue(entry.varOrFunctionVar, entry.boolValue, entry.variableId);
                                                                    generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);

                                                                }

                                                                printf("bool variable assignment\n");
                                                            }
                                                            else if (currentType == 5)
                                                            {
                                                                VariableEntry entry1 = getEnumVarValue($1, peek(&blockStack), yylineno, 0);
                                                                VariableEntry entry2 = getEnumVarValue($3, peek(&blockStack), yylineno, 0);

                                                                // check for incompatible enum types
                                                                if (strcmp(entry1.enumParent, entry2.enumParent) != 0)
                                                                {
                                                                    fprintf(stderr, "Error at %d: Incompatible ENUM types\n", yylineno);
                                                                    exit(EXIT_FAILURE);
                                                                }

                                                                createAndAssignNewEnum(2, entry1.enumParent, $1, entry2.enumValue, yylineno, peek(&blockStack));
                                                                generateAssignVariableValue($1, $3, 0);
                                                            }
                                                            // printf("hello here\n");
                                                        }
                            |   VARIABLE '=' intExpr    {
                                                            VariableEntry entry = $3;
                                                            assignIntVariableValue($1, entry, peek(&blockStack), yylineno);
                                                            char* finalName = returnIntValueToPrintableValue(entry.varOrFunctionVar, entry.intValue, entry.variableId);
                                                            generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);
                                                            printf("int variable assignment\n");
                                                            uniqueIdArrayCounter = 0;


                                                        }
                            |   VARIABLE '=' floatExpr  {
                                                            VariableEntry entry = $3;
                                                            assignFloatVariableValue($1, entry, peek(&blockStack), yylineno);
                                                            char* finalName = returnIntValueToPrintableValue(entry.varOrFunctionVar, entry.intValue, entry.variableId);
                                                            generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);
                                                            printf("float variable assignment\n");
                                                            uniqueIdArrayCounter = 0;
                                                        }
                            |   VARIABLE '=' CHAR_VALUE {
                                                            // create char_value                                                            
                                                            VariableEntry entry = createEmptyVariableEntry();
                                                            entry.varOrFunctionVar = 4;
                                                            entry.variableType = 2;
                                                            entry.charValue = $3;
                                                            varOrFunctionVarIndetifier = 4; 
                                                            typeIdentifier = 2;

                                                            // assign the value
                                                            assignCharVariableValue($1, entry, peek(&blockStack), yylineno);
                                                            char* finalName = returnCharValueToPrintableValue(entry.varOrFunctionVar, entry.charValue, entry.variableId);
                                                            generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);
                                                            printf("char variable assignment\n");
                                                            uniqueIdArrayCounter = 0;
                                                        }
                            |   VARIABLE '=' STRING_VALUE   {
                                                                // create string_value                                                            
                                                                VariableEntry entry = createEmptyVariableEntry();
                                                                entry.varOrFunctionVar = 4;
                                                                entry.variableType = 3;
                                                                entry.stringValue = strdup($3);
                                                                varOrFunctionVarIndetifier = 4; 
                                                                typeIdentifier = 3;
                                                                // assign the value
                                                                assignStringVariableValue($1, entry, peek(&blockStack), yylineno);
                                                                char* finalName = returnStringValueToPrintableValue(entry.varOrFunctionVar, entry.stringValue, entry.variableId);
                                                                generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);
                                                                printf("string variable assignment\n");
                                                                uniqueIdArrayCounter = 0;
                                                            }
                            |   VARIABLE '=' boolFactor {
                                                            // create bool value
                                                            VariableEntry entry = createEmptyVariableEntry();
                                                            entry.varOrFunctionVar = 4;
                                                            entry.variableType = 4;
                                                            entry.boolValue = $3;
                                                            varOrFunctionVarIndetifier = 4; 
                                                            typeIdentifier = 4;

                                                            // assign the value
                                                            assignBoolVariableValue($1, entry, peek(&blockStack), yylineno);
                                                            char* finalName = returnBoolValueToPrintableValue(entry.varOrFunctionVar, entry.boolValue, entry.variableId);
                                                            generateAssignVariableValue($1, finalName, entry.varOrFunctionVar);
                                                            printf("bool variable assignment\n");
                                                            uniqueIdArrayCounter = 0;
                                                        }
                            /* |   VARIABLE '=' FALSE {printf("bool variable assignment'\n");} */
                            |   VARIABLE '=' ENUM_IDENTIFIER    {
                                                                    // try to get the first variable
                                                                    VariableEntry entry = getEnumVarValue($1, peek(&blockStack), yylineno, 0);
                                                                    createAndAssignNewEnum(2, entry.enumParent, $1, $3, yylineno, peek(&blockStack));
                                                                }
                            ;

/* 
conditionalStatement        :   ifStatement statement {;}
                            |   forStatement statement {;}
                            ; */

intExpr                     :   '(' intExpr ')' {$$ = $2;}
                            |   intExpr '+' intPriorityExpr {
                                                                VariableEntry entry = performIntOperation($1, $3, '+', yylineno);
                                                                $$ = entry;
                                                                printf("int addition\n");
                                                            }
                            |   intExpr '-' intPriorityExpr {
                                                                VariableEntry entry = performIntOperation($1, $3, '-', yylineno);
                                                                $$ = entry;
                                                                // printf("heere %s\n", entry.variableId);
                                                                printf("int substraction\n");
                                                            }
                            |   intPriorityExpr {$$ = $1;}
                            ;

intPriorityExpr             :   intPriorityExpr '*' intFactor   {
                                                                    VariableEntry entry = performIntOperation($1, $3, '*', yylineno);
                                                                    $$ = entry;
                                                                    // printf("%s\n", entry.variableId);
                                                                    printf("int multiplication\n");
                                                                    
                                                                    
                                                                }
                            |   intPriorityExpr '/' intFactor   {
                                                                    VariableEntry entry = performIntOperation($1, $3, '/', yylineno);
                                                                    $$ = entry;
                                                                    printf("int division\n");
                                                                }
                            |   intFactor   {
                                                printf("int factor\n");
                                                $$ = $1;
                                                VariableEntry entry = $1;
                                                intFactorToFile(entry);
                                            }
                            ;

intFactor                   :   INT_NUM {
                                            VariableEntry entry = createEmptyVariableEntry();
                                            entry.varOrFunctionVar = 4;
                                            entry.variableType = 0;
                                            entry.intValue = $1;
                                            $$ = entry; 

                                            varOrFunctionVarIndetifier = 4; 
                                            typeIdentifier = 0;
                                        }
                            |   VARIABLE    {
                                                VariableEntry entry = getIntVarValue($1, peek(&blockStack), yylineno);

                                                // throw error if it is a function (handled somewhere else)
                                                if (entry.varOrFunctionVar == 2) 
                                                {
                                                    fprintf(stderr, "Error at %d: \"%s\" is a function\n", yylineno, $1);
                                                    exit(EXIT_FAILURE);
                                                }
                                                $$ = entry;

                                                varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($1);
                                                typeIdentifier = checkVariableTypeValue($1, peek(&blockStack));
                                            }
                            |   '(' intExpr ')' {$$ = $2;}
                            ;
boolFactor                  :   TRUE    {$$ = 1;}
                            |   FALSE   {$$ = 0;}
                            ;

floatExpr                   :   '(' floatExpr ')' {$$ = $2;}
                            |   floatExpr '+' floatPriorityExpr {
                                                                    VariableEntry entry = performFloatOperation($1, $3, '+', yylineno);
                                                                    $$ = entry;
                                                                    printf("float addition\n");
                                                                }
                            |   floatExpr '-' floatPriorityExpr {
                                                                    VariableEntry entry = performFloatOperation($1, $3, '-', yylineno);
                                                                    $$ = entry;
                                                                    printf("float substraction\n");
                                                                }
                            |   floatPriorityExpr {$$ = $1;}
                            ;

floatPriorityExpr           :   floatPriorityExpr '*' floatFactor   {
                                                                        VariableEntry entry = performFloatOperation($1, $3, '*', yylineno);
                                                                        $$ = entry;
                                                                        printf("float multiplication\n");
                                                                    }
                            |   floatPriorityExpr '/' floatFactor   {
                                                                        VariableEntry entry = performFloatOperation($1, $3, '/', yylineno);
                                                                        $$ = entry;
                                                                        printf("float division\n");
                                                                    }
                            |   floatFactor {
                                                printf("float factor\n");
                                                $$ = $1;
                                                VariableEntry entry = $1;
                                                floatFactorToFile(entry);
                                            }
                            ;

floatFactor                 :   FLOAT_NUM   {
                                                VariableEntry entry = createEmptyVariableEntry();
                                                entry.varOrFunctionVar = 4;
                                                entry.variableType = 1;
                                                entry.floatValue = $1;
                                                $$ = entry; 

                                                varOrFunctionVarIndetifier = 4; 
                                                typeIdentifier = 1;
                                            }
                            |   VARIABLE    {
                                                VariableEntry entry = getFloatVarValue($1, peek(&blockStack), yylineno);

                                                // throw error if it is a function (handled somewhere else)
                                                if (entry.varOrFunctionVar == 2) 
                                                {
                                                    fprintf(stderr, "Error at %d: \"%s\" is a function\n", yylineno, $1);
                                                    exit(EXIT_FAILURE);
                                                }
                                                $$ = entry;

                                                varOrFunctionVarIndetifier = checkvarOrFunctionVarValue($1);
                                                typeIdentifier = checkVariableTypeValue($1, peek(&blockStack));
                                            }
                            |   '(' floatExpr ')' {$$ = $2;}
                            ;

/* 
ifStatement                 :   IF {;} '(' condition ')' ifOpenBracket body ifCloseBracket {printf("if statement\n");}
                            |   ELSE IF {;} '(' condition ')' ifOpenBracket body ifCloseBracket {printf("else if statement\n");}
                            |   ELSE {;} elseOpenBracket body elseCloseBracket {printf("else statement\n");}
                            ; */

ifStatement                 :   IF '(' condition ')'    {
                                                            startIf();
                                                            ComparisonEntry test = $3;
                                                            if (test.comparisonResult == 0)
                                                            {
                                                                fprintf(stderr, "Warning at %d: if is always false\n", yylineno);
                                                            }
                                                        } ifOpenBracket body ifCloseBracket ifTrailStatement {finishIfstatement(); printf("if statement\n");}
                            ;

ifTrailStatement            :   ELIF '(' condition ')'  {
                                                            startIf();
                                                            ComparisonEntry test = $3;
                                                            if (test.comparisonResult == 0)
                                                            {
                                                                fprintf(stderr, "Warning at %d: if is always false\n", yylineno);
                                                            }
                                                        } ifOpenBracket body ifCloseBracket  {printf("else if statment\n");} ifTrailStatement 
                            |   ELSE  elseOpenBracket body elseCloseBracket {printf("else statment\n");}
                            |
                            ;

/* ifBody                      :   statement
                            |
                            ; */

condition                   :   '(' condition ')' {$$ = $2;}
                            |   condition OR priorityCondtion {$$ = generateOrAndCondition($1, $3, 0);}
                            |   condition AND priorityCondtion {$$ = generateOrAndCondition($1, $3, 1);}
                            |   NOT condition {$$ = generateNotCondition($2);}
                            |   priorityCondtion {$$ = $1;}
                            ;

priorityCondtion            :   VARIABLE prioritySymbol VARIABLE    {
                                                                        int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);
                                                                        int varTypeOperand2 = getVariableType($3, peek(&blockStack), yylineno);
                                                                        if (varTypeOperand1 != varTypeOperand2)
                                                                        {
                                                                            fprintf(stderr, "Error at %d: Comparison type mismatch\n", yylineno);
                                                                            exit(EXIT_FAILURE);
                                                                        }

                                                                        // get the 2 variables from memory
                                                                        VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);
                                                                        VariableEntry entry2 = checkVarValue($3, varTypeOperand1, peek(&blockStack), yylineno);
                                                                        
                                                                        // -1: cannot determine, 0: false, 1: true
                                                                        int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);

                                                                        char* opString = getComparisonString($2);

                                                                        $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);
                                                                    }
                            |   VARIABLE prioritySymbol INT_NUM     {
                                                                        int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);
                                                                        if (varTypeOperand1 != 0) // int
                                                                        {
                                                                            fprintf(stderr, "Error at %d: Comparison type mismatch\n", yylineno);
                                                                            exit(EXIT_FAILURE);
                                                                        }
                                                                        // prepare the first entry
                                                                        VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);

                                                                        //prepare the second entry
                                                                        VariableEntry entry2 = createEmptyVariableEntry();
                                                                        entry2.varOrFunctionVar = 4;
                                                                        entry2.variableType = 0;
                                                                        entry2.intValue = $3;
                                                                        
                                                                        // -1: cannot determine, 0: false, 1: true
                                                                        int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                        char* opString = getComparisonString($2);
                                                                        $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);

                                                                    }
                            |   VARIABLE prioritySymbol FLOAT_NUM   {
                                                                        int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);
                                                                        if (varTypeOperand1 != 1) // float
                                                                        {
                                                                            fprintf(stderr, "Error at %d: Comparison type mismatch\n", yylineno);
                                                                            exit(EXIT_FAILURE);
                                                                        }
                                                                        // prepare the first entry
                                                                        VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);

                                                                        //prepare the second entry
                                                                        VariableEntry entry2 = createEmptyVariableEntry();
                                                                        entry2.varOrFunctionVar = 4;
                                                                        entry2.variableType = 1;
                                                                        entry2.floatValue = $3;

                                                                        // -1: cannot determine, 0: false, 1: true
                                                                        int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                        char* opString = getComparisonString($2);
                                                                        $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);
                                                                    }
                            |   VARIABLE prioritySymbol CHAR_VALUE  {
                                                                        int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);
                                                                        if (varTypeOperand1 != 2) // char
                                                                        {
                                                                            fprintf(stderr, "Error at %d: Comparison type mismatch\n", yylineno);
                                                                            exit(EXIT_FAILURE);
                                                                        }
                                                                        // prepare the first entry
                                                                        VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);

                                                                        //prepare the second entry
                                                                        VariableEntry entry2 = createEmptyVariableEntry();
                                                                        entry2.varOrFunctionVar = 4;
                                                                        entry2.variableType = 2;
                                                                        entry2.charValue = $3;

                                                                        // -1: cannot determine, 0: false, 1: true
                                                                        int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                        char* opString = getComparisonString($2);
                                                                        $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);
                                                                    }
                            |   VARIABLE prioritySymbol STRING_VALUE    {
                                                                            int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);
                                                                            if (varTypeOperand1 != 3) // string
                                                                            {
                                                                                fprintf(stderr, "Error at %d: Comparison type mismatch\n", yylineno);
                                                                                exit(EXIT_FAILURE);
                                                                            }
                                                                            // prepare the first entry
                                                                            VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);
                                                                            
                                                                            // prepare the second eny
                                                                            VariableEntry entry2 = createEmptyVariableEntry();
                                                                            entry2.varOrFunctionVar = 4;
                                                                            entry2.variableType = 3;
                                                                            entry2.stringValue = $3;
                                                                            

                                                                            // -1: cannot determine, 0: false, 1: true
                                                                            int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                            char* opString = getComparisonString($2);
                                                                            $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);

                                                                        } 
                            |   VARIABLE prioritySymbol boolFactor      {
                                                                            int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);
                                                                            if (varTypeOperand1 != 4) // bool
                                                                            {
                                                                                fprintf(stderr, "Error at %d: Comparison type mismatch\n", yylineno);
                                                                                exit(EXIT_FAILURE);
                                                                            }
                                                                            // prepare the first entry
                                                                            VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);
                                                                            
                                                                            // prepare the second entry
                                                                            VariableEntry entry2 = createEmptyVariableEntry();
                                                                            entry2.varOrFunctionVar = 4;
                                                                            entry2.variableType = 4;
                                                                            entry2.boolValue = $3;
                                                                            

                                                                            // -1: cannot determine, 0: false, 1: true
                                                                            int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                            char* opString = getComparisonString($2);
                                                                            $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);
                                                                        }
                            |   INT_NUM  prioritySymbol INT_NUM         {
                                                                            // prepare the first entry
                                                                            VariableEntry entry1 = createEmptyVariableEntry();
                                                                            entry1.varOrFunctionVar = 4;
                                                                            entry1.variableType = 0;
                                                                            entry1.intValue = $1;

                                                                            // prepare the second entry
                                                                            VariableEntry entry2 = createEmptyVariableEntry();
                                                                            entry2.varOrFunctionVar = 4;
                                                                            entry2.variableType = 0;
                                                                            entry2.intValue = $3;

                                                                            // -1: cannot determine, 0: false, 1: true
                                                                            int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                            char* opString = getComparisonString($2);
                                                                            $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);
                                                                        }
                            |   FLOAT_NUM prioritySymbol FLOAT_NUM  {
                                                                        //prepare the first entry
                                                                        VariableEntry entry1 = createEmptyVariableEntry();
                                                                        entry1.varOrFunctionVar = 4;
                                                                        entry1.variableType = 1;
                                                                        entry1.floatValue = $1;

                                                                        //prepare the second entry
                                                                        VariableEntry entry2 = createEmptyVariableEntry();
                                                                        entry2.varOrFunctionVar = 4;
                                                                        entry2.variableType = 1;
                                                                        entry2.floatValue = $3;

                                                                        // -1: cannot determine, 0: false, 1: true
                                                                        int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                        char* opString = getComparisonString($2);
                                                                        $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);
                                                                    }
                            |   CHAR_VALUE prioritySymbol CHAR_VALUE    {
                                                                            //prepare the first entry
                                                                            VariableEntry entry1 = createEmptyVariableEntry();
                                                                            entry1.varOrFunctionVar = 4;
                                                                            entry1.variableType = 2;
                                                                            entry1.charValue = $1;

                                                                            //prepare the second entry
                                                                            VariableEntry entry2 = createEmptyVariableEntry();
                                                                            entry2.varOrFunctionVar = 4;
                                                                            entry2.variableType = 2;
                                                                            entry2.charValue = $3;

                                                                            // -1: cannot determine, 0: false, 1: true
                                                                            int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                            char* opString = getComparisonString($2);
                                                                            $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);
                                                                        }
                            |   STRING_VALUE prioritySymbol STRING_VALUE    {
                                                                                // prepare the second eny
                                                                                VariableEntry entry1 = createEmptyVariableEntry();
                                                                                entry1.varOrFunctionVar = 4;
                                                                                entry1.variableType = 3;
                                                                                entry1.stringValue = $1;
                                                                                

                                                                                // prepare the second eny
                                                                                VariableEntry entry2 = createEmptyVariableEntry();
                                                                                entry2.varOrFunctionVar = 4;
                                                                                entry2.variableType = 3;
                                                                                entry2.stringValue = $3;
                                                                               
                                                                               // -1: cannot determine, 0: false, 1: true
                                                                                int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                                char* opString = getComparisonString($2);
                                                                                $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);
                                                                            }
                            |   boolFactor prioritySymbol boolFactor    {
                                                                            // prepare the first entry
                                                                            VariableEntry entry1 = createEmptyVariableEntry();
                                                                            entry1.varOrFunctionVar = 4;
                                                                            entry1.variableType = 4;
                                                                            entry1.boolValue = $1;

                                                                            // prepare the second entry
                                                                            VariableEntry entry2 = createEmptyVariableEntry();
                                                                            entry2.varOrFunctionVar = 4;
                                                                            entry2.variableType = 4;
                                                                            entry2.boolValue = $3;

                                                                            // -1: cannot determine, 0: false, 1: true
                                                                            int comparisonResult = comparisonLogic(entry1, entry2, $2, peek(&blockStack), yylineno);
                                                                            char* opString = getComparisonString($2);
                                                                            $$ = generatePriorityComparison(returnValueToPrintableValue(entry1), returnValueToPrintableValue(entry2), opString, comparisonResult);
                                                                        }
                            |   VARIABLE    {   
                                                int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);
                                                
                                                // prepare the first entry
                                                VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);

                                                int comparisonResult = singleComparisonLogic(entry1);
                                                $$ = generateSingleCondtion(entry1, comparisonResult);
                                                
                                            }
                            |   INT_NUM     {
                                                // prepare the first entry
                                                VariableEntry entry1 = createEmptyVariableEntry();
                                                entry1.varOrFunctionVar = 4;
                                                entry1.variableType = 0;
                                                entry1.intValue = $1;

                                                int comparisonResult = $1 != 0 ? 1 : 0;
                                                $$ = generateSingleCondtion(entry1, comparisonResult);
                                            }
                            |   FLOAT_NUM   {
                                                //prepare the first entry
                                                VariableEntry entry1 = createEmptyVariableEntry();
                                                entry1.varOrFunctionVar = 4;
                                                entry1.variableType = 1;
                                                entry1.floatValue = $1;

                                                int comparisonResult = $1 != 0.0 ? 1 : 0;
                                                $$ = generateSingleCondtion(entry1, comparisonResult);
                                            }
                            |   CHAR_VALUE  {
                                                //prepare the first entry
                                                VariableEntry entry1 = createEmptyVariableEntry();
                                                entry1.varOrFunctionVar = 4;
                                                entry1.variableType = 2;
                                                entry1.charValue = $1;

                                                int comparisonResult = 1;
                                                $$ = generateSingleCondtion(entry1, comparisonResult);
                                            }
                            |   STRING_VALUE    {
                                                    // prepare the first entry
                                                    VariableEntry entry1 = createEmptyVariableEntry();
                                                    entry1.varOrFunctionVar = 4;
                                                    entry1.variableType = 3;
                                                    entry1.stringValue = $1;

                                                    int comparisonResult = 1;
                                                    $$ = generateSingleCondtion(entry1, comparisonResult);
                                                }
                            |   boolFactor  {
                                                // prepare the first entry
                                                VariableEntry entry1 = createEmptyVariableEntry();
                                                entry1.varOrFunctionVar = 4;
                                                entry1.variableType = 4;
                                                entry1.boolValue = $1;

                                                int comparisonResult = $1 == 1 ? 1 : 0;
                                                $$ = generateSingleCondtion(entry1, comparisonResult);
                                            }
                            ;

prioritySymbol              :   EQ      {$$ = 0;}
                            |   NEQ     {$$ = 1;}
                            |   GT      {$$ = 2;}
                            |   GE      {$$ = 3;}
                            |   LT      {$$ = 4;}
                            |   LE      {$$ = 5;}
                            ;

ifOpenBracket               :   '{' {
                                        int newCurrentBlock = addBlock(3, peek(&blockStack));
                                        push(&blockStack, newCurrentBlock);
                                        printf("if open bracket\n");bracketCount++;
                                    }
                            ;

ifCloseBracket              :   '}' {
                                        printf("if close bracket\n");
                                        endIf();
                                        pop(&blockStack);
                                        bracketCount--;
                                    }
                            ;

elseOpenBracket             :   '{' {openElse(); printf("else open bracket\n");bracketCount++;}
                            ;

elseCloseBracket            :   '}' {closeElse(); printf("else close bracket\n");bracketCount--;}
                            ;

enumStatement               :   enumStartStatement '{' enumList '}' {
                                                                            // create empty function variable
                                                                            VariableEntry entry1 = createEmptyVariableEntry();
                                                                            entry1.varOrFunctionVar = 3;

                                                                            // add enum type to the symbol table
                                                                            insertVariable(4, $1, 2, globalFunctionVariableType, entry1, yylineno, peek(&blockStack));
                                                                            
                                                                            endEnumType($1);

                                                                            enumTypeValuesounter = 0;
                                                                    }
                            ;
enumStartStatement          :   ENUM VARIABLE   {
                                                    // check for variable
                                                    VariableEntry tempVariable = findVariable($2, peek(&blockStack));
                                                    // variable already exists at this block level or is the name of a function
                                                    if (tempVariable.variableId != NULL || findFunction(tempVariable.variableId) != -1)
                                                    {
                                                        // throw error
                                                        fprintf(stderr, "Error at %d: Failed to create ENUM type. redeclaration of variable %s\n", yylineno, $2);
                                                        exit(EXIT_FAILURE);
                                                    }
                                                    
                                                    $$ = $2; 
                                                    globalEnumTypeVariableName = strdup($2); 
                                                    
                                                    enumTypeValuesounter = 0;
                                                    startEnumType($2);
                                                }
                            ;

enumList                    :   ENUM_IDENTIFIER {
                                                    enumTypeValuesArray[enumTypeValuesounter] = strdup($1); 
                                                    enumTypeValuesounter++;
                                                    addEnumValues(globalEnumTypeVariableName, $1);
                                                }
                            |   ENUM_IDENTIFIER ',' enumList {
                                                                enumTypeValuesArray[enumTypeValuesounter] = strdup($1);
                                                                enumTypeValuesounter++;
                                                                addEnumValues(globalEnumTypeVariableName, $1);
                                                            }
                            ;

whileStatement              :   whileStartStatement '(' condition ')' {
                                                            whileLoopAfterCondition();
                                                            ComparisonEntry test = $3;
                                                            if (test.comparisonResult == 0)
                                                            {
                                                                fprintf(stderr, "Warning at %d: while loop statement is always false\n", yylineno);
                                                            }
                                                        } whileOpenBracket body whileCloseBracket {printf("while statement\n");}
                            ;

whileStartStatement         :   WHILE   {startWhile();}
                            ;

/* whileBody                   :   statement
                            |
                            ; */

whileOpenBracket            : '{'   {
                                        int newCurrentBlock = addBlock(3, peek(&blockStack));
                                        push(&blockStack, newCurrentBlock);
                                        printf("while open bracket\n");
                                        bracketCount++;
                                    }
                            ;

whileCloseBracket           : '}'   {
                                        finishWhileLoopStatement();
                                        printf("while close bracket\n");
                                        pop(&blockStack);
                                        bracketCount--;
                                    }
                            ;


repeatStatement             :   REPEAT repeatOpenBracket body repeatCloseBracket UNTIL '(' condition ')'    {
                                                                                                                finishRepeatUntilStatement();
                                                                                                                pop(&blockStack);
                                                                                                            }

/* repeatBody                  :   statement
                            |
                            ; */

repeatOpenBracket           : '{'   {
                                        startRepeatUntil();
                                        int newCurrentBlock = addBlock(3, peek(&blockStack));
                                        push(&blockStack, newCurrentBlock);
                                        printf("repeat open bracket\n");
                                        bracketCount++;
                                    }
                            ;

repeatCloseBracket          : '}' {printf("repeat close bracket\n");bracketCount--;}
                            ;


forStatement                : forStartAssignment '(' forAssignStatement forSemi1 condition  {
                                                                                                ComparisonEntry test = $5;
                                                                                                if (test.comparisonResult == 0)
                                                                                                {
                                                                                                    fprintf(stderr, "Warning at %d: for loop statement is always false\n", yylineno);
                                                                                                }
                                                                                            } forSemi2 forIncrement ')' forOpenBracket body forCloseBracket { printf("for statement\n");}
                            ;

forStartAssignment          :   FOR {
                                        int newCurrentBlock = addBlock(3, peek(&blockStack));
                                        push(&blockStack, newCurrentBlock);    
                                    }
                            ;

forAssignStatement          :   defStatement
                            |   assignStatement
                            ;

/* 
forBody                     :   statement
                            |
                            ; */

forIncrement                :   VARIABLE INC {;}
                            |   VARIABLE DEC {;}
                            |   assignStatement {;}
                            ;

forSemi1                    :   ';' {forLoopSemiColon1();}
                            ;

forSemi2                    :   ';' {forLoopSemiColon2();}
                            ;

forOpenBracket              :   '{' {printf("for open bracket\n");bracketCount++;}
                            ;

forCloseBracket             :   '}' {
                                        finishForLoopStatement();
                                        pop(&blockStack);
                                        printf("for close bracket\n");
                                        bracketCount--;
                                    }
                            ;

body                        :   statement
                            |
                            ;

switchStatement             :   SWITCH {startSwitchStatement();} '('  switchVariable ')' switchBody {finishSwitchStatement(); printf("switch statement\n");}
                            ;

switchVariable              :   VARIABLE    {
                                                // check if variable exists
                                                int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);
                                                
                                                // check if it is initialized
                                                VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);
                                                
                                                // check if it is not a function
                                                if(findFunction($1) != -1) {
                                                    fprintf(stderr, "Error at %d: \"%s\" is a function\n", yylineno, $1);
                                                    exit(EXIT_FAILURE);
                                                }
                                                // used to check for type compatiblities
                                                globalSwitchVariableType = varTypeOperand1;
                                                globalSwitchVariableName = strdup($1);
                                            }

switchBody                  :   openBlock caseStatement closeBlock
                            |   openBlock caseStatement defaultStatement BREAK ';' closeBlock
                            ;

caseStatement               :   CASE caseComparator ':' {
                                                            int newCurrentBlock = addBlock(3, peek(&blockStack));
                                                            push(&blockStack, newCurrentBlock);
                                                        } body BREAK ';' {
                                                                            switchCaseEnd();
                                                                            pop(&blockStack);
                                                                        }
                            |   caseStatement caseStatement
                            ;

caseComparator              :   INT_NUM     {
                                                if (globalSwitchVariableType != 0)
                                                {
                                                    fprintf(stderr, "Error at %d: case label can not be an integer constant\n", yylineno);
                                                    exit(EXIT_FAILURE);
                                                }
                                                // prepare the first entry
                                                VariableEntry entry1 = createEmptyVariableEntry();
                                                entry1.varOrFunctionVar = 4;
                                                entry1.variableType = 0;
                                                entry1.intValue = $1;
                                                
                                                char* finalName = returnValueToPrintableValue(entry1);
                                                switchCaseStart(globalSwitchVariableName, finalName);
                                                
                                            }
                            |   FLOAT_NUM   {
                                                if (globalSwitchVariableType != 1)
                                                {
                                                    fprintf(stderr, "Error at %d: case label can not be a float constant\n", yylineno);
                                                    exit(EXIT_FAILURE);
                                                }
                                            }
                            |   CHAR_VALUE  {
                                                if (globalSwitchVariableType != 2)
                                                {
                                                    fprintf(stderr, "Error at %d: case label can not be a char constant\n", yylineno);
                                                    exit(EXIT_FAILURE);
                                                }
                                            }
                            |   STRING_VALUE    {
                                                    if (globalSwitchVariableType != 3)
                                                    {
                                                        fprintf(stderr, "Error at %d: case label can not be a string constant\n", yylineno);
                                                        exit(EXIT_FAILURE);
                                                    }
                                                }
                            |   boolFactor  {
                                                if (globalSwitchVariableType != 4)
                                                {
                                                    fprintf(stderr, "Error at %d: case label does not reduce to a bool constant\n", yylineno);
                                                    exit(EXIT_FAILURE);
                                                }
                                            }
                            |   ENUM_IDENTIFIER {
                                                    if (globalSwitchVariableType != 5)
                                                    {
                                                        fprintf(stderr, "Error at %d: case label does not reduce to an enum constant\n", yylineno);
                                                        exit(EXIT_FAILURE);
                                                    }
                                                }
                            ;

defaultStatement            :   DEFAULT ':' {
                                                switchDefaultCase();
                                                int newCurrentBlock = addBlock(3, peek(&blockStack));
                                                push(&blockStack, newCurrentBlock);
                                                
                                            } body {pop(&blockStack);}

funcStatement               :   specialFuncType '(' argument ')' openFunctionBlock funcBody closeFunctionBlock    {
                                                                                                                                // create empty function variable
                                                                                                                                VariableEntry entry1 = createEmptyVariableEntry();
                                                                                                                                entry1.varOrFunctionVar = 2;
                                                                                                                                entry1.variableType = globalFunctionVariableType;

                                                                                                                                // add function entry to the symbol table
                                                                                                                                insertVariable(3, $1, 2, globalFunctionVariableType, entry1, yylineno, peek(&blockStack));

                                                                                                                                globalFunctionVariableType = -1;

                                                                                                                                finishFunctionDecleration();
                                                                                                                            } 
                            ;

openFunctionBlock           :   '{' {startFunction(globalFunctionVariableName); bracketCount++;}
                            ;

closeFunctionBlock          :   '}' {
                                        if (theFunctionReturnedSomething == 0)
                                        {
                                            fprintf(stderr, "Error at %d: non-void function should return a value\n", yylineno);
                                            exit(EXIT_FAILURE);
                                        }
                                        pop(&blockStack);
                                        bracketCount--;
                                        checkBrackets();
                                    }


specialFuncType             :   funcType VARIABLE   {
                                                        // check for variable
                                                        VariableEntry tempVariable = findVariable($2, peek(&blockStack));
                                                        // variable already exists at this block level or is the name of a function
                                                        if (tempVariable.variableId != NULL || findFunction(tempVariable.variableId) != -1)
                                                        {
                                                            // throw error
                                                            fprintf(stderr, "Error at %d: redeclaration of variable %s\n", yylineno, $2);
                                                            exit(EXIT_FAILURE);
                                                        }


                                                        $$ = $2;
                                                        functionArgumentTypesCounter = 0;
                                                        int newCurrentBlock = addBlock(3, peek(&blockStack));
                                                        push(&blockStack, newCurrentBlock);    

                                                        globalFunctionVariableName = strdup($2);
                                                    }
                            ;

funcType                    :   FUNC INT    {globalFunctionVariableType = 0; theFunctionReturnedSomething = 0;}
                            |   FUNC FLOAT  {globalFunctionVariableType = 1; theFunctionReturnedSomething = 0;}
                            |   FUNC CHAR   {globalFunctionVariableType = 2; theFunctionReturnedSomething = 0;}
                            |   FUNC STRING {globalFunctionVariableType = 3; theFunctionReturnedSomething = 0;}
                            |   FUNC BOOL   {globalFunctionVariableType = 4; theFunctionReturnedSomething = 0;}
                            |   FUNC VOID   {globalFunctionVariableType = 5;}
                            ;

argument                    :   argumentIdentifier arguments
                            |
                            ;

arguments                   :   ',' argumentIdentifier arguments
                            |
                            ;


argumentIdentifier          :   INT VARIABLE    {
                                                    // add it to the symbol table
                                                    VariableEntry entry = createEmptyVariableEntry();
                                                    insertVariable(0, $2, 0, 0, entry, yylineno, peek(&blockStack));
                                                    generateInitializeVariable($2);

                                                    // add it to the function
                                                    functionArgumentTypesArray[functionArgumentTypesCounter] = 0;
                                                    functionArgumentTypesCounter++;
                                                }
                            |   FLOAT VARIABLE  {
                                                    // add it to the symbol table
                                                    VariableEntry entry = createEmptyVariableEntry();
                                                    insertVariable(0, $2, 0, 1, entry, yylineno, peek(&blockStack));
                                                    generateInitializeVariable($2);

                                                    // add it to the function
                                                    functionArgumentTypesArray[functionArgumentTypesCounter] = 1;
                                                    functionArgumentTypesCounter++;
                                                }
                            |   CHAR VARIABLE   {
                                                    // add it to the symbol table
                                                    VariableEntry entry = createEmptyVariableEntry();
                                                    insertVariable(0, $2, 0, 2, entry, yylineno, peek(&blockStack));
                                                    generateInitializeVariable($2);

                                                    // add it to the function
                                                    functionArgumentTypesArray[functionArgumentTypesCounter] = 2;
                                                    functionArgumentTypesCounter++;
                                                }
                            |   STRING VARIABLE {
                                                    // add it to the symbol table
                                                    VariableEntry entry = createEmptyVariableEntry();
                                                    insertVariable(0, $2, 0, 3, entry, yylineno, peek(&blockStack));
                                                    generateInitializeVariable($2);

                                                    // add it to the function
                                                    functionArgumentTypesArray[functionArgumentTypesCounter] = 3;
                                                    functionArgumentTypesCounter++;
                                                }
                            |   BOOL VARIABLE   {
                                                    // add it to the symbol table
                                                    VariableEntry entry = createEmptyVariableEntry();
                                                    insertVariable(0, $2, 0, 4, entry, yylineno, peek(&blockStack));
                                                    generateInitializeVariable($2);

                                                    // add it to the function
                                                    functionArgumentTypesArray[functionArgumentTypesCounter] = 4;
                                                    functionArgumentTypesCounter++;
                                                }
                            ; 


funcBody                    :   statement   
                            |   statement funcReturnStatement ';' 
                            |   funcReturnStatement ';' 
                            |
                            ;

funcReturnStatement         :   RETURN VARIABLE     {
                                                        // check if variable exists
                                                        int varTypeOperand1 = getVariableType($2, peek(&blockStack), yylineno);

                                                        // check if it has the same type
                                                        if (varTypeOperand1 != globalFunctionVariableType)
                                                        {
                                                            fprintf(stderr, "Error at %d: Return type mismatch\n", yylineno);
                                                            exit(EXIT_FAILURE);
                                                        }

                                                        // // check if it is initialized
                                                        // VariableEntry entry1 = checkVarValue($2, varTypeOperand1, peek(&blockStack), yylineno);

                                                        // check if it is not a function
                                                        if(findFunction($2) != -1) {
                                                            fprintf(stderr, "Error at %d: \"%s\" is a function\n", yylineno, $2);
                                                            exit(EXIT_FAILURE);
                                                        }
                                                        theFunctionReturnedSomething = 1;

                                                        generateReturnValue($2);
                                                    }
                            |   RETURN INT_NUM      {
                                                        // check if it has the same type
                                                        if (globalFunctionVariableType != 0)
                                                        {
                                                            fprintf(stderr, "Error at %d: Return type mismatch\n", yylineno);
                                                            exit(EXIT_FAILURE);
                                                        }
                                                        theFunctionReturnedSomething = 1;

                                                        VariableEntry entry = createEmptyVariableEntry();
                                                        entry.varOrFunctionVar = 4;
                                                        entry.variableType = 0;
                                                        entry.intValue = $2;

                                                        char* finalValue = returnValueToPrintableValue(entry);
                                                        generateReturnValue(finalValue);
                                                    }
                            |   RETURN FLOAT_NUM    {
                                                        // check if it has the same type
                                                        if (globalFunctionVariableType != 1)
                                                        {
                                                            fprintf(stderr, "Error at %d: Return type mismatch\n", yylineno);
                                                            exit(EXIT_FAILURE);
                                                        }
                                                        theFunctionReturnedSomething = 1;

                                                        VariableEntry entry = createEmptyVariableEntry();
                                                        entry.varOrFunctionVar = 4;
                                                        entry.variableType = 1;
                                                        entry.floatValue = $2;
                                                        
                                                        char* finalValue = returnValueToPrintableValue(entry);
                                                        generateReturnValue(finalValue);

                                                    }
                            |   RETURN CHAR_VALUE   {
                                                        // check if it has the same type
                                                        if (globalFunctionVariableType != 2)
                                                        {
                                                            fprintf(stderr, "Error at %d: Return type mismatch\n", yylineno);
                                                            exit(EXIT_FAILURE);
                                                        }
                                                        theFunctionReturnedSomething = 1;

                                                        // create char_value                                                            
                                                        VariableEntry entry = createEmptyVariableEntry();
                                                        entry.varOrFunctionVar = 4;
                                                        entry.variableType = 2;
                                                        entry.charValue = $2;

                                                        char* finalValue = returnValueToPrintableValue(entry);
                                                        generateReturnValue(finalValue);

                                                    } 
                            |   RETURN STRING_VALUE     {
                                                            // check if it has the same type
                                                            if (globalFunctionVariableType != 3)
                                                            {
                                                                fprintf(stderr, "Error at %d: Return type mismatch\n", yylineno);
                                                                exit(EXIT_FAILURE);
                                                            }
                                                            theFunctionReturnedSomething = 1;

                                                            VariableEntry entry = createEmptyVariableEntry();
                                                            entry.varOrFunctionVar = 4;
                                                            entry.variableType = 3;
                                                            entry.stringValue = $2;

                                                            char* finalValue = returnValueToPrintableValue(entry);
                                                            generateReturnValue(finalValue);

                                                        }
                            |   RETURN boolFactor       {
                                                            // check if it has the same type
                                                            if (globalFunctionVariableType != 4)
                                                            {
                                                                fprintf(stderr, "Error at %d: Return type mismatch\n", yylineno);
                                                                exit(EXIT_FAILURE);
                                                            }
                                                            theFunctionReturnedSomething = 1;

                                                            // create bool value
                                                            VariableEntry entry = createEmptyVariableEntry();
                                                            entry.varOrFunctionVar = 4;
                                                            entry.variableType = 4;
                                                            entry.boolValue = $2;

                                                            char* finalValue = returnValueToPrintableValue(entry);
                                                            generateReturnValue(finalValue);

                                                        }
                            |   RETURN                  {
                                                            // check if it has the same type
                                                            if (globalFunctionVariableType != 5)
                                                            {
                                                                fprintf(stderr, "Error at %d: Return type mismatch\n", yylineno);
                                                                exit(EXIT_FAILURE);
                                                            }
                                                        }
                            ;

funcCallStatement           :   funcCallStartStatement '(' callArgument ')' {   
                                                                                // get the function
                                                                                VariableEntry entry = findVariable(globalFunctionCallVariableName, peek(&blockStack));
                                                                                
                                                                                // if they don't have the same number of parameters
                                                                                if (callArgumentTypesCounter != entry.functionArgumentTypesCounter)
                                                                                {
                                                                                    fprintf(stderr, "Error at %d: function %s is called with incorrect number of parameters\n", yylineno, globalFunctionCallVariableName);
                                                                                    exit(EXIT_FAILURE);
                                                                                }
                                                                                
                                                                                // compare input arguments with the function parameters
                                                                                
                                                                                for (int i = 0; i < entry.functionArgumentTypesCounter; i++)
                                                                                {
                                                                                    if (entry.functionArgumentTypesArray[i] != callArgumentTypesArray[i])
                                                                                    {
                                                                                        fprintf(stderr, "Error at %d: function %s is called with incorrect parameters\n", yylineno, globalFunctionCallVariableName);
                                                                                        exit(EXIT_FAILURE);
                                                                                    }
                                                                                }

                                                                                $$ = entry;
                                                                            }
                            ;
funcCallStartStatement      :   VARIABLE    {
                                                // check if variable exists
                                                int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);

                                                // check if it is initialized
                                                VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);

                                                // check if it is a function
                                                if(findFunction($1) == -1) {
                                                    fprintf(stderr, "Error at %d: \"%s\" is not a function\n", yylineno, $1);
                                                    exit(EXIT_FAILURE);
                                                }

                                                globalFunctionCallVariableName = strdup($1);
                                            }
                            ;


callArgument                :   callArgumentType callArguments
                            |
                            ;

callArguments               :   ',' callArgumentType callArguments
                            |
                            ;

callArgumentType            :   VARIABLE    {
                                                // check if variable exists
                                                int varTypeOperand1 = getVariableType($1, peek(&blockStack), yylineno);

                                                // check if it is initialized
                                                VariableEntry entry1 = checkVarValue($1, varTypeOperand1, peek(&blockStack), yylineno);

                                                // check if it is not a function
                                                if(findFunction($1) != -1) {
                                                    fprintf(stderr, "Error at %d: can not pass function %s as an argument to another function\n", yylineno, $1);
                                                    exit(EXIT_FAILURE);
                                                }

                                                callArgumentTypesArray[callArgumentTypesCounter] = varTypeOperand1;
                                                callArgumentTypesCounter++;
                                            }
                            |   INT_NUM     {
                                                callArgumentTypesArray[callArgumentTypesCounter] = 0;
                                                callArgumentTypesCounter++;
                                            }
                            |   FLOAT_NUM   {
                                                callArgumentTypesArray[callArgumentTypesCounter] = 1;
                                                callArgumentTypesCounter++;
                                            }
                            |   CHAR_VALUE  {
                                                callArgumentTypesArray[callArgumentTypesCounter] = 2;
                                                callArgumentTypesCounter++;
                                            }
                            |   STRING_VALUE    {
                                                    callArgumentTypesArray[callArgumentTypesCounter] = 3;
                                                    callArgumentTypesCounter++;
                                                }
                            |   boolFactor  {
                                                callArgumentTypesArray[callArgumentTypesCounter] = 4;
                                                callArgumentTypesCounter++;
                                            }
                            ;

openBlock                   :   '{' {
                                        int newCurrentBlock = addBlock(3, peek(&blockStack));
                                        push(&blockStack, newCurrentBlock);
                                        bracketCount++;
                                    }
                            ;

closeBlock                  :   '}' {
                                        pop(&blockStack);
                                        bracketCount--;
                                    }
                            ;
%%

void checkBrackets()
{
    if  (bracketCount != 0)
    {
        fprintf(stderr, "Error at %d: Mismatched brackets\n", yylineno);
        exit(EXIT_FAILURE);
    }
}

void closeProgram() {
    if(bracketCount == 0) {
        printf("exit\n");
        exit(EXIT_SUCCESS);
    } else {
        yyerror("syntax error brackets");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_FAILURE);
}

int yyerror(const char* s)
{
  fprintf(stderr, "%s at %d\n",s, yylineno);
  return 1;
}

int main(void) {
    yyparse();
    return 0;
}