#include "../mpc/mpc.h"
#include <stdio.h>
#include <editline/readline.h>
#include <editline/history.h>

typedef struct lval{
	int type;
	long num;
	char* err;
	char* op;
	int count;
	struct lval** cell;
} lval;

enum {LVAL_NUM, LVAL_OP, LVAL_ERR, LVAL_SEXPR};

lval* lval_sexpr() {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}
lval* lval_num(long num) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = num;
	return v;
}

lval* lval_err(char* m) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(sizeof(strlen(m) + 1));
	strcpy(v->err, m);
	return v;
}

lval* lval_op(char* op) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_OP;
	v->op = malloc(sizeof(strlen(op) + 1));
	strcpy(v->op, op);
	return v;
}

lval* lval_add(lval* x, lval* y) {
	x->count++;
	x->cell = realloc(x->cell, sizeof(lval*)*x->count);
	x->cell[x->count - 1] = y;
	return x;
}

void lval_del(lval* v) {
	switch(v->type) {
		case LVAL_OP:
			free(v->op);
			free(v);
			break;
		case LVAL_NUM:
			free(v);
			break;
		case LVAL_ERR:
			free(v->err);
			free(v);
			break;
		case LVAL_SEXPR:
			for(int i = 0; i < v->count; i++) { lval_del(v->cell[i]); }
			free(v->cell);
			free(v);
			break;
	}
}


lval* lval_read(mpc_ast_t* t) {
	if (strstr(t->tag, "number")) {
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err("Wrong number");
	}
	if (strstr(t->tag, "operator")) {
		return lval_op(t->contents);
	}

	lval* x = lval_sexpr();

	for(int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->tag, "regex")==0) {
			continue;
		}
		if (strcmp(t->children[i]->contents, "(")==0) {
			continue;
		}
		if (strcmp(t->children[i]->contents, ")")==0) {
			continue;
		}
		/*
		if (strstr(t->children[i]->tag, ">")) {
		}*/
		lval_add(x, lval_read(t->children[i]));
	}
	return x;
}

lval* lval_pop(lval* v, int i);

lval* lval_take(lval* v, int index) {
	lval* result = lval_pop(v, index);
	lval_del(v);
	return result;
}

lval* lval_pop(lval* v, int i) {
	lval* result = v->cell[i];
	memmove(&v->cell[i],&v->cell[i+1], sizeof(lval*)*v->count-i-1);
	v->count--;
	v->cell = realloc(v->cell, sizeof(lval*)*v->count);
	return result;
}

lval* built_op(lval* x, lval* op, lval* y) {
	if(y->type != LVAL_NUM) {
		lval_del(x);
		lval_del(y);
		return lval_err("wrong num");
	}

	if(strcmp(op->op , "+")==0) {
		x->num += y->num;
		lval_del(y);
		return x;
	}
	if(strcmp(op->op , "-")==0) {
		x->num -= y->num;
		lval_del(y);
		return x;
	}
	if(strcmp(op->op , "*")==0) {
		x->num *= y->num;
		lval_del(y);
		return x;
	}
	if(strcmp(op->op ,"/")==0) {
		if (y->num == 0) {
			lval_del(x);
			lval_del(y);
			return lval_err("divide by zero");
		}
		x->num /= y->num;
		lval_del(y);
		return x;
	}
	return lval_err("wrong op");
}

lval* lval_eval(lval* v);

lval* lval_sexpr_eval(lval* v) {
	for(int i = 0; i < v->count; i++) {
		v->cell[i] = lval_eval(v->cell[i]);
	}

	for(int i = 0; i < v->count; i++) {
		if(v->cell[i]->type == LVAL_ERR) {
			return lval_take(v, i);
		}
	}

	lval* op = lval_pop(v, 0);

	if(op->type != LVAL_OP) {
		lval_del(op);
		lval_del(v);
		return lval_err("wrong operator");
	}

	lval* x = lval_pop(v, 0);
	if(x->type != LVAL_NUM) {
		lval_del(v);
		lval_del(x);
		return lval_err("wrong num");
	}

	if(strcmp(op->op, "-")==0 && v->count == 0) {
		lval_del(v);
		x->num = - x->num;
		lval_del(op);
		return x;
	}

	while(v->count > 0) {
		x = built_op(x, op, lval_pop(v, 0));
	}
	lval_del(v);
	return x;
}

lval* lval_eval(lval* v) {
	if(v->type == LVAL_SEXPR) {
		return lval_sexpr_eval(v);
	}
	return v;
}

void lval_print(lval* v) {
	switch (v->type) {
		case LVAL_ERR:
			printf("%s", v->err);
			lval_del(v);
			break;
		case LVAL_OP:
			printf("%s", v->op);
			lval_del(v);
			break;
		case LVAL_NUM:
			printf("%li", v->num);
			lval_del(v);
			break;
		case LVAL_SEXPR:
			putchar('(');
			for(int i = 0; i < v->count; i++) {
				if(i < v->count) {
					putchar(' ');
				}
				lval_print(v->cell[i]);
			}
			puts(")");
			lval_del(v);
			break;
	}
}

void lval_println(lval* v) {
	lval_print(v);
	putchar('\n');
}

int main(int argc, char** argv) {
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
			"  							                      \
				number   : /-?[0-9]+/ ;                         \
				operator : '+' | '-' | '*' | '/' ;            \
				expr     :  <number>  | <sexpr> ;\
				sexpr     :  '(' <operator> <expr>+ ')' ;\
				lispy   : /^/ <operator> <expr>* /$/ ;         \
			",
			Number, Operator, Expr, Sexpr, Lispy);

	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	while(1) {
		char* input = readline("lispy> ");
		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			mpc_ast_print(r.output);
			lval_println(lval_eval(lval_read(r.output)));
			mpc_ast_delete(r.output);
		}else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}
	//mpc_cleanup(5, Number, Operator, Expr, Exit,Lispy);
	mpc_cleanup(5, Number, Operator, Expr, Sexpr, Lispy);
	return 0;
}
