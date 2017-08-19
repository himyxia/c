#include "../mpc/mpc.h"
#include <editline/readline.h>
#include <editline/history.h>

typedef struct lval{
	int type;
	long num;
	char* err;
	// what is this?
	char* sym;
	int count;
	struct lval** cell;
} lval;

enum { LVAL_SYM, LVAL_QEXPR,LVAL_SEXPR, LVAL_NUM, LVAL_ERR};

//enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};


lval* lval_num(long x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
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
	v->err = malloc(strlen(m) + 1);
	strcpy(v->err, m);
	return v;
}

lval* lval_sym(char* s) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym=malloc(strlen(s) + 1);
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

lval* lval_read_num(mpc_ast_t* t) {
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ?
		lval_num(x):lval_err("invalid number");
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count-1] = x;
	return v;
}
/*
typedef struct mpc_ast_t {
	// eg: expr|number|regex
	// containing a list of all the rules used to parse that particular item
  char *tag;
  // for branches this is empty, but for leaves we can use it to find the operator or number to user
  char *contents;
  mpc_state_t state;
  int children_num;
  struct mpc_ast_t** children;
} mpc_ast_t;
*/

lval* lval_read(mpc_ast_t* t) {
	if (strstr(t->tag, "number")) {
		return lval_read_num(t);
	}

	if (strstr(t->tag, "symbol")) {
		return lval_sym(t->contents);
	}

	if (strstr(t->tag, "qexpr")) {
		return lval_qexpr();
	}

	lval* x = NULL;

	// what is this ?
	if (strcmp(t->tag, ">")==0) {
		x = lval_sexpr();
	}
	if(strstr(t->tag, "sexpr")) {
		x = lval_sexpr();
	}

	for (int i = 0; i< t->children_num; i++) {
		if (strcmp(t->children[i] ->contents, "(") == 0)  {
			continue;
		}
		if (strcmp(t->children[i] ->contents, ")") == 0)  {
			continue;
		}
		if (strcmp(t->children[i] ->contents, "}") == 0)  {
			continue;
		}
		if (strcmp(t->children[i] ->contents, "{") == 0)  {
			continue;
		}
		if (strcmp(t->children[i] ->tag, "regex") == 0)  {
			continue;
		}
		x = lval_add(x, lval_read(t->children[i]));
	}
	return x;
}

void lval_del(lval* v) {
	switch(v->type) {
		case LVAL_NUM: break;
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;
		case LVAL_QEXPR:
		case LVAL_SEXPR:
					   for(int i = 0; i < v->count; i++) {
						   lval_del(v->cell[i]);
					   }
					   free(v->cell);
					   break;
	}
	free(v);
}

// forward declaration
void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
	putchar(open);

	for(int i = 0; i < v->count; i++) {
		lval_print(v->cell[i]);
		if(i != (v->count-1)) {
			putchar(' ');
		}
	}
	putchar(close);
}

void lval_print(lval* v) {
	switch(v->type) {
		case LVAL_NUM:
			printf("%li", v->num);
			break;
		case LVAL_ERR:
			printf("Error: %s", v->err); 
			break;
		case LVAL_SYM:
			printf("%s", v->sym);
			break;
		case LVAL_SEXPR:
			lval_expr_print(v, '(', ')');
			break;
		case LVAL_QEXPR:
			lval_expr_print(v, '{', '}');
			break;
	}
}

void lval_println(lval* v) {
	lval_print(v);
	putchar('\n');
}

/*
   lval eval_op(lval x, char* op, lval y) {
   if (x.type == LVAL_ERR){
   return x;
   }
   if (x.type == LVAL_ERR) {
   return y;
   }
   if (strcmp(op, "+") == 0) {return lval_num(x.num+y.num);}
   if (strcmp(op, "-") == 0) {return lval_num(x.num-y.num);}
   if (strcmp(op, "*") == 0) {return lval_num(x.num*y.num);}
   if (strcmp(op, "/") == 0) {
   return y.num == 0
   ? lval_err(LERR_DIV_ZERO)
   : lval_num(x.num / y.num);
   }
   return lval_err(LERR_BAD_OP);
   }

   long eval_number_of_leaves(mpc_ast_t* t) 
   {
   int total = 0;
   if (strstr(t->tag, "number")) {
   total = 1;
   return total;
   }
   total += 1;
// eval(expr)
total += eval_number_of_leaves(t->children[2]);
int i = 3;
while(strstr(t->children[i]->tag, "expr")) {
total += eval_number_of_leaves(t->children[i]);
i++;
}
total += 1;
return total;
}
 */
#define LASSERT(args, cond, err) \
	if (!(cond)){lval_del(args); return lval_err(err);}
lval* builtin(lval* a, char* func);
lval* lval_eval_sexpr(lval* v);
lval* lval_take(lval* v, int i);
lval* lval_pop(lval* v, int i);
lval* builtin_op(lval* a , char* op);

lval* lval_eval(lval* v) {
	if (v->type == LVAL_SEXPR) {
		return lval_eval_sexpr(v);
	}
	return v;
}

lval* lval_eval_sexpr(lval* v) {
	for (int i = 0; i < v-> count; i++) {
		v->cell[i] = lval_eval(v->cell[i]);
	}

	for(int i = 0; i < v->count; i++ ) {
		if (v->cell[i] -> type == LVAL_ERR) {
			return lval_take(v,i);
		}
	}

	if (v->count == 0) {return v;}

	if (v->count == 1) {
		// why
		return lval_take(v, 0);
	}

	lval* f = lval_pop(v, 0);
	if(f->type != LVAL_SYM) {
		lval_del(f);
		lval_del(v);
		return lval_err("S-expression Does not start with symbol!");
	}

	lval* result = builtin(v, f->sym);
	lval_del(f);
	return result;
}

lval* lval_pop(lval* v, int i) {
	lval* x = v->cell[i];

	memmove(&v->cell[i], &v->cell[i+1],
			sizeof(lval*) * (v->count-i-1));
	v->count--;

	v->cell = realloc(v->cell, sizeof(lval*)*v->count);
	return x;
}

lval* lval_take(lval* v, int i) {
	lval* x = lval_pop(v, i);
	lval_del(v);
	return x;
}

/*
   lval eval(mpc_ast_t* t) 
   {
// the branch can only be number or rules
if (strstr(t->tag, "number")) {
errno = 0;
long x = strtol(t->contents, NULL, 10);
//return atoi(t->contents);
//in case we give a number greater than long type
return errno != ERANGE ? lval_num(x): lval_err(LERR_BAD_NUM);
}

// t must be rules
// t's second child is operator
// ( op  expr+ )
char* op = t-> children[1] -> contents;

// eval(expr);
lval x = eval(t->children[2]);

int i = 3;

while(strstr(t->children[i]->tag, "expr")) {
x = eval_op(x, op, eval(t->children[i]));
i++;
}
return x;
}
 */

lval* builtin_head(lval* a) {
	/*
	   if (a->count != 1) {
	   lval_del(a);
	// head {1 2 3 4 } {1 2 3}
	return lval_err("Function 'head' passed too many arguments!");
	}

	if (a->cell[0] ->type != LVAL_QEXPR) {
	lval_del(a);
	return lval_err("Function 'head' passed incorrect types!");
	}

	if (a->cell[0] ->count == 0) {
	lval_del(a);
	return lval_err("Function 'head' passed {}");
	}
	 */
	LASSERT(a, a->count == 1, "Function 'head' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect types!");
	LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}");


	lval* v = lval_take(a, 0);

	// delete all elements that are not head and return
	while(v->count > 1) {
		lval_del(lval_pop(v,1));
	}
	return v;
}

lval* builtin_tail(lval* a) {
	/*
	   if (a->count != 1) {
	   lval_del(a);
	// head {1 2 3 4 } {1 2 3}
	return lval_err("Function 'head' passed too many arguments!");
	}

	if (a->cell[0] ->type != LVAL_QEXPR) {
	lval_del(a);
	return lval_err("Function 'head' passed incorrect types!");
	}

	if (a->cell[0] ->count == 0) {
	lval_del(a);
	return lval_err("Function 'head' passed {}");
	}
	 */
	LASSERT(a, a->count == 1, "Function 'tail' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect types!");
	LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}");

	lval* v  = lval_take(a, 0);
	// delete first element and return
	lval_del(lval_pop(v, 0));

	return v;
}

// convert s-expression to q-expression
lval* builtin_list(lval* a) {
	a->type = LVAL_QEXPR;
	return a;
}

lval* lval_join(lval* x, lval* y) {
	while(y->count) {
		x = lval_add(x, lval_pop(y, 0));
	}
	lval_del(y);
	return x;
}

lval* builtin_join(lval* a) {
	for (int i = 0; i < a-> count; i++) {
		LASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type.");
	}

	lval* x = lval_pop(a, 0);

	while(a -> count) {
		x = lval_join(x, lval_pop(a, 0));
	}
	lval_del(a);
	return x;
}

lval* builtin_eval(lval* a) {
	LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");
	LASSERT(a, a->cell[0] -> type== LVAL_QEXPR, "Function 'eval' passed incorrect type!");
	lval* x = lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return lval_eval(x);
}

lval* builtin_op(lval* a , char* op) {
	for(int i = 0; i < a -> count; i++ ) {
		if (a->cell[i]->type != LVAL_NUM) {
			lval_del(a);
			return lval_err("Cannot operator on non-number!");
		}
	}

	lval* x = lval_pop(a, 0);

	if ((strcmp(op, "-")==0) && a->count == 0) {
		x->num = -x->num;
	}

	while(a->count> 0) {
		lval* y = lval_pop(a, 0);

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
				x = lval_err("Division By Zero!");
				break;
			}
			x->num /= y->num;
		}
	}

	lval_del(a);
	return x;
}

lval* builtin(lval* a, char* func) {
	puts("enter builtin func");
	if(strcmp("list", func) == 0) {
		return builtin_list(a);
	}
	if(strcmp("head", func) == 0) {
		puts("enter here");
		return builtin_head(a);
	}
	if(strcmp("join", func) == 0) {
		return builtin_join(a);
	}
	if(strcmp("eval", func) == 0) {
		return builtin_eval(a);
	}
	if(strcmp("tail", func) == 0) {
		puts("enter tail");
		return builtin_tail(a);
	}
	if(strcmp("+-*/", func) == 0) {
		return builtin_op(a, func);
	}

	// why should we delete it again ?
	lval_del(a);
	return lval_err("Unknown Funtion!");
}

int main(int argc, char** argv) 
{
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	// we use / to embrace regular expression
	// <abab> the rule called abab is required
	mpca_lang(MPCA_LANG_DEFAULT,
			"  							                      \
			number   : /-?[0-9]+/ ;                         \
			symbol : \"list\" | \"head\"| \"tail\" |\"join\"   \
			|\"eval\" | '+' | '-' | '*' | '/';            \
			sexpr : '(' <expr>* ')' ;           \
			qexpr : '{' <expr>* '}' ;             \
			expr     : <number>  | <symbol> | <sexpr> | <qexpr>;\
			lispy   : /^/ <expr>* /$/ ;         \
			",
			Number, Symbol, Sexpr,Qexpr, Expr, Lispy);
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	while(1) {
		char* input = readline("lispy> ");
		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			//mpc_ast_print(r.output);
			lval* x = lval_eval(lval_read(r.output));
			//lval* x = lval_read(r.output);
			//printf("%li\n", result);
			lval_println(x);
			lval_del(x);
			mpc_ast_delete(r.output);
		}else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}
	mpc_cleanup(6, Number, Symbol, Sexpr,Qexpr,Expr, Lispy);
	return 0;
}
