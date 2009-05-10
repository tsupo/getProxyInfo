/*
 *  getProxyInfo.c
 *      ブラウザの設定情報から proxy サーバに関する情報を取得する
 *
 *      written by H.Tsujimura
 *          13 Dec 2006     CodeZine 公開用にソース整理
 *          27 Oct 2006     Firefox 2 対応
 *          19 Sep 2006     Opera 9 対応
 *          31 Oct 2005     Firefox 関連処理修正
 *           7 Jul 2004     最初の版
 *
 * Copyright (c) 2006, Oki Software Co., Ltd.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer. 
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution. 
 * - Neither the name of the Oki Software Co., Ltd. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * History:
 * $Log: /getProxyInfo/getProxyInfo.c $
 * 
 * 2     07/07/03 13:04 Tsujimura543
 * Visual Studio 2005 でのビルド結果を反映
 * 
 * 1     06/12/13 15:59 Tsujimura543
 * CodeZine 公開用にソースを整理
 */

#include <stdio.h>
#include <windows.h>

#ifndef	lint
static char	*rcs_id =
"$Header: /getProxyInfo/getProxyInfo.c 2     07/07/03 13:04 Tsujimura543 $";
#endif


int
strncmpi( const char *s, const char *t, int n )
{
    int ret = 0;
    int i, j;

    while ( *s && *t && ( n > 0 ) ) {
        i = *s++;
        j = *t++;
        if ( i != j ) {
            if ( ( i >= 'a' ) && ( i <= 'z' ) )
                i &= ~0x20;
            if ( ( j >= 'a' ) && ( j <= 'z' ) )
                j &= ~0x20;
            if ( i != j ) {
                ret = i - j;
                break;
            }
        }
        n--;
    }

    if ( ( ret == 0 ) && ( n > 0 ) ) {
        if ( *s )
            ret = *s;
        if ( *t )
            ret = -(*t);
    }

    return ( ret );
}



/* デフォルトのポート番号 */
#define DEFAULT_HTTP_PORT       80


/* ブラウザ種別 */
#define BR_UNKNOWN      0   /* 不明    */
#define BR_IEXPLORER    1   /* IE      */
#define BR_FIREFOX      2   /* Firefox */
#define BR_OPERA        3   /* Opera   */
#define BR_EXTRA        4   /* その他  */


/*
 *  「通常使用するブラウザ」の取得
 */
int
getDefaultBrowser( void )
{
    int     ret = BR_UNKNOWN;
    long    r;
    HKEY    hk;
    long    type;
    char    buf[BUFSIZ];
    long    sz = BUFSIZ - 1;

    r = RegOpenKeyEx( HKEY_CLASSES_ROOT,
                      "http\\shell\\open\\ddeexec\\Application",
                      0,
                      KEY_READ,
                      &hk );
    if ( r != ERROR_SUCCESS )
        r = RegOpenKeyEx( HKEY_CLASSES_ROOT,
                      "htmlfile\\shell\\open\\command",
                      0,
                      KEY_READ,
                      &hk );
    if ( r == ERROR_SUCCESS ) {
        r = RegQueryValueEx( hk,
                             "",
                             NULL,
                             (unsigned long *)&type,
                             (unsigned char *)buf,
                             (unsigned long *)&sz );
        if ( r == ERROR_SUCCESS ) {
            if ( type == REG_SZ ) {
                if ( strstr( buf, "iexplore" ) ||
                     strstr( buf, "IExplore" )    )
                    ret = BR_IEXPLORER;
                else if ( strstr( buf, "firefox" ) ||
                          strstr( buf, "Firefox" )    )
                    ret = BR_FIREFOX;
                else if ( strstr( buf, "opera" ) ||
                          strstr( buf, "Opera" )    )
                    ret = BR_OPERA;
                else
                    ret = BR_EXTRA;
            }
        }
        RegCloseKey( hk );
    }

    return ( ret );
}


#ifdef _MSC_VER
#pragma warning ( disable : 4996 )  // for Visual C++ 2005
#endif
/*
 *  Internet Explorer の設定情報から、proxy関連情報を取得する
 */
BOOL
getProxyInfoViaInternetExplorer(
        BOOL           *useProxy,   /* (O) proxy を使っているか否か */
        char           *proxyServer,/* (O) proxy サーバ名           */
        unsigned short *proxyPort,  /* (O) proxy ポート番号         */
        BOOL           verbose      /* (I) verbose モード           */
    )
{
    /* 以下のレジストリから proxy 関連情報を取得する                   */
    /*   HKEY_CURRENT_USER                                             */
    /*     Software\Microsoft\Windows\CurrentVersion\Internet Settings */
    /*       ProxyServer                                               */

    long    ret;
    HKEY    hk;
    long    type;
    char    buf[BUFSIZ], *p;
    long    sz = BUFSIZ - 1;
    BOOL    isInternetExplorerActive = FALSE;

    ret = RegOpenKeyEx( HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                        0,
                        KEY_READ,
                        &hk );
    if ( ret == ERROR_SUCCESS ) {
        isInternetExplorerActive = TRUE;
                            /* Internet Explorer インストール済み */

        ret = RegQueryValueEx( hk,
                               "ProxyServer",
                               NULL,
                               (unsigned long *)&type,
                               (unsigned char *)buf,
                               (unsigned long *)&sz );
        if ( ret == ERROR_SUCCESS ) {
            if ( type == REG_SZ ) {
                p = strchr( buf, ':' );
                if ( p ) {
                    *p = '\0';
                    strcpy( proxyServer, buf );
                    *proxyPort = (unsigned short)(atol( p + 1 ) & 0xFFFF);
                    if ( (*proxyPort > 0) && (proxyServer[0] != '\0') ) {
                        if ( verbose )
                            fprintf( stderr, "\tproxy = %s:%d\n",
                                     proxyServer, *proxyPort );
                        *useProxy = TRUE;
                    }
                }
            }
        }
        RegCloseKey( hk );
    }

    return ( isInternetExplorerActive );
}

/*
 *  Firefox の設定情報から、proxy関連情報を取得する
 */
BOOL
getProxyInfoViaFirefox(
        BOOL           *useProxy,   /* (O) proxy を使っているか否か */
        char           *proxyServer,/* (O) proxy サーバ名           */
        unsigned short *proxyPort,  /* (O) proxy ポート番号         */
        BOOL           verbose      /* (I) verbose モード           */
    )
{
    /* 手順                                                              */
    /*  (1) 環境変数 USERPROFILE の指すディレクトリ以下のファイルを探索  */
    /*  (2) Application Data/Mozilla/Firefox/profiles.ini というファイル */
    /*      から                                                         */
    /*        Path=Profiles/*.default                                    */
    /*      という行を取得する (* 部分は環境によって変わる)              */
    /*  (4) Application Data/Mozilla/Firefox/Profiles/*.default/prefs.js */
    /*      (ただし、設定ファイルを置く場所を(Profiles/*.default以外の)  */
    /*       別のところに変更している場合も考慮して Path  の示すディレク */
    /*      トリ直下から prefs.js を探した方がいい)                      */
    /*      というファイルの中に                                         */
    /*        user_pref("network.proxy.http", "proxyサーバ名");          */
    /*        user_pref("network.proxy.http_port", ポート番号);          */
    /*      という記述があれば、proxy を使用する                         */
    /* 注意: 用途に合わせて profile を複数用意し使い分けているような場合 */
    /*       は、この手順で proxy の設定を取得できるとは限らない (取得で */
    /*       きるのは、現在 active 状態になっている設定のみ)             */

#if 0
    long    ret;
    HKEY    hk;
    long    type;
    long    sz = BUFSIZ - 1;
#endif
    char    buf[BUFSIZ];
    BOOL    isFirefoxActive = FALSE;

#if 0
    ret = RegOpenKeyEx( HKEY_CURRENT_USER,
                        "Software\\Mozilla\\Mozilla Firefox",
                        0,
                        KEY_READ,
                        &hk );
    if ( ret == ERROR_SUCCESS ) {
        // Firefox 2.x では、このレジストリを使用しなくなった
        // (Firefox 1.x のときのみ有効)
        ret = RegQueryValueEx( hk,
                               "CurrentVersion",
                               NULL,
                               (unsigned long *)&type,
                               (unsigned char *)buf,
                               (unsigned long *)&sz );
        if ( ret == ERROR_SUCCESS ) {
            /* Firefox インストール済みであると判断 */
            isFirefoxActive = TRUE;
        }
        RegCloseKey( hk );
    }

    if ( isFirefoxActive == TRUE ) {
#endif
        char    *p = getenv("USERPROFILE");

        if ( p && *p ) {
            char    targetDir[MAX_PATH];
            char    filename[MAX_PATH];
            char    path[MAX_PATH];
            FILE    *fp;
            BOOL    isRelative = TRUE;

            sprintf( targetDir,
                     "%s\\Application Data\\Mozilla\\Firefox\\", p );
            sprintf( filename, "%sprofiles.ini", targetDir );
            path[0] = '\0';

            if ( ( fp = fopen( filename, "r" ) ) != NULL ) {
                char    *p;

                isFirefoxActive = TRUE;
                while ( ( p = fgets( buf, BUFSIZ - 1, fp ) ) != NULL ) {
                    if ( !strncmp( p, "IsRelative=", 11 ) ) {
                        p += 11;
                        isRelative = (atol( p ) == 0) ? FALSE : TRUE;
                        continue;
                    }
                    if ( !strncmp( p, "Path=", 5 ) ) {
                        p += 5;
                        strcpy( path, p );
                        while ( (path[strlen(p) - 1] == '\n') ||
                                (path[strlen(p) - 1] == '\r') ||
                                (path[strlen(p) - 1] == ' ' ) ||
                                (path[strlen(p) - 1] == '/' ) ||
                                (path[strlen(p) - 1] == '\\')    )
                            path[strlen(p) - 1] = '\0';
                        break;
                    }
                }
                fclose( fp );

                if ( path[0] ) {
                    if ( isRelative )
                        sprintf( filename, "%s%s\\prefs.js",
                                 targetDir, path );
                    else
                        sprintf( filename, "%s\\prefs.js", path );

                    if ( ( fp = fopen( filename, "r" ) ) != NULL ) {
                        while ( (p = fgets( buf, BUFSIZ - 1, fp )) != NULL ) {
                            if ( !strncmp( p,
                                           "user_pref(\"network.proxy.http\"",
                                           30 ) ) {
                                char    *q;

                                p += 30;
                                q = strchr( p, '"' );
                                if ( q ) {
                                    q++;
                                    p = strchr( q, '"' );
                                    if ( p ) {
                                        *p = '\0';
                                        strcpy( proxyServer, q );
                                        *useProxy  = TRUE;
                                        *proxyPort = DEFAULT_HTTP_PORT;
                                            /* とりあえず、デフォルト値を */
                                            /* 入れておく                 */
                                    }
                                }
                            }
                            else if ( !strncmp( p,
                                      "user_pref(\"network.proxy.http_port\"",
                                                35 ) ) {
                                p += 35;
                                while ( *p && ((*p < '0') || (*p > '9')) )
                                    p++;
                                if ( *p ) {
                                    *proxyPort =
                                        (unsigned short)(atol( p ) & 0xFFFF);
                                    if ( *useProxy == TRUE )
                                        break;
                                }
                            }
                        }
                        fclose( fp );
                    }
                    else {
                        if ( verbose )
                            fprintf( stderr,
                                     "設定ファイル(%s)が見つかりません\n",
                                     filename );
                    }
                }
            }
            else {
                if ( verbose )
                    fprintf( stderr, "設定ファイル(%s)が見つかりません\n",
                             filename );
            }
        }
#if 0
    }
#endif

    if ( *useProxy == TRUE )
        if ( verbose )
            fprintf( stderr, "\tproxy = %s:%d\n",
                     proxyServer, *proxyPort );

    return ( isFirefoxActive );
}


/*
 *  Opera の設定情報から、proxy関連情報を取得する
 *      -- OperaDef6.ini 形式のファイルからproxy関連情報を取得する
 */
BOOL
readProxySettingFromOperaIni(
        const char     *filename,   /* (I) 解析対象 .ini ファイル名 */
        BOOL           *useProxy,   /* (O) proxy を使っているか否か */
        char           *proxyServer,/* (O) proxy サーバ名           */
        unsigned short *proxyPort,  /* (O) proxy ポート番号         */
        BOOL           verbose      /* (I) verbose モード           */
    )
{
    BOOL    found = FALSE;
    char    buf[BUFSIZ];
    char    *p;
    FILE    *fp;

    if ( ( fp = fopen( filename, "r" ) ) != NULL ) {
        while ( ( p = fgets( buf, BUFSIZ - 1, fp ) ) != NULL ) {
            if ( !strncmpi( p, "HTTP Server=",  12 ) ||
                 !strncmpi( p, "HTTPS Server=", 13 )    ) {
                    /* Opera 8 までは Server,                     */
                    /* Opera 9 は     server [s が小文字になった] */
                char    *q;

                found = TRUE;
                p = strchr( p + 11, '=' ) + 1;
                q = strchr( p, ':' );
                if ( !q ) {
                    q = p + (strlen(p) - 1);
                    while ( (q > p) && (*q == '\n') || (*q == '\r') )
                        *q-- = '\0';
                    strcpy( proxyServer, p );
                    *proxyPort = DEFAULT_HTTP_PORT;
                                        /* とりあえず、デフォルト値を */
                                        /* 入れておく                 */
                }
                else {
                    *q++ = '\0';
                    strcpy( proxyServer, p );
                    *proxyPort = (unsigned short)(atol( q ) & 0xFFFF);
                }
                *useProxy = TRUE;

                break;
            }
        }
        fclose( fp );
    }
    else {
        if ( verbose )
            fprintf( stderr, "設定ファイル(%s)が見つかりません\n", filename );
    }

    return ( found );
}

/*
 *  Opera の設定情報から、proxy関連情報を取得する
 *      -- 処理本体
 */
BOOL
getProxyInfoViaOpera(
        BOOL           *useProxy,   /* (O) proxy を使っているか否か */
        char           *proxyServer,/* (O) proxy サーバ名           */
        unsigned short *proxyPort,  /* (O) proxy ポート番号         */
        BOOL           verbose      /* (I) verbose モード           */
    )
{
    /* 手順                                                               */
    /*  (1) レジストリ                                                    */
    /*        HKEY_CURRENT_USER\Software\Opera Software                   */
    /*      が存在すれば、Opera インストール済み                          */
    /*  (2) 上記レジストリから キー                                       */
    /*        Last Directory2                                             */
    /*      の値を取得 (Last Directory2 の値が取れなかった場合は          */
    /*      Last Directory3 の値を取得)                                   */
    /*  (3) Last Directory2 も Last Directory3 も取れなかった場合は       */
    /*        Plugin Path                                                 */
    /*      の値を取得 (Plugin Path の値が取れなかった場合は              */
    /*      Last CommandLine v2 の値を取得)                               */
    /*  (4) 上記 (2) または (3) で得た情報から Opera のインストール先ディ */
    /*      レクトリがどこか判断                                          */
    /*  (5) Opera のインストール先ディレクトリにある OperaDef6.ini という */
    /*      ファイルの中に                                                */
    /*         HTTP Server=proxyサーバ名:ポート番号                       */
    /*      という記述があれば、proxy を使用する (Opera 6 ～ 8)           */
    /*  (6) 環境変数 USERPROFILE の指すディレクトリ以下の                 */
    /*      Application Data/Opera/Opera/profile/opera6.ini というファイ  */
    /*      ルの中に                                                      */
    /*         HTTP server=proxyサーバ名:ポート番号                       */
    /*      という記述があれば、proxy を使用する (Opera 9)                */

    long    ret;
    HKEY    hk;
    long    type;
    long    sz = BUFSIZ - 1;
    char    buf[BUFSIZ];
    char    *p;
    BOOL    isOperaActive = FALSE;

    ret = RegOpenKeyEx( HKEY_CURRENT_USER,
                        "Software\\Opera Software",
                        0,
                        KEY_READ,
                        &hk );
    if ( ret == ERROR_SUCCESS ) {
        ret = RegQueryValueEx( hk,
                               "Last Directory2",
                               NULL,
                               (unsigned long *)&type,
                               (unsigned char *)buf,
                               (unsigned long *)&sz );
        if ( ret != ERROR_SUCCESS )
            ret = RegQueryValueEx( hk,
                                   "Last Directory3",
                                   NULL,
                                   (unsigned long *)&type,
                                   (unsigned char *)buf,
                                   (unsigned long *)&sz );
                        // ↑↑↑ 通常はこれが hit する (Opera 6 以降) ↑↑↑
        if ( ret != ERROR_SUCCESS ) {
            ret = RegQueryValueEx( hk,
                                   "Plugin Path",
                                   NULL,
                                   (unsigned long *)&type,
                                   (unsigned char *)buf,
                                   (unsigned long *)&sz );
            if ( ret == ERROR_SUCCESS ) {
                p = strstr( buf, "\\\\Program\\\\Plugins" );
                if ( p && *p )
                    *p = '\0';
            }
        }
        if ( ret != ERROR_SUCCESS ) {
            // for Opera 9
            ret = RegQueryValueEx( hk,
                                   "Last CommandLine v2",
                                   NULL,
                                   (unsigned long *)&type,
                                   (unsigned char *)buf,
                                   (unsigned long *)&sz );
            if ( ret == ERROR_SUCCESS ) {
                p = strstr( buf, "\\\\Opera.exe" );
                if ( p && *p )
                    *p = '\0';
            }
        }
        if ( ret == ERROR_SUCCESS ) {
            /* Opera インストール済みであると判断 */
            isOperaActive = TRUE;
        }
        RegCloseKey( hk );
    }

    if ( isOperaActive == TRUE ) {
        BOOL    found = FALSE;
        char    filename[MAX_PATH];

        sprintf( filename, "%s\\OperaDef6.ini", buf );

        found = readProxySettingFromOperaIni( filename,
                                              useProxy,
                                              proxyServer, proxyPort,
                                              verbose );
        if ( found == FALSE ) {
            // for Opera 9
            char    *p = getenv( "USERPROFILE" );

            if ( p && *p ) {
                char    targetDir[MAX_PATH];

                sprintf( targetDir,
                         "%s\\Application Data\\Opera\\Opera\\profile\\", p );
                sprintf( filename, "%sopera6.ini", targetDir );
                found = readProxySettingFromOperaIni( filename,
                                                      useProxy,
                                                      proxyServer, proxyPort,
                                                      verbose );
            }
        }
    }

    if ( *useProxy == TRUE )
        if ( verbose )
            fprintf( stderr, "\tproxy = %s:%d\n",
                     proxyServer, *proxyPort );

    return ( isOperaActive );
}


/*
 *  proxy サーバ名 と proxy ポート番号の取得
 *    ・proxy.txt というファイルが存在する場合、当該ファイルから情報を読み取る
 *        proxy.txt の
 *          1行目に「proxyサーバ名(または IPアドレス)」、
 *          2行目に「ポート番号」
 *        を記述する
 *    ・proxy.txt というファイルが存在しない場合、「通常使用するブラウザ」の
 *      proxy 関連情報を読み取る(この版では、Internet Explorer、Firefox、Opera
 *       の3ブラウザに対応)
 */
BOOL
getProxyInfo(
        char           *proxyServer,/* (O) proxy サーバ名   */
        unsigned short *proxyPort,  /* (O) proxy ポート番号 */
        BOOL           verbose      /* (I) verbose モード   */
    )
{
    FILE    *fp;
    BOOL    useProxy = FALSE;   /* proxy サーバを利用しているか否か */

    if ( !proxyServer || !proxyPort )
        return ( useProxy );

    if ( ( fp = fopen( "proxy.txt", "r" ) ) != NULL ) {
        char    *p, *q;
        char    buf[BUFSIZ];

        while ( ( p = fgets( buf, BUFSIZ - 1, fp ) ) != NULL ) {
            if ( p[strlen(p) - 1] == '\n' )
                p[strlen(p) - 1] = '\0';
            if ( ((q = strchr( p, '.' )) != NULL) ||
                (*p < '0') || (*p > '9') ) {
                strcpy( proxyServer, p );
            }
            else if ( (*p >= '0') && (*p <= '9') ) {
                *proxyPort = (unsigned short)((atol( p ) & 0xFFFF));
            }
        }
        fclose( fp );

        if ( (*proxyPort > 0) && (proxyServer[0] != '\0') ) {
            if ( verbose )
                fprintf( stderr, "\tproxy = %s:%d\n",
                         proxyServer, *proxyPort );
            useProxy = TRUE;
        }
    }

    if ( useProxy == FALSE ) {
        int browser = getDefaultBrowser();

        switch ( browser ) {
        case BR_FIREFOX:
            getProxyInfoViaFirefox( &useProxy,
                                    proxyServer, proxyPort,
                                    verbose );
            if ( useProxy == FALSE )
                getProxyInfoViaInternetExplorer( &useProxy,
                                                 proxyServer, proxyPort,
                                                 verbose );
            if ( useProxy == FALSE )
                getProxyInfoViaOpera( &useProxy,
                                      proxyServer, proxyPort,
                                      verbose );
            break;

        case BR_OPERA:
            getProxyInfoViaOpera( &useProxy,
                                  proxyServer, proxyPort,
                                  verbose );
            if ( useProxy == FALSE )
                getProxyInfoViaInternetExplorer( &useProxy,
                                                 proxyServer, proxyPort,
                                                 verbose );
            if ( useProxy == FALSE )
                getProxyInfoViaFirefox( &useProxy,
                                        proxyServer, proxyPort,
                                        verbose );
            break;

        case BR_IEXPLORER:
        default:
            getProxyInfoViaInternetExplorer( &useProxy,
                                             proxyServer, proxyPort,
                                             verbose );
            if ( useProxy == FALSE )
                getProxyInfoViaFirefox( &useProxy,
                                        proxyServer, proxyPort,
                                        verbose );
            if ( useProxy == FALSE )
                getProxyInfoViaOpera( &useProxy,
                                      proxyServer, proxyPort,
                                      verbose );
            break;
        }
    }

    return ( useProxy );
}
#ifdef _MSC_VER
#pragma warning ( default : 4996 )  // for Visual C++ 2005
#endif


/*
 *  proxyサーバを利用しているかどうかを調査する
 *  (proxyサーバを利用している場合は、proxy関連情報を取得する)
 */
BOOL
isUsedProxy(
        char           *server,     /* (O) proxy サーバ名   */
        unsigned short *proxyPort,  /* (O) proxy ポート番号 */
        BOOL           verbose      /* (I) verbose モード   */
    )
{
    return ( getProxyInfo( server, proxyPort, verbose ) );
}



int
main( int argc, char *argv[] )
{
    char            server[MAX_PATH];
    unsigned short  proxyPort = 0;
    BOOL            useProxy  = FALSE;
    BOOL            verbose   = FALSE;

    if ( argc > 1 ) {
        int i, j;

        for ( i = 1; i < argc; i++ ) {
            if ( argv[i][0] == '-' ) {
                for ( j = 1; argv[i][j]; j++ ) {
                    switch ( argv[i][j] ) {
                    case 'v':
                        verbose = !verbose;
                        break;
                    }
                }
            }
        }
    }

    useProxy = isUsedProxy( server, &proxyPort, verbose );
    if ( useProxy ) {
        fputs( "以下のproxyサーバを利用しています\n", stdout );
        printf( "\tproxyサーバ名:   %s\n", server );
        printf( "\tproxyポート番号: %d\n", proxyPort );
    }
    else
        fputs( "proxyサーバを利用していません\n", stdout );

    return ( useProxy == FALSE ? 0 : 1 );
}
