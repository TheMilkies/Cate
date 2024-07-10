#include "parser.h"
#include "error.h"
#include "common.h"
#include "cmd_args.h"
#include "system_functions.h"
#include <stdarg.h>

static void error(Parser* p, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    cate_error_line_va(p->cur->line, fmt, args);
    if(p->cur_class)
        p->cur_class->bools |= CLASS_BOOL_HAS_ISSUE;
}
#define error(fmt, ...) error(p, fmt, __VA_ARGS__)

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
    if(err)
        cate_error("can not open file %s", path);
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
#define cur p->cur
#define match(k) (p->i <= p->tokens.size && cur->kind == k)

static void optional_rparen(Parser* p) {
    if(match(TOK_RPAREN))
        next();
    else
        warn(p, "expected a ')' but i guess i don't need it");
}

static string_view string_or_out_file(Parser* p);
static void set_class_bool(Parser* p, ClassBools* original, ClassBools bit);
static LibraryKind expect_library_kind(Parser* p);
static ClassBools get_bool_property(Parser* p, string_view* v);
static string_view* get_string_property(Parser* p,
                                        CateClass* c, string_view* v);
static SavedStringIndexes* get_array_property(Parser* p,
                                        CateClass* c, string_view* v);
static void set_class_string(Parser* p, string_view* v);
static void set_class_array(Parser* p, SavedStringIndexes* arr);
static uint8_t parse_if_part(Parser* p);
static uint8_t parse_cond(Parser* p);
static void skip_block(Parser* p, int32_t* opened_blocks);
static string_view expect_string(Parser* p);
static void run_function(Parser* p);
static void class_method(Parser* p);

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
    int32_t opened_blocks = 0;

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
            if(peek(p, 0) == TOK_LPAREN) {
                run_function(p);
                continue;
            }

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

            if(peek(p, 0) == TOK_LPAREN) {
                class_method(p);
                continue;
            }
            Token* child = expect(TOK_IDENTIFIER);

            //bool property
            {
            ClassBools bp = get_bool_property(p, &child->text);
            if(bp) {
                set_class_bool(p, &p->cur_class->bools, bp);
                continue;
            }
            }

            //string properties
            {
            string_view* sp =
                get_string_property(p, p->cur_class, &child->text);
            if(sp) {
                set_class_string(p, sp);
                continue;
            }
            }

            //array properties
            {
            SavedStringIndexes* arr =
                get_array_property(p, p->cur_class, &child->text);
            if(arr) {
                set_class_array(p, arr);
                continue;
            }
            }

            error(
                "property \""sv_fmt"\" doesn't exist", sv_p(child->text));
        }   break;

        case TOK_IF: {
            if(opened_blocks)
                error("nested ifs are not supported",0);
            uint8_t found_true = 0;
            uint8_t cond = 0;
            /* this is very dumb!
            it tries to find one true thing, then just skips the rest */

        restart_if:
            cond = parse_if_part(p);

            expect(TOK_LCURLY);
            ++opened_blocks;
            
            if(found_true) continue;
            if(cond)
                found_true = 1;
            else
                skip_block(p, &opened_blocks);

            if(match(TOK_ELSE)) {
                next();
                if(match(TOK_IF))
                    goto restart_if;
                
                expect(TOK_LCURLY);
                ++opened_blocks;
                if(found_true)
                    skip_block(p, &opened_blocks);
            }

        } break;

        case TOK_ELSE: {
            //dangling else, has to be false.
            next();
            if(match(TOK_IF))
                parse_if_part(p);
            expect(TOK_LCURLY);
            ++opened_blocks;
            skip_block(p, &opened_blocks);
        }	break;

        case TOK_RCURLY: {
            --opened_blocks;
            if(opened_blocks < 0) {
                error("too many '}'",0);
            }
            next();
        }	break;
        
        default:
            error("%s is not allowed here", tok_as_text(cur->kind));
            break;
        }
    }
    if(opened_blocks)
        error("unclosed if statements", 0);
}

static uint8_t parse_if_part(Parser* p) {
    expect(TOK_IF);
    expect(TOK_LPAREN);
    uint8_t cond = parse_cond(p);
    optional_rparen(p);
    return cond;
}

static void skip_block(Parser* p, int32_t* opened_blocks) {
    while(p->i <= p->tokens.size && cur->kind != TOK_RCURLY) {
        next();
    }
    expect(TOK_RCURLY);
    --*opened_blocks;
}

static CateClass* find_class_or_exit(Parser* p, string_view* name) {
    CateClass* c = find_class(name);
    if(!c) {
        error("undefined class \""sv_fmt"\"", svptr_p(name));
    }
    return c;
}

static uint8_t parse_cond(Parser* p) {
    //invert count is just so we won't need recursion
    uint8_t result = 0, invert_count = 0;
restart:
    switch (cur->kind) {
    case TOK_EXCLAMATION_MARK:
        next();
        ++invert_count;
        goto restart;
        break;
    
    case TOK_TRUE: next(); result = 1; break;
    case TOK_FALSE:next(); result = 0; break;

    case TOK_IDENTIFIER: {
        Token* name = expect(TOK_IDENTIFIER);
        if(cate_platform_check(&name->text)) {
            result = 1;
            goto done;
        }

        ClassBools fg = get_bool_property(p, &name->text);
        if(fg)
            result = (p->globals.bools & fg) != 0;

        if(match(TOK_DOT)) {
            CateClass* c = find_class_or_exit(p, &name->text);
            next();

            Token* child = expect(TOK_IDENTIFIER);
            fg = get_bool_property(p, &child->text);
            if(!fg) {
                error("invalid property in 'if'",0);
                result = 0;
            }
            result = (c->bools & fg) != 0;
        }
    }	break;

    case TOK_RPAREN:
        error("expected a condition",0);
        return 0;
        break;

    default: {
        error("invalid condition",0);
        return 0;
    }   break;
    }

done:
    if(invert_count && invert_count % 2 != 0)
        result = !result;
    
    return result;
}

static string_view string_or_out_file(Parser* p) {
    if(match(TOK_STRING_LITERAL)) {
        return expect_string(p);
    }

    if(!match(TOK_IDENTIFIER)) {
        error("expected a string or class name, which'd be treated as outfile."
        NL BOLD_CYAN "example: " COLOR_RESET
        hl_func_params("print", "\"Hello!\"") " => \"Hello!\""
        NL BOLD_CYAN "example: " COLOR_RESET
        hl_func_params("print", "slib") " => \"libslib.a\""
        ,0);
    }

    CateClass* c = find_class_or_exit(p, &cur->text);
    if(!c->bools & CLASS_BOOL_BUILT) {
        error("can't use \""sv_fmt"\"'s outfile "
        "because it wasn't build", sv_p(c->name));
    }
    next();
    return c->out_name;
}

static void class_method(Parser* p) {
    string_view fn = expect(TOK_IDENTIFIER)->text;
    expect(TOK_LPAREN);

    #define method(name)\
    if(sv_equalc(&fn, #name, sizeof(#name)/sizeof(#name[0])-1)) {\
        class_ ## name (p->cur_class);\
        goto done;\
    }

    method(build)
    method(clean)
    method(install)

    #undef method
done:
    optional_rparen(p);
}

static void run_function(Parser* p) {
    string_view fn = expect(TOK_IDENTIFIER)->text;
    expect(TOK_LPAREN);

    if (sv_equalc(&fn, "mkdir", 5) || sv_equalc(&fn, "create_directory", 16)) {
        string_view dir = expect_string(p);
        if(!cate_sys_mkdir(dir.text)) {
            error("failed to create directory \""sv_fmt"\"",
                sv_p(dir));
        }
    }

    else if (sv_equalc(&fn, "system", 6)) {
        string_view cmd = string_or_out_file(p);
        optional_rparen(p);
        if(cmd_args.flags & CMD_DISABLE_SYSTEM)
            return;

        int status = cate_sys_system(cmd.text);
        if(status != 0)
            cate_error("command \"%s\" exited with code %i",
                cmd.text, status);
    }

    else if (sv_equalc(&fn, "print", 5) || sv_equalc(&fn, "error", 5)) {
        uint8_t is_err = fn.text[0] == 'e';
        FILE* out = stdout;
        if(is_err) {
            out = stderr;
            fprintf(out, BOLD_RED "Script error: " COLOR_RESET);
        }

        while (!match(TOK_RPAREN)) {
            if(!match(TOK_STRING_LITERAL) && !match(TOK_RPAREN)) {
                error("only strings are allowed in "hl_func(sv_fmt), sv_p(fn));
                next();
                break;
            }
            fprintf(out, sv_fmt" ", sv_p(cur->text));
            next();
        }
        fprintf(out, "\n");
        if(is_err) exit(4);
    }

    else if (sv_equalc(&fn, "copy", 4)) {
        string_view f1 = string_or_out_file(p);
        string_view f2 = string_or_out_file(p);
        if(!cate_sys_copy(f1.text, f2.text))
            cate_error("failed to copy \"%s\" to \"%s\"",
                f1.text, f2.text);
    }

    else if (sv_equalc(&fn, "move", 4)) {
        string_view f1 = string_or_out_file(p);
        string_view f2 = string_or_out_file(p);
        if(!cate_sys_move(f1.text, f2.text))
            cate_error("failed to move \"%s\" to \"%s\"",
                f1.text, f2.text);
    }

    else if (sv_equalc(&fn, "subcate", 7)) {
        todo("subcate()");
    }
    
    else {
        error("invalid function \""hl_func(sv_fmt)"()\"", sv_p(fn));
        while(!match(TOK_RPAREN)) next();
    }

    optional_rparen(p);
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

static inline void definitions(Parser* p) {
    char buf[512] = {0};
    string_view buf_as_sv = {.text = buf};

    expect(TOK_LCURLY);
    while (!match(TOK_RCURLY)) {
        string_view def = expect_string(p);
        if(def.length >= 512-3) //-3 for "-D" and null terminator
            error("definition \""sv_fmt"\" is too long", sv_p(def));
        if(def.length == 0) continue;
        
        //"def" -> "-Ddef" (save a step in the build)
        strncat(buf, "-D", 3);
        //since the '"' character is turned into a null,
        //we add it to save a step (+1)
        strncat(buf, def.text, def.length+1); 

        //save it in the table
        buf_as_sv.length = def.length+3;
        save_string(&buf_as_sv, &p->cur_class->defines);

        //clear the buffer
        memset(buf, 0, buf_as_sv.length);
    }
    
    expect(TOK_RCURLY);
}

static void set_class_array(Parser* p, SavedStringIndexes* arr) {
    expect(TOK_ASSIGN);

    if(arr == &p->cur_class->defines)
        return definitions(p);
    
    if(match(TOK_RECURSIVE))
        todo("direct item = recursive()");

    expect(TOK_LCURLY);
    while (!match(TOK_RCURLY)) {
        switch (cur->kind) {
        case TOK_RECURSIVE:
            todo("recursive() in {}");
            break;
        
        case TOK_IDENTIFIER:
        case TOK_STRING_LITERAL: {
            string_view item = string_or_out_file(p);
            save_string(&item, arr);
        }   break;
        
        default:
            error("can't add %s to an array of strings",
                tok_as_text(cur->kind));
            break;
        }
    }
    
    expect(TOK_RCURLY);
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