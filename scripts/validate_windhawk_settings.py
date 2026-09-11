import re
from pathlib import Path

p = Path(r"mods/asteski-vd-auto-launcher.wh.cpp")
if not p.exists():
    print("ERROR: file not found:", p)
    raise SystemExit(1)

text = p.read_text(encoding='utf-8')
start = text.find('// ==WindhawkModSettings==')
end = text.find('// ==/WindhawkModSettings==')
if start == -1 or end == -1:
    print('No Windhawk settings block found')
    raise SystemExit(1)

block = text[start:end]
lines = block.splitlines()

errors = []
current_key = None
for i, line in enumerate(lines, start=1):
    # find mapping entry lines like "- key: value"
    m = re.match(r"\s*-[ ]([A-Za-z0-9_]+):(.*)$", line)
    if m:
        current_key = m.group(1)
        continue
    # skip blank lines and comment markers
    if line.strip() == '' or line.strip().startswith('/*') or line.strip().startswith('*/'):
        continue
    # metadata lines should start with two spaces then $name: or $description:
    m2 = re.match(r"(\s*)(\$[A-Za-z]+):", line)
    if m2:
        indent = len(m2.group(1))
        if indent != 2:
            errors.append((i, indent, line))
    else:
        # other lines that are indented but not metadata are ok
        pass

if not errors:
    print('OK: settings block looks good (no indentation errors detected)')
else:
    print('Indentation errors detected:')
    for ln, indent, txt in errors:
        print(f'  Line {ln}: indent={indent} -> {txt}')

print('\n--- Block preview ---\n')
print(block)
