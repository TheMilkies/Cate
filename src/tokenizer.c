#include "tokenizer.h"
#include <ctype.h>
#include <stdlib.h>
#include "common.h"
#include "error.h"

static void maybe_keyword(Token* t) {
    static_assert(TOK_COUNT_SIZE == 19,
        "added token types? if they are keywords; add them here");
    string_view *v = &t->text;
    if(sv_ccmp(v, "Project")) {
        t->kind = TOK_PROJECT;
    } else if(sv_ccmp(v, "Library")) {
        t->kind = TOK_LIBRARY;
    } else if(sv_ccmp(v, "recursive")
           || sv_ccmp(v, "iterate")) {
        t->kind = TOK_RECURSIVE;
    } else if(sv_ccmp(v, "static")) {
        t->kind = TOK_STATIC;
    } else if(sv_ccmp(v, "dynamic")) {
        t->kind = TOK_DYNAMIC;
    } else if(sv_ccmp(v, "true")) {
        t->kind = TOK_TRUE;
    } else if(sv_ccmp(v, "false")) {
        t->kind = TOK_FALSE;
    } else if(sv_ccmp(v, "if")) {
        t->kind = TOK_IF;
    } else if(sv_ccmp(v, "else")) {
        t->kind = TOK_ELSE;
    }
}

uint8_t cate_tokenize(string_view *line, TokensArray *tokens) {
    static_assert(TOK_COUNT_SIZE == 19,
        "added token types? if they are not keywords; add them here");
    size_t i = 0, line_num = 1;
    uint8_t has_error = 0;
    #define cur line->text[i]
    #define peek(n) line->text[i+n]
    #define while_line(cond) while (i < line->length && (cond))
    #define skip_until(ch) while_line(cur != ch) ++i;
    while (i < line->length) {
        switch (cur) {
        //semicolons are ignored
        case ';': case ',': ++i; break;
        //comments
        case '#': skip_until('\n'); break;
        case '/': {
            if(peek(1) == '/') {
                skip_until('\n');
            } else if(peek(1) == '*') {
                i += 2;
                while (i < line->length) {
                    if(cur == '*' && peek(1) == '/') {
                        i += 2;
                        break;
                    }
                }

                if(i >= line->length) {
                    cate_error("unterminated comment");
                }
            }
            else goto bad;
        }   break;

        case '"': {
            size_t begin = ++i;
            while_line (cur != '"')
                ++i;

            //skip the '"' and null terminate
            //UNIX-like OSs expect null-terminated things in calls.
            cur = 0;
            
            Token result = {
                .kind = TOK_STRING_LITERAL,
                .line = line_num,
                .text = sv_substring(line, begin, i)
            };
            ++i;
            da_append((*tokens), result);
        }   break;

        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_': {
            const size_t begin = i;
            while (isalnum(cur) || cur == '_')
                ++i;
            
            Token result = {
                .kind = TOK_IDENTIFIER,
                .line = line_num,
                .text = sv_substring(line, begin, i)
            };
            maybe_keyword(&result);
            da_append((*tokens), result);
        }   break;

        case ' ': case '\t': case '\r': case '\n': case '\f': case '\v':
            while(isspace(cur)) {
                if(cur == '\n') ++line_num;
                ++i;
            }
            break;

#define quick(ch, type) case ch: {\
    Token result = {\
        .kind = type,\
        .line = line_num,\
        .text =\
        {.length = 1, .text = line->text+i}};\
    ++i;\
    da_append((*tokens), result);\
} break;

        quick('.', TOK_DOT)
        quick('=', TOK_ASSIGN)
        quick('{', TOK_LCURLY)
        quick('}', TOK_RCURLY)
        quick('(', TOK_LPAREN)
        quick(')', TOK_RPAREN)
        quick('!', TOK_EXCLAMATION_MARK)

        case 0: return 0; break;
        default: {
        bad:
            cate_error_line(line_num, "no such token '%c'\n", cur);
            has_error = 1;
            ++i;
        }   break;
        }
    }
    return has_error;
}

const char* tok_as_text(TokenKind k) {
    static_assert(TOK_COUNT_SIZE == 19,
        "added token types? add their names here");
    static const char* const names[TOK_COUNT_SIZE] = {
        [TOK_NONE] = "end of file",
        [TOK_DOT] = "a '.'",
        [TOK_ASSIGN] = "an '='",
        [TOK_LCURLY] = "a '{'",
        [TOK_RCURLY] = "a '}'",
        [TOK_LPAREN] = "a '('",
        [TOK_RPAREN] = "a ')'",
        [TOK_PROJECT] = "'Project'",
        [TOK_LIBRARY] = "'Library'",
        [TOK_STATIC] = "'static'",
        [TOK_DYNAMIC] = "'dynamic'",
        [TOK_RECURSIVE] = "'recursive'",
        [TOK_STRING_LITERAL] = "a string",
        [TOK_IDENTIFIER] = "an identifier",
        [TOK_TRUE] = "'true'",
        [TOK_FALSE] = "'false'",
        [TOK_IF] = "'if'",
        [TOK_ELSE] = "'else'",
        [TOK_EXCLAMATION_MARK] = "an '!'",
    };

    if(k >= TOK_COUNT_SIZE) return "this is a bug";
    return names[k];
}