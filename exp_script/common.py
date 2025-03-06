import os
from pathlib import Path

EXP_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(EXP_SCRIPT_DIR)
BUILD_DIR = os.path.join(ROOT_DIR, "build")
# CONFIG_DIR = os.path.join(BUILD_DIR, "conf")
CONFIG_DIR = os.path.join(ROOT_DIR, "conf")
EVAL_SETTING_FILE = os.path.join(EXP_SCRIPT_DIR, "eval_settings.ini")