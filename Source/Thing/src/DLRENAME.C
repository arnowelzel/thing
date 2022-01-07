/**
 * Thing
 * Copyright (C) 1994-2012 Arno Welzel, Thomas Binder
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * @copyright  Arno Welzel, Thomas Binder 1994-2012
 * @author     Arno Welzel, Thomas Binder
 * @license    LGPL
 */

/**=========================================================================
 DLRENAME.C

 Thing
 Umbenenne von Dateien oder Ordner in Grož- bzw. Kleinbuchstaben
 =========================================================================*/

#include "..\include\globdef.h"
#include "..\include\types.h"
#include "rsrc\thing.h"
#include "rsrc\thgtxt.h"

/**-------------------------------------------------------------------------
 -------------------------------------------------------------------------*/

short dl_rename(short pitem) {
	short i, j;
	char ipath[MAX_PLEN], oldpath[MAX_PLEN], npath[MAX_PLEN], iname[MAX_FLEN];
	short ok;
	short whandle;
	W_PATH *wpath;
	char *readbuf;
	FILESYS fs;

	dcopy = pmalloc(sizeof(DCOPY));
	if (!dcopy) {
		frm_alert(1, rs_frstr[ALNOMEM], altitle, conf.wdial, 0L);
		return 0;
	}
	for (i = 0; i < 32; i++)
		dcopy->dlst[i] = 0;

	/* Objekte in einem Fenster ? */
	if (desk.sel.win) {
		/* Jo ... */
		switch (desk.sel.win->class) {
		case WCPATH:
			lbuf = pmalloc(MAX_KBDLEN);
			if (lbuf) {
				ok = sel2buf(lbuf, iname, ipath, (short)MAX_KBDLEN);
				if (ok)
				{
					readbuf = lbuf;
					while (get_buf_entry(readbuf, ipath, &readbuf) && ok) {
						j = (short) strlen(ipath);
						full2comp(ipath, npath, iname);
						if (strlen(iname) > 0)
						{
							strcpy (oldpath, ipath);
							if (pitem == POPBIGLETTER )
							{
								i = 0;
								while (iname[i]) {
									iname[i] = nkc_toupper(iname[i]);
									i++;
								}
							}
							else
							{
								i = 0;
								while (iname[i]) {
									iname[i] = nkc_tolower(iname[i]);
									i++;
								}
							}
							strcat (npath, iname);
							Frename(0, oldpath, npath);
							/* Laufwerks-Update vormerken */
							fsinfo(npath, &fs);
							i = fs.biosdev;
							if ((i >= 0) && (i <= 31))
								dcopy->dlst[i] = 1;
						}
					}
					/* Verzeichnisse aktualisieren */
					for (i = 0; i < MAX_PWIN; i++) {
						if (glob.win[i].state & WSOPEN) {
							wpath = (W_PATH *) glob.win[i].user;
							if (wpath->rel == -1) {
								/* Nicht mehr benoetigte Verzeichnisfenster schliessen */
								tb.topwin = &glob.win[i];
								dl_closewin();
							} else {
								/* Veraenderte Laufwerke aktualisieren */
								j = wpath->filesys.biosdev;
								if (j >= 0 && j <= 31) {
									if (dcopy->dlst[j]) {
										wpath_update(&glob.win[i]);
										win_redraw(&glob.win[i], tb.desk.g_x, tb.desk.g_y, tb.desk.g_w, tb.desk.g_h);
										win_slide(&glob.win[i], S_INIT, 0, 0);
									}
								}
							}
						}
					}
					/* Aktives Fenster neu ermitteln - koennte veraendert sein! */
					get_twin(&whandle);
					tb.topwin = win_getwinfo(whandle);
					win_newtop(tb.topwin);
				}
				pfree(lbuf);
			} else {
				frm_alert(1, rs_frstr[ALNOMEM], altitle, conf.wdial, 0L);
			}
			break;
		}
	}

	pfree (dcopy);
	dcopy = 0L;
	return (1);
}
