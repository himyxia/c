#include "../mpc/mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <editline/history.h>
#define LASSERT(args, cond, err) \
   if (!cond) { lval_del(args); return lval_err(err); }

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

typedef lval* (*lbuiltin) (lenv*, lval*);

struct lval{
	int type;
	long num;
	char *err;
	char *sym;

	lbuiltin fun;

	int count;
	lval **cell;
};

struct lenv {
	int count;
	char** syms;
	lval** vals;
};

//enum {LERR_DIV_ZERO, LERR_BAD_OPR, LERR_BAD_NUM, LERR_UNKNOWN};

enum {LVAL_NUM, LVAL_FUN, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR};

lenv *lenv_new (void) {
	lenv *e = malloc(sizeof(lenv));
	e->count = 0;
	e->syms = NULL;
	e->vals = NULL;
	return e;
}

void lval_del(lval *l);

lval *lval_new (void) {
	lval *l = malloc(sizeof(lval));
	l->count = 0;
	l->cell = NULL;
	return l;
}



lval *lval_err (char *msg);

lval *lval_copy(lval *l);

lval *lenv_get(lenv *e, lval *sym) {
	for(int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], sym->sym) == 0) {
			return lval_copy(e->vals[i]);
		}
	}
	return lval_err("undefine symbolic");
}

void lenv_del_all(lenv *e) {
	for (int i = 0; i < e->count; i++) {
		free(e->syms[i]);
		lval_del(e->vals[i]);
	}
	free(e->syms);
	free(e->vals);
	free(e);
}

lval *lval_fun (lbuiltin func) {
	lval *v = malloc(sizeof(lval));
	v->type= LVAL_FUN;
	v->fun = func;
	return v;
}

lval *lval_sym (char *sym);

void lenv_put(lenv *e, lval *k,  lval *v) {
	for(int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			// override
			lval_del(e->vals[i]);
			e->vals[i] = lval_copy(v);
			return;
		}
	}
	// insert new one
	e->count++;
	e->syms = realloc(e->syms, sizeof(char*) * e->count);
	e->vals = realloc(e->vals, sizeof(lval*) * e->count);

	e->vals[e->count - 1] = lval_copy(v);

	e->syms[e->count-1] = malloc(strlen(k->sym) + 1);
	strcpy(e->syms[e->count-1], k->sym);
}


lval *lval_copy(lval *l) {
	lval *x = malloc(sizeof(lval));
	x->type = l->type;

	switch (l->type) {
		case LVAL_NUM:
			x ->num = l->num;
			break;
		case LVAL_FUN:
			x ->fun = l->fun;
			break;
		case LVAL_SYM:
			x->sym = malloc(strlen(l->sym)+1);
			strcpy(x->sym, l->sym);
			break;
		case LVAL_ERR:
			x->err = l->err;
			break;
		case LVAL_QEXPR:
		case LVAL_SEXPR:
			x->count = l->count;
			x->cell = malloc(sizeof(lval*) * x->count);
			for (int i = 0; i < x->count; i++) {
				x->cell[i] = lval_copy(l->cell[i]);
			}
			break;
	}
	return x;
}

lval *lval_num (long num) {
	lval *ret = malloc(sizeof(lval));
	ret->num = num;
	ret->type= LVAL_NUM;

	ret->cell = NULL;
	ret->sym = NULL;
	ret->err = NULL;

	return ret;
}

lval *lval_sym (char *sym) {
	lval *ret = malloc(sizeof(lval));
	ret->type= LVAL_SYM;
	ret->sym = malloc(sizeof(sym)+1);
	strcpy(ret->sym, sym);

	ret->cell = NULL;
	ret->err = NULL;
	return ret;
}

lval *lval_err (char *msg) {
	lval *ret = malloc(sizeof(lval));
	ret->type= LVAL_ERR;
	ret->err = malloc(strlen(msg) + 1);
	strcpy(ret->err, msg);

	ret->cell = NULL;
	ret->sym = NULL;

	return ret;
}

lval *lval_qexpr (void) {
	lval *ret = malloc(sizeof(lval));
	ret->type= LVAL_QEXPR;
	ret->count = 0;

	ret->cell = NULL;
	ret->sym = NULL;
	ret->err = NULL;

	return ret;
}

lval *lval_sexpr (void) {
	lval *ret = malloc(sizeof(lval));
	ret->type= LVAL_SEXPR;
	ret->count = 0;

	ret->cell = NULL;
	ret->sym = NULL;
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
	if  (l->type== LVAL_NUM) {
		printf("%li", l->num);
	}
	if  (l->type== LVAL_SYM) {
		printf("%s", l->sym);
	}
	if  (l->type== LVAL_ERR) {
		printf("Error: %s", l->err);
	}
	if  (l->type== LVAL_SEXPR ) {
		lval_expr_print('(', l, ')');
	}
	if  (l->type== LVAL_QEXPR ) {
		lval_expr_print('{', l, '}');
	}
	if  (l->type== LVAL_FUN ) {
		printf("<function>");
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
	if (strstr(t->tag, "symbolic")) {
		return lval_sym(t->contents);
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
	switch (l->type) {
		case LVAL_NUM:
			break;
		case LVAL_FUN:
			break;
		case LVAL_SYM:
			free(l->sym);
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

lval *builtin_op (lenv *e, lval *l, char *op){
	for (int i = 0; i < l->count; i++) {
		if (l->cell[i]->type != LVAL_NUM) {
			lval_del(l);
			return lval_err("bad num");
		}
	}

	lval *x = lval_pop(l, 0);
	if (l->count == 1 && l->count == 0) {
		x->num = -x->num;
	}

	while(l->count > 0) {
		lval *y = lval_pop(l, 0);
		if (strcmp(op, "+") == 0) {
			x->num += y->num; 
		}
		if (strcmp(op, "-") == 0) {
			x->num -= y->num; 
		}
		if (strcmp(op, "*") == 0) {
			x->num *= y->num; 
		}
		if (strcmp(op, "/") == 0) {
			if (y->num == 0) {
				lval_del(x);
				lval_del(y);
				x = lval_err("bad operator");
				break;
			}
			x->num /= y->num; 
		}
		if (strcmp(op, "%") == 0) {
			x->num %= y->num; 
		}
		lval_del(y);
	}
	lval_del(l);
	return x;
}

lval *builtin_add (lenv *e, lval *l){
	return builtin_op(e, l, "+");
}

lval *builtin_sub (lenv *e, lval *l){
	return builtin_op(e, l, "-");
}

lval *builtin_mul (lenv *e, lval *l){
	return builtin_op(e, l, "*");
}

lval *builtin_div (lenv *e, lval *l){
	return builtin_op(e, l, "/");
}

lval *builtin_mod (lenv *e, lval *l){
	return builtin_op(e, l, "%");
}


lval *builtin_def(lenv *e, lval *l) {
	if(l->cell[0]->type != LVAL_QEXPR) {
		lval_del(l);
		return lval_err("not q expression");
	}
	lval *syms = l->cell[0];
	for(int i = 0; i < syms->count; i++) {
		if(syms->cell[i]->type != LVAL_SYM) {
			lval_del(l);
			return lval_err("not symbolic inside q expression");
		}
	}
	if (syms -> count != l->count -1) {
		lval_del(l);
		return lval_err("sym number is not equal to number counter");
	}
	for (int i = 0; i < syms->count; i++) {
		lenv_put(e, syms->cell[i], l->cell[i+1]);
	}
	lval_del(l);
	return lval_sexpr();
}

lval *builtin_list(lenv *e, lval *l) {
	l->type= LVAL_QEXPR;
	return l;
}

lval *lval_join(lval *x, lval *y);

lval *builtin_join(lenv *e, lval *l) {
	for (int i = 0; i < l->count;  i++) {
		if (l->cell[0] -> type!= LVAL_QEXPR) {
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

lval *lval_take(lval *v, int i);

lval *builtin_tail(lenv *e, lval *l) {
	LASSERT(l, l->count == 1, "function 'tail' passed too many arguments");
	LASSERT(l, l->cell[0]->type== LVAL_QEXPR, "function 'tail' passed too many arguments");
	LASSERT(l, l->cell[0]->count != 0, "function 'tail' passed {}");
	lval* v = lval_take(l, 0);
	lval_del(lval_pop(v, 0));
	return v;
}


lval *builtin_head(lenv *e, lval *l) {
	LASSERT(l, l->count == 1, "function 'head' passed too many arguments");
	LASSERT(l, l->cell[0]->type== LVAL_QEXPR, "function 'head' passed too many arguments");
	LASSERT(l, l->cell[0]->count != 0, "function 'head' passed {}");
	lval *v = lval_take(l, 0);
	while(v->count > 1) {
		lval_del(lval_pop(v, 1));
	}
	return v;
}

void lenv_fill_builtin(lenv *e, char *name, lbuiltin func)  {
	lval *k = lval_sym(name);
	lval *v = lval_fun(func);
	lenv_put(e, k, v);
	lval_del(k);
	lval_del(v);
}
void lenv_fill_builtins(lenv *e) {
	lenv_fill_builtin(e, "add",  builtin_add);
	lenv_fill_builtin(e, "sub",  builtin_sub);
	lenv_fill_builtin(e, "div",  builtin_div);
	lenv_fill_builtin(e, "mul",  builtin_mul);
	lenv_fill_builtin(e, "mod",  builtin_mod);

	lenv_fill_builtin(e, "head", builtin_head);
	lenv_fill_builtin(e, "list", builtin_list);
	lenv_fill_builtin(e, "tail", builtin_tail);
	lenv_fill_builtin(e, "join", builtin_join);

	lenv_fill_builtin(e, "def",  builtin_def);
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

lval *lval_pop(lval *v, int i) {
	lval *x = v->cell[i];
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));
	v->count--;
	v->cell = realloc(v->cell, sizeof(lval*)*v->count);
	return x;
}

lval *lval_eval(lenv *e, lval *l);

lval *lval_sexpr_eval(lenv *e, lval *l) {
	for(int i = 0; i < l->count; i++) {
		l->cell[i] = lval_eval(e, l->cell[i]);
	}
	for(int i = 0; i < l->count; i++) {
		if (l->cell[i]->type== LVAL_ERR) {
			return lval_take(l, i);
		}
	}
	if (l->count == 0) {
		return l;
	}
	if (l -> count == 1) {
		return lval_take(l, 0);
	}
	lval *f = lval_pop(l, 0);
	
	if (f->type!= LVAL_FUN) {
		lval_del(f);
		lval_del(l);
		return lval_err("not legall symbolic");
	}
	lval *ret = f->fun(e, l);
	lval_del(f);
	return ret;
}

lval *lval_eval(lenv *e, lval *l) {
	if (l->type == LVAL_SYM) {
			lval* x = lenv_get(e, l);
			lval_del(l);
			return x;
	}
	if (l->type == LVAL_SEXPR) {
		return lval_sexpr_eval(e, l);
	}
	return l;
}


int main(int argc, char** argv) {
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbolic = mpc_new("symbolic");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Lispy = mpc_new("lispy");
	mpc_parser_t* Exit = mpc_new("exit");

	// operator : '+' | '-' | '*' | '/' | \"mod\" | \"head\" | \"tail\" | \"list\" | \"join\";           
	// we move these operator to builtin function
	// and using symbolic to represent variable
	
	mpca_lang(MPCA_LANG_DEFAULT,
			"  							                      \
				exit    : \"\\q\" ;                          \
				number  : /-?[0-9]+/ ;                         \
				symbolic : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;  \
				sexpr   :  '(' <expr>* ')' ;   \
				qexpr   :  '{' <expr>* '}' ;   \
				expr    :  <number>  | <symbolic> | <sexpr> | <qexpr> ; \
				lispy   : /^/ <exit> /$/ | /^/ <expr>* /$/ ;         \
			",
			Exit, Number, Symbolic, Sexpr, Qexpr, Expr, Lispy);

	puts("Lispy Version 0.0.0.0.1");
	puts("Press \\q to Exit\n");

	lenv *env = lenv_new();
	lenv_fill_builtins(env);
	while(1) {
		char* input = readline("lispy> ");
		add_history(input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			lval *t = lval_eval(env, (lval_read(r.output)));
			lval_print(t);
			putchar('\n');
			lval_del(t);
			mpc_ast_delete(r.output);
		}else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}
	lenv_del_all(env);
	mpc_cleanup(6, Exit, Number, Symbolic, Sexpr, Qexpr, Expr, Lispy);
	return 0;
}
