# PPS_project


## BNF Grammar of configuration file

```bnf
"network" ::=  <setter>  "(" <list-AS> ");" 
<list-AS> ::= ( <AS>  [*("," <AS>)] ) | <empty> 
<AS> ::= "{" <AS-name> [<AS-links>] [<AS-routing_policies>]  "};"
<AS-name> ::= "name" <setter> <name> ";"
<AS-links> ::= "links" <setter> "[" <name> [*("," <name>)] "]" ";"
<AS-routing_policies> ::= "routing_policies" <setter> "(" <policy> [*("," <policy>)] | <empty> ");"
<policy>::= "{" <policy-destination>  <policy-netmask> <policy-gateway> <policy-interface> <policy-metric> "};" 
<policy-destination> ::= "policy-destination" <setter> <name> 
<policy-netmask> ::= "policy-netmask" <setter> <name> 
<policy-gateway> ::= "policy-gateway" <setter> <name> 
<policy-interface> ::= "policy-interface" <setter> <name> 
<policy-metric> ::= "policy-metric" <setter> <integer>
<setter> ::= (":" | "=" )
<name>	::= [A-Za-z\*][-A-Za-z0-9_\*]*
<integer>	::= [-+]?[0-9]+
<empty> ::= 
```