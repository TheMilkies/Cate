#include "cate_lang.h"
#include <ctype.h>

/*-------.
| common |
`------*/
#if __STDC_VERSION__ == 201112L
    #define static_assert(cond, msg) _Static_assert(cond, msg)
#elif __STDC_VERSION__ >= 202311L
#else
    #define static_assert(cond, msg)
#endif

#define todo(text) {\
    fputs(text " is not implemented\n", stderr);\
    exit(1);\
}

//only use this with constant preprocessor strings
//sv_ccmp(v, str): BAD
//sv_ccmp(v, "str"): GOOD
#define sv_ccmp(sv, text) (sv_equalc(sv, text, sizeof(text)/sizeof(text[0])-1))

/*-------.
| system |
`------*/
#ifndef _WIN32
#include <unistd.h>
#define NL "\n"
static int file_exists(char* file) {
    return access(file, F_OK) == 0;
}
#else
#define NL "\r\n"
#error win32 unimplemented currently
#endif

/*------.
| error |
`-----*/
//TODO: make the error functions an interface maybe?
#define BOLD_RED "\033[1;31m"
#define COLOR_RESET "\033[0m"
#include <stdarg.h>

void cate_error(const char* fmt, ...) {
    fprintf(stderr, BOLD_RED "cate error: " COLOR_RESET);
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, NL);
    exit(-1);
}

void cate_error_line(size_t line, const char* fmt, ...) {
    fprintf(stderr, BOLD_RED "cate error in line %zu: " COLOR_RESET, line);
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, NL);
    exit(-1);
}

/*----------.
| tokenizer |
`---------*/
static TokenKind maybe_keyword(string_view* v) {
    static_assert(CTOK_COUNT_SIZE == 18,
        "added token types? if they are keywords; add them here");
    if(sv_ccmp(v, "Project")) {
        return CTOK_PROJECT;
    } else if(sv_ccmp(v, "Library")) {
        return CTOK_LIBRARY;
    } else if(sv_ccmp(v, "static")) {
        return CTOK_STATIC;
    } else if(sv_ccmp(v, "dynamic")) {
        return CTOK_DYNAMIC;
    } else if(sv_ccmp(v, "true")) {
        return CTOK_TRUE;
    } else if(sv_ccmp(v, "false")) {
        return CTOK_FALSE;
    } else if(sv_ccmp(v, "if")) {
        return CTOK_IF;
    } else if(sv_ccmp(v, "else")) {
        return CTOK_ELSE;
    }
    return CTOK_IDENTIFIER;
}

static void skip_whitespace(string_view *line, size_t* i) {
    while(*i < line->length && isspace(line->text[*i])) *i += 1;
}

void cate_tokenize(string_view *line, TokensArray *tokens,
                        TokenValuesArray* values) {
    static_assert(CTOK_COUNT_SIZE == 18,
        "added token types? if they are not keywords; add them here");
    size_t i = 0, line_num = 1;

    #define cur line->text[i]
    #define next() ++i
    #define peek(n) line->text[i+n]
    #define in_range() (i < line->length)
    #define while_in(cond) while(in_range() && (cond))
    #define skip_until(ch) while_in(cur != ch) next();
    #define save() {\
        da_append(*tokens, tok);\
        da_append(*values, val);\
    }

    while (i < line->length) {
        skip_whitespace(line, &i);
        Token tok = {.line = line_num};
        string_view val = {0};

        switch (cur) {
        //semicolons are ignored
        case ';': case ',':  next(); break;
        //comments
        case '#': skip_until('\n'); break;
        case '/': {
            if(peek(1) == '/') {
                skip_until('\n');
            } else if(peek(1) == '*') {
                i += 2;
                while (in_range()) {
                    if(cur == '*' && peek(1) == '/') {
                        i += 2;
                        break;
                    }
                    if(cur == '\n') line_num += 1;
                    next();
                }

                if(i >= line->length)
                    cate_error("unterminated comment");
            }
            else goto bad;
        }   break;

        case '"': {
            tok.kind = CTOK_STRING_LITERAL;
            size_t begin = next();

            while_in(cur != '"') {
                if(cur == '\n') line_num += 1;
                next();
            }
            if(i >= line->length)
                cate_error("unterminated string");

            val = sv_substring(line, begin, i);
            // cur = 0; //null terminate 
            next();
            save();
        }   break;

        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_': {
            tok.kind = CTOK_IDENTIFIER;
            const size_t begin = i;
            while_in (isalnum(cur) || cur == '_')
                next();
            val = sv_substring(line, begin, i);

            tok.kind = maybe_keyword(&val);
            if(tok.kind != CTOK_IDENTIFIER) {
                da_append(*tokens, tok);
            } else {
                save();
            }
        }   break;

        case ' ': case '\t': case '\r': case '\n': case '\f': case '\v':
            while(isspace(cur)) {
                if(cur == '\n') ++line_num;
                next();
            }
            break;

        #define quick(ch, type) case ch: {\
        tok.kind = type;\
        next();\
        da_append(*tokens, tok);\
        } break;

        quick('.', CTOK_DOT)
        quick('=', CTOK_ASSIGN)
        quick('{', CTOK_LCURLY)
        quick('}', CTOK_RCURLY)
        quick('(', CTOK_LPAREN)
        quick(')', CTOK_RPAREN)
        quick('!', CTOK_EXCLAMATION_MARK)

        case 0: return; break;

        default:
        bad:
            cate_error_line(line_num, "no such token '%c'\n", cur);
            break;
    }
    }

    #undef save
}

const char* ctok_as_text(TokenKind k) {
    static_assert(CTOK_COUNT_SIZE == 18,
        "added token types? add their names here");
    static const char* const names[CTOK_COUNT_SIZE] = {
        [CTOK_NONE] = "end of file",
        [CTOK_DOT] = "a '.'",
        [CTOK_ASSIGN] = "an '='",
        [CTOK_LCURLY] = "a '{'",
        [CTOK_RCURLY] = "a '}'",
        [CTOK_LPAREN] = "a '('",
        [CTOK_RPAREN] = "a ')'",
        [CTOK_PROJECT] = "'Project'",
        [CTOK_LIBRARY] = "'Library'",
        [CTOK_STATIC] = "'static'",
        [CTOK_DYNAMIC] = "'dynamic'",
        [CTOK_STRING_LITERAL] = "a string",
        [CTOK_IDENTIFIER] = "an identifier",
        [CTOK_TRUE] = "'true'",
        [CTOK_FALSE] = "'false'",
        [CTOK_IF] = "'if'",
        [CTOK_ELSE] = "'else'",
        [CTOK_EXCLAMATION_MARK] = "an '!'",
    };

    if(k >= CTOK_COUNT_SIZE) return "this is a bug";
    return names[k];
}

/*------.
| catel |
`-----*/
void catel_parse(string_view* line, Catel* catel) {
    size_t i = 0;
    _CateSysPath* to_edit = 0;
    string_view property = {0}, value = {0};
    while (in_range()) {
        while_in(isspace(cur)) next();
        if(!cur) break;
        //parse property
        if(!property.length) {
            size_t begin = i;
            while_in(isalpha(cur)) next();
            property = sv_substring(line, begin, i);

            if(sv_ccmp(&property, "dir") || sv_ccmp(&property, "directory")) {
                to_edit = &catel->dir;
            } else if(sv_ccmp(&property, "def")
                || sv_ccmp(&property, "default")) {
                to_edit = &catel->def;
            } else {
                cate_error("catel does not have \"" sv_fmt "\"\n"
                            "available are: def, default, dir, directory",
                    sv_p(property));
            }
            continue;
        }

        //get the value
        if(cur == '"') {
            next();
            size_t begin = i;
            while_in(cur != '"') next();
            if(!in_range()) {
                cate_error("unterminated string in catel!");
            }
            value = sv_substring(line, begin, i);
            next();
        } else {
            size_t begin = i;
            while_in(!isspace(cur)) next();
            value = sv_substring(line, begin, i);
        }

        if(property.length && value.length) {
            if(!to_edit) {
                cate_error("invalid catel state?");
            }
            if(value.length+1 >= FILENAME_MAX) {
                cate_error("catel value too long for this platform");
            }
            memcpy(to_edit->x, value.text, value.length);
            to_edit->x[value.length] = 0;

            //reset them for next pass
            property.length = 0;
            value.length = 0;
            to_edit = 0;
        }
    }
    #undef cur

    if(property.length) {
        cate_error("catel expects a value for \"" sv_fmt "\"",
                    sv_p(property));
    }

    #undef cur
    #undef next
    #undef peek
    #undef in_range
    #undef while_in
    #undef skip_until
}

void catel_init(Catel* catel) {
    if(file_exists("cate")) {
        memcpy(catel->dir.x, "cate", 5);
    }
    if(!file_exists(".catel")) return;

    string_view file = {0};
    if(sv_load_file(&file, ".catel")) {
        cate_error("failed to open catel file!");
    }

    catel_parse(&file, catel);

    free(file.text);
}

/*--------.
| context |
`-------*/
void cate_context_destroy(CateContext* context) {
    free(context->classes.data);
    free(context->opened_files.data);
}