#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAXDATA 100
#define MAXRAW 100

void die(const char *msg) 
{
	if(errno) {
		perror(msg);
	}else {
		printf("Error: %s\n", msg);
	}
	
	exit(1);
}

struct Address {
	int id;
	int set;
	char name[MAXDATA];
	char email[MAXDATA];
};

struct Database{
	struct Address rows[MAXRAW];
} ;

struct Connection {
	struct Database *db;
	FILE *file;
};

void Address_print(struct Address *addr) {
	printf("\t %d    %s     %s.\n", addr->id, addr->name, addr->email);
	puts("\n");
}
void Database_load(struct Connection *conn) {
	int rc = fread(conn->db, sizeof(struct Database), 1, conn->file);
	if(rc != 1) {
		die("Failed to load database.");
	}
}

struct Connection *Database_open(const char *filename, char mode) {
	struct Connection *conn = malloc(sizeof(struct Connection));
	conn->db = malloc(sizeof(struct Database));

	if (mode == 'c') {
		conn->file = fopen(filename, "w");
	}else {
		conn->file = fopen(filename, "r+");

		if(conn->file) {
			Database_load(conn);
		}
	}

	return conn;
}

void Database_create(struct Connection *conn) {
	int i = 0;
	for(i = 0; i < MAXRAW; i++) {
		struct Address ad = {.id = i, .set = 0};
		conn->db->rows[i] = ad;
	}
}

void Database_write(struct Connection *conn) {
	rewind(conn->file);

	int rc = fwrite(conn->db, sizeof(struct Database), 1, conn->file);
	if(rc != 1) die("Faile to write database.");

	rc = fflush(conn->file);
	if(rc == -1)  die("cannno flush database."); 
}

void Database_close(struct Connection *conn) {
	if(conn) {
		if(conn->file) fclose(conn->file);
		if(conn->db) free(conn->db);
		free(conn);
	}
}

void Database_get(int id, struct Connection *conn) {
	struct Address *addr = &conn->db->rows[id];
	if(addr->set) {
		Address_print(addr);
	} else {
		die("Not set yet.");
	}
}

void Database_set(int id, char *name, char *email, struct Connection *conn) {
	struct Address *addr = &conn->db->rows[id];
	if(addr->set) die("Already set, delete it first!");

	addr->set = 1;
	char *res = strncpy(addr->name, name, MAXDATA);
	if(!res) die("Name copy failed");

	res = strncpy(addr->email, email, MAXDATA);
	if(!res) die("Email copy failed");
}

int main(int argc, char *argv[])
{
	if(argc < 3) {
		die("few argument1");
	}

	char *filename = argv[1];
	char action = argv[2][0];

	struct Connection *conn = Database_open(filename, action);

	int id;
	switch(action) {
		case 'c':
			Database_create(conn);
			Database_write(conn);
			break;
		case 's':
			if(argc < 6) {
				die("few argument1");
			}
			id = atoi(argv[3]);
			Database_set(id, argv[4], argv[5], conn);
			Database_write(conn);
			break;
		case 'g':
			if(argc < 4) {
				die("few argument1");
			}
			id = atoi(argv[3]);
			Database_get(id,conn);
			
			break;
		default:
			die("Unsupported mode");
	}

	Database_close(conn);
	return 0;
}
