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

#ifdef __MINT__
#include <sysvars.h>		/* fuer getSystemLanguage() */
#endif
#include "..\include\thingtbx.h"

/*
 * Diese Routine ermittelt die Landessprache des Desktops. Als
 * erstes wird der Systemheader ausgewertet. Wenn der Test auf
 * den _AKP-Cookie erfolgreich ist, wird dessen Einstellung ge-
 * nommen und zum Schluss wird noch die Desktopsprache von MTOS
 * ueberprueft, da sie die Ausschlaggebende waere.
 *
 * @return
 */
WORD getSystemLanguage(void) {
	WORD language, du;
	LONG oldStack = 0, ret;
#ifdef __MINT__
	OSHEADER *syshdr;
#else
	SYSHDR *syshdr;
#endif

	if (tb.sys & SY_AGI) {
		appl_getinfo(3, &language, &du, &du, &du);
		return (language);
	} else if (getCookie('_AKP', &ret) == TRUE) {
		return (language = (WORD)(ret >> 8));
	} else {
		if (!Super((VOID *) 1l))
			oldStack = Super(0l);
#ifdef __MINT__
		syshdr = *(OSHEADER **) 0x4f2;
		syshdr = syshdr->os_beg;
		language = syshdr->os_conf >> 1;
#else
		syshdr = *(SYSHDR **) 0x4f2;
		syshdr = syshdr->os_base;
		language = syshdr->os_palmode >> 1;
#endif
		if (oldStack)
			Super((VOID *) oldStack);

		return (language);
	}
}
