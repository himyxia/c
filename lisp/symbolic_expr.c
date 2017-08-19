#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <editline/history.h>
#include "../mpc/mpc.h"


// lisp uses the same structures to represent data and code
// we will separate out the process of reading in input, and evaluating the input we have stored.
// s_expr: an internal list structure that is built up recursively of numbers, symbols, and other lists.
// s_expr: is just a number of other Expressions between parentheses, where an Expression can be a Number, Operator, or other S-Expression
// To evaluate an S-Expression we look at the first item in the list, and take this to be the operator. 
// We then look at all the other items in the list, and take these as operands to get the result.

/*
 * Our new definition of lval needs to contain a reference to itself which means we have to slightly change how it is defined. 
 * Before we open the curly brackets we can put the name of the struct, and then refer to this inside the definition using struct lval. 
 * Even though a struct can refer to its own type, it must only contain pointers to its own type, not its own type directly. 
 * Otherwise the size of the struct would refer to itself, and grow infinite in size when you tried to calculate it!
*/
typedef struct lval {
	int type;
	long num;
	char* err;
	char* sym;
	int count;
	struct lval** cell;
} lval;

enum {LVAL_NUMBER, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};

// create constructor for lval 
lval* lval_num(long x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUMBER;
	v->num = x;
	return v;
}
lval* lval_err(char* m) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m)+1);
	strcpy(v->err, m);
	return v;
}
lval* lval_sym(char* s) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(sizeof(s) + 1);
	strcpy(v->sym, s);
	return v;
}
lval* lval_sexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}
void lval_delete(lval* v) {
	switch(v->type) {
		case LVAL_NUMBER:
			break;
		case LVAL_ERR:
			free(v->err);
			break;
		case LVAL_SYM:
			free(v->sym);
			break;
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++) {
				lval_delete(v->cell[i]);
			}
			free(v->cell);
			break;
	}
	free(v);
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*)*v->count);
	v->cell[v->count-1] = x;
	return v;
}

// first stage: convert the abstract syntax tree into an S-Expression
lval* lval_read(mpc_ast_t* t) {
	if(strstr(t->tag, "number")) {
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE?lval_num(x):lval_err("invalid number");
	}
	if(strstr(t->tag, "symbol")) {
		return lval_sym(t->contents);
	}

	lval* x = NULL;
	if (strcmp(t->tag, ">")==0) {
		// means it is root, we use sexpr to represent it 
		// in this case, eg: + 1 2 we translate it to  (+ 1 2)
		x = lval_sexpr();
	}
	if (strstr(t->tag, "sexpr")) {
		// sexpr node
		x = lval_sexpr();
	}
	for(int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) {continue;}
		if (strcmp(t->children[i]->contents, ")") == 0) {continue;}
		if (strcmp(t->children[i]->contents, "{") == 0) {continue;}
		if (strcmp(t->children[i]->contents, "}") == 0) {continue;}
		if (strcmp(t->children[i]->tag, "regex") == 0) {continue;}
		x = lval_add(x, lval_read(t->children[i]));
	}
	return x;
}

void lval_print(lval* v);

void lval_expr_print(char open, lval* v, char close) {
	putchar(open);
	for(int i = 0; i < v->count; i++) {
		 lval_print(v->cell[i]);
		if (i != (v->count-1)) {
			putchar(' ');
		}
	}
	putchar(close);
}

void lval_print(lval* v) {
 switch(v->type) {
	 case LVAL_NUMBER:
		 printf("%li", v->num);
		 break;
	 case LVAL_ERR:
		 printf("Error: %s", v->err);
		 break;
	 case LVAL_SYM:
		 printf("%s", v->sym);
		 break;
	 case LVAL_SEXPR:
		lval_expr_print('(', v, ')');
		break;
 }
}

void lval_println(lval* v) {
	lval_print(v);
	putchar('\n');
}

// second stage: evaluate this S-Expression using our normal Lisp rules.
// We need to adapt it to deal with lval* and our more relaxed definition of what constitutes an expression. We can think of our evaluation function as a kind of transformer. It takes in some lval* and transforms it in some way to some new lval*. 
	// In some cases it can just return exactly the same thing. 
	// In other cases it may modify the input lval* and return it. 
	// In many cases it will delete the input, and return something completely different. If we are going to return something new we must always remember to delete the lval* we get as input.
	
// for S-Expressions we first evaluate all the children of the S-Expression. 
	// If any of these children are errors we return the first error we encounter using a function we'll define later called lval_take.
	// If the S-Expression has no children we just return it directly. This corresponds to the empty expression, denoted by (). We also check for single expressions. These are expressions with only one child such as (5). In this case we return the single expression contained within the parenthesis.
// If neither of these are the case we know we have a valid expression with more than one child. 
	// In this case we separate the first element of the expression using a function we'll define later called lval_pop. We then check this is a symbol and not anything else. 
	// If it is a symbol we check what symbol it is, and pass it, and the arguments, to a function builtin_op which does our calculations. 
	// If the first element is not a symbol we delete it, and the values passed into the evaluation function, returning an error.
lval* lval_pop(lval* v, int i) {
	lval* x = v->cell[i];
	// & get the address of some data 
	// don't understand this ??
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count-i-1));
	v->count--;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	return x;
}

lval* lval_take(lval* v, int i) {
	lval* x = lval_pop(v, i);
	lval_delete(v);
	return x;
}

lval* lval_eval_sexpr(lval* v);

lval* lval_eval(lval* v) {
	if (v->type == LVAL_SEXPR) {
		return lval_eval_sexpr(v);
	}
	return v;
}

lval* builtin_op(lval* a, char* op) {
	for(int i = 0; i < a->count; i++) {
		if(a->cell[i]->type != LVAL_NUMBER) {
			lval_delete(a);
			return lval_err("Cannot operate on non-number!");
		}
	}
	lval* x = lval_pop(a, 0);

	if(strcmp(op, "-") == 0 && a->count == 0) {
		x->num = -x->num;
	}
	while(a->count>0) {
		lval* y = lval_pop(a, 0) ;
		if(strcmp(op,"+") == 0) {x->num += y->num;}
		if(strcmp(op,"-") == 0) {x->num -= y->num;}
		if(strcmp(op,"*") == 0) {x->num *= y->num;}
		if(strcmp(op,"/") == 0) {
			if (y->num == 0) {
				lval_delete(x);
				lval_delete(y);
				x = lval_err("Division by zero!");
				break;
			}
			x->num /= y->num;
		}
		lval_delete(y);
	}
	lval_delete(a);
	return x;
}

lval* lval_eval_sexpr(lval* v) {
	for (int i = 0; i < v->count; i++) {
		v->cell[i] = lval_eval(v->cell[i]);
	}
	for (int i = 0; i < v->count; i++) { 
		if(v->cell[i]->type == LVAL_ERR) {
			return lval_take(v, i);
		}
	}

	if(v->count == 0) {
		return v;
	}

	if (v->count == 1) {
		return lval_take(v, 0);
	}

	lval* f = lval_pop(v, 0);

	if (f->type != LVAL_SYM) {
		lval_delete(f);
		lval_delete(v);
		return lval_err("S-expression does not start with symbol!");
	}

	lval* result = builtin_op(v, f->sym);
	lval_delete(f);
	return result;
}


int main(int argc, char** argv) {
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	// use / / to encolse the regular expr
	mpca_lang(MPCA_LANG_DEFAULT,
			"   \
			number : /-?[0-9]+/; \
			symbol : '+' | '-' | '*' | '/'; \
			sexpr : '(' <expr>* ')'; \
			expr : <number> | <symbol> | <sexpr> ; \
			lispy : /^/ <expr>* /$/; \
			",
			Number, Symbol, Sexpr,Expr, Lispy);
	while(1) {
		char* input = readline("lispy> ");
		add_history(input);

		mpc_result_t r;

		if(mpc_parse("<stdin>", input, Lispy, &r)) {
			mpc_ast_print(r.output);
			lval* result = lval_eval(lval_read(r.output));
			lval_println(result);
			lval_delete(result);
			mpc_ast_delete(r.output);
		}else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}
	mpc_cleanup(5,Number, Symbol, Sexpr,Expr, Lispy);
	return 0;
} 
