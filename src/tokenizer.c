#include "tokenizer.h"
#include <ctype.h>

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
                while (i < line->length) {
                    if(cur == '*' && peek(1) == '/') {
                        i += 2;
                        break;
                    }
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