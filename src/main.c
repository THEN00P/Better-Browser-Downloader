#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <psp2/appmgr.h>
#include <psp2/apputil.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/sysmodule.h>

#include "sqlite3.h"
#include "graphics.h"
#include "net.h"

#define printf psvDebugScreenPrintf

#define APP_DB "ur0:/shell/db/app.db"

int sceAppMgrGetAppParam(char *param);

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

void do_uri_mod(void) {
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

	/* grab app param from our custom uri
	   full app param looks like:
	   type=LAUNCH_APP_BY_URI&uri=vpk:install?test
	*/
	char AppParam[1024];
	sceAppMgrGetAppParam(AppParam);

	// checks if argument is present, if not it does the uri mod and opens the website
	int arg_len = strlen(AppParam);
	if (arg_len == 0) {
		do_uri_mod();
		sceKernelExitProcess(0);
		// this makes hard crashes occasionally, no idea why. Commented out for now
		//sceAppMgrLaunchAppByUri(0x20000, "http://vpkmirror.com");
	}

	// get the part of the argument that we need
	char *vpk_name;
	vpk_name = strchr(AppParam, '?')+1;

	// create url based off the vpk name
	char *vpk_url = malloc(1024 * sizeof(char));
	snprintf(vpk_url, 1024, "http://vpkmirror.com/files/vpk/%s", vpk_name);

	// check if ptmp directory exists, create directory if necessary
	int dir_chk = sceIoDopen("ux0:/ptmp");
	if (dir_chk != 0) {
		sceIoMkdir("ux0:/ptmp", 0777);
	}

	//download vpk
	download(vpk_url, "ux0:/ptmp/brew.vpk");

	sceKernelDelayThread(3 * 1000 * 1000);

	httpTerm();
	netTerm();

	sceKernelExitProcess(0);
}
