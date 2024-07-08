#include "parser.h"
#include "error.h"
#include <stdarg.h>

void cate_open(const char* path) {
    //struct CateFullPath open_path = {0};
    /*
    TODO
    [ ] Add the directory provided by Catel
    [ ] Add the .cate extension if it doesn't have it
    [ ] Get the real full path
    [ ] Check if it was loaded already (ctx.loaded_paths)
         - skip if it did
         - add it to ctx.loaded_paths
    [ ] Tokenize
    [ ] Start the parser
    */

    Parser p = {0};
    string_view file = {0};
    int err = sv_load_file(&file, path);
    if(err) {
        cate_error("can not open file %s", path);
        exit(1);
    }
    err = cate_tokenize(&file, &p.tokens);
    if(err) exit(1);

    parse(&p);

    free(file.text);
}

static void globals_init(struct Globals* g) {
    static struct Globals def = {
        .compiler = sv_from_const("cc"),
        .bools = CLASS_BOOLS_DEFAULT,
    };
    *g = def;
}

static CateClass* find_class(CateContext* c, string_view* name) {
    for (size_t i = 0; i < c->classes.size; ++i) {
        if(sv_equal(&c->classes.data[i].name, name))
            return &c->classes.data[i];
    }

    return 0;
}

#define match(k) (p->i < p->tokens.size && p->cur->kind == k)
static inline void next(Parser* p) {
    p->cur = &p->tokens.data[p->i++];
}
static inline TokenKind peek(Parser* p, uint8_t count) {
    return p->tokens.data[p->i+count].kind;
}
static void error(Parser* p, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    cate_error_line_va(p->cur->line, fmt, args);
    if(p->cur_class)
        p->cur_class->bools |= CLASS_BOOL_HAS_ISSUE;
    //yea we should honestly just exit.
    exit(-1);
}
static void warn(Parser* p, const char* text) {
    cate_warn_line(p->cur->line, text);
}
static Token* expect(Parser* p, TokenKind k) {
    Token* backup = p->cur;
    if(backup->kind != k) {
        error(p, "expected %s but got %s",
            tok_as_text(k), tok_as_text(backup->kind));
    }
    next(p);
    return backup;
}

static void optional_rparen(Parser* p) {
    if(match(TOK_RPAREN)) next(p);
    else
        warn(p, "expected a ')' but i guess i don't need it");
}

void parse(Parser* p) {
    globals_init(&p->globals);
    while (p->i <= p->tokens.size) {
        
    }
    
}