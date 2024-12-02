// stdafx.h : Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#ifndef _WIN32_WINNT		// Lassen Sie die Verwendung spezifischer Features von Windows XP oder später zu.                   
#define _WIN32_WINNT 0x0501	// Ändern Sie dies in den geeigneten Wert für andere Versionen von Windows.
#endif						

#include <stdio.h>
#ifdef _WIN32
    #include <tchar.h>
#else
    // Define Windows-compatible macros/types for non-Windows platforms
    typedef char TCHAR;
    #define _T(x) x
#endif




// TODO: Hier auf zusätzliche Header, die das Programm erfordert, verweisen.
