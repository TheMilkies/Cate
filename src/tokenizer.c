#include "tokenizer.h"
#include <ctype.h>

static TokenKind maybe_keyword(string_view* v) {
    static_assert(TOK_COUNT_SIZE == 19,
        "added token types? if they are keywords; add them here");
    if(sv_ccmp(v, "Project")) {
        return TOK_PROJECT;
    } else if(sv_ccmp(v, "Library")) {
        return TOK_LIBRARY;
    } else if(sv_ccmp(v, "recursive")
           || sv_ccmp(v, "iterate")) {
        return TOK_RECURSIVE;
    } else if(sv_ccmp(v, "static")) {
        return TOK_STATIC;
    } else if(sv_ccmp(v, "dynamic")) {
        return TOK_DYNAMIC;
    } else if(sv_ccmp(v, "true")) {
        return TOK_TRUE;
    } else if(sv_ccmp(v, "false")) {
        return TOK_FALSE;
    } else if(sv_ccmp(v, "if")) {
        return TOK_IF;
    } else if(sv_ccmp(v, "else")) {
        return TOK_ELSE;
    }
    return TOK_IDENTIFIER;
}

void cate_tokenize(string_view *line, TokensArray *tokens,
                        TokenValuesArray* values) {
    static_assert(TOK_COUNT_SIZE == 19,
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
                    next();
                }

                if(i >= line->length)
                    cate_error("unterminated comment");
            }
            else goto bad;
        }   break;

        case '"': {
            tok.kind = TOK_STRING_LITERAL;
            size_t begin = next();

            skip_until('"');
            if(i >= line->length)
                cate_error("unterminated string");

            val = sv_substring(line, begin, i);
            next();
            save();
        }   break;

        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_': {
            tok.kind = TOK_IDENTIFIER;
            const size_t begin = i;
            while_in (isalnum(cur) || cur == '_')
                next();
            val = sv_substring(line, begin, i);

            tok.kind = maybe_keyword(&val);
            save();
        }   break;

        case ' ': case '\t': case '\r': case '\n': case '\f': case '\v':
            while(isspace(cur)) {
                if(cur == '\n') ++line_num;
                next();
            }
            break;

        #define quick(ch, type) case ch: {\
        tok.kind = type;\
        val.length = 1;\
        val.text = line->text+i;\
        next();\
        save();\
        } break;

        quick('.', TOK_DOT)
        quick('=', TOK_ASSIGN)
        quick('{', TOK_LCURLY)
        quick('}', TOK_RCURLY)
        quick('(', TOK_LPAREN)
        quick(')', TOK_RPAREN)
        quick('!', TOK_EXCLAMATION_MARK)

        case 0: return; break;

        default:
        bad:
            cate_error_line(line_num, "no such token '%c'\n", cur);
            break;
    }
    }
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
