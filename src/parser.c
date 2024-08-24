#include "parser.h"
#include "error.h"
#include "common.h"
#include "cmd_args.h"
#include "system_functions.h"
#include "path_builder.h"
#include "recursive.h"
#include "catel.h"
#include <stdarg.h>

static void error(Parser* p, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    cate_error_line_va(p->cur->line, fmt, args);
    if(p->cur_class)
        p->cur_class->bools |= CLASS_BOOL_HAS_ISSUE;
}
#define error(fmt, ...) error(p, fmt, __VA_ARGS__)

static uint8_t was_loaded(const struct CateFullPath* path) {
    for (size_t i = 0; i < ctx.loaded_files.size; ++i) {
        if(strncmp(path->x, ctx.loaded_files.data[i].x, PATH_MAX) == 0)
            return 1;
    }
    return 0;
}

void cate_open(char* path) {
    struct CatePathBuilder path_builder = {0};
    char* to_open = catel_build_path(&path_builder, path);

    //check realpath
    {
        struct CateFullPath fullpath = {0};
        char* res = realpath(to_open, fullpath.x);
        if(!res)
            cate_error("can not open file \"%s\"", to_open);
        if(was_loaded(&fullpath)) return;

        da_append(ctx.loaded_files, fullpath);
    }

    Parser p = {0};
    string_view file = {0};
    int err = sv_load_file(&file, to_open);
    if(err)
        cate_error("can not open file %s", to_open);
    err = cate_tokenize(&file, &p.tokens);
    if(err) exit(1);

    parse(&p);

    free(p.tokens.data);
    free(file.text);
}

static void globals_init(struct Globals* g) {
    static struct Globals def = {
        .compiler = sv_from_const("cc"),
        .bools = CLASS_BOOLS_DEFAULT,
    };

    if(cate_sys_file_exists("cate"))
        def.build_dir = sv_from_const("cate/build/");
    else
        def.build_dir = sv_from_const("build/");

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
    if(find_class(&name))
        error("\""sv_fmt"\" was already defined", sv_p(name));

    CateClass c = {
        .kind = kind,
        .name = name,
        .compiler = p->globals.compiler,
        .standard = p->globals.standard,
        .build_dir = p->globals.build_dir,
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
            if(!p->cur_class)
                error(sv_fmt" was not defined", sv_p(cur->text));
            
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

            //bool property
            {
            ClassBools bp = get_bool_property(p, &child->text);
            if(bp) {
                set_class_bool(p, &p->cur_class->bools, bp);
                continue;
            }
            }

            if(sv_ccmp(&child->text, "type")) {
                expect(TOK_ASSIGN);
                if(p->cur_class->kind != CLASS_LIBRARY) {
                    error("can't change library kind of "sv_fmt
                    "because it's a Project", sv_p(p->cur_class->name));
                }
                class_change_type(p->cur_class, expect_library_kind(p));
                continue;
            }

            error(
                "property \""sv_fmt"\" doesn't exist", sv_p(child->text));
        }   break;

        case TOK_IF: {
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

        case TOK_LCURLY: {
            ++opened_blocks;
            if(opened_blocks > 32)
                error("too many opened blocks, what are even you doing?",0);
            next();
            break;
        }

        case TOK_RCURLY: {
            --opened_blocks;
            if(opened_blocks < 0)
                error("too many '}'",0);
            next();
        }	break;

        case TOK_RECURSIVE: {
            next();
            expect(TOK_LPAREN);
            expect_string(p);
            optional_rparen(p);
            cate_warn_line(cur->line,
                hl_func("recursive") " outside of assignment, skipped");
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
    int32_t origin = *opened_blocks-1;
    while(p->i <= p->tokens.size && *opened_blocks > origin) {
        if(match(TOK_LCURLY))
            ++*opened_blocks;
        else if(match(TOK_RCURLY)) {
            --*opened_blocks;
            if(*opened_blocks == origin)
                break;
        }
        next();
    }
    expect(TOK_RCURLY);
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

        if(sv_ccmp(&name->text, "exists")) {
            expect(TOK_LPAREN);
            uint8_t bad = 0;
            while (!match(TOK_RPAREN)) {
                string_view f = string_or_out_file(p);
                if(!cate_sys_file_exists(f.text))
                    bad = 1;
            }
            
            result = (bad != 1);
            expect(TOK_RPAREN);
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
    if(!(c->bools & CLASS_BOOL_BUILT)) {
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
    if(sv_ccmp(&fn, #name)) {\
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

static void print_fn(Parser* p, string_view* fn, FILE* out) {
    while (!match(TOK_RPAREN)) {
    if(!match(TOK_STRING_LITERAL) && !match(TOK_RPAREN)) {
            error("only strings are allowed in "hl_func(sv_fmt), svptr_p(fn));
            next();
            break;
        }
        fprintf(out, sv_fmt" ", sv_p(cur->text));
        next();
    }
}

static void run_function(Parser* p) {
    string_view fn = expect(TOK_IDENTIFIER)->text;
    expect(TOK_LPAREN);

    #define is_fn(name) sv_ccmp(&fn, name)

    if (is_fn("mkdir") || is_fn("create_directory")) {
        string_view dir = expect_string(p);
        if(!cate_sys_mkdir(dir.text)) {
            error("failed to create directory \""sv_fmt"\"",
                sv_p(dir));
        }
    }

    else if (is_fn("system")) {
        string_view cmd = string_or_out_file(p);
        optional_rparen(p);
        if(cmd_args.flags & CMD_DISABLE_SYSTEM)
            return;

        int status = cate_sys_system(cmd.text);
        if(status != 0)
            cate_error("command \"%s\" exited with code %i",
                cmd.text, status);
    }

    else if (is_fn("print") || is_fn("error")) {
        uint8_t is_err = fn.text[0] == 'e';
        FILE* out = stdout;
        if(is_err) {
            out = stderr;
            fprintf(out, BOLD_RED "Script error: " COLOR_RESET);
        }

        print_fn(p, &fn, out);
        fprintf(out, "\n");
        if(is_err) exit(1);
    }

    else if (is_fn("write") || is_fn("append")) {
        char* mode = fn.text[0] == 'w' ? "w" : "a";
        string_view path = string_or_out_file(p);
        cate_sys_convert_path(path.text);

        FILE* out = fopen(path.text, mode);
        if(!out)
            error("can not open file \""sv_fmt"\"", sv_p(path));

        print_fn(p, &fn, out);
        fclose(out);
    }

    #define two_str_fn(name) \
    (is_fn(#name)) {\
        string_view f1 = string_or_out_file(p);\
        string_view f2 = string_or_out_file(p);\
        if(!cate_sys_##name(f1.text, f2.text))\
            cate_error("failed to "#name" \"%s\" to \"%s\"",\
                f1.text, f2.text);\
    }

    else if two_str_fn(copy)
    else if two_str_fn(move)

    else if (is_fn("subcate")) {
        string_view file = expect_string(p);
        cate_open(file.text);
    }
    
    else {
        error("invalid function \""hl_func(sv_fmt)"\"", sv_p(fn));
        while(!match(TOK_RPAREN)) next();
        expect(TOK_RPAREN);
    }

    #undef is_fn
    #undef two_str_fn

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
    if(cur->kind != TOK_STRING_LITERAL)
        error("expected a string literal (\"text\")",0);
    
    string_view v = cur->text;
    next();
    return v;
}

//TODO: Flatten all of these into a single function with a hashmap.
//(These things are the slowest parts of cate3)

static ClassBools get_bool_property(Parser* p, string_view* v) {
    static_assert(CLASS_BOOL_END == 129, "added a flag? add it here.");

    if (sv_ccmp(v, "smol"))
        return CLASS_BOOL_SMOL;
    else if (sv_ccmp(v, "link"))
        return CLASS_BOOL_LINK;
    else if (sv_ccmp(v, "thread"), sv_ccmp(v, "threading"))
        return CLASS_BOOL_THREAD;
    else if (sv_ccmp(v, "auto"))
        return CLASS_BOOL_AUTO;
    else if (sv_ccmp(v, "built"))
        return CLASS_BOOL_BUILT;
    return CLASS_BOOL_NONE;
}

static string_view* get_string_property(Parser* p,
                                        CateClass* c, string_view* v) {
    if(sv_ccmp(v, "flags"))
        return &c->flags;
    else if(sv_ccmp(v, "out"))
        return &c->out_name;
    else if(sv_ccmp(v, "cc") || sv_ccmp(v, "compiler"))
        return &c->compiler;
    else if(sv_ccmp(v, "std") || sv_ccmp(v, "standard"))
        return &c->standard;
    else if(sv_ccmp(v, "final_flags"))
        return &c->final_flags;
    else if(sv_ccmp(v, "obj_dir")
        ||  sv_ccmp(v, "object_dir")
        ||  sv_ccmp(v, "build_dir")
        ||  sv_ccmp(v, "build_directory")
    )
        return &c->build_dir;

    return 0;
}

static SavedStringIndexes* get_array_property(Parser* p,
                                        CateClass* c, string_view* v) {
    if(sv_ccmp(v, "files"))
        return &c->files;
    else if(sv_ccmp(v, "incs") || sv_ccmp(v, "includes")
    || sv_ccmp(v, "include_paths"))
        return &c->includes;
    else if(sv_ccmp(v, "libs") || sv_ccmp(v, "libraries"))
        return &c->libraries;
    else if(sv_ccmp(v, "defs") || sv_ccmp(v, "definitions"))
        return &c->defines;

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

static void append_array_item(CateClass* c,
                    SavedStringIndexes* arr, string_view* str) {
    if(arr == &c->includes) {
        char buf[str->length+2];
        string_view res = {.length = str->length+2, .text = buf};
        memcpy(buf, "-I", 2);
        memcpy(&buf[2], str->text, str->length);
        save_string(&res, arr);
    } else {
        save_string(str, arr);
    }
}

static void do_recursive(Parser* p, SavedStringIndexes* arr,
                             string_view* globber) {
    RecursiveData data = {.arr = arr};

    size_t asterisk_loc = sv_find(globber, 0, '*');
    if(asterisk_loc == SV_NOT_FOUND)
        error("wildcard '*' not found in "hl_func("recursive"), 0);

    if(asterisk_loc > 0 && globber->text[asterisk_loc-1] != '/')
        error("\"src/idk*.c\" is not allowed in "hl_func("recursive"), 0);

    if(globber->text[asterisk_loc+1] == '*') {
        data.subrecursion = 1;
    } else if(globber->text[asterisk_loc+1] == '/') {
        error("'path/*/*' is not allowed in "hl_func("recursive")
        "\nmaybe use 'path/**'?", 0);
    }

    string_view the_path = sv_substring(globber, 0, asterisk_loc);

    uint8_t extension_allowed = 1;
    if(arr == &p->cur_class->includes) {
        extension_allowed = 0;
        data.to_get = RECURSIVE_GET_DIRS;
    } else {
        data.to_get |= RECURSIVE_GET_FILES_WITH_EXT;
    }

    size_t dot_loc = sv_find_last(globber, '.');
    if(dot_loc == SV_NOT_FOUND && extension_allowed) {
        error("extension '.something' not found in "
            hl_func("recursive"), 0);
    } else if(!extension_allowed) {
        error("extension are not allowed in this kind of "
            hl_func("recursive"), 0);
    }

    data.extension = sv_substring(globber, dot_loc,
                                globber->length);
    if(!cate_recursive(&data, &the_path))
        cate_error(hl_func("recursive") " failed!");
}

static void handle_recursive(Parser* p, SavedStringIndexes* arr) {
    string_view globber = {0};
    if(!match(TOK_RECURSIVE)) {
        globber = expect_string(p);
        do_recursive(p, arr, &globber);
        return;
    }

    next();
    expect(TOK_LPAREN);
    globber = expect_string(p);
    do_recursive(p, arr, &globber);
    expect(TOK_RPAREN);
}

static void clean_array(CateClass* c, SavedStringIndexes* arr) {
    arr->size = 0;
    if(c->bools & CLASS_BOOL_BUILT) {
        c->bools |= CLASS_IBOOL_RELINK;
        c->bools &= ~(CLASS_IBOOL_RELINK);
    }
    
    if(arr == &c->libraries)
        c->loaded_lib_paths.size = 0;
}

static void set_class_array(Parser* p, SavedStringIndexes* arr) {
    expect(TOK_ASSIGN);
    clean_array(p->cur_class, arr);

    if(arr == &p->cur_class->defines)
        return definitions(p);
    
    if(match(TOK_RECURSIVE))
        return handle_recursive(p, arr);

    expect(TOK_LCURLY);
    while (!match(TOK_RCURLY)) {
        switch (cur->kind) {
        case TOK_RECURSIVE:
            handle_recursive(p, arr);
            break;
        
        case TOK_IDENTIFIER:
        case TOK_STRING_LITERAL: {
            TokenKind k = cur->kind;
            string_view item = string_or_out_file(p);
            if(k == TOK_STRING_LITERAL) {
                if(sv_find(&item, 0, '*') != SV_NOT_FOUND) {
                    do_recursive(p, arr, &item);
                    continue;
                }
            }
            append_array_item(p->cur_class, arr, &item);
        }   break;
        
        default:
            error("can't add %s to an array of strings",
                tok_as_text(cur->kind));
            break;
        }
    }
    
    if(arr == &p->cur_class->libraries)
        class_change_libraries(p->cur_class);
    
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

    #define str_prop(n1, n2) \
    if(sv_ccmp(v, #n1) || sv_ccmp(v, #n2)) {\
        next();\
        set_class_string(p, &g->n2);\
        return 1;\
    }

    //alright, maybe it's a string property?
    str_prop(cc, compiler)
    else str_prop(std, standard)

    if(v->text[0] == 'o' || v->text[0] == 'c')  {
        if(sv_ccmp(v, "obj_dir")
        || sv_ccmp(v, "object_dir")
        || sv_ccmp(v, "build_dir")
        || sv_ccmp(v, "build_directory")
        ) {
            set_class_string(p, &g->build_dir);
        }
    }

    #undef str_prop

    return 0;
}