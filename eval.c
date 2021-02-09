/**************************************************************************
 * C S 429 EEL interpreter
 * 
 * eval.c - This file contains the skeleton of functions to be implemented by
 * you. When completed, it will contain the code used to evaluate an expression
 * based on its AST.
 * 
 * Copyright (c) 2021. S. Chatterjee, X. Shen, T. Byrd. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/ 

#include "ci.h"

extern bool is_binop(token_t);
extern bool is_unop(token_t);
char *strrev(char *str);

/* infer_type() - set the type of a non-root node based on the types of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated.
 * (STUDENT TODO)
 */

static void infer_type(node_t *nptr) {
    if(nptr == NULL) return;
    if(terminate || ignore_input) return;
    if(nptr->node_type != NT_LEAF){
        infer_type(nptr->children[0]);
        infer_type(nptr->children[1]); 
        if(nptr->children[1] != NULL && nptr->tok != TOK_QUESTION){
            if(nptr->children[0]->type != nptr->children[1]->type && nptr->tok != TOK_TIMES){
                handle_error(ERR_TYPE);
                return;
            }
        } 
        if(nptr->tok == TOK_EQ || nptr->tok == TOK_LT || nptr->tok == TOK_GT || nptr->tok == TOK_AND ||
        nptr->tok == TOK_OR){
            nptr->type = BOOL_TYPE;
        } else{
            nptr->type = nptr->children[0]->type;
        }
    } else{
        nptr->type = nptr->type;
    }
}

/* infer_root() - set the type of the root node based on the types of children
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated. 
 */

static void infer_root(node_t *nptr) {
    //print_tree(nptr);
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;

    // check for assignment
    if (nptr->type == ID_TYPE) {
        infer_type(nptr->children[1]);
    } else {
        for (int i = 0; i < 3; ++i) {
            infer_type(nptr->children[i]);
        }
        if (nptr->children[0] == NULL) {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        nptr->type = nptr->children[0]->type;
    }
    return;
}

/* eval_node() - set the value of a non-root node based on the values of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The val field of the node is updated.
 * (STUDENT TODO) 
 */

static void eval_node(node_t *nptr) {
    if(nptr == NULL) return;
    if(terminate || ignore_input) return;
    if(nptr->node_type != NT_LEAF){
        eval_node(nptr->children[0]);
        eval_node(nptr->children[1]);
        if(nptr->type == INT_TYPE){
            if(nptr->tok == TOK_PLUS){
                nptr->val.ival = nptr->children[0]->val.ival + nptr->children[1]->val.ival;
            } else if(nptr->tok == TOK_DIV){
                if(nptr->children[1]->val.ival == 0){
                    handle_error(ERR_EVAL);
                    return;
                }
                nptr->val.ival = nptr->children[0]->val.ival / nptr->children[1]->val.ival;
            } else if(nptr->tok == TOK_MOD){
                if(nptr->children[1]->val.ival == 0){
                    handle_error(ERR_EVAL);
                    return;
                }
                nptr->val.ival = nptr->children[0]->val.ival % nptr->children[1]->val.ival;
            } else if(nptr->tok == TOK_TIMES){
                if(nptr->children[1]->type == STRING_TYPE){
                    handle_error(ERR_TYPE);
                    return;
                }
                nptr->val.ival = nptr->children[0]->val.ival * nptr->children[1]->val.ival;
            } else if(nptr->tok == TOK_BMINUS){
                nptr->val.ival = nptr->children[0]->val.ival - nptr->children[1]->val.ival;
            } else if(nptr->tok == TOK_UMINUS){
                nptr->val.ival = (-1) * (nptr->children[0]->val.ival);
            } else if(nptr->tok == TOK_NOT){
                handle_error(ERR_TYPE);
                return;
            }
        } else if(nptr->type == STRING_TYPE){
            if(nptr->tok == TOK_UMINUS){
                nptr->val.sval = strrev(nptr->children[0]->val.sval);
            } else if(nptr->tok == TOK_PLUS){ 
                char *str = malloc(strlen(nptr->children[0]->val.sval) + strlen(nptr->children[1]->val.sval) + 1);
                strcpy(stpcpy(str, nptr->children[0]->val.sval), nptr->children[1]->val.sval);
                nptr->val.sval = str;
            } else if(nptr->tok == TOK_TIMES){
                int count = nptr->children[1]->val.ival;
                char *str = (char*)malloc(strlen(nptr->children[0]->val.sval) * count + 1);
                strcpy(str, "");
                for(int index = 0; index < count; index++){
                    strcat(str, nptr->children[0]->val.sval);
                }
                nptr->val.sval = str;
            } 
        } else if(nptr->type == BOOL_TYPE){
            if(nptr->tok == TOK_EQ){
                if(nptr->children[0]->type == STRING_TYPE){
                    int boo = strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval);
                    if(boo == 0){
                        nptr->val.bval = 1;
                    } else{
                        nptr->val.bval = 0;
                    }
                } else if(nptr->children[0]->type == INT_TYPE){
                    if(nptr->children[0]->val.ival == nptr->children[1]->val.ival){
                    nptr->val.bval = 1;
                    } else {
                    nptr->val.bval = 0;
                    }
                }
            } else if(nptr->tok == TOK_LT){
                if(nptr->children[0]->type == INT_TYPE){
                    int one = nptr->children[0]->val.ival;
                    int two = nptr->children[1]->val.ival;
                    if(one < two){
                        nptr->val.bval = 1;
                    } else{
                        nptr->val.bval = 0;
                    }
                } else if(nptr->children[0]->type == STRING_TYPE){
                    int one = strlen(nptr->children[0]->val.sval);
                    int two = strlen(nptr->children[1]->val.sval);
                    if(one < two){
                        nptr->val.bval = 1;
                    } else{
                        nptr->val.bval = 0;
                    }
                }
            } else if(nptr->tok == TOK_GT){
                if(nptr->children[0]->type == INT_TYPE){
                    int one = nptr->children[0]->val.ival;
                    int two = nptr->children[1]->val.ival;
                    if(one > two){
                        nptr->val.bval = 1;
                    } else{
                        nptr->val.bval = 0;
                    }
                } else if(nptr->children[0]->type == STRING_TYPE){
                    int one = strlen(nptr->children[0]->val.sval);
                    int two = strlen(nptr->children[1]->val.sval);
                    if(one > two){
                        nptr->val.bval = 1;
                    } else{
                        nptr->val.bval = 0;
                    }
                } else{
                    handle_error(ERR_TYPE);
                    return;
                }
            } else if(nptr->tok == TOK_AND){
                if(nptr->children[0]->val.bval == 1 && nptr->children[1]->val.bval == 1){
                    nptr->val.bval = 1;
                } else{
                    nptr->val.bval = 0;
                }
            } else if(nptr->tok == TOK_OR){
                if(nptr->children[0]->val.bval == 1 || nptr->children[1]->val.bval == true){
                    nptr->val.bval = 1;
                } else{
                    nptr->val.bval = 0;
                }
            } else if(nptr->tok == TOK_NOT){
                nptr->val.bval = !(nptr->children[0]->val.bval);
            } else if(nptr->tok == TOK_QUESTION){
                if(nptr->children[0]->val.bval == 1){
                    if(nptr->children[1]->type == INT_TYPE){
                        if(nptr->children[2]->type == BOOL_TYPE){
                            handle_error(ERR_TYPE);
                            return;
                        }
                        nptr->val.ival = nptr->children[1]->val.ival;
                    } else{
                        nptr->val.bval = nptr->children[1]->val.bval;
                    }
                } else {
                    if(nptr->children[1]->type == INT_TYPE){
                        nptr->val.ival = nptr->children[2]->val.ival;
                    } else{
                        //nptr->val.bval = nptr->children[2]->val.bval;
                        nptr->val.bval = 0;
                    }
                }
            }
        }
    } else{
        if(nptr->type == BOOL_TYPE){
            if(nptr->tok == TOK_TRUE){
                nptr->val.bval = 1;
            } else if(nptr->tok == TOK_FALSE){
                nptr->val.bval = 0;
            }
        }
        nptr->val = nptr->val;
    }
}

/* eval_root() - set the value of the root node based on the values of children 
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The val dield of the node is updated. 
 */

void eval_root(node_t *nptr) {
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;

    // check for assignment
    if (nptr->type == ID_TYPE) {
        eval_node(nptr->children[1]);
        if (terminate || ignore_input) return;
        
        if (nptr->children[0] == NULL) {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        put(nptr->children[0]->val.sval, nptr->children[1]);
        return;
    }

    for (int i = 0; i < 2; ++i) {
        eval_node(nptr->children[i]);
    }
    if (terminate || ignore_input) return;
    
    if (nptr->type == STRING_TYPE) {
        (nptr->val).sval = (char *) malloc(strlen(nptr->children[0]->val.sval) + 1);
        if (! nptr->val.sval) {
            logging(LOG_FATAL, "failed to allocate string");
            return;
        }
        strcpy(nptr->val.sval, nptr->children[0]->val.sval);
    } else {
        nptr->val.ival = nptr->children[0]->val.ival;
    }
    return;
}

/* infer_and_eval() - wrapper for calling infer() and eval() 
 * Parameter: A pointer to a root node.
 * Return value: none.
 * Side effect: The type and val fields of the node are updated. 
 */

void infer_and_eval(node_t *nptr) {
    infer_root(nptr);
    eval_root(nptr);
    return;
}

/* strrev() - helper function to reverse a given string 
 * Parameter: The string to reverse.
 * Return value: The reversed string. The input string is not modified.
 * (STUDENT TODO)
 */

char *strrev(char *str) {
    char *start = str;
    char *end = start + strlen(str) - 1;
    char temp;

    while(end > start){
        temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    return str;
}