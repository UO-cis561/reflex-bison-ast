# Json to Dot

'Dot' is a little language for drawing directed graphs.  It is 
the input language of the 'dot' program, which is part of the 
graphviz open source graph visualization package developed originally 
at Bell Laboratories.    Dot programs are also accepted as input by 
OmniGraffle, a popular graph editing program for MacOS.  There may 
be other programs I don't know about that accept the dot language. 

The AST structure is currently a tree that can be dumped as a 
JSON structure. json_to_dot.py can translate this JSON structure
into the dot language.  The dot file can then be processed by 
the 'dot' program from graphviz, or read into OmniGraffle. 

As a practical matter, the AST representation of a moderate size program will 
be far too complex to usefully visualize as a tree. If you want to use 
this to see the tree structure, you will need to create a very, very small
Quack program for just the part of the tree structure you want to 
visualize.  

## Wish list

There might be ways to compress the visual representation a bit ... 
but I'm not sure exactly how. 

It would be useful to be able to translate just a portion of the 
JSON structure.  Instead of writing a tiny Quack program to visualize, 
I'd like to be able to specify which part of a larger program to visualize, 
e.g., "the third IF statement, two levels down."  This seems doable. 

The program currently doesn't have a real command line interface ... it 
assumes it has one command line argument which is a path to a file. 
It needs the argparse module to give reasonable command line parsing. 