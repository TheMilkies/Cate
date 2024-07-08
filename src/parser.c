#include "parser.h"
#include "error.h"
#include "common.h"
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

static void set_class_bool(Parser* p, ClassBools* original, ClassBools bit);
static LibraryKind expect_library_kind(Parser* p);
static ClassBools get_bool_property(Parser* p, string_view* v);
static uint8_t parse_if_part(Parser* p);
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

static uint8_t is_global(Parser* p);
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

        case TOK_IDENTIFIER: {
            if(is_global(p)) continue;

            p->cur_class = find_class(&cur->text);
            if(!p->cur_class) {
                error(sv_fmt" was not defined", sv_p(cur->text));
            }
            next();
            goto parse_dot;
        }   break;

        case TOK_DOT: {
        parse_dot:
            expect(TOK_DOT);
            if(!p->cur_class)
                error("no object selected.", 0);

            Token* child = expect(TOK_IDENTIFIER);

            //bool property
            {
            ClassBools bp = get_bool_property(p, &child->text);
            if(bp) {
                set_class_bool(p, &p->cur_class->bools, bp);
                continue;
            }
            }

            error(
                "property \""sv_fmt"\" doesn't exist", sv_p(child->text));
        }   break;
        
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

static uint8_t expect_bool(Parser* p) {
    if(cur->kind != TOK_TRUE && cur->kind != TOK_FALSE) {
        error("expected a boolean value ("
        traffic_light("true", " | ", "false")")", 0);
    }
    uint8_t v = cur->kind == TOK_TRUE;
    next();
    return v;
}

static string_view expect_string(Parser* p) {
    if(cur->kind != TOK_STRING_LITERAL) {
        error("expected a string literal (\"text\")",0);
    }
    string_view v = cur->text;
    next();
    return v;
}

static ClassBools get_bool_property(Parser* p, string_view* v) {
    static_assert(CLASS_BOOL_END == 33, "added a flag? add it here.");
    if (sv_equalc(v, "smol", 4)) {
        return CLASS_BOOL_SMOL;
    } else if (sv_equalc(v, "link", 4)) {
        return CLASS_BOOL_LINK;
    } else if (sv_equalc(v, "thread", 6)) {
        return CLASS_BOOL_THREAD;
    } else if (sv_equalc(v, "auto", 4)) {
        return CLASS_BOOL_AUTO;
    } else if (sv_equalc(v, "built", 5)) {
        return CLASS_BOOL_BUILT;
    }
    return CLASS_BOOL_NONE;
}

static string_view* get_string_property(Parser* p,
                                        CateClass* c, string_view* v) {
    if(sv_equalc(v, "out", 3)) {
        return &c->out_name;
    } else if(sv_equalc(v, "cc", 2) || sv_equalc(v, "compiler", 8)) {
        return &c->compiler;
    } else if(sv_equalc(v, "std", 3) || sv_equalc(v, "standard", 8)) {
        return &c->standard;
    } else if(sv_equalc(v, "flags", 5)) {
        return &c->flags;
    } else if(sv_equalc(v, "final_flags", 11)) {
        return &c->final_flags;
    }
    else if(sv_equalc(v, "obj_dir", 7)
        ||  sv_equalc(v, "object_dir", 10)
        ||  sv_equalc(v, "build_dir", 9)
        ||  sv_equalc(v, "build_directory", 15)
    ) {
        return &c->build_dir;
    }

    return 0;
}

static SavedStringIndexes* get_array_property(Parser* p,
                                        CateClass* c, string_view* v) {
    if(sv_equalc(v, "files", 5)) {
        return &c->files;
    } else if(sv_equalc(v, "incs", 4) || sv_equalc(v, "includes", 8)
    || sv_equalc(v, "include_paths", 13)) {
        return &c->includes;
    } else if(sv_equalc(v, "libs", 4) || sv_equalc(v, "libraries", 9)) {
        return &c->libraries;
    } else if(sv_equalc(v, "defs", 4) || sv_equalc(v, "definitions", 11)) {
        return &c->defines;
    }

    return 0;
}

static void set_class_bool(Parser* p, ClassBools* original,
                            ClassBools bit) {
    expect(TOK_ASSIGN);
    uint8_t v = expect_bool(p);
    if(v)
        *original |= bit;
    else
        *original &= ~bit;
}

static void set_class_string(Parser* p, string_view* v) {
    expect(TOK_ASSIGN);
    *v = expect_string(p);
}

static uint8_t is_global(Parser* p) {
    struct Globals* const g = &p->globals;
    string_view* const v = &cur->text;

    ClassBools flags = get_bool_property(p, v);
    if(flags) {
        next();
        set_class_bool(p, &g->bools, flags);
        return 1;
    }

    //alright, maybe it's a string property?
    if(sv_equalc(v, "cc", 2) || sv_equalc(v, "compiler", 8)) {
        next();
        set_class_string(p, &g->compiler);
        return 1;
    } else if(sv_equalc(v, "standard", 8) || sv_equalc(v, "std", 3)) {
        next();
        set_class_string(p, &g->standard);
        return 1;
    }

    return 0;
}