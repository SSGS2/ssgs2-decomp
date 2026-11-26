#!/usr/bin/env python3
"""List the functions that do not match the original yet, smallest first.

Reads the report the build writes, so run `python -m ninja` first.

    python tools/unmatched.py                      # the 50 smallest
    python tools/unmatched.py --min-percent 90     # only ones that are nearly there
    python tools/unmatched.py --path src/game/golf # only one folder
    python tools/unmatched.py --limit 0            # all of them
"""
import argparse
import json

ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
ap.add_argument("--report", default="build/R2PE9B/report.json")
ap.add_argument("--min-percent", type=float, default=0, help="lowest current match percentage")
ap.add_argument("--path", default="src/", help="only files under this path")
ap.add_argument("--limit", type=int, default=50, help="how many to show (0 = all)")
args = ap.parse_args()

rows = []
for unit in json.load(open(args.report, encoding="utf-8"))["units"]:
    meta = unit.get("metadata", {})
    source = "" if meta.get("auto_generated") else meta.get("source_path", "").replace("\\", "/")
    if not source.startswith(args.path):
        continue
    for fn in unit.get("functions", []):
        percent = float(fn.get("fuzzy_match_percent", 0))
        if args.min_percent <= percent < 100:
            rows.append((int(fn.get("size", 0)), -percent, fn["name"], unit["name"]))
rows.sort()

print(f"{len(rows)} functions do not match yet. Size is in bytes; compare one with:")
print("  build/tools/objdiff-cli diff -p . -u <unit> -o - <function>\n")
print(f"{'size':>6} {'match':>6}  function  (unit)")
for size, percent, name, unit in rows[:args.limit or None]:
    print(f"{size:6} {-percent:5.1f}%  {name}  ({unit})")
