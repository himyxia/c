#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <editline/history.h>
#include "../mpc/mpc.h"

// adding function feature

/*
 * A third method is to think of functions as partial computations. Like the Mathematical model they can take some inputs. These values are required before the function can complete the computation. This is why it is called partial. But like the computational model, the body of the function consists of a computation specified in some language of commands. These inputs are called "unbound variables", and to finish the computation one simply supplies them. Like fitting a cog into a machine which previously spinning aimlessly, this completes all that is needed for the computation to run, and the machine runs. The output of these partial computations is itself a variable with an unknown value. This output can be placed as input to a new function, and so one function relies on another.
 */
/*
 * The first step will be to write a builtin function that can create user defined functions. Here is one idea as to how it can be specified. The first argument could be a list of symbols, just like our def function. These symbols we call the formal arguments, also known as the unbound variables. They act as the inputs to our partial computation. The second argument could be another list. When running the function this is going to be evaluated with our builtin eval function.
 *
 * This function we'll call just \, (a homage to The Lambda Calculus as the \ character looks a little bit like a lambda). To create a function which takes two inputs and adds them together, we would then write something like this.
 * eg: 
 * 		\ {x y} {+ x y}
 * 		(\ {x y} {+ x y}) 10 20
 *	    def {add-together} (\ {x y} {+ x y})
 *	    add-together 10 20
 */

/*
 * function consist of :
 * 	formal arguments
 * 		we must bind before we can evaluate the function
 * 	2, the q-expr to represent the function body
 * 	3, a location to store the values assigned to the formal arguments
 */

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;
enum {LVAL_NUMBER, LVAL_ERR, LVAL_FUN, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR};
typedef lval*(*lbuiltin)(lenv*, lval*);
/*
 * We will store our builtin functions and user defined functions under the same type LVAL_FUN. 
 * This means we need a way internally to differentiate between them. 
 * To do this we can check if the lbuiltin function pointer is NULL or not. 
 * If it is not NULL we know the lval is some builtin function, otherwise we know it is a user function.
 */
struct lval {
	int type;
	long num;
	char* err;
	char* sym;

	lbuiltin builtin;
	lenv* env;
	lval* formals;
	lval* body;

	int count;
	lval** cell;
};

// create constructor for lval 
lval* lval_lambda(lval* formals, lval* body) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;
	v->builtin = NULL;
	v->env = lenv_new();
	v->formals = formals;
	v->body = body;
	return v;
}

lval* builtin_lambda(lenv* e, lval* a) {
	LASSERT_NUM("\\", a, 2);
	LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
	LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

	for(int i = 0; i < a->cell[0]->count; i++) {
		LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
			"Cannot define non-symbol. Got %s, Expected %s.",
			ltype_name(a->cell[0]->cell[i]->type),ltype_name(LVAL_SYM));
	}

	lval* formals = lval_pop(a, 0);
	lval* body = lval_pop(a, 0);
	lval_delete(a);
	return lval_lambda(formals, body);
}
lval* lval_fun(lbuiltin builtin) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;
	v->builtin = builtin;
	return v;
}
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

lval* lval_err(char* fmt, ...) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;

	va_list va;
	va_start(va, fmt);

	v->err = malloc(512);
	vsnprintf(v->err, 511, fmt, va);
	v->err = realloc(v->err, strlen(v->err) + 1);
	va_end(va);
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
		case LVAL_FUN: 
			if(!v->builtin) {
				lenv_del(v->env);
				lval_delete(v->formals);
				lval_delete(v->body);
			}
			break;
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

lval* lval_copy(lval* v) {
	lval* x = malloc(sizeof(lval));
	x->type = v->type;
	switch(v->type) {
		case LVAL_FUN: 
			if(v->builtin) {
				x->builtin = v->builtin; 
			}else {
				x->builtin = NULL;
				x->env = lenv_copy(v->env);
				x->formals = lval_copy(v->formals);
				x->body = lval_copy(v->body);
			}
			break;
		case LVAL_NUMBER: 
			x->num = v->num; 
			break;
		case LVAL_ERR:
			x->err = malloc(strlen(v->err) + 1);
			strcpy(x->err, v->err);
			break;
		case LVAL_SYM:
			x->sym = malloc(strlen(v->sym) + 1);
			strcpy(x->sym, v->sym);
			break;
		case LVAL_SEXPR:
		case LVAL_QEXPR:
			x->count = v->count;
			x->cell = malloc(sizeof(lval*)*x->count);
			for(int i = 0; i < x->count; i++) {
				x->cell[i] = lval_copy(v->cell[i]);
			}
			break;
	}
	return x;
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*)*v->count);
	v->cell[v->count-1] = x;
	return v;
}

char* ltype_name(int t) {
	switch(t) {
		case LVAL_FUN:
			return "Function";
		case LVAL_NUMBER:
			return "Number";
		case LVAL_SYM:
			return "Symbol";
		case LVAL_ERR:
			return "Error";
		case LVAL_SEXPR:
			return "S-Expression";
		case LVAL_QEXPR:
			return "Q-Expression";
		default: return "Unknown";
	}
}
/* environment */
// Our environment structure must encode a list of relationships between names and values. There are many ways to build a structure that can do this sort of thing. We are going to go for the simplest possible method that works well. This is to use two lists of equal length. One is a list of lval*, and the other is a list of char*. Each entry in one list has a corresponding entry in the other list at the same position.
struct lenv {
	lenv* par;
	int count;
	char** syms;
	lval** vals;
};

lenv* lenv_new(void) {
	lenv* e = malloc(sizeof(lenv));
	e->par = NULL;
	e->count = 0;
	e->syms = NULL;
	e->vals = NULL;
	return e;
}
void lenv_del(lenv* e) {
	for(int i = 0; i < e->count; i++) {
		free(e->syms[i]);
		lval_delete(e->vals[i]);
	}
	free(e->syms);
	free(e->vals);
	free(e);
}

lenv* lenv_copy(lenv* e) {
	lenv* n = malloc(sizeof(lenv));
	n->par = e->par;
	n->count = e->count;
	n->syms = malloc(sizeof(char*)*n->count);
	n->vals = malloc(sizeof(lval*)*n->count);
	for(int i = 0; i < e->count; i++) {
		n->syms[i] = malloc(strlen(e->syms[i]) + 1);
		strcpy(n->syms[i], e->syms[i]);
		n->vals[i] = lval_copy(e->vals[i]);
	}
	return n;
}

// we use lval's symbol as variable name~!!
lenv* lenv_get(lenv* e, lval* k) {
	for(int i = 0; i < e->count; i++) {
		if(strcmp(e->syms[i], k->sym) == 0) {
			return lval_copy(e->vals[i]);
		}
	}
	if(e->par) {
		return lenv_get(e->par, k);
	}else {
		return lval_err("Unbound Symbol '%s'", k->sym);
	}
}
// symbol is a lval*, value is also a lval*
void lenv_put(lenv* e, lval* k, lval* v) {
	for(int i = 0; i < e->count; i++) {
		if(strcmp(e->syms[i], k->sym) == 0) {
			lval_delete(e->vals[i]);
			e->vals[i] = lval_copy(v);
			return;
		}
	}
	e->count++;
	e->vals = realloc(e->vals, sizeof(lval*)*e->count);
	e->syms = realloc(e->syms, sizeof(char*)*e->count);

	e->vals[e->count-1] = lval_copy(v);
	e->syms[e->count-1] = malloc(strlen(k->sym) + 1);
	strcpy(e->syms[e->count-1], k->sym);
}

void lenv_def(lenv* e, lval* k, lval* v) {
	while(e->par) {
		e = e->par;
	}
	lenv_put(e, k, v);
}

// first: convert the abstract syntax tree into an S-Expression
lval* lval_read(mpc_ast_t* t) {
	if(strstr(t->tag, "number")) {
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE?lval_num(x):lval_err("invalid number %s", t->contents);
	}
	if(strstr(t->tag, "symbol")) {
		return lval_sym(t->contents);
	}

	lval* x = NULL;
	if (strcmp(t->tag, ">")==0) {
		// means it is root, we use sexpr to represent it 
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
	 case LVAL_FUN:
		if (v->builtin) {
			printf("<builtin function>");
		}else {
			printf("(\\ "); 
			lval_print(v->formals);
			putchar(' ');
			lval_print(v->body);
			putchar(')');
		}
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

lval* lval_eval_sexpr(lenv* e, lval* v);

lval* lval_eval(lenv* e, lval* k) {
	if(k->type == LVAL_SYM) {
		lval* x = lenv_get(e, k);
		lval_delete(k);
		return x;
	}
	if (k->type == LVAL_SEXPR) {
		return lval_eval_sexpr(e, k);
	}
	return k;
}

//converts the input S-Expression to a Q-Expression and returns it.
lval* builtin_list(lenv* e, lval* a) {
	a->type = LVAL_QEXPR;
	return a;
}

#define LASSERT(args, cond, fmt, ...) \
	if(!(cond)) { lval* err = lval_err(fmt, ##__VA_ARGS__);lval_delete(args);return err;}
#define LASSERT_TYPE(func, args, index, expect) \
	LASSERT(args, args->cell[index]->type == expect, \
		"Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.",\
		func, index, ltype_name(args->cell[index]->type), ltype_name(expect))
#define LASSERT_NUM(func, args, num) \
	LASSERT(args, args->count == num, \
		"Function '%s' passed incorrect number of arguments, Got %i, Expected %i.", func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
	LASSERT(args, args->cell[index]->count != 0, \
		"Function '%s' passed {} for argument %i.", func, index);

//takes as input some single Q-Expression, which it converts to an S-Expression, and evaluates using lval_eval
lval* builtin_eval(lenv* e, lval* a) {
	LASSERT_NUM("eval", a, 1);
	LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);
	lval* x = lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return lval_eval(e,x);
}

lval* lval_join(lval* x, lval* y) {
	while(y->count) {
		x = lval_add(x, lval_pop(y, 0));
	}
	lval_delete(y);
	return x;
}

lval* builtin_join(lenv* e, lval* a) {
	for(int i = 0; i < a->count; i++) {
		LASSERT_TYPE("join", a, 0, LVAL_QEXPR);
	}
	lval* x = lval_pop(a, 0);
	while(a->count) {
		x = lval_join(x, lval_pop(a, 0));
	}
	lval_delete(a);
	return x;
}
lval* builtin_head(lenv* e, lval* a) {
	LASSERT_NUM("head", a, 1);
	LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
	LASSERT_NOT_EMPTY("head", a, 0);
	lval* v = lval_take(a, 0);
	while(v->count > 1) {
		lval_delete(lval_pop(v, 1));
	}
	return v;
}

lval* builtin_tail(lenv* e, lval* a) {
	LASSERT_NUM("tail", a, 1);
	LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
	LASSERT_NOT_EMPTY("tail", a, 0);
	lval* v = lval_take(a, 0);
	while(v->count > 1) {
		lval_delete(lval_pop(v, 1));
	}
	return v;
}

lval* builtin_op(lenv* e, lval* a, char* op) {
	for(int i = 0; i < a->count; i++) {
		LASSERT_TYPE(op, a, i, LVAL_NUMBER);
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

lval* lval_eval_sexpr(lenv* e, lval* v) {
	for (int i = 0; i < v->count; i++) {
		v->cell[i] = lval_eval(e, v->cell[i]);
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
	if (f->type != LVAL_FUN) {
		lval* err = lval_err(
			"s-expression starts with incorrect type. "
			"Got %s, Expected %s.",
			ltype_name(f->type), ltype_name(LVAL_FUN));

		lval_delete(v);
		lval_delete(f);
		return err;
	}

	lval* result = lval_call(e, f, v);
	lval_delete(f);
	return result;
}

lval* builtin_add(lenv* e, lval* a) {
	return builtin_op(e, a, "+");
}
lval* builtin_sub(lenv* e, lval* a) {
	return builtin_op(e, a,"-");
}
lval* builtin_mul(lenv* e, lval* a) {
	return builtin_op(e, a, "*");
}
lval* builtin_div(lenv* e, lval* a) {
	return builtin_op(e, a, "/");
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin builtin) {
	lval* k = lval_sym(name);
	lval* v = lval_fun(builtin);
	lenv_put(e, k, v);
	lval_delete(k);
	lval_delete(v);
}


lval* builtin_var(lenv* e, lval* a, char* func) {
	LASSERT_TYPE(func, a, 0, LVAL_QEXPR);
	lval* syms = a->cell[0];
	for(int i = 0; i < syms -> count; i++) {
		LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
			"Function '%s' cannot define non-symbol!"
			"Got %s, Expected %s.", func,
			ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
	}

	LASSERT(a, (syms->count == a->count-1),
		"Function '%s' cannot define incorrect number of values to symbols!"
		"Got %i, Expected %i.", func,
		syms->count, a->count-1);
	
	for(int i = 0; i < syms->count; i++) {
		if(strcmp(func, "def") == 0) {
			lenv_def(e, syms->cell[i], a->cell[i+1]);
		}
		if(strcmp(func, "=") == 0) {
			lenv_put(e, syms->cell[i], a->cell[i+1]);
		}
	}
	lval_delete(a);
	return lval_sexpr();
}

lval* builtin_def(lenv* e, lval* a) {
	return builtin_var(e, a, "def");
}
lval* builtin_put(lenv* e, lval* a) {
	return builtin_var(e, a, "=");
}

void lenv_add_builtins(lenv* e) {
	lenv_add_builtin(e, "list", builtin_list);
	lenv_add_builtin(e, "head", builtin_head);
	lenv_add_builtin(e, "tail", builtin_tail);
	lenv_add_builtin(e, "eval", builtin_eval);
	lenv_add_builtin(e, "join", builtin_join);


	lenv_add_builtin(e, "+", builtin_add);
	lenv_add_builtin(e, "-", builtin_sub);
	lenv_add_builtin(e, "*", builtin_mul);
	lenv_add_builtin(e, "/", builtin_div);

	lenv_add_builtin(e, "def", builtin_def);
	lenv_add_builtin(e, "=", builtin_put);
}

lval* lval_call(lenv* e, lval* f, lval* a) {
	if(f->builtin) {
		return f->builtin(e, a);
	}

	int given = a->count;
	int totoal = f->formals->count;

	while(a->count) {
		if(f->formals->count == 0) {
			lval_delete(a);
			return lval_err(
					"Function passed too many arguments. "
					"Got %i, Expected %i.", given, total);
		}
		lval* sym = lval_pop(f->formals, 0);
		// variable arguments
		if(strcmp(sym->sym, "&") == 0) {
			if(f->formals->count != 1) {
				lval_delete(a);
				return lval_err("Function format invalid. "
						"Symbol '&' not followed by single simpbol");
			}
			lval* nsym = lval_pop(f->formals, 0);
			lenv_put(f->env, nsym, builtin_list(e, a));
			lval_delete(sym); 
			lval_delete(nsym); 
			break;
		}
		lval* val = lval_pop(a, 0);
		lenv_put(f->env, sym, val);
		lval_delete(sym);
		lval_delete(val);
	}
	lval_delete(a);
	if(f->formals->count > 0 &&
		strcmp(f->formats->cell[0]->sym, "&") == 0) {
		if(f->formals->count != 2) {
			return lval_err("Function format invalid."
					"Symbol '&' not followed by single symbol.");
		}
		lval_delete(lval_pop(f->formals, 0));
		lval* sym = lval_pop(f->formals, 0);
		lval* val = lval_qexpr();
		lenv_put(f->env, sym, val);
		lval_delete(sym);
		lval_delete(val);
	}

	if(f->formals->count == 0) {
		f->env->par = e;
		return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body));
	}else {
		return lval_copy(f);
	}
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

	mpca_lang(MPCA_LANG_DEFAULT,
			"   \
			number : /-?[0-9]+/; \
			symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/; \
			sexpr : '(' <expr>* ')'; \
			qexpr : '{' <expr>* '}'; \
			expr : <number> | <symbol> | <sexpr> | <qexpr> ; \
			lispy : /^/ <expr>* /$/; \
			",
			Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
	lenv* e = lenv_new();
	lenv_add_builtins(e);
	while(1) {
		char* input = readline("lispy> ");
		add_history(input);
		mpc_result_t r;
		if(mpc_parse("<stdin>", input, Lispy, &r)) {
			mpc_ast_print(r.output);
			lval* result = lval_eval(e, lval_read(r.output));
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
	lenv_del(e);
	return 0;
} 
