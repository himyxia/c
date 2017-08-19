#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <editline/history.h>
#include "../mpc/mpc.h"


/*The special symbols used to define the rules on the right hand side work as follows.
 * 
 * "ab" abThe string ab is required.
 * 'a' The character a is required.
 * 'a' 'b'    First 'a' is required, then 'b' is required.
 * 'a' | 'b'  requiredEither 'a' is required, or 'b' is required.
 * 'a'* Zero or more 'a' are required.
 * 'a'+ One or more 'a' are required.
 * <abba> The rule called abba is required.
 * */

/*
 * In an mpc grammar we write regular expressions by putting them between forward slashes /. Using the above guide our Number rule can be expressed as a regular expression using the string /-?[0-9]+/.
 *
 * */
/*
 * abstract syntax tree
 * branch: contents are empty
 * leave: operator or number
 */
/*
typedef struct mpc_ast_t {
	char* tag;
	char* contents;
	mpc_state_t state;
	int children_num;
	struct mpc_ast_t** children;
} mpc_ast_t;
*/
/*
lispy> + (- 1 2) (* 2 4)
> 
operator|char:1:1 '+'
expr|> 
char:1:3 '('
operator|char:1:4 '-'
expr|number|regex:1:6 '1'
	//tag: expr|number|regex
	// a list of all the rules used to parse the particular item
expr|number|regex:1:8 '2'
char:1:9 ')'
expr|> 
char:1:11 '('
operator|char:1:12 '*'
expr|number|regex:1:14 '2'
expr|number|regex:1:16 '4'
char:1:17 ')'
 */
typedef struct lval {
	int type;
	long num;
	char* operator;
	char* content;
	int count;
	struct lval** children;
}lval;

enum {LVAL_NUMBER, LVAL_OPERATOR, LVAL_EXPR, LVAL_CHAR,LVAL_LISPY};

lval* lval_read(mpc_ast_t* v) {
	errno = 0;
	lval* l = malloc(sizeof(lval));
	//puts("enter lval_read");
	//printf("v->tag: %s\n", v->tag);
	if (strstr(v->tag, "number")) {
		//puts("enter lval_read number");
		//printf("enter lval_read number: %s \n",v->contents);
		l->type = LVAL_NUMBER;
		long x = strtol(v->contents,NULL, 10);
		if (errno != ERANGE) {
			l->num = x;
		}else {
			printf("error happend: when parsing number !\n");
		}
		return l;
	}

	if (strstr(v->tag, "operator")) {
	//	puts("enter lval_read operator");
		l->type = LVAL_OPERATOR;
		l->operator = malloc(strlen(v->contents) + 1);
		strcpy(l->operator, v->contents);
		return l;
	}

	lval* x = NULL;

	// means it is a expr
	if (strstr(v->tag, ">")) {
		x = malloc(sizeof(lval));
		x -> count = 0;
		x->type = LVAL_EXPR;
		x-> children = NULL;
	}

	for (int i = 0; i < v->children_num; i++ ) {
		if (strcmp(v->children[i]->contents, "(") == 0) {
			continue;
		}
		if (strcmp(v->children[i]->contents, ")") == 0) {
			continue;
		}
		x->count++;
		x->children = realloc(x->children,sizeof(lval*)*x->count);
		x->children[i] = lval_read(v->children[i]);
	}
	return x;
}

void lval_print(lval* l) {
	//puts("enter lval_print");
	switch(l->type) {
		case LVAL_NUMBER:
			printf("%li ", l->num);
			break;
		case  LVAL_OPERATOR:
			printf("%s ", l->operator);
			break;
		case LVAL_EXPR:
			putchar('(');
			for(int i = 0; i < l->count; i++) {
				lval_print(l->children[i]);
				if (i != l->count -1 ) {
					putchar(' ');
				}
			}
			putchar(')');
			break;
	}
}

void lval_del(lval* l) {
	switch (l->type) {
		case LVAL_NUMBER: break;
		case LVAL_OPERATOR:
			free(l->operator);
			break;
		case LVAL_EXPR:
			for(int i = 0; i < l->count; i++) {
				lval_del(l->children[i]);
			}
			free(l->children);
			break;
	}
	free(l);
}

lval* lval_pop(lval* l, int i) {
	lval* x = l -> children[i];
	memmove(&l->children[i], &l->children[i+1], sizeof(lval*) * (l->count-1));
	l->count--;
	l->children = realloc(l->children, sizeof(lval*) * l->count);
	return x;
}

void lval_println(lval* t);

lval* lval_eval(lval* l) {
	//puts("enter lval_eval");
	if (l->type == LVAL_EXPR) {
		for (int i = 0; i < l->count; i++) {
			l->children[i] = lval_eval(l->children[i]);
		}
		//puts("before pop");
		lval* f = lval_pop(l, 0);

		char* func = f->operator;

		//puts("after pop");

		free(f);

		//puts("want to see how much");
		lval* x = lval_pop(l,0);
		while (l->count > 0) {
			lval* y = lval_pop(l,0);
			if (strcmp(func, "+") == 0) {
				//puts("enter add");
				x->num += y->num;
			}
			if (strcmp(func, "-") == 0) {
				//puts("enter sub");
				x->num -= y->num;
			}
			if (strcmp(func, "*") == 0) {
			//	puts("enter mul");
				x->num *= y->num;
			}
			if (strcmp(func, "/") == 0) {
			//	puts("enter div");
				x->num /= y->num;
			}
		}
		x->type = LVAL_NUMBER;
		return x;
	}
	return l;
}

void lval_println(lval* t) {
	//puts("enter lval_println");
	lval_print(t);
	putchar('\n');
}

int main(int argc, char** argv) {
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	// use / / to encolse the regular expr
	mpca_lang(MPCA_LANG_DEFAULT,
			"   \
			number : /-?[0-9]+/; \
			operator : '+' | '-' | '*' | '/'; \
			expr : <number> | '(' <operator> <expr>+ ')'; \
			lispy : /^/ <operator> <expr>+ /$/; \
			",
			Number, Operator, Expr, Lispy);
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
			mpc_ast_delete(r.output);
		}else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);
	}
	mpc_cleanup(4,Number, Operator, Expr, Lispy);
	return 0;
} 

//TODO write a regex matching strings of consecutive a and b such as ababa or aba
//TODO write a regular expression matching pit, pot and respite but not peat, spit, or part.
//TODO change the grammar to recognize decimal numbers such as 0.01, 5.21, or 10.2.
//TODO change the grammar to make the operators written conventionally, between two expressions.
//TODO use the grammar from the previous chapter to parse Doge. You must add start and end of input
