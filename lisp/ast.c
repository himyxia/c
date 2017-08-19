#include "../mpc/mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <editline/history.h>

#define LASSERT(args, cond, err) \
   if (!cond) { lval_del(args); return lval_err(err); }

typedef struct lval{
	long num;
	char *opr;
	char *err;
	int typ;
	int count;
	struct lval **cell;
} lval;

//enum {LERR_DIV_ZERO, LERR_BAD_OPR, LERR_BAD_NUM, LERR_UNKNOWN};

enum {LVAL_NUM, LVAL_ERR, LVAL_OPR, LVAL_SEXPR, LVAL_QEXPR};

lval *lval_num (long num) {
	lval *ret = malloc(sizeof(lval));
	ret->num = num;
	ret->typ = LVAL_NUM;

	ret->cell = NULL;
	ret->opr = NULL;
	ret->err = NULL;

	return ret;
}

lval *lval_opr (char *opr) {
	lval *ret = malloc(sizeof(lval));
	ret->typ = LVAL_OPR;
	ret->opr = malloc(sizeof(opr)+1);
	strcpy(ret->opr, opr);

	ret->cell = NULL;
	ret->err = NULL;
	return ret;
}

lval *lval_err (char *msg) {
	lval *ret = malloc(sizeof(lval));
	ret->typ = LVAL_ERR;
	ret->err = malloc(strlen(msg) + 1);
	strcpy(ret->err, msg);

	ret->cell = NULL;
	ret->opr = NULL;
	ret->err = NULL;

	return ret;
}

lval *lval_qexpr (void) {
	lval *ret = malloc(sizeof(lval));
	ret->typ = LVAL_QEXPR;
	ret->count = 0;

	ret->cell = NULL;
	ret->opr = NULL;
	ret->err = NULL;

	return ret;
}

lval *lval_sexpr (void) {
	lval *ret = malloc(sizeof(lval));
	ret->typ = LVAL_SEXPR;
	ret->count = 0;

	ret->cell = NULL;
	ret->opr = NULL;
	ret->err = NULL;

	return ret;
}

lval *lval_add(lval *l, lval *v) {
	l->count++;
	l->cell = realloc(l->cell, sizeof(lval*) * l->count);
	l->cell[l->count-1] = v;
	return l;
}

void lval_print(lval *l);

void lval_expr_print(char open, lval *l, char end) {
	putchar(open);
	for(int i = 0; i < l->count; i++) {
		lval_print(l->cell[i]);
		if (i != l->count-1) {
			putchar(' ');
		}
	}
	putchar(end);
}

void lval_print(lval *l) {
	if  (l->typ == LVAL_NUM) {
		printf("%li", l->num);
	}
	if  (l->typ == LVAL_OPR) {
		printf("%s", l->opr);
	}
	if  (l->typ == LVAL_ERR) {
		printf("Error: %s", l->err);
	}
	if  (l->typ == LVAL_SEXPR ) {
		lval_expr_print('(', l, ')');
	}
	if  (l->typ == LVAL_QEXPR ) {
		lval_expr_print('{', l, '}');
	}
}

lval *lval_read(mpc_ast_t *t) {
	if (strstr(t->tag, "exit")) {
		exit(0);
	}
	if (strstr(t->tag, "number")) {
		errno = 0;
		long x = strtol(t->contents,NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err("bad number");
	}
	if (strstr(t->tag, "operator")) {
		return lval_opr(t->contents);
	}
	lval *l = NULL;
	if (strcmp(t->tag, ">") == 0 ) {
		l = lval_sexpr();
	}
	if(strstr(t->tag, "sexpr")) {
		l = lval_sexpr();
	}
	if(strstr(t->tag, "qexpr")) {
		l = lval_qexpr();
	}
	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
		l = lval_add(l,  lval_read(t->children[i]));
	}
	return l;
}
lval *lval_pop(lval *v, int i);


void lval_del(lval *l) {
	switch (l->typ) {
		case LVAL_NUM:
			break;
		case LVAL_OPR:
			free(l->opr);
			break;
		case LVAL_ERR:
			free(l->err);
			break;
		case LVAL_QEXPR:
		case LVAL_SEXPR:
			for(int i = 0; i < l->count; i++) {
				lval_del(l->cell[i]);
			}
			free(l->cell);
			break;
	}
	free(l);
}

lval *builtin_op(lval *l, char *opr) {
	for(int i = 0; i < l->count; i++) {
		if (l->cell[i]->typ != LVAL_NUM) {
			lval_del(l);
			free(opr);
			return lval_err("bad number");
		}
	}
	lval *x = lval_pop(l, 0);
	if (strcmp(opr, "-") == 0 && l->count == 1) {
		x -> num = -x->num;
	}
	while(l->count > 0) {
		lval *y = lval_pop(l, 0);
		if(strcmp(opr, "+")==0) { x->num += y->num; }
		if(strcmp(opr, "-")==0) { x->num -= y->num; }
		if(strcmp(opr, "*")==0) { x->num *= y->num; }
		if(strcmp(opr, "mod")==0) { x->num %= y->num; }
		if(strcmp(opr, "/")==0) { 
			if (y->num == 0) {
				lval_del(x);
				lval_del(y);
				x = lval_err("bad operator");
			}
			x->num /= y->num; 
		}
		lval_del(y);
	}
	lval_del(l);
	return x;
}
lval *lval_join(lval *x, lval *y) {
	while(y->count) {
		x = lval_add(x, lval_pop(y, 0));
	}
	lval_del(y);
	return x;
}

lval *lval_take(lval *v, int i) {
	lval *ret = lval_pop(v, i);
	lval_del(v);
	return ret;
}

lval *builtin_list(lval *l) {
	l->typ = LVAL_QEXPR;
	return l;
}

lval *builtin_join(lval *l) {
	for (int i = 0; i < l->count;  i++) {
		if (l->cell[0] -> typ != LVAL_QEXPR) {
			return lval_err("function 'join' passed wrong type");
		}
	}
	lval *x = lval_pop(l, 0);
	while(l->count) {
		x = lval_join(x, lval_pop(l, 0));
	}
	lval_del(l);
	return x;
}

lval *builtin_tail(lval *l) {
	LASSERT(l, l->count == 1, "function 'tail' passed too many arguments");
	LASSERT(l, l->cell[0]->typ == LVAL_QEXPR, "function 'tail' passed too many arguments");
	LASSERT(l, l->cell[0]->count != 0, "function 'tail' passed {}");
	lval* v = lval_take(l, 0);
	lval_del(lval_pop(v, 0));
	return v;
}


lval *builtin_head(lval *l) {
	LASSERT(l, l->count == 1, "function 'head' passed too many arguments");
	LASSERT(l, l->cell[0]->typ == LVAL_QEXPR, "function 'head' passed too many arguments");
	LASSERT(l, l->cell[0]->count != 0, "function 'head' passed {}");
	lval *v = lval_take(l, 0);
	while(v->count > 1) {
		lval_del(lval_pop(v, 1));
	}
	return v;
}

lval *builtin(lval *l, char *opr) {
	if (strstr("+-*/mod",opr)) {
		return builtin_op(l, opr);
	}
	if (strcmp("head", opr) == 0) {
		return builtin_head(l);
	}
	if (strcmp("tail", opr) == 0) {
		return builtin_tail(l);
	}
	if (strcmp("list", opr) == 0) {
		return builtin_list(l);
	}
	if (strcmp("join", opr) == 0) {
		return builtin_join(l);
	}
	return NULL;
}

lval *lval_pop(lval *v, int i) {
	lval *x = v->cell[i];
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));
	v->count--;
	v->cell = realloc(v->cell, sizeof(lval*)*v->count);
	return x;
}


lval *lval_eval(lval *l);

lval *lval_sexpr_eval(lval *l) {
	for(int i = 0; i < l->count; i++) {
		l->cell[i] = lval_eval(l->cell[i]);
	}
	for(int i = 0; i < l->count; i++) {
		if (l->cell[i]->typ == LVAL_ERR) {
			return lval_take(l, i);
		}
	}
	if (l->count == 0) {
		return l;
	}
	if (l -> count == 1) {
		return lval_take(l, 0);
	}
	lval *opr = lval_pop(l, 0);
	if (opr->typ != LVAL_OPR) {
		lval_del(l);
		lval_del(opr);
		return lval_err("no operation");
	}
	lval *ret = builtin(l, opr->opr);
	lval_del(opr);
	return ret;
}

lval *lval_eval(lval *l) {
	switch (l->typ) {
		case LVAL_SEXPR:
			return lval_sexpr_eval(l);
		default:
			return l;
	}
}

int main(int argc, char** argv) {
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Lispy = mpc_new("lispy");
	mpc_parser_t* Exit = mpc_new("exit");

	mpca_lang(MPCA_LANG_DEFAULT,
			"  							                      \
				exit    : \"\\q\" ;                          \
				number   : /-?[0-9]+/ ;                         \
				operator : '+' | '-' | '*' | '/' | \"mod\" | \"head\" | \"tail\" | \"list\" | \"join\";            \
				sexpr     :  '(' <expr>* ')' ;   \
				qexpr     :  '{' <expr>* '}' ;   \
				expr     :  <number>  | <operator> | <sexpr> | <qexpr> ; \
				lispy   : /^/ <exit> /$/ | /^/ <expr>* /$/ ;         \
			",
			Exit, Number, Operator, Sexpr, Qexpr, Expr, Lispy);

	puts("Lispy Version 0.0.0.0.1");
	puts("Press \\q to Exit\n");

	while(1) {
		char* input = readline("lispy> ");
		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			//mpc_ast_print(r.output);
			lval_print(lval_eval((lval_read(r.output))));
			putchar('\n');
			mpc_ast_delete(r.output);
		}else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}
	mpc_cleanup(6, Exit, Number, Operator, Sexpr, Qexpr, Expr, Lispy);
	return 0;
}
