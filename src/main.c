#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <psp2/io/fcntl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/stat.h>
#include <psp2/sysmodule.h>

#include "sqlite3.h"
#include "graphics.h"
#include "net.h"

#define printf psvDebugScreenPrintf

#define APP_DB "ur0:/shell/db/app.db"

void sql_simple_exec(sqlite3 *db, const char *sql) {
	char *error = NULL;
	int ret = 0;
	ret = sqlite3_exec(db, sql, NULL, NULL, &error);
	if (error) {
		printf("Failed to execute %s: %s\n", sql, error);
		sqlite3_free(error);
		goto fail;
	}
	return;
fail:
	sqlite3_close(db);
}

void do_uri_mod() {
	int ret;

	sqlite3 *db;
	ret = sqlite3_open(APP_DB, &db);
	if (ret) {
		printf("Failed to open the database: %s\n", sqlite3_errmsg(db));
	}

	sql_simple_exec(db, "DELETE FROM tbl_uri WHERE titleId='VPKMIRROR'");
	sql_simple_exec(db, "INSERT INTO tbl_uri VALUES ('VPKMIRROR', '1', 'vpk', NULL)");

	sqlite3_close(db);
	db = NULL;

	return;
}

int main(int argc, char *argv[]) {
	psvDebugScreenInit();
	netInit();
	httpInit();

	/*
	<_SMOKE_> so, I'm trying to pass an argument via a custom uri to my homebrew, but it seems arguments arent supported in the vitasdk? Any suggestions?
	<frangarcj> _SMOKE_, how?
	<endrift> _SMOKE_: I've been trying to figure it out myself, to no avail at the moment.
	<frangarcj> I mean the custom url
	<frangarcj> newlib does not support params right now i think
	<endrift> newlib absolutely does not
	<endrift> it intentionally passes in 0, ""
	*/

	printf("argc: %d argv: %s\n", argc, argv[0]);

	sceKernelDelayThread(5 * 1000 * 1000);

	httpTerm();
	netTerm();

	sceKernelExitProcess(0);
}
