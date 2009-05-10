/*
 *  getProxyInfo.c
 *      �u���E�U�̐ݒ��񂩂� proxy �T�[�o�Ɋւ�������擾����
 *
 *      written by H.Tsujimura
 *          13 Dec 2006     CodeZine ���J�p�Ƀ\�[�X����
 *          27 Oct 2006     Firefox 2 �Ή�
 *          19 Sep 2006     Opera 9 �Ή�
 *          31 Oct 2005     Firefox �֘A�����C��
 *           7 Jul 2004     �ŏ��̔�
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
 * Visual Studio 2005 �ł̃r���h���ʂ𔽉f
 * 
 * 1     06/12/13 15:59 Tsujimura543
 * CodeZine ���J�p�Ƀ\�[�X�𐮗�
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



/* �f�t�H���g�̃|�[�g�ԍ� */
#define DEFAULT_HTTP_PORT       80


/* �u���E�U��� */
#define BR_UNKNOWN      0   /* �s��    */
#define BR_IEXPLORER    1   /* IE      */
#define BR_FIREFOX      2   /* Firefox */
#define BR_OPERA        3   /* Opera   */
#define BR_EXTRA        4   /* ���̑�  */


/*
 *  �u�ʏ�g�p����u���E�U�v�̎擾
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
 *  Internet Explorer �̐ݒ��񂩂�Aproxy�֘A�����擾����
 */
BOOL
getProxyInfoViaInternetExplorer(
        BOOL           *useProxy,   /* (O) proxy ���g���Ă��邩�ۂ� */
        char           *proxyServer,/* (O) proxy �T�[�o��           */
        unsigned short *proxyPort,  /* (O) proxy �|�[�g�ԍ�         */
        BOOL           verbose      /* (I) verbose ���[�h           */
    )
{
    /* �ȉ��̃��W�X�g������ proxy �֘A�����擾����                   */
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
                            /* Internet Explorer �C���X�g�[���ς� */

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
 *  Firefox �̐ݒ��񂩂�Aproxy�֘A�����擾����
 */
BOOL
getProxyInfoViaFirefox(
        BOOL           *useProxy,   /* (O) proxy ���g���Ă��邩�ۂ� */
        char           *proxyServer,/* (O) proxy �T�[�o��           */
        unsigned short *proxyPort,  /* (O) proxy �|�[�g�ԍ�         */
        BOOL           verbose      /* (I) verbose ���[�h           */
    )
{
    /* �菇                                                              */
    /*  (1) ���ϐ� USERPROFILE �̎w���f�B���N�g���ȉ��̃t�@�C����T��  */
    /*  (2) Application Data/Mozilla/Firefox/profiles.ini �Ƃ����t�@�C�� */
    /*      ����                                                         */
    /*        Path=Profiles/*.default                                    */
    /*      �Ƃ����s���擾���� (* �����͊��ɂ���ĕς��)              */
    /*  (4) Application Data/Mozilla/Firefox/Profiles/*.default/prefs.js */
    /*      (�������A�ݒ�t�@�C����u���ꏊ��(Profiles/*.default�ȊO��)  */
    /*       �ʂ̂Ƃ���ɕύX���Ă���ꍇ���l������ Path  �̎����f�B���N */
    /*      �g���������� prefs.js ��T������������)                      */
    /*      �Ƃ����t�@�C���̒���                                         */
    /*        user_pref("network.proxy.http", "proxy�T�[�o��");          */
    /*        user_pref("network.proxy.http_port", �|�[�g�ԍ�);          */
    /*      �Ƃ����L�q������΁Aproxy ���g�p����                         */
    /* ����: �p�r�ɍ��킹�� profile �𕡐��p�ӂ��g�������Ă���悤�ȏꍇ */
    /*       �́A���̎菇�� proxy �̐ݒ���擾�ł���Ƃ͌���Ȃ� (�擾�� */
    /*       ����̂́A���� active ��ԂɂȂ��Ă���ݒ�̂�)             */

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
        // Firefox 2.x �ł́A���̃��W�X�g�����g�p���Ȃ��Ȃ���
        // (Firefox 1.x �̂Ƃ��̂ݗL��)
        ret = RegQueryValueEx( hk,
                               "CurrentVersion",
                               NULL,
                               (unsigned long *)&type,
                               (unsigned char *)buf,
                               (unsigned long *)&sz );
        if ( ret == ERROR_SUCCESS ) {
            /* Firefox �C���X�g�[���ς݂ł���Ɣ��f */
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
                                            /* �Ƃ肠�����A�f�t�H���g�l�� */
                                            /* ����Ă���                 */
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
                                     "�ݒ�t�@�C��(%s)��������܂���\n",
                                     filename );
                    }
                }
            }
            else {
                if ( verbose )
                    fprintf( stderr, "�ݒ�t�@�C��(%s)��������܂���\n",
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
 *  Opera �̐ݒ��񂩂�Aproxy�֘A�����擾����
 *      -- OperaDef6.ini �`���̃t�@�C������proxy�֘A�����擾����
 */
BOOL
readProxySettingFromOperaIni(
        const char     *filename,   /* (I) ��͑Ώ� .ini �t�@�C���� */
        BOOL           *useProxy,   /* (O) proxy ���g���Ă��邩�ۂ� */
        char           *proxyServer,/* (O) proxy �T�[�o��           */
        unsigned short *proxyPort,  /* (O) proxy �|�[�g�ԍ�         */
        BOOL           verbose      /* (I) verbose ���[�h           */
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
                    /* Opera 8 �܂ł� Server,                     */
                    /* Opera 9 ��     server [s ���������ɂȂ���] */
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
                                        /* �Ƃ肠�����A�f�t�H���g�l�� */
                                        /* ����Ă���                 */
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
            fprintf( stderr, "�ݒ�t�@�C��(%s)��������܂���\n", filename );
    }

    return ( found );
}

/*
 *  Opera �̐ݒ��񂩂�Aproxy�֘A�����擾����
 *      -- �����{��
 */
BOOL
getProxyInfoViaOpera(
        BOOL           *useProxy,   /* (O) proxy ���g���Ă��邩�ۂ� */
        char           *proxyServer,/* (O) proxy �T�[�o��           */
        unsigned short *proxyPort,  /* (O) proxy �|�[�g�ԍ�         */
        BOOL           verbose      /* (I) verbose ���[�h           */
    )
{
    /* �菇                                                               */
    /*  (1) ���W�X�g��                                                    */
    /*        HKEY_CURRENT_USER\Software\Opera Software                   */
    /*      �����݂���΁AOpera �C���X�g�[���ς�                          */
    /*  (2) ��L���W�X�g������ �L�[                                       */
    /*        Last Directory2                                             */
    /*      �̒l���擾 (Last Directory2 �̒l�����Ȃ������ꍇ��          */
    /*      Last Directory3 �̒l���擾)                                   */
    /*  (3) Last Directory2 �� Last Directory3 �����Ȃ������ꍇ��       */
    /*        Plugin Path                                                 */
    /*      �̒l���擾 (Plugin Path �̒l�����Ȃ������ꍇ��              */
    /*      Last CommandLine v2 �̒l���擾)                               */
    /*  (4) ��L (2) �܂��� (3) �œ�����񂩂� Opera �̃C���X�g�[����f�B */
    /*      ���N�g�����ǂ������f                                          */
    /*  (5) Opera �̃C���X�g�[����f�B���N�g���ɂ��� OperaDef6.ini �Ƃ��� */
    /*      �t�@�C���̒���                                                */
    /*         HTTP Server=proxy�T�[�o��:�|�[�g�ԍ�                       */
    /*      �Ƃ����L�q������΁Aproxy ���g�p���� (Opera 6 �` 8)           */
    /*  (6) ���ϐ� USERPROFILE �̎w���f�B���N�g���ȉ���                 */
    /*      Application Data/Opera/Opera/profile/opera6.ini �Ƃ����t�@�C  */
    /*      ���̒���                                                      */
    /*         HTTP server=proxy�T�[�o��:�|�[�g�ԍ�                       */
    /*      �Ƃ����L�q������΁Aproxy ���g�p���� (Opera 9)                */

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
                        // ������ �ʏ�͂��ꂪ hit ���� (Opera 6 �ȍ~) ������
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
            /* Opera �C���X�g�[���ς݂ł���Ɣ��f */
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
 *  proxy �T�[�o�� �� proxy �|�[�g�ԍ��̎擾
 *    �Eproxy.txt �Ƃ����t�@�C�������݂���ꍇ�A���Y�t�@�C���������ǂݎ��
 *        proxy.txt ��
 *          1�s�ڂɁuproxy�T�[�o��(�܂��� IP�A�h���X)�v�A
 *          2�s�ڂɁu�|�[�g�ԍ��v
 *        ���L�q����
 *    �Eproxy.txt �Ƃ����t�@�C�������݂��Ȃ��ꍇ�A�u�ʏ�g�p����u���E�U�v��
 *      proxy �֘A����ǂݎ��(���̔łł́AInternet Explorer�AFirefox�AOpera
 *       ��3�u���E�U�ɑΉ�)
 */
BOOL
getProxyInfo(
        char           *proxyServer,/* (O) proxy �T�[�o��   */
        unsigned short *proxyPort,  /* (O) proxy �|�[�g�ԍ� */
        BOOL           verbose      /* (I) verbose ���[�h   */
    )
{
    FILE    *fp;
    BOOL    useProxy = FALSE;   /* proxy �T�[�o�𗘗p���Ă��邩�ۂ� */

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
 *  proxy�T�[�o�𗘗p���Ă��邩�ǂ����𒲍�����
 *  (proxy�T�[�o�𗘗p���Ă���ꍇ�́Aproxy�֘A�����擾����)
 */
BOOL
isUsedProxy(
        char           *server,     /* (O) proxy �T�[�o��   */
        unsigned short *proxyPort,  /* (O) proxy �|�[�g�ԍ� */
        BOOL           verbose      /* (I) verbose ���[�h   */
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
        fputs( "�ȉ���proxy�T�[�o�𗘗p���Ă��܂�\n", stdout );
        printf( "\tproxy�T�[�o��:   %s\n", server );
        printf( "\tproxy�|�[�g�ԍ�: %d\n", proxyPort );
    }
    else
        fputs( "proxy�T�[�o�𗘗p���Ă��܂���\n", stdout );

    return ( useProxy == FALSE ? 0 : 1 );
}
