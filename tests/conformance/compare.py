import sys


def main():
    if len(sys.argv) < 3:
        print("usage: compare.py cases.tsv output [name]", file=sys.stderr)
        return 2

    expected = {}
    order = []
    with open(sys.argv[1], encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 4:
                continue
            expected[parts[0]] = parts[3]
            order.append(parts[0])

    got = {}
    with open(sys.argv[2], encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if not line:
                continue
            parts = line.split("\t", 1)
            if len(parts) != 2:
                continue
            got[parts[0]] = parts[1]

    passed = failed = skipped = 0
    for cid in order:
        value = got.get(cid)
        if value is None:
            print(f"FAIL {cid}: missing")
            failed += 1
        elif value.startswith("SKIP:"):
            print(f"SKIP {cid}: {value[5:]}")
            skipped += 1
        elif value.startswith("FAIL:"):
            print(f"FAIL {cid}: {value[5:]}")
            failed += 1
        elif value != expected[cid]:
            print(f"FAIL {cid}: expected {expected[cid]!r} got {value!r}")
            failed += 1
        else:
            passed += 1

    name = sys.argv[3] if len(sys.argv) > 3 else sys.argv[2]
    print(f"SUMMARY {name} passed={passed} failed={failed} skipped={skipped}")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
