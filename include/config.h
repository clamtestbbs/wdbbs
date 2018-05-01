/*-------------------------------------------------------*/
/* config.h     ( WD-BBS Ver 2.3 )                         */
/*-------------------------------------------------------*/
/* target : site-configurable settings                   */
/* create : 95/03/29                                     */
/* update : 98/12/09                                     */
/*-------------------------------------------------------*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "../wdbbs.conf"                              /*r2.20180502: ¸Õ¦æ§ó¦³¼u©Êªº³]©wÀÉ */

/* ----------------------------------------------------- */
/* ©w¸q BBS ¯¸¦W¦ì§}                                     */
/* ------------------------------------------------------*/

#ifndef BOARDNAME
#define BOARDNAME       "­·»P¹Ð®Jªº¹ï¸Ü fork ª©¥»"   /* ¤¤¤å¯¸¦W */
#endif

#ifndef BBSNAME
#define BBSNAME         "WD-fork BBS"                /* ­^¤å¯¸¦W */
#endif

#ifndef MYHOSTNAME
#define MYHOSTNAME      "wdbbs.net"                  /* ºô§} */
#endif

#ifndef MYIP
#define MYIP            "140.129.79.234"             /* IP */
#endif

#define MYVERSION       "Fork WD"                    /* µ{¦¡ª©¥» */

#ifndef MYBBSID
#define MYBBSID         "WD-F"                       /* Âà«H¥N½X */
#endif

#ifndef BBSHOME
#define BBSHOME         "/home/bbs"                  /* BBS ªº®a */
#endif

#ifndef BBSUSER
#define BBSUSER         "bbs"
#endif

#ifndef BBSUID
#define BBSUID          9999
#endif

#ifndef BBSGID
#define BBSGID          99
#endif

#ifndef TAG_VALID
#define TAG_VALID        "[WD-F BBS]To "        
                        /* shakalaca.000814: §ï¥Î MagicKey »{ÃÒ
                                             ¨ä¹ê´N¤£¥Î³o­Óª±·N¤F :p */
#endif

/* ----------------------------------------------------- */
/* ²ÕºA³W¹º                                              */
/* ------------------------------------------------------*/

#ifndef __CYGWIN__
#define HAVE_CHKLOAD                /* check cpu load */
                                    /* r2.20180502: cygwin may not check cpu load */
#endif // __CYGWIN__

#ifdef HAVE_CHKLOAD
  #define MAX_CPULOAD     (10)            /* CPU ³Ì°ªload */
  #define MAX_SWAPUSED    (10)            /* SWAP³Ì°ª¨Ï¥Î²v */
#endif

#define WITHOUT_CHROOT                /* ¤£»Ý­n root set-uid */
#define HAVE_MOVIE                    /* Åã¥Ü°ÊºA§i¥ÜªO */
#define INTERNET_PRIVATE_EMAIL        /* ¥i¥H±H¨p¤H«H¥ó¨ì Internet */
#define BIT8                          /* ¤ä´©¤¤¤å¨t²Î */
#define GB_SUPPORT                    /* ¤ä´©Åã¥Ü ÁcÅé->Â²Åé¤¤¤å½X */
#define DEFAULTBOARD "SYSOP"          /* ¹w³]¬ÝªO */
#define LOGINATTEMPTS (3)             /* ³Ì¤j¶i¯¸¥¢»~¦¸¼Æ */
#define LOGINASNEW                    /* ±Ä¥Î¤W¯¸¥Ó½Ð±b¸¹¨î«× */
#define INTERNET_EMAIL                /* ¤ä´© InterNet Email ¥\¯à(§t Forward) */
#undef  REG_MAGICKEY                  /* µo¥X MagicKey eMail ¨­¥÷»{ÃÒ«H¨ç */
#define REG_FORM                      /* shakalaca: ¶ñµù¥U³æ */
                                      /* shakalaca.000813: »{ÃÒ¤è¦¡½Ð¾Ü¤@ */
#undef  NEWUSER_LIMIT                 /* ·s¤â¤W¸ôªº¤T¤Ñ­­¨î */
#undef  POSTNOTIFY                    /* ·s¤å³¹³qª¾¥\¯à */
#undef  INVISIBLE                     /* ÁôÂÃ BBS user ¨Ó¦Û¦ó³B */
#define MULTI_NUMS        (1)         /* ³Ì¦h­«½Æ¤W¯¸¤H¦¸ (guest°£¥~) */
#define INITIAL_SETUP                 /* ­è¶}¯¸¡AÁÙ¨S«Ø¥ß¹w³]¬ÝªO[SYSOP] */
#define HAVE_MMAP                     /* ±Ä¥Î mmap(): memory mapped I/O */
#define HAVE_ANONYMOUS                /* ´£¨Ñ Anonymous ªO */
#define HAVE_ORIGIN                   /* Åã¥Ü author ¨Ó¦Û¦ó³B */
#define HAVE_MAILCLEAN                /* ²M²z©Ò¦³¨Ï¥ÎªÌ­Ó¤H«H½c */
#define WHERE                         /* ¬O§_¦³¬G¶m¥\¯à */
#define HAVE_NOTE_2                   /* wildcat:¤p¬ÝªO¥\¯à */
#define HAVE_GEM_GOPHER               /* shakalaca: ³s½uºëµØ°Ï */
#define FORM_REG                      /* shakalaca: ¶ñµù¥U³æ */

/* ----------------------------------------------------- */
/* ÀH BBS ¯¸³W¼Ò¦¨ªø¦ÓÂX¼W                               */
/* ----------------------------------------------------- */

#ifndef MAXUSERS
#define MAXUSERS        (65536)         /* ³Ì°ªµù¥U¤H¼Æ */
#endif

#ifndef MAXBOARD
#define MAXBOARD        (512)           /* ³Ì¤j¶}ª©­Ó¼Æ */
#endif

#ifndef MAXACTIVE
#define MAXACTIVE       (256)           /* ³Ì¦h¦P®É¤W¯¸¤H¼Æ */
#endif

#define MAX_FRIEND      (128)           /* ¸ü¤J cache ¤§³Ì¦hªB¤Í¼Æ¥Ø */
#define MAX_REJECT      (32)            /* ¸ü¤J cache ¤§³Ì¦hÃa¤H¼Æ¥Ø */
#define MAX_MOVIE       (256)           /* ³Ì¦h°ÊºA¬Ýª©¼Æ */
#define MAX_FROM        (512)           /* ³Ì¦h¬G¶m¼Æ */
#define MAX_REVIEW      (10)            /* ³Ì¦h¤ô²y¦^ÅU */

/* ----------------------------------------------------- */
/* ¨ä¥L¨t²Î¤W­­°Ñ¼Æ                                      */
/* ----------------------------------------------------- */

#define MAX_HISTORY     8                /* °ÊºA¬ÝªO«O«ù 12 µ§¾ú¥v°O¿ý */
#define MAXKEEPMAIL     (100)            /* ³Ì¦h«O¯d´X«Ê MAIL¡H */
#define MAXEXKEEPMAIL   (400)            /* ³Ì¦h«H½c¥[¤j¦h¤Ö«Ê */
#define MAX_NOTE        (32)             /* ³Ì¦h«O¯d´X½g¯d¨¥¡H */
#define MAXSIGLINES     (16)             /* Ã±¦WÀÉ¤Þ¤J³Ì¤j¦æ¼Æ */
#define MAXQUERYLINES   (16)             /* Åã¥Ü Query/Plan °T®§³Ì¤j¦æ¼Æ */
#define MAXPAGES        (999)            /* more.c ¤¤¤å³¹­¶¼Æ¤W­­ (lines/22) */
#define MOVIE_INT       (8)              /* ¤Á´«°Êµeªº¶g´Á®É¶¡ 12 ¬í */
#define MAXTAGS         (256)                /* post/mail reader ¼ÐÅÒ¼Æ¥Ø¤§¤W­­ */

/* ----------------------------------------------------- */
/* µo§b¹L¤[¦Û°ÊÃ±°h                                      */
/* ------------------------------------------------------*/

#define LOGIN_TIMEOUT        (30)                /* login ®É¦h¤[¨S¦¨¥\Ã±¤J´NÂ_½u */

#define DOTIMEOUT

#ifdef  DOTIMEOUT
#define IDLE_TIMEOUT    (100*60)        /* ¤@¯ë±¡ªp¤§ timeout (¬í)*/
#define SHOW_IDLE_TIME                  /* Åã¥Ü¶¢¸m®É¶¡ */
#endif

/* ----------------------------------------------------- */
/* chat.c & xchatd.c ¤¤±Ä¥Îªº port ¤Î protocol           */
/* ------------------------------------------------------*/

#define CHATPORT        3838

#define MAXROOM         16              /* ³Ì¦h¦³´X¶¡¥]´[¡H */

#define EXIT_LOGOUT     0
#define EXIT_LOSTCONN   -1
#define EXIT_CLIERROR   -2
#define EXIT_TIMEDOUT   -3
#define EXIT_KICK       -4

#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"

#define BADCIDCHARS " *:/\,;.?"        /* Chat Room ¤¤¸T¥Î©ó nick ªº¦r¤¸ */


/* ----------------------------------------------------- */
/* ¨t²Î°Ñ¼Æ      cache                                   */
/* ----------------------------------------------------- */
#define MAGIC_KEY       1999   /* ¨­¤À»{ÃÒ«H¨ç½s½X */

#define SEM_ENTER      -1      /* enter semaphore */
#define SEM_LEAVE      1       /* leave semaphore */
#define SEM_RESET      0       /* reset semaphore */

#define BRDSHM_KEY      1215
#define UIDSHM_KEY      1217
#define UTMPSHM_KEY     1219
#define FILMSHM_KEY     1221    /* °ÊºA¬Ýª© , ¸`¤é */
#define FROMSHM_KEY     1223    /* whereis, ³Ì¦h¨Ï¥ÎªÌ */

#define BRDSEM_KEY      2005
#define FILMSEM_KEY     2000    /* semaphore key */
#define FROMSEM_KEY     2003    /* semaphore key */

/* ----------------------------------------------------- */
/* ¥Ó½Ð±b¸¹®É­n¨D¥Ó½ÐªÌ¯u¹ê¸ê®Æ                          */
/* ----------------------------------------------------- */

#define SHOWUID                 /* ¬Ý¨£¨Ï¥ÎªÌ UID */
#define SHOWTTY                 /* ¬Ý¨£¨Ï¥ÎªÌ TTY */
#define SHOWPID                 /* ¬Ý¨£¨Ï¥ÎªÌ PID */

#define REALINFO                /* ¯u¹ê©m¦W */

#ifdef  REALINFO
#undef  ACTS_REALNAMES          /* ¥D¥Ø¿ýªº (U)ser Åã¥Ü¯u¹ê©m¦W */
#undef  POST_REALNAMES          /* ¶K¤å¥ó®Éªþ¤W¯u¹ê©m¦W */
#undef  MAIL_REALNAMES          /* ±H¯¸¤º«H¥ó®Éªþ¤W¯u¹ê©m¦W */
#undef  QUERY_REALNAMES         /* ³Q Query ªº User §iª¾¯u¹ê©m¦W */
#endif

/* ----------------------------------------------------- */
/* WD ¦Û­q©w¸qâ                                          */
/* ----------------------------------------------------- */

#define ANNOUNCE_BRD        "Announce"
#define VOTEBOARD           "VoteBoard"

#define DEF_MAXP        5000        /* ¬ÝªO¤å³¹°ò¥»¤W­­¼Æ¶q */
#define DEF_MAXT        365        /* ¬ÝªO¤å³¹°ò¥»«O¯d¤Ñ¼Æ */

#define COLOR1        "\x1b[46;37m"
#define COLOR2        "\x1b[1m\x1b[44;33m"

#endif //_CONFIG_H_
