if exists("b:did_indent")
  finish
endif
let b:did_indent = 1

setlocal nosmartindent
setlocal nolisp
setlocal autoindent

setlocal indentexpr=GetEsIndent(v:lnum)

if exists("*GetEsIndent")
  finish
endif

function! GetEsIndent(lnum)
  let prev = prevnonblank(a:lnum-1)

  if prev == 0
    return 0
  endif

  let prevline = getline(prev)
  let line = getline(a:lnum)

  let ind = indent(prev)

  if prevline =~ '[\[{]\s*$'
    let ind += &sw
  endif

  if line =~ '^\s*[\]}]'
    let ind -= &sw
  endif

  return ind
endfunction

