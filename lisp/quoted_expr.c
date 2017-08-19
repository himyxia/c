#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <editline/history.h>
#include "../mpc/mpc.h"

//quoted_expression: surrounded by {}
typedef struct lval {
	int type;
	long num;
	char* err;
	char* sym;
	int count;
	struct lval** cell;
} lval;

enum {LVAL_NUMBER, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR};

// create constructor for lval 
lval* lval_num(long x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUMBER;
	v->num = x;
	return v;
}

lval* lval_qexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
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
		case LVAL_QEXPR:
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

// first: convert the abstract syntax tree into lval
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
		// eg: + 1 2 -> (+ 1 2)
		x = lval_sexpr();
	}
	if (strstr(t->tag, "sexpr")) {
		// encouter sexpr node
		x = lval_sexpr();
	}

	if (strstr(t->tag, "qexpr")) {
		// encouter sexpr node
		x = lval_qexpr();
	}
	for(int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(")==0) {continue;}
		if (strcmp(t->children[i]->contents, ")")==0) {continue;}
		if (strcmp(t->children[i]->contents, "{")==0) {continue;}
		if (strcmp(t->children[i]->contents, "}")==0) {continue;}
		if (strcmp(t->children[i]->tag, "regex")==0) {continue;}
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
	 case LVAL_QEXPR:
		lval_expr_print('{', v, '}');
		break;
 }
}

void lval_println(lval* v) {
	lval_print(v);
	putchar('\n');
}

lval* lval_pop(lval* v, int i) {
	lval* x = v->cell[i];
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

//converts the input S-Expression to a Q-Expression and returns it.
lval* builtin_list(lval* a) {
	a->type = LVAL_QEXPR;
	return a;
}

#define LASSERT(args, cond, err)  \
	if(!(cond)) {lval_delete(args); return lval_err(err);}

//takes as input some single Q-Expression, which it converts to an S-Expression, and evaluates using lval_eval
lval* builtin_eval(lval* a) {
	LASSERT(a, a->count == 1,
		"Function 'eval' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
		"Function 'eval' passed incorrect type!");
	lval* x = lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return lval_eval(x);
}

lval* lval_join(lval* x, lval* y) {
	while(y->count) {
		x = lval_add(x, lval_pop(y, 0));
	}
	lval_delete(y);
	return x;
}

lval* builtin_join(lval* a) {
	for(int i = 0; i < a->count; i++) {
		LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
			"Function 'join' passed incorrect type!");
	}
	lval* x = lval_pop(a, 0);
	while(a->count) {
		x = lval_join(x, lval_pop(a, 0));
	}
	lval_delete(a);
	return x;
}
lval* builtin_head(lval* a) {
	LASSERT(a, a->count == 1,
		"Function 'head' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
		"Function 'head' passed incorrect type!");
	LASSERT(a, a->cell[0]->count != 0,
		"Function 'head' passed {}!");
	lval* v = lval_take(a, 0);
	while(v->count > 1) {
		lval_delete(lval_pop(v, 1));
	}
	return v;
}

lval* builtin_tail(lval* a) {
	LASSERT(a, a->count == 1,
		"Function 'tail' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
		"Function 'tail' passed incorrect type!");
	LASSERT(a, a->cell[0]->count != 0,
		"Function 'tail' passed {}!");
	lval* v = lval_take(a, 0);
	while(v->count > 1) {
		lval_delete(lval_pop(v, 1));
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

lval* builtin(lval* a, char* func) {
	if(strcmp("list", func) == 0) {return builtin_list(a);}
	if(strcmp("head", func) == 0) {return builtin_head(a);}
	if(strcmp("tail", func) == 0) {return builtin_tail(a);}
	if(strcmp("join", func) == 0) {return builtin_join(a);}
	if(strcmp("eval", func) == 0) {return builtin_eval(a);}
	if(strstr("+-*/", func)) {return builtin_op(a, func);}
	lval_delete(a);
	return lval_err("Unknown Function!");
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

	lval* result = builtin(v, f->sym);
	lval_delete(f);
	return result;
}

int main(int argc, char** argv) {
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	// use / / to encolse the regular expr
	mpca_lang(MPCA_LANG_DEFAULT,
			"   \
			number : /-?[0-9]+/; \
			symbol : \"list\" | \"head\" | \"tail\" \
				| \"join\" | \"eval\" |'+' | '-' | '*' | '/'; \
			sexpr : '(' <expr>* ')'; \
			qexpr : '{' <expr>* '}'; \
			expr : <number> | <symbol> | <sexpr> | <qexpr> ; \
			lispy : /^/ <expr>* /$/; \
			",
			Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
	while(1) {
		char* input = readline("lispy> ");
		add_history(input);

		mpc_result_t r;
		// r.output -> mpc_ast_t*
		// ast -> abstract syntax tree

		if(mpc_parse("<stdin>", input, Lispy, &r)) {
			mpc_ast_print(r.output);
			//lval_println(lval_read(r.output));
			//lval_println(lval_eval(lval_read(r.output)));
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
	mpc_cleanup(5,Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
	return 0;
} 
