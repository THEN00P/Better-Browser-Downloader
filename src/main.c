#include "main.h"
#include "init.h"
#include "net.h"
#include "sqlite3.h"

#define printf psvDebugScreenPrintf

#define APP_DB "ur0:/shell/db/app.db"

int sceAppMgrGetAppParam(char *param);

volatile int dialog_step = DIALOG_STEP_NONE;
static int is_in_archive = 0;

int isInArchive() {
	return is_in_archive;
}

void sql_simple_exec(sqlite3 *db, const char *sql) {
	char *error = NULL;
	int ret = 0;
	ret = sqlite3_exec(db, sql, NULL, NULL, &error);
	if (error) {
		//printf("Failed to execute %s: %s\n", sql, error);
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
		//printf("Failed to open the database: %s\n", sqlite3_errmsg(db));
	}

	sql_simple_exec(db, "DELETE FROM tbl_uri WHERE titleId='VPKMIRROR'");
	sql_simple_exec(db, "INSERT INTO tbl_uri VALUES ('VPKMIRROR', '1', 'vpk', NULL)");

	sqlite3_close(db);
	db = NULL;

	return;
}

int extract_vpk(char *path) {
	//focusOnFilename(path);
}

int main(int argc, char *argv[]) {
	/* grab app param from our custom uri
	   full app param looks like:
	   type=LAUNCH_APP_BY_URI&uri=vpk:install?test
	*/
	char AppParam[1024];
	sceAppMgrGetAppParam(AppParam);

	// checks if argument is present, if not it does the uri mod and opens the website
	int arg_len = strlen(AppParam);
	if (arg_len == 0) {
		//printf("Installing uri mod...\n");
		do_uri_mod();
		sceAppMgrLaunchAppByUri(0x20000, "http://vpkmirror.com");
		sceKernelExitProcess(0);
	}

	// argument recieved, init everything
	initVita2dLib();
	initSceAppUtil();
	netInit();
	httpInit();

	// get the part of the argument that we need
	char *vpk_name;
	vpk_name = strchr(AppParam, '?')+1;

	// create url based off the vpk name
	char *vpk_url = malloc(1024 * sizeof(char));
	snprintf(vpk_url, 1024, "http://vpkmirror.com/files/vpk/%s", vpk_name);

	// check if directory exists, create directory if necessary
	int dir_chk = sceIoDopen("ux0:/data/VPKMirror");
	if (dir_chk != 0) {
		sceIoMkdir("ux0:/data/VPKMirror", 0777);
	}

	// download vpk
	//printf("Downloading vpk..\n");
	char *vpk_path = malloc(1024 * sizeof(char));
	snprintf(vpk_path, 1024, "ux0:/data/VPKMirror/%s", vpk_name);
	//download(vpk_url, vpk_path);

	while (1) {
		startDrawing();
		initMessageDialog(SCE_MSG_DIALOG_BUTTON_TYPE_OK, "test message!");
		isMessageDialogRunning();
		updateMessageDialog();
		vita2d_common_dialog_update();
		endDrawing();
	}

	//sceKernelDelayThread(3 * 1000 * 1000);

	httpTerm();
	netTerm();
	finishSceAppUtil();
	finishVita2dLib();

	sceKernelExitProcess(0);
}
