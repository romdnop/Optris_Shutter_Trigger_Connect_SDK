// stdafx.h : Includedatei f�r Standardsystem-Includedateien
// oder h�ufig verwendete projektspezifische Includedateien,
// die nur in unregelm��igen Abst�nden ge�ndert werden.
//

#pragma once

#ifndef _WIN32_WINNT		// Lassen Sie die Verwendung spezifischer Features von Windows XP oder sp�ter zu.                   
#define _WIN32_WINNT 0x0501	// �ndern Sie dies in den geeigneten Wert f�r andere Versionen von Windows.
#endif						

#include <stdio.h>
#ifdef _WIN32
    #include <tchar.h>
#else
    // Define Windows-compatible macros/types for non-Windows platforms
    typedef char TCHAR;
    #define _T(x) x
#endif




// TODO: Hier auf zus�tzliche Header, die das Programm erfordert, verweisen.
