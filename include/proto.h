#include "hdr.h"

/* announce.c */
FILE *go_cmd(ITEM *node, int *sock);
char *nextfield(register char *data, register char *field);
FILE *my_open(char *path);
int go_proxy(char *fpath, ITEM *node, int update);
int g_additem(GMENU *pm, ITEM *myitem);
int go_menu(GMENU *pm, ITEM *node, int update);
int g_searchtitle(GMENU *pm, int rev);
int gem(char *maintitle, ITEM *path, int update);
int valid_dirname(char *fname);
int copy_stamp(char *fpath, char *fname);
void atitle(void);
int load_paste(void);
int a_copyitem(char *fpath, char *title, char *owner);
//int a_menu(char *maintitle, char *path, int lastlevel, int mode); //to be fixed
int AnnounceSelect(void);
int Announce(void);
int Log(void);
int XFile(void);
int HELP(void);
int user_gem(char *uid);
int user_allpost(char *uid);
void my_gem(void);
void my_allpost(void);
/* bbs.c */
void log_board(char *board, time_t usetime);
void log_board2(char *mode, time_t usetime);
void log_board3(char *mode, char *str, int *num);
void make_blist(void);
void make_bmlist(void);
void set_board(void);
void doent(int num, fileheader *ent);
int cmpbnames(char *bname, boardheader *brec);
int cmpfilename(fileheader *fhdr);
int cmpfmode(fileheader *fhdr);
int cmpfowner(fileheader *fhdr);
int do_select(int ent, fileheader *fhdr, char *direct);
int outgo_post(fileheader *fh, char *board);
void do_reply_title(int row, char *title);
int brdperm(char *brdname, char *userid);
int do_unanonymous_post(char *fpath);
int do_all_post(char *fpath);
int getindex(char *fpath, char *fname, int size);
int do_post(void);
int edit_post(int ent, fileheader *fhdr, char *direct);
int man(void);
int cite(int ent, fileheader *fhdr, char *direct);
int Cite_post(int ent, fileheader *fhdr, char *direct);
int Cite_posts(int ent, fileheader *fhdr, char *direct);
int edit_title(int ent, fileheader *fhdr, char *direct);
//int add_tag(int ent, fileheader *fhdr, char *direct); //to be checked
int del_tag(int ent, fileheader *fhdr, char *direct);
int gem_tag(int ent, fileheader *fhdr, char *direct);
int mark(int ent, fileheader *fhdr, char *direct);
int score(int ent, fileheader *fhdr, char *direct);
int del_range(int ent, fileheader *fhdr, char *direct);
int save_mail(int ent, fileheader *fh, char *direct);
int b_notes(void);
int board_select(void);
int board_digest(void);
int go_chat(void);
void Read(void);
void ReadSelect(void);
int Select(void);
int Post(void);
void cancel_post(fileheader *fhdr, char *fpath);
void note(void);
int m_sysop(void);
int Goodbye(void);
#ifdef NO_SO
void va_do_voteboardreply(va_list pvar);
int do_voteboard();
int t_chat();
#endif

/* board.c */
void brc_update(void);
void read_brc_buf(void);
int brc_initial(char *boardname);
void brc_addlist(char *fname);
int brc_unread(char *fname);
int Ben_Perm(boardheader *bptr);
//int check_newpost(boardstat *ptr);
int have_author(char *brdname, int n);
//int board(void);
int local_board(void);
int good_board(void);
int Boards(void);
int New(void);
int Favor(void);
int favor_edit(void);
char *b_namearray(char buf[][12 + 1], int *pnum, char *tag);
void force_board(char *bname);
void voteboard(void);
/* cache.c */
int reload_ucache(void);
void resolve_ucache(void);
int ci_strcmp(register char *s1, register char *s2);
int searchuser(char *userid);
int getuser(char *userid);
char *getuserid(int num);
void setuserid(int num, char *userid);
int searchnewuser(int mode);
char *u_namearray(char buf[][12 + 1], int *pnum, char *tag);
void resolve_utmp(void);
void setutmpmode(int mode);
int resetutmpent(void);
void getnewutmpent(user_info *up);
int apply_ulist(int (*fptr)(void));
user_info *search_ulist(int (*fptr)(void), int farg);
int count_multi(void);
user_info *search_ulistn(int (*fptr)(void), int farg, int unum);
int count_logins(int (*fptr)(void), int farg, int show);
void purge_utmp(user_info *uentp);
int count_ulist(void);
void touch_boards(void);
int reload_bcache(void);
void resolve_boards(void);
int getbnum(char *bname);
int apply_boards(int (*func)(void));
boardheader *getbcache(char *bname);
int touchbtotal(char *bname);
int inbtotal(char *bname, int add);
char *getbname(int bid);
int haspostperm(char *bname);
void reload_filmcache(void);
void resolve_garbage(void);
int reload_fcache(void);
void resolve_fcache(void);
user_info *searchowner(char *userid);
/* edit.c */
int star_ime(int key, char *ans1, char *ans2);
int check_local(int i);
int beginsig(char *s);
int ask(char *prompt);
int undelete_line(void);
int indent_spcs(void);
int insert_string(char *str);
char *ask_tmpbuf(int y);
void auto_backup(void);
void restore_backup(void);
void write_header(FILE *fp);
int showsignature(char *fname);
void addsignature(FILE *fp);
int edit_outs(char *text);
int goto_line(int lino);
char *strcasestr(const char *big, const char *little);
int search_str(int mode);
int match_paren(void);
int block_del(int hide);
int block_shift_left(void);
int block_shift_right(void);
int transform_to_color(char *line);
int block_color(void);
int vedit(char *fpath, int saveheader);
/* io.c */
int passwd_outs(char *text);
void init_alarm(void);
void oflush(void);
void output(char *s, int len);
void ochar(int c);
void add_io(int fd, int timeout);
void add_flush(int (*flushfunc)(void));
int num_in_buf(void);
int dogetch(void);
int igetch(void);
//int getdata(int line, int col, char *prompt, char *buf, int len, int echo, char *ans);  //to be fixed
char getans(char *prompt);
int igetkey(void);
/* mail.c */
int m_internet(void);
void mail_forward(fileheader *fhdr, char *direct, int mode);
int chkmailbox(void);
void hold_mail(char *fpath, char *receiver);
//int do_send(char *userid, char *title); //to be fixed
void my_send(char *uident);
int m_send(void);
int multi_send(char *title, int inmail);
int mail_list(void);
int mail_all(void);
int mail_mbox(void);
int m_new(void);
int mail_read(int ent, fileheader *fhdr, char *direct);
int mail_reply(int ent, fileheader *fhdr, char *direct);
int mail_save(int ent, fileheader *fhdr, char *direct);
int m_read(void);
int bbs_sendmail(char *fpath, char *title, char *receiver, char *key);
int doforward(char *direct, fileheader *fh, int mode);
int chkmail(int rechk);
/* mbbsd.c */
void log_usies(char *mode, char *msg);
void u_exit(char *mode);
void system_abort(void);
void abort_bbs(void);
void leave_bbs(void);
int dosearchuser(char *userid);
int show_last_call_in(void);
int add_distinct(char *fname, char *line);
int del_distinct(char *fname, char *line);
int where(char *from);
void check_BM(void);
void setup_utmp(int mode);
int do_aloha(char *hello);
void check_max_online(void);
void start_client(void);
int bad_host(char *name);
#ifdef NO_SO
void b_closepolls(void);
#endif

/* menu.c */
void showtitle(char *title, char *mid);
int null_menu(void);
void movie(int i);
void domenu(int cmdmode, char *cmdtitle, int cmd, MENU *cmdtable);
#ifdef NO_SO
int all_vote(void);
#endif

/* more.c */
char *Ptt_prints(char *str, int mode);
int more(char *fpath, int promptend);
int more_web(char *fpath, int promptend);
/* name.c */
void CreateNameList(void);
void AddNameList(char *name);
int RemoveNameList(char *name);
int InNameList(char *name);
void ShowNameList(int row, int column, char *prompt);
int CountNameList(void);
void LoadNameList(int *reciper, char *listfile, char *msg);
void ToggleNameList(int *reciper, char *listfile, char *msg);
int chkstr(char *otag, char *tag, char *name);
struct word *AppearInList(struct word *cwlist, char *data);
void namecomplete(char *prompt, char *data);
void usercomplete(char *prompt, char *data);
void brdcomplete(char *prompt, char *data);
void idlist_add(char *fpath, char *desc, char *uident);
void idlist_delete(char *fpath, char *uident);
/* read.c */
keeploc *getkeep(char *s, int def_topline, int def_cursline);
void fixkeep(char *s, int first);
int Tagger(time_t chrono, int recno, int mode);
void EnumTagName(char *fname, int locus);
void EnumTagFhdr(fileheader *fhdr, char *direct, int locus);
int AskTag(char *msg);
void i_read(int cmdmode, char *direct, void (*dotitle)(void), void *(*doentry)(void), struct one_key *rcmdlist, int *num_record);
#ifdef NO_SO
int b_vote();
int b_results();
int make_vote();
#endif

/* record.c */
int get_sum_records(char *fpath, int size);
int get_records(char *fpath, char *rptr, int size, int id, int number);
//int substitute_record(char *fpath, char *rptr, int size, int id); //to be fixed
//int rec_del(char fpath[], int size, int id);  //to check: move to daorec.h
int delete_range(char *fpath, int id1, int id2);
int delete_range2(char *fpath, int id1, int id2);
int search_rec(char *dirname, int (*filecheck)(void));
int delete_files(char *dirname, int (*filecheck)(void));
int delete_file(char *dirname, int size, int ent, int (*filecheck)(void));
//int rec_search(char *fpath, char *rptr, int size, int (*fptr)(void), int farg); //to check: move to daorec.h
void stampfile(char *fpath, fileheader *fh);
void stampdir(char *fpath, fileheader *fh);
void stamplink(char *fpath, fileheader *fh);
int gem_files(char *dirname, int (*filecheck)(void));
/* register.c */
int chkpasswd(char *passwd, char *test);
int bad_user_id(char *userid);
void new_register(void);
int check_register(void);
/* screen.c */
void initscr(void);
void move(int y, int x);
void getyx(int *y, int *x);
void redoscr(void);
void refresh(void);
void clear(void);
void clrtobot(void);
void clrtoeol(void);
void clrchyiuan(int x, int y);
void outch(int c);
void outc(int ch);
void outs(register char *str);
void outmsg(register char *msg);
void outz(register char *msg);
void prints(char *fmt, ...);
void scroll(void);
void rscroll(void);
int region_scroll_up(int top, int bottom);
int region_scroll_down(int top, int bottom);
void standout(void);
void standend(void);
void vs_save(screenline *screen);
void vs_restore(screenline *screen);
/* stuff.c */
void setuserfile(char *buf, char *fname);
void setbdir(char *buf, char *boardname);
int invalid_fname(char *str);
int invalid_pname(char *str);
int valid_ident(char *ident);
int userid_is_BM(char *userid, char *list);
int is_BM(char *list);
off_t dashs(char *fname);
long dasht(char *fname);
int dashl(char *fname);
int dashf(char *fname);
int dashd(char *fname);
int belong(char *filelist, char *key);
int belong_spam(char *filelist, char *key);
char *Cdatelite(time_t *clock);
char *Cdate(time_t *clock);
void pressanykey(char *fmt, ...);
void bell(void);
int search_num(int ch, int max);
void stand_title(char *title);
void cursor_show(int row, int column);
void cursor_clear(int row, int column);
int cursor_key(int row, int column);
void printdash(char *mesg);
void update_data(void);
int show_file(char *filename, int y, int lines, int mode);
int chyiuan_ansi(char *buf, char *str, int max);
int answer(char *s);
int capture_screen(void);
int edit_note(void);
char *my_ctime(const time_t *t);
int show_help(int mode);
int mail2user(userec muser, char *title, char *fname);
void debug(char *mode);
void Security(int x, int y, char *sysopid, char *userid);
int show_hint_message(void);
/* talk.c */
char *modestring(user_info *uentp, int simple);
int cmpuids(int uid, user_info *urec);
int is_hidden(char *user);
int cmppids(pid_t pid, user_info *urec);
int is_friend(user_info *ui);
int is_rejected(user_info *ui);
int my_query(char *uident);
int my_write(pid_t pid, char *hint);
void t_display_new(void);
int t_display(void);
int pal_type(char *userid, char *whoask);
void friend_add(char *uident);
int cmpuname(char *userid, PAL *pal);
void friend_load(void);
int t_users(void);
int t_pager(void);
int t_idle(void);
int t_query(void);
void talkreply(void);
void t_aloha(void);
int count_multiplay(int unmode);
int lockutmpmode(int unmode);
int unlockutmpmode(void);
/* user.c */
void user_display(userec *u, int real);
void uinfo_query(userec *u, int real, int unum);
int u_info(void);
int u_cloak(void);
unsigned u_habit(void);
void showplans(char *uid);
int u_editfile(void);
int u_register(void);
int u_list(void);
/* term.c */
char *tparm(const char *str, ...);
void init_tty(void);
int reset_tty(void);
int restore_tty(void);
int term_init(char *term);
int do_move(int destcol, int destline);
int save_cursor(void);
int restore_cursor(void);
int change_scroll_range(int top, int bottom);
int scroll_forward(void);
int scroll_reverse(void);
/* admin.c */
int x_reg(void);
int invalid_brdname(char *brd);
unsigned setperms(unsigned pbits);
int m_newbrd(void);
int m_mod_board(char *bname);
int m_board(void);
int m_user(void);
int m_mclean(void);
int m_register(void);
int search_key_user(void);
int reload_cache(void);
/* list.c */
int belong_list(char *fname, char *userid);
//int list_add(void); //to be fixed
void listdoent(int num, fileheader *ent);
void listtitle(void);
int ListMain(void);
void ListEdit(char *fname);
