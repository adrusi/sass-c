#ifndef STRBUF_H_
#define STRBUF_H_

typedef struct {
  char *string;
  int len;
  int cap;
} strbuf;

extern strbuf strbuf_new(void);

extern void strbuf_append(strbuf *buf, char *str);

extern void strbuf_appendchar(strbuf *buf, char c);

extern void strbuf_free(strbuf buf);

#endif
