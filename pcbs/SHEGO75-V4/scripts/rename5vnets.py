import re
import sys
from pathlib import Path

# Configuration
PROJECT_DIR = Path('.')  # current folder
OLD_NET = '+5V'
NEW_NET = '+5V_LED'
BACKUP_EXT = '.bak'

def safe_replace(file_path, old, new):
    text = file_path.read_text(encoding='utf-8')
    backup = file_path.with_suffix(file_path.suffix + BACKUP_EXT)
    backup.write_text(text, encoding='utf-8')

    # Regex that matches +3.3V as a standalone token inside parentheses or quotes
    pattern = re.compile(r'(?<=\()' + re.escape(old) + r'(?=[\)\s"])|(?<=")' + re.escape(old) + r'(?=")')
    new_text = pattern.sub(new, text)

    # Also catch free-floating +3.3V tokens not handled above
    new_text = re.sub(r'\b\+3\.3V\b', new, new_text)

    file_path.write_text(new_text, encoding='utf-8')
    print(f'✔ Updated {file_path.name}')

# Process all schematic & PCB files in this folder
for file in PROJECT_DIR.glob('**/*.kicad_sch'):
    safe_replace(file, OLD_NET, NEW_NET)

for file in PROJECT_DIR.glob('**/*.kicad_pcb'):
    safe_replace(file, OLD_NET, NEW_NET)

print('✅ Done! Backups saved with .bak extension.')
