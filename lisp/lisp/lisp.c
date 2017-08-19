#include "mpc/mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <editline/history.h>

struct lval;
struct lenv;

typedef struct lval lval;
typedef struct lenv lenv;

typedef lval* (*lbuiltin)(lenv *e, lval *l);

typedef struct lenv{
	lenv *par;
	int count;
	char** syms;
	lval** vals;
} lenv;

struct lval{
	int type;
	long num;
	char *sym;
	char *err;

	lbuiltin builtin;

	lval *formals;
	lval *body;
	lenv *env;

	int count;
	lval **cell;
} ;

lenv *lenv_new();
lval *lval_copy(lval *l);
void lenv_put(lenv *e, lval *k, lval *v);

enum {LVAL_EXIT, LVAL_FUN, LVAL_NUM, LVAL_SEXPR, LVAL_QEXPR, LVAL_ERR, LVAL_SYM};

char *ltype_name(int type) {
	switch (type) {
		case LVAL_FUN:
			return "function type";
		case LVAL_NUM:
			return "number";
		case LVAL_ERR :
			return "error";
		case LVAL_SYM:
			return "symbolic";
		case LVAL_SEXPR :
			return "symbolic expression";
		case LVAL_QEXPR :
			return "quote expression";
	}
	return "unknown type";
}

lval *lval_lambda() {
	lval *l = malloc(sizeof(lval));
	l->builtin = NULL;
	l->type = LVAL_FUN;
	l->formals= NULL;
	l->body = NULL;
	l->env = lenv_new();
	return l;
}

lval *lval_sexpr() {
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_SEXPR;
	l->count = 0;
	l->cell = NULL;
	return l;
}

lval *lval_qexpr() {
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_QEXPR;
	l->count = 0;
	l->cell = NULL;
	return l;

}
lval *lval_sym(char *sym) {
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_SYM;
	l->sym = malloc(strlen(sym) + 1);
	strcpy(l->sym, sym);
	return l;
}

lval *lval_fun(lbuiltin builtin) {
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_FUN;
	l->builtin = builtin;
	return l;
}

lval *lval_err(char *fmt, ...) {
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_ERR;
	va_list va;
	va_start(va, fmt);
	l->err = malloc(512);
	vsnprintf(l->err, 511, fmt, va);
	l->err = realloc(l->err, strlen(l->err)+ 1);
	va_end(va);
	return l;
}

lval *lval_num(long num) {
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_NUM;
	l->num = num;
	return l;
}

lval *lval_add(lval *x, lval *y);

lenv *lenv_copy(lenv *l);

void lval_del(lval *l);


void lenv_del(lenv *e) {
	for(int i = 0; i < e->count; i++) {
		free(e->syms[i]);
		lval_del(e->vals[i]);
	}
	free(e->syms);
	free(e->vals);
	free(e);
}

lenv *lenv_new() {
	lenv *e = malloc(sizeof(lenv));
	e->count = 0;
	e->par = NULL;
	e->syms = NULL;
	e->vals = NULL;
	return e;
}

lval *lval_pop(lval *l , int i);

#define LASSERT(con, args, fmt, ...) \
	if(!con) {lval* err = lval_err(fmt, ##__VA_ARGS__); return err;}

#define LASSERT_NUM(fun, args, num) \
	LASSERT(args->count == num, args, "function %s pass wrong type, expected %i got %i", fun, num, args->count)

#define LASSERT_TYPE(fun, args, expect, index) \
	LASSERT(args->cell[index]->type == expect, args, "function %s pass wrong type, expected %s got %s", fun, ltype_name(expect), ltype_name(args->cell[index]->type))

#define LASSERT_NOT_EMPTY(fun, args, index) \
	LASSERT(args->cell[index]->count != 0, args, "function %s pass empty set", fun)

lval *builtin_op(lenv *e, char *op,  lval *l) {
	for (int i = 0; i < l->count; i++) {
		LASSERT_TYPE(op, l, LVAL_NUM, i);
	}

	lval *x = lval_pop(l, 0);

	if (strcmp("-", op) == 0 && l->count == 0) {
		x->num = - x->num;
		return x;
	}
	
	while (l->count) {
		lval *y = lval_pop(l, 0);
		if (strcmp("+", op) == 0) { x->num += y->num; }
		if (strcmp("-", op) == 0) { x->num -= y->num; }
		if (strcmp("*", op) == 0) { x->num *= y->num; }
		if (strcmp("^", op) == 0) { 
			x -> num <<= (y->num-1);
		}
		if (strcmp("%", op) == 0) { x->num %= y->num; }
		if (strcmp("/", op) == 0) { 
			if (y->num == 0) {
				x = lval_err("divide by zero");
			}
			x->num /= y->num; 
		}
		lval_del(y);
	}
	return x;
}

lval *builtin_expn(lenv *e, lval *l) {
	return builtin_op(e, "^", l);
}

lval *builtin_add(lenv *e, lval *l) {
	return builtin_op(e, "+", l);
}

lval *builtin_sub(lenv *e, lval *l) {
	return builtin_op(e, "-", l);
}

lval *builtin_mul(lenv *e, lval *l) {
	return builtin_op(e, "*", l);
}

lval *builtin_div(lenv *e, lval *l) {
	return builtin_op(e, "/", l);
}

lval *builtin_mod(lenv *e, lval *l) {
	return builtin_op(e, "%", l);
}

lval *builtin_list(lenv *e, lval *l) {
	l->type = LVAL_QEXPR;
	return l;
}

lval *lval_take(lval *l , int i);

lval *builtin_head(lenv *e, lval *l) {
	LASSERT_NUM("head", l, 1);
	LASSERT_TYPE("head", l, LVAL_QEXPR, 0);
	LASSERT_NOT_EMPTY("head", l, 0);

	lval *ret = lval_copy(l->cell[0]);
	while(ret->count > 1) {
		lval_del(lval_pop(ret, 1));
	}
	return ret;
}


lval *builtin_tail(lenv *e, lval *l) {
	LASSERT_NUM("tail", l, 1);
	LASSERT_TYPE("tail", l, LVAL_QEXPR, 0);
	LASSERT_NOT_EMPTY("tail", l, 0);

	lval *x = lval_copy(l->cell[0]);
	lval_del(lval_pop(x, 0));
	return x;
}

lval *builtin_join(lenv *e, lval *l) {
	LASSERT_NUM("join", l, 2);
	for (int i= 0; i < l->count; i++) {
		LASSERT_TYPE("join", l, LVAL_QEXPR, i);
	}
	//TODO we may need to lval_join() func
	lval *x = lval_pop(l, 0);
	lval *y = lval_pop(l, 0);
	while(y->count) {
		lval_add(x, lval_copy(lval_pop(y, 0)));
	}
	lval_del(y);
	return x;
}

lval *builtin_def(lenv *e, lval *l) {
	LASSERT_TYPE("def", l, LVAL_QEXPR, 0);
	LASSERT_NOT_EMPTY("def", l, 0);

	lval *syms = l->cell[0];
	for (int i = 0; i < syms->count; i++) {
		LASSERT_TYPE("def", syms, LVAL_SYM, i);
	}
	LASSERT_NUM("def", syms, l->count);
	while(e->par) {
		e = e->par;
	}
	for (int i = 0; i < syms ->count; i++ ) {
		lenv_put(e, syms->cell[i], l->cell[i+1]);
	}
	return lval_sexpr();
}

lval *builtin_lambda(lenv *e, lval *l) {
	LASSERT_NUM("lambda", l, 2);
	
	for(int i = 0; i < l->count; i++) {
		LASSERT_TYPE("lambda", l, LVAL_QEXPR, i);
	}

	lval *la = lval_lambda();
	la->env = lenv_new();
	la->formals = lval_copy(l->cell[0]);
	la->body = lval_copy(l->cell[1]);
	return la;
}

void lenv_add_builtin(lenv *e, char *s, lbuiltin f) {
	lval *k = lval_sym(s);
	lval *v = lval_fun(f);
	lenv_put(e, k, v);
	lval_del(k);
	lval_del(v);
}


void lenv_add_builtins(lenv *e) {
	// add, sub, div. mul, mod
	lenv_add_builtin(e, "+", builtin_add);
	lenv_add_builtin(e, "-", builtin_sub);
	lenv_add_builtin(e, "*", builtin_mul);
	lenv_add_builtin(e, "/", builtin_div);
	lenv_add_builtin(e, "%", builtin_mod);
	lenv_add_builtin(e, "^", builtin_expn);

	lenv_add_builtin(e, "list", builtin_list);
	lenv_add_builtin(e, "head", builtin_head);
	lenv_add_builtin(e, "tail", builtin_tail);
	lenv_add_builtin(e, "join", builtin_join);

	lenv_add_builtin(e, "def", builtin_def);
	lenv_add_builtin(e, "\\", builtin_lambda);
}

void lval_print(lval *l);

void lval_print_expr (char open, lval *l, char end){
	putchar(open);
	for (int i = 0; i < l->count; i++) {
		lval_print(l->cell[i]);
		if (i != l->count-1) {
			putchar(' ');
		}
	}
	putchar(end);

}

void lval_print_lambda (lval *l){
	printf("<user defined function>");
	putchar('(');
	putchar('\\');
	putchar(' ');
	lval_print(l->formals);
	putchar(' ');
	lval_print(l->body);
	putchar(')');
}

void lval_print(lval *l) {
	switch (l->type) {
		case LVAL_NUM:
			printf("%li", l->num);
			break;
		case LVAL_SYM:
			printf("%s", l->sym);
			break;
		case LVAL_FUN:
			if (l->builtin) {
				printf("<builtin-function> %p", l->builtin);
			}else {
				// lambda
				lval_print_lambda(l);
			}
			break;
		case LVAL_ERR:
			printf("ERR: %s", l->err);
			break;
		case LVAL_SEXPR:
			lval_print_expr('(', l, ')');
			break;
		case LVAL_QEXPR:
			lval_print_expr('{', l, '}');
			break;
	}
}

void lval_println(lval *l) {
	lval_print(l);
	putchar('\n');
}

lval *lval_eval(lenv *e, lval *l);

lval *builtin_eval(lenv *e, lval *l) {
	l->type = LVAL_SEXPR;
	return lval_eval(e, l);
}


lenv *lenv_copy(lenv *l) {
	lenv *e = lenv_new();
	if (l->par) {
		e->par = l->par;
	}
	for (int i = 0; i < l->count; i++) {
		lval *tmp = lval_sym(l->syms[i]);
		lenv_put(e, tmp, l->vals[i]);
		lval_del(tmp);
	}
	return e;
}

void lval_del(lval *l) {
	switch (l->type) {
		case LVAL_FUN:
			if (l->builtin == NULL) {
				lval_del(l->formals);
				lval_del(l->body);
				lenv_del(l->env);
			}
			break;
		case LVAL_NUM:
			break;

		case LVAL_ERR :
			free(l->err);
			break;
		case LVAL_SYM:
			free(l->sym);
			break;

		case LVAL_SEXPR :
		case LVAL_QEXPR :
			for(int i = 0; i < l->count; i++) {
				lval_del(l->cell[i]);
			}
			free(l->cell);
			break;
	}
	free(l);
}

void lenv_put(lenv *e, lval *k, lval *v) {
	for(int i = 0; i < e->count; i++) {
		if(strcmp(e->syms[i], k->sym)==0) {
			// override
			lval_del(e->vals[i]);
			e->vals[i] = lval_copy(v);
			return;
		}
	}

	e->count++;
	e->syms = realloc(e->syms, sizeof(lval*) * (e->count));
	e->vals = realloc(e->vals, sizeof(lval*) * (e->count));

	e->syms[e->count-1] = malloc(strlen(k->sym)+1);
	strcpy(e->syms[e->count-1], k->sym);

	// for decouple, it's better to copy
	e->vals[e->count-1] = lval_copy(v);
}

lval *lval_call(lenv *e, lval *f, lval *l) {
	if (f->builtin != NULL) {
		// builtin function
		return f->builtin(e, l);
	}

	int total = f->formals->count;
	int given = l->count;
	while(l->count) {
		if (f->formals->count == 0) {
			return lval_err("invalid expression: pass too many argument Got %i, Expected %i", given, total);
		}
		lval *x = lval_pop(f->formals, 0);
		if (strcmp(x->sym, "&") == 0) {
			if (f->formals->count != 1) {
				lval_del(x);
				return lval_err("invalid expression: should have only one paramenter after &");
			}
			lval *xs = lval_pop(f->formals, 0);
			lenv_put(f->env, xs,  builtin_list(e, lval_copy(l)));
			break;
		}
		lenv_put(f->env, x,  lval_pop(l, 0));
	}

	if (f->formals->count > 0 && strcmp(f->formals->cell[0]->sym, "&") == 0) {
		if (f->formals->count != 2) {
			return lval_err("invalid expression: should have only one paramenter after &");
		}
		lval_del(lval_pop(f->formals, 0));
		lval *sym = lval_pop(f->formals, 0);
		lval *val = lval_qexpr();
		lenv_put(f->env, sym,  val);
		lval_del(sym);
		lval_del(val);
	}

	if (f->formals->count == 0) {
		// it is also for decouple
		f->env->par = e;
		return builtin_eval(f->env,f->body);
	}else {
		return lval_copy(f);
	}
}

lval *lval_pop(lval *l , int i) {
	lval *r = l->cell[i];
	memmove(&l->cell[i], &l->cell[i+1], sizeof(lval*)*(l->count - i - 1)); // why does this work?, look at how they are generated
	l->count--;
	l->cell = realloc(l->cell, sizeof(lval*) * l->count);
	return r;
}

lval *lval_take(lval *l , int i) {
	lval *x = lval_pop(l, i);
	lval_del(l);
	return x;
}

lval *lval_add(lval *x, lval *y) {
	x->count++;
	x->cell = realloc(x->cell, sizeof(lval*) * (x->count));
	x->cell[x->count-1] = y;
	return x;
}

lval *lval_copy(lval *l) {
	lval *x = malloc(sizeof(lval));
	x->type = l->type;

	if (l->type == LVAL_NUM) {
		x->num = l->num;
	}
	if (l->type == LVAL_SYM) {
		x->sym = malloc(strlen(l->sym)+1);
		strcpy(x->sym, l->sym);
	}
	if (l->type == LVAL_ERR) {
		x->err = malloc(strlen(l->err)+1);
		strcpy(x->err, l->err);
	}
	if (l->type == LVAL_FUN) {
		if (l->builtin) {
			// builtin func
			x->builtin = l->builtin;
		}else {
			x->builtin = NULL;
			x->formals = lval_copy(l->formals);
			x->body = lval_copy(l->body);
			x->env = lenv_copy(l->env);
		}
	}
	if (l->type == LVAL_SEXPR || l->type == LVAL_QEXPR) {
		x->count = 0;
		x->cell = NULL;
		for (int i = 0; i < l->count; i++) {
			lval_add(x, lval_copy(l->cell[i]));
		}
	}
	return x;
}

lval *lenv_get(lenv *e, lval *k) {
	for(int i = 0; i < e->count; i++) {
		if(strcmp(e->syms[i], k->sym)==0) {
			return lval_copy(e->vals[i]);
		}
	}
	if(e->par) {
		return lenv_get(e->par, k);
	}
	return lval_err("Undefined symbolic %s", k->sym);
}

lval *lval_eval_sexpr(lenv *e, lval *l) {
	for(int i = 0; i < l->count; i++) {
		l->cell[i] = lval_eval(e, l->cell[i]);
	}
	for(int i = 0; i < l->count; i++) {
		if (l->cell[i]->type == LVAL_ERR) {
			return lval_take(l, i);
		}
	}
	if(l->count == 0) {
		return l;
	}
	if(l->count == 1) {
		return lval_take(l, 0);
	}
	lval *f = lval_pop(l, 0);

	if (f->type != LVAL_FUN) {
		lval *err = lval_err("Illegal Symbolic: expected %s,  got %s", ltype_name(LVAL_FUN), ltype_name(f->type));
		lval_del(f);
		return err;
	}
	lval *ret = lval_call(e, f, l);
	lval_del(f);
	return ret;
}

void lenv_println(lenv *e) {
	if (e->par != NULL ) {
		printf("par:->");
		lenv_println(e->par);
	}
	for(int i = 0; i < e->count; i++) {
		printf("%s", e->syms[i]);
		printf(":");
		lval_println(e->vals[i]);
	}
}

lval *lval_eval(lenv *e, lval *l) {
	if (l->type == LVAL_SYM) {
		if (strcmp(l->sym, "env")==0) {
			lenv_println(e);
			return lval_sexpr();
		}else {
			lval *f = lenv_get(e, l);
			return f;
		}
	}
	if (l->type == LVAL_SEXPR) {
		return lval_eval_sexpr(e, l);
	}
	return l;
}

lval *lval_read_num(mpc_ast_t *t) {
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err("number out of range");
}

lval *lval_read(mpc_ast_t *t) {
	if (strstr(t->tag, "exit")) {
		exit(0);
	}
	if (strstr(t->tag, "number")) {
		return lval_read_num(t);
	}
	if (strstr(t->tag, "symbolic")) {
		return lval_sym(t->contents);
	}
	lval *l = NULL;
	if(strcmp(t->tag, ">") == 0) {
		// root
		l = lval_sexpr();
	}
	if(strstr(t->tag, "sexpr")) {
		// subroot
		l = lval_sexpr();
	}
	if(strstr(t->tag, "qexpr")) {
		// subroot
		l = lval_qexpr();
	}
	for(int i = 0; i < t->children_num; i++) {
		if(strcmp(t->children[i]->tag, "regex") == 0) {
			// for root /^ /$
			continue;
		}
		if(strcmp(t->children[i]->contents, "(") == 0) {continue;}
		if(strcmp(t->children[i]->contents, ")") == 0) {continue;}
		if(strcmp(t->children[i]->contents, "{") == 0) {continue;}
		if(strcmp(t->children[i]->contents, "}") == 0) {continue;}
		lval_add(l, lval_read(t->children[i]));
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

	mpca_lang(MPCA_LANG_DEFAULT,
			"  							                      \
				exit    : \"\\q\" | \"q\" | \":q\" ;                          \
				number   : /-?[0-9]+/ ;                         \
				symbolic : /[a-zA-Z0-9_+\\-*\\/\\\\=<>^!&]+/ ;   \
				sexpr     :  '(' <expr>* ')' ;   \
				qexpr     :  '{' <expr>* '}' ;   \
				expr     :  <exit> | <number>  | <symbolic> | <sexpr> | <qexpr> ; \
				lispy   : /^/ <exit> /$/ | /^/ <expr>* /$/ ;         \
			",
			Exit, Number, Symbolic, Sexpr, Qexpr, Expr, Lispy);

	puts("Lispy Version 0.0.0.0.1");
	puts("Press \\q to Exit\n");

	lenv *e = lenv_new();
	lenv_add_builtins(e);

	while(1) {
		char* input = readline("lispy> ");
		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			lval *l = lval_eval(e, lval_read(r.output));
			lval_println(l);
			lval_del(l);
			mpc_ast_delete(r.output);
		}else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}
	lenv_del(e);
	mpc_cleanup(6, Exit, Number, Symbolic, Sexpr, Qexpr, Expr, Lispy);
	return 0;
}
