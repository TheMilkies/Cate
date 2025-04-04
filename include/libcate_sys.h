/*
    Cross-platform library that implements everything Cate needs.
    Only depends on libc and the platforms' functions.
    Brings its own string_view and dynamic arrays implementation.

    It translates POSIX paths to the platform's native paths automatically.

    License:
    Copyright (c) 2025 @ayinsonu and @TheMilkies

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/
#ifdef __cplusplus
#define restrict
#endif

#ifndef LIBCATE_SYS_H
#define LIBCATE_SYS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*-------------.
| error values |
`------------*/
typedef enum {
    CERR_SUCCESS = 0,
    CERR_INVALID_PATH,
    CERR_TOO_LONG,
    CERR_DOESNT_EXIST,
    CERR_ALREADY_EXISTS,
    CERR_NO_PERMISSION,
    CERR_OUT_OF_MEMORY,
} C_Err;
// #define $(f, ...)

/*---------------.
| dynamic arrays |
`--------------*/
void __cate_da_append(void* array, void* to_append, size_t size);
void __cate_da_pop_back(void* array);
void __cate_da_free(void* array);
#define da_append(array, to_append)\
	__cate_da_append(&(array), &to_append, sizeof(to_append))
#define da_pop(array) __cate_da_pop_back(&(array))
#define da_free(array) __cate_da_free(&(array))
#define da_top(array) ((array).data[(array).size-1])
#define da_at(array, index) ((array).data[index])

#define da_type(type) struct {type* data; size_t size, capacity;}

/*-------------.
| string_views |
`------------*/
typedef struct {
    char* text;
    size_t length;
} cate_sv;

#define sv_pp_strlen(s) (sizeof(s)/sizeof(s[0]))
#define sv_from_const(s) (cate_sv){s, sv_pp_strlen(s)-1}
#define sv_fmt "%.*s"
#define sv_p(s) s.length, s.text
#define svptr_p(s) s->length, s->text
#define SV_NOT_FOUND ((size_t)-1)

static inline cate_sv sv_from_cstr(char* s) {
	return (cate_sv) {s, strlen(s)};
}

int sv_load_file(cate_sv* s,const char *file_name);
cate_sv sv_substring(cate_sv *s, size_t begin, size_t end);
int sv_equal(const cate_sv *s1, const cate_sv *s2);
int sv_equalc(const cate_sv *s1, const char *s2, size_t length);
size_t sv_find(const cate_sv *s, size_t from, const char c);
size_t sv_find_last(const cate_sv *s, const char c);
int sv_ends_with(cate_sv* restrict s, char* restrict e, size_t l);
int sv_starts_with(cate_sv* restrict s, char* restrict e, size_t l);
int sv_ends_with_sv(cate_sv* restrict s, cate_sv* restrict e);

/*------.
| paths |
`-----*/
typedef struct {
    char x[FILENAME_MAX];
    size_t length;
} CateSysPath;

void cs_path_relative(CateSysPath* p); // nothing in POSIX and windows
C_Err cs_path_directory_separator(CateSysPath* p); // '/' in POSIX
C_Err cs_path_append(CateSysPath* p, char* text);

#ifndef __unix__
#error unimplemented
#define cs_path_translate(path)
#else
#define cs_path_translate(path) path
#endif

/*-------.
| system |
`------*/
int cs_is_admin();
size_t cs_get_thread_count();
int cs_file_exists(char* file);

/* checks if file1's modification date is newer than file2's.
   will returns true if file2 doesn't exist */
int cs_newer_than(char* file1, char* file2);

/* create a directory and its parents, like mkdir -p */
C_Err cs_create_directory(char* dir);
/* copy a file or directory (src) to (dest), like cp -r */
C_Err cs_copy(char* src, char* dest);
/* move a file or directory (src) to (dost), like mv */
C_Err cs_move(char* src, char* dest);
/* remove a file or directory, like rm -r */
C_Err cs_remove(char* path);
/* remove a file, like plain rm */
C_Err cs_remove_single(const char* file);
/* run the smolization command */
C_Err cs_smolize(char* file);

/*----------.
| processes |
`---------*/
struct CateSysProc;
typedef struct CateSysProc CateSysProc;
typedef da_type(char*) Command;
static void cmd_free(Command* c);

CateSysProc* cs_proc_create(Command* cmd, C_Err* err);
void cs_dry_run(Command* cmd);
int cs_proc_exited(CateSysProc* proc);
int cs_proc_get_exit_code(CateSysProc* proc);
void cs_proc_kill(CateSysProc* proc);
int cs_proc_wait(CateSysProc* proc);
void cs_proc_free(CateSysProc* proc);

#endif // LIBCATE_SYS_H

#ifdef LIBCATE_SYS_IMPL
#include <errno.h>
//generic for any platform

/*-------------------.
| errors and logging |
`------------------*/
#define fatal(text) do{fprintf(stderr, "cate: " text "\n");\
    exit(-1);} while(0);
#define log(text, ...) printf("cate: " text, __VA_ARGS__);
#define fatal_f(text, ...) do{fprintf(stderr, "cate: " text "\n", __VA_ARGS__);\
    exit(-1);} while(0);

/*-------.
| memory |
`------*/
//allocate and zero-init N bytes
static void* xalloc(size_t n) {
    void* ptr = calloc(sizeof(uint8_t), n);
    if(!ptr) {
        fatal("out of memory!");
    }
    return ptr;
}

static void* xrealloc(void* ptr, size_t n) {
    void* new_ptr = realloc(ptr, n);
    if(!new_ptr) {
        fatal("out of memory!");
    }
    return new_ptr;
}

/*---------------.
| dynamic arrays |
`--------------*/
struct __DA {
	void* data;
	size_t size;
	size_t capacity;
};

void __cate_da_append(void* array, void* to_append, size_t size) {
	struct __DA* a = (struct __DA*)array;
	if(!a->data) {
		a->capacity = size*2;
		a->data = xalloc(a->capacity);
		a->size = 0;
	}

	const size_t used = a->size*size;
	if(used >= a->capacity) {
		a->capacity *= 2;
		a->data = xrealloc(a->data, a->capacity);
	}

	memcpy((uint8_t*)(a->data)+used, to_append, size);
	++a->size;
}

void __cate_da_pop_back(void* array) {
	struct __DA* a = (struct __DA*)array;
	if(a->size > 0)
		a->size -= 1;
}

void __cate_da_free(void* array) {
	if(!array) return;
	struct __DA* a = (struct __DA*)array;
	if(a->data)
		free(a->data);
	a->data = 0;
	a->size = 0;
	a->capacity = 0;
}

/*-------------.
| string_views |
`------------*/
int sv_equal(const cate_sv *s1, const cate_sv *s2) {
	if(s1->length != s2->length) return 0;
	return memcmp(s1->text, s2->text, s1->length) == 0;
}

int sv_equalc(const cate_sv *s1, const char *s2, size_t length) {
	if(s1->length != length) return 0;
	return memcmp(s1->text, s2, length) == 0;
}

int sv_load_file(cate_sv* s,const char *file_name) {
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

		s->text = (char*)xalloc(sizeof(char) * (bufsize + 1));

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

size_t sv_find(const cate_sv *s, size_t from, const char c) {
	for (size_t i = from; i < s->length; ++i)
		if(s->text[i] == c) return i;
	
	return SV_NOT_FOUND;
}

size_t sv_find_last(const cate_sv *s, const char c) {
	size_t index = SV_NOT_FOUND;
	for (size_t i = 0; i < s->length; ++i)
		if(s->text[i] == c) index = i;
	
	return index;
}

cate_sv sv_substring(cate_sv *s, size_t begin, size_t end) {
	cate_sv v = {
		.text = (s->text + begin),
		.length = (end - begin)
	};

	if(end < begin || end > s->length)
		return (cate_sv) {0, 0};

	return v;
}

int sv_ends_with(cate_sv* restrict s, char* restrict e, size_t l) {
    if(l > s->length) return 0;
    return strncmp(s->text+s->length-l, e, l) == 0;
}

int sv_starts_with(cate_sv* restrict s, char* restrict e, size_t l) {
    if(l > s->length) return 0;
    return strncmp(s->text, e, l) == 0;
}

int sv_ends_with_sv(cate_sv* restrict s, cate_sv* restrict e) {
    if(e->length > s->length) return 0;
    return strncmp(s->text+s->length-e->length, e->text, e->length) == 0;
}

/*------------.
| impl: POSIX |
`-----------*/
#ifdef __unix__
//test platforms: FreeBSD 5.0, FreeBSD 14, NetBSD 10.1, Debian 12.
//planned support: AIX?, Solaris 9
#define RELATIVE_DIR "./"
#define DIR_SEPARATOR "/"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <dirent.h>
#include <ftw.h>

//since some unices don't have threads, we have to do this. 
#ifdef _SC_NPROCESSORS_ONLN
size_t cs_get_thread_count() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}
#else
int cs_get_thread_count() {
    return 1;
}
#endif //_SC_NPROCESSORS_ONLN

int cs_file_exists(char* file) {
    return access(file, F_OK) == 0;
}

int cs_newer_than(char* file1, char* file2) {
    struct stat result;
    if(stat(file1, &result) != 0)
        return 1;
    const size_t f1_time = result.st_mtime;

    if(stat(file2, &result) != 0)
        return 1;
    const size_t f2_time = result.st_mtime;
    return f1_time > f2_time;
}

int cs_is_admin() { return geteuid() == 0; }

/*----------.
| processes |
`---------*/
#include <sys/wait.h>
#include <signal.h>

struct CateSysProc {
    pid_t pid;
    int status;
};

struct CateSysProc {
    pid_t pid;
    int status;
};

CateSysProc* cs_proc_create(Command* cmd, C_Err* err) {
    CateSysProc* p = (CateSysProc*)xalloc(sizeof(*p));
    p->pid = fork();
    if(p->pid == 0) {
        execvp(cmd->data[0], cmd->data);
        fatal_f("%s: program not found!\n", cmd->data[0]);
    } else if (p->pid < 0) {
        *err = CERR_OUT_OF_MEMORY;
    }

    return p;
}

int cs_proc_exited(CateSysProc* proc) {
    if(waitpid(proc->pid, &proc->status, WNOHANG) == -1) {
        fatal("failed to get process status?");
    }
    return WIFEXITED(proc->status);
}

int cs_proc_wait(CateSysProc* proc) {
    if(waitpid(proc->pid, &proc->status, 0) == -1) {
        fatal("failed to get process status?");
    }
    return WEXITSTATUS(proc->status);
}

void cs_proc_free(CateSysProc* proc) {
    free(proc); //null is ignored (dry run)
}

int cs_proc_get_exit_code(CateSysProc* proc) {
    if(!WIFEXITED(proc->status))
        fatal("this is a bug #0");
    return WEXITSTATUS(proc->status);
}

void cs_proc_kill(CateSysProc* proc) {
    kill(proc->pid, SIGKILL);
}

#endif //impl: POSIX

void cs_path_relative(CateSysPath* p) {
    //for posix, the relative path can just be Nothing
    p->length = 0;
    p->x[0] = '\0';
    // p->length = sv_pp_strlen(RELATIVE_DIR);
    // memcpy(p->x, RELATIVE_DIR, p->length);
}

static inline _cs_path_append(CateSysPath* p, char* text, size_t length) {
    if(p->length+length > sizeof(p->x))
        return CERR_TOO_LONG;

    memcpy(&p->x[p->length], text, length);
    p->length += length;

    return CERR_SUCCESS;
}

C_Err cs_path_directory_separator(CateSysPath* p) {
    return _cs_path_append(p, DIR_SEPARATOR, sv_pp_strlen(DIR_SEPARATOR));
}

C_Err cs_path_append(CateSysPath* p, char* text) {
    return _cs_path_append(p, text, strlen(text)+1);
}

#endif //impl