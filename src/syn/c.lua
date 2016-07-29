--
-- Syntax Highlighting for C
--

local mymodule = {}

--
--  Start of position
--
local start = 0
local global = ""

function syntafx_range(a,b,col)
   local str = string.sub(global, a, b)

   -- Opens a file in append mode
   file = io.open("synt.in", "a")

   -- appends a word test to the last line of the file
   file:write( a .. "-" .. b .. "[" .. str .. "]" .. "-> " ..col .. "\n")
   -- closes the open file
   file:close()
end



local P, R, S, C = lpeg.P, lpeg.R, lpeg.S, lpeg.C

local whitespace = S' \t\v\n\f'
local digit = R'09'
local letter = R('az', 'AZ') + P'_'
local alphanum = letter + digit
local hex = R('af', 'AF', '09')
local exp = S'eE' * S'+-'^-1 * digit^1
local fs = S'fFlL'
local is = S'uUlL'^0

local hexnum = P'0' * S'xX' * hex^1 * is^-1
local octnum = P'0' * digit^1 * is^-1
local decnum = digit^1 * is^-1
local floatnum = digit^1 * exp * fs^-1 +
digit^0 * P'.' * digit^1 * exp^-1 * fs^-1 +
digit^1 * P'.' * digit^0 * exp^-1 * fs^-1
local numlit = hexnum + octnum + floatnum + decnum

local charlit =
P'L'^-1 * P"'" * (P'\\' * P(1) + (1 - S"\\'"))^1 * P"'"

local stringlit =
P'L'^-1 * P'"' * (P'\\' * P(1) + (1 - S'\\"'))^0 * P'"'

local inclit =
P'L'^-1 * P'<' * (P'\\' * P(1) + (1 - S'>'))^0 * P'>'

local ccomment = P'/*' * (1 - P'*/')^0 * P'*/'
local newcomment = P'//' * (1 - P'\n')^0
local comment = (ccomment + newcomment)
/ function(...) syntax_range(start, start+#..., 1) start = start + 1 + #... end

local literal = (numlit + charlit + stringlit)
/ function(...) syntax_range(start, start+#..., 2) start = start + 1 +  #... end

local include =               (inclit)
/ function(...) syntax_range(start, start+#..., 3) start = start + 1 + #... end


local preprocess = C(
P"#define" +
P"#ifdef" +
P"#include"
                    ) / function(...) syntax_range(start, start+#..., 3) start = start + 1 + #... end

local keyword = C(
P"auto" +
P"_Bool" +
P"break" +
P"case" +
P"char" +
P"_Complex" +
P"const" +
P"continue" +
P"default" +
P"do" +
P"double" +
P"else" +
P"enum" +
P"extern" +
P"float" +
P"for" +
P"goto" +
P"if" +
P"_Imaginary" +
P"inline" +
P"int" +
P"long" +
P"register" +
P"restrict" +
P"return" +
P"short" +
P"signed" +
P"sizeof" +
P"static" +
P"struct" +
P"switch" +
P"typedef" +
P"union" +
P"unsigned" +
P"void" +
P"volatile" +
P"while"
) / function(...) syntax_range(start, start+#..., 4) start = start + 1  + #... end

local identifier = (letter * alphanum^0 - keyword * (-alphanum))
/ function(...) syntax_range(start, start+#..., 5) start = start + 1 + #... end

local op = C(
P"..." +
P">>=" +
P"<<=" +
P"+=" +
P"-=" +
P"*=" +
P"/=" +
P"%=" +
P"&=" +
P"^=" +
P"|=" +
P">>" +
P"<<" +
P"++" +
P"--" +
P"->" +
P"&&" +
P"||" +
P"<=" +
P">=" +
P"==" +
P"!=" +
P";" +
P"{" + P"<%" +
P"}" + P"%>" +
P"," +
P":" +
P"=" +
P"(" +
P")" +
P"[" + P"<:" +
P"]" + P":>" +
P"." +
P"&" +
P"!" +
P"~" +
P"-" +
P"+" +
P"*" +
P"/" +
P"%" +
P"<" +
P">" +
P"^" +
P"|" +
P"?"
            )/ function(...) syntax_range(start, start+#..., 6) start = start + 1 + #... end

local tokens = (comment + identifier + keyword +
literal + include +preprocess + op + whitespace)^0

function mymodule.parse(input)
   start = 0
   global = input
   lpeg.match(tokens, input)
end

return mymodule
