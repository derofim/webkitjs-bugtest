LIBWEBKITGTK_VERSION=libwebkitgtk_version
LIBJAVASCRIPTCOREGTK_VERSION=libjavascriptcoregtk_version
LIBWEBKIT2GTK_VERSION=libwebkit2gtk_version
AC_SUBST([LIBWEBKITGTK_VERSION])
AC_SUBST([LIBJAVASCRIPTCOREGTK_VERSION])
AC_SUBST([LIBWEBKIT2GTK_VERSION])

if test -z "$AR_FLAGS"; then
    AR_FLAGS="cruT"
fi
AC_SUBST([AR_FLAGS])

DOLT
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
