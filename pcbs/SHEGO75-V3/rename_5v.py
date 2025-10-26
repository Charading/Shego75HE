import os
import shutil

def rename_5v_led_labels(project_dir):
    for root, dirs, files in os.walk(project_dir):
        for file in files:
            if "led" in file.lower() and file.endswith(".kicad_sch"):
                path = os.path.join(root, file)
                backup = path + ".bak"
                shutil.copy2(path, backup)
                print(f"Processing: {path}")

                new_lines = []
                changed = False

                with open(path, "r", encoding="utf-8") as f:
                    for line in f:
                        # Only replace isolated +5V labels (property, value, or label lines)
                        if '(property "Value" "+5V")' in line:
                            line = line.replace('+5V', '+5V_LED')
                            changed = True
                        elif '(property "Value" "+5V"' in line:
                            line = line.replace('+5V', '+5V_LED')
                            changed = True
                        elif '(text " +5V")' in line:
                            line = line.replace('+5V', '+5V_LED')
                            changed = True
                        elif '(power_symbol ' in line and '+5V' in line:
                            line = line.replace('+5V', '+5V_LED')
                            changed = True
                        new_lines.append(line)

                if changed:
                    with open(path, "w", encoding="utf-8") as f:
                        f.writelines(new_lines)
                    print(f"✅ Updated +5V → +5V_LED in {file}")
                else:
                    print(f"ℹ️ No +5V labels found in {file}")

if __name__ == "__main__":
    project_dir = input("Enter your KiCad project folder path: ").strip('"')
    rename_5v_led_labels(project_dir)
    print("\n✅ Done! All changes backed up with .bak files.")
