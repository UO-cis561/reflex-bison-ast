#
# Python script to translate json dump of AST into the 'dot' input language
# of graphviz (also readable by OmniGraffle).
#

import json
from io import IOBase
import sys
from typing import List
from typing import Dict
from typing import Tuple
from typing import Union

import logging
logging.basicConfig()
log = logging.getLogger(__name__)
log.setLevel(logging.WARN)

PROLOGUE = "digraph AST {"
EPILOGUE = "}"

NODE_COUNT = 0

def gen_name() -> str:
    """Return a unique node name"""
    global NODE_COUNT
    NODE_COUNT += 1
    return f"node_{NODE_COUNT}"


def load(source : Union[str, IOBase]) -> Dict:
    """Loads dict structure from input that is either a
    file handle or a path to a file
    """
    if not isinstance(source, IOBase):
        source = open(source, "r")
    return json.load(source)

def dump_node(node: Union[dict,list], name: str, to_stream: IOBase = sys.stdout):
    if isinstance(node,dict):
        dump_structure(node, name, to_stream)
    elif isinstance(node,list):
        dump_list(node, name, to_stream)
    else:
        dump_leaf_value(node, name, to_stream)

def dump_leaf_value(node, name: str, to_stream: IOBase):
    """Not a dict or list; should be string or integer"""
    # TBD: Maybe these should be moved into their parent node?
    log.debug(f"A leaf with value {node}")
    print(f"""{name}[shape=plaintext,label="{node}"];""", file=to_stream)

def dump_structure(node: dict, name: str, to_stream: IOBase):
    """Dump a representation of the node 'root' (a dict) as a node with name 'name',
    plus all the descendents of root in the AST structure.
    """
    log.debug(f"""A dict structure with kind {node["kind"]}""")
    print(f"""{name}[shape=box,label="{node["kind"]}"];""",file=to_stream)
    for field in node:
        if field == "kind":
            # Treated specially above
            continue
        child = node[field]
        child_name = gen_name()
        dump_node(child, child_name, to_stream)
        print(f"""{name} -> {child_name} [taillabel="{field}"];""")

def dump_list(node: list, name: str, to_stream: IOBase):
    """Dump a representation of a list (e.g., block of statements),
    by formatting it as a record with a port per child.
    """
    if len(node) == 0:
        log.debug("An empty list")
    else:
        log.debug(f"""A list of {node[0]["kind"]}""")
    # First the fields of the record
    print(f'{name}[shape=record,label="',end="",file=to_stream)
    field_sep = ""
    for i in range(len(node)):
        print(f"{field_sep}<e_{i}>{i}", end="", file=to_stream)
        field_sep = "|"
    print("\"];", file=to_stream)
    # Then children and connections to them
    for i in range(len(node)):
        child_name = gen_name()
        dump_node(node[i], child_name, to_stream)
        print(f"{name}:e_{i} -> {child_name};")


def main():
    #TBD: Give this a decent CLI
    source = sys.argv[1]
    ast = load(source)
    print(PROLOGUE)
    dump_node(ast,"root")
    print(EPILOGUE)

if __name__ == "__main__":
    main()









