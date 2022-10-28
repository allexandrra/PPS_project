# PPS_project


## BNF Grammar of configuration file

```bnf
"network" ::=  <setter>  "(" <list-AS> ");" 
<list-AS> ::= ( <AS>  [*("," <AS>)] ) | <empty> 
<AS> ::= "{" <AS-name> [<AS-links>] <AS-ip> <AS-netmask>  "};"
<AS-name> ::= "name" <setter> <name> ";"
<AS-links> ::= "links" <setter> "[" <name> [*("," <name>)] "]" ";"
<AS-ip> ::= "ip" <setter> <name> 
<AS-netmask> ::= "netmask" <setter> <name> 
<setter> ::= (":" | "=" )
<name>	::= [A-Za-z\*][-A-Za-z0-9_\*]*
<integer>	::= [-+]?[0-9]+
<empty> ::= 
```

## New format Message : TRUSTRATE

TRUST: 4 bytes
Define the inherent trust of one 