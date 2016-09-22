/*
	VPKMirror
	Copyright (C) 2016, SMOKE

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "main.h"
#include "init.h"
#include "net.h"
#include "sqlite3.h"

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

int main() {
	psvDebugScreenInit();
	initSceAppUtil();

	printf("VPK Mirror direct installer v1.1 created by SMOKE & haxxey\n\n");

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
		printf("This should now open the browser.\nFrom there you can click a link and it will return here.\n");
		sceKernelDelayThread(3 * 1000 * 1000); // 3 seconds
		sceAppMgrLaunchAppByUri(0xFFFFF, "http://vpkmirror.com");
		sceKernelDelayThread(10000);
		sceAppMgrLaunchAppByUri(0xFFFFF, "http://vpkmirror.com");
		sceKernelExitProcess(0);
	}

	// argument recieved, init everything else
	netInit();
	httpInit();

	// get the part of the argument that we need
	char *file_name;
	file_name = strchr(AppParam, '?')+1;

	// create url based off the file name
	char *file_url = malloc(512 * sizeof(char));
	snprintf(file_url, 512, "http://vpkmirror.com/files/vpk/%s", file_name);

	// download file
	printf("Downloading %s...\n", file_name);
	char *file_path = malloc(512 * sizeof(char));
	snprintf(file_path, 512, "ux0:/temp/%s", file_name);
	download(file_url, file_path);

	// install vpk
	printf("Installing %s...\n", file_name);
	installPackage(file_path);

	// cleanup
	sceIoRemove(file_path);

	printf("\nAuto exiting in 5 seconds...\n");

	sceKernelDelayThread(5 * 1000 * 1000);

	httpTerm();
	netTerm();
	finishSceAppUtil();

	sceKernelExitProcess(0);
}
