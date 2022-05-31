#!/bin/python
import sys


def countChar(line, percentStart, percentEnd):
    print #python 2 syntax
    print("original: " + line)
    charCount = {}
    length = len(line)
    p10 = length // 10
    start = p10 * percentStart
    end = p10 * percentEnd
    if percentEnd == 10:
        end = length
    substring = line[start:end]
    sLength = len(substring)
    print
    print("{}0 percentile to {}0 percentile : {}".format(percentStart, percentEnd, substring))
    for ch in substring:
        if ch == '\n': continue
        if ch not in charCount:
            charCount[ch] = 1
        else:
            charCount[ch] = charCount[ch] + 1
    for ch in charCount:
        charCount[ch] = float(charCount[ch]) / float(sLength)
    print
    print(charCount)
    print

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: 2 integer arguments: [0, 10]")
        exit()
    try:
        ps = int(sys.argv[1])
        pe = int(sys.argv[2])
        if ps > 10 or ps < 0:
            raise Exception("Range")
        if pe > 10 or pe < 0:
            raise Exception("Range")
    except:
        print("Usage: 2 integer arguments: [0, 10]")
        exit()
    header = 2 # ignore two lines of header
    for line in sys.stdin:
        if header > 0:
            header -= 1
            continue
        countChar(line[:len(line)-1], ps, pe) #discard \n from the end of the line
        break