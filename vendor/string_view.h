#ifndef SIMPLE_STRING_VIEW_H
#define SIMPLE_STRING_VIEW_H
#include <stdio.h>
#include <string.h>

typedef struct {
    char* text;
    size_t length;
} string_view;

#define __PP_STRLEN(s) (sizeof(s)/sizeof(s[0]))
#define sv_from_const(s) (string_view){s, __PP_STRLEN(s)-1}
#define sv_fmt "%.*s"
#define sv_p(s) s.length, s.text
#define svptr_p(s) s->length, s->text
#define SV_NOT_FOUND ((size_t)-1)
static inline string_view sv_from_cstr(char* s) {
	return (string_view) {s, strlen(s)};
}

#define sv_debug_print(sv) printf(sv_fmt"\n", sv_p(sv))
#define svptr_debug_print(sv) printf(sv_fmt"\n", svptr_p(sv))

int sv_load_file(string_view* s,const char *file_name);
string_view sv_substring(string_view *s, size_t begin, size_t end);
int sv_equal(const string_view *s1, const string_view *s2);
int sv_equalc(const string_view *s1, const char *s2, size_t length);
size_t sv_find(const string_view *s, size_t from, const char c);
size_t sv_find_last(const string_view *s, const char c);
int sv_ends_with(string_view* restrict s, char* restrict e, size_t l);
int sv_ends_with_sv(string_view* restrict s, string_view* restrict e);

#endif // SIMPLE_STRING_VIEW_H

#ifdef SIMPLE_SV_IMPL
#include <stdlib.h>
#include <errno.h>
int sv_equal(const string_view *s1, const string_view *s2) {
	if(s1->length != s2->length) return 0;
	return memcmp(s1->text, s2->text, s1->length) == 0;
}
int sv_equalc(const string_view *s1, const char *s2, size_t length) {
	if(s1->length != length) return 0;
	return memcmp(s1->text, s2, length) == 0;
}

int sv_load_file(string_view* s,const char *file_name) {
	if(file_name[0] == '\0') return EINVAL;

	FILE *fp = fopen(file_name, "rb");
	if (fp == NULL) {
		return errno;
	}

	if (fseek(fp, 0L, SEEK_END) == 0) {
		/* get the size of the file. */
		long bufsize = ftell(fp);
		if (bufsize == -1) { 
			goto bad;
		}

		s->text = calloc(1, sizeof(char) * (bufsize + 1));

		if (fseek(fp, 0L, SEEK_SET) != 0) {
			goto bad;
		}

		s->length = fread(s->text, sizeof(char), bufsize, fp);
		if (ferror(fp) != 0)
			goto bad;

		s->text[s->length++] = '\0';
	}
	fclose(fp);

	return 0;

bad:;
	int err = errno;
	free(s->text);
	s->text = 0;
	s->length = 0;
	fclose(fp);
	return err;
}

size_t sv_find(const string_view *s, size_t from, const char c) {
	for (size_t i = from; i < s->length; ++i)
		if(s->text[i] == c) return i;
	
	return SV_NOT_FOUND;
}

size_t sv_find_last(const string_view *s, const char c) {
	size_t index = SV_NOT_FOUND;
	for (size_t i = 0; i < s->length; ++i)
		if(s->text[i] == c) index = i;
	
	return index;
}

string_view sv_substring(string_view *s, size_t begin, size_t end) {
	string_view v = {
		.text = (s->text + begin),
		.length = (end - begin)
	};

	if(end < begin || end > s->length)
		return (string_view) {0, 0};

	return v;
}

int sv_ends_with(string_view* restrict s, char* restrict e, size_t l) {
    if(l > s->length) return 0;
    return strncmp(s->text+s->length-l, e, l) == 0;
}

int sv_ends_with_sv(string_view* restrict s, string_view* restrict e) {
    if(e->length > s->length) return 0;
    return strncmp(s->text+s->length-e->length, e->text, e->length) == 0;
}
#endif // SIMPLE_SV_IMPL