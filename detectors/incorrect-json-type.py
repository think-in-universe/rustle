#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.getcwd())
from utils.core import *
import re

PROJ_PATH = os.environ['NEAR_SRC_DIR']

if len(sys.argv) == 2:
    if sys.argv[1] == '-h' or sys.argv[1] == '--help':
        print('Usage: detectors/non-callback-private.py [path to project]')
        sys.exit()
    else:
        PROJ_PATH = sys.argv[1]
elif len(sys.argv) > 2:
    print('Usage: detectors/non-callback-private.py [path to project]')
    sys.exit()

TMP_PATH = os.environ['TMP_DIR']

structMember_dict = dict()  # <memberName, memberType>
with open(TMP_PATH + '/.struct-members.tmp', 'r') as f:
    structNum = int(f.readline())
    for i in range(structNum):
        structName, structMemNum, structFile = f.readline().split('@')
        structMemNum = int(structMemNum)
        structMember_dict[structName] = dict()
        for j in range(structMemNum):
            memName = f.readline().strip()
            memType = f.readline().strip()
            structMember_dict[structName][memName] = memType  # <memberName, memberType>

with open(TMP_PATH + '/.incorrect-json-type.tmp', 'w') as out_file:
    for path in getFiles(PROJ_PATH):
        results = findFunc(path)
        for func in results:
            func_name = func['name']
            note_issue = ''

            typesWITHu128 = ['u128', 'u64', 'Balance']
            for structName, structDetail in structMember_dict.items():
                for typeWITHu128 in typesWITHu128:
                    if typeWITHu128 in structDetail.values():
                        typesWITHu128.append(structName)
                        break
            if func['visibility'] == 'public(near_bindgen)':
                for i in typesWITHu128:
                    if re.search(r'(:|\s)' + i + r'(,|\s|$)', func['args']):
                        note_issue += i + ' with too large integer type in public function args, use near_sdk::json_types instead; '
                        break
                for i in typesWITHu128:
                    if i == func['return']:
                        note_issue += i + ' with too large integer type in public function return value, use near_sdk::json_types instead; '
                        break
            if note_issue != '':
                print('[!] ' + note_issue)
                out_file.write(func['struct'] + '::' + func['struct_trait'] + '::' + func_name + '@' + path + '@' + note_issue + '\n')
    out_file.close()