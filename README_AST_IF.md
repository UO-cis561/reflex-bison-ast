# Adding a program construct

## Order of development

Compiler projects in class are highly weird in one way:
We typically build the whole scanner, then the whole parser without
AST, then actions to build the AST, then the type checker, then 
the code generator.   No reasonable developer would approach a real 
compiler project that way.  

What would we do in a real project?  What should you do when you
develop a translator for a project that is not for a compiler construction
course?   Build out, incrementally from a tiny core language. 

An incremental development plan is a challenging but necessary part of any
significant software project.  It's often difficult to make a good 
judgment about an order of construction that optimizes two things: 

* Minimizing risk by prioritizing features or aspects that are 
least certain.  (Do things that could damage your schedule and 
 design first, to give yourself maximum 
room to adjust if they don't go as planned.)  
* Ensure that the developing system can be tested frequently after 
very short development steps, without investing more than is 
necessary in creating test infrastructure. 

With programming language translators (compilers, interpreters,
source-to-source transformers, etc) we at least know an approach
that is pretty good at letting us test end-to-end functionality very 
incrementally.  We don't start by building any part of the translator
for the whole language.  Instead, we build most of the translator (perhaps
leaving out some parts that are not required for testing) for a tiny 
subset of the language.  Then each incremental development step
adds to each part of the translator for some small addition to the 
language.  

## So what's this about?

I started this project with just a language of arithmetic expressions.
In fact, I started with just addition.  The first version could interpret
x + 7 + 3, as long as x was always 0  (since I hadn't added any code 
for keeping values in variables).  It couldn't even subtract.
Then when I included subtraction, multiplication, and division, at 
first there were no precedence rules, so 5 - 3 * 2 evaluated to 4 rather
than -1.  Then I added precedence.  And then assignments and variasble
storage.  And so on.  

Now I am going to add 'if/elif/else/fi'.  I have chosen this construct
next because the abstract syntax tree (AST) for this construct differs
from the concrete syntax.  There is no 'elif' in the abstract syntax.
Also, the 'else' clause is optional in the concrete syntax, but in 
the abstract syntax tree the 'else' part will always be present.  
(It may contain an empty block of statements.)  The difference bertween
a parse tree and an abstract syntax tree is sometimes a little puzzling
to students, as well as adding some challenge to writing action 
routines in the parser, so it seems like a good thing to illustrate with 
an example. 

This document is where I will capture a record of my steps
(and perhaps miss-steps) in adding this construct to the tiny example
language. 

## The grand plan

I plan to approach this in the following order: 

* Abstract syntax tree classes first.   This is because I can compile 
and perhaps even test this bit of AST without involving the parser 
or scanner, but I can't write the parser action routes (in the .yxx file)
until I have the AST classes, and I can't write the scanner pattern
and action rule until I have the parser.  So _AST first_ is the order 
that minimizes the time gap between checks that I haven't totally screwed up. 

* Parser next.  I can't test the parser until I have the scanner, 
but I can at least compile it with a scanner that will never produce 
the tokens needed for the new construct. 

* Scanner last.  But it's very simple, so it's hardly an issue. 

I might alter this just a little:  Maybe the first version will 
just be a simple if/else/fi, and then I'll add elif clauses and 
make the else part optional.  That would again minimize the gap 
between soundness checks.  

## Step 1: Adding the AST classes

The AST will support only if/else, so there will be an If node 
with three children:  condition, truepart, falsepart. Condition
will be an expression (and for now I can treat zero as false
and other values as true).  The truepart and falsepart will be 
blocks of statements.  Since I have expressions and blocks already, 
I should need just one more node type. 

This much at least compiles (although I really need to rethink
the str() methods and consider building a real pretty-printer): 

```
    class If : public ASTNode {
        ASTNode &cond_; // The boolean expression to be evaluated
        Block &truepart_; // Execute this block if the condition is true
        Block &falsepart_; // Execute this block if the condition is false
    public:
        explicit If(ASTNode &cond, Block &truepart, Block &falsepart) :
            cond_{cond}, truepart_{truepart}, falsepart_{falsepart} { };
        std::string str() override {
            return "if " + cond_.str() + ":\n" +
                truepart_.str() + "\n" +
                falsepart_.str() + "\n";
        }
        int eval(EvalContext& ctx) override;

    };

```

Should I believe the 'eval' part now or move straight on to 
the parser and scanner?  Maybe it's simple enough to go now. 
(In our Quack compiler, the 'eval' for conditional branching 
will have some iteresting twists and turns and will be worth
a separate incremental development step.)

Easy peasy.  In ASTNode.cpp: 

```
    // An 'if' statement, in this initial cut, evaluates its condition to an integer
    // and chooses the true (then) part or the false (else) part depending on whether
    // the integer is zero.
    int If::eval(EvalContext &ctx) {
        int cond = cond_.eval(ctx);
        // Might as well use C's ill-considered interpretation of ints as booleans
        if (cond) {
            return truepart_.eval(ctx);
        }
        return falsepart_.eval(ctx);
    }
}

```

## Step 2: The parser, simplified case

On to the parser!  (Which should be the hard part, if it's hard.)  I'll probably start by building concrete syntax that exactly matches the abstract syntax. 

First question:  Where does it go in the grammar?  So far I have programs made up of statements, and statements being assignments and expressions: 

```
stmt: assignment { $$ = $1; }
    | expr { $$ = $1; }
    ;

```

So this is simple ... we have a new kind of statement, which is an 'if' statement.  (In an expression-oriented language, 'if' might be a kind of expression instead.) 

```
stmt: assignment { $$ = $1; }
    | expr { $$ = $1; }
    | ifstmt { $$ = $1; }
    ;
```

We'll start simply, making the 'if' statement match the abstract syntax: 

```
ifstmt:  IF expr THEN block ELSE block FI {  };
```

We'll need lexical elements (tokens) for IF, THEN, ELSE, FI, but I'll deal with that in the scanner.  For now I just need to tell the parser that these are tokens.  I'll make them be string-valued tokens, even though I have no use for the strings. 

```
// The following token values are not actually used ---
//   all we really need is the name of the token,
//   but we'll associate the matched text_ with them just
//   for simplicity
%token <str> PLUS  MINUS TIMES DIV GETS
%token <str> IF THEN ELSE FI
```

If I try compiling the project at this point, I expect error messages because the action associated with the 'ifstmt' grammar rule is empty, but let's try it just to make sure I haven't messed up some simple bit of bison syntax. 

And that turns out to be useful, because my IDE Clion has over-zealously translated the bison associativity directives %left to %left_ when I was renaming fields in the AST nodes.  Easy to fix. 

Now we get an error message that is actually relevant to the code I just added:  I forgot to tell bison what semantic value type the non-terminal 'ifstmt' would have.  

```
[ 11%] Generating calc.tab.cxx, calc.tab.hxx, location.hh, position.hh, stack.hh
/Users/michal/Dropbox/18F-Compilers/dev/reflex-bison-ast/calc.yxx:80.21-22: error: $1 of ‘stmt’ has no declared type
     | ifstmt { $$ = $1; }
```

So I'll add that: 

```
%type <node> ifstmt
```

To my surprise, the code now builds without errors.  That's because bison has helpfully (mis-)applied a default rule ... it will probably return the IF token as the semantic value of the whole statement.  I could prevent that by giving a more specific token type for ifstmt, but if I do that a lot I'll end up with a very complex %union for the semantic types.  I'll give up a little error checking to keep it simple and just use ASTNode (the `<node>` alternative) as the semantic type. 

Time to add an AST-building action!  I think it's going to be simple (for now): 

```
ifstmt:  IF expr THEN block ELSE block FI
 {    $$ = new AST::IF($2, $4, $6);  }
 ;
```

It's simple so far only because I have matched the concrete syntax to the abstract syntax ... and the whole point of this exercise is to break down a more complex concrete syntax into a simple abstract syntax.  But walk first, run later ... let's try to build this. 

Misspelled "If" as "IF".  Fixed.  Also forgot to take the pointer value on a *ASTNode to get an &ASTNode.  Fixed.  Now the code is: 

```
ifstmt:  IF expr THEN block ELSE block FI
 {    $$ = new AST::If(*$2, *$4, *$6);  }
 ;
```

These are stupid little errors, and they make me glad that I am starting with the simplified concrete syntax rather than dealing with them when I'm trying to deal with the more complex pattern. 

So now I have a clean compile, but I can't process any patters with if/else/fi in them because I am not recognizing the tokens I need.  So let's do that next ... 

## Step 3: Token patterns in scanner

Next we turn attention to calc.lxx, the scanner (lexer) source code.  Keywords are super simple to recognize.  Of course they can be textually different than the names of the tokens.  I'll make THEN be ':', as in Python: 

```
if   { yylval.str = strdup(text()); return yy::parser::token::IF; }
\:   { yylval.str = strdup(text()); return yy::parser::token::THEN; }
else   { yylval.str = strdup(text()); return yy::parser::token::ELSE; }
fi     { yylval.str = strdup(text()); return yy::parser::token::FI; }
```

But RE/flex is not happy!  It says: 

```
[ 11%] Generating lex.yy.cpp, lex.yy.h
/Users/michal/Dropbox/18F-Compilers/dev/reflex-bison-ast/calc.lxx:21: warning: rule cannot be matched because a previous rule subsumes it, perhaps try to move this rule up?
/Users/michal/Dropbox/18F-Compilers/dev/reflex-bison-ast/calc.lxx:23: warning: rule cannot be matched because a previous rule subsumes it, perhaps try to move this rule up?
/Users/michal/Dropbox/18F-Compilers/dev/reflex-bison-ast/calc.lxx:24: warning: rule cannot be matched because a previous rule subsumes it, perhaps try to move this rule up?
Scanning dependencies of target parser
```

This is because I have a rule for identifiers that will match all of these keywords except for the ':': 

```
[[:alnum:]]+  { yylval.str = strdup(text()); return yy::parser::token::IDENT; }
```

The advice RE/flex gives me to move the definitions of keywords up is correct:  I will place all the keywords and punctuation _before_ the general rules for identifiers and numbers: 

```
\+  { yylval.str = strdup(text()); return yy::parser::token::PLUS; }
\*  { yylval.str = strdup(text()); return yy::parser::token::TIMES; }
\-  { yylval.str = strdup(text()); return yy::parser::token::MINUS; }
\/  { yylval.str = strdup(text()); return yy::parser::token::DIV; }
if   { yylval.str = strdup(text()); return yy::parser::token::IF; }
\:   { yylval.str = strdup(text()); return yy::parser::token::THEN; }
else   { yylval.str = strdup(text()); return yy::parser::token::ELSE; }
fi     { yylval.str = strdup(text()); return yy::parser::token::FI; }

[[:digit:]]+  { yylval.num = atoi(text());   return yy::parser::token::NUMBER; }
[[:alnum:]]+  { yylval.str = strdup(text()); return yy::parser::token::IDENT; }
```

And we're in business: 

```
[100%] Built target parser
```

Before I try a sample program with the new syntax, let's make sure I haven't broken what was working before.  I'll just run my existing test case without adding an 'if'.  The test case input is 

```
x = 2
y = 3
z = 4
x+y*z+5
```

I have a configured script in my IDE to run it through my driver file, which is salted with print statements. It says: 

```
/bin/bash run.sh cmake-build-debug/parser sample.txt
Running
Successfully parsed x = 2;;
y = 3;;
z = 4;;
((x + (y * z)) + 5);

Evaluates to 19
Done

Process finished with exit code 0
```

OK, we haven't broken what was working.  Time to see if we got the 'eval' and 'str' methods right by exercising them with a test input.  I'll edit it ... 

```
a = 0
b = 42
if a:
   x = 2
else 
   x = 3
fi
if b:  y = 3 else y = 4 fi
z = 4
x+y*z+5
```

I expect the expression to be evaluated with x=3 and y=4, so it should be 3+(4*4)+5 = 24.  What are the chances? 

```
/bin/bash run.sh cmake-build-debug/parser sample.txt
Running
Successfully parsed a = 0;;
b = 42;;
if a:
x = 2;;

x = 3;;

;
if b:
y = 3;;

y = 4;;

;
z = 4;;
((x + (y * z)) + 5);

Evaluates to 20
Done
```

For starts, the str() method I use for printing if statements is definitely broken.  Also I'm getting the wrong result.  But aside from that, things are looking peachy. 

I'll switch to a C style syntax for printing, although it's way too simple-minded to indent properly. 

```
       std::string str() override {
            return "if " + cond_.str() + " {\n" +
                truepart_.str() + "\n" +
                "} else {\n" +
                falsepart_.str() + "\n" +
                "}\n";
```

And this now prints the AST as 

```
a = 0;;
b = 42;;
if a {
x = 2;;

} else {
x = 3;;

}
;
if b {
y = 3;;

} else {
y = 4;;

}
;
z = 4;;
((x + (y * z)) + 5);
```

Not pretty, but good enough.  But what's with the broken eval?  Or is it broken?  Let's see ... x should be 3, y should be 3, z is 4, so 3+3*4+5 is 3+12+5 is 20. Duh. Eval seems to be working. 

## Step 4: Back to Bison

After that extensive testing (ok, after a single working test case) it is finally time to tackle a difference between concrete syntax and abstract syntax.  (And at this point I took a break to move from home to school.) 

(A few hours pass here while I take care of other things)

OK ... sketched out grammar for how parsing if/then/elif/elif/.../else/fi can build AST for nested if statements.  This might almost be too easy.  The basic plan is: 

```
<ifstmt> ::= if <cond> then <block> <alternatives> fi
   with <alternatives> returning a block to be used like 
   the current falsepart block. 
<alternatives> ::= empty  { return empty block }
<alternatives> ::= else <block>  { return block }
<alternatives> ::= elif <cond> then <block> <alternatives>
  { build an if/else statement from cond, block, alternatives, 
    wrap it in a block, and return that. }
```

Wrapping the if/else in a block seems unnecessary, but I defined the truepart and falsepart of the If node as Block rather than ASTNode, so 
I'll go with that for now ... later we might want to simplfy the 
AST by reorganizing it a little to let individual statements be blocks. 

In bison: 

```
ifstmt:  IF expr THEN block if_alternatives FI
 {    $$ = new AST::If(*$2, *$4, *$5);  } ;

if_alternatives:   /* empty */  { $$ = new AST::Block(); };
if_alternatives:   ELSE block   { $$ = *$2; };
if_alternatives:   ELIF cond block if_alternatives
 {  $$ = new AST::Block();
    $$->append(new AST::If(*$2, *$3, *$4);
 };
 ```
 
 Bison warns me that I need to tell it more about the new tokens and non-terminals in this part of the grammar, and I've also used 'cond' where I should have just used 'expr'.  Adding the new declarations ...  
 
```
%token <str> IF THEN ELSE FI ELIF

// Abstract syntax tree nodes
%type <node> expr leaf program stmt assignment
%type <block> block
%type <node> ifstmt if_alternatives
```

But this isn't quite right ... I've given <if_alternatives> a type of 
AST::ASTNode, but my class definition in AST.h says it needs to be an AST::Block.   I could change either AST.h or calc.yxx.  For now I'll keep the more specific class declaration and fix the type in bison: 

```
// Abstract syntax tree nodes
%type <node> expr leaf program stmt assignment
%type <block> block if_alternatives
%type <node> ifstmt  
```

Almost there!  I got too much in the habit of dereferencing pointers and 
`{ $$ = *$2; };` should be `{ $$ = $2}` in the production for the final _else_ clause.  Also I missed a paren.  So now it's 

```
if_alternatives:   /* empty */  { $$ = new AST::Block(); };
if_alternatives:   ELSE block   { $$ = $2; };
if_alternatives:   ELIF expr block if_alternatives
 {  $$ = new AST::Block();
    $$->append(new AST::If(*$2, *$3, *$4));
 };
```

and once again I test that the old code was working before I try a run with data requiring the new grammar.  (It works.) 

All that is left (maybe!) is to test some code that includes elif and possibly omits an else.  

```
a = 0
b = 42
if a:
   x = 2
elif b:
   x = 3
   if b - b:
     y = 2
   elif a
     y = 3
   else
     y =4
   fi
fi
z = 4
if b - b: z = 5 fi
x+y*z+5
```

That syntax is horrible, but for now I just want it to work.  This should go down the first 'elif' branch, setting x to 3, then the else branch to set y to 4.  After setting z to 4, it should take the (empty) else branch of the second if because b-b is zero.  So x=3, y=4, z=4, and `x+y*z+5` should be `3+4*4+5` which is `3+16+5` which is 24.  Crossing fingers. 

And I got 5 instead.  

But why did I get 5?  OH, here's the real error message: 

```
/bin/bash run.sh cmake-build-debug/parser sample.txt
Running
syntax error at 5.5
Successfully parsed a = 0;;
b = 42;;
if a {
...
```
And line 5 of the input is the 'elif'.   This tells me two things: 

* I forgot to define an 'elif' token in the scanner, and 
* The error messages from my parser stink

I'll take care of only the first problem for now. 

## Back to RE/flex

This is really simple ... we just need one more token: 

```
elif   { yylval.str = strdup(text()); return yy::parser::token::ELIF; }
```

But I still get that syntax error in line 5.  Could my input actually be wrong?  Um, yeah.  The grammar doesn't take a 'then' after 'elif'.  But maybe it should, for consistency with the 'then' after 'if'.  And maybe I should spell it 'then' instead of ':'.  So I'll fix the scanner: 

```
then   { yylval.str = strdup(text()); return yy::parser::token::THEN; }
```

and fix the parser to allow the 'then' there: 

```
if_alternatives:   ELIF expr THEN block if_alternatives
 {  $$ = new AST::Block();
    $$->append(new AST::If(*$2, *$4, *$5));
 };
```

and finally fix the test program to use the new spelling: 

```
a = 0
b = 42
if a then
   x = 2
elif b then
   x = 3
   if b - b then 
     y = 2
   elif a then
     y = 3
   else
     y =4
   fi
fi
z = 4
if b - b then z = 5 fi
x+y*z+5
```

And now I get 24!  

Also I get a really terrible printed form of the program, and I've learned that my error messages are terrible, so there is plenty of work left to do, but this concludes the exercise of building some AST that is simpler than the concrete grammar. 



