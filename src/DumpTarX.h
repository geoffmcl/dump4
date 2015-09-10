
// DumpTarX.h
#ifndef _DumpTarX_h_
#define _DumpTarX_h_

/* General Interface */

struct xhdr_tab
{
  char const *keyword;
  void (*coder) (struct tar_stat_info const *, char const *,
		 struct xheader *, void const *data);
  void (*decoder) (struct tar_stat_info *, char const *, char const *, size_t);
  bool protect;
};

struct keyword_list
{
  struct keyword_list *next;
  char *pattern;
  char *value;
};

extern struct xhdr_tab const xhdr_tab[];

#endif //_DumpTarX_h_
// eof - DumpTarX.h


