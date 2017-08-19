#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <editline/history.h>
#include "../mpc/mpc.h"

typedef struct lval{
	long num;
	int type;
	int err;
} lval;

enum {LVAL_NUM, LVAL_ERR};

enum {ERR_DIVIDE_BY_ZERO, ERR_BAD_NUM, ERR_BAD_OP};

lval lval_num(long num) {
	lval v;
	v.type = LVAL_NUM;
	v.num = num;
	return v;
}

lval lval_err(int err) {
	lval v;
	v.type = LVAL_ERR;
	v.err = err;
	return v;
}

lval builtin_op(lval x, char* op, lval y) {
	if(x.type == LVAL_ERR) {
		return x;
	}
	if(y.type == LVAL_ERR) {
		return y;
	}

	lval result;
	if(strcmp(op, "+")==0) {
		result = lval_num(x.num + y.num);
		return result;
	}
	if(strcmp(op, "-")==0) {
		result = lval_num(x.num - y.num);
		return result;
	}
	if(strcmp(op, "*")==0) {
		result = lval_num(x.num * y.num);
		return result;
	}
	if(strcmp(op, "/")==0) {
		if(y.num == 0) {
			result = lval_err(ERR_DIVIDE_BY_ZERO);
		}else {
			result = lval_num(x.num / y.num);
		}
		return result;
	}
	return lval_err(ERR_BAD_OP);
}

void lval_print(lval a) {
	switch(a.type) {
		case LVAL_ERR:
			if(a.err == ERR_BAD_OP){
				printf("Error: bad operator");
			}
			if(a.err == ERR_BAD_NUM){
				printf("Error: bad number");
			}
			if(a.err == ERR_DIVIDE_BY_ZERO){
				printf("Error: divide by zero");
			}
			break;
		case LVAL_NUM:
			printf("%li", a.num);
			break;
	}
}

void lval_println(lval a) {
	lval_print(a);
	putchar('\n');
}

lval eval(mpc_ast_t* t) {
	if(strstr(t->tag, "number")) {
		long y = atoi(t->contents);
		return lval_num(y);
	}
	char* op = t->children[1]->contents;

	lval x = eval(t->children[2]);

	int i = 3;
	while(i < t->children_num) {
		if(strstr(t->children[i]->tag, "expr")) {
			x = builtin_op(x, op, eval(t->children[i]));
		}
		i++;
	}
	return x;
}

int main(int argc, char** argv) {
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

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

		if(mpc_parse("<stdin>", input, Lispy, &r)) {
			mpc_ast_print(r.output);
			lval_println(eval(r.output));
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
