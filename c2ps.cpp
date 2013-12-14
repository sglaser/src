/*
 *
 * C2ps.c : a C to PostScript program. Does the following:
 *
 * keywords =>		bold courier or times bold
 * character string =>	courier
 * comments =>		italic courier or italic times
 * regular test =>	courier or times
 * linenumbers on lefththand side
 * procedure names on righthand side
 * date and full file name at top of page
 *
 * The C program should compile and execute on most computers. Has been
 * tested on VAX/VMS, VAX/ULTRIX, RISC/ULTRIX and MS-DOS.
 *
 * The PostScript program is an ecapsulated PostScript program and should work
 * on most printers. Has been tested on LN03R, LPS20 and
 * Digital's PostScript previewer.
 *
 * Dag Framstad @NWO          OSLENG::DAGFRA
 * 20-NOV-1989
 *
 * Modifications: Steve Glaser @ SHR	STARCH::GLASER	31 July 1990
 *                Steve Glaser @ SHR    STARCH::GLASER  12 December 1990
 */

/*
 * $Log: c2ps.cpp,v $
 * Revision 3.5  2013-09-24 18:00:36-07  sglaser
 * more updates
 *
 * Revision 3.4  2002-05-23 16:17:51-07  sglaser
 * make it compile on linux
 *
 * Revision 3.3  2002/01/09 23:29:14  sglaser
 * Remove prefix "/afs/pacelinesystems.com" from file names in listing output
 *
 * Revision 3.2  2002/01/09 23:18:23  sglaser
 * switch to C++, make it compile again
 *
 * Revision 3.1  2002/01/09 21:40:11  sglaser
 * change page trailers
 *
 * Revision 3.0  2002/01/09 21:38:52  sglaser
 * *** empty log message ***
 *
 *
 * Revision 2.6  2001/06/27 18:32:07  sglaser
 * bugfix - allow # in non C/C++ files to work properly
 * bugfix - use getcwd() to prevent buffer overflow bug
 *
 * Revision 2.5  2001/05/17 00:15:23  sglaser
 * Y2K fix
 *
 * Revision 2.4  1999/01/20 22:49:59  sglaser
 * add -verilog and -vera switches
 *
 * Revision 2.3  1998/04/07 16:05:30  sglaser
 * add C2PS_DEFAULTS to -help
 *
 * Revision 2.2  1998/04/07 16:02:48  sglaser
 * add -rotate to -help
 *
 * Revision 2.1  1998/04/07 16:00:33  sglaser
 * Initial revision
 *
 * Revision 1.22  1995/06/22  20:28:19  glaser
 * increase MAXCHARSINLINE to 10K
 *
 * Revision 1.21  1995/06/22  20:27:10  glaser
 * unknown changes
 *
 * Revision 1.20  1994/02/16  05:32:22  glaser
 * add .icc suffix defaulting to c++
 *
 * Revision 1.19  1994/01/18  23:25:16  glaser
 * include errno.h to make alpha c89 happier
 *
 * Revision 1.18  1993/12/13  19:53:55  glaser
 * s/Courier-Italic/Courier-Oblique/g (DEC PS Printers don't have Courier-Italic).
 *
 * Revision 1.17  1992/12/04  03:55:51  glaser
 * bugfix -- prepend_args() -- arrays should be static
 *
 * Revision 1.16  1992/12/02  22:51:01  glaser
 * add C2PS_DEFAULTS support, do argument debugging if argv[0] == "a.out"
 *
 * Revision 1.15  1992/10/14  18:46:39  glaser
 * add -ext switch to default language from extension
 *
 * Revision 1.14  1992/10/14  18:21:23  glaser
 * eliminate superfluous message
 *
 * Revision 1.13  1991/10/21  20:59:16  glaser
 * don't print anything for zero length files
 * (was printing a blank page 0 and getting the even/odd page count off (-2))
 *
 * Revision 1.12  1991/10/02  01:12:59  glaser
 * fix type in September
 *
 * Revision 1.11  1991/08/14  15:31:48  glaser
 * change todays_date to time_t (was long) for ANSI C compatability
 *
 * Revision 1.10  1991/08/14  15:22:40  glaser
 * att -1 -2 -4 and -8 switches to force blank pages on n-up and duplesx printers
 *
 * Revision 1.9  1991/05/10  17:27:17  glaser
 * close the input file so we don't run out of file descriptors
 *
 * Revision 1.8  91/03/27  21:15:51  glaser
 * move date up so it won't collide with very long pathnames
 * 
 * Revision 1.7  91/03/26  15:21:40  glaser
 * bug fixes with line numbers and continuation lines
 * 
 * Revision 1.6  91/03/15  20:05:55  glaser
 * fix butg - make c++ mode work
 * 
 * Revision 1.5  91/03/15  16:24:47  glaser
 * add c++ support
 * rework comment end detection to be more language independant
 * 
 * Revision 1.4  91/03/15  16:20:49  glaser
 * better error messages
 * 
 * Revision 1.3  90/12/13  01:25:37  glaser
 * add -trellis, -internal, -confidential, -restricted and -bottom
 * 
 *
 */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#ifdef VMS
#include <time.h>
#else
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdlib.h>	/* for getenv() */
#endif

#define LINEWIDTH	12
#define NORMFSIZE	10
#define SMALLFSIZE	8
#define BIGFSIZE	12
#define MAXCHARSINLINE	10000
#define BOTTOM		72
#define BOTLINE		BOTTOM - 2 * LINEWIDTH
#define LMARG		60
#define TRUE		1
#define FALSE		0

#define COMMENT_END_STAR_SLASH	1	/* C style comment */
#define COMMENT_END_NEWLINE	2	/* Trellis or C++ comment */

#ifndef MAXPATHLEN
#define MAXPATHLEN	MAXCHARSINLINE
#endif

#define LANG_C		1
#define LANG_TRELLIS	2
#define LANG_CPP	3
#define LANG_VERILOG	4
#define LANG_VERA	5

int language = LANG_CPP;		/* default is really based on suffix */
int language_set = 0;

char rcs_ident[] = "$Header: /home/sglaser/hw/pvt/sglaser/Source/RCS/c2ps.cpp,v 3.5 2013-09-24 18:00:36-07 sglaser Exp $";

static char *c_keywords[] = {
    "_align",
    "asm",
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "entry",
    "extern",
    "float",
    "for",
    "fortran",
    "globaldef",
    "globalref",
    "globalvalue",
    "goto",
    "if",
    "int",
    "long",
    "noshare",
    "readonly",
    "register",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
    0
};

static char *cpp_keywords[] = {
    "_align",
    "asm",
    "auto",
    "break",
    "case",
    "catch",
    "char",
    "class",
    "const",
    "continue",
    "default",
    "delete",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "iinline",
    "int",
    "long",
    "new",
    "operator",
    "private",
    "protected",
    "public",
    "register",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "template",
    "this",
    "throw",
    "try",
    "typedef",
    "union",
    "unsigned",
    "virtual",
    "void",
    "volatile",
    "while",
    0
};

static char *trellis_keywords[] = {
    "and",
    "cand",
    "cor",
    "div",
    "eqv",
    "in",
    "mod",
    "or",
    "xor",
    "not",
    "allocate",
    "as",
    "bind",
    "builtin",
    "component",
    "define",
    "exclude",
    "field",
    "from",
    "get",
    "inherit",
    "is",
    "operation",
    "put",
    "type_module",
    "var",
    "abstract",
    "allocate_visible",
    "base",
    "constant",
    "get_only",
    "mutable",
    "no_subtypes",
    "override",
    "private",
    "public",
    "put_only",
    "returns",
    "signals",
    "subtype_of",
    "subtype_visible",
    "yeilds",
    "begin",
    "case",
    "continue",
    "do",
    "else",
    "elseif",
    "end",
    "except",
    "exit",
    "for",
    "if",
    "leave",
    "loop",
    "on",
    "otherwise",
    "resignal",
    "return",
    "signal",
    "subtype",
    "then",
    "to",
    "type_case",
    "unless",
    "when",
    "with",
    "yield",
    0
};

static char *verilog_keywords[] = {
    "always", "and", "assign", "begin", "buf", "bufif0", "bufif1", "case",
    "casex", "casez", "cmos", "deassign", "default", "defparam", "disable",
    "edge", "else", "end", "endcase", "endmodule", "endfunction",
    "endprimitive", "endspecify", "endtable", "endtask", "event", "for",
    "force", "forever", "fork", "function", "highz0", "highz1", "if",
    "ifnone", "initial", "inout", "input", "integer", "join", "large",
    "macromodule", "medium", "module", "nand", "negedge", "nmos", "nor",
    "not", "notif0", "notif1", "or", "output", "parameter", "pmos", "posedge",
    "primitive", "pull0", "pull1", "pulldown", "rcmos", "real", "realtime",
    "reg", "release", "repeat", "rnmos", "rpnos", "rtran", "rtranif0",
    "rtranif1", "scalared", "small", "specify", "specparam", "strong0",
    "strong1", "supply0", "supply1", "table", "task", "time", "tran",
    "tranif0", "tranif1", "tri", "tri0", "tri1", "triand", "trior", "trireg",
    "vectored", "wait", "weak0", "weak1", "while", "wire", "wor", "xnor",
    "xor", 0
};

static char *vera_keywords[] = {
    "all", "any", "begin", "bind", "bind_var", "bit", "break", "breakpoint",
    "case", "class", "continue", "coverage_block", "default", "depth", "else",
    "end", "enum", "event", "extern", "extends", "for", "fork", "funciton",
    "if", "inout", "input", "integer", "interface", "join", "local",
    "negedge", "new", "none", "null", "output", "port", "posedge", "program",
    "reg", "repeat", "return", "shadow", "soft", "state", "static", "super",
    "task", "terminate", "this", "trans", "typedef", "var", "vector",
    "verilog_node", "verilog_task", "void", "while", "with", 0 };

static char *no_keywords[] = {
    0
};

/*
 * indexed by language and thus must track the defines for LANG_*
 */
char **keyword_array[] = {no_keywords,
			  c_keywords,
			  trellis_keywords,
			  cpp_keywords,
			  verilog_keywords,
			  vera_keywords };

typedef struct key_begin_end {
    char* name;
    int   offset;
} key_begin_end_t;

static key_begin_end_t no_func_start_end[] = { { 0, 0 } };
static key_begin_end_t verilog_func_start_end[] = {
    { "module",   1 }, { "endmodule",   -1 },
    { "task",	  1 }, { "endtask",     -1 },
    { "function", 1 }, { "endfunction", -1 },
    { 0, 0 } };

key_begin_end_t *func_start_end_array[] = { no_func_start_end, /* none */
					    no_func_start_end, /* C */
					    no_func_start_end, /* trellis */
					    no_func_start_end, /* C++ */
					    verilog_func_start_end, /* Verilo	g */
					    no_func_start_end }; /* Vera */

struct paper_sizes {
    char *name;
    int   x;
    int   y;
} paper_sizes[] = {
    {"-letter",	612,	792},
    {"-a3",	594,	846},
    {"-a4",	846,   1184},
    {"-legal",	612,   1108},
    {"-ledger",	792,   1224}};

#define MAX_PAPER_SIZE 4	/* -ledger */

char   *argv0,
       *bottom_text = 0,
       *header_string = 0,
        ifname[120],
        ofname[120],
        ifname_full[MAXPATHLEN],
        ibuffer[MAXCHARSINLINE],
        obuffer[MAXCHARSINLINE],
        tmpbuffer[MAXCHARSINLINE],
        timbuf[50],
        cword[120],
        curfuncs[120],
        funcname[120],
        txtchar = ' ';

int     paper_size = 0,
	fixed_font = FALSE,
        rotate_text = FALSE,
        txtmode,
	process_mode,
        moreonline ,
        comment_style,
        lastwasbslash,
        have_funcname,
        top_of_page,
	seen_directive,
	seen_non_blank;

int     i,
	j,
        x,
        urx,
        ury,
        top,
        topline,
        pageno = 0,
        page_skip = 1,
	pagecount = 0,
        duplex = 0,
        lineno = 1,
        func_depth = 0,
        paren_depth = 0,
        square_bracket_depth = 0,
        func_name_search = 0,
        ypos,
        rmarg,
	wrap_col,
        ibuffp,
        obuffp,
        cwordp;

FILE *infile = NULL,
     *outfile = NULL;

int     IsKeyword(),
        IsItAFunc();

void Usage(),
     MakePaperSize(),
     MakeProlog(),
     PrintPage(),
     MakeNewPage(),
     PrintBlankPage(),
     WriteBuffer(),
     WriteFont(),
     WriteLineNo(),
     WhatToPutIn(),
     PutWordInBuffer(),
     WasKeyword(),
     WasNotKeyword(),
     WasAFunc(),
     PutCharInWord(),
     InComMode(),
     StartComMode(),
     StopTxtMode(),
     InTxtMode(),
     StartTxtMode(),
     InFileMode(),
     ParseFile(),
     MakeTrailer(),
     ResetForNewFile(),
     ResetTimbuf();

void print_args(const char* tag, int argc, char** argv)
{
    int i;
    if (strcmp(argv0, "a.out") == 0) {
	printf("%s: argc = %d\n", tag, argc);
	for (i = 0; i < argc; i++)
	    printf("argc[%d] = \"%s\"\n", i, argv[i]);
    }
}

void
prepend_args(int* argc_ptr, char*** argv_ptr)
{
#ifndef VMS
    static char* new_argv[128];
    static char argbuff[1024];
    int count;

    count = 1;
    new_argv[0] = (*argv_ptr)[0];

    print_args("prepend_args::before", *argc_ptr, *argv_ptr);
    {
	char* p = argbuff;
	char* q = getenv("C2PS_DEFAULTS");
	
	if (q == NULL)
	    return;

	if (strlen(q) >= sizeof(argbuff)) {
	    fprintf(stderr, "c2ps: C2PS_DEFAULTS longer than %d\n", sizeof(argbuff));
	    Usage();
	}

	do {
	    char* saved_p = p;
	    while (isgraph(*q)) {	/* printable, not including space */
		*p++ = *q++;
	    }
	    *p++ = '\0';
	    
	    new_argv[count++] = saved_p;
	    new_argv[count] = NULL;
	    
	    while (isspace(*q)) {
		q++;
	    }
	} while (*q != '\0');
    }
    print_args("prepend_args::middle 1", count, new_argv);
    {
	char** argv = *argv_ptr+1;
	while (*argv != NULL) {
	    new_argv[count++] = *argv++;
	}
	new_argv[count] = NULL;
    }
    print_args("prepend_args::middle 2", count, new_argv);
    *argv_ptr = new_argv;
    *argc_ptr = count;
    print_args("prepend_args::after", *argc_ptr, *argv_ptr);
#endif
}


/*
 * main entry point
 */
int
main(int argc, char** argv)
{
    char    name[80];
    char   *dotpos;
    int	    found_file_name;

    //    extern char *strrchr();

    argv0 = argv[0];

    print_args("main::before", argc, argv);
    prepend_args(&argc, &argv);
    print_args("main::after", argc, argv);

    if (argc <= 1)
	Usage();

    found_file_name = FALSE;
    ofname[0] = '\0';

    for (i = 1; i < argc; i++) {
	if ((argv[i][0] == '-') && (argv[i][1] != '\0')) {
	    if (found_file_name == FALSE) {
		/*
		 * process options that must preceed any input file names
		 */
		for (j = 0; j <= MAX_PAPER_SIZE; j++) {
		    if (strcmp (argv[i], paper_sizes[j].name) == 0) {
			paper_size = j;
			goto next_option;
		    }
		}
		if ((strcmp(argv[i], "-o") == 0) && ((i+1) < argc)) {
		    i++;
		    strcpy(ofname, argv[i]);
		    goto next_option;
		}
	    }

	    /*
	     * process other options
	     */
	    if ((strcmp(argv[i], "-hdr") == 0) && ((i+1) < argc)) {
		i++;
		header_string = argv[i];
	    } else if (strcmp(argv[i], "-text") == 0) {
		process_mode = 1;
		goto next_option;
	    } else if (strcmp(argv[i], "-c") == 0) {
		process_mode = 0;
		language = LANG_C;
		language_set = 1;
		goto next_option;
	    } else if (strcmp(argv[i], "-trellis") == 0) {
		process_mode = 0;
		language = LANG_TRELLIS;
		language_set = 1;
		goto next_option;
	    } else if (strcmp(argv[i], "-c++") == 0) {
		process_mode = 0;
		language = LANG_CPP;
		language_set = 1;
		goto next_option;
	    } else if (strcmp(argv[i], "-verilog") == 0) {
		process_mode = 0;
		language = LANG_VERILOG;
		language_set = 1;
		goto next_option;
	    } else if (strcmp(argv[i], "-vera") == 0) {
		process_mode = 0;
		language = LANG_VERA;
		language_set = 1;
		goto next_option;
	    } else if (strcmp(argv[i], "-ext") == 0) {
		process_mode = 0;
		language = LANG_CPP;
		language_set = 0;
		goto next_option;
	    } else if (strcmp(argv[i], "-internal") == 0) {
		bottom_text = "Nvidia Internal Use Only";
		goto next_option;
	    } else if (strcmp(argv[i], "-confidential") == 0) {
		bottom_text = "Nvidia Confidential";
		goto next_option;
	    } else if (strcmp(argv[i], "-restricted") == 0) {
		bottom_text = "Nvidia Restricted Distribution";
		goto next_option;
	    } else if ((strcmp(argv[i], "-bottom") == 0) && i+1 < argc) {
		i++;
		bottom_text = argv[i];
		goto next_option;
	    } else if (strcmp(argv[i], "-proportional") == 0) {
		fixed_font = FALSE;
		goto next_option;
	    } else if (strcmp(argv[i], "-fixed") == 0) {
		fixed_font = TRUE;
		goto next_option;
	    } else if (strcmp(argv[i], "-rotate") == 0) {
		rotate_text = TRUE;
		goto next_option;
	    } else if (strcmp(argv[i], "-1") == 0) {
		page_skip = 1;
		goto next_option;
	    } else if (strcmp(argv[i], "-2") == 0) {
		page_skip = 2;
		goto next_option;
	    } else if (strcmp(argv[i], "-4") == 0) {
		page_skip = 4;
		goto next_option;
	    } else if (strcmp(argv[i], "-8") == 0) {
		page_skip = 8;
		goto next_option;
	    } else if (strcmp(argv[i], "-duplex") == 0) {
	        duplex = 1;
	    } else {
		Usage();
	    }

	} else {

	    if (ofname[0] == '\0') {
		strcpy(ofname, argv[i]);
		dotpos = strrchr(ofname, '.');
		if (dotpos != NULL)
		    *dotpos = '\0';
		strcat(ofname, ".ps");
	    }
	
	    if (outfile == NULL) {
		if ((strcmp(ofname,"-") == 0) || 
		    (strcmp(ofname, "-.ps") == 0)) {
		    outfile = stdout;
		} else {
		    if ((outfile = fopen(ofname, "w+")) == NULL) {
#ifdef VMS
			fprintf(stderr, "%s: can't open '%s'\n", argv0, ofname);
#else
			fprintf(stderr, "%s: can't open '%s' %s\n", argv0, ofname, strerror(errno));
#endif
			exit(1);
		    }
		}
		MakePaperSize();
		MakeProlog();
	    }

	    found_file_name = TRUE;
	    strcpy(ifname, argv[i]);

#ifdef VMS
	    strcpy(ifname_full, ifname);
#else
	    if (ifname[0] == '/') {
		strcpy(ifname_full, ifname);
	    } else if (ifname[0] == '-') {
		strcpy(ifname_full, "standard input");
	    } else {
		getcwd(ifname_full, sizeof(ifname_full) - 2 - strlen(ifname));
		strcat(ifname_full, "/");
		strcat(ifname_full, ifname);
	    }
#endif
	    if (ifname[0] == '-') {
		infile = stdin;
	    } else if ((infile = fopen(ifname, "r")) == NULL) {
#ifdef VMS
		fprintf(stderr, "%s : can't open '%s'\n", argv0, ifname);
#else
		fprintf(stderr, "%s : can't open '%s' %s\n", argv0, ifname, strerror(errno));
#endif
		exit(1);
	    }

	    if (language_set == 0) {
		dotpos=strrchr(ifname, '.');
		if (dotpos != NULL) {
		    if ((strcmp(dotpos, ".c") == 0) ||
		        (strcmp(dotpos, ".h") == 0))
		    {
			process_mode = 0;
			language = LANG_C;
		    }
		    else if ((strcmp(dotpos, ".cxx") == 0) ||
			     (strcmp(dotpos, ".hxx") == 0) ||
			     (strcmp(dotpos, ".icc") == 0) ||
			     (strcmp(dotpos, ".cpp") == 0) ||
			     (strcmp(dotpos, ".hpp") == 0) ||
			     (strcmp(dotpos, ".C") == 0) ||
			     (strcmp(dotpos, ".H") == 0) ||
			     (strcmp(dotpos, ".cc") == 0) ||
			     (strcmp(dotpos, ".hh") == 0) ||
			     (strcmp(dotpos, ".CC") == 0) ||
			     (strcmp(dotpos, ".HH") == 0))
		    {
			process_mode = 0;
		        language = LANG_CPP;
		    }
		    else if ((strcmp(dotpos, ".verilog") == 0) ||
			     (strcmp(dotpos, ".v") == 0) ||
			     (strcmp(dotpos, ".vh") == 0) ||
			     (strcmp(dotpos, ".vs") == 0))
		    {
			process_mode = 0;
			language = LANG_VERILOG;
		    }
		    else if ((strcmp(dotpos, ".vr") == 0) ||
			     (strcmp(dotpos, ".vrh") == 0))
		    {
			process_mode = 0;
			language = LANG_VERA;
		    }
		    else if ((strcmp(dotpos, ".trellis") == 0))
		    {
			process_mode = 0;
			language = LANG_TRELLIS;
		    } else {
			/* -text */
			process_mode = 1;
		    }
		} else {
		    /* -text */
		    process_mode = 1;
		}
	    }
	    ResetTimbuf();
	    ParseFile();
	    fclose(infile);
	    infile = NULL;

	}

    next_option:
	if (i >= argc)
	    Usage();
    }
    if (outfile != NULL)
	MakeTrailer();
    exit(0);
}



/*
 * Check if kword is a reserved word
 */
int     IsKeyword(char* kword)
{
    char ** ptr = NULL;
    int     iskey = FALSE;


    switch (language) {

    case LANG_CPP:
    case LANG_C:
    case LANG_VERA:

	/*
	 * keywords can't occur on preprocessor lines
	 */
	if ((obuffp > 0) && (obuffer[obuffp - 1] == '#'))
	    break;

	/* fall through */

    case LANG_TRELLIS:
    case LANG_VERILOG:

	/*
	 * search the appropriate keyword array
	 */
	for (ptr = keyword_array[language]; *ptr != NULL; ptr++) {
	    if (strcmp(*ptr, kword) == 0) {
		iskey = TRUE;
		break;
	    }
	}
	break;

    }
    return iskey;
}



/*
 * Print usage help text when giving wrong command
 */
void Usage() {
    fprintf(stderr, "usage: %s\t[-text | -trellis | -c | -c++ | -verilog]\n", argv0);
    fprintf(stderr, "\t\t[-proportional | -fixed] [-o outputfile]\n");
    fprintf(stderr, "\t\t[-letter | -a3 | -a4 | -legal | -ledger]\n");
    fprintf(stderr, "\t\t[-internal | -confidential | -restricted | -bottom string] \n");
    fprintf(stderr, "\t\t[-duplex] [-rotate] [-1 | -2 | -4 | -8] files\n");
    fprintf(stderr, "default: %s -c -proportional -letter (modified by environment variable C2PS_DEFAULTS)\n", argv0);
    exit(1);
}


/*
 * Set new font
 *
 *   1:  Font for ordinary text
 *   2:  Font for keywords
 *   3:  Font for texts
 *   4:  Font for comments
 *   5:  Font for linenumbers
 *   6:  Font for bottomtext
 *   7:  Font for toptext
 *   8:  Font for procedures
 *   9:  Font for page numbers
 *   10: Font for text files
 *   default: What font?
 */
void WriteFont(int fn)
{
    switch (fn) {
    case 1:
	    fprintf(outfile, "ordfn ");
	    break;
	case 2:
	    fprintf(outfile, "keyfn ");
	    break;
	case 3:
	    fprintf(outfile, "txtfn ");
	    break;
	case 4:
	    fprintf(outfile, "comfn ");
	    break;
	case 5:
	    fprintf(outfile, "linfn ");
	    break;
	case 6:
	    fprintf(outfile, "botfn ");
	    break;
	case 7:
	    fprintf(outfile, "topfn ");
	    break;
	case 8:
	    fprintf(outfile, "prcfn ");
	    break;
	case 9:
	    fprintf(outfile, "pagfn ");
	    break;
	case 10:
	    fprintf(outfile, "filfn ");
	    break;
	default:
	    if (process_mode == 1)	 WriteFont(10);
	    else if (txtmode)		 WriteFont(3);
	    else if (comment_style != 0) WriteFont(4);
	    else			 WriteFont (1);
    }
}

/*
 * Define the size of the PostScript BoundingBox depending on the papersize
 */
void MakePaperSize()
{
    if (rotate_text) {
	urx = paper_sizes[paper_size].y;
	ury = paper_sizes[paper_size].x;
    } else {
	urx = paper_sizes[paper_size].x;
	ury = paper_sizes[paper_size].y;
    }

    ypos = top = ury - NORMFSIZE - 90;
    topline = top + 2 * LINEWIDTH;
    rmarg = urx - 36;
    /*
     * 4.65 points per character (8 pt courier).
     * round down to the nearest multiple of 8 characters
     */
    wrap_col = ((((rmarg - LMARG) * 100) / 465) / 8) * 8;
}


static char *month[12] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December" };

static char *wday[7] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday" };

time_t todays_date;

/*
 * set the time buffer to the file timestamp or today's date
 */
void ResetTimbuf() {
    struct tm  *lt;
#ifndef VMS
    struct stat statb;
#endif

#ifdef VMS
    lt = localtime(&todays_date);
#else
    if (infile == NULL) {
	lt = localtime(&todays_date);
    } else {
	if (fstat(fileno(infile), &statb) != 0) {
#ifdef VMS
	    perror(argv0);
#else
	    fprintf(stderr, "%s: on '%s' #1 can't fstat(%d): %s\n", argv0, ifname, fileno(infile), strerror(errno));
#endif
	}
	lt = localtime(&statb.st_mtime);
    }
#endif
    if ((lt->tm_min == 0) && (lt->tm_sec == 0)
      && ((lt->tm_hour == 0) || (lt->tm_hour == 12))) {
	sprintf(timbuf, "%s %d %s %04d at %s",
		 wday[lt->tm_wday],
		 lt->tm_mday,
		 month[lt->tm_mon],
		 lt->tm_year + 1900,
		 lt->tm_hour == 0 ? "12 midnight" : "12 noon");
    } else {
	sprintf(timbuf, "%s  %d %s %04d at %02d:%02d:%02d %s",
		 wday[lt->tm_wday],
		 lt->tm_mday,
		 month[lt->tm_mon],
		 lt->tm_year + 1900,
		 ((lt->tm_hour == 0) ? 12:
		  (lt->tm_hour > 12) ? lt->tm_hour-12 : lt->tm_hour),
		 lt->tm_min,
		 lt->tm_sec,
		 lt->tm_hour < 12 ? "AM" : "PM");
    }
}

/*
 * emit the PostScript Prolog
 */
void MakeProlog() {
    struct tm *lt;

    time(&todays_date);
    lt = localtime(&todays_date);

    fprintf(outfile, "%%!PS-Adobe-2.0 EPSF-2.0\n");
    fprintf(outfile, "%%%%BoundingBox: 0 0 %d %d\n", urx, ury);
    fprintf(outfile, "%%%%DocumentFonts: Courier");
    if (fixed_font) {
	fprintf(outfile, " Courier-Oblique");
	fprintf(outfile, " Courier-Bold");
    } else {
	fprintf(outfile, " Times-Italic");
	fprintf(outfile, " Times-Bold");
	fprintf(outfile, " Times-Roman");
    }
    fprintf(outfile, " Helvetica-Oblique\n");
    fprintf(outfile, "%%%%Title: %s\n", ofname);
    fprintf(outfile, "%%%%Creator: %s %s\n", argv0, rcs_ident);
    fprintf(outfile, "%%%%CreationDate: %s %s %d %02d:%02d:%02d %d\n",
	    wday[lt->tm_wday], month[lt->tm_mon], lt->tm_mday,
	    lt->tm_hour, lt->tm_min, lt->tm_sec, lt->tm_year + 1900);
    fprintf(outfile, "%%%%Pages: (atend)\n");
    fprintf(outfile, "%%%%EndComments\n");

    /* define the newfont procedure, stack: fontsize font */
    fprintf(outfile, "/nf {findfont exch scalefont setfont} def\n");
    /* define other the fonts procedures */
    if (fixed_font) {
	fprintf(outfile, "/keyfn {%d /Courier-Bold nf} def\n", NORMFSIZE);
	fprintf(outfile, "/ordfn {%d /Courier nf} def\n", NORMFSIZE);
	fprintf(outfile, "/comfn {%d /Courier-Oblique nf} def\n", NORMFSIZE);
    } else {
	fprintf(outfile, "/keyfn {%d /Times-Bold nf} def\n", NORMFSIZE);
	fprintf(outfile, "/ordfn {%d /Times-Roman nf} def\n", NORMFSIZE);
	fprintf(outfile, "/comfn {%d /Times-Italic nf} def\n", NORMFSIZE);
    }
    fprintf(outfile, "/txtfn {%d /Courier nf } def\n", NORMFSIZE);
    fprintf(outfile, "/filfn {%d /Courier nf } def\n", SMALLFSIZE);
    fprintf(outfile, "/linfn {%d /Helvetica-Oblique nf} def\n", SMALLFSIZE);
    fprintf(outfile, "/botfn {%d /Helvetica-Oblique nf} def\n", BIGFSIZE);
    fprintf(outfile, "/topfn {%d /Helvetica-Oblique nf} def \n", BIGFSIZE);
    fprintf(outfile, "/prcfn {%d /Helvetica-Oblique nf} def \n", BIGFSIZE);
    fprintf(outfile, "/pagfn {%d /Helvetica-Oblique nf} def \n", 2 * BIGFSIZE);
    /* define the show procedure */
    fprintf(outfile, "/s /show load def\n");
    /* define the rightshow procedure, stack: string */
    fprintf(outfile, "/rs {dup stringwidth pop neg 0 rmoveto s} def\n");
    /* define the centershow procedure, stack: string */
    fprintf(outfile, "/cs {dup stringwidth pop neg 2 div 0 rmoveto s} def\n");
    /* define the moveto procedure */
    fprintf(outfile, "/m /moveto load def\n");
    /* define the lineto  procedure */
    fprintf(outfile, "/l {newpath moveto lineto stroke} def\n");
    WriteFont(1);
    if (duplex) {
      fprintf(outfile, "<< /Duplex true >> setpagedevice\n");
    }
    if (rotate_text) {
	/* coordinates were swapped earlier so this is really urx */
	fprintf(outfile, "%d 0 translate 90 rotate\n", ury);
    }
    fprintf(outfile, "\n%%%%EndProlog\n");
}



/*
 * print the current page
 */
void PrintPage() {

    if (bottom_text != 0) {
	/*
	 * bottom of page text
	 */
	WriteFont(6);
	fprintf(outfile, "%d %d m ", rmarg, BOTLINE);
	fprintf(outfile, "(%s)rs\n", bottom_text);
    }

    /*
     * top of page text
     */
    WriteFont(7);
    fprintf(outfile, "%d %d m (%s)s ", LMARG, topline+LINEWIDTH+4, timbuf);
#define AFS_PREFIX "/xyzzy/"
    if (strncmp(ifname_full, AFS_PREFIX, strlen(AFS_PREFIX)) == 0)
      fprintf(outfile, "%d %d m (%s)rs\n", rmarg, topline, &ifname_full[strlen(AFS_PREFIX)-strlen("/home/")]);
    else
      fprintf(outfile, "%d %d m (%s)rs\n", rmarg, topline, ifname_full);
    WriteFont(9);
    fprintf(outfile, "%d %d m (%d)rs\n", rmarg, topline+LINEWIDTH+4, pageno);
    /*
     * top of page separator line
     */
    fprintf(outfile, "%d %d %d %d l\n", LMARG-4, topline-4, rmarg+4, topline-4);
    /*
     * left vertical rule
     */
    fprintf(outfile, "%d %d %d %d l\n", LMARG-4, BOTLINE, LMARG-4, topline+BIGFSIZE+BIGFSIZE+4);

    curfuncs[0] = '\0';		/* not currently using this, but... */
    fprintf(outfile, "showpage\n");
    if (rotate_text) {
	/* coordinates were swapped earlier so this is really urx */
	fprintf(outfile, "%d 0 translate 90 rotate\n", ury);
    }
}



/*
 * set up a new page
 */
void MakeNewPage() {
    pageno++;
    pagecount++;
    top_of_page = TRUE;
    fprintf(outfile, "%%%%Page: %d %d\n", pagecount, pagecount);
    /*
     * if in the middle of a function, print continuation name
     */
    if (func_depth > 0 && have_funcname == FALSE) {
	WriteFont(8);
	fprintf(outfile, "%d %d m (...%s)rs\n", rmarg, top, funcname);
    }
    WriteFont(0);
}

void PrintBlankPage() {

    /* from MakeNewPage() */
    pageno++;
    pagecount++;
    top_of_page = TRUE;
    fprintf(outfile, "%%%%Page: %d %d\n", pagecount, pagecount);

    /* from PrintPage() */
    if (bottom_text != 0) {
	/*
	 * bottom of page text
	 */
	WriteFont(6);
	fprintf(outfile, "%d %d m ", rmarg, BOTLINE);
	fprintf(outfile, "(%s)rs\n", bottom_text);
    }

    /*
     * center of page text
     */
    WriteFont(7);
    fprintf(outfile, "%d %d m (This Page Intentionally Blank)cs ",
	    LMARG + (rmarg-LMARG)/2, BOTLINE + (topline - BOTLINE)/2);

    /*
     * top of page separator line
     */
    fprintf(outfile, "%d %d %d %d l\n", LMARG-4, topline-4, rmarg+4, topline-4);
    /*
     * left vertical rule
     */
    fprintf(outfile, "%d %d %d %d l\n", LMARG-4, BOTLINE, LMARG-4, topline+BIGFSIZE+BIGFSIZE+4);

    curfuncs[0] = '\0';		/* not currently using this, but... */
    fprintf(outfile, "showpage\n");
    if (rotate_text) {
	/* coordinates were swapped earlier so this is really urx */
	fprintf(outfile, "%d 0 translate 90 rotate\n", ury);
    }
}


/*
 * write the output buffer to the file
 */
void WriteBuffer() {
    obuffer[obuffp] = '\0';
    fprintf(outfile, "(%s)s\n", obuffer);
    obuffp = 0;
}



/*
 * write the linenumber on the right side
 */
void WriteLineNo(int ln)
{
    if (ypos > BOTTOM) {
	fprintf(outfile, "%d %d m ", LMARG - 8, ypos);
	WriteFont(5);
	fprintf(outfile, "(%d)rs\n", ln);
	WriteFont(0);
    }
}


#define PutCharInBuffer(c) obuffer[obuffp++] = (c)

/*
 * put characters in output buffer escaping magic postscript characters
 * and supressing nul, newline, return and formfeed
 */
void WhatToPutIn(char c)
{
    int     tmpi,
            tmpj;

    switch (c) {

	case '\\':
	case '(':
	case ')':
	    PutCharInBuffer('\\');
	    PutCharInBuffer(c);
	    x++;
	    break;

	case '\t':
	    tmpi = ((x / 8) + 1) * 8 - x;
	    for (tmpj = tmpi; tmpj > 0; tmpj--)
		PutCharInBuffer (' ');
	    x += tmpi;
	    break;

	case '\0':
	case '\n':
	case '\f':
	case '\r':
	    break;

	default:
	    PutCharInBuffer(c);
	    x++;
	    break;
    }
}



/*
 * put the current word in the output buffer
 */
void PutWordInBuffer() {
    obuffer[obuffp] = '\0';
    strcat(obuffer, cword);
    obuffp += strlen(cword);
    x += strlen(cword);
    cwordp = 0;
}



/*
 * The last word was a keyword, switch fonts and emit it.
 */
void WasKeyword() {
    WriteBuffer();
    WriteFont(2);
    fprintf(outfile, "(%s)s ", cword);
    WriteFont(1);
    fprintf(outfile, "\n");
/*    WhatToPutIn(ibuffer[ibuffp]); */
    cwordp = 0;
}


/*
 * return true if the current line begins a function
 */
int IsItAFunc(int comment,	/* style of comment we're in (0 in not in a comment) */
	      int tmp, 		/* lookahead pointer into temporary buffer */
	      int par,		/* number of unclosed parens we have seen */
	      int seen,		/* number of lines looked ahead */
	      int lines_seen)	/* true if we have seen an open paren */
{
    long ptrpos;

    /*
     * bail out if function prolog too long
     */
    if (lines_seen > 20)
	return FALSE;

    while (tmp < MAXCHARSINLINE) {
	if (comment != 0) {
	    switch (tmpbuffer[tmp]) {
	    case '*':
		if (comment == COMMENT_END_STAR_SLASH) {
		    if (tmpbuffer[tmp + 1] == '/') {
			comment = 0;
			tmp++;
		    }
		}
		break;
	    case '\n':
	    case '\f':
	    case '\r':
	    case '\0':
		if (comment == COMMENT_END_NEWLINE) {
		    comment = 0;
		}
		ptrpos = ftell(infile);
		if ((fgets(tmpbuffer, MAXCHARSINLINE, infile)) == NULL
		    && ferror(infile) != 0) {
#ifdef VMS
		    perror(argv0);
#else
		    fprintf(stderr, "%s: on '%s' #2 can't fgets(infile): %s\n", argv0, ifname, strerror(errno));
#endif
		    return FALSE;
		}
		if (IsItAFunc(comment, 0, par, seen, lines_seen+1)) {
		    if (fseek(infile, ptrpos, 0) != 0) {
#ifdef VMS
			perror(argv0);
#else
			fprintf(stderr, "%s: on '%s' #3 can't fseek to %d: %s\n", argv0, ifname, ptrpos, strerror(errno));
#endif
		    }
		    return TRUE;
		} else {
		    if (fseek(infile, ptrpos, 0) != 0) {
#ifdef VMS
			perror(argv0);
#else
			fprintf(stderr, "%s: on '%s' #4 can't fseek to %d: %s\n", argv0, ifname, ptrpos, strerror(errno));
#endif
		    }
		    return FALSE;
		}
	    }
	} else {

#define DEFAULT_ACTION { if (!seen) return FALSE; else if (par == 0 && seen) return TRUE; }

	    switch (tmpbuffer[tmp]) {
	    case '/':
		if ((language == LANG_C) || (language == LANG_CPP) || (language == LANG_VERILOG) || (language == LANG_VERA)) {
		    if (tmpbuffer[tmp + 1] == '/') {
			comment = COMMENT_END_NEWLINE;
			tmp++;
		    } else if (tmpbuffer[tmp + 1] == '*') {
			comment = COMMENT_END_STAR_SLASH;
			tmp++;
		    } else {
			return FALSE;
		    }
		} else DEFAULT_ACTION;
		break;
		
	    case '!':
		if (language == LANG_TRELLIS) {
		    comment = COMMENT_END_NEWLINE;
		} else DEFAULT_ACTION;
		break;
		
	    case '(':
		seen = TRUE;
		par++;
		break;

	    case ')':
		par--;
		break;

	    case ';':
	    case ',':
		if (par == 0)
		    return FALSE;
		break;

	    case '\n':
	    case '\f':
	    case '\r':
	    case '\0':
		ptrpos = ftell(infile);
		if ((fgets(tmpbuffer, MAXCHARSINLINE, infile)) == NULL) {
		    return FALSE;
#if 0
#ifdef VMS
		    perror(argv0);
#else
		    fprintf(stderr, "%s: on '%s' #5 can't fgets, ptrpos=%d: %s\n", argv0, ifname, ptrpos, strerror(errno));
#endif
#endif
		}
		if (IsItAFunc(comment, 0, par, seen, lines_seen+1)) {
		    if (fseek(infile, ptrpos, 0) != 0) {
#ifdef VMS
			perror(argv0);
#else
			fprintf(stderr, "%s: on '%s' #6 can't fseek to %d: %s\n", argv0, ifname, ptrpos, strerror(errno));
#endif
		    }
		    return TRUE;
		} else {
		    if (fseek(infile, ptrpos, 0) != 0) {
#ifdef VMS
			perror(argv0);
#else
			fprintf(stderr, "%s: on '%s' #7 can't fseek to %d: %s\n", argv0, ifname, ptrpos, strerror(errno));
#endif
		    }
		    return FALSE;
		}

	    case ' ':
	    case '\t':
		break;

	    default:
		DEFAULT_ACTION;
		break;
	    }
	}
	tmp++;
    }
}
#undef DEFAULT_ACTION

/*
 * last word was a function name, remember is so it can be written on
 * the right-hand side of the page
 */
void WasAFunc() {
    //printf("function: %s, %d->%d\n", cword, have_funcname, TRUE);
    strcpy(funcname, cword);
    have_funcname = TRUE;
    if (curfuncs[0] == '\0')
	strcpy(curfuncs, funcname);
    else
	if (strlen(curfuncs) < 80) {
	    strcat(curfuncs, "  ");
	    strcat(curfuncs, funcname);

	}
}



/*
 * the last word was not a keyword, check if it was a function
 */
void WasNotKeyword() {
    if (func_depth == 0 && seen_directive == FALSE) {
	strcpy(tmpbuffer, ibuffer);
	if (IsItAFunc(FALSE, ibuffp, 0, FALSE, 0))
	    WasAFunc();
    }
    PutWordInBuffer();
}




/*
 * append the current input character to the current word buffer
 */
void PutCharInWord() {
    cword[cwordp++] = ibuffer[ibuffp];
    cword[cwordp] = '\0';
}



/*
 * process the file in comment mode
 */
void InComMode() {
    switch (ibuffer[ibuffp]) {
	case '*':
	    if (comment_style == COMMENT_END_STAR_SLASH) {
		if (ibuffer[ibuffp + 1] == '/') {
		    WhatToPutIn(ibuffer[ibuffp++]);
		    WhatToPutIn(ibuffer[ibuffp]);
		    WriteBuffer();
		    WriteFont(1);
		    comment_style = 0;
		}
		else
		    WhatToPutIn(ibuffer[ibuffp]);
	    }
	    else
		WhatToPutIn(ibuffer[ibuffp]);
	    break;
	case '\f':
	case '\r':
	case '\n':
	case '\0':
	    if (obuffp > 0)
		WriteBuffer();
	    if (comment_style == COMMENT_END_NEWLINE) {
		WriteFont(1);
		comment_style = 0;
	    }
	    moreonline = FALSE;
	    if (ibuffer[ibuffp] == '\f')
		ypos = 0;
	    break;
	default:
	    WhatToPutIn(ibuffer[ibuffp]);
	    break;
    }
}



/*
 * enter comment mode
 */
void StartComMode(int style, int is_two_char)
{
    comment_style = style;
    WriteBuffer();
    WriteFont(4);
    if (is_two_char) {
	WhatToPutIn(ibuffer[ibuffp++]);     /* 2 char comment begin */
    }
    WhatToPutIn(ibuffer[ibuffp]);
}


/*
 * process the file in file mode
 */
void InFileMode() {
    switch (ibuffer[ibuffp]) {
	case '\f':
	case '\r':
	case '\n':
	case '\0':
	    if (obuffp > 0)
		WriteBuffer();
	    moreonline = FALSE;
	    if (ibuffer[ibuffp] == '\f')
		ypos = 0;
	    break;
	default:
	    if (x >= wrap_col) {
		WhatToPutIn('\\');
		WriteBuffer();
		ypos -= LINEWIDTH;
		if (ypos < BOTTOM) {
		    PrintPage();
		    MakeNewPage();
		    ypos = top;
		}
		fprintf(outfile, "%d %d m ", LMARG, ypos);
		x = 0;
	    }
	    WhatToPutIn(ibuffer[ibuffp]);
	}
}

/*
 * enter text mode
 */
void StopTxtMode() {
    WhatToPutIn(ibuffer[ibuffp]);
    WriteBuffer();
    WriteFont(1);
    txtmode = FALSE;
}


/*
 * process the file in text mode
 */
void InTxtMode() {
    switch (ibuffer[ibuffp]) {
    case '\'':
    case '\"':
	if (ibuffer[ibuffp] == txtchar && ibuffp > 0 && !lastwasbslash)
	    StopTxtMode();
	else
	    WhatToPutIn(ibuffer[ibuffp]);
	lastwasbslash = FALSE;
	break;
    case '\f':
    case '\r':
    case '\n':
    case '\0':
	if (obuffp > 0)
	    WriteBuffer();
	moreonline = FALSE;
	if (ibuffer[ibuffp] == '\f')
	    ypos = 0;
	lastwasbslash = FALSE;
	break;
    default:
	if (!lastwasbslash && ibuffer[ibuffp] == '\\')
	    lastwasbslash = TRUE;
	else
	    lastwasbslash = FALSE;
	WhatToPutIn(ibuffer[ibuffp]);
    }
}



/*
 * enter text mode
 */
void StartTxtMode() {
    txtchar = ibuffer[ibuffp];
    txtmode = TRUE;
    WriteBuffer();
    WriteFont(3);
    WhatToPutIn(ibuffer[ibuffp]);

}



/*
 * parse the input file
 */
void ParseFile()
{
    int is_word_char;

    pageno = 0;
    lineno = 1;
    func_depth = 0;
    txtmode = FALSE;
    txtchar = ' ';
    moreonline = TRUE;
    comment_style = 0;
    lastwasbslash = FALSE;
    have_funcname = FALSE;
    seen_directive = FALSE;
    seen_non_blank = FALSE;

    while (fgets(ibuffer, MAXCHARSINLINE, infile) != NULL) {

	/* check if this line is empty or not */
	if (ibuffer[0] != '\n') {	/* not empty line */

	    if (pageno == 0) {		/* the first page */
		MakeNewPage();
		ypos = top;
	    }

	    if (ypos < BOTTOM) {	/* not enough space on */
		PrintPage();		/* this page print it and */
		MakeNewPage();		/* make a new one   */
		ypos = top;
	    }

	    /* move to the right position */
	    fprintf(outfile, "%d %d m ", LMARG, ypos);
	}
	else			/* empty line */
	    moreonline = FALSE;


	/* examine each charater if the line is not empty */
	for (cwordp = ibuffp = obuffp = x = 0, moreonline = TRUE;
		moreonline; ibuffp++) {

	    if (process_mode == 1)	    /* processing a text file */
		InFileMode();

	    else if (txtmode)    	    /* we are printing text */
		InTxtMode();

	    else if (comment_style != 0)   /* we are printing comments */
		InComMode();

	    else			/* other text */
		switch (ibuffer[ibuffp]) {
		case '\n':
		case '\0':
		case '\r':
		case '\f':
		    if (ibuffp > 0) {
			if (cwordp > 0) {
			    if (IsKeyword(cword))
				WasKeyword();
			    else
				WasNotKeyword();
			    WhatToPutIn(ibuffer[ibuffp]);
			}
		    }
		    moreonline = FALSE;
		    if (ibuffer[ibuffp] == '\f') {
			ypos = 0;
		    }
		    /*
		     * check for continuation line
		     */
		    switch (language) {
		    case LANG_C:
		    case LANG_CPP:
			/*
			 * handle C style continuation lines (\newline)
			 */
			if (! (ibuffp > 0
			       && ibuffer[ibuffp] == '\n'
			       && ibuffer[ibuffp-1] == '\\'
			       )) {
			    seen_directive = FALSE;
			    seen_non_blank = FALSE;
			}
		    case LANG_TRELLIS:
			/*
			 * handle trellis continuation lines (TBD)
			 */
			break;
		    default:
			break;
		    }
		    break;

		case '\'':
		    if (language == LANG_C || language == LANG_CPP) {
			seen_non_blank = TRUE;
			StartTxtMode();
		    }
		    else goto process_chars;
		    break;

		case '\"':
		    seen_non_blank = TRUE;
		    StartTxtMode();
		    break;

		case '#':
		    if (language == LANG_C || language == LANG_CPP) {
			if (seen_non_blank == TRUE || ibuffp == 0)
			    seen_directive = TRUE;
		    }
		    goto process_chars;
		    break;

		case '{':
		    if (language == LANG_C || language == LANG_CPP)
			func_depth++;
		    goto process_chars;
		    break;

		case '}':
		    if (language == LANG_C || language == LANG_CPP)
			func_depth--;
		    goto process_chars;
		    break;

		case ']':
		    if (language == LANG_VERILOG)
			square_bracket_depth--;
		    goto process_chars;
		    break;

		case '[':
		    if (language == LANG_VERILOG)
			square_bracket_depth++;
		    goto process_chars;
		    break;

		case ')':
		    if (language == LANG_VERILOG)
			paren_depth--;
		    goto process_chars;
		    break;
		case '(':
		    if (language == LANG_VERILOG)
			paren_depth++;
		    goto process_chars;
		    break;

		default:
    process_chars:
		    if (! isspace(ibuffer[ibuffp]))
			seen_non_blank = TRUE;
		    
		    /*
		     * set is_word_char if the character is a legal
		     * "word" character in the current language
		     */
		    switch (language) {
		    case LANG_C:
			is_word_char = isalnum(ibuffer[ibuffp]) ||
			    	       ibuffer[ibuffp] == '_' ||
				       ibuffer[ibuffp] == '$';
			break;
		    case LANG_CPP:
			is_word_char = isalnum(ibuffer[ibuffp]) ||
			    	       ibuffer[ibuffp] == '_' ||
				       ibuffer[ibuffp] == '$';
			break;
		    case LANG_VERILOG:
		    case LANG_VERA:
			is_word_char = isalnum(ibuffer[ibuffp]) ||
			    	       ibuffer[ibuffp] == '_' ||
				       ibuffer[ibuffp] == '$';
			break;
		    case LANG_TRELLIS:
			is_word_char = isalnum(ibuffer[ibuffp]) ||
			    	       ibuffer[ibuffp] == '_' ||
				       ibuffer[ibuffp] == '#' ||
				       ibuffer[ibuffp] == '?';
			break;
		    }

		    if (is_word_char) {
			/*
			 * gather characters of the current word
			 */
			PutCharInWord();
		    } else {
			/*
			 * non-word character; finish the current word, if any
			 */
			if (cwordp != 0) {
			    if (IsKeyword(cword)) {
				WasKeyword();
				{
				    /*
				     * manage function depth for languages that use begin / end pairs
				     */
				    key_begin_end_t* ptr = func_start_end_array[language];
				    while (ptr->name != NULL) {
					if (strcmp(cword, ptr->name) == 0) {
					    func_depth += ptr->offset;
					    if ((language == LANG_VERILOG) && (ptr->offset > 0)) {
						func_name_search = 1;
					    }
					    //printf("found %s, depth=%d search = %d\n", ptr->name, func_depth, func_name_search);
					    break;
					}
					ptr++;
				    }
				}
			    } else {
				WasNotKeyword();
				if ((language == LANG_VERILOG) &&
				    (func_name_search == 1))
				{
				    if (square_bracket_depth == 0) {
					WasAFunc();
					func_name_search = 0;
					//printf("searched %s, depth=%d search = %d\n", cword, func_depth, func_name_search);
				    }
				}
			    }
			}
			/*
			 * check for start of a comment
			 */
			switch (language) {

			case LANG_C:
			case LANG_CPP:
			case LANG_VERILOG:
			case LANG_VERA:
			    if (ibuffer[ibuffp] == '/' && ibuffer[ibuffp + 1] == '*')
				StartComMode(COMMENT_END_STAR_SLASH, TRUE);
			    else if (ibuffer[ibuffp] == '/' && ibuffer[ibuffp + 1] == '/')
				StartComMode(COMMENT_END_NEWLINE, TRUE);
			    else
				WhatToPutIn(ibuffer[ibuffp]);
			    break;

			case LANG_TRELLIS:
			    if (ibuffer[ibuffp] == '!')
				StartComMode(COMMENT_END_NEWLINE, FALSE);
			    else
				WhatToPutIn(ibuffer[ibuffp]);
			    break;
			}
		    }
		}
	}

	/*
	 * write what we got till now
	 */
	if (obuffp > 0)
	    WriteBuffer();
	/*
	 * we have a function on this line
	 */
	if (have_funcname == TRUE) {
	    fprintf(outfile, "%d %d m ", rmarg, ypos);
	    WriteFont(8);
	    fprintf(outfile, "(%s)rs ", funcname);
	    //printf("function used %s %d->%d\n", funcname, have_funcname, FALSE);
	    have_funcname = FALSE;
	    WriteFont(0);
	}
	/*
	 * put out the line number at the top of page and every 5 lines
	 */
	if (lineno % 5 == 0 || top_of_page == TRUE)
	    WriteLineNo(lineno);

	lineno++;
	top_of_page = FALSE;
	ypos -= LINEWIDTH;
    }

    if (pageno != 0)
	PrintPage();
    
    /*
     * put out blank pages to pad to the next page_skip page boundary
     * (useful for N-up and/or duplex printing)
     */
    while (pageno % page_skip != 0)
	PrintBlankPage();
}


/* Makes the PostScript Trailer */
void MakeTrailer() {
    fprintf(outfile, "%%%%Trailer\n");
    fprintf(outfile, "%%%%Pages: %d\n", pagecount);
    fclose(outfile);
    if (infile != NULL) fclose(infile);
}
