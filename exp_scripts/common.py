import os
from pathlib import Path

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
BUILD_DIR = os.path.join(ROOT_DIR, "build")
CONFIG_DIR = os.path.join(BUILD_DIR, "conf")
EVAL_SETTING_FILE = os.path.join(SCRIPT_DIR, "eval_settings.ini")
CODE_TEST_BIN = "CodeTest"