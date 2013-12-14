#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

static char rcsid[] = "$Id: count.cpp,v 1.8 2013-09-24 17:40:52-07 sglaser Exp $";

struct fdata_t {
  static unsigned int max_fname_len;
  static unsigned int nup;
  static unsigned int nrows;
  bool processed;
  unsigned int max_col;
  unsigned int lines;
  unsigned int pages;
  const char* fname;
  fdata_t(const char* fname_p = 0) :
    fname(fname_p),
    max_col(0),
    lines(0),
    pages(0),
    processed(false)
  {
    int len = (fname_p != 0) ? strlen(fname_p) : 0;
    if (len > max_fname_len)
      max_fname_len = len;
  }
  fdata_t& operator + (const fdata_t& f) {
    if (max_col < f.max_col)
      max_col = f.max_col;
    lines += f.lines;
    pages += f.pages;
    if (fdata_t::nup == 2) {
	if ((f.pages % 2) == 1)
	    pages++;
    }
    return *this;
  }
};

unsigned int fdata_t::max_fname_len = 0;
unsigned int fdata_t::nup = 2;
unsigned int fdata_t::nrows = 50;

const char* program_name = "??";

ostream& operator<< (ostream& s, const fdata_t& f) {
  s << setw(f.max_fname_len+2) << f.fname;
  s << setw(5) << f.pages << " page" << ((f.pages > 1) ? "s " : "  ");
  s << setw(6) << f.lines << " line" << ((f.lines > 1) ? "s " : "  ");
  s << setw(6) << f.max_col << " max col" << endl;
  return s;
}

void
process(fdata_t& f)
{
  ifstream fs;
  istream* s = 0;
  if (f.fname != 0) {
    fs.open(f.fname);
    if (fs) s = &fs;
  } else {
    s = &cin;
  }
  if (s != 0) {
    int c;
    f.lines = 0;
    f.pages = 0;
    f.max_col = 0;
    int col = 0;
    int row = 0;
    c = (*s).get();
    while (*s && (c != EOF)) {
      if (c == '\f') {
	f.lines++;
	f.pages++;
	col = 0;
	row = 0;
      } else if (c == '\n') {
	f.lines++;
	if (row++ > f.nrows) {
	  f.pages++;
	  row = 0;
	}
	col = 0;
      } else if (c == '\t') {
	while ((++col) % 8 != 0) {
	  /* nothing */
	}
      } else {
	col++;
      }
      if (col > f.max_col) {
	f.max_col = col;
      }
      c = (*s).get();
    }
    if ((row > 0) || (col > 0)) {
      f.lines++;
      f.pages++;
      row = 0;
      col = 0;
    }
    f.processed = true;
    if (f.fname) fs.close();
  } else {
    cerr << program_name << ": Can't open " << f.fname << " ignroring file" << endl;
  }
  
}

int
main(int argc, char** argv)
{
  vector<fdata_t> files;

  program_name = argv[0];

  if (argc == 1) {
    cerr << "Usage: count [-1] [-2] [-50] [-66] [files or -]" << endl;
    exit(1);
  }
  for (int i = 1; i < argc; i++) {
    //cout << i << " " << argv[i] << endl;
    if (strcmp(argv[i], "-") == 0) {
      fdata_t fdata_stdin(0);
      files.push_back(fdata_stdin);
    } else if (strcmp(argv[i], "-1") == 0) {
	fdata_t::nup = 1;
    } else if (strcmp(argv[i], "-2") == 0) {
	fdata_t::nup = 2;
    } else if (strcmp(argv[i], "-50") == 0) {
	fdata_t::nrows = 50;
    } else if (strcmp(argv[i], "-66") == 0) {
	fdata_t::nrows = 66;
    } else {
      fdata_t fdata_argv(argv[i]);
      files.push_back(fdata_argv);
    }
  }

  vector<fdata_t>::iterator fi;
  for (fi = files.begin(); fi != files.end(); ++fi) {
    process(*fi);
  }

  fdata_t total("TOTAL-->");
  int i = 0;
  for (fi = files.begin(); fi != files.end(); ++fi) {
    if (fi->processed) {
      cout << *fi;
      total = total + *fi;
      i++;
    }
  }
  if (i > 1) {
    cout << total;
  }
  return 0;
}

