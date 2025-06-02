if exists("b:current_syntax")
  finish
endif

let b:current_syntax = "evalset"

syntax keyword evalsetKeyword true false nil
highlight link evalsetKeyword Keyword

syntax match evalsetComment "#.*$"
highlight link evalsetComment Comment

syntax match evalsetString /".*"/
highlight link evalsetString String

syntax match esInteger "\-\?\<\d\+\>" display
syntax match esFloat "\-\?\<[0-9][0-9_]*\%(\.[0-9][0-9_]*\)\%([eE][+-]\=[0-9_]\+\)\=" display
syntax match esPath "\$\(/[A-z_]*\)\+" display
syntax match esAssign "="
syntax match esPlus "+"
syntax match esStar "*"

highlight link esFloat Float
highlight link esInteger Number
highlight link esPath Identifier
highlight link esAssign Operator
highlight link esPlus Operator
highlight link esStar Operator

let b:current_syntax = "evalset"

