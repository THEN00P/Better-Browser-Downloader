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
#include "psp2/display.h" 
#include "psp2/io/stat.h" 
#include "psp2/ctrl.h"
#include "vita2d.h"

#define APP_DB "ur0:/shell/db/app.db"

extern unsigned int basicfont_size;
extern unsigned char basicfont[];

SceCtrlData     pad;
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

	sql_simple_exec(db, "DELETE FROM tbl_uri WHERE titleId='VPKSOURCE'");
	sql_simple_exec(db, "INSERT INTO tbl_uri VALUES ('VPKSOURCE', '1', 'get', NULL)");

	sqlite3_close(db);
	db = NULL;

	return;
}

int main() {
	psvDebugScreenInit();
	initSceAppUtil();
	
	printf("Better Browser Dowloader v0.95\nCreated by SMOKE & haxxey. Modified by THEN00P.\n\n");

	/* grab app param from our custom uri
	   full app param looks like:
	   type=LAUNCH_APP_BY_URI&uri=get:?test
	*/
	char AppParam[1024];
	sceAppMgrGetAppParam(AppParam);

	// checks if argument is present, if not it does the uri mod and opens the website
	int arg_len = strlen(AppParam);
	if (arg_len == 0) {
		do_uri_mod();
		printf("This should now open the browser.\nFrom there you can click a link and it will return here.\n");
		sceKernelDelayThread(3 * 1000 * 1000); // 3 seconds
		sceAppMgrLaunchAppByUri(0xFFFFF, "http://vpksource.com");
		sceKernelDelayThread(10000);
		sceAppMgrLaunchAppByUri(0xFFFFF, "http://vpksource.com");
		sceKernelExitProcess(0);
	}

	// argument recieved, init everything else
	netInit();
	httpInit();

	// get the part of the argument that we need
	char *file_name_get;
	file_name_get = strchr(AppParam, '?')+1;
	char *file_name;
	file_name = strchr(AppParam, '/')+1;
	while(strchr(file_name, '/') != NULL ){
		file_name = strchr(file_name, '/')+1;
	}
	char *file_type;
	file_type = strchr(file_name, '.')+1;
	while(strchr(file_type, '.') != NULL ){
		file_type = strchr(file_type, '.')+1;
	}
	
	// create url based off the file name
	char *file_url = malloc(512 * sizeof(char));
	snprintf(file_url, 512, "%s", file_name_get);

	// download file
	if(strcmp(file_type, "vpk")==0){
		print_color("VPK Files can harm your device.",RED);
		printf("\n\nPress X to install %s.\nPress O to cancel.\n\n", file_name);
	}
	else if(strcmp(file_type, "suprx")==0){
		print_color("Plugins can harm your device.",RED);
		printf("\n\nPress X to install %s.\nPress O to cancel.\n\n", file_name);
	}
	else {
		char *file_path = malloc(512 * sizeof(char));
		int ret;
		ret = doesDirExist("ux0:download/");
		if ( ret == 1 ) {
			printf("Downloading %s to ux0:/download/... ", file_name);
			snprintf(file_path, 512, "ux0:/download/%s", file_name);
			download(file_url, file_path);
			print_color("DONE\n\n", GREEN);
			sceKernelDelayThread(1000000); // Delay for a second
			sceAppMgrLaunchAppByUri(0x20000, "psgm:play?titleid=VITASHELL");
			goto end;
		}
		else {
			printf("Creating folder ux0:/download/... ");
			ret = sceIoMkdir("ux0:download/", 0777);
			if ( ret == 0 )	{
				print_color("DONE\n\n", GREEN);
				printf("Downloading %s to ux0:/download/... ", file_name);
				snprintf(file_path, 512, "ux0:/download/%s", file_name);
				download(file_url, file_path);
				print_color("DONE\n", GREEN);
				sceKernelDelayThread(1000000); // Delay for a second
				sceAppMgrLaunchAppByUri(0x20000, "psgm:play?titleid=VITASHELL");
				goto end;
			}
			else {
				print_color("ERROR\n", RED);
				goto end;
			}
		}
	}
	
	while (1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons & SCE_CTRL_CROSS) {
			
			char *file_path = malloc(512 * sizeof(char));
			
			if(strcmp(file_type, "vpk")==0){
				printf("Downloading %s... ", file_name);
				snprintf(file_path, 512, "ux0:/temp/%s", file_name);
				download(file_url, file_path);
				print_color("DONE\n", GREEN);
				
				// install vpk
				printf("Installing %s... ", file_name);
				installPackage(file_path);

				// cleanup
				sceIoRemove(file_path);
			}
			
			else if(strcmp(file_type, "suprx")==0){
				int ret;
				ret = doesDirExist("ux0:plugins/");
				if ( ret == 1 ) {
					printf("Downloading %s to ux0:/plugins/... ", file_name);
					snprintf(file_path, 512, "ux0:/plugins/%s", file_name);
					download(file_url, file_path);
					print_color("DONE\n\n", GREEN);
				}
				else {
					printf("Creating folder ux0:/plugins/... ");
					ret = sceIoMkdir("ux0:plugins/", 0777);
					if ( ret == 0 )	{
						print_color("DONE\n\n", GREEN);
						printf("Downloading %s to ux0:/plugins/... ", file_name);
						snprintf(file_path, 512, "ux0:/plugins/%s", file_name);
						download(file_url, file_path);
						print_color("DONE\n", GREEN);
					}
					else{
						print_color("ERROR\n", RED);
					}
				}
			}
			netTerm();
			finishSceAppUtil();
			sceKernelExitProcess(0);
		}
		
		
		if (pad.buttons & SCE_CTRL_CIRCLE) break;
	}
	end:
	httpTerm();
	netTerm();
	finishSceAppUtil();
	sceKernelExitProcess(0);
}
