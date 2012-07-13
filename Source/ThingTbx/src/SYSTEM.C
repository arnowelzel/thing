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

#include <aes.h>
#include <tos.h>
#include <string.h>
#include "..\include\thingtbx.h"
#ifdef __MINT__
#include <sysvars.h>		/* fuer getSystemLanguage() */
#endif


/*
 * Diese Routine ermittelt die Landessprache des Desktops. Als
 * erstes wird der Systemheader ausgewertet. Wenn der Test auf
 * den _AKP-Cookie erfolgreich ist, wird dessen Einstellung ge-
 * nommen und zum Schluss wird noch die Desktopsprache von MTOS
 * ueberprueft, da sie die Ausschlaggebende waere.
 *
 *	Wert	Land
	0	USA
	1	Deutschland
	2	Frankreich
	3	England
	4	Spanien
	5	Italien
	6	Schweden
	7	franz. Schweiz
	8	deutsche Schweiz
	9	Türkei
	10	Finnland
	11	Norwegen
	12	Dänemark
	13	Saudi-Arabien
	14	Niederlande
	15	CSSR
	16	Ungarn
 *
 * @return
 */
WORD getSystemLanguage(BYTE *sysLanguageCode) {
	WORD sysLanguageId, du;
	LONG oldStack = 0, ret;
#ifdef __MINT__
	OSHEADER *syshdr;
#else
	SYSHDR *syshdr;
#endif

	if (tb.sys & SY_AGI) {
		appl_getinfo(3, &sysLanguageId, &du, &du, &du);
	} else if (getCookie('_AKP', &ret) == TRUE) {
		sysLanguageId = (WORD)(ret >> 8);
	} else {
		if (!Super((VOID *) 1l))
			oldStack = Super(0l);
#ifdef __MINT__
		syshdr = *(OSHEADER **) 0x4f2;
		syshdr = syshdr->os_beg;
		sysLanguageId = syshdr->os_conf >> 1;
#else
		syshdr = *(SYSHDR **) 0x4f2;
		syshdr = syshdr->os_base;
		sysLanguageId = syshdr->os_palmode >> 1;
#endif
		if (oldStack)
			Super((VOID *) oldStack);
	}

	switch (sysLanguageId) {
		case 1:
		case 8:
			strcpy(sysLanguageCode, "de");
			break;
		case 2:
		case 7:
			strcpy(sysLanguageCode, "fr");
			break;
		case 4:
			strcpy(sysLanguageCode, "es");
			break;
		case 5:
			strcpy(sysLanguageCode, "it");
			break;
		case 9:
			strcpy(sysLanguageCode, "tr");
			break;
		case 10:
			strcpy(sysLanguageCode, "fi");
			break;
		case 11:
			strcpy(sysLanguageCode, "no");
			break;
		case 12:
			strcpy(sysLanguageCode, "da");
			break;
		case 14:
			strcpy(sysLanguageCode, "nl");
			break;
		case 16:
			strcpy(sysLanguageCode, "hu");
			break;
		default:
			strcpy(sysLanguageCode, "en");
			break;
	}

	return (sysLanguageId);
}

#if 0
/**
 * Infos Åber ein Dateisystem ermitteln
 *
 * @param *file
 * @param line
 * @param *name
 * @param *fs
 */
#ifdef DEBUG
void fsinfo_debug(char *file, int line, char *name, FILESYS *fs) {
DEBUGLOG((0, "fsinfo() called in file %s, line %d\n", file, line));
	fs_info(name, fs);
}
#endif

void getFilesystemInfo(char *name, FILESYS *filesys) {
	XATTR xattr;
	long mode, ret;
	int namelen;
	char lpath[MAX_PLEN], *p;

DEBUGLOG((0, "fsinfo(%s)\n", name));
	/* Default-Werte, falls kein MiNT etc. vorhanden ist */
	filesys->flags = UPCASE | TOS;
	filesys->namelen = 12;
	if (*name)
		filesys->biosdev = (*name & ~32) - 'A';
	else
		filesys->biosdev = Dgetdrv();

	if (!name[0]) {
DEBUGLOG((0, "fsinfo: Name is empty, exiting\n"));
		return;
	}

	strcpy(lpath, name);
	lpath[0] = nkc_toupper(lpath[0]);
	p = strrchr(lpath, '\\');
	if (p)
		p[1] = 0;

	/* Aktuelles Verzeichnis setzen */
DEBUGLOG((0, "fsinfo: Setting dir to %s\n", lpath));
	if (set_dir(lpath)) {
DEBUGLOG((0, "fsinfo: set_dir() failed, exiting\n"));
		return;
	}

	mode = Dpathconf(".", -1);
	if (mode < 0L) {
		/* Kein Dpathconf() verfÅgbar */
		clr_drv();
DEBUGLOG((0, "fsinfo: No Dpathconf() available (%ld), exiting\n", mode));
		return;
	}
DEBUGLOG((0, "fsinfo: max. mode for Dpathconf(): %ld\n", mode));

	filesys->flags |= UNIXATTR | OWNER | STIMES | SYMLINKS;
	/* Maximale LÑnge eines Dateinamens ermitteln */
	if (mode >= 3) {
		ret = Dpathconf(".", 3);
		/* Bei Fehler 'normal' ermitteln */
		if (ret < 0L)
			ret = Dpathconf(lpath, 3);

		if (ret == 0x7fffffffL)
			namelen = MAX_FLEN - 1;
		else
			namelen = (int) ret;
		if (namelen >= MAX_FLEN)
			namelen = MAX_FLEN - 1;
		if (namelen < 12)
			namelen = 12; /* Kleiner Patch */
		filesys->namelen = namelen;
	}

	/* TOS-Modus (8+3) ermitteln */
	if (mode >= 5) {
		ret = Dpathconf(".", 5);
		/* Bei Fehler 'normal' ermitteln */
		if (ret < 0L)
			ret = Dpathconf(lpath, 5);
		if (ret >= 0L)
			switch ((int) ret) {
			case 0: /* Kein TOS-System */
			case 1:
				filesys->flags &= ~TOS;
				break;
			case 2: /* TOS (8+3) */
				filesys->flags |= TOS;
				filesys->flags &= ~UNIXATTR;
				break;
			}
	}

	/* PrÅfen, ob case-sensitiv */
	if (mode >= 6) {
		ret = Dpathconf(".", 6);
		/* Bei Fehler 'normal' ermitteln */
		if (ret < 0L)
			ret = Dpathconf(lpath, 6);
		if (ret >= 0L)
			switch ((int) ret) {
			case 0: /* case-sensitiv */
			case 2:
				filesys->flags &= ~UPCASE;
				break;
			case 1: /* nicht case-sensitiv */
				filesys->flags |= UPCASE;
				break;
			}
	}

	/* PrÅfen, ob Unix-Filemodi vorhanden sind */
	ret = -32L;
	if (mode >= 7) {
		ret = Dpathconf(".", 7);
		if (ret < 0L)
			ret = Dpathconf(lpath, 7);
DEBUGLOG((0, "fsinfo: Dpathconf(7) -> %lo (0x%lx)\n", ret, ret));
		if (ret >= 0L) {
			if ((ret & (0xfffL << 8L)) == 0)
				filesys->flags &= ~UNIXATTR;
			if ((ret & 0x01000000L) == 0L)
				filesys->flags &= ~SYMLINKS;
		}
	}

	/*
	 * Unter MagiC auch dann als nicht vorhanden ansehen, wenn das FS
	 * Dpathconf-Modus 7 nicht unterstÅtzt
	 */
	if ((ret < 0L) && (tb.sys & SY_MAGX))
		filesys->flags &= ~(UNIXATTR | OWNER | STIMES);

	if (mode >= 8) {
		ret = Dpathconf(".", 8);
		if (ret < 0L)
			ret = Dpathconf(lpath, 8);
		if (ret >= 0L) {
			/* PrÅfen, ob Fileeigner oder -gruppe unterstÅtzt wird */
			if (ret & 0x30)
				filesys->flags |= OWNER;
			else
				filesys->flags &= ~OWNER;
			/* PrÅfen, ob a- oder ctime unterstÅtzt wird */
			if (ret & 0x600)
				filesys->flags |= STIMES;
			else
				filesys->flags &= ~STIMES;
			/* PrÅfen, ob dev-Feld gÅltig und ggf. Åbernehmen */
			if (ret & 2) {
				ret = Fxattr(1, ".", &xattr);
				if (ret < 0L)
					ret = Fxattr(1, lpath, &xattr);
				if ((ret == 0L) && ((int) xattr.dev >= 0) && (xattr.dev < 32))
					filesys->biosdev = xattr.dev;
			}
		}
	}
	title_update(filesys->biosdev);
DEBUGLOG((0, "fsinfo: Result:\n"));
DEBUGLOG((0, "  upcase: %d\n", filesys->flags & UPCASE));
DEBUGLOG((0, "  namelen: %d\n", filesys->namelen));
DEBUGLOG((0, "  tos: %d\n", filesys->flags & TOS));
DEBUGLOG((0, "  unixattr: %d\n", filesys->flags & UNIXATTR));
DEBUGLOG((0, "  owner: %d\n", filesys->flags & OWNER));
DEBUGLOG((0, "  stimes: %d\n", filesys->flags & STIMES));
DEBUGLOG((0, "  biosdev: %d\n", filesys->biosdev));
	clr_drv();
}

/**
 * Verzeichniss des aktuellen LW auf '\' setzen, und als aktuelles
 * LW/Verzeichnis das Startverzeichnis von Thing setzen -
 * - z.B. nach AusfÅhrung eines Programms, das evtl. die Pfade verÑndert hat
 */
void clr_drv(void) {
	Dsetpath("\\");
	set_dir(tb.homepath);
}
#endif
