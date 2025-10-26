import os
import shutil

def replace_3v3_labels(root_dir):
    for root, dirs, files in os.walk(root_dir):
        for file in files:
            if file.endswith(".kicad_sch") or file.endswith(".sch"):
                path = os.path.join(root, file)
                backup = path + ".bak"
                shutil.copy2(path, backup)

                with open(path, "r", encoding="utf-8") as f:
                    data = f.read()

                # Replace +3.3V with +3V3 (case-sensitive)
                new_data = data.replace("+3.3V", "+3V3")

                # Only overwrite if changes were made
                if new_data != data:
                    with open(path, "w", encoding="utf-8") as f:
                        f.write(new_data)
                    print(f"Updated: {path}")
                else:
                    print(f"No changes: {path}")

if __name__ == "__main__":
    directory = input("Enter the KiCad project folder path: ").strip('"')
    replace_3v3_labels(directory)
    print("\n✅ Done! All +3.3V nets renamed to +3V3 (backups saved as .bak).")
