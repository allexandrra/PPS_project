# PPS_project


## BNF Grammar of configuration file

```bnf
"network" ::=  <setter>  "(" <list-AS> ");" 
<list-AS> ::= ( <AS>  [*("," <AS>)] ) | <empty> 
<AS> ::= "{" <AS-name> [<AS-links>]  "};"
<AS-name> ::= "name" <setter> <name> ";"
<AS-links> ::= "links" <setter> "[" <name> [*("," <name>)] "]" ";"
<setter> ::= (":" | "=" )
<name>	::= [A-Za-z\*][-A-Za-z0-9_\*]*
<integer>	::= [-+]?[0-9]+
<empty> ::= 
```