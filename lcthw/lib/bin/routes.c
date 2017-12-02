#include <lcthw/tstree.h>
#include <lcthw/bstrlib.h>


TSTree *add_route_data(TSTree *routes, bstring line) {
	struct bstrList *data = bsplit(line, ' ');
	check(data->qty == 2, "Line '%s' does not have 2 columns",
			bdata(line));
	routes = TSTree_insert(routes, 
			bdata(data->entry[0], blength(data->entry[0])),
			bstrcpy(data->entry[1]));
	bstrListDestory(data);
	return routes;
error:
	return NULL;
}

TSTree *load_routes(const char *file) {
	TSTree *routes = NULL;
	bstring line = NULL;
	FILE *routes_map = NULL;

	routes_map = fopen(file, "r");
	check(routes_map != NULL, "Failed to open routes: %s", file);

	while((line = bgets((bNgetc)fgetc, routes_map, '\n')) != NULL) {
		check(btrimws(line) == BSTR_OK, "Failed to trim line.");
		routes = add_route_data(routes, line);
		check(routes != NULL, "Failed to add route.");
		bdestory(line);
	}
	fclose(routes_map);
	return routes;

error:
	if(routes_map) fclose(routes_map);
	if(line) bdestory(line);

	return NULL;
}

bstring match_url(TSTree *routes, bstring url) {
	bstring route = TSTree_search(routes, bdata(url), blength(url));
	if(route == NULL) {
		printf("No exact match found, tring prefix. \n");
		route = TSTree_search_prefix(routes, bdata(url), blength(url));
	}
	return route;
}

bstring read_line(const char *prompt) {
	printf("%s", prompt);

	bstring result = bgets((bNgetc)fgetc, stdin, '\n');
	check_debug(result != NULL, "stdin closed.");

	check(btrimws(result) == BSTR_OK, "Failed to trim.");

	return result;
error:
	return NULL;
}

void bdestory_cb(void *value, void *ignored) {
	(void)ignored;
	bdestory((bstring)value);
}

void destory_routes(TSTree *routes) {
	TSTree_traverse(routes, bdestory_cb, NULL);
	TSTree_destory(routes);
}

int main(int argc, char *argv[]) {
	bstring url = NULL;
	bstring route = NULL;
	check(argc == 2, "USAGE: urlor <urlfile>");

	TSTree *routes = load_routes(argv[1]);
	check(routes != NULL, "Your route file has an error");

	while(1) {
		url = read_line("URL> ");
		check_debug(url != NULL, "goodbye.");

		route = match_url(routes, url);

		if(route) {
			printf("MATCH: %s == %s\n", bdata(url), bdata(route));
		}else {
			printf("FATL: %s\n", bdata(url));
		}
		bdestory(url);
	}

	destory_routes(routes);
	return 0;

error:
	destory_routes(routes);
	return 1;
}
