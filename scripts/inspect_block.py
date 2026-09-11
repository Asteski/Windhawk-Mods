from pathlib import Path
p=Path('mods/asteski-vd-auto-launcher.wh.cpp')
text=p.read_text(encoding='utf-8')
start=text.find('// ==WindhawkModSettings==')
end=text.find('// ==/WindhawkModSettings==')
block=text[start:end]
for i,line in enumerate(block.splitlines(),1):
    if i>=1 and i<=40:
        print(f'{i:02d}: {line!r}')
