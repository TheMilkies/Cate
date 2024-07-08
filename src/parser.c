#include "parser.h"
#include "error.h"
#include <stdarg.h>

static void error(Parser* p, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    cate_error_line_va(p->cur->line, fmt, args);
    if(p->cur_class)
        p->cur_class->bools |= CLASS_BOOL_HAS_ISSUE;
    //yea we should honestly just exit.
    exit(-1);
}
#define error(fmt, ...) error(p, fmt, __VA_ARGS__);

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
    [x] Tokenize
    [x] Start the parser
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
    free(p.tokens.data);
}

static void globals_init(struct Globals* g) {
    static struct Globals def = {
        .compiler = sv_from_const("cc"),
        .bools = CLASS_BOOLS_DEFAULT,
    };
    *g = def;
}

static CateClass* find_class(string_view* name) {
    for (size_t i = 0; i < ctx.classes.size; ++i) {
        if(sv_equal(&ctx.classes.data[i].name, name))
            return &ctx.classes.data[i];
    }

    return 0;
}

#define match(k) (p->i <= p->tokens.size && p->cur->kind == k)
static inline void next(Parser* p) {
    p->cur = &p->tokens.data[p->i++];
}
#define next() next(p)

static inline TokenKind peek(Parser* p, uint8_t count) {
    return p->tokens.data[p->i+count].kind;
}

static void warn(Parser* p, const char* text) {
    cate_warn_line(p->cur->line, text);
}
static Token* expect(Parser* p, TokenKind k) {
    Token* backup = p->cur;
    if(backup->kind != k) {
        error("expected %s but got %s",
            tok_as_text(k), tok_as_text(backup->kind));
    }
    next();
    return backup;
}
#define expect(k) expect(p, k)

static void optional_rparen(Parser* p) {
    if(match(TOK_RPAREN))
        next();
    else
        warn(p, "expected a ')' but i guess i don't need it");
}

static LibraryKind expect_library_kind(Parser* p);
#define cur p->cur

static CateClass* new_class(Parser* p, ClassKind kind) {
    string_view name = expect(TOK_IDENTIFIER)->text;
    if(find_class(&name)) {
        error("\""sv_fmt"\" was already defined", sv_p(name));
    }

    CateClass c = {
        .kind = kind,
        .name = name,
        .compiler = p->globals.compiler,
        .standard = p->globals.standard,
        .bools = p->globals.bools,
    };
    da_append(ctx.classes, c);

    return &da_top(ctx.classes);
}

void parse(Parser* p) {
    globals_init(&p->globals);
    next();

    while (p->i <= p->tokens.size) {
        switch (cur->kind) {
        case TOK_PROJECT:
            next();
            p->cur_class = new_class(p, CLASS_PROJECT);
            break;
        case TOK_LIBRARY:
            next();
            p->cur_class = new_class(p, CLASS_LIBRARY);

            expect(TOK_LPAREN);
            p->cur_class->as.lib.kind = expect_library_kind(p);
            optional_rparen(p);
            break;
        
        default:
            error("%s is not allowed", tok_as_text(cur->kind));
            break;
        }
    }
}

static LibraryKind expect_library_kind(Parser* p) {
    if(cur->kind != TOK_STATIC && cur->kind != TOK_DYNAMIC) {
        error("expected a "hl_var("LibraryType")" value ("
        choose_light("static", " | ", "dynamic")")",0);
    }
    LibraryKind k =
        cur->kind == TOK_DYNAMIC
        ? LIBRARY_DYNAMIC
        : LIBRARY_STATIC;
    next();
    return k;
}